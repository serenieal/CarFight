# CarFight InGame UI Design

- 문서 버전: v0.35.1
- 작성일: 2026-07-30
- 최근 갱신일: 2026-08-23
- 문서 상태: Supporting Design Reference / CF-FQ-032 Historical implementation checkpoints retained
- 기능 ID: `CF-FQ-032`
- Current Applicability: `CF-FQ-039`는 이 문서의 구조·소유권 설계만 참고하며 현재 구현 상태는 `Systems/UI/InGameUI.md`, 현재 작업 상태는 `InGameUIVisualPlan.md`가 우선한다.
- 대표 Plan: `InGameUIVisualPlan.md v0.1.2`
- 시각 콘셉트: `InGameUIVisualConcept.md v0.8.1`
- 제작 규격 초안: `InGameUIStyleSpec.md v0.86.1`
- 자산화 실행 규격: `InGameUIAssetizationSpec.md v0.16.1`
- 1440p Wireframe: `ConceptArt/CFHUDWireframe_1440p.xml`
- Vehicle Panel 상세: `InGameUIVehiclePanelSpec.md`
- Vehicle Panel Wireframe: `ConceptArt/CFVehiclePanel_1440p.xml`

---

## 1. 설계 목표

현재 검증된 외부 3인칭 조준 UI를 보존하면서, 인게임 UI를 LocalPlayer 소유 공통 구조로 옮길 수 있는 구현 계약을 정의한다.

핵심 목표:

```text
게임플레이 객체는 상태를 계산한다.
Presenter는 표시 데이터를 만든다.
Widget은 표시만 한다.
UI Subsystem은 화면 수명과 레이어를 관리한다.
```

---

## 2. 소유권

### ACFPlayerController

최소 책임:

- Pawn 수명과 독립된 Pause 입력
- System, Gameplay와 UI Input Mapping Context 수명
- Possessed Pawn 변경 통지
- 게임 입력과 UI 입력 모드 전환
- 마우스 커서, 포커스와 Pause 해제 입력 보장
- `UCFUISubsystem`에 화면 전환 요청 전달

비책임:

- Widget 직접 생성과 레이어 배치
- 차량 상태 계산
- 타겟 선택과 락온 계산
- HUD View Data 생성

`ACFPlayerController`는 UI 요청의 입력 진입점이며 UI Root 소유자가 아니다.

### UCFUISubsystem

권장 부모:

```text
ULocalPlayerSubsystem
```

책임:

- 로컬 플레이어별 UI 상태 관리
- 현재 World용 UI Root 생성과 제거
- Game, HUD, Screen, Panel, Menu, Modal, System과 Debug 레이어 관리
- 인게임 HUD 열기와 닫기
- Pause Menu 전환
- 현재 화면 상태 보관
- Pawn 교체 시 표시 데이터 소스 재연결
- 향후 타이틀·차고·결과 화면 확장 진입점

비책임:

- 피해 계산
- 타겟 선택
- 센서 탐지
- 락온 계산
- 무기 자원 소비

Subsystem은 LocalPlayer 수명을 가지지만 Root Widget은 World 수명을 가진다.

```text
LocalPlayer 유지
→ UCFUISubsystem 유지

World 종료
→ 이전 Root 제거
→ 이전 PlayerController·Pawn·Component 이벤트 해제

새 World 시작
→ 새 PlayerController 확인
→ 새 Root 생성
→ 새 Pawn 데이터 소스 연결
```

Subsystem과 Presenter의 Pawn·Actor 참조는 `TWeakObjectPtr`를 기본으로 한다.

UI 디자인 자산의 기본 참조는 Widget이 콘텐츠 경로를 직접 Load하는 방식으로 공급하지 않는다. D1-09A에서 `UCFUISubsystem`에 Config 기반 Soft Reference 진입점을 추가해 기본 Style·Density·HUD Layout을 World/LocalPlayer 수명 안에서 한 번 해석하고 자식 Widget에 전달한다. Data Asset 누락 시에는 D1-08 Native Safe Fallback을 유지한다.

### UCFUIRootWidget / 향후 WBP_CFUIRoot

현재 `UI-P0-01B`는 Unreal Asset을 만들지 않고 C++ `UCFUIRootWidget`을 실제 Root로 사용한다.

현재 책임:

- `UCanvasPanel` 기반 Root와 8개 표준 레이어 생성
- 레이어별 명시적 ZOrder
- Widget 추가·제거·레이어 정리
- `EnsureLayerTree()`를 통한 CreateWidget·Automation 공통 초기화

향후 `WBP_CFUIRoot`를 도입해도 다음 책임만 추가한다.

- 해상도와 Safe Zone
- 디자이너가 조정할 시각 컨테이너
- 레이어 시각 구성

Root Widget은 게임플레이 데이터를 직접 조회하지 않는다.

### HUD Layout Profile 확장 경계

P0에서는 승인된 기본 16:9 TPS 배치를 고정 사용한다. 다만 후속 사용자 배치 커스터마이징을 위해 각 HUD 모듈을 안정적인 Slot ID로 관리하고 위치 데이터를 별도 Layout Profile로 분리한다.

권장 타입:

```text
ECFHUDSlotId
FCFHUDSlotLayout
UCFHUDLayoutData
```

기본 Profile 후보:

```text
DA_CFHUDLayout_Default
```

기본 Slot:

```text
MissionSummary
AlertFeed
TargetPanel
VehiclePanel
RadarPanel
WeaponPanel
ReticleLayer
```

구현 원칙:

- `WBP_CFInGameHUD`는 Slot Container와 Safe Zone만 소유한다.
- 각 기능 Widget은 자신의 내부 정보 구조만 소유하고 화면 좌표를 하드코딩하지 않는다.
- 기본 Profile이 Anchor·Alignment·NormalizedPosition·PixelOffset·DesiredSize를 제공한다.
- 후속 커스텀 배치는 기본 Profile 위에 Slot Override만 저장한다.
- Profile Version을 두어 새 Slot 추가와 기본값 변경 시 기존 사용자 설정을 마이그레이션할 수 있게 한다.
- 현재 범위에서는 사용자 편집 화면, 저장과 Reset UI를 구현하지 않는다.

### CarFight 전체 UI 커스터마이징 불변 계약

이 계약은 `WeaponPanel`에만 적용하지 않는다. HUD, Pause, Target, Radar, Alert, Garage, Fitting, Inventory, Mission, Result, Loading과 Tutorial을 포함한 CarFight 전체 UI의 공통 설계 원칙이다.

```text
Gameplay System
→ 상태와 판정 소유

Presenter / ViewModel
→ 플레이어에게 표시할 의미 데이터와 포맷 소유

Blueprint Visual Widget
→ 배치·브러시·아이콘·폰트·애니메이션 소유

Style / Layout / Density Data Asset
→ 공통 색상·크기·간격·패널 위치와 밀도 기본값 소유

User Override
→ 사용자별 위치·Scale·Opacity·Visibility와 선택 Theme 소유
```

필수 구조:

- 하나의 거대한 Widget에 전체 UI 기능과 시각 요소를 몰아넣지 않는다.
- Root는 Layer와 Slot Container만 소유하고 각 기능은 교체 가능한 독립 Widget으로 구성한다.
- 각 주요 모듈은 안정적인 `PanelId` 또는 `SlotId`를 가지며 Visual Widget Class를 설정에서 교체할 수 있어야 한다.
- C++은 Gameplay 데이터를 직접 그리지 않고, Blueprint는 Gameplay Component를 직접 Cast해 판정을 재계산하지 않는다.
- 색상, Font, Padding, Spacing, Border, Radius, Motion, Panel Size와 기본 Visibility를 Widget 그래프에 무분별하게 하드코딩하지 않는다.
- 기본값은 Style·Layout·Density Data Asset이 소유하고 개별 Widget Override는 명시적인 `bOverride...` 계약을 사용한다.
- 지원되지 않거나 표시할 필요가 없는 항목은 `Collapsed` 처리하고 부모 자동 Layout이 빈 공간까지 제거해야 한다.
- 개별 정보 Panel 내부는 `VerticalBox`, `HorizontalBox`, `GridPanel`, `WrapBox`, `SizeBox`, `ScaleBox`, `WidgetSwitcher` 같은 자동 Layout을 우선한다.
- `CanvasPanel` 고정 좌표는 Root Layer, World Projection, Reticle, Radar Blip과 Screen Edge Marker처럼 실제 좌표 배치가 필요한 범위로 제한한다.
- 기본 Visual Asset은 수정 불가능한 최종 UI가 아니라 복제·상속·교체할 수 있는 기본 Preset이다.
- 사용자가 외형 Widget을 교체해도 Presenter 입력 계약과 Gameplay 시스템이 깨지지 않아야 한다.

전역 밀도 Profile 후보:

```text
Compact
Standard
Expanded
Custom
```

화면비 Profile 후보:

```text
DA_CFHUDLayout_16x9
DA_CFHUDLayout_21x9
DA_CFHUDLayout_32x9
DA_CFHUDLayout_Custom
```

전역 Density를 기본값으로 사용하되 `VehiclePanel`, `WeaponPanel`, `RadarPanel`, `TargetPanel`처럼 주요 모듈은 별도 Density Override를 허용한다. `Compact`는 정보 의미를 삭제하는 모드가 아니라 Padding·보조 Caption·장식과 Tile 크기를 줄이고 핵심 수치·상태를 유지하는 모드다.

향후 사용자 편집 확장 경계:

```text
PanelId
PositionOffset
RenderScale
Opacity
bVisible
DensityOverride
ThemeOverride
LayoutProfileVersion
```

P0에서 HUD 편집 화면과 SaveGame 저장을 구현하지 않더라도 위 필드를 추가할 수 없는 고정 구조로 만들지 않는다. 새 Panel이나 Token이 추가되면 Profile Version과 기본값으로 기존 사용자 Override를 안전하게 마이그레이션한다.

모든 UI 구현·리뷰는 다음 질문을 통과해야 한다.

```text
이 색상·크기·간격을 Data Asset에서 바꿀 수 있는가?
이 모듈의 위치와 Scale을 Layout Profile에서 바꿀 수 있는가?
이 Visual Widget을 Blueprint Class 교체로 대체할 수 있는가?
표시 항목을 숨기면 빈 공간도 함께 사라지는가?
외형을 바꿔도 Gameplay 판정과 View Data 계약이 유지되는가?
16:9·21:9·32:9 Profile을 별도로 적용할 수 있는가?
```

---

## 3. UI 레이어 계약

```text
Game
HUD
Screen
Panel
Menu
Modal
System
Debug
```

권장 우선순위:

```text
Game < HUD < Screen < Panel < Menu < Modal < System < Debug
```

레이어 의미:

```text
Screen
= Title, Garage, Fitting, Mission Select와 Result 같은 주요 전체 화면

Panel
= 현재 Screen 또는 InGame 위에 추가되는 상세 정보 패널
```

`CF-FQ-032`에서는 Screen Layer와 전환 진입점만 준비하며 실제 Fitting Screen은 생성하지 않는다. `UI-P0-01B`에서는 C++ CanvasPanel 레이어를 실제 구현했고, Screen과 Modal 전환 시 PlayerController의 Input Mode·Cursor·Focus를 함께 적용한다. `Pause Menu`가 열려도 HUD를 파괴하지 않는 정책은 유지하지만 실제 게임 Pause는 `UI-P0-02` 범위다.

---

## 4. 화면 상태

권장 열거형:

```text
ECFUIScreenState

None
InGame
Paused
Modal
Transition
```

권장 View Mode:

```text
ECFHUDViewMode

ThirdPerson
Turret
```

현재 P0는 `ThirdPerson`만 완성 대상으로 한다. `Turret`은 데이터와 표시 전환 계약만 준비할 수 있다.

---

## 5. 완전 Pause 설계

### 구현된 진입

```text
Pause 입력
→ ACFPlayerController가 UCFUISubsystem에 요청
→ UISubsystem이 Modal·World·Root 상태 확인
→ UCFPauseMenuWidget을 Menu Layer에 단일 생성
→ PlayerController가 차량 Drive·Look 입력 중립화
→ PlayerController가 눌린 키 상태 Flush
→ APlayerController::SetPause(true)
→ World Pause 성공 확인
→ GameAndUI + Cursor + Continue Focus
→ Pawn Gameplay 입력은 억제하고 PlayerController Pause·Back 공통 입력은 유지
→ 화면 상태 Paused
```

### 구현된 해제

```text
Pause 재입력 / Back / Continue 버튼
→ APlayerController::SetPause(false)
→ 눌린 키 상태 Flush
→ Pause Menu 이벤트 해제·Menu Layer 제거
→ 기존 Primary Screen이 있으면 UIOnly 복귀
→ 없으면 InGame GameOnly 복귀
```

### 입력 잔류 제거

```text
ACFVehiclePawn::ClearGameplayInputForPause()
→ VehicleMove 결과와 입력 소유권 초기화
→ 목표·현재 Steering 초기화
→ UCFVehicleDriveComp::ClearDriveInputs()
   - Throttle 0
   - Steering 0
   - Brake 0
   - Handbrake false
→ UCFVehicleCameraComp::ClearLookInput()
→ Controller FlushPressedKeys()
```

차량의 실제 물리 Velocity는 강제로 0으로 만들지 않는다. Pause 전 이동 관성은 World Pause 동안 정지된 상태로 보존되며 해제 후 물리 시뮬레이션이 이어진다.

### Launcher·Projectile·Timer 보존

- 진행 중 Ripple·Salvo에 `CancelFireSequence()`를 호출하지 않는다.
- Launcher SequenceRuntime, Projectile 활성 상태와 Motor 상태를 재생성하거나 Reset하지 않는다.
- LauncherComp, Projectile Actor, ProjectileMovement와 ProjectileMotor는 `Tick Even When Paused`를 사용하지 않는다.
- Projectile Life Timer와 일반 게임 진행 Timer는 World `TimerManager`의 게임 시간을 사용한다.
- 따라서 Pause 동안 상태는 보존되고 게임 시간 진행만 멈추며, Pause 해제 후 같은 런타임 상태에서 이어진다.

### Pause Menu 수명

- `UCFUISubsystem`이 World별 `UCFPauseMenuWidget` 인스턴스를 소유한다.
- Pause Menu는 `Menu` 레이어에 정확히 하나만 존재한다.
- Root 해제, Controller 해제와 World Cleanup에서 활성 Pause를 해제하고 Menu를 제거한다.
- 현재 단계는 에셋 없는 C++ Pause Menu이며 향후 Blueprint 파생 Widget을 사용해도 Continue 이벤트와 수명 계약은 유지한다.

### 필수 규칙

- Pause 상태를 Widget Visibility만으로 표현하지 않는다.
- `Global Time Dilation = 0`으로 대체하지 않는다.
- 게임플레이 Actor에 `Tick Even When Paused`를 추가하지 않는다.
- Pause 중 입력 모드는 `GameAndUI`를 사용하되 Pawn Gameplay Context와 Pawn Input은 억제한다.
- 이 구조로 Continue 버튼 Focus와 PlayerController Pause·Back 공통 입력을 동시에 유지한다.
- HUD는 표시 수명만 유지하며 Gameplay 입력을 받지 않는다.
- 실제 입력 장치, Focus, 물리와 시퀀스 재개는 사용자 PIE로 최종 확인한다.

---

## 6. View Data 구조

### FCFVehicleHUDData

후보 필드:

```text
SpeedKmh
DriveDirection
GearDisplayText
bHandbrakeActive
bBoostAvailable
CurrentShield
MaximumShield
CurrentArmor
MaximumArmor
CurrentVehicleIntegrity
MaximumVehicleIntegrity
ComponentStates
```

### FCFWeaponHUDData

후보 필드:

```text
WeaponDisplayName
WeaponIcon
WeaponGroupIndex
bIsSelected
FireState
ResourceChannels
LockViewData
```

### FCFResourceViewData

후보 필드:

```text
ResourceType
CurrentValue
MaximumValue
NormalizedValue
PrimaryText
SecondaryText
State
bIsVisible
```

권장 ResourceType:

```text
Ammo
ReserveAmmo
VehicleBattery
WeaponCharge
Heat
Cooldown
Reload
None
```

### FCFTargetHUDData — 구현 준비 계약

현재 Source의 `FCFTargetHUDData v1.0.0`은 선택 여부, `TargetId`, 이름, 관계, 분류, `InformationLevel`, `TrackState`, 거리 슬롯까지만 가진다. 아래 계약은 **후속 Target Knowledge / Sensor Runtime이 생겼을 때 Source를 확장하기 위한 구현 준비안**이며 이번 문서 작업에서 C++ 타입을 실제 추가하지 않는다.

Target Panel은 하나의 거대한 Flat 구조체에 모든 값을 계속 추가하기보다 의미별 Sub-ViewData로 분리한다.

```text
FCFTargetHUDData
├─ Availability
├─ bHasSelectedTarget
├─ bSelectedTargetValid
├─ TargetId
├─ ContactState
├─ Tracking
├─ Identity
├─ Analysis
├─ Defense
├─ Weapons[]
└─ Modules[]
```

#### Target UI 전용 상태 타입 후보

```text
ECFTargetContactViewState
- None
- Live
- LastKnown
- DestroyedHold
- LostTransition

ECFTargetKnowledgeState
- Unknown
- Estimated
- Known
- Unavailable
- NotApplicable

ECFTargetIntelFreshness
- Current
- Stale

ECFTargetAnalysisViewState
- Hidden
- Building
- Decaying
- CompletedTransition

ECFTargetIntelCategory
- Identification
- Shield
- Weapon
- Armor
- Vehicle
- DamageModule

ECFTargetDistanceViewMode
- None
- LiveDistance
- LastKnownPositionDistance
```

`ECFUIViewAvailability`은 채널 자체의 `Unknown / Unavailable / KnownZero / Known`을 구분하는 현재 공통 타입으로 유지한다. `Estimated`, `NotApplicable`, 동적 정보의 `Stale`은 그 타입에 억지로 끼워 넣지 않고 Target Intelligence 전용 상태축으로 분리한다.

`ECFTargetTrackState`의 `Visible / Occluded / Estimated / SignalLost`는 현재 TargetSelect 추적 품질이고, `Live / LastKnown / DestroyedHold`는 Sensor/Selection 수명 상태이므로 같은 enum으로 합치지 않는다.

#### FCFTargetTrackingHUDData 후보

```text
ContactId
ContactState
TrackState
DistanceAvailability
DistanceMode
DistanceMeters
SpeedAvailability
SpeedKmh
SpeedFreshness
```

`DistanceMode`는 최소 `LiveDistance / LastKnownPositionDistance / None` 의미를 가져 Widget이 `DISTANCE`와 `LAST POS`를 자체 추론하지 않게 한다. Last Known의 `DistanceMeters`는 플레이어 현재 위치에서 저장된 Last Known 위치까지의 거리이며 Target 현재 위치 추정값이 아니다.

속도는 실제 Sensor가 제공할 때만 사용한다. Last Known의 마지막 속도를 남기면 `SpeedFreshness=Stale`로 명시하며 Presenter가 `LAST SPEED` 의미를 만든다. Widget은 마지막 위치와 속도로 Target 위치를 외삽하지 않는다.

#### FCFTargetIdentityHUDData 후보

```text
InformationLevel
Relation
Category
NameKnowledge
DisplayName
TypeKnowledge
TypeText
FactionKnowledge
FactionText
```

Name / Type / Faction은 서로 독립 Knowledge를 가진다. `ID1 Hostile`이라고 이름이나 Faction을 자동 Known으로 만들지 않고, `ID2`도 실제 Source가 제공하지 않은 필드는 Unknown/NotApplicable을 유지한다.

`TargetId`, 내부 Actor Name, Object Path, Component Name은 Player-facing Text fallback으로 사용하지 않는다. Provider가 명시한 DisplayName/ShortDisplayName만 표시한다.

#### 공통 Target Intelligence 값 규칙

정밀 숫자와 질적 상태를 같은 필드로 위조하지 않기 위해 Ratio 계열은 최소 다음 의미를 함께 가진다.

```text
KnowledgeState
Freshness
bHasNormalizedValue
NormalizedValue
QualitativeText
```

- `Known + bHasNormalizedValue=true`일 때만 정확한 0~1 Ratio를 Bar/Percent로 표시한다.
- `Estimated`는 Provider가 제공한 `LOW / DAMAGED / ACTIVE` 같은 질적 값 또는 추정 범위만 사용한다.
- `Stale`은 마지막 값을 유지하되 현재값처럼 갱신하지 않는다.
- `Known Zero`에 해당하는 실제 0과 `Unknown`은 절대 같은 0% Bar로 합치지 않는다.

#### FCFTargetAnalysisHUDData 후보

```text
State
Category
ProgressAvailability
NormalizedProgress
AnalysisRevision
CompletionRevision
LastCompletedCategory
```

`Building`은 정상 증가, `Decaying`은 조건 손실 후 감소, `Hidden`은 분석 없음/Progress 0, `CompletedTransition`은 짧은 완료 Presentation 의미다.

`AnalysisRevision`은 현재 진행 상태 변경을, `CompletionRevision`은 실제 Category 완료 사건만 식별한다. 다음 Category가 즉시 시작되더라도 Presenter가 새 정보 Highlight를 한 번만 처리할 수 있게 하며, 단순 10Hz Refresh가 완료 효과를 재발생시키지 않게 한다.

Analysis Build/Decay 속도, Range, LOS/Occlusion, Category Queue와 완료 판정은 Scanner/Tactical Analysis Gameplay Provider가 소유한다. Target Panel은 `NormalizedProgress`를 계산하거나 Category를 선택하지 않는다.

#### FCFTargetDefenseHUDData 후보

```text
Shield
Armor[6]
VehicleIntegrity
```

Shield / Vehicle 항목:

```text
KnowledgeState
Freshness
bHasNormalizedValue
NormalizedValue
QualitativeText
```

Armor 항목:

```text
ArmorDirection
KnowledgeState
Freshness
bHasNormalizedValue
NormalizedValue
QualitativeText
```

Armor는 `Front / Rear / Left / Right / Top / Bottom` 여섯 방향을 ViewData에서 끝까지 보존한다. 일부 방향만 Known이어도 평균 Armor 하나로 합치지 않는다. Armor Block이 활성화된 뒤 Unknown 방향은 해당 Cell을 유지해 `???`를 표시할 수 있다.

현재 CF-FQ-033의 실제 적 차량 Defense 상태를 Target UI가 직접 읽어 공개하면 안 된다. Scanner/Knowledge Provider가 해당 정보가 Player에게 공개됐다고 명시한 경우에만 이 구조를 채운다.

#### FCFTargetWeaponIntelData 후보

```text
StableDisplayId
KnowledgeState
DisplayOrder
FamilyText
CountKnowledge
Count
MountSummaryText
```

Weapon Instance를 그대로 나열하지 않고 Provider가 Player-facing Family Summary로 변환한다. 정확한 Count가 Known일 때만 `Count`를 사용하며, Estimated 수량은 Provider가 `MULTIPLE` 또는 범위 Text를 제공한다. Ammo/Heat/Charge/Cooldown/Reload/LauncherSequence는 기본 Target Weapon Intelligence에 포함하지 않는다.

#### FCFTargetModuleIntelData 후보

```text
StableModuleId
KnowledgeState
Freshness
DisplayOrder
ModuleDisplayName
SeverityText
FunctionalStateText
```

Module Damage Runtime이 없는 현재에는 배열을 비우고 Damage Block을 Collapse한다. Widget이 Component 이름, Bone, GameplayTag, 휠 Index를 Player-facing 이름으로 변환하지 않는다.

후속 Runtime에서 Engine/Wheel/Turret 등이 추가돼도 이 Row 계약을 재사용하며 휠 개수 4개를 하드코딩하지 않는다. `Severity`와 `FunctionalState`는 Provider가 별도 의미로 제공하고 Widget이 HP나 속도를 보고 추정하지 않는다.

#### Target Lifecycle 공급 규칙

```text
Live
→ Tracking Current / 분석 가능

LastKnown
→ Identity·정적 Intel 유지
→ 동적 Intel Stale
→ 신규 Analysis 시작 금지
→ 기존 미완료 Analysis는 Decay 가능

DestroyedHold
→ Destroyed Known 명시
→ Analysis 즉시 종료
→ 새 Intel 획득 금지
→ UI 선택 Hold만 유지

LostTransition
→ Sensor Provider가 Contact Lost를 확정한 직후의 짧은 Presentation
→ 이후 No Target
```

`Contact Lost` 만료와 `Destroyed` 판정은 Widget Timer가 소유하지 않는다. Sensor/Knowledge/Selection 수명 Provider가 상태를 확정하고 HUD는 결과만 표시한다. `DestroyedTargetHoldDuration=1.25초`도 Widget Blueprint Literal이 아니라 상위 Selection/Presentation 정책 값으로 유지한다.

### FCFRadarHUDData / FCFRadarContactHUDData — 구현 준비 계약

Radar는 **Sensor Contact Runtime을 표시용 좌표와 상태로 변환한 데이터만** 소비한다. Widget은 World Actor를 검색하거나 Sensor 탐지 판정을 수행하지 않는다.

데이터 흐름은 다음으로 고정한다.

```text
Sensor / Contact Tracking Runtime
→ ContactId, WorldLocation, Tracking State, Relation/Identification, Sensor Range, Scan State
→ HUD Adapter / Presentation Resolver
→ Heading-Up 상대 좌표 + Display Range 정규화 + 고저차 정규화
→ FCFRadarHUDData
→ WBP_CFRadarPanel
```

#### ContactId와 TargetId 분리

```text
ContactId
= 현재 Sensor Tracking Record의 안정 ID
= ID0 Unknown Contact에도 존재 가능

ResolvedTargetId
= 신뢰 가능한 Entity Resolution이 끝난 뒤 연결되는 Target/Entity ID
= 미확인 Contact에서는 None 가능
```

Contact가 존재한다는 이유만으로 Entity Identity를 만들어내지 않는다. `Contact Lost → 재탐지`에서 새 Contact Record가 만들어져도 신뢰 가능한 Entity Resolution이 있으면 보존된 정적 Knowledge를 재연결할 수 있지만 자동 Target 재선택은 하지 않는다.

#### ECFRadarContactViewState 후보

```text
Live
LastKnown
DestroyedHold
```

`ContactLost`는 목록에서 계속 유지할 영속 Contact 상태가 아니라 Contact 제거 사건이다. Fade Out 같은 짧은 Presentation이 필요하면 Presenter의 이전 Snapshot/Revision으로 처리하며 Widget이 자체 Last Known Timer를 시작하지 않는다.

#### FCFRadarContactHUDData 확장 후보

```text
ContactId
ResolvedTargetId
Relation
InformationLevel
ContactState
NormalizedPosition
NormalizedAltitude
bShowAltitudeStalk
bSelected
bInsideDisplayRange
bShowSelectedEdgeMarker
SelectedEdgeDirection
DiscoveryRevision
```

- `NormalizedPosition`은 현재 `DisplayRangeMeters` 기준 Radar 평면 좌표다.
- `NormalizedAltitude`는 Scanner의 고저차 표시 범위로 정규화·Clamp한 값이며 실제 World Z를 픽셀로 직접 변환하지 않는다.
- `bShowAltitudeStalk`는 무시 가능한 고저차 범위와 Contact 상태를 Resolver가 반영한 결과다.
- `bInsideDisplayRange=false`인 일반 Contact는 내부 Blip을 표시하지 않는다.
- 표시 범위 밖 **선택 Target만** `bShowSelectedEdgeMarker=true`와 `SelectedEdgeDirection`으로 2-Corner Edge Bracket을 표시할 수 있다.
- `DiscoveryRevision`은 새 Contact Tracking Record가 실제 신규 탐지로 생성될 때만 바뀌는 사건 Revision이다. 단순 10~20Hz Refresh나 같은 Record의 LastKnown→Live 재획득으로 증가시키지 않아 신규 Contact Brightness Flash가 반복되지 않게 한다.

Neutral Contact의 Relation 값은 보존하되 현재 승인 Visual에서 Neutral 전용 기본 Blip 형상은 확정하지 않는다. 구현 시 새 모양을 임의 발명하지 않고 Style 결정 또는 명시적 User 결정을 먼저 받는다.

#### ECFRadarScanViewState 후보

```text
Unavailable
Ready
Scanning
Cooldown
```

후속 JAMMED/OFFLINE 같은 Scanner 장애 상태가 실제 Gameplay에 생기면 확장한다. 현재 없는 상태를 UI만으로 생성하지 않는다.

#### FCFRadarScanHUDData 후보

```text
Availability
State
ProgressAvailability
NormalizedProgress
CooldownAvailability
RemainingCooldownSeconds
ScanCycleRevision
```

- `Scanning`에서만 Sweep을 활성화한다.
- Sweep 회전각은 순수 시각 진행 표현이며 개별 Contact 탐지 판정 시점이 아니다.
- `NormalizedProgress`는 Scanner Runtime의 실제 Scan Cycle 진행도만 사용한다.
- `Cooldown` 남은 시간은 Scanner Provider 값이며 Widget이 시작 시각으로 재계산하지 않는다.
- `ScanCycleRevision`은 실제 Scan 시작/완료/취소 같은 상태 사건을 구분해 일반 Refresh와 분리한다.

#### FCFRadarHUDData 확장 후보

```text
Availability
DisplayRangeAvailability
DisplayRangeMeters
MaximumRangeAvailability
MaximumDetectionRangeMeters
RangePresetIndex
RangePresetCount
bCanZoomIn
bCanZoomOut
Scan
Contacts[]
```

`DisplayRangeMeters`는 현재 UI 확대/축소 범위이고 `MaximumDetectionRangeMeters`는 실제 Scanner/Sensor 성능 상한이다. 둘은 같은 값일 필요가 없다.

Range Zoom 입력 규칙:

```text
Wheel Up
→ 다음 작은 Range Preset
→ Zoom In

Wheel Down
→ 다음 큰 Range Preset
→ Zoom Out
```

Range Preset 값은 Scanner 성능 Profile이 소유하며 모든 Scanner에 동일한 3개 또는 4개 수치를 하드코딩하지 않는다. 기존 디자인 예시 `0.5 / 1 / 2 / 4 km`는 4 km Scanner의 예일 뿐 전역 규칙이 아니다.

Radar의 3개 Range Ring은 **Range 단계 3개를 의미하지 않는다.** 항상 현재 표시 범위의 `25% / 50% / 75%` 기준선이고 Scanner 외곽이 100%다. 실제 Zoom Preset 개수와 독립이다.

`RANGE 현재 / 최대` Text는 `DisplayRangeMeters / MaximumDetectionRangeMeters`에서 Presenter가 Player-facing m/km 문자열을 만든다. Widget이 Unit 변환이나 Scanner Profile 조회를 하지 않는다.

#### Sensor Provider와 UI 책임 경계

Sensor / Scanner Gameplay가 소유:

- Passive Tracking 범위와 탐지 판정
- Visual Acquisition 판정
- Active Scan 360° 탐지와 Scanner Occluder
- Scan Duration / Cooldown / 취소 / 결과 Commit
- Contact 생성·갱신·Last Known 전환·만료
- Last Known WorldLocation
- Relation / Identification 승격 근거
- Scanner 최대 탐지거리와 Range Preset 목록
- Tactical Analysis Range / Capability / Build / Decay

HUD Adapter / Resolver가 소유:

- 플레이어 차량 기준 상대 벡터
- Heading Up 회전
- 현재 Display Range 정규화
- 고저차 정규화와 Stalk 표시 여부
- 일반 Out-of-range Contact 숨김
- 선택 Out-of-range Edge Direction
- `bSelected` 매핑
- Player-facing Range/Scan Text용 display-ready 값

Blueprint Widget이 소유:

- Radar 타원 Surface / Range Ring / Frame
- `◆ / ▼ / ●` 승인 Symbol과 관계색 적용
- Stalk, Selected Bracket, Off-range Edge Bracket 시각 배치
- Scan Sweep 시각 Animation
- Range / Scan Status Text 레이아웃

Widget이 소유하지 않음:

- Actor 검색
- LOS/Scanner Occlusion Trace
- 적대 판정
- Contact Lost Timer
- Scan Cooldown 계산
- Target Identity 추정
- Radar Zoom에 따른 Sensor 성능 변경

#### 갱신 주기

기본 권장 주기:

```text
Sensor Contact / Radar Presentation
→ 10~20 Hz

Scan 시작·완료·취소 / Contact 생성·제거 / Target 선택 변경
→ Event 즉시 Refresh

Style / Layout
→ 변경 사건에서만
```

Radar Contact 위치를 모든 Widget Tick에서 Actor별로 다시 계산하지 않는다. Contact 수가 늘어날 수 있으므로 한 Resolver가 한 주기에서 Player Transform과 Display Range를 한 번 기준으로 전체 Contact를 변환하는 구조를 사용한다.

#### 현재 Source 마이그레이션 경계

현재 `CFHUDViewData.h v1.3.0`의 단순 Target/Radar 타입은 삭제하거나 즉시 이름을 바꾸지 않는다. 실제 Sensor/Knowledge Runtime 착수 시 다음 순서를 사용한다.

```text
1. 새 Sub-ViewData와 enum을 additive 추가
2. 기존 FCFTargetHUDData / FCFRadarHUDData에 새 필드 추가
3. Provider가 실제 Source가 있는 필드만 채움
4. Presenter/Automation을 새 계약으로 이동
5. 기존 Flat 필드는 참조 0 확인 뒤 별도 Migration에서 제거 검토
```

실제 Provider가 없는 동안 Radar는 계속 `Unavailable`, Target Intelligence 추가 Block은 Empty/Unavailable로 유지한다. 문서에 구조가 정의됐다는 이유만으로 Mock 값을 Production Runtime에 넣지 않는다.


---

## 7. 공개 정보 모델

권장 공개 상태:

```text
Unknown
Estimated
Visual
Identified
Scanned
Analyzed
```

단일 Target 단계만으로 모든 필드 공개를 강제하지 않는다.

예:

```text
Identity = Visual
Affiliation = Unknown
Defense = Scanned
Weapon = Estimated
Component = Unknown
```

Widget 표시:

```text
Unknown   → ???
Estimated → 추정 표기
Known     → 실제 값
Unavailable → N/A 또는 해당 행 비활성
```

`Unknown`과 `Unavailable`을 같은 의미로 사용하지 않는다.

---

## 8. 이벤트와 프레임 갱신

이벤트 갱신:

- 선택 타겟 변경
- 식별 정보 변경
- 무기 변경
- 탄약·배터리·열 변경
- 차량 방어 계층 변경
- 부품 상태 변경
- 경고 생성·해제

프레임 갱신:

- SpeedKmh
- Reticle 월드 투영
- Target Marker 화면 좌표
- Radar Contact 상대 위치

가능하면 프레임 갱신도 하나의 Presenter가 수집하고 각 Widget이 Pawn을 별도 조회하지 않는다.

---

## 9. 기존 AimReticle 통합

현재 의미 계약:

```text
Image_CenterDot
= 플레이어 조준 레티클

Image_WeaponReticle
= CurrentMuzzleDirection 기반 터렛 레티클
```

통합 원칙:

1. 의미와 계산 소유권을 변경하지 않는다.
2. `VehicleAimComp`와 `BuildFireFeedbackViewData()` 결과를 계속 사용한다.
3. UI Root 아래로 수명을 옮기더라도 표시 결과가 같아야 한다.
4. 기존 Pawn 생성 경로와 새 Root 생성 경로가 동시에 활성화되지 않게 전환 게이트를 둔다.
5. 사용자 PIE 회귀 통과 전 기존 생성 코드를 제거하지 않는다.

### 9.1 Reticle Layer 의미 분리

Reticle Layer는 중앙에 보이는 모든 조준 정보를 하나의 상태로 합치지 않고 다음 의미를 독립적으로 유지한다.

```text
Command Reticle
→ Image_CenterDot 계열
→ 플레이어가 Camera Aim Trace로 지정한 조준 목표

Turret Reticle
→ Image_WeaponReticle
→ CurrentMuzzleDirection 기반 현재 터렛/총구 방향
→ 착탄점·탄도 예측이 아님

World Target Marker
→ 선택 Target의 월드 위치/Tracking 상태를 화면에 투영
→ Command/Turret Reticle과 별개

Lock Indicator
→ 후속 Fire-Control Provider가 실제 Lock 상태를 제공할 때만 표시
→ 현재 Target 선택만으로 생성하지 않음

Immediate Fire State
→ 기존 FCFVehicleFireFeedbackViewData 결과
→ 발사 성공·Cooldown·Blocked·NoWeapon·정렬 상태 등 짧은 피드백
```

현재 구현에서 실제로 존재하는 것은 Command Reticle, CurrentMuzzleDirection 기반 Turret Reticle, Reticle State와 FireFeedback이다. 별도 `LockProgress`, `LockState`, `TargetLock` Runtime은 현재 소스에서 확인되지 않으므로 Lock UI는 **미래 View Data 계약**으로만 설계하고 가짜 진행도를 만들지 않는다.

### 9.2 Command / Turret Reticle 권한

- `Command Reticle`의 월드 기준점은 기존 Camera Aim 결과가 소유한다.
- `Turret Reticle`은 `bHasValidTurretReticlePoint / TurretReticleWorldLocation`만 소비한다.
- Legacy `WeaponPreviewWorldLocation`, `DirectImpact`, `LaunchDirection`을 제품 Turret Reticle의 착탄점으로 재해석하지 않는다.
- 두 Reticle 사이 화면상 간격은 **플레이어 요구 조준과 현재 총구 방향의 실제 차이**를 자연스럽게 보여주는 정보다. UI가 둘을 인위적으로 합치거나 보간해 일치시켜 보이지 않는다.
- 정렬 중 `bAllowFireWhileAligning=true`라면 실제 발사는 CurrentMuzzleDirection으로 가능할 수 있으므로 `TurretAligning`을 빨간 발사 불가 상태로 강제하지 않는다.
- `WeaponNotAligned`도 기존 계약대로 주 Reticle을 일반 `FireRejected` 빨강으로 자동 덮어쓰지 않는다.
- `MuzzleBlocked`는 실제 발사 경로 차단이므로 기존 `AimBlocked / Blocked` 피드백을 유지한다.
- `OutOfArc`는 현재 P0에서 표시/디버그 의미가 우선이며 단독 발사 차단으로 UI가 재해석하지 않는다.

### 9.3 Immediate Fire State

Reticle 주변 즉시 상태는 기존 `FCFVehicleFireFeedbackViewData`를 우선 소비한다.

```text
FireSuccess
Cooldown
FireRejected
NoWeapon
AimBlocked
OutOfArcWarning
TurretAligning DisplayKey
```

- 같은 순간 여러 설명을 늘어놓지 않고 현재 행동 판단에 필요한 **대표 상태 한 줄**을 우선 표시한다.
- `CooldownRatio`와 실제 남은 시간이 제공되면 Compact Progress/Text에 사용할 수 있지만 Widget이 Weapon Component를 직접 조회해 재계산하지 않는다.
- `FirePending`은 타입은 존재하지만 현재 동기 발사 구조에서 활성 경로가 없으므로 실제 Provider 사건 없이 표시하지 않는다.
- `Reloading`도 실제 Ammo/Reload Runtime이 연결되기 전에는 enum 존재만으로 표시하지 않는다.
- 내부 `RejectReason` 이름을 그대로 사용자 Text로 노출하지 않고 Presenter의 Player-facing 문구 또는 기존 DisplayKey 매핑을 사용한다.
- CarFight 전역 `CF-PDL-0009`에 따라 Reticle/FireFeedback/Lock 완료·실패에도 게임/UI 사운드를 사용하지 않는다.

---

## 9A. Mission Summary

Mission Summary는 좌상단 Compact HUD이며 **Mission Domain을 소유하지 않는다**. 현재 실제 C++에는 `MissionSummary` HUD Slot만 있고 Mission/Objective Runtime Provider는 아직 확인되지 않았다.

따라서 현재 Runtime 계약은 다음으로 제한한다.

```text
Mission Provider 없음
→ 실제 Gameplay MissionSummary Collapsed

D1 Visual Prototype
→ Mock Mission View Data 사용 가능
→ Gameplay 구현 증거로 취급하지 않음
```

후속 Mission Runtime이 생기면 Presenter가 최소 다음 의미 데이터를 공급한다.

```text
MissionDisplayName
PrimaryObjectiveIdentity
PrimaryObjectiveText
ProgressVariant
ProgressText
NormalizedProgress
RemainingGameTime
AdditionalObjectiveCount
ObjectiveState
```

위 이름은 후속 C++ 타입 확정 전의 의미 계약이며 CF-FQ-032가 Mission Runtime enum이나 클래스 이름을 선행 확정하지 않는다.

- Mission Provider가 현재 대표 Objective를 명시한다. `PrimaryObjectiveIdentity`는 MissionSummary와 Objective World Marker가 같은 Primary를 가리키는지 연결하기 위한 안정적인 Presentation Identity 의미이며 내부 Actor 이름/Object Path를 Player-facing Text로 노출하기 위한 값이 아니다. Widget이 거리·남은 시간·목표 종류·보상·완료율을 비교해 자체 Priority를 정하지 않는다.
- 여러 Objective가 있어도 기본 HUD는 대표 1개만 직접 표시하고 나머지는 `+N OBJECTIVES`처럼 요약할 수 있다. 자동 Carousel은 사용하지 않는다.
- `Count / Normalized / TimeRemaining / StateText`는 Presentation Variant이며 Widget이 Kill Count·Actor 수·거리·점령률을 World에서 직접 계산하지 않는다.
- Progress 정밀값이 Provider에 없으면 정확한 Percent나 분자/분모를 추정 생성하지 않는다.
- Mission Timer는 **Game Time 기준** Provider 값을 표시하며 World Pause 중 감소하지 않는다. Widget이 Real Time으로 독자 진행시키지 않는다.
- Objective Completed/Failed 판정과 다음 Objective 전환 시점은 Mission Provider가 소유한다.
- 정상 진행은 MissionSummary가, 실패 임박 같은 즉시 대응 사건은 Mission Provider가 별도 Alert를 실제 제공한 경우에만 Alert Feed가 표시한다.
- Objective World Marker·방향·화면 투영은 Game/Reticle Layer가 소유하고 MissionSummary가 Actor를 직접 검색하지 않는다.
- P0 MissionSummary는 비상호작용 HUD이며 Focus·Cursor·Input Mode를 변경하지 않는다.
- 실제 `480×128` Geometry, Footer와 Overflow 시각 규칙은 `InGameUIStyleSpec.md`가 소유한다.

---

## 10. 차량 HUD

상세 시각·표시 계약의 SSOT는 `InGameUIVehiclePanelSpec.md v0.20.0`이다. 여기서는 다른 HUD와 연결되는 구조만 요약한다.

### 10.1 패널 구조

```text
VehiclePanel 896 × 416 / Bottom Left

상단 좌측   SpeedGauge
상단 우측   ArmorBodyMap
중단         ShieldRow
최하단       IntegrityRow
```

- `SpeedGauge`는 **비대칭 실제 Engine RPM Gauge + 큰 3자리 디지털 속도 + 작은 km/h + 단일 Gear Slot** 구조다.
- RPM Gauge는 좌측 세로 구간에서 부드러운 곡선을 거쳐 상단 긴 수평 구간으로 이어진다.
- Gauge Tick은 21개이며 고정 시각 스케일 85%부터 Red Zone을 사용한다.
- RPM Gauge가 속도를 다시 그리는 가짜 Speed Arc가 되지 않게 하고 실제 Engine RPM Provider가 없으면 속도 비율 등으로 임의 구동하지 않는다.
- Gear Slot은 `R`, `N`, 실제 Provider가 제공하는 전진 기어 단수를 표시한다. 실제 Gear Data가 없으면 단수를 추정하지 않는다.

### 10.2 방어 데이터와 배치

```text
Shield
Front / Left / Right / Rear / Top / Bottom Armor
Vehicle Integrity
```

- 우측 `ArmorBodyMap` 중앙에는 화면 왼쪽을 바라보는 고정 차량 실루엣을 둔다.
- 화면 위치는 좌=Front, 상=Right, 우=Rear, 하=Left이며 Top은 좌상단 Badge, Bottom은 우하단 Badge다.
- 6방향 Armor는 방향 Text와 Current/Maximum 숫자를 기본 제거하고 각 Plate/Badge 우측 세로 Bar의 `NormalizedArmor`로 표시한다.
- Shield는 상단 Cluster와 Integrity 사이의 전체 폭 Bar, Vehicle Integrity는 최하단 전체 폭 Bar다.
- Shield/Integrity는 중앙 Current/Maximum Text를 유지하고 Fill 끝 Chevron으로 재생/수리 대기·진행 상태를 같은 시각 문법으로 표현할 수 있다.
- 미장착, Known Zero와 Unavailable을 같은 0% 파괴 상태로 합치지 않는다.

### 10.3 부품 상태

현재 실제 부품별 내구도·기능 저하 Runtime이 없으며 **고정 VehiclePanel에는 부품 손상 목록을 넣지 않는다**.

후속 Module Damage Runtime이 생겨도 VehiclePanel 내부에 자동으로 3-Row 손상 목록을 복원하지 않는다. 즉시 대응이 필요한 손상은 Alert Feed, 상세 상태는 후속 Vehicle Detail/Module UI가 소유하도록 별도 설계한다.

VehiclePanel Widget은 피해 분배·Armor 방향 판정·RPM·Gear·재생/수리 판정을 직접 계산하지 않고 Presenter가 제공한 상태를 표시한다.

---

## 11. 무기 HUD

무기마다 고정된 탄약 UI를 만들지 않는다.

```text
Weapon HUD
→ 0~N Resource Channel 표시
```

표시 예:

```text
기관포: Ammo + Reserve + Heat
레이저: WeaponCharge + Heat
미사일: Ammo + Lock
레일건: Ammo + VehicleBattery 또는 WeaponCharge + Heat
방어장비: VehicleBattery + Cooldown
```

에너지 무기라는 이름만으로 차량 배터리를 자동 소비한다고 가정하지 않는다.

---

## 12. Radar

Radar는 Sensor Provider가 만든 Contact 목록만 소비한다.

좌표 변환:

```text
Contact World Location
→ Player Vehicle 기준 상대 벡터
→ Heading Up 회전
→ Radar Range로 정규화
→ 원형 경계 Clamp
```

Radar Widget은 다음을 판정하지 않는다.

- 대상이 센서에 잡히는가
- 적대 관계인가
- 마지막 접촉이 언제 사라지는가
- 가시 식별이 완료되었는가

---

## 13. Target Panel

Target Panel은 선택 대상의 **Tracking / Identification / Target Intelligence**를 서로 다른 정보축으로 받아 표현한다. 구체적인 ID0/ID1/ID2 공개 규칙과 Visual Layout은 `InGameUIStyleSpec.md`가 SSOT로 소유한다.

Target이 없으면 다음과 같은 Compact 상태를 사용한다.

```text
NO TARGET
```

Target이 있으면 Identification 수준과 무관하게 현재 선택 상태를 유지하고, 확보된 정보만 공개한다.

```text
UNKNOWN CONTACT            # ID0 예시
NAME       ???
TYPE       ???
FACTION    ???
DISTANCE   842 m            # Tracking 정보는 ID와 독립
SPEED      67 km/h          # Sensor가 제공할 때
```

관계가 판명되면 정체를 몰라도 ID1 정보를 표시할 수 있다.

```text
HOSTILE CONTACT            # ID1 예시
NAME       ???
TYPE       ???
FACTION    ???
DISTANCE   842 m
SPEED      67 km/h
```

Identity가 확보되면 ID2에서 실제 알려진 필드를 공개한다.

```text
MARAUDER-02                # ID2 예시
TYPE       Combat SUV
FACTION    Black Dogs
DISTANCE   842 m
SPEED      67 km/h
```

`SHIELD / ARMOR / VEHICLE` 정확 수치, 장착 무기, 부품 손상과 약점은 ID2만으로 자동 공개하지 않는다. 이들은 별도의 Target Intelligence/Analysis가 실제 정보를 제공할 때만 활성화한다.

Target Intelligence는 전역 Scan Level 하나가 아니라 **필드별 Knowledge State**를 사용한다.

```text
Unknown    = 아직 모름
Estimated  = 관찰·센서로 추정
Known      = 신뢰 가능한 Source로 확인
Unavailable= 현재 Provider/Capability로 제공 불가
```

Knowledge와 별도로 동적 정보에는 `Current / Stale` Freshness를 적용한다. 한번 확인한 Weapon Type 같은 정적 Intel은 유지할 수 있지만 Shield/Armor/Integrity 현재값은 Live 분석이 끊긴 뒤 계속 갱신하지 않는다.

기본 Intelligence Source:

```text
Visual / Combat Observation
Mission / Database / Prior Intel
Scanner Tactical Analysis
후속 Friendly Telemetry
```

Friendly IFF는 탐지 즉시 ID2를 제공하지만 Shield/Armor/Weapon/Damage 전체 Telemetry를 자동 공개하지 않는다.

Scanner Tactical Analysis는 선택 Target이 ID2가 된 뒤 별도 두 번째 입력 없이 자동으로 이어질 수 있으며, 기본 조건은 `Selected Target + ID2 + Live Contact + Tactical Analysis Range + Scanner LOS/Occlusion + Category Capability`다.

분석 진행 중 Target이 Tactical Analysis Range를 벗어나거나 Scanner Occluder 뒤로 숨는 등 유효 조건을 잃으면 **미완료 진행률은 즉시 초기화하지 않고 서서히 감소**한다. 조건을 다시 만족하면 남아 있는 진행률에서 분석을 재개하며, 충분히 오래 놓쳐 감쇠가 0에 도달한 경우에만 해당 분석을 처음부터 다시 시작한다. 감쇠 속도는 Scanner 또는 Tactical Analysis Profile의 조정값으로 두며 이미 Known으로 확정된 Intelligence에는 적용하지 않는다.

Tactical Analysis 성능은 `진행 증가 속도(Build)`와 `조건 손실 시 감쇠 속도(Decay)`를 독립 조정축으로 취급한다. 기본 튜닝에서는 Decay가 Build보다 느리게 해 순간적인 차폐가 과도한 손실이 되지 않게 한다. 고성능 Scanner는 더 빠른 Build, 더 느린 Decay, 더 긴 Analysis Range와 더 많은 Category Capability를 독립적으로 조합할 수 있다.

Target Panel에서는 여러 Category Progress를 동시에 늘어놓지 않고 현재 우선 분석 중인 미완료 Category 하나의 Compact Progress를 표시한다. 정상 분석 중에는 값이 증가하고, 분석 조건을 잃으면 같은 Bar가 역방향으로 감소하면서 `INTERRUPTED` 계열 짧은 상태 Text와 낮은 명도로 구분한다. 감쇠 상태에는 반복 Blink/Pulse/경고 Flash를 사용하지 않으며, 조건 재확보 시 남은 Progress에서 자연스럽게 정상 분석으로 복귀한다.

기본 Tactical Analysis Queue는 한 번에 하나의 Category를 처리하며 기본 Priority는 `Shield → Weapon → Armor → Vehicle → Damage/Module`이다. Priority는 Scanner/Tactical Analysis Profile에서 Override할 수 있고 Capability가 없거나 `NotApplicable / Unavailable`, 이미 충분히 Known인 Category는 자동으로 Skip한다. `Estimated`는 Scanner로 Known 승격 가치가 있으므로 분석 대상에 남긴다.

현재 Category의 미완료 Progress가 남아 있는 상태에서 조건을 잃으면 해당 Category가 재개 우선권을 유지한다. 다시 조건을 만족하면 다른 Category로 전환하지 않고 기존 Progress부터 재개하며, 0까지 감쇠한 경우에만 Knowledge State와 Priority를 다시 평가한다. Target을 바꾸면 이전 Target의 미완료 Progress는 같은 Decay 규칙을 따르고 새 Target은 독립 Queue를 시작한다.

이미 Known인 동적 Intel의 `Current / Stale` 갱신은 초기 Category 획득 Queue와 분리된 Refresh/Freshness 경로가 담당한다. 기본 P0 HUD에는 현재 분석 Category 하나만 표시하고 플레이어가 Queue를 직접 편집하거나 Category를 강제로 선택하는 입력은 요구하지 않는다.

Category 완료 피드백은 개별 Field가 아니라 **Category당 1회**로 처리한다. Progress가 100%에 도달하면 Target Panel 내부에서 새로 공개되거나 `Estimated → Known`으로 승격된 정보만 짧은 Brightness/Opacity Highlight로 강조하고, Panel 전체·Radar Blip·Target Bracket·Alert Feed에는 완료 효과를 중복 전파하지 않는다. Scale Pop·Bounce·반복 Glow/Pulse는 사용하지 않는다.

CarFight 프로젝트 전역 결정 `CF-PDL-0009`에 따라 Tactical Analysis 완료에 게임/UI 사운드를 사용하지 않는다. 완료 피드백은 Target Panel 내부의 짧은 1회 Brightness/Opacity Highlight로 시각적으로 완결한다. Capability 부족이나 이미 Known이라 Skip된 Category에는 완료 Highlight를 발생시키지 않으며 마지막 Category에도 별도 대형 Banner나 추가 완료 연출을 사용하지 않는다.

완료 연출은 Scanner Gameplay 진행을 지연시키지 않는다. 다음 유효 Category는 즉시 분석을 시작할 수 있고 HUD Progress Label만 자연스럽게 다음 Category로 전환한다.

Target의 Defense Intelligence는 플레이어 VehiclePanel을 축소 복제하지 않고 **Compact Intelligence Card**로 표시한다. Shield와 Vehicle Integrity는 각각 `Label + 얇은 Horizontal Bar + 상태값` 한 줄 구조를 사용하고, Armor는 차량 실루엣 없이 다음 고정 2열×3행 Grid를 사용한다.

```text
FRONT  | REAR
LEFT   | RIGHT
TOP    | BOTTOM
```

Armor Block이 한번 활성화되면 여섯 방향 Cell 위치는 고정하고 아직 모르는 방향은 `???`로 남긴다. 방향별 Known Ratio가 있을 때만 Percent/Micro Bar를 사용하며, Estimated 정보에서는 실제 Source가 제공한 질적 상태나 추정 범위만 표시하고 UI가 정확한 Percent를 임의 생성하지 않는다.

Defense Block의 표시 순서는 `Shield → Armor → Vehicle Integrity`로 고정하며 이는 Scanner 분석 Priority와는 별개의 Presentation 순서다. Defense Intelligence가 하나도 없으면 Block 전체를 Collapse하고, 일부 정보만 있으면 해당 Block만 아래쪽으로 추가한다. Target Panel의 Top Right Anchor/Header는 유지하고 아래쪽으로만 확장한다.

Dynamic Defense Intel이 Stale이 되면 마지막 값을 지우지 않고 낮은 시각 강도와 `STALE` 의미를 함께 제공한다. VehiclePanel의 Current/Maximum 숫자, 차량 실루엣, Armor Plate 이미지는 Target Panel에 복제하지 않는다.

Target Weapon Intelligence는 플레이어 WeaponPanel을 축소 복제하지 않고 **Weapon Family 기준 Compact Summary**로 표시한다. 동일 Family의 개별 Weapon Instance는 하나의 Row로 Grouping하며 기본 형식은 `Family/확인된 Display Type + Count + 선택적 Mount Summary`다.

```text
CANNON ×2   · FRONT / ROOF
MISSILE ×4  · REAR
LASER ×1
```

정확한 Count가 Known일 때만 `×N`을 사용한다. Estimated 수량은 `MULTIPLE` 또는 실제 Source가 제공한 추정 범위를 사용하고 정확한 개수를 UI에서 추정 생성하지 않는다. Mount도 Intelligence Source가 제공한 Player-facing Label만 표시하며 내부 Mount Profile ID나 Transform을 Widget이 해석하지 않는다.

기본 Target Panel은 Weapon Family Summary를 최대 3 Row까지 직접 표시하고 4개 이상이면 `앞의 2개 + +N TYPES`로 축약한다. Row 순서는 Provider의 안정적인 Display Order를 사용하고 UI에서 자체 위협도 계산으로 재정렬하지 않는다. Known Weapon Family/Count/Mount는 현재 P0에서 정적 Knowledge로 유지하며 Last Known 전환만으로 Stale 처리하지 않는다.

Target Weapon Block에는 Ammo·Magazine·Heat·Charge·Cooldown·Reload·Launcher Sequence 같은 적의 내부 운용 상태를 표시하지 않는다. Family Semantic Icon은 선택 요소이며 Text만으로 정보가 완전해야 한다.

Target Damage/Module Intelligence는 **확인된 손상·기능 이상만 보여주는 Compact Damage Summary**로 사용한다. 현재 CF-FQ-033 P0에는 실제 Module Damage Provider가 없으므로 Damage Block 전체를 Collapse하고, Scanner에 Capability 설정이 있더라도 Provider 부재 Category는 `Unavailable`로 Skip한다. 가짜 분석 Progress·완료음·가짜 Module 상태를 생성하지 않는다.

후속 Engine/Wheel/Turret Runtime이 구현되면 기본 Row는 `Player-facing Module Name + Severity + 선택적 Functional State`를 사용한다.

```text
ENGINE · CRITICAL · DEGRADED
TURRET · DISABLED · NO TRAVERSE
FRONT LEFT WHEEL · DAMAGED
```

Module 이름·위치·기능 상태는 Provider가 Player-facing 값으로 제공해야 하며 Widget이 내부 Component 이름, Bone, GameplayTag, Transform을 해석해 사용자 문구를 만들지 않는다. 휠 개수도 4개로 하드코딩하지 않는다.

Severity와 Functional State는 별도 의미다. 향후 CF-FQ-033의 4단계 상태는 `NOMINAL / DAMAGED / CRITICAL / DISABLED` 같은 표시 후보로 매핑할 수 있지만 실제 Runtime enum·임계값은 CF-FQ-032가 선행 확정하지 않는다. 정확한 Module HP·Damage Formula·임계값은 Compact Target Panel에서 기본 공개하지 않는다.

Damage Summary는 손상 Module을 최대 3 Row까지 직접 표시하고 4개 이상이면 Provider의 안정적인 Display Order 기준 앞의 2개 + `+N DAMAGED` Overflow로 축약한다. 모든 Module Coverage가 실제로 Known이고 손상이 하나도 없다고 확정된 경우에만 단일 `MODULES · NOMINAL` 상태를 표시할 수 있다. 일부 Module만 확인된 상태에서는 전체 정상 판정을 만들지 않는다.

Module Damage는 Repair·추가 피격으로 바뀔 수 있는 동적 Intelligence다. Live 분석이 끊기면 마지막 상태를 `Stale`로 유지하고 임의로 악화/회복시키지 않는다. 후속 Repair Runtime에서 정상 복구가 확인되면 해당 Damage Row를 제거하고, 모든 손상이 해소되며 전체 Coverage가 Known이면 `MODULES · NOMINAL`로 전환할 수 있다.

Engine은 실제 출력 제한/시동 불가 Provider가 있을 때만 `DEGRADED` 같은 기능 상태를 표시하고, 느리게 주행한다는 이유만으로 엔진 손상을 추정하지 않는다. Wheel은 손상된 개별 위치를 유지하되 Provider가 Grouping을 허용하면 동일 상태 복수 Wheel을 `WHEELS ×2 · DAMAGED`처럼 묶을 수 있다. Turret Damage는 기존 Weapon Family/장착 정보와 분리해 회전·조준·발사 기능 저하만 Damage Summary에서 표현한다.

전체 Module 목록, HP, Damage Log, Weak Point, Repair ETA는 기본 Target Panel에 넣지 않고 후속 Expanded Target Detail 영역으로 남긴다.

Target Panel 전체 Density는 우상단 **기본 폭 384를 유지하고 세로로만 확장**한다. 2560×1440 기준 초기/부분 정보 크기 `384×336`은 유지하되 기본 HUD 최대 높이는 `640`으로 제한한다. 이 최대값은 Widget Literal이 아니라 Layout/Density Token으로 관리하고 다른 해상도에서는 Safe Region과 Layout Scale 규칙으로 검증한다.

전체 Block 순서는 다음으로 고정한다.

```text
Header / Relation / Identity
Tracking / Kinematic
현재 Analysis Progress
Defense — Shield / Armor / Vehicle
Weapon Summary
Damage / Module Summary
```

Header/Relationship, 핵심 Tracking 정보와 현재 진행 중인 Analysis Progress는 Pinned 정보로 유지한다. Tactical Intelligence가 많아져도 이 영역을 제거하지 않는다. 기본 전투 HUD에는 Target Panel ScrollBox, 자동 Carousel, 시간 기반 Page Rotation을 사용하지 않는다.

높이가 부족하면 Knowledge를 삭제하지 않고 Presentation만 단계적으로 압축한다. 먼저 Weapon Mount 같은 Secondary Text를 줄이고, Weapon은 `2 Row + +N TYPES`, Damage는 `2 Row + +N DAMAGED` Overflow를 사용한다. Armor 6방향은 2×3 의미를 유지한 채 Cell Padding/Micro Bar를 줄이며 평균 Armor Bar로 합치지 않는다. Shield와 Vehicle도 서로 독립 Bar를 유지한다.

절대 정보 보존 우선순위는 `Target Identity/Relation → Tracking → Current Analysis → Defense → Weapon → Damage → Secondary Detail`이다. 이 우선순위는 Block 자체를 재정렬하는 규칙이 아니라 **무엇부터 축약할지**의 기준이다. Critical Damage는 Severity Style로 강조할 수 있지만 Damage Block이 Header 위로 이동하지 않는다.

긴 Target/Type/Faction/Weapon/Module 이름은 Provider의 Short Display Name을 우선하고 필요하면 Ellipsis를 사용한다. Font를 가독성 하한 이하로 줄이거나 내부 Asset Name으로 대체하지 않는다. 모든 Compact 규칙 후에도 상세 정보가 넘치면 `+N` 요약은 남기고 나머지는 후속 Expanded Target Detail이 소유한다.

Target Tracking/Selection 상태는 `Live Contact → Last Known Contact → Contact Lost`의 흐름을 사용한다. 선택 중인 Live Contact가 Last Known으로 바뀌어도 **선택을 즉시 해제하지 않고 Last Known 유지기간 동안 같은 Target을 유지**한다.

Last Known에서는 기존 Identity/Relationship과 정적 Intel을 보존하고 Target Panel에 `LAST KNOWN` 상태를 추가한다. 실시간 `DISTANCE` 대신 저장된 마지막 탐지 위치까지의 `LAST POS` 의미를 사용하며, 마지막 Speed를 남길 경우 `LAST SPEED/Stale`로만 표시하거나 Compact에서는 숨길 수 있다. 마지막 위치와 속도로 Target의 현재 위치를 외삽하지 않는다.

```text
MARAUDER-02
LAST KNOWN
LAST POS   842 m
LAST SPEED 67 km/h · STALE
```

Shield/Armor/Vehicle/Module 같은 동적 Intel은 마지막 샘플에서 멈추고 Stale로 전환한다. Last Known 중에는 새로운 Identification/Tactical Analysis를 시작하지 않으며, 이미 진행 중이던 미완료 Analysis만 기존 `INTERRUPTED + Decay` 규칙으로 감소한다. 0에 도달하면 Progress를 숨기고 Live 재획득 전까지 새 Category를 시작하지 않는다.

Radar에서는 Last Known Ghosted Blip/Stalk와 기존 Selected Target Bracket을 함께 유지한다. Last Known 기간 안 같은 Contact를 다시 Live로 획득하면 별도 재선택 없이 Ghosting과 `LAST KNOWN` 상태만 제거하고 정상 Tracking으로 복귀한다. 동적 Intel은 실제 새 샘플이 들어온 항목만 Current로 복귀하며 남아 있던 Analysis Progress는 그대로 재개한다.

UI의 Selected Target 유지와 Weapon/Lock의 Live Target 유효성은 분리한다. Last Known을 계속 선택해 보여줘도 Weapon Gameplay가 현재 Actor 위치, Aim Point 또는 Lock 성공을 자동으로 얻지 않으며 각 Fire-Control 시스템의 Live/LOS/Sensor 조건을 따로 만족해야 한다.

`Contact Lost`는 UI 자체 Timer가 아니라 Sensor/Contact Tracking Provider가 Last Known 만료 또는 Contact Record 무효화를 판정한다. 현재 선택 Target이 Contact Lost가 되면 그 순간 자동 선택 해제하고 Radar Bracket/Ghost Contact도 종료한다. Target Panel은 짧은 `TARGET LOST` 상태 전환 뒤 `NO TARGET`으로 돌아간다.

Contact Lost 시 다음 Contact를 자동 선택하지 않는다. 이후 같은 Entity가 다시 탐지돼 이전 Identity/정적 Knowledge를 재사용할 수 있더라도 Target은 자동 재선택하지 않고 플레이어의 정상 Target Selection을 요구한다. 수동 Target 해제/변경 역시 Contact Knowledge 자체를 삭제하지 않는다.

P0 기본 Target Cycle은 신규 선택 후보로 Live Contact를 우선하고 Ghosted Last Known Contact를 자동 Cycle 후보로 포함하지 않는다. 이미 선택된 Target이 Last Known으로 넘어가면서 선택을 유지하는 것과 새로운 Ghost Contact를 선택하는 기능은 분리한다. 후자가 필요해지면 별도 Inspect/Command UX로 확장한다.

`Destroyed`와 `Contact Lost`는 같은 상태로 취급하지 않는다. 파괴가 Known이라는 것은 전투 결과이고 Contact Lost는 Sensor 신뢰 종료이므로, Sensor Lost만으로 Target이 Destroyed라고 추정하지 않는다.

Destroyed Target Lifecycle은 Gameplay/Vehicle State/신뢰 가능한 Intelligence Provider가 `Destroyed`를 명시적으로 Known으로 제공할 때 시작한다. 현재 선택 Target이 파괴되면 즉시 선택을 지우지 않고 기본 **1.25초 `DestroyedTargetHoldDuration`** 동안 Kill Confirmation Hold를 유지한다. 이 값은 Style/Gameplay Token에서 조정하고 Widget Timer Literal로 고정하지 않는다.

Hold 진입 즉시 해당 Target의 Identification/Tactical Analysis를 종료한다. 미완료 Progress를 Decay시키거나 다음 Category로 넘기지 않고, Analysis Complete/Unlock/완료음도 발생시키지 않는다. 이미 확보한 Knowledge Memory는 유지할 수 있다.

Target Panel은 Hold 동안 기존 Name/Identity/Relationship과 `DESTROYED`를 명확하게 보여주고, 기본 Preset에서는 Analysis Progress를 제거한 뒤 `Identity + DESTROYED + 유효한 거리 정보` 중심 Compact Terminal State를 사용한다. Wreck 위치가 계속 Live Tracking되면 `DISTANCE`는 실제 Wreck 거리로 유지할 수 있지만 Speed를 임의 0으로 만들지 않는다. Shield/Armor/Module 마지막 값도 파괴됐다는 이유만으로 일괄 0/Disabled로 덮어쓰지 않는다.

Radar는 Hold 동안 Contact Record가 남아 있으면 기존 Blip/Stalk와 Selected Target Bracket을 낮은 시각 강도로 잠깐 유지한다. 파괴 확인만으로 기본 `◆ / ▼ / ●` Symbol을 X/해골/폭발 아이콘으로 바꾸지 않는다. Hold 종료와 함께 Selected Target Bracket은 제거하며 이후 Wreck를 Radar에 계속 표시할지는 별도 World/Wreck Contact 정책이 소유한다.

1.25초 Hold가 끝나면 Destroyed Target을 자동 선택 해제하고 **`TARGET LOST` 없이 `DESTROYED → NO TARGET`**으로 전환한다. 다음 Live Contact를 자동 선택하지 않는다. 다만 Hold 중 플레이어가 Target Cycle/직접 선택으로 다른 Live Target을 고르면 즉시 새 Target으로 전환하고 Destroyed Hold는 종료한다.

UI의 1.25초 선택 유지가 Weapon Lock이나 Fire-Control 유효시간을 연장하지 않는다. 파괴 확인 시 실제 Target Validity는 Weapon/Fire-Control Provider가 별도로 판정하며, 이미 발사된 Projectile/Missile이나 Launcher Sequence의 취소·유지는 각 Weapon Gameplay System이 소유한다.

자동 선택 해제 뒤에도 Identity/Faction/정적 Loadout/Destroyed Known 결과는 별도 Knowledge Memory로 보존할 수 있다. 후속 Repair·Respawn Gameplay가 생기더라도 Destroyed Target을 UI가 임의로 Live Target으로 복구하지 않고 별도 Entity/Lifecycle 계약을 따른다.

정보 공개 원칙:

```text
Shield
- 관찰: Shield 존재·활성 사실 또는 상태 추정
- Database: 명목 정적 사양
- Scanner: 현재 Shield Ratio/Broken 상태

Armor
- 관찰: 실제 보이는 방향의 손상 추정
- Database: 명목 6방향 구조
- Scanner: Front/Left/Right/Rear/Top/Bottom 현재 상태를 방향별로 확인

Vehicle
- 관찰: 연기·화재·파손 등으로 상태 추정
- Scanner: 현재 Vehicle Integrity Ratio 확인
- Destroyed가 명백하면 Scanner 없이 Known 가능

Weapon
- 관찰: 보이는 Mount·발사 Signature로 존재/Family 추정
- Scanner: 실제 장착 Weapon Type/Family·개수·확인 가능한 Mount 정보
- Ammo/Heat/Charge/Cooldown/Reload 내부 수치는 기본 공개 안 함

Damage / Module
- Vehicle Integrity와 별도 의미
- 현재 독립 Module Damage Runtime이 없으면 가짜 정보 생성 금지
- 후속 Module Runtime과 Scanner Capability가 생길 때 실제 손상 부품·Severity·Functional State로 확장
```

적 Target의 Shield/Armor/Vehicle 상태는 기본적으로 내부 Current/Maximum 원시 숫자보다 Normalized Ratio/Percent와 상태 표현을 우선한다. 절대 숫자는 실제 Gameplay/Intel 계약이 제공할 때만 표시한다.

동일 의미 행의 `Unknown ↔ Known ↔ Unavailable` 변화는 기존 Slot 위치를 유지하고 값만 교체한다. 반면 아직 활성화되지 않은 상세 Intelligence Block은 Collapse할 수 있다. Panel의 Top Right Anchor와 Header는 고정하고 추가 정보는 아래로 확장해 정보 공개 때 Panel 전체가 이동하지 않도록 한다.

---

## 14. Blueprint 노출 규칙

Blueprint에서 조정하는 변수와 함수에는 한국어 Tooltip을 제공한다.

예:

```text
DisplayRefreshRate
Tooltip: 속도와 화면 위치처럼 자주 변하는 HUD 데이터를 초당 몇 번 갱신할지 설정합니다.

UnknownDisplayText
Tooltip: 선택한 대상의 정보가 아직 공개되지 않았을 때 표시할 텍스트입니다.

RadarDisplayRange
Tooltip: 레이더 원의 가장자리가 나타내는 최대 월드 거리입니다.
```

C++ 내부 구현 세부 정보는 불필요하게 Blueprint에 노출하지 않는다.

---

## 15. 오류 방지

금지 패턴:

```text
Widget Tick
→ Get Player Pawn
→ Cast To BP_CFVehiclePawn
→ 여러 Component 조회
```

권장 패턴:

```text
UISubsystem / Presenter
→ 현재 Pawn 데이터 소스 연결
→ View Data 갱신
→ Widget 이벤트 전달
```

`TargetActor`가 파괴되거나 Pawn이 교체되는 경우 약한 참조와 유효성 검사를 사용한다.

---

## 16. 파일과 클래스 버전 관리

새 C++ 파일은 파일 상단 또는 클래스 문서에 버전과 책임을 기록한다.

예:

```text
UCFUISubsystem v1.0.0
FCFInGameUIViewData v1.0.0
```

변경 시:

- Version
- Changelog
- Migration
- Blueprint 영향
- 사용자 PIE 회귀

를 관련 Plan 또는 Systems 문서에 함께 기록한다.

---

## 17. 확정 구현 세부 계약

### 17.1 프레임워크와 의존성

```text
UMG
+ Enhanced Input
+ ACFPlayerController
+ UCFUISubsystem
+ World별 WBP_CFUIRoot
```

CommonUI는 현재 P0에 도입하지 않는다. 향후 타이틀, 차고, 피팅, 임무와 결과 화면이 실제 구현 범위가 될 때 별도 Gate로 재검토한다.

#### 17.1A UI 디자인 품질 계약

시각 콘셉트와 아트 디렉션은 `InGameUIVisualConcept.md`가 소유한다. Color·Typography·Spacing·Shape·Opacity·Motion Token, Base Widget 5종과 전체 2560×1440 좌표 규격은 `InGameUIStyleSpec.md`와 `CFHUDWireframe_1440p.xml`이 소유한다. VehiclePanel 내부 계약은 `InGameUIVehiclePanelSpec.md`와 `CFVehiclePanel_1440p.xml`이 소유한다. VehiclePanel은 Draft / User Review Pending이며 WeaponPanel·Unreal Asset·Font·Icon과 실제 화면 검토가 남아 있으므로 UI-DESIGN-GATE 전체 PASS는 아니다.

UI 디자인 품질은 프레임워크 선택과 독립된 책임이다. CommonUI를 사용하지 않아도 완성도 높은 HUD를 만들 수 있으며, CommonUI를 사용해도 별도 디자인 시스템이 없으면 투박한 UI가 그대로 남는다.

현재 P0 시각 목표는 완성형 아트가 아니라 다음 조건을 갖춘 `Clean Prototype`이다.

```text
- 정보 우선순위가 즉시 읽힌다.
- 버튼은 버튼으로 인식되고 클릭·Focus 상태가 보인다.
- 같은 종류의 패널·게이지·텍스트가 동일한 규칙을 사용한다.
- 색상만으로 상태를 구분하지 않는다.
- 중심 조준 영역을 불필요하게 가리지 않는다.
- 16:9, 21:9와 32:9에서 Safe Zone과 시선 흐름을 유지한다.
```

디자인 시스템의 단일 기준 후보:

```text
DA_CFUIStyle
- Color Tokens
- Typography Tokens
- Spacing Tokens
- Panel / Button / Status Bar Style
- Focus / Hover / Pressed / Disabled 상태
- Alert Motion과 기본 전환 시간
```

공통 Blueprint Widget 후보:

```text
WBP_CFButtonBase
WBP_CFPanelBase
WBP_CFStatusBar
WBP_CFInfoRow
WBP_CFAlertItem
```

책임 분리:

```text
C++ View Data / Presenter
→ 값, 상태 의미, 공개 범위, 우선순위

Blueprint / UMG / Style Data
→ 배치, 시각 계층, 색상, 폰트, 아이콘, 애니메이션
```

에셋 없는 C++ WidgetTree는 입력·Pause·수명 계약의 검증 수단이며 최종 시각 자산으로 승격하지 않는다. UI-P0-03~05의 데이터·수명 작업은 디자인 시스템 없이 진행할 수 있지만, UI-P0-06의 신규 차량·무기 HUD 시각 구현 전에는 `UI-DESIGN-GATE`를 통과해야 한다.

#### 17.1B CommonUI 도입 Gate

CommonUI가 해결하려는 범위:

```text
- Activatable Screen 수명
- 화면 Stack과 Back 처리
- 키보드·마우스·게임패드 내비게이션
- 입력 장치 전환과 입력 Glyph 확장
- 전체 화면 메뉴의 일관된 Focus 정책
```

CommonUI가 해결하지 않는 범위:

```text
- HUD 정보 구조
- 레티클·Radar·방어 게이지 디자인
- 색상·폰트·여백·아이콘 품질
- 데이터 계약과 Gameplay 계산
```

현재 상태:

```text
CommonUI Plugin: 미활성
CommonUI / CommonInput Module: 미의존
CommonActivatableWidget / CommonButtonBase: 미사용
현재 P0 Runtime: UMG + Enhanced Input + UCFUISubsystem
```

`UI-COMMONUI-GATE`는 타이틀, 차고, 피팅, 임무 선택과 결과 화면 중 첫 전체 화면 Flow를 실제 구현하기 전에 수행한다.

판단 기준:

- 중첩되는 전체 화면과 Modal Stack이 필요한가
- 게임패드 전용 내비게이션·Back·Focus 복원이 복잡해지는가
- 입력 장치별 Glyph와 장치 전환 표시가 필요한가
- 여러 화면이 공통 Activatable 수명 계약을 필요로 하는가
- 현재 `UCFUISubsystem` 방식으로 유지할 때 중복 코드가 실질적으로 증가하는가

허용 결론:

```text
A. 현재 UMG + UCFUISubsystem 유지
B. 상시 전투 HUD는 UMG 유지 + 전체 화면 Menu만 CommonUI 사용
C. 충분한 근거가 있을 때 전체 화면 UI 기반을 CommonUI로 확장
```

기본 권장은 B가 필요한 시점에만 도입하는 혼합 구조다. 상시 전투 HUD를 `CommonActivatableWidget`으로 일괄 변환하지 않는다.

Migration 불변 조건:

- `UCFUISubsystem`의 외부 화면 요청 진입점을 유지한다.
- View Data와 Presenter/Provider를 CommonUI 타입에 종속시키지 않는다.
- Gameplay 계산을 Widget이나 CommonUI Action으로 이동하지 않는다.
- CommonUI 도입과 기존 HUD 시각 개선을 같은 완료 조건으로 묶지 않는다.

### 17.2 입력 Mapping Context

```text
IMC_CFSystem
- Pause
- 공통 Back

IMC_CFGameplay
- 주행
- 조준
- 발사
- 타겟 선택

IMC_CFUI
- 메뉴 이동
- 확인
- 취소
- 탭 이동
```

- `IA_Pause`는 Pawn이 없어도 처리 가능해야 한다.
- Pause 해제 입력은 Pause 중에도 실행 가능해야 한다.
- 같은 Mapping Context를 중복 등록하지 않는다.
- Pawn 교체와 EndPlay에서 Gameplay Context 소유권을 정리한다.
- 메뉴가 열린 동안 Gameplay Action이 실행되지 않아야 한다.

### 17.3 Pause 입력 잔류

Pause 진입 전에 다음 입력을 명시적으로 중립화한다.

```text
Throttle = 0
Steering = 0
Brake = 정책에 따른 안전값
Handbrake = 입력 해제 또는 명시적 유지 정책
Look Input = 0
Fire Hold = 해제
Input Ownership = 해제
```

Launcher의 Ripple·Salvo 상태는 Pause 때문에 취소하지 않는다. World Tick이 멈춘 동안 상태를 보존하고 Resume 후 남은 간격부터 계속한다.

### 17.4 기존 HUD 소유권 이전

```text
LegacyPawnOwned
UISubsystemOwned
```

AimReticle을 먼저 이전하고 회귀 검증한 뒤 TargetSelect HUD를 이전한다. Possess만 해제되고 Pawn이 월드에 남는 경우에도 이전 Widget과 데이터 구독이 분리되어야 한다.

현재 TargetSelect HUD의 공개 이름 fallback으로 `Actor::GetName()`을 사용하는 동작은 플레이어 HUD에서 제거한다. 미공개 이름은 `???`이며 Actor 이름은 Debug Layer에서만 허용한다.

### 17.5 값 상태와 정보 공개

```text
Unknown
Unavailable
Known
```

`Known` 상태에서 현재 값이 0이면 실제 0으로 표시한다. `Unknown`, `Unavailable`과 실제 0을 빈 문자열이나 음수 값으로 암시하지 않는다.

```text
Authoritative Target Data
→ 선택 가능 여부, 관계 필터와 장비 호환성 등 게임플레이 판정

Target Knowledge View
→ 이름, 진영과 방어 상태 등 플레이어에게 공개 가능한 정보
```

실제 내부 관계가 Hostile이더라도 Affiliation Knowledge가 Unknown이면 UI에는 `???`를 표시한다.

현재 `UCFVehicleHealthComp`는 Vehicle Integrity로만 표시한다. 실제 Runtime이 없는 Shield와 Armor는 `Unavailable`로 표시한다.

### 17.6 갱신 주기

이벤트 기반:

- 선택 타겟, 식별 정보, 무기, 자원, 방어 상태, 부품 상태와 경고 변경
- 실제 Damage Resolved, Shield/Armor Break, Integrity Damage와 Destroyed 전환
- 로컬 공격자의 실제 Hit Confirmation 사건

프레임 기반:

- Reticle, Target Marker와 화면 밖 방향 투영

제한 주기 기반 초기값:

```text
SpeedKmh: 20~30 Hz
Target Distance: 10~20 Hz
Radar Contact: 10~20 Hz
```

같은 텍스트, 색상과 아이콘은 값이 변경될 때만 다시 적용한다.

### 17.7 Reticle / World Marker / 화면 밖 방향 투영

| 요소 | 기준 |
|---|---|
| Command Reticle | Camera Aim / 플레이어가 지정한 조준 목표 |
| Turret Reticle | `CurrentMuzzleDirection` 기반 실제 터렛·총구 방향 |
| Selected World Target Marker | 선택 Target의 Provider가 허용한 World Anchor / Screen Bounds |
| Lock Indicator | 후속 Fire-Control Provider의 실제 Lock 상태 |
| Radar 상단 | 차량 진행 방향 |
| Off-screen Marker | 카메라 View 기준 Target 방향 |
| 포탑 방향 표시 | 차량 차체 기준 상대 각도 |

#### 17.7.1 World Target Marker 권한

World Target Marker는 Target을 새로 찾거나 추적 상태를 판정하지 않는다.

```text
Target/Contact Provider
→ Tracking State + 표시 가능한 World Anchor + 선택 상태
→ Projection Presenter
→ On-screen / Edge Marker View Data
→ Widget
```

- 현재 선택 Target의 마커가 기본 우선 대상이다. 기존 TargetSelect Candidate 표시를 이 설계가 임의로 다수의 World Marker로 증식시키지 않는다.
- Marker는 `Selected Target`을 뜻하며 Lock 성공을 뜻하지 않는다.
- Identity·Faction·Defense 전체 정보를 중앙에 다시 복제하지 않는다. 기본 World Marker는 선택 Bracket, 관계/Tracking의 최소 상태와 실제 Provider가 준 선택적 거리 정도만 사용한다.
- Target Panel의 Name/Intel과 World Marker의 전투 위치 인지를 분리한다.
- World Marker는 Target Actor의 내부 Component/Bone을 임의 검색해 Anchor를 만들지 않는다. Provider가 Player-facing Aim/Marker Anchor를 제공하거나 안전한 Actor 기준점을 명시한다.

#### 17.7.2 화면 투영 상태

화면 밖 마커는 단순 화면 좌표 Clamp가 아니라 다음 절차를 사용한다.

```text
World Anchor / World Direction
→ Camera View Space 변환
→ Z 기준 전방/후방 판정
→ 2D 방향 계산
→ Safe Region 경계와 Ray 교차
→ Edge Marker 위치·Orientation 계산
```

표현 상태 의미:

```text
OnScreen
→ World Anchor가 카메라 앞이며 Safe Region 안
→ Target Bounds/Anchor를 따라 정상 Selected Marker 표시

OffScreen
→ 카메라 앞이지만 Safe Region 밖
→ 경계 교차점에 Edge Marker 표시

BehindCamera
→ 카메라 뒤
→ 화면 좌표를 억지 Clamp하지 않고 View 방향을 2D Edge 방향으로 변환

Occluded
→ World Visibility와 Sensor Tracking을 같은 값으로 간주하지 않음
→ Sensor/Tracking Provider가 Last Known으로 바꾸면 Last Known 규칙 사용
→ 별도 Provider 허가 없이 정확한 Actor-bound X-ray Marker를 생성하지 않음

SensorLost / Contact Lost
→ Tracking Provider가 Contact를 무효화하면 Marker 제거
→ UI가 자체 Timer로 위치를 계속 추적하지 않음
```

- `BehindCamera`에서도 Marker를 화면 중앙을 가로질러 반전시키지 않고 Safe Region 가장자리에 안정적으로 유지한다.
- Edge Marker는 방향 정보를 주는 것이 목적이며 거리를 표현하기 위해 가장자리에서 더 멀리 밀거나 크기를 거리 비례로 바꾸지 않는다.
- Live → Last Known이면 현재 Target Panel/Radar 계약과 동일하게 마지막 유효 World Position을 사용하고 이동을 외삽하지 않는다.
- Last Known Marker는 Ghosted 상태로 낮은 시각 강도를 사용하며 같은 Entity를 Live로 재획득하면 별도 재선택 없이 정상 Marker로 복귀한다.
- Contact Lost면 Selected Marker와 Edge Marker를 함께 제거한다. 다음 Target을 자동 선택하지 않는다.
- Destroyed Hold 동안에는 선택 상태 수명만 유지하며 Marker를 낮은 시각 강도로 잠깐 유지할 수 있다. X·해골 아이콘으로 자동 교체하지 않고 Hold 종료 시 제거한다.

#### 17.7.3 Target Bounds와 Reticle 충돌

- On-screen Selected Marker는 Provider가 Screen Bounds를 제공하면 그 Bounds 바깥에 배치하고, Bounds가 없으면 안정적인 최소 크기의 Anchor Marker를 사용한다.
- Marker 크기를 대상의 실제 월드 크기에 무제한 비례시키지 않고 최소/최대 시각 크기를 Style Token으로 Clamp한다.
- Command/Turret Reticle은 World Target Marker보다 높은 HUD Draw Order를 유지한다.
- Target Marker와 Command Reticle이 겹쳐도 Reticle을 가리지 않도록 Open Bracket 중앙을 비운다.
- Target Marker가 Reticle과 겹쳤다는 사실만으로 자동 Lock이나 Fire 가능 판정을 만들지 않는다.

#### 17.7.3A Objective World Marker / Mission Navigation 권한

Objective World Marker는 MissionSummary와 다른 표시 책임을 가진다.

```text
MissionSummary
= 현재 Primary Objective가 무엇인지
= 진행/Timer/상태를 Compact Text로 설명

Objective World Marker
= 그 Primary Objective가 Navigation Anchor를 가질 때
= 화면에서 어디로 가야 하는지 공간 방향을 표시
```

현재 실제 C++ 조사에서는 Mission/Objective Runtime Provider, MissionManager, Objective Actor Registry가 확인되지 않았다. 현재 UI Source에 존재하는 것은 `ECFHUDSlotId::MissionSummary`와 `ECFHUDSlotId::ReticleLayer` Layout Slot뿐이다.

따라서 Current Runtime 계약은 다음과 같다.

```text
Mission Runtime Provider 없음
→ 실제 Gameplay MissionSummary Collapsed
→ 실제 Gameplay Objective World Marker Collapsed

D1 Visual Prototype / Static Mock
→ Mock MissionSummary / Mock Objective Marker 가능
→ 실제 Mission Gameplay 구현 또는 Runtime Binding 증거가 아님
```

이번 설계 때문에 `ECFHUDSlotId::ObjectiveMarker` 같은 새 고정 HUD Slot을 추가하지 않는다. Objective는 월드 투영 UI이므로 향후 **Game Layer 의미 책임 / 기존 Stretch ReticleLayer 내부 Projection Container**에서 구현할 수 있다. 실제 소유권 이전 시 Root Game Layer 구조와 함께 재검토한다.

#### 17.7.3B Objective Navigation 의미 데이터

후속 Mission Provider가 실제 구현되면 Objective Navigation Presenter가 최소 다음 의미를 받을 수 있어야 한다.

```text
PrimaryObjectiveIdentity
bNavigationMarkerAvailable
PresentationWorldAnchor
bPresentationAnchorValid
AnchorPrecision                 # Exact / Approximate 의미 후보
NavigationVisibilityPolicy     # Mission이 공개를 허용한 범위
DistanceText                   # 선택적 Player-facing 표시값
ObjectiveState
AssociatedTargetIdentity       # 선택적, 신뢰 가능한 동일 Entity 연결용
```

위 이름은 C++ 타입 확정 전의 **의미 계약**이며 CF-FQ-032가 Mission Runtime enum/class를 선행 확정하지 않는다.

권장 흐름:

```text
Mission Gameplay Provider
→ Primary Objective 결정
→ Player-facing Navigation 정보 생성
→ Objective Navigation Presenter
→ Camera Projection / Safe Region 해결
→ Objective Marker View Data
→ Widget
```

금지 흐름:

```text
Objective Widget Tick
→ World Actor 검색
→ Mission Tag 검색
→ 가장 가까운 Actor 선택
→ ActorLocation 직접 조회
→ Marker 생성
```

Mission Provider가 Navigation Anchor를 제공하지 않는 Objective는 정상적인 **Text-only Objective**다. MissionSummary는 유지할 수 있지만 World Marker는 Collapse한다.

#### 17.7.3C Primary 1개 / Secondary Marker 제한

P0 Objective World Navigation은 Mission Provider가 명시한 **Primary Objective 1개만** 직접 표시한다.

```text
PrimaryObjectiveIdentity
→ Navigation Marker 후보 1개

AdditionalObjectiveCount
→ MissionSummary +N OBJECTIVES 용도
→ 자동 World Marker 생성 근거가 아님
```

- 여러 Objective가 활성화돼도 Widget이 거리, 화면 중앙 근접도, 남은 시간, Objective 종류, 위험도 또는 생성 순서로 자체 Primary를 선택하지 않는다.
- `+N OBJECTIVES`를 근거로 Secondary World Marker를 생성하거나 자동 순환하지 않는다.
- 후속 Mission Gameplay가 동시 다중 Navigation을 실제 요구하면 Provider가 별도의 `NavigationRelevant / DisplayPriority` 의미를 소유하는 확장 계약으로 추가한다. 현재 P0 기본 HUD에는 포함하지 않는다.
- Primary가 변경되면 MissionSummary와 Objective Marker가 같은 Presentation Snapshot에서 함께 바뀌는 구조를 권장한다. 한 프레임 이상 서로 다른 Objective를 가리키는 상태를 만들지 않는다.

#### 17.7.3D Anchor Precision / 정보 공개 경계

Objective Navigation은 Mission이 플레이어에게 **알려주기로 한 위치만** 사용한다.

```text
Exact
→ 정확한 Player-facing Navigation Anchor 제공
→ 정상 Point Marker

Approximate
→ 검색 구역 중심, 추정 위치 등 근사 Anchor 제공
→ UI는 그 Anchor만 표시
→ 실제 Objective Actor를 찾아 정확 위치로 보정하지 않음

No Anchor
→ World Marker 없음
```

- `AnchorPrecision`은 현재 미구현 Mission Runtime의 정확한 enum을 의미하지 않고 미래 Presenter가 정확/근사 위치 차이를 보존해야 한다는 계약이다.
- Mission이 Search Area만 공개했는데 TargetSelect, Sensor Contact 또는 Actor Registry에서 실제 목표 Actor 위치를 읽어 Exact Marker로 승격하지 않는다.
- Objective가 Actor 기반이어도 Widget이 Component/Bone/Bounds를 직접 검색하지 않는다. Mission Provider가 Player-facing PresentationWorldAnchor를 제공한다.
- Objective Marker가 World Geometry 뒤에서도 계속 표시돼야 하는지, LOS에서만 보여야 하는지, 특정 Mission 상태에서 숨겨야 하는지는 `NavigationVisibilityPolicy` 또는 동등한 Provider 결과가 소유한다.
- Through-wall Navigation Marker가 허용돼도 이는 **Mission Navigation 지식**이지 Sensor Tracking/X-ray Target 정보가 아니다. Target Identification, Lock, Fire-Control 유효성을 자동 제공하지 않는다.

#### 17.7.3E Objective Projection / 거리 갱신

Objective는 기존 Target Marker와 같은 기본 투영 수학을 재사용한다.

```text
PresentationWorldAnchor
→ Camera View Space
→ Front / Behind 판정
→ Projected Screen Direction
→ Navigation Safe Region 검사
→ OnScreen 또는 Edge Marker View Data
```

단, **시각 실루엣은 Target과 공유하지 않는다.**

```text
OnScreen Objective
→ Open Hex Beacon + Bottom Stem

OffScreen / BehindCamera Objective
→ Open Edge Hex Beacon

Selected Target
→ 기존 4-Corner / 2-Corner Bracket 유지
```

- Projection은 Camera/Anchor가 움직일 수 있으므로 렌더 프레임 기준으로 갱신할 수 있다.
- 거리 Text, Objective State Text 같은 비공간 값은 실제 Provider 변경 또는 약 `5~10 Hz` Presentation 갱신으로 충분하며 매 Tick Text 재생성을 요구하지 않는다.
- `DistanceText`가 Provider/Presenter에서 없으면 거리 행을 표시하지 않는다.
- Exact Anchor의 거리 계산을 Presenter가 허용하는 구조라면 Player Pawn 위치와 Presentation Anchor의 거리만 계산하고 Actor 내부 정보는 조회하지 않는다. 최종 Formatting/정밀도는 Player-facing 정책을 따른다.
- Approximate Anchor에서 UI가 정확한 Meter 정밀도를 자동 생성하지 않는다. 필요하면 Provider가 `~1.4 km`처럼 근사 의미를 포함한 Text를 제공한다.
- 거리값은 Objective 이름·설명을 복제하지 않고 Marker의 한 줄 보조 정보로만 사용한다.

#### 17.7.3F Persistent HUD / Safe Region

Objective Marker는 Persistent HUD Panel 뒤에 숨는 Screen Marker가 되지 않게 기존 Navigation Safe Region을 사용한다.

```text
Projected Anchor
→ Navigation Safe Region 내부
   → OnScreen Objective Marker

Projected Anchor
→ Safe Region 밖 또는 Panel 예약 영역
   → Edge Navigation Marker
```

- MissionSummary, AlertFeed, TargetPanel, VehiclePanel, RadarPanel, WeaponPanel 위치를 Objective 때문에 이동시키지 않는다.
- UI가 전체 Viewport 좌표를 단순 Clamp해 Panel 위에 Objective를 겹쳐 놓지 않는다.
- 16:9 중앙 Combat 시야를 우선하고 울트라와이드에서도 기존 Central HUD Canvas / Safe Region 원칙을 유지한다.

#### 17.7.3G Target Marker 충돌과 동일 Entity 결합

Mission Objective가 현재 Selected Target과 **신뢰 가능한 동일 Entity**라면 두 Full Marker를 중첩하지 않는다.

```text
PrimaryObjectiveIdentity / AssociatedTargetIdentity
== SelectedTargetIdentity
→ Target 4-Corner Bracket 유지
→ Objective Marker를 작은 Open Hex Mission Badge로 축소
→ Target Bracket 상단 중앙 바깥에 부착
```

- Target Selection Bracket은 선택 상태를 계속 소유한다.
- Objective Badge는 같은 Entity가 Mission Primary라는 의미만 추가한다.
- 같은 Entity라고 자동 Target Selection, Lock, Tactical Analysis 또는 Fire 허가를 만들지 않는다.
- Target Marker가 이미 같은 위치의 거리값을 표시하면 Objective Badge는 거리 Text를 중복하지 않는다.
- Stable Identity가 없으면 Screen 좌표가 겹친다는 이유로 두 Marker를 같은 Entity로 추정하지 않는다.

서로 다른 Target과 Objective가 화면에서 겹치는 경우:

- Selected Target Marker의 Anchor/Bounds는 Objective 충돌 때문에 이동하지 않는다.
- Objective World Anchor도 다른 위치처럼 보이도록 큰 Screen Offset을 주지 않는다.
- Draw Order는 `Objective < Selected Target < Command/Turret Reticle`을 유지한다.
- 충돌 시 Objective의 DistanceText/보조 Text를 먼저 Collapse하고 핵심 Open Hex Shape는 가능한 유지한다.
- Reticle 주변에서는 Objective 보조 Text를 숨겨 조준 판독성을 우선한다. Objective 위치를 거짓으로 옮기지 않는다.

#### 17.7.3H Edge Marker 충돌 해결

Selected Target과 Primary Objective가 같은 Safe Region Edge 근처에 투영될 수 있다.

P0 충돌 해결 순서:

```text
1. Selected Target Edge Marker
   → 원래 Ray-SafeRegion 교차 위치 유지

2. Primary Objective Edge Marker
   → 같은 Edge의 Tangential 방향으로 이동
   → 최소 40 Design Unit 간격 목표
   → 최대 56 Design Unit까지만 이동

3. 공간 부족
   → Objective DistanceText Collapse
   → Objective Icon-only 유지
```

- Objective Edge Marker를 이동해도 그 열린 면/방향 표현은 원래 Objective bearing을 계속 가리킨다.
- Edge Tangential Shift는 **UI 충돌 회피**일 뿐 World Anchor를 변경하지 않는다.
- Target Edge Marker를 Objective 때문에 이동시키지 않는다. 전투 Target 선택의 즉시 판독을 우선한다.
- P0는 Primary Objective 1개만 있으므로 Objective끼리 Stack, `+N` Edge Bubble, Carousel을 만들지 않는다.

#### 17.7.3I Reticle / Lock / Fire-Control 우선순위

공간 표시의 기본 의미 우선순위는 다음이다.

```text
Command/Turret Reticle + Immediate Fire State
→ 플레이어 조준/즉시 발사 판단

Selected Target + Lock
→ 전투 대상 선택과 Fire-Control

Primary Objective Marker
→ Mission Navigation
```

- Objective Anchor와 Command Reticle이 화면에서 일치해도 Aim Target, Lock, Fire 허가가 아니다.
- Objective가 Selected Target과 같은 Entity여도 Lock 상태는 실제 Fire-Control Provider가 별도 제공해야 한다.
- Objective Marker가 Target 관계색, `HOSTILE`, ID 상태, Scanner Intelligence를 자동 공개하지 않는다.
- Hit Confirmation과 Incoming Damage Direction은 순간 전투 피드백이므로 기존 높은 Draw Order를 유지하며 Objective Marker가 이들을 가리지 않는다.

#### 17.7.3J Radar / Sensor와 Mission Navigation 분리

Objective World Marker가 존재한다고 Radar Contact를 생성하지 않는다.

```text
Sensor Provider Contact
→ Radar Blip

Mission Provider Navigation Anchor
→ Objective World Marker
```

- Sensor 범위 밖 Objective를 Mission Marker가 존재한다는 이유로 Radar의 `◆ / ▼ / ●` Contact로 만들지 않는다.
- Objective Target이 실제 Sensor Contact이기도 하면 Radar는 Sensor 의미로 자신의 Contact를 표시한다. Mission 여부는 Radar Contact의 관계/식별 상태를 덮어쓰지 않는다.
- 후속 Radar Mission Overlay가 필요하면 Sensor Contact와 다른 별도 Overlay Channel/Shape를 설계한다. 현재 Radar P0 계약에는 포함하지 않는다.

#### 17.7.3K Lifecycle / Pause / Destroyed 경계

Objective `Active / Completed / Failed`, Marker 유지시간, 다음 Primary 전환은 Mission Provider가 소유한다.

- Completed/Failed 순간 짧은 1회 시각 전환은 허용하지만 Widget이 임의 `1초 후 제거` 같은 Gameplay Hold Timer를 만들지 않는다.
- Provider가 완료 상태를 유지하면 그 기간만 Marker가 완료 Style을 표시할 수 있고 새 Primary가 제공되면 같은 Snapshot에서 전환한다.
- World Pause 중 Mission Game Time, Timer, Objective Lifecycle은 정지한다. Projection Animation이 UI 시간으로 동작하더라도 Gameplay Objective 진행을 Real Time으로 갱신하지 않는다.
- Player Vehicle Destroyed만으로 Mission Objective를 Failed 처리하거나 Marker를 삭제하지 않는다.
- 기존 Player Destroyed 계약에서 상위 Flow가 `bShowCombatHUD=false` 또는 동등한 Terminal HUD 종료를 제공하면 Objective World Marker도 Game/Reticle Navigation 표현과 함께 Collapse한다.
- 미래 Spectate가 Mission Navigation을 계속 보여줘야 하면 Spectate용 Provider 정책을 별도로 정의하며 기존 Player Combat Marker를 암묵 재사용하지 않는다.
- Mission 전체 Success/Failure와 Result Screen은 기존 Outcome Flow가 소유한다. Objective Marker가 중앙 Result Banner를 만들지 않는다.
- CF-PDL-0009에 따라 Objective 생성, Primary 변경, 화면 밖 전환, 완료·실패에 게임/UI 사운드를 추가하지 않는다.

#### 17.7.4 Lock / Fire-Control 미래 계약

현재 실제 소스에는 별도 `LockProgress / LockState / TargetLock` Runtime이 확인되지 않는다. 따라서 다음은 **후속 Provider가 구현될 때 UI가 소비할 의미 계약**이며 Current System으로 기록하지 않는다.

```text
LockSupported
LockState
NormalizedAcquireProgress   # 실제 Provider가 진행도를 제공할 때만
LockedTargetIdentity        # 현재 Selected Target과 같은지 검증 가능한 안정 식별자
DisplayFailureReason        # Player-facing / 선택적
```

후속 상태 의미 후보:

```text
Hidden / NotSupported
→ 해당 무기·장비가 Lock을 사용하지 않음

Idle
→ Lock 지원은 하지만 현재 획득 진행 없음

Acquiring
→ 실제 Provider가 획득 진행 중

Locked
→ 실제 Provider가 Lock 성공을 확정

Blocked
→ 실제 Provider가 Lock 불가 이유를 제공

Lost
→ 실제 Provider가 기존 Lock 상실 사건을 제공하는 짧은 전환 상태
```

정확한 C++ enum 이름과 Gameplay 상태 전이는 실제 Lock/Fire-Control 기능이 소유한다.

- Selected Target만으로 `Acquiring`을 시작하지 않는다.
- Target이 Reticle 안에 있다는 사실만으로 Progress를 증가시키지 않는다.
- ID2, Scanner Tactical Analysis 완료, Radar Live Contact도 자동 Lock 성공 조건이 아니다.
- Lock이 필요 없는 무기는 `LOCK N/A`를 상시 표시하지 않고 Lock UI 전체를 Collapse한다.
- `NormalizedAcquireProgress`가 없으면 HUD가 시간을 추정해 가짜 Percent를 만들지 않는다.
- Lock Indicator는 기본적으로 Selected World Target Marker 주변에 붙으며 Command Reticle 위치를 끌어당기거나 Turret Reticle 의미를 바꾸지 않는다.
- Lock 성공 후에도 실제 발사 가능 여부는 Weapon/Fire-Control 검증이 소유한다. Cooldown, Ammo, MuzzleBlocked, 정렬 정책 같은 독립 조건을 우회하지 않는다.
- Last Known 선택 유지가 Lock 수명을 자동 연장하지 않는다. Lock Provider가 유효성을 잃으면 UI는 즉시 그 결과를 따른다.
- Destroyed Hold도 Lock/자동조준/발사 유효시간을 연장하지 않는다.
- CarFight 전역 사운드 비지원 정책에 따라 Acquire/Locked/Lost 피드백은 시각 정보만으로 완결한다.

#### 17.7.5 Damage Feedback Source of Truth

Damage Feedback는 `FireFeedback`과 같은 입력을 재해석하는 기능이 아니다.

현재 Runtime에서 이미 존재하는 두 핵심 입력은 다음이다.

```text
FCFDamageHitContext
→ HitActor
→ ImpactLocation / ImpactNormal
→ IncomingDirection
→ InstigatorActor
→ WeaponId / ProjectileId
→ bBlockingHit

FCFVehicleDamageResult
→ bDamageAccepted
→ bAppliedToAnyLayer
→ ArmorDirection
→ Shield / Armor / Integrity 실제 적용 결과
→ bShieldBrokenThisHit
→ bArmorBrokenThisHit
→ bDestroyedThisHit
```

의미 분리:

```text
FireSuccess
= Fire Command 승인

DamageHitContext
= 무엇이 어디에서 어떤 방향으로 맞았는지

VehicleDamageResult
= 그 Hit가 실제 방어층에 어떤 결과를 만들었는지

Hit Confirmation
= 로컬 공격자의 실제 VehicleDamageResult가 방어층 감소를 확인한 경우

Incoming Damage Feedback
= 로컬 플레이어 차량에 실제 VehicleDamageResult가 적용된 경우
```

따라서 `FCFVehicleFireFeedbackViewData.FireSuccess`를 Hit Confirmation으로 재사용하지 않는다.

현재 구현에는 제품 HUD가 그대로 사용하기에는 중요한 연결 공백이 있다.

```text
피격 차량 UCFVehicleDefenseComp.OnVehicleDamageResolved
→ FCFVehicleDamageResult는 제공
→ 원본 FCFDamageHitContext는 Event Param에 없음

피격 차량 UCFVehicleHealthComp.OnVehicleDamaged
→ FCFDamageHitContext 제공
→ Integrity까지 피해가 도달한 경우에만 발생
→ Shield/Armor-only Hit에는 발생하지 않음

공격자 HitScan 경로
→ FCFVehicleDamageResult를 계산
→ 현재 Pawn에는 Integrity 호환 FCFDamageApplyResult만 저장

공격자 Projectile 경로
→ Projectile Actor는 FCFVehicleDamageResult 보존
→ Pool → Owner Pawn 전달은 현재 DamageHitContext + Integrity 호환 결과 중심
```

결론:

- `UI-P0-03` Damage Presenter/Provider는 **원본 HitContext와 전체 VehicleDamageResult가 동시에 존재하는 실제 Damage Resolution 지점**에서 Player-facing Damage Event를 받아야 한다.
- Shield/Armor-only Hit를 놓치므로 `LastDamageApplyResult.bApplied`만 폴링해서 Hit Confirmation을 만들지 않는다.
- 피격 방향을 얻기 위해 `OnVehicleDamaged`만 구독하지 않는다. Integrity에 도달하지 않은 Shield/Armor Hit가 누락된다.
- `LastDamageHitContext`, `LastVehicleDamageResultSummary`, Actor 이름 같은 Debug Cache/String을 Gameplay HUD Source로 사용하지 않는다.
- Widget이 매 프레임 `FindComponentByClass`, Actor 검색 또는 Projectile Pool 내부 상태를 조회하지 않는다.
- 실제 C++ Event/Adapter 형태는 UI-P0-03 구현 시 결정하되 **HitContext + VehicleDamageResult 한 사건 단위 결합**은 불변 계약으로 둔다.

후속 Damage Presentation 의미 데이터는 최소 다음 정보를 제공할 수 있어야 한다.

```text
DamageEventId
EventGameTime
Perspective                  # Incoming / OutgoingConfirm
bAppliedToAnyLayer
bDirectionValid
SourceDirectionWorld
ArmorDirection
DeepestAffectedLayer         # None / Shield / Armor / Integrity
bShieldBrokenThisHit
bArmorBrokenThisHit
bDestroyedThisHit
```

필요한 Panel 수치 갱신은 기존 Vehicle Defense View Data가 소유하고 중앙 Damage Feedback View Data에 전체 내부 계산값을 중복 복사하지 않는다.

`DamageEventId`는 같은 HitScan 또는 Projectile Impact가 Direction Indicator와 Hit Confirmation에 중복 재생되지 않게 하는 사건 식별자다. Object Path, Actor 이름, Component 이름을 Player-facing ID로 사용하지 않는다.

#### 17.7.6 Incoming Damage Direction 계산 계약

Incoming Damage Direction의 목적은 **공격원이 플레이어 기준 어느 방향에 있었는지** 알려주는 것이다. 6방향 Armor 판정과는 별도 좌표계다.

우선 입력:

```text
SourceDirectionWorld = Normalize(-DamageHitContext.IncomingDirection)
```

`IncomingDirection`은 탄이 대상 쪽으로 진행한 월드 방향이므로 화면에서 공격원이 있는 쪽을 가리키려면 반대 방향을 사용한다.

Fallback:

```text
IncomingDirection 무효
+ InstigatorActor 유효
+ InstigatorActor != HitActor
→ Normalize(InstigatorLocation - HitActorLocation)
```

- 두 입력이 모두 무효이면 `bDirectionValid=false`로 두고 Direction Indicator를 숨긴다.
- Self Damage, Environment, Damage Over Time처럼 방향 의미가 불명확한 Damage Event에 임의 전방 방향을 넣지 않는다.
- `InstigatorActor` Fallback은 방향 계산에만 사용할 수 있으며 이름, 진영과 식별 상태를 HUD에 자동 공개하지 않는다.

Camera-relative 방향 계산은 Projection Presenter가 담당한다.

```text
SourceDirectionWorld
→ World Up 평면에 수평 투영
→ 현재 Camera Forward의 수평 투영과 signed yaw 계산
→ 0° = 화면 12시
→ +90° = 화면 3시
→ ±180° = 화면 6시
→ -90° = 화면 9시
```

- 외부 3인칭 Camera가 차량 둘레를 회전할 수 있으므로 `ArmorDirection`을 그대로 화면 12/3/6/9시로 매핑하지 않는다.
- Damage Event가 살아 있는 동안 World Source Direction은 사건 시점 값으로 보존하고, 현재 Camera가 회전하면 Screen Angle은 새 Camera 기준으로 다시 투영할 수 있다.
- 정확한 공격자 Actor 위치를 계속 추적하는 방식이 아니라 **피격 순간의 World Direction**을 보존하므로 벽 뒤 공격자를 지속 추적하는 X-ray 정보가 되지 않는다.
- `ArmorDirection`은 별도로 VehiclePanel의 Front/Left/Right/Rear/Top/Bottom Plate Highlight에 사용한다.
- Top/Bottom 같은 수직 의미는 Provider가 `ArmorDirection` 또는 별도 Vertical Cue를 명시한 경우에만 Style이 보조 표시한다.

#### 17.7.7 Damage Layer 분류

중앙 Direction Indicator와 Hit Confirmation은 정확한 수치보다 **이번 Hit가 실제로 도달한 가장 깊은 방어층**을 대표 상태로 사용한다.

Provider 분류 순서:

```text
DamageAppliedToIntegrity > 0
→ Integrity

else DamageAbsorbedByArmor > 0 또는 ArmorBefore > ArmorAfter
→ Armor

else DamageAbsorbedByShield > 0 또는 ShieldBefore > ShieldAfter
→ Shield

else
→ None
```

- `bAppliedToAnyLayer=false`이면 기본 Damage Feedback 사건을 표시하지 않는다.
- Shield를 깨고 Armor에 도달한 Hit는 Armor로, Armor를 관통해 Integrity에 도달한 Hit는 Integrity로 대표한다.
- 이 분류는 Damage 자체를 다시 계산하는 것이 아니라 **이미 확정된 VehicleDamageResult의 실제 적용 결과를 표시용으로 분류**하는 것이다.
- Module Damage와 Physical Impulse는 현재 Runtime 적용값이 0인 P1 예약 영역이므로 현재 Hit Marker 종류를 만들지 않는다.

#### 17.7.8 Outgoing Hit Confirmation 계약

Outgoing Hit Confirmation은 로컬 플레이어가 만든 Damage Event 중 실제 방어층 감소가 확인된 경우에만 생성한다.

기본 조건:

```text
Local Player가 Damage Event의 Source
AND bAppliedToAnyLayer == true
AND DeepestAffectedLayer != None
→ Hit Confirmation
```

- 선택 Target 여부는 필수 조건이 아니다. 발사 후 플레이어가 다른 방향을 보더라도 Projectile이 실제 피해를 적용하면 확인할 수 있다.
- Target Lock 여부도 필수 조건이 아니다.
- World Geometry Impact, MissingHealthComponent, SelfDamageBlocked, TargetDestroyed 등의 Reject 결과는 Damage Hit Confirmation이 아니다.
- Impact FX 성공 여부를 Hit Confirmation으로 사용하지 않는다.
- 로컬 플레이어 자신에게 허용된 Self Damage가 적용되는 특수 경우에는 기본적으로 Incoming Damage Feedback을 우선하고 Outgoing Hit Marker는 중복 표시하지 않는다.

Destroyed 처리:

- `bDestroyedThisHit=true`이면 Hit Confirmation을 조금 강하게 표현할 수 있다.
- 하지만 대상 파괴 수명과 선택 유지 판정은 Target Lifecycle이 소유한다.
- Hit Confirmation이 Destroyed Target을 자동 선택하거나 다음 Target을 선택하지 않는다.
- 큰 Kill Banner, Kill Feed와 Score 처리는 현재 CF-FQ-032 범위가 아니다.

현재 구현상 **Shield/Armor-only Hit를 포함한 정확한 Outgoing Hit Confirmation을 만들려면 전체 `FCFVehicleDamageResult`를 공격자 Damage Presentation 경로까지 전달하는 후속 연결이 필요하다.** 이 연결은 UI-P0-03 구현 항목이며 이번 디자인 문서 갱신은 Source를 수정하지 않는다.

#### 17.7.9 Local Vehicle Damage Feedback 계약

로컬 피격 Feedback은 한 사건을 여러 Widget이 중복 판정하지 않도록 Presenter가 역할을 나눈다.

```text
Damage Presentation Provider
→ Incoming Direction Event
→ VehiclePanel Damage Pulse Event
→ 필요 시 Alert Event
```

VehiclePanel은 다음을 직접 계산하지 않는다.

- IncomingDirection으로 ArmorDirection 재판정
- Shield/Armor/Integrity 피해량 재계산
- Break 임계값 재계산
- Critical Integrity 임계값 재계산

대신 실제 `FCFVehicleDamageResult.ArmorDirection`과 현재 Vehicle View Data를 사용한다.

```text
Shield 감소
→ Shield Bar 1회 Highlight

Armor 감소
→ ArmorDirection에 해당하는 Plate/Badge만 Highlight

Integrity 감소
→ Integrity Bar 1회 Highlight
→ Style에서 허용한 짧은 Peripheral Damage Vignette
```

Alert 경계:

- 일반 Hit마다 Alert를 만들지 않는다.
- `OnShieldBroken`, `OnArmorBroken`, Destroyed와 같은 실제 사건은 Gameplay/Presenter가 Alert로 승격할지 결정한다.
- `INTEGRITY CRITICAL` 같은 임계값 사건도 Widget이 Integrity Ratio를 보고 독립 생성하지 않는다.
- Alert가 생성되더라도 동일 사건의 Direction Arc와 VehiclePanel Highlight는 즉시 공간 정보/상태 정보 역할이 다르므로 함께 존재할 수 있다.
- 같은 문장을 Reticle과 VehiclePanel, Alert에 반복하지 않는다.

#### 17.7.10 Target Intelligence와 Damage Confirmation 경계

Hit Confirmation은 플레이어가 자기 공격의 **효과가 있었다는 사실**을 알려주는 피드백이지 Target Intelligence 데이터베이스가 아니다.

- Shield Hit Marker는 정확한 Shield Current/Maximum을 공개하지 않는다.
- Armor Hit Marker는 어떤 Armor 수치가 남았는지 공개하지 않는다.
- Integrity Hit Marker는 Target Integrity Percent를 자동 공개하지 않는다.
- 실제 Shield Impact Effect처럼 관찰 가능한 현상을 Knowledge로 승격할지는 Target Intelligence Provider가 별도 처리한다.
- Damage Feedback Widget이 Target Knowledge State를 직접 변경하지 않는다.

기본 P0에서는 Floating Damage Number를 사용하지 않는다. 후속 옵션이 생겨도 Target Intelligence/Provider가 공개를 허용한 Player-facing 수치만 표시해야 하며 내부 Damage Formula를 Widget이 계산하지 않는다.

#### 17.7.11 수명, 중복과 Pause

Damage Feedback 사건 수명은 Game Time을 기준으로 한다.

```text
World Running
→ Direction / Hit Confirm Duration 진행

World Paused
→ 남은 Duration 정지

Resume
→ 남은 Duration부터 계속
```

동일 Damage Event는 `DamageEventId`로 1회만 처리한다.

Incoming Direction은 같은 방위의 연사 Hit를 가까운 Angle Bucket으로 Merge/Refresh할 수 있다. 이는 Damage Event를 하나로 합쳐 Gameplay 결과를 잃는 것이 아니라 **화면 Direction Arc의 시각 중복만 줄이는 Presenter 정책**이다.

Hit Confirmation은 고연사 중 매 Event를 받을 수 있지만 동일 프레임에 많은 Marker Instance를 쌓지 않는다. 하나의 Marker 상태를 갱신하고 가장 최근/가장 깊은 Layer 피드백을 우선한다.

CF-PDL-0009에 따라 Incoming Damage, Hit Confirmation, Shield/Armor Break와 Destroyed Confirmation에 UI/Game Sound를 추가하지 않는다.

#### 17.7.11A Reticle Layer Global Declutter 권한

개별 Reticle/Marker Widget이 서로를 직접 조회해 숨기거나 이동시키지 않는다. UI-P0-03 이후 중앙 World Projection은 하나의 **공통 Reticle Layer Presentation Resolver** 또는 동등한 Presenter 단계가 한 프레임의 화면 상태를 모아 결정하는 구조를 사용한다.

정확한 클래스명은 구현 시 확정하며 현재 의미 계약은 다음이다.

```text
Gameplay / Provider
→ 각 요소의 실제 상태와 표시 가능한 데이터 제공

Projection Presenter
→ World Anchor / Bounds를 Screen Space로 변환

Reticle Layer Presentation Resolver
→ Stable Identity Merge
→ Safe Region
→ Declutter / Auxiliary Text Budget
→ Edge Collision
→ Resolved Presentation

Widget
→ 계산 없이 결과 표시
```

Declutter가 절대 소유하지 않는 것:

```text
Target Selection
Lock 획득/상실
Mission Primary Objective 선택
Objective 완료/실패
Damage Result
Hit Confirmation 발생 여부
Incoming Damage Event 발생 여부
Knowledge 공개 상태
Fire 가능 여부
```

즉 화면 표현을 Compact하게 만든다는 이유로 Gameplay State를 Clear하거나 다음 대상을 선택하지 않는다.

#### 17.7.11B P0 Reticle Layer 입력 집합

현재 확정된 P0 의미 집합은 다음으로 제한한다.

```text
Command Reticle             1
Turret Reticle              1
Selected Target             0..1
Primary Objective           0..1
Lock                        0..1 / 미래 Provider, Selected Target 부착
Immediate Fire State        0..1 대표 상태
Hit Confirmation            0..1 현재 Presentation State
Incoming Damage Direction   0..4 Angle Bucket
```

이 상한은 Widget이 근처 Actor를 검색해 N개 중 몇 개를 고르는 규칙이 아니다. 이미 각 Gameplay/Provider 계약에서 **Selected Target 1개, Primary Objective 1개**로 제한된 결과를 합친 것이다.

- Target Candidate 목록을 World Marker로 자동 확장하지 않는다.
- Radar Contact를 World Marker로 자동 확장하지 않는다.
- `AdditionalObjectiveCount`를 Secondary Objective Marker 수로 해석하지 않는다.
- Multi-lock과 Secondary Objective Marker가 실제 추가될 경우 별도 Density 확장 설계를 먼저 수행한다.

#### 17.7.11C Resolved Presentation 의미 데이터

향후 공통 Resolver는 각 요소에 대해 최소 다음 의미를 내부적으로 다룰 수 있다.

```text
PresentationId
PresentationRole
SourceVisible
ScreenAnchor
ScreenBounds
AuxiliaryText
StableEntityIdentity        # 있을 때만
ResolvedDisplayMode         # Full / Compact / ShapeOnly / AttachedBadge / CollapsedBySource
ResolvedAuxiliaryVisible
ResolvedOpacity
ResolvedScreenOffset        # 허용된 UI 충돌 회피 범위에서만
```

- 이름은 C++ 타입 확정 전 의미 Draft다.
- `PresentationRole`은 Gameplay Threat Score가 아니라 Command, Target, Objective, Lock 같은 UI Semantic Role이다.
- `StableEntityIdentity`가 없는 요소를 Screen 좌표가 가깝다는 이유로 같은 Entity로 Merge하지 않는다.
- `CollapsedBySource`는 Provider/Gameplay가 실제 표시 불가를 제공한 경우를 뜻한다. 단순 중앙 혼잡은 먼저 Compact/ShapeOnly를 사용한다.
- `ResolvedScreenOffset`은 Objective Edge Tangential Shift처럼 명시적으로 허용된 Presentation Offset에만 사용한다. On-screen World Anchor를 임의로 다른 Actor 위치처럼 이동시키는 용도가 아니다.

#### 17.7.11D 중앙 보호 Zone

중앙 보호 Zone은 Viewport 중심이 아니라 **현재 Command Reticle Screen Position**을 기준으로 계산한다.

Style 기본값은 `InGameUIStyleSpec.md v0.86.0`이 소유한다.

```text
Aim Core Radius      = 56
Combat Ring Radius   = 136
Status Lane          = 144 × 24 / Command 아래 Gap 8
```

의미:

```text
Aim Core
→ Command / Turret / Hit Confirm 핵심 Geometry 보호
→ 실제 Selected Target / Lock Open Geometry는 같은 위치에 존재 가능
→ 자유로운 World Marker Text 금지

Combat Ring
→ Incoming Damage Direction과 Target/Lock이 함께 읽히는 영역
→ World Marker 보조 Text 총량 제한

Outer Marker Field
→ 정상 Target/Objective Full 표현 가능

Edge Band
→ Safe Region 경계 Marker
```

Zone은 Marker를 World Position에서 밀어내는 Force Field가 아니다. 동일 위치가 실제 투영 결과이면 Shape는 그대로 두고 Secondary Text부터 줄인다.

#### 17.7.11E 결정적 충돌 해결 파이프라인

공통 Resolver는 매 프레임 다음 순서로 해결한다.

```text
1. 각 Provider의 SourceVisible / State 확정
2. World Anchor / Bounds Screen Projection
3. Stable Entity Identity로 Target + Objective 결합 여부 확정
4. Persistent HUD 예약영역 / Navigation Safe Region 적용
5. Selected Target Anchor / Bounds 고정
6. Primary Objective 위치 Resolve
   - On-screen Anchor 거짓 이동 금지
   - Edge에서만 기존 Tangential Shift 허용
7. Lock Geometry를 Selected Target에 부착
8. Command Status Lane 예약
9. Target/Lock Auxiliary Text Budget 적용
10. Objective Full → Compact → ShapeOnly 축약
11. Hit Confirmation / Incoming Damage transient Overlay 적용
12. 최종 ZOrder 렌더링
```

이 순서를 사용하면 자동사격 Hit Marker나 연속 피격 Arc가 발생할 때 Persistent Target/Objective가 매번 재배치되는 것을 방지할 수 있다.

#### 17.7.11F Semantic Priority와 Reflow 금지

Presentation 충돌 우선순위:

```text
Command / Turret
→ Immediate Fire State
→ Hit Confirmation / Incoming Damage
→ Selected Target / Lock
→ Primary Objective
```

다만 Hit/Incoming은 **Transient Overlay**이므로 높은 우선순위라는 이유로 Persistent Marker를 이동시키지 않는다.

```text
Hit Confirm 발생
→ Target 위치 유지
→ Objective 위치 유지
→ Hit Marker만 높은 Layer에서 짧게 표시

Incoming Damage 발생
→ 기존 Direction Bucket만 갱신
→ Target/Objective Layout 재Resolve 원인으로 사용하지 않음
```

Transient Feedback가 Persistent Marker의 Full/Compact 상태까지 매 Bullet 변경시키는 구현을 금지한다.

#### 17.7.11G Auxiliary Text Budget

중앙 화면이 복잡해지는 주원인은 Shape보다 **반복 Text**이므로 Text 예산을 별도로 둔다.

Command Status Lane:

```text
Immediate Fire State 최대 1줄
→ BLOCKED / ALIGNING / COOLDOWN / NO WEAPON 등
```

이 Lane은 다른 의미가 사용하지 않는다.

Selected Target 주변:

```text
최대 1 Auxiliary Line

우선순위:
1. 실제 Lock Blocked / Failure
2. 실제 Lock Progress / Locked State
3. LAST / DESTROYED 같은 Marker State
4. Target Distance
```

- Lock Geometry와 Target Bracket 자체는 별도 Shape이므로 유지 가능하다.
- `LOCK 62%`와 `842 m`를 같은 위치에 두 줄로 쌓지 않는다.

Combat Ring의 Command Status Lane 외 World Marker Auxiliary Text는 기본 **최대 1줄**이다.

Primary Objective:

- Combat Ring 밖에서는 선택적 Distance를 표시할 수 있다.
- Combat Ring 안에서 Target/Lock 보조 Text가 이미 있으면 Objective Distance를 숨긴다.
- Aim Core 안에서는 Objective 기본 ShapeOnly를 사용한다.
- 같은 Target Entity의 Objective Badge에는 별도 Distance를 중복하지 않는다.

Incoming Damage Vertical Cue:

- Arc는 기존 최대 4 Direction Bucket을 유지한다.
- `TOP / BOTTOM` Micro Label은 동시에 최대 1개만 표시한다.
- 여러 후보면 DeepestAffectedLayer 우선, 같은 Layer면 최신 Event 우선이다.

Hit Confirmation에는 Text를 추가하지 않는다.

#### 17.7.11H Persistent Marker Display 단계

Persistent World Marker는 공간이 부족할 때 데이터를 삭제하지 않고 Presentation만 다음처럼 축약한다.

```text
Full
→ Shape + Auxiliary

Compact
→ Shape + 핵심 Auxiliary 최대 1

ShapeOnly
→ Shape만

AttachedBadge
→ 동일 Entity의 하위 의미를 상위 Marker에 부착
```

Target:

- 핵심 Selected 4-Corner Bracket은 공간 혼잡만으로 제거하지 않는다.
- Lock은 실제 Provider 상태가 있을 때 Target 바깥 Segment로 유지한다.
- 보조 Text부터 줄인다.

Objective:

- Full → Compact → ShapeOnly 순으로 먼저 축약한다.
- Aim Core에서는 ShapeOnly가 기본이다.
- Selected Target과 동일 Entity면 AttachedBadge를 사용한다.

World Marker의 Shape 자체가 사라지는 것은 Contact Lost, Objective Navigation Anchor 없음, Provider Hidden, Terminal Combat HUD 종료 같은 **실제 Source 상태**가 있을 때다.

#### 17.7.11I Safe Region / Edge Density

Persistent Panel 예약영역은 기존 Navigation Safe Region으로 제외한다.

- Target/Objective는 Panel 위에 그대로 그리지 않는다.
- Command/Turret Reticle은 실제 Aim 결과이므로 Panel 회피를 위해 Clamp하지 않는다.
- 향후 Aim이 Persistent Panel 영역까지 실제로 이동할 수 있는 Camera 설계로 바뀌면 Marker 해킹이 아니라 HUD Layout Profile을 다시 검토한다.

P0 Edge Band 독립 Marker:

```text
Selected Target Edge 0..1
Primary Objective Edge 0..1
```

충돌 시:

```text
Target Edge
→ 원래 Ray 교차 위치 유지

Objective Edge
→ 같은 Edge Tangential Shift
→ 기본 최소 간격 40
→ 최대 Shift 56

그래도 공간 부족
→ Objective Distance 제거
→ Objective ShapeOnly 유지
```

`+N`, Stack, Carousel은 P0에 추가하지 않는다.

#### 17.7.11J Hysteresis와 화면 안정성

Projection 좌표가 경계에서 흔들릴 때 Marker가 Full/Compact를 매 프레임 전환하지 않도록 Style Token 기반 Hysteresis를 적용한다.

```text
Collision Enter Padding = 8
Release Padding         = 16
Auxiliary Fade          = 0.10 s
```

- Release는 Enter보다 더 큰 분리를 요구해 경계 떨림을 줄인다.
- 전환은 Opacity 위주로 처리하고 위치 Spring, Scale Pop, Bounce를 사용하지 않는다.
- Declutter 전환은 Gameplay Event가 아니므로 Alert, Sound, Haptic을 발생시키지 않는다.

#### 17.7.11K Density / Accessibility 일관성

UI Density 또는 사용자 HUD Scale이 달라져도 다음은 바뀌지 않는다.

```text
Command/Turret 의미
Target > Objective 충돌 우선순위
Status Lane 독점
Stable Identity 기반 Merge
Transient Feedback Reflow 금지
```

Compact:
- Auxiliary Text를 더 일찍 축약할 수 있다.
- 핵심 Shape 의미는 유지한다.

Expanded:
- Outer Marker Field에서 Auxiliary를 더 오래 유지할 수 있다.
- Aim Core에 긴 Text를 허용하지 않는다.

Accessibility Marker Scale이 커져 충돌이 늘어나도 Gameplay 데이터를 삭제하지 않고 같은 Full → Compact → ShapeOnly 순서를 사용한다.

#### 17.7.11L 갱신 주기 / 성능 경계

Projection과 Spatial Resolve는 Camera/World Anchor가 움직이므로 하나의 중앙 Presenter에서 **프레임 단위로** 수행할 수 있다.

반면 다음 Text/State는 값이 바뀔 때만 갱신한다.

```text
Target Distance Formatting
Objective Distance Formatting
Lock Display Text
Immediate Fire Display Text
```

- 각 Widget이 Tick에서 다른 Widget Geometry를 조회하지 않는다.
- 각 Widget이 Pawn, Target Actor, Mission Actor, Damage Component를 직접 검색하지 않는다.
- 공통 Resolver는 이미 Provider/Presenter가 준비한 Screen/Presentation 입력만 사용한다.
- Hit/Incoming Event 수명과 DamageEventId 중복 억제는 기존 Damage Presentation 계약을 유지한다.

현재 Mission Provider와 Lock Provider는 없으므로 이 입력은 자연스럽게 absent/Collapsed다. Declutter 구현을 위해 가짜 Objective/Lock Runtime을 만들지 않는다.

이 통합은 **D1-11 Visual User Preview Gate를 변경하지 않으며 UI-P0-03 Runtime Integration에서 실제 Presentation 구조를 구현할 때 적용하는 설계 계약**이다.

#### 17.7.12 Player Vehicle Life State와 Outcome State 분리

플레이어 자신의 차량 상태와 게임 전체 결과 상태는 별도 권한으로 관리한다.

```text
Player Vehicle Life State
= 현재 플레이어 차량이 생존/위험/파괴 중인지

Player Outcome State
= 차량 파괴 이후 게임이 Respawn / Spectate / Mission Failure / Result 중 무엇을 선택했는지
```

현재 확정 가능한 생존 상태 의미:

```text
Operational
→ 정상 생존 상태

Critical
→ 생존 중 위험 상태
→ 아직 주행·조준·발사가 가능한 상태일 수 있음

Destroyed
→ UCFVehicleHealthComp가 최초 파괴를 확정한 상태
→ 차량 생존 상태 종료
```

Outcome 상태는 현재 Runtime이 없으므로 의미 후보만 고정한다.

```text
None
DestroyedHold
RespawnPending
Spectate
Result
```

정확한 C++ enum·클래스 이름은 후속 Player/Mission Flow 기능이 소유한다.

중요 불변 조건:

- `Critical != Destroyed`다.
- `Destroyed != Defeat`다.
- `Destroyed != Result`다.
- `OnVehicleDestroyed`를 받은 UI가 Mission Failure나 Game Over를 직접 결정하지 않는다.
- 차량이 파괴되어도 향후 Gameplay는 Respawn, Spectate, 미션 계속, Mission Failure 또는 Result 중 다른 경로를 선택할 수 있다.

#### 17.7.13 현재 Destroyed Runtime 사실

현재 실제 구현에서 `UCFVehicleHealthComp`는 다음만 보장한다.

```text
CurrentIntegrity가 최초 0 도달
→ bDestroyed = true
→ DamageApplyResult.bDestroyedThisHit = true
→ OnVehicleDestroyed(DamageHitContext) 1회 Broadcast

후속 피해
→ TargetDestroyed Reject
```

현재 `VehicleHealthComp`가 직접 하지 않는 일:

```text
차량 입력 차단
Chaos 물리 정지
메쉬 숨김/교체
Pawn Destroy
Possession 해제
Respawn
Spectate 전환
Mission Failure
Result Screen 표시
World Pause
```

현재 실제 후속 반응으로 확인된 것은 다음이다.

- `CombatFxComp`가 `OnVehicleDestroyed`를 구독해 Destroyed FX를 1회 재생한다.
- `LauncherComp`는 활성 Ripple/Salvo 중 Owner Health가 Destroyed면 `OwnerDestroyed` 사유로 남은 예약 발사를 취소한다.
- Destroyed 차량은 `IsTargetSelectable=false`, `TargetTrackState=Invalid`가 되어 새 Target으로 정상 선택되기 어렵다.
- `VehicleCameraMode`에는 `Destroyed / Spectate` enum과 Mode Flag가 존재하지만 현재 Health Destroyed와 자동 연결된 호출 경로는 확인되지 않았다.
- `UCFUISubsystem`에는 Screen Layer와 `SetPrimaryScreenWidget`이 있지만 Result/Death/GameOver Widget이나 Player Outcome Runtime은 현재 존재하지 않는다.

따라서 UI 문서가 위 미구현 Gameplay 상태를 Current System처럼 주장하지 않는다.

#### 17.7.14 Integrity Critical Presentation Policy

현재 Gameplay Runtime에는 `IntegrityCritical` 권위 상태가 없으므로 Critical은 UI-P0-03 Presentation Provider가 만드는 **Player-facing 위험 상태**로 설계한다.

권장 기본 Presenter Preset:

```text
CriticalIntegrityEnterRatio = 0.25
CriticalIntegrityExitRatio  = 0.30
```

- 이 값은 파괴 임계값이 아니라 경고 표시용 기본 Preset이다.
- Enter/Exit 값을 다르게 두어 수리·피해가 경계에서 반복될 때 Alert가 빠르게 켜졌다 꺼지는 현상을 막는다.
- 정확한 값은 Gameplay/UX 튜닝으로 변경 가능하며 Widget Blueprint Literal로 넣지 않는다.
- Presenter가 Critical 정책을 제공하지 않는 구현 단계에서는 Widget이 `IntegrityRatio <= 0.25`를 독립 계산해 경고를 만들지 않는다.

기본 상태 전이:

```text
Operational
→ Presenter Critical Enter
→ Critical

Critical
→ Presenter Critical Clear
→ Operational

Critical 또는 Operational
→ authoritative OnVehicleDestroyed
→ Destroyed
```

Destroyed로 진입하면 Critical Persistent Alert와 Critical-only Accent는 종료하고 Destroyed Terminal 표현으로 교체한다.

#### 17.7.15 Player Destroyed Presentation Event

UI-P0-03 이후 Player Lifecycle/Presentation Provider는 Health의 최초 Destroyed 사건을 소비해 최소 다음 의미를 제공할 수 있어야 한다.

```text
PlayerVehicleLifeState
bCombatInteractionAllowed
bVehicleInputAllowed
bShowCombatHUD
OutcomeState
OutcomeTitle                 # Outcome Provider가 실제로 제공할 때만
OutcomeDescription           # 선택적
```

필드 이름은 의미 Draft이며 최종 C++ 타입명을 미리 고정하지 않는다.

`PlayerVehicleLifeState=Destroyed`의 Source of Truth는 `VehicleHealthComp.OnVehicleDestroyed / IsDestroyed`다. Widget이 Integrity Text가 0인지 보고 재판정하지 않는다.

반면 아래 권한은 Health 상태만으로 자동 추론하지 않는다.

```text
bCombatInteractionAllowed
bVehicleInputAllowed
bShowCombatHUD
OutcomeState
```

이 값은 상위 Player Lifecycle / Game Flow가 실제 Gameplay 상태와 함께 결정해야 한다.

#### 17.7.16 현재 Input / Fire Runtime Gap

현재 소스 교차검증 결과 Player Destroyed UI 구현 전에 반드시 해결해야 할 Gameplay 통합 공백이 있다.

```text
HandleFireStarted / ValidateFireCommandInternal
→ VehicleHealthComp.IsDestroyed 차단 없음

Throttle / Steering / Brake / Handbrake / Look 입력
→ Health Destroyed와 자동 연결된 차단 없음

Launcher active sequence
→ Destroyed를 감지해 남은 Ripple/Salvo는 취소됨
```

따라서 다음 구현을 금지한다.

```text
Health Destroyed 감지
→ UI Reticle/Weapon만 숨김
→ 실제 Pawn은 여전히 새 Fire/Drive 입력 가능
```

이는 화면상으로는 조작 불가처럼 보이지만 실제 Gameplay는 살아 있는 불일치다.

UI-P0-03 또는 후속 Player Lifecycle 통합에서 Player Destroyed Presentation을 실제 적용할 때는 **한 권위 있는 Combat/Vehicle Availability 상태**가 Drive·새 Fire·HUD의 행동 가능 상태에 함께 반영되어야 한다.

- UI는 Gameplay Input을 직접 막는 소유자가 아니다.
- 기존 Pause용 `ClearGameplayInputForPause`를 Destroyed 정책으로 임의 재사용하지 않는다. Pause는 Launcher 상태를 보존하는 별도 계약이다.
- Destroyed 순간 이미 발사된 Projectile을 자동 제거하지 않는다. Projectile은 독립 수명을 계속 가질 수 있다.
- 이미 진행 중인 Launcher 시퀀스의 `OwnerDestroyed` 취소는 현재 Runtime 동작을 유지한다.
- 이 Runtime 공백은 **D1-11 User Preview Gate를 막는 문제가 아니며 UI-P0-03 Runtime Integration 선행조건**으로 기록한다.

#### 17.7.17 Destroyed Combat HUD 상태 전이

Player Lifecycle Provider가 Destroyed와 실제 Combat Availability를 함께 제공할 때 기본 HUD 전이는 다음이다.

```text
Operational / Critical
→ Combat HUD 정상 활성

Destroyed Event
→ 최종 Damage Event 1회 표현
→ VehiclePanel Destroyed Terminal State
→ Critical Alert Clear

CombatInteractionAllowed=false 확정
→ Reticle / Lock / Target Marker / Weapon / Radar Fade·Collapse
→ 새 Incoming / Outgoing Combat Feedback 억제

OutcomeState 변경
→ DestroyedHold / Respawn / Spectate / Result 중 실제 상태 표현
```

모듈별 소유권:

| 모듈 | Destroyed 기본 계약 |
|---|---|
| VehiclePanel | 가장 오래 남는 Terminal HUD. 실제 최종 Shield/Armor/Integrity와 `DESTROYED` 표시 |
| Command/Turret Reticle | 실제 Combat Interaction 금지와 함께 Fade/Collapse |
| Lock Indicator | 실제 Fire-Control/Lifecycle에서 무효화된 뒤 Collapse |
| Selected Target Marker | HUD 표시만 종료 가능. UI가 TargetSelect Gameplay State를 강제 Clear하지 않음 |
| WeaponPanel | 실제 Combat Interaction 금지와 함께 Fade/Collapse |
| RadarPanel | Terminal Combat HUD에서 기본 Collapse. Spectate 전용 Radar가 필요하면 별도 상태 |
| AlertFeed | Critical Alert 종료. Combat Alert 신규 생성 억제. Result 진입 시 HUD와 함께 Collapse |
| MissionSummary | Destroyed만으로 자동 Mission Failed 처리하지 않음. Mission/Outcome Provider 결과를 따름 |
| Incoming Damage | 최초 파괴 Hit의 현재 1회 표현만 완료하고 새 Arc 생성 중지 |
| Hit Confirmation | Player Terminal HUD에서는 새 Marker 억제. 이미 발사된 Projectile의 Gameplay Damage는 계속 가능 |

VehiclePanel의 Speed/RPM/Gear는 DestroyedHold에서 실제 Provider가 값 갱신을 계속하면 물리 상태대로 표시한다. UI가 차량 파괴를 이유로 0값을 생성하지 않는다.

#### 17.7.18 Destroyed Hold 수명 소유권

Player 자신의 Destroyed Hold는 기존 **Target Destroyed 1.25초 Hold와 완전히 다른 수명**이다.

- Target 1.25초 Hold는 적 Target 선택 해제 전 Kill Confirmation 계약이다.
- Player Destroyed Hold는 상위 Player/Outcome Flow가 다음 상태를 결정하기 전의 Terminal Presentation이다.
- Player Destroyed Widget이 Target의 1.25초 값을 재사용하지 않는다.
- 기본 UX 권장으로 상위 Flow가 약 **1.0초 이상의 월드 파괴 확인 구간**을 줄 수 있으나 이는 Style/Widget Timer가 아니라 Player Outcome 정책이 소유한다.
- Result가 즉시 World Pause를 적용하는 미래 정책이 생기더라도 `Game Time 1초 Timer가 끝나야 Result로 이동`하는 구조를 만들지 않는다. 상위 Flow가 명시적인 상태 전환 사건을 발행해야 한다.

#### 17.7.19 Camera Destroyed / Spectate 경계

현재 Camera 타입에 `Destroyed`와 `Spectate`가 존재하지만 실제 Health Destroyed 자동 연결은 확인되지 않았다.

후속 Player Lifecycle은 필요할 경우:

```text
DestroyedHold
→ Camera Destroyed Mode

RespawnPending / Spectate
→ Camera Spectate Mode
```

같은 전환을 요청할 수 있다.

- UI가 `SetCameraModeFlags`를 직접 호출해 카메라 Gameplay 상태를 소유하지 않는다.
- Destroyed Terminal Plate가 보인다고 Destroyed Camera가 활성됐다고 가정하지 않는다.
- Result Screen이 활성화됐다고 Spectate Camera가 자동 필요하다고 가정하지 않는다.

#### 17.7.20 Result Flow와 UISubsystem 경계

현재 `UCFUISubsystem`은 미래 Result를 수용할 구조적 진입점을 이미 가진다.

```text
UI Root
→ Screen Layer
→ SetPrimaryScreenWidget(...)
→ FullScreen + 지정 Input Mode
```

그러나 현재 Result Widget과 Outcome Provider는 구현돼 있지 않다.

미래 Result Flow 원칙:

```text
VehicleHealth OnVehicleDestroyed
→ Player Vehicle Destroyed State
→ 상위 Player/Mission/Session Flow 판단
→ 필요 시 Result Outcome 확정
→ Result Presenter/View Data 생성
→ UISubsystem Screen Layer에 Result Screen 표시
```

Result Screen 최소 의미 계약 후보:

```text
OutcomeTitle
OutcomeSummary
PrimaryActionAvailability
SecondaryActionAvailability
```

- `DEFEAT`, `VICTORY`, `MISSION FAILED` 같은 Title은 Outcome Provider가 공급한다.
- Retry/Continue/Exit 버튼의 존재와 실행 가능 여부도 상위 Flow가 공급한다.
- Result Screen이 World를 Pause할지는 Game Flow가 결정한다. `SetPrimaryScreenWidget` 자체를 World Pause API로 취급하지 않는다.
- Result가 활성화되면 기존 Combat HUD는 Collapse하고 Screen Layer가 Focus/Input을 소유한다.
- Pause Menu를 Result Screen과 동일 화면으로 만들지 않는다. Pause와 Result는 서로 다른 상태/레이어 책임이다.

#### 17.7.21 Respawn / Pawn 교체 경계

현재 `UCFUISubsystem.OnCurrentPawnChanged`는 Possession 변경을 UI에 알릴 수 있다.

후속 Respawn이 구현되면 기본 수명은 다음을 따른다.

```text
Old Pawn Destroyed
→ Terminal Presentation
→ Gameplay Flow가 새 Pawn 생성/소유권 전환
→ OnCurrentPawnChanged(Old, New)
→ Presenter가 Old Health/Defense 이벤트 구독 해제
→ Terminal Damage/Critical 상태 Clear
→ New Pawn View Data로 HUD 재바인딩
```

- UI가 Old Pawn의 `ResetHealthToMaximum()`을 호출해 Respawn을 흉내 내지 않는다.
- `ResetHealthToMaximum`은 현재 Health Runtime의 명시적 초기화/PIE 반복 검증 API이며 Respawn Flow 자체가 아니다.
- 새 Pawn이 준비되지 않았는데 이전 HUD를 정상 Operational 상태로 되돌리지 않는다.
- Respawn 시 Target/Weapon/Defense 상태 복원 여부는 새 Pawn Runtime Snapshot과 Gameplay Flow가 소유한다.

### 17.8 시각 경고와 Alert Feed

Alert Feed는 전투 로그 저장소가 아니라 **현재 플레이어가 즉시 알아야 하는 상태를 제한적으로 표시하는 Presenter 기반 HUD**다. Gameplay System이 위험 상태와 임계값을 판정하고 Presenter가 플레이어용 Alert 수명을 구성하며 Widget은 그 결과를 표시한다.

경고 View Data는 최소 다음 필드를 가진다.

```text
AlertKey
Category
Severity
StartGameTime
Duration
bPersistent
DisplayText
SourceId
```

필드 의미:

```text
AlertKey
→ 같은 의미의 활성 Alert를 안정적으로 식별하는 Semantic Key

Category
→ Vehicle / Weapon / Target / Mission 등 표시·필터링용 의미 범주

Severity
→ Notice / Warning / Critical
→ Widget이 수치 임계값을 보고 재계산하지 않음

StartGameTime
→ 현재 활성 주기가 시작된 Game Time

Duration
→ 비영구 Alert의 표시 유효시간

bPersistent
→ 실제 상태 해제 사건까지 활성 집합에 남아야 하는 상태

DisplayText
→ Provider/Presenter가 만든 Player-facing 짧은 문구

SourceId
→ 같은 Category 안에서 차량·무기 Slot 등 출처를 안정적으로 구분할 필요가 있을 때 사용
```

`AlertKey`와 `SourceId`는 UI 내부 Object Path, Component 이름, Debug 문자열을 그대로 표시하기 위한 값이 아니다. 플레이어에게 보이는 Text와 내부 안정 식별자를 분리한다.

#### Alert 활성 집합과 수명

Presenter는 화면에 보이는 3개 Row만 저장하지 않고 **현재 유효한 Active Alert Set**을 관리한다.

기본 상태 흐름:

```text
Inactive
→ Activated
→ Updated / Escalated
→ Cleared 또는 Expired
→ Inactive
```

- 같은 `AlertKey`가 이미 Active이면 새 Entry를 추가하지 않고 기존 View Data를 갱신한다.
- 같은 Key의 일반 값 갱신은 새로운 Alert 발생으로 취급하지 않는다.
- `Warning → Critical`처럼 Severity가 상승하면 기존 Entry를 승격하고 표시 우선순위를 즉시 다시 평가한다.
- 상태가 실제로 Cleared된 뒤 나중에 같은 Key가 다시 Activated되면 새로운 활성 주기로 취급하고 `StartGameTime`을 새로 시작할 수 있다.
- 같은 위험이 유지되는 동안 매 Tick, 매 프레임 또는 반복 피해 이벤트마다 새로운 Key를 만들어 Alert를 증식시키지 않는다.

#### 표시 선택과 정렬

기본 표시 우선순위는 다음으로 고정한다.

```text
Critical
→ Warning
→ Notice
```

같은 Severity에서는 **현재 활성 주기가 더 최근에 시작된 Alert**를 기본적으로 먼저 표시한다. 단순 값 갱신 때문에 같은 Severity Row의 순서를 계속 뒤집지 않는다.

Presenter는 Active Set 전체에서 우선순위가 높은 최대 3개를 `Visible Alerts`로 제공한다. 3개를 넘은 Alert는 삭제하지 않고 Active Set에 남긴다. 상위 Alert가 Cleared/Expired되면 아직 유효한 다음 Alert가 표시 후보로 올라온다.

비영구 Alert가 보이지 않는 동안 자신의 `Duration`이 끝났다면 나중에 Slot이 비었다는 이유로 오래된 알림을 뒤늦게 재생하지 않는다. Persistent Alert는 실제 상태가 계속 유효한 동안 숨겨져 있어도 다시 표시 후보가 될 수 있다.

`+N` Overflow Count와 Compact 3-Row Geometry 같은 실제 표현 규칙은 `InGameUIStyleSpec.md v0.80.0`이 소유한다.

#### 시간축과 Pause

Alert Duration은 **Game Time**을 기준으로 한다.

```text
World Running
→ Duration 진행

World Paused
→ Duration 정지

Resume
→ 남은 Duration부터 계속
```

싱글플레이 완전 Pause 중 Notice/Warning이 실시간 초 단위로 사라지지 않게 한다. UI Animation이 Pause Menu 표현을 위해 별도 실시간 Tick을 사용하더라도 Alert Gameplay 수명 자체를 Real Time으로 감소시키지 않는다.

Persistent Alert는 `Duration`을 매우 크게 설정하는 편법으로 만들지 않는다. 실제 Gameplay/Presenter의 상태 해제 사건이 수명을 종료해야 한다.

#### 소유 Panel과 중복 제거

Alert Feed는 다른 HUD 모듈이 이미 충분히 전달하는 평상시 상태를 복제하지 않는다.

기본 소유권:

```text
Target Tactical Analysis 완료
→ Target Panel 내부 피드백

Active Scan 신규 Contact
→ Radar 신규 Contact 피드백

Target Last Known / Contact Lost / Destroyed Hold
→ Target Panel + Radar 상태 표현

Ammo / Heat / Charge / Reload의 평상시 변화
→ WeaponPanel

Shield / Armor / Integrity의 평상시 변화
→ VehiclePanel

Mission 목표와 일반 진행 로그
→ MissionSummary
```

위 상태가 실제로 즉각적인 플레이어 대응이 필요한 사건으로 승격될 때만 해당 Gameplay/Presenter가 별도 Alert를 제공한다. 예를 들면 `OVERHEATED`, `WEAPON OFFLINE`, `SHIELD DEPLETED`, `INTEGRITY CRITICAL`, 시간 제한 실패 임박 같은 사건이다.

Widget은 Weapon Heat 수치나 Integrity Ratio를 직접 읽어 `Critical` 임계값을 재계산하지 않는다. 같은 상태를 Target/Radar/Vehicle/Weapon Widget과 Alert Widget이 각각 독립 판정하는 구조를 금지한다.

#### Interaction과 Style 경계

P0 Alert Feed는 비상호작용 HUD다.

- Alert 생성이 Input Mode, Cursor, Focus를 바꾸지 않는다.
- 클릭, 수동 Dismiss, Scroll History를 기본 기능으로 두지 않는다.
- Mission Log 또는 Message History가 필요해지면 별도 Screen/Panel이 소유한다.
- 색상, Font, Icon, Motion, Compact Row 높이, 최대 3개 Geometry, Critical Pulse 범위와 Overflow 표현은 `InGameUIStyleSpec.md v0.80.0`이 소유한다.
- 색상만으로 Severity를 전달하지 않는다.

### 17.9 피팅 호환 계약

공유하는 것은 UI 기반과 정의 타입이며, Runtime HUD 상태와 피팅 편집 상태는 공유하지 않는다.

```text
공유 가능
- UI Root와 Screen Layer
- Screen 전환과 Focus 정책
- IMC_CFUI
- DA_CFUIStyle
- ResourceType / StatType
- 차량·무기·장갑 Definition Data

분리 필수
- InGame HUD View Data / Fitting Preview View Data
- Runtime 상태 / Saved Loadout
- Gameplay Pawn / Garage Preview Actor
- UCFUISubsystem / Loadout 소유 시스템
```

후속 피팅 기능의 기본 흐름은 다음으로 제한한다.

```text
Saved Loadout
→ Fitting Draft
→ Validate / Preview
→ Commit 또는 Cancel
→ 출격 시 Runtime Snapshot
→ Gameplay Runtime Components
```

필수 경계:

- `UCFUISubsystem`은 Loadout, Draft와 저장 데이터를 소유하지 않는다.
- 피팅 계산은 Widget이나 Blueprint 그래프가 아니라 별도 C++ 도메인 서비스가 소유한다.
- 피팅 Widget은 실제 Gameplay Pawn을 직접 수정하지 않는다.
- 전투 중 피팅은 P0에서 허용하지 않는다.
- 피팅 콘텐츠, Loadout 클래스, Preview Actor와 Fitting View Data는 별도 Plan에서 설계한다.
- 실제 피팅 기획을 시작하기 전까지 구체 클래스명과 기능 ID를 확정하지 않는다.

---

## 18. Changelog

### v0.35.0 - 2026-08-14

- TargetPanel의 기존 설계 결정을 실제 C++ 확장에 사용할 구현 준비 ViewData 계약으로 정리했다. 현재 `FCFTargetHUDData v1.0.0` Source는 변경하지 않고 Tracking / Identity / Analysis / Defense / Weapon / Module Sub-ViewData와 additive migration 순서를 정의했다.
- `ECFTargetTrackState`의 Visible/Occluded/Estimated/SignalLost와 Sensor 수명의 Live/LastKnown/DestroyedHold를 별도 상태축으로 분리하고 `ECFTargetKnowledgeState`, `ECFTargetIntelFreshness`, Analysis Category/Progress Revision 후보를 정의했다.
- Target Intelligence의 Unknown/Estimated/Known/Unavailable/NotApplicable, Current/Stale, 정확 Ratio와 질적 Estimated 값을 분리해 UI가 적 차량의 숨겨진 Runtime 수치를 직접 읽거나 추정하지 못하게 했다.
- Shield / 6방향 Armor / Vehicle, Weapon Family Summary, Module Damage Summary의 구현 준비 필드를 기존 Compact Target Panel 시각 계약과 연결했다. Module Runtime 부재 시 Damage Block은 계속 Collapse한다.
- Live → Last Known → Contact Lost와 Destroyed Hold 수명을 ViewData 공급 규칙으로 연결하고 Contact Lost/Destroyed 타이머·판정을 Widget이 소유하지 않도록 했다.
- Radar/Sensor는 Contact Runtime → HUD Adapter/Presentation Resolver → display-ready `FCFRadarHUDData` 흐름으로 정의했다. ContactId와 ResolvedTargetId를 분리해 ID0 Unknown Contact가 Entity Identity 없이도 존재할 수 있게 했다.
- Radar Contact에 Live/LastKnown/DestroyedHold, 정규화 평면 위치·고저차, Display Range 내/외, 선택 Edge Marker와 신규 탐지 `DiscoveryRevision` 후보를 정의했다.
- Radar Scan은 Ready/Scanning/Cooldown/Unavailable과 Progress/Cooldown/ScanCycleRevision으로 분리하고 Sweep을 Scanning 시각 표현으로만 사용하도록 했다.
- `DisplayRangeMeters`와 `MaximumDetectionRangeMeters`를 분리해 단계식 Wheel Zoom이 Sensor 탐지 성능을 변경하지 않게 했고, Range Ring 3개는 Zoom 단계 수가 아니라 현재 표시 범위 25/50/75% 기준임을 명시했다.
- Passive Tracking, Visual Acquisition, 360° Active Scan, Scanner Occlusion, Last Known 만료, Range Preset과 Tactical Analysis Build/Decay는 Sensor/Scanner Gameplay Provider 소유로 유지했다.
- 실제 Sensor/Knowledge Provider가 없으므로 Source·Config·Unreal Asset·Build·Automation은 변경하지 않았고 Production Radar는 계속 Unavailable, Target Intelligence 추가 Block은 비어 있는 상태를 유지한다.

### v0.34.0 - 2026-08-12

- CF-FQ-032 Reticle Layer Global Declutter / Marker Density / 중앙 시야 보호 구조 계약을 추가했다.
- 개별 Widget이 서로를 직접 Tick 조회하는 방식 대신 UI-P0-03 이후 공통 Projection/Presentation Resolver가 Stable Identity Merge, Safe Region, Declutter, Edge Collision을 한 프레임 단위로 해결하도록 했다.
- Declutter는 Presentation-only이며 Target Selection, Lock, Mission Primary, Objective Lifecycle, Damage Result, Knowledge와 Fire 가능 상태를 변경하지 않는다고 명시했다.
- P0 입력 집합을 Command 1 / Turret 1 / Selected Target 0..1 / Primary Objective 0..1 / Lock 0..1 / Immediate State 0..1 / Hit State 0..1 / Incoming 최대 4 Bucket으로 제한했다.
- 미래 Resolved Presentation 의미 데이터와 Full / Compact / ShapeOnly / AttachedBadge / CollapsedBySource 상태를 정의하되 정확한 C++ 타입명은 UI-P0-03에 남겼다.
- Command 위치 기준 Aim Core 56 / Combat Ring 136 / Status Lane 144×24의 중앙 Zone을 구조 계약에 연결했다.
- Provider 검증 → Projection → Identity Merge → Safe Region → Target 고정 → Objective Resolve → Lock 부착 → Text Budget → transient overlay의 결정적 충돌 파이프라인을 정의했다.
- Hit Confirmation과 Incoming Damage를 transient overlay로 분류해 자동사격·연속 피격이 Persistent Target/Objective Marker 위치와 Full/Compact 상태를 매 Event 흔들지 않도록 했다.
- Command Status Lane은 Immediate Fire State 1줄 전용으로, Selected Target 주변 Auxiliary도 최대 1줄로 제한하고 Lock Failure/Progress → Target State → Distance 순으로 축약하도록 했다.
- Objective는 Combat Ring에서 Distance부터 줄이고 Aim Core에서는 ShapeOnly를 기본으로 하며 동일 Target Entity는 AttachedBadge를 유지하도록 했다.
- Incoming Direction Arc 최대 4개는 유지하면서 TOP/BOTTOM Micro Label은 가장 깊은 Layer·최신 Event 기준 최대 1개만 표시하도록 했다.
- Edge Density를 Target 0..1 + Objective 0..1로 유지하고 기존 Target 고정 / Objective 40 간격·최대 56 Shift 규칙을 통합했다.
- Collision Enter 8 / Release 16 / Auxiliary Fade 0.10초 Hysteresis를 정의해 경계 떨림을 방지하고 Scale Pop·위치 Spring·Sound를 금지했다.
- Density/Accessibility가 달라도 Semantic 우선순위와 Stable Identity Merge, transient reflow 금지 규칙을 유지하도록 했다.
- Mission/Lock Runtime Provider 부재를 유지하고 Declutter 설계를 이유로 가짜 Runtime을 만들지 않도록 했다.
- D1-11 New Production User Preview Pending / D1-11 NOT PASS, D1-12 Not Started, UI-P0-03 Not Started 상태는 변경하지 않았다.

### v0.33.0 - 2026-08-12

- CF-FQ-032 Objective World Marker / Mission Navigation 구조 계약을 MissionSummary와 분리해 추가했다.
- 실제 Source 조사에서 Mission/Objective Runtime Provider·Manager·Registry가 없고 `MissionSummary`·`ReticleLayer` Layout Slot만 존재함을 다시 확인해 실제 Gameplay에서는 MissionSummary와 Objective Marker를 Collapse하고 Visual Mock만 허용하는 경계를 유지했다.
- MissionSummary 의미 데이터에 `PrimaryObjectiveIdentity`를 추가해 Summary와 World Marker가 같은 Primary를 소비하도록 연결하고 내부 Actor 이름/Object Path와 분리했다.
- P0 Navigation은 Provider가 명시한 Primary Objective 1개만 표시하며 `AdditionalObjectiveCount`와 `+N OBJECTIVES`에서 Secondary World Marker를 생성하지 않도록 했다.
- 미래 Objective Navigation 의미 데이터로 Player-facing PresentationWorldAnchor, AnchorPrecision, NavigationVisibilityPolicy, DistanceText와 선택적 AssociatedTargetIdentity를 정의하되 실제 C++ enum/class는 Mission 기능에 남겼다.
- Exact / Approximate / No Anchor를 구분해 Mission이 공개하지 않은 실제 Actor 위치를 Widget·TargetSelect·Sensor에서 역으로 찾아 정확한 Marker로 만들지 못하게 했다.
- Objective는 Target과 같은 Camera View Space/Safe Region 투영을 재사용하지만 On-screen Open Hex Beacon, Off-screen Open Edge Hex로 시각 의미를 분리하도록 했다.
- 거리 Text는 Provider/Presenter Player-facing 값으로 제한하고 Approximate 위치에서 정확한 Meter 값을 UI가 생성하지 않도록 했다.
- Persistent HUD 예약 영역은 Navigation Safe Region으로 피하고 Panel 자체를 이동시키지 않도록 했다.
- 같은 Objective/Selected Target Entity는 Target Bracket + 작은 Objective Mission Badge로 결합하고 Stable Identity 없이 Screen 좌표만으로 병합하지 않도록 했다.
- 서로 다른 Marker 충돌은 `Objective < Selected Target < Command/Turret Reticle` Draw Order와 Objective 보조 Text 우선 Collapse로 해결하며 World Anchor를 거짓 이동하지 않는다.
- Edge 충돌은 Selected Target 위치를 고정하고 Objective만 Tangential 방향 최대 56 Design Unit 이동해 기본 40 간격을 확보하도록 했다.
- Objective Navigation과 Radar Sensor Contact를 분리하고 미래 Radar Mission Overlay를 별도 Channel로 남겼다.
- Player Destroyed, Pause, Objective 완료/실패와 Result 수명은 기존 Player/Mission/Outcome Provider 책임을 유지하며 Objective Widget Timer와 사운드 생성을 금지했다.
- Objective는 새 고정 HUD Slot을 요구하지 않고 현재 Stretch ReticleLayer/Game Layer 의미 하위 Projection 요소로 구현할 수 있게 설계해 D1-11 Production Asset Gate를 보호했다.

### v0.32.0 - 2026-08-12

- CF-FQ-032 Player Integrity Critical → Vehicle Destroyed → 미래 Respawn/Spectate/Result 구조 계약을 현재 VehicleHealth·Launcher·CombatFx·Camera·UISubsystem Runtime과 교차검증해 추가했다.
- `Critical != Destroyed`, `Destroyed != Defeat/Result`를 불변 계약으로 고정하고 VehicleHealth의 OnVehicleDestroyed가 Mission/Game Outcome을 직접 결정하지 않도록 했다.
- 현재 VehicleHealth는 bDestroyed/1회 OnVehicleDestroyed만 소유하고 입력·물리·메쉬·Possession·Respawn·Result를 자동 처리하지 않는 실제 구현 경계를 기록했다.
- CombatFx의 1회 Destroyed FX, Launcher의 OwnerDestroyed 시퀀스 취소, Destroyed 차량 TargetSelect Invalid 동작을 현재 후속 반응으로 확인했다.
- Camera에는 Destroyed/Spectate 모드 타입이 있지만 Health 상태와 자동 연결되지 않았고 UISubsystem에는 Screen Layer 진입점만 있으며 Result Runtime은 없음을 명시했다.
- Integrity Critical은 Presenter 위험 상태로 정의하고 기본 Enter 25% / Exit 30% Hysteresis를 조정 가능한 Presentation Preset으로 제안했다. Widget 자체 임계값 계산은 금지했다.
- Player Destroyed Presentation이 실제 적용될 때 필요한 의미 데이터를 PlayerVehicleLifeState / Combat·Vehicle Input Availability / HUD Visibility / Outcome State로 분리했다.
- 현재 Fire/Drive 입력에는 Health Destroyed 차단이 없는 통합 공백을 확인하고 UI만 Reticle·Weapon을 숨기는 가짜 조작불가 상태를 금지했다. 이 항목은 D1-11이 아니라 UI-P0-03 Runtime Integration 선행조건이다.
- Terminal 상태에서 VehiclePanel은 최종 실제 방어값과 DESTROYED를 유지하고 실제 Combat Availability=false일 때만 Reticle·Lock·Target Marker·Weapon·Radar를 Fade/Collapse하도록 했다.
- Player Destroyed Hold와 Target Destroyed 1.25초 Hold를 분리하고 Player Hold 수명은 상위 Outcome Flow가 소유하도록 했다.
- 미래 Result는 VehicleHealth가 아니라 Player/Mission/Session Flow → Result Presenter → UISubsystem Screen Layer 순으로 진입하며 FullScreen UI와 World Pause를 같은 상태로 취급하지 않도록 했다.
- Respawn은 새 Pawn Possession과 OnCurrentPawnChanged를 통해 HUD를 재바인딩하며 UI가 Old Pawn ResetHealthToMaximum을 Respawn API처럼 호출하지 않도록 했다.

### v0.31.0 - 2026-08-12

- 현재 HitDamage·DamageHitContext·VehicleDefense·FireFeedback Runtime을 교차검증한 뒤 Incoming Damage Direction, Outgoing Hit Confirmation과 Local Damage Feedback 구조 계약을 추가했다.
- `FireSuccess=발사 승인`과 `Hit Confirmation=실제 방어층 감소`를 분리해 기존 FireFeedback 의미를 보존했다.
- `FCFDamageHitContext.IncomingDirection / InstigatorActor`와 `FCFVehicleDamageResult.bAppliedToAnyLayer / ArmorDirection / Layer Result`를 Damage Presentation의 실제 Source of Truth로 고정했다.
- 현재 `OnVehicleDamageResolved`에는 HitContext가 없고 `OnVehicleDamaged`는 Integrity 피해에만 발생하므로 Shield/Armor-only 피격 방향을 제품 HUD에 정확히 전달하려면 UI-P0-03에서 HitContext + VehicleDamageResult 결합 Event/Adapter가 필요함을 명시했다.
- 공격자 HitScan/Projectile 경로도 현재 Integrity 호환 결과만 Pawn에 전달하는 부분이 있어 Shield/Armor-only Outgoing Hit Confirmation을 위해 전체 VehicleDamageResult 전달이 필요함을 기록했다.
- Incoming 방향은 `-IncomingDirection`을 우선 Source Direction으로 사용하고 유효하지 않을 때만 Instigator→HitActor 방향을 Fallback으로 사용하도록 했다. Actor Identity는 방향 계산 외 Player-facing 정보로 노출하지 않는다.
- Camera-relative 방위와 차량 Local `ArmorDirection`을 분리해 Direction Arc는 화면 방위를, VehiclePanel은 실제 Front/Left/Right/Rear/Top/Bottom Plate 피해를 표현하도록 했다.
- VehicleDamageResult의 실제 적용량으로 DeepestAffectedLayer를 Shield→Armor→Integrity 순서로 분류하고 `bAppliedToAnyLayer=false` 사건은 Damage Hit Confirmation에서 제외했다.
- 일반 타격은 Alert로 중복하지 않고 Direction / VehiclePanel / Alert의 역할을 분리했다.
- Target Intelligence와 Hit Confirmation을 분리하고 Floating Damage Number를 P0 기본 HUD에서 사용하지 않도록 했다.
- Damage Feedback Duration과 중복 억제를 Game Time / DamageEventId 기준으로 설계하고 CF-PDL-0009 사운드 비지원 정책을 유지했다.

### v0.30.0 - 2026-08-12

- 이전 세션에서 발견된 동기화 누락을 먼저 교정하고 HUD 상세 설계를 Reticle Layer → World Target Marker → Lock/Fire-Control 순서로 진행했다.
- 문서 헤더를 실제 CF-FQ-032 체크포인트인 D1-07~10 PASS, D1-11 Production Rework Automated PASS·신규 1920×1080 User Preview Pending, D1-11 NOT PASS, AssetizationSpec v0.11.0 기준으로 동기화했다.
- Mission Runtime Provider 부재를 명시하고 MissionSummary를 Provider 기반 미래 계약으로 추가했다. Runtime Provider 연결 전 실제 Gameplay에서는 Collapse하며 Mock Data는 Visual Prototype 증거로만 제한했다.
- Vehicle HUD의 구형 `560×416 / D·R·N 중심` 요약을 `896×416`, 비대칭 실제 RPM Gauge, 3자리 속도, 단일 Gear Slot, 6방향 Armor 세로 Bar, Shield/Integrity 전체 폭 Bar 계약으로 교정했다.
- Project 전역 `CF-PDL-0009`와 충돌하던 현재 Tactical Analysis 완료음 문구를 제거하고 시각 완료 피드백 전용으로 교정했다. 과거 Changelog의 당시 기록은 역사 이력으로 보존한다.
- Reticle Layer를 Command Reticle / CurrentMuzzleDirection 기반 Turret Reticle / World Target Marker / 미래 Lock Indicator / Immediate Fire State의 독립 의미축으로 정의했다.
- 기존 CF-FQ-025 이중 레티클과 CF-FQ-017 FireFeedback 의미를 보존하고 Weapon Preview를 착탄점 Reticle로 재해석하지 않도록 했다.
- World Target Marker의 OnScreen / OffScreen / BehindCamera / Occluded / Last Known / Contact Lost / Destroyed Hold 투영·수명 경계를 정의했다.
- 화면 밖 Marker는 View Space → Safe Region 경계 교차 방식으로 계산하고 정확한 X-ray Marker를 UI가 임의 생성하지 않도록 했다.
- 현재 별도 Lock Runtime이 없음을 확인해 Lock UI를 미래 Provider 계약으로 제한하고 Selected/ID2/Scanner/Reticle 정렬만으로 가짜 Lock Progress·성공을 만들지 않도록 했다.
- Lock과 Selected Target, Command Aim, Turret Aim, 실제 Fire Validation의 권한을 분리하고 Last Known/Destroyed UI 선택 유지가 Lock·발사 유효시간을 연장하지 않도록 했다.

### v0.28.0 - 2026-08-12

- HUD 상세 설계의 다음 우선순위인 Alert Feed 구조 계약을 `InGameUIStyleSpec.md v0.80.0`과 동기화했다.
- Alert Feed를 Gameplay 판정이 아닌 Presenter 기반 Active Alert Set으로 정의하고 Widget의 위험 임계값 재계산을 금지했다.
- `AlertKey` 제자리 갱신, Severity 상승 시 승격, Clear 뒤 재활성화와 매 Tick 중복 생성 금지 수명을 확정했다.
- 표시 우선순위를 `Critical → Warning → Notice`, 같은 Severity는 현재 활성 주기 시작 시각 기준으로 정의하고 최대 3개 표시와 숨겨진 Active Set을 분리했다.
- 비표시 중 만료된 일회성 Alert는 뒤늦게 재생하지 않고 Persistent Alert만 실제 상태가 유지되는 동안 다시 표시 후보가 될 수 있게 했다.
- Alert Duration을 Game Time 기준으로 고정해 싱글플레이 World Pause 중 수명이 감소하지 않도록 했다.
- Target/Radar/Weapon/Vehicle/Mission의 평상시 상태 표현과 Alert Feed의 중복 생성을 금지하고 실제 대응 필요 사건만 Provider가 Alert로 승격하도록 했다.
- P0 Alert Feed를 비상호작용 HUD로 유지하고 Focus·Cursor·Input Mode 변경과 Scroll History를 제외했다.
- 실제 Compact Row Geometry·Icon·Motion·Overflow 표현은 StyleSpec v0.80.0이 소유하도록 문서 역할을 분리했다.

### v0.27.0 - 2026-08-12

- Destroyed Target Lifecycle을 Contact Lost와 분리하고 Gameplay/신뢰 가능한 Provider의 명시적 Destroyed Known에서만 진입하도록 했다.
- 기본 1.25초 Kill Confirmation Hold 동안 선택을 유지한 뒤 자동 선택 해제하도록 했다.
- 파괴 진입 즉시 Identification/Tactical Analysis를 종료하고 미완료 Progress/다음 Category/가짜 완료 피드백을 중단하도록 했다.
- Target Panel은 `Identity + DESTROYED + 유효 거리` 중심 Compact Terminal State, Radar는 기존 Blip/Bracket의 짧은 저강도 유지로 표현하도록 했다.
- Hold 종료 시 `TARGET LOST` 없이 `DESTROYED → NO TARGET`으로 전환하고 다음 Target 자동 선택을 금지했다.
- Hold 중 플레이어가 다른 Live Target을 선택하면 즉시 전환할 수 있게 했다.
- UI Hold와 Fire-Control 유효성을 분리하고 Destroyed 결과 Knowledge Memory는 선택 수명과 별도로 보존 가능하게 했다.

### v0.26.0 - 2026-08-12

- Target Panel의 Live → Last Known → Contact Lost 상태 전환과 Target Selection 수명 규칙을 확정했다.
- 선택 Target은 Last Known 동안 유지하고 `LAST KNOWN`, `LAST POS`, 선택적 `LAST SPEED/Stale` 표현을 사용하도록 했다.
- Last Known에서는 Identity/정적 Intel은 보존하고 동적 Intel은 Stale, 신규 Analysis는 금지하며 기존 미완료 Progress만 Decay하도록 했다.
- Last Known Selected Target 유지와 Weapon/Lock Fire-Control 유효성을 분리해 UI 선택이 실시간 Aim/Lock 권한을 만들지 않도록 했다.
- Last Known 기간 내 재획득 시 별도 재선택 없이 Live로 복귀하고 남은 Analysis를 재개하도록 했다.
- Contact Lost는 Sensor/Tracking Provider가 소유하고 선택 Target이면 자동 해제 후 짧은 `TARGET LOST → NO TARGET` 전환을 사용하도록 했다.
- Contact Lost 이후 자동 다음 Target 선택과 자동 재선택을 금지하고 수동 해제/변경이 Knowledge Memory를 삭제하지 않도록 했다.
- P0 신규 Target Cycle은 Live Contact 우선으로 두고 Last Known Ghost의 신규 자동 선택은 후속 Inspect/Command 기능으로 분리했다.
- Destroyed와 Contact Lost를 별도 상태로 유지했다.

### v0.25.0 - 2026-08-12

- Target Panel의 폭 384를 유지하고 2560×1440 기준 기본 HUD 최대 높이를 640으로 제한하도록 Density 계약을 확정했다.
- Block 순서를 `Header/Identity → Tracking → Analysis → Defense → Weapon → Damage`로 고정했다.
- Header/Relationship, 핵심 Tracking과 현재 Analysis Progress를 Pinned 정보로 정의했다.
- ScrollBox·자동 Carousel·Page Rotation을 기본 전투 HUD에서 사용하지 않도록 했다.
- 높이 부족 시 Secondary Detail, Weapon/Damage Overflow, Compact Armor/Bar 순으로 Presentation을 압축하고 Knowledge는 보존하도록 했다.
- Armor 6방향과 Shield/Vehicle 독립 상태는 Density 때문에 평균값으로 합치지 않도록 했다.
- 긴 Localization Text는 Short Display Name/Ellipsis를 사용하고 Font 과축소·내부 Asset Name 대체를 금지했다.
- 남는 상세 Overflow는 `+N` Summary를 유지한 채 후속 Expanded Target Detail로 넘기도록 했다.

### v0.24.0 - 2026-08-12

- 현재 Module Damage Runtime 부재 시 Target Damage Block을 Collapse하고 Provider 없는 Category는 가짜 분석 없이 Unavailable Skip하도록 했다.
- 후속 Module Damage를 `Module Name + Severity + Functional State` Compact Summary로 정의하고 손상/기능 이상 Module만 기본 표시하도록 했다.
- Engine·Wheel·Turret 후속 확장 예시를 추가하되 실제 Runtime enum·임계값·휠 개수는 UI 문서가 하드코딩하지 않도록 했다.
- Damage Summary를 최대 3 Row, 4개 이상은 2 Row + `+N DAMAGED` Overflow로 축약하도록 했다.
- 전체 Coverage가 Known일 때만 `MODULES · NOMINAL`을 허용하고 부분 분석 상태에서 전체 정상 판정을 만들지 않도록 했다.
- Module Damage를 동적 Intel로 취급해 Stale·Repair 복구와 Row 제거 규칙을 연결했다.
- 정확한 Module HP·Damage Formula·Damage Log·Weak Point·Repair ETA는 Compact Target Panel에서 제외했다.

### v0.23.0 - 2026-08-12

- Target Weapon Intelligence를 개별 Weapon Instance 목록이 아닌 Weapon Family 기준 Compact Summary로 확정했다.
- 기본 Row를 `Family/Display Type + Count + 선택적 Mount Summary`로 두고 정확한 Count/Mount는 실제 Intelligence Source가 제공할 때만 표시하도록 했다.
- Weapon Family Summary는 최대 3 Row, 4개 이상은 앞의 2개 + `+N TYPES` Overflow로 축약하도록 했다.
- Provider의 안정적인 Display Order를 사용하고 Widget이 자체 위협도 계산으로 Row를 재정렬하지 않도록 했다.
- Known Weapon Loadout 정보는 P0에서 정적 Knowledge로 보존하고 Last Known 전환만으로 Stale 처리하지 않도록 했다.
- 적 Target의 Ammo/Heat/Charge/Cooldown/Reload 등 내부 운용 상태를 Weapon Summary에 추가하지 않는 계약을 유지했다.

### v0.22.0 - 2026-08-12

- Target Panel의 Shield/Armor/Vehicle Defense Intelligence 실제 배치를 Compact Intelligence Card 구조로 확정했다.
- Shield와 Vehicle Integrity는 얇은 1행 상태 Bar, Armor는 `FRONT|REAR / LEFT|RIGHT / TOP|BOTTOM` 고정 2열×3행 Grid로 정의했다.
- Target Panel에서 VehiclePanel의 차량 실루엣·Armor Plate·Current/Maximum 숫자를 축소 복제하지 않도록 했다.
- Armor Block 활성화 후 미확인 방향은 `???` Cell로 유지하고 Known Ratio에서만 Percent/Micro Bar를 기본 표시하도록 했다.
- Estimated 정보는 Source가 제공한 질적 상태/추정 범위만 사용하고 가짜 정밀 Percent를 생성하지 않도록 했다.
- Defense Display 순서를 `Shield → Armor → Vehicle Integrity`로 고정하되 Scanner 분석 Priority와 분리했다.
- Dynamic Defense Intel의 Stale 상태는 마지막 값을 유지하면서 낮은 시각 강도와 비색상 상태 의미를 제공하도록 했다.

### v0.21.0 - 2026-08-12

- Tactical Analysis Category 완료 피드백을 개별 Field가 아닌 Category당 1회로 정의했다.
- 새 정보는 Target Panel 내부에서만 짧은 Brightness/Opacity Highlight를 사용하고 Panel 전체·Radar·Target Bracket·Alert Feed에는 완료 효과를 중복 전파하지 않도록 했다.
- 완료음은 짧고 낮은 우선순위의 `ScannerAnalysisComplete` Semantic UI Sound Event 1회로 두고 실제 Sound Asset은 후속 Audio Style이 소유하도록 했다.
- 자동 Skip에는 완료 피드백을 내지 않고 실제 Intelligence 획득/승격에만 적용하도록 했다.
- 완료 연출이 Scanner Gameplay를 지연하지 않으며 마지막 Category에도 별도 Banner/추가 완료음을 사용하지 않도록 했다.

### v0.20.0 - 2026-08-12

- Tactical Analysis 기본 Priority를 `Shield → Weapon → Armor → Vehicle → Damage/Module`로 확정하고 한 번에 하나의 Category를 순차 분석하도록 했다.
- Priority를 Scanner/Tactical Analysis Profile Override 대상으로 두고 Capability 없음·NotApplicable/Unavailable·Known Category는 자동 Skip하도록 했다.
- Estimated Category는 Known 승격 대상으로 유지하고 완료된 동적 Intel Refresh는 초기 획득 Queue와 분리했다.
- 미완료 Progress가 남은 Category는 조건 재획득 시 우선 재개하고, 0까지 감쇠한 경우에만 Priority를 재평가하도록 했다.
- Target 전환 시 이전 Target의 미완료 Progress는 Decay하고 새 Target은 독립 Queue를 시작하도록 했다.
- 기본 P0에서는 수동 Category 선택과 Queue 편집을 요구하지 않도록 했다.

### v0.19.0 - 2026-08-12

- Tactical Analysis의 Build/Decay 속도를 분리된 Scanner/Profile 조정축으로 정의하고 기본적으로 Decay가 Build보다 느리게 튜닝하도록 했다.
- 고성능 Scanner가 빠른 Build, 느린 Decay, 긴 Range와 넓은 Category Capability를 독립적으로 가질 수 있게 했다.
- Target Panel에는 현재 우선 분석 Category 하나의 Compact Progress만 표시하고 감쇠 중에는 같은 Bar가 역방향 감소하도록 했다.
- 감쇠 상태는 `INTERRUPTED` 계열 짧은 Text와 낮은 명도로 구분하고 Blink/Pulse/경고 Flash는 사용하지 않도록 했다.

### v0.18.0 - 2026-08-12

- 사용자 결정으로 Tactical Analysis 도중 Range/LOS/Occlusion 조건을 잃었을 때 미완료 진행률을 즉시 초기화하지 않고 서서히 감소하도록 했다.
- 조건 재확보 시 남은 진행률에서 분석을 재개하고, 진행률이 0까지 감쇠된 경우에만 처음부터 다시 시작하도록 했다.
- 감쇠 속도는 Scanner/Tactical Analysis Profile 조정값으로 두고 이미 Known으로 확정된 Intelligence와 분리했다.

### v0.17.0 - 2026-08-12

- ID2 이후 Target Intelligence를 Shield/Armor/Vehicle/Weapon/Damage 범주별 Knowledge State로 분리했다.
- `Unknown / Estimated / Known / Unavailable`과 별도 `Current / Stale` Freshness 개념을 도입해 정보 신뢰도와 최신성을 분리했다.
- Visual/Combat Observation, Mission/Database/Prior Intel, Scanner Tactical Analysis와 후속 Friendly Telemetry를 정보 Source로 구분했다.
- Scanner Tactical Analysis는 ID2 이후 선택 Target에 자동 연속되며 Live Contact·Analysis Range·LOS/Occlusion·Category Capability 조건을 요구하도록 했다.
- Shield·Armor·Vehicle은 적 Target에서 내부 절대 HP보다 Ratio/Percent 중심으로 공개하고, 방향별 Armor Knowledge를 보존하도록 했다.
- Weapon은 실제 장착 Type/Family·개수·Mount까지 분석 가능하되 적의 Ammo/Heat/Charge/Cooldown/Reload 내부 운영 상태는 기본 공개에서 제외했다.
- 현재 Module Damage Runtime 부재 상태에서는 부품 손상을 추정 생성하지 않고 후속 Runtime/Scanner Capability로 확장하도록 했다.
- 정적 Intel과 동적 상태의 보존 정책을 분리해 Live 분석이 끊긴 동적 값은 Stale로 유지하고 실시간처럼 갱신하지 않도록 했다.

### v0.16.0 - 2026-08-12

- Contact Identification 확정안과 동기화해 Target Panel을 `Tracking / Identification / Target Intelligence`의 분리된 정보축으로 재정의했다.
- ID0/ID1/ID2 예시를 추가하고 거리·속도 같은 Tracking 정보가 Identity 공개와 독립임을 명시했다.
- ID2가 SHIELD/ARMOR/VEHICLE 수치와 무장·손상 정보를 자동 공개하지 않도록 하고 별도 Target Intelligence에 책임을 분리했다.
- 동일 의미 행의 상태 변화는 Slot 위치를 유지하고, 아직 활성화되지 않은 상세 Intelligence Block은 Collapse하도록 Panel 안정성 규칙을 정리했다.
- 상세 공개 범위와 Visual Layout의 SSOT를 `InGameUIStyleSpec.md`로 명확히 연결했다.

### v0.15.0 - 2026-08-07

- D1-09A~12의 정확한 구현 파일·Asset·Widget Tree·검증·중단 조건을 `InGameUIAssetizationSpec.md v0.1.0`으로 분리했다.
- Pretendard와 IBM Plex Mono의 OFL 1.1 상용 게임 번들 사용 적격성을 PASS로 확정하고 Font Asset은 `UFont` Runtime Composite + `UFontFace` 구조를 사용하도록 연결했다.
- C++은 Style/Density/Layout 해석과 Visual Context/Focus Bridge를 소유하고 Blueprint는 Base Widget·HUD Prototype·Pause Visual을 소유하는 역할 분리를 구체화했다.
- D1-11은 실제 Gameplay HUD가 아닌 1080p Visual Contract Prototype, D1-12는 Native Pause Fallback을 유지하는 Visual Migration으로 범위를 고정했다.
- Source·Config·Unreal Asset·Build·Automation·PIE·commit·push는 수행하지 않았다.

### v0.14.0 - 2026-08-07

- D1-09A~12의 실행 상세는 `InGameUIAssetizationSpec.md v0.1.0`으로 분리해 잠갔고 `InGameUIStyleSpec.md v0.17.0`은 공통 Style/Token SSOT 역할을 유지한다.
- 실제 코드 조사에서 확인된 Font Asset Binding·HUD Layout·Density 타입 누락을 D1-09A 직접 선행조건으로 기록하고 별도 기능으로 확장하지 않도록 했다.
- Widget 개별 콘텐츠 경로 Load 대신 `UCFUISubsystem`의 Config 기반 Soft Reference 해석 → 자식 Widget 전달 경로를 디자인 계약으로 추가했다.
- D1-11은 기능 HUD가 아닌 1080p Layout/Style Visual Prototype, D1-12는 기존 Pause 기능을 보존하는 Blueprint Visual Migration으로 범위를 제한했다.
- Source·Config·Unreal Asset·Build·Automation은 추가 실행하지 않았고 commit·push도 수행하지 않았다.

### v0.13.0 - 2026-08-07

- 사용자 결정에 따라 커스터마이징 가능 구조를 `WeaponPanel` 한정이 아닌 CarFight 전체 UI의 불변 설계 계약으로 승격했다.
- Gameplay System → Presenter/ViewModel → Blueprint Visual Widget → Style·Layout·Density Data Asset → User Override의 소유권 계층을 확정했다.
- Root·기능 Widget 분리, 안정적인 PanelId·SlotId, Visual Widget Class 교체, 자동 Layout과 빈 공간 Collapse를 필수 구조로 기록했다.
- 색상·폰트·간격·크기·위치·Motion 하드코딩을 제한하고 명시적 Override와 기본 Preset 상속·복제 경로를 요구했다.
- Compact·Standard·Expanded·Custom Density와 16:9·21:9·32:9 Layout Profile, 후속 사용자 HUD 편집·저장·마이그레이션 확장 경계를 정의했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.12.0 - 2026-08-07

- VehiclePanel 상세 소유 문서와 560×416 Wireframe을 설계 문서에 연결했다.
- Speed, D/R/N, Drive State, Handbrake와 실제 Gear Data 미지원 경계를 기록했다.
- Shield, 6방향 Armor와 Integrity의 독립 View Data와 표시 도형을 정의했다.
- 현재 부품 손상 Runtime 미구현 상태와 후속 손상 항목 최대 3개 표시 규칙을 기록했다.
- VehiclePanel은 Draft / User Review Pending, WeaponPanel은 Pending으로 유지했다.
- Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.11.0 - 2026-08-07

- 외부 3인칭 차량 TPS와 16:9 기준 HUD 배치를 승인 상태로 반영했다.
- Mission TL, Alert TC, Target TR, Vehicle BL, Radar BC, Weapon BR, Reticle Center Slot을 확정했다.
- 내 차량 정보와 무기창은 Slot 위치와 내부 정보 구조를 분리해 각각 별도 상세 설계 Pending으로 유지했다.
- `ECFHUDSlotId`, `FCFHUDSlotLayout`, `UCFHUDLayoutData`와 기본 Layout Profile 기반의 후속 사용자 배치 커스터마이징 확장 경계를 추가했다.
- 현재 P0에서는 사용자 배치 편집·저장 기능을 구현하지 않는다.
- CommonUI 판단은 다루지 않았고 Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.10.0 - 2026-08-06

- `InGameUIStyleSpec.md v0.1.0`을 UI 제작 규격 소유 문서로 연결했다.
- DA_CFUIStyle Token, Base Widget 5종과 2560×1440 Wireframe의 설계 소유권을 분리했다.
- `ConceptArt/CFHUDWireframe_1440p.xml`을 1440p 좌표·Anchor 검토 기준으로 연결했다.
- Style Spec과 Wireframe은 User Review Pending이며 Unreal Asset과 실제 화면 검증 전에는 UI-DESIGN-GATE PASS로 판정하지 않는다.
- CommonUI 판단은 다루지 않았고 Source·Unreal Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.9.0 - 2026-08-06

- 디자인 무게 C형 70:20:10 균형의 사용자 승인을 반영했다.
- 차량 탑재형 전술 인터페이스, 평면 스타일과 Palette를 포함한 Visual Direction을 Accepted로 기록했다.
- Style Data·Base Widget·Font·Icon·Wireframe과 실제 해상도 검토는 Pending으로 유지해 UI-DESIGN-GATE PASS를 보류했다.
- CommonUI Gate와 시각 디자인 Gate의 독립성을 유지했다.
- 문서만 수정했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.8.0 - 2026-08-06

- 차량 탑재형 전술 인터페이스, 스타일라이즈드 평면 UI와 Cyan 중심 Palette 방향의 사용자 승인을 반영했다.
- 디자인 무게 A·B·C는 미결정으로 유지하고 최종 Accepted 판정을 보류했다.
- UI-DESIGN-GATE와 CommonUI Gate의 독립성을 유지했다.
- 문서만 수정했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.7.0 - 2026-08-06

- 상세 시각 콘셉트 소유 문서로 `InGameUIVisualConcept.md v0.1.0`을 연결했다.
- `차량 탑재형 전술 인터페이스` 콘셉트와 형태·재질·색상·Typography·Motion·HUD 요소별 디자인 초안이 Working Draft임을 명시했다.
- UI-DESIGN-GATE는 사용자 검토 Pending이며 최종 Style Asset·Wireframe·Palette 확정 전에는 PASS로 판정하지 않는다.
- CommonUI Gate와 시각 콘셉트 Gate의 독립성을 유지했다.
- 문서만 수정했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.6.0 - 2026-08-06

- UI 디자인 품질과 CommonUI 프레임워크 도입을 독립된 설계 계약으로 분리했다.
- `UI-DESIGN-GATE`의 Clean Prototype 기준, `DA_CFUIStyle`, 공통 Base Widget 후보와 C++ View Data·Blueprint 표현 책임을 정의했다.
- 에셋 없는 C++ WidgetTree는 기능·수명 검증용이며 최종 시각 품질 기준이 아님을 명시했다.
- `UI-COMMONUI-GATE`의 목적, 도입 판단 기준, UMG 유지·혼합 구조·확장 선택지를 정의했다.
- CommonUI는 HUD 미감 개선 도구가 아니며, 상시 전투 HUD를 Activatable Widget으로 일괄 변환하지 않는다고 확정했다.
- CommonUI 도입 시에도 UCFUISubsystem 진입점, View Data·Presenter와 Gameplay/UI 책임 분리를 유지하도록 Migration 불변 조건을 추가했다.
- 문서만 수정했으며 Source·Asset·Build·Automation·commit·push는 수행하지 않았다.

### v0.5.0 - 2026-08-01

- UI-P0-02의 실제 `SetPause` 기반 싱글플레이 Pause 흐름을 구현 상태로 갱신했다.
- PlayerController, UISubsystem, VehiclePawn과 VehicleDriveComp의 Pause 책임을 분리했다.
- 차량 Drive·Look 입력과 Controller 눌린 키 상태의 중립화 순서를 확정했다.
- C++ `UCFPauseMenuWidget`의 Menu 레이어 단일 수명, Continue·Pause·Back 해제와 Focus 계약을 기록했다.
- Launcher Ripple·Salvo, Projectile와 Motor 상태를 취소하지 않고 World Pause로 보존·재개하는 정책을 확정했다.
- 게임플레이 Tick과 World TimerManager가 Pause 진행 정지의 소유자임을 기록했다.
- 공식 Editor Build와 전체 CarFight Automation 32/32 Success를 기록했다.
- 실제 사용자 PIE와 Blueprint Pause Menu Asset은 미완료로 분리했다.

### v0.4.0 - 2026-08-01

- `ACFPlayerController`의 Controller 소유 Mapping Context, Pause·Back 요청과 Possession 변경 통지 계약을 실제 구현했다.
- Context 에셋이 비어 있을 때 기존 Pawn Gameplay 입력을 유지하고 System 입력은 C++ Fallback Key로 처리하도록 했다.
- UI 입력 전환은 기존 Pawn Binding과 Mapping Context를 삭제하지 않고 일시 억제·복원한다.
- `UCFUISubsystem`이 LocalPlayer 수명을, `UCFUIRootWidget`이 World 수명을 소유하도록 구현했다.
- Root는 C++ `UCanvasPanel` 기반으로 Game·HUD·Screen·Panel·Menu·Modal·System·Debug 8개 레이어와 명시적 ZOrder를 제공한다.
- World Cleanup에서 이전 Root와 Pawn 약한 참조를 제거하도록 했다.
- Screen·Modal 전환과 Input Mode·Cursor·Focus 기반을 구현했다.
- 실제 Full Pause, Pause Menu Asset과 기존 HUD 이전은 구현하지 않았다.
- 공식 Editor Build와 전체 CarFight Automation 29/29 Success를 기록했다.

### v0.3.0 - 2026-07-31

- 주요 전체 화면을 위한 Screen Layer를 UI Root 계약에 추가했다.
- 피팅과 공유할 UI 기반·표현 타입과 분리해야 할 Runtime·Loadout 데이터를 구분했다.
- `UCFUISubsystem`이 Loadout을 소유하지 않으며 피팅 Widget이 Gameplay Pawn을 직접 수정하지 않는다고 확정했다.
- 후속 피팅 흐름을 Saved Loadout → Draft → Validate/Preview → Commit/Cancel → Runtime Snapshot으로 정의했다.
- 피팅 콘텐츠와 구체 클래스는 별도 Plan에서 기획하도록 범위를 분리했다.

### v0.2.0 - 2026-07-31

- 최소 `ACFPlayerController`와 World별 Root 수명 계약을 확정했다.
- UMG 기반 P0를 유지하고 CommonUI 도입을 후속 재검토로 확정했다.
- Mapping Context, Pause 입력 잔류, Launcher 상태 보존과 기존 HUD 단계 이전 계약을 추가했다.
- `Unknown`, `Unavailable`, 실제 0과 Target Knowledge 공개 경계를 구체화했다.
- 이벤트·프레임·제한 주기 갱신, View Space 화면 밖 투영과 Alert 중복 제거를 확정했다.

### v0.1.0 - 2026-07-30

- LocalPlayer UI Root, 레이어, 완전 Pause와 View Data 구조를 정의했다.
- 기존 AimReticle의 의미를 보존한 수명 이전 전략을 정리했다.
- 차량, 무기, 타겟과 Radar 데이터 계약을 제안했다.
- 미공개 정보, 공통 자원 채널, Blueprint Tooltip과 금지 패턴을 추가했다.
