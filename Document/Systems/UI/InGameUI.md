# InGameUI

- Version: 1.1.11
- Date: 2026-08-31
- Status: Current / CF-FQ-032 Done / CF-FQ-039 Armor Modular Runtime/WBP + Common Plate·Direction 2-Icon Production Assetization PASS / Vehicle-specific Silhouette Source + Production Texture·VehicleData Catalog Persisted Binding PASS
- Scope: CarFight 싱글플레이 LocalPlayer 인게임 UI Root, Production HUD, AimReticle, Target Marker, Pause, HUD ViewData/Presenter와 Visual Context의 현재 구현 계약

---

## 1. 문서 목적

이 문서는 CarFight의 현재 인게임 UI가 실제 Runtime에서 어떤 객체가 수명을 소유하고, Gameplay 상태가 어떤 경로로 Production HUD에 전달되며, Pause·AimReticle·Target Marker가 어느 Layer에 존재하는지를 기록한다.

이 문서는 미래 UI 기획이나 Visual 품질 목표가 아니라 **현재 구현된 시스템 계약**을 기준으로 한다. UI-P0-10에서 Build·Automation·fresh PIE RuntimeRead로 검증된 기술 사실만 Current로 승격하며, 사용자 미확인 Visual·UX·조작감·Zoom Feel은 완료된 Current 품질로 확대하지 않는다.

현재 핵심 원칙은 다음과 같다.

```text
Gameplay Runtime 계산
  -> C++ Gameplay Component / Pawn
  -> UCFHUDDataProvider
  -> FCFInGameUIViewData
  -> UCFHUDPresenter
  -> Production UMG Widget

UI 수명
  -> LocalPlayer UCFUISubsystem
  -> UCFUIRootWidget
  -> 명시 Layer
  -> Production HUD / AimReticle / Target Marker / Pause Menu
```

---

## 2. 현재 책임 경계

### `UCFUISubsystem`

- `ULocalPlayerSubsystem` 기반 UI 수명 소유자다.
- 현재 LocalPlayer의 `ACFPlayerController`를 등록하고 World별 UI Root를 관리한다.
- Production HUD, AimReticle, TargetSelect World Marker와 Pause Menu를 단일 인스턴스로 생성·연결·정리한다.
- Possessed Pawn 변경 시 현재 Pawn 약한 참조를 갱신하고 AimReticle·Target Marker를 같은 수명 경계에서 Rebind한다.
- Style, Density, HUD Layout, HUD Visual Data와 Config Widget Class를 해석해 Consumer에 주입한다.
- Config Style/Density/HUD Layout이 없거나 검증에 실패하면 전역 mutable CDO를 반환하지 않고 각 LocalPlayer UISubsystem이 소유하는 transient Native fallback을 사용한다.
- AimReticle/TargetSelect Widget Class와 Layer ZOrder의 runtime owner이며, `ACFVehiclePawn`의 동명 Legacy Class/ZOrder 필드는 저장 역직렬화 호환 전용이다.
- Gameplay 판정 자체를 소유하지 않는다.

### `UCFUIRootWidget`

- LocalPlayer UI의 단일 Root Widget이다.
- `UCFUISubsystem`이 생성한 뒤 `AddToViewport(RootViewportZOrder)`로 Viewport 계층에 등록한다.
- Game, HUD, Screen, Panel, Menu, Modal, System, Debug의 8개 Layer를 제공한다.
- 개별 Gameplay 상태를 계산하지 않고 Widget 수명과 Layer 부모 역할만 한다.

### `UCFHUDDataProvider`

- 현재 Possessed Vehicle Pawn과 Runtime Component의 HUD용 읽기·변환 경계를 소유한다.
- Gameplay Runtime/Component 상태를 `FCFInGameUIViewData`로 투영한다.
- Pawn Rebind 시 이전 Pawn Delegate를 해제하고 새 Pawn Source에 연결한다.
- Radar Zoom 요청 같은 UI 의도를 현재 HUD 상태에 반영하되 Sensor 탐지 Runtime이나 Scanner 원본 설정을 직접 변조하지 않는다.
- Presenter가 Gameplay Actor/Component를 다시 조회하지 않도록 필요한 의미 데이터를 ViewData에 포함한다.

### `UCFHUDPresenter`

- `FCFInGameUIViewData`를 Production Widget에 적용하는 Presentation 계층이다.
- Gameplay Actor/Component를 직접 검색하거나 Gameplay 규칙을 재계산하지 않는다.
- Text, Visibility, Progress, Brush/Color, Radar dynamic visual 같은 실제 화면 표현으로 ViewData를 변환한다.

### Blueprint / UMG

- 정적 Layout, Anchor, Slot, 패널 배치, Image/Text 구조와 미술 표현을 소유한다.
- Gameplay Component Cast나 전투 판정을 새로 소유하지 않는다.
- Designer에서 편집해야 하는 정적 위치·크기를 C++가 불필요하게 하드코딩하지 않는다.

---

## 3. UI Root와 Layer 구조

현재 `ECFUILayer`는 아래 8개 Layer를 사용한다.

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

현재 주요 인게임 Widget 배치는 다음과 같다.

| Widget / 기능 | Layer | Local ZOrder | 수명 소유자 |
| --- | --- | ---: | --- |
| `WBP_CFInGameHUD` Production HUD | HUD | 0 | `UCFUISubsystem` |
| `WBP_AimReticle` | HUD | 10 | `UCFUISubsystem` |
| `WBP_TargetSelect` World Marker | Game | 0 | `UCFUISubsystem` |
| `UCFPauseMenuWidget` | Menu | 0 | `UCFUISubsystem` |

`Screen`, `Panel`, `Modal`, `System`, `Debug` Layer는 동일 Root 체계의 다른 화면·패널·Modal·시스템 메시지·디버그 UI를 위한 독립 Layer다.

Production HUD와 AimReticle은 같은 HUD Layer 안에서 Local ZOrder로 앞뒤를 결정한다. Target Marker는 월드 위치 투영 성격 때문에 Game Layer를 사용하며, Pause Menu는 Menu Layer에서 인게임 HUD와 분리된다.

---

## 4. LocalPlayer / World 수명

현재 수명 흐름은 다음과 같다.

```text
ACFPlayerController::BeginPlay
  -> LocalPlayer의 UCFUISubsystem 조회
  -> RegisterPlayerController(this)
  -> UI Root 보장
  -> Production HUD / AimReticle / Target Marker 생성·연결

Possession 변경
  -> ACFPlayerController::OnPossess / OnUnPossess
  -> UCFUISubsystem::NotifyPossessedPawnChanged
  -> CurrentPawn 약한 참조 갱신
  -> RebindAimReticleToCurrentPawn
  -> RebindTargetSelectToCurrentPawn
  -> OnCurrentPawnChanged 방송

World Cleanup / Controller EndPlay
  -> Pawn 참조 해제
  -> Presenter/Widget 연결 정리
  -> Root 및 Layer Widget 정리
```

UISubsystem이 LocalPlayer보다 짧은 Pawn lifetime을 소유하지 않도록 AimReticle과 Target Marker는 현재 Pawn을 약한 참조로 사용한다. Pawn 교체 후 이전 Pawn 이벤트나 상태가 새 HUD에 남지 않는 것이 Current 계약이다.

---

## 5. Production HUD 데이터 흐름

Production HUD의 기본 데이터 흐름은 다음과 같다.

```text
ACFVehiclePawn + Runtime Components
  -> UCFHUDDataProvider
      -> FCFInGameUIViewData
          -> UCFHUDPresenter
              -> WBP_CFInGameHUD
                  -> Vehicle / Weapon / Target / Radar 등 Production Panel
```

Provider는 현재 Gameplay 소유자가 이미 계산한 값을 읽고 HUD용 의미로 정규화한다. Presenter는 이 ViewData만 소비해 실제 Widget 속성을 갱신한다.

### 현재 Production 채널

- Vehicle
  - Speed / RPM
  - 차량 기본 상태
  - Shield / Armor / Integrity 등 실제 Defense Runtime이 제공하는 상태
- Weapon
  - 활성 무기 선택
  - Cooldown
  - finite Ammo / Magazine / Reserve
  - Reload
  - Launcher Sequence
  - Heat
  - WeaponCharge
  - Runtime Source가 없는 채널은 임의 값을 만들지 않고 Unavailable/Collapsed를 유지한다.
- Target
  - 선택 여부
  - Sensor Knowledge 수준
  - 관계, 식별 정보, 거리와 분석 상태 등 Provider가 승인한 의미 정보
- Radar
  - Heading-Up Contact 표현
  - Scanner-owned Range Profile 기반 Display Range/Zoom
  - 실제 Active Scan 도달 가능 범위를 넘지 않는 최대 표시 범위
  - 관계별 Image contact
  - 선택 Target의 Radar range-out edge 표현
- View / Alert / Style
  - 기존 VehicleCamera/Aim Runtime의 read-only View Mode 표현
  - AlertKey 기반 Notice/Warning/Critical lifecycle
  - Root에서 주입되는 Style/Density/Layout Visual Context

`VehicleBattery`처럼 실제 Gameplay Provider가 없는 future shared-power 채널을 다른 값에서 추정해 표시하지 않는다. player-facing WeaponGroup도 실제 Runtime owner가 생기기 전까지 내부 Mount/Profile ID로 대체하지 않는다.

---

## 6. AimReticle 특수 소비 경계

AimReticle은 Production HUD의 일반 Provider/Presenter 소비 경로와 다른 기존 특수 Consumer다.

```text
UCFUISubsystem
  -> WBP_AimReticle 단일 생성 / HUD Layer Z10
  -> Current Vehicle Pawn Rebind

UCFAimReticleWidget
  -> Weak VehiclePawnRef
  -> VehicleAimComp 상태 읽기
  -> VehiclePawn.BuildFireFeedbackViewData() 읽기
  -> Reticle / Weapon Reticle / FireFeedback 표시 갱신
```

즉 AimReticle이 `UCFHUDDataProvider`를 사용하는 것으로 해석하면 안 된다. UISubsystem은 **수명과 Rebind**를 소유하고, `UCFAimReticleWidget`은 현재 Pawn의 Aim/FireFeedback 표시값을 직접 읽는 기존 UI 특수 경로를 유지한다.

`Image_CenterDot`은 플레이어 조준 의도 Reticle이고, `Image_WeaponReticle`은 현재 Muzzle Direction 기반 터렛 조준 3D 지점의 화면 투영이다. Projectile 착탄 위치를 의미하지 않는다.

세부 Aim/FireFeedback 의미는 `Document/Systems/UI/AimReticle.md`가 소유한다.

---

## 7. Target Marker와 TargetPanel 경계

`WBP_TargetSelect`은 **월드 위치 Marker**만 담당한다. Target의 이름·거리·Knowledge·관계와 같은 의미 정보의 주 화면 표시는 Production TargetPanel의 Provider/Presenter 경로가 담당한다.

현재 Target Marker 수명은 다음과 같다.

```text
UCFUISubsystem
  -> WBP_TargetSelect 단일 생성 / Game Layer Z0
  -> Current Vehicle Pawn Rebind

UCFTargetSelectWidget
  -> Weak VehiclePawnRef
  -> TargetSelectComp 이벤트 캐시
  -> Candidate / Selected World Projection
```

현재 화면 밖 동작:

- Candidate는 화면 밖에서 숨긴다.
- 유효한 Selected Target은 Camera View Space 방향을 사용해 Safe Region 가장자리의 2-Corner Open Bracket으로 표시한다.
- 단순 raw screen-coordinate Clamp로 BehindCamera 방향을 계산하지 않는다.
- Screen-edge Image는 persisted WBP에 저장하지 않고 Runtime에서 하나만 생성한다.
- Screen-edge 관계색은 Provider가 이미 판정한 관계를 소비한다.
  - Friendly = Blue
  - Hostile = Red
  - Neutral / Unknown = Gray
- Screen-edge Neutral만 Unknown 회색을 재사용하며 전역 Neutral palette를 바꾸지 않는다.

Radar의 selected range-out edge와 Screen-off Target Marker는 서로 다른 Presentation 위치지만 같은 Target 선택 의미를 재계산하지 않는다.

---

## 8. Radar 현재 계약

Radar는 Heading-Up이다. Contact의 정규화 위치와 관계·Knowledge는 Provider가 현재 Sensor Snapshot을 이용해 구성하고 Presenter가 Image visual로 소비한다.

현재 범위 계약:

```text
DisplayRangeMeters = 현재 화면 표시 범위
MaximumDetectionRangeMeters = 실제 Sensor/Scanner가 제공하는 최대 탐지 성능
Range Presets = Scanner/Sensor profile 소유
Zoom In = 더 작은 preset
Zoom Out = 더 큰 preset
```

일반 Contact가 현재 Display Range 밖이면 숨기지만, 현재 Selected Target은 Radar 바깥 방향을 outer-edge marker로 유지할 수 있다. Radar 최대 Display Range를 실제 Active Scan 도달 범위보다 크게 만들지 않는다.

Range preset 수치나 생산용 tuning을 UI 코드에 임의 하드코딩하지 않는다. 기술적 Zoom 입력·표현 경로는 Current지만 실제 production preset tuning과 사용자 Zoom Feel 평가는 별도 content/USER 판단이다.

---

## 9. Pause와 입력 경계

싱글플레이 Pause 수명은 `ACFPlayerController`와 `UCFUISubsystem`이 분담한다.

### Pause 진입

```text
Pause 입력
  -> ACFPlayerController
  -> UCFUISubsystem::NotifyPauseInputRequested
  -> EnterSinglePlayerPause
      -> UI Root 보장
      -> Pause Menu 생성 / Menu Layer Z0
      -> ACFPlayerController::SetSinglePlayerPaused(true)
          -> Vehicle Gameplay Input 중립화
          -> FlushPressedKeys
          -> Unreal SetPause(true)
      -> ScreenState = Paused
      -> ApplyUIInputMode(GameAndUI, ContinueButton, Cursor=true)
      -> Continue 버튼 Focus
```

### Pause 해제

```text
Continue / Pause Toggle / Back
  -> SetSinglePlayerPaused(false)
  -> Pause Menu 제거
  -> 이전 Primary Screen 또는 InGame Input Mode 복구
  -> 눌린 키 상태 Flush
```

현재 Controller 입력 소유권:

- System/UI Mapping Context는 Controller가 자신이 등록한 것만 제거·복원한다.
- 차량 Gameplay Mapping은 현재 Pawn의 `DefaultInputMappingContext`가 소유한다.
- Controller의 `GameplayInputMappingContext`는 원자적인 소유권 이전 전까지 None을 유지한다.
- Pause를 위해 Pawn Gameplay Context를 삭제하지 않고 입력을 중립화·억제한다.
- Pause/Back InputAction이 없는 경우 Escape, Gamepad Special Right, BackSpace, Gamepad FaceButton Right 등의 bounded C++ fallback이 존재한다.
- Pause Continue는 Enter와 Gamepad FaceButton Bottom fallback도 지원한다.

World Pause 중 별도 `bTickEvenWhenPaused`를 허용하지 않는 Gameplay 진행은 멈춘다. P0-10에서는 Launcher/Projectile/Timer뿐 아니라 Weapon Heat 냉각, WeaponCharge 회복, TargetUse 주기 재평가가 Pause 중 진행되지 않는 계약을 자동 검증했다.

Current Product에는 실제 AI Controller/BehaviorTree Runtime과 Lock-on Gameplay Runtime이 없으므로 Pause의 AI/Lock 항목을 구현된 기능처럼 기록하지 않는다.

---

## 10. Config와 Visual Data

현재 `DefaultGame.ini`의 UISubsystem 기본 연결은 다음과 같다.

```text
DefaultStyleDataAsset
  = /Game/CarFight/UI/Style/DA_CFUIStyle_Default

DefaultDensityDataAsset
  = /Game/CarFight/UI/Density/DA_CFUIDensity_Standard

DefaultHUDLayoutDataAsset
  = /Game/CarFight/UI/Layout/DA_CFHUDLayout_1080_16

DefaultHUDVisualDataAsset
  = /Game/CarFight/UI/HUD/Visual/DA_CFHUDVisual_Default

DefaultInGameHUDWidgetClass
  = /Game/CarFight/UI/HUD/WBP_CFInGameHUD

DefaultAimReticleWidgetClass
  = /Game/CarFight/UI/WBP_AimReticle

DefaultTargetSelectWidgetClass
  = /Game/CarFight/UI/WBP_TargetSelect
```

UISubsystem은 Style/Density/Layout을 Production Styled Widget에 Visual Context로 주입한다. Target Screen-edge 표현에는 HUD Visual Data의 Edge texture와 Style relation color / Safe Margin을 주입한다.

Config Asset이 비어 있거나 검증에 실패하는 경우 Style/Density/HUD Layout은 `UCFUISubsystem` 자신을 Outer로 하는 transient Native fallback 객체를 lazy 생성한다. Getter 반환 타입과 Blueprint API는 유지하지만 전역 `GetMutableDefault<>` CDO를 외부에 노출하지 않는다. 따라서 fallback Consumer가 실수로 값을 변경해도 다른 LocalPlayer나 전역 CDO에 전파되지 않는 것이 Current 계약이다.

AimReticle과 TargetSelect의 실제 Class는 위 Config Soft Class를 Subsystem 수명에서 해석한다. `ACFVehiclePawn::AimReticleWidgetClass`, `AimReticleZOrder`, `TargetSelectWidgetClass`, `TargetSelectHudZOrder`는 기존 Asset 역직렬화를 위해 이름과 타입만 보존하며 Pawn constructor/runtime은 Class를 hard-load하거나 Legacy ZOrder를 소비하지 않는다.

이 연결은 Content path의 Current 기본값이며, Widget 또는 Pawn 내부가 임의로 Project path를 하드코딩해 직접 Load하는 구조로 확대하지 않는다.

### 10.1 VehiclePanel persisted editable 구조

2026-08-24 AssetDump persisted evidence 기준 Vehicle Armor 영역은 이미 아래 canonical 구조다.

```text
WBP_CFArmorBodyMap
└─ CanvasPanel_Root
   ├─ Image_VehicleSilhouette
   ├─ WBP_ArmorFront  : WBP_CFArmorSector
   ├─ WBP_ArmorRight  : WBP_CFArmorSector
   ├─ WBP_ArmorRear   : WBP_CFArmorSector
   ├─ WBP_ArmorLeft   : WBP_CFArmorSector
   ├─ WBP_ArmorTop    : WBP_CFArmorSector
   └─ WBP_ArmorBottom : WBP_CFArmorSector

WBP_CFArmorSector
└─ SizeBox_Root
   └─ HorizontalBox_Content
      ├─ SizeBox_Plate
      │  └─ Overlay_Plate
      │     ├─ Image_ArmorPlate
      │     ├─ Text_Direction
      │     └─ Image_DirectionIcon
      ├─ Spacer_ArmorGap
      └─ SizeBox_ArmorProgress
         └─ ProgressBar_Armor
```

현재 계약:

- 6방향 Armor는 **같은 `WBP_CFArmorSector` Generated Class를 재사용**한다. 별도 `WBP_CFArmorSlot`은 존재하지 않으며 현재 구조에서 추가할 이유도 없다.
- BodyMap의 VehicleSilhouette와 6개 Sector는 각각 독립 `CanvasPanelSlot`을 사용한다. 저장된 실제 Position/Size는 Designer Asset이 소유하며 Editor Bridge scaffold 값으로 재적용하지 않는다.
- Presenter는 `WBP_ArmorFront/Right/Rear/Left/Top/Bottom`의 안정 instance name을 통해 Runtime Armor Ratio만 전달한다. Designer 재배치가 Runtime 방향 의미를 바꾸지 않는다.
- `UCFHUDPresenter v1.26.1`은 `VehicleViewData.VehicleDataAsset` identity로 `UCFHUDVisualData`의 차량별 silhouette catalog를 조회하고 차량 identity가 바뀔 때만 Brush resource를 교체한다. catalog miss/fallback null에서는 이전 차량 Brush를 비우고 `Image_VehicleSilhouette`을 Collapsed 처리하며, 유효 Texture가 돌아오면 동일 Designer Slot을 다시 표시한다. 기존 단일 `VehicleSilhouette`은 compatibility fallback 의미를 유지한다.
- `UCFArmorSectorWidget v1.2.1`은 legacy 방향-baked Plate와 modular common Plate + 별도 `DirectionIconTexture`를 함께 지원한다. `DirectionIconRotationDegrees`는 재사용 Sector 인스턴스별 Designer-owned 회전값이며 C++은 `Image_DirectionIcon`의 Render Transform Angle만 적용한다.
- 현재 persisted Production 상태에서는 `Image_DirectionIcon`이 실제 `Overlay_Plate`에 존재하고 `DA_CFHUDVisual_Default`의 common `ArmorDirectionArrow` / `ArmorDirectionChevron2` Texture binding도 완료됐다. 각 Sector의 실제 Icon 선택·회전은 Designer instance가 소유하고 Texture 미설정/누락 시에만 `Text_Direction` fallback을 사용한다. 기존 P2 방향 Plate 6종은 compatibility scaffold이며 새 Production Visual authority가 아니다.
- `SetArmorPercent()`는 Presenter가 전달한 0~1 Ratio를 clamp하고 `>0.60 Armor / >0.30 Caution / <=0.30 Critical` Presentation 색으로 변환해 `Image_ArmorPlate`와 `ProgressBar_Armor`에 함께 적용한다. Gameplay Defense 계산과 Ratio source는 변경하지 않는다.
- 0%는 현재 Critical tint + empty Bar까지 구현돼 있다. Broken X/갈라진 Plate 같은 추가 Art는 CF-FQ-039 USER Visual Target/refinement가 소유한다.
- Armor icon-first/fallback 표시와 현재 배치는 2026-08-24 USER Visual PASS를 받았다. 관련 결함 없이 해당 Armor slice를 반복하지 않는다.
- 저장 `Border_Surface`는 `DA_CFHUDVisual_Default.VehiclePanelFrame`의 P2 VehiclePanel Frame을 9-Slice Box로 소비한다. 전체 Frame Texture와 Designer Layout은 기존 persisted Asset이 계속 소유한다.
- `UCFHUDPresenter v1.25.1`은 `ProgressBar_Shield/Integrity`의 기존 Widget/Slot/Ratio sink를 유지하면서 `SurfaceRaised` dark track, 2px inset, `LeftToRight + Scale + non-marquee` Style을 Runtime Presentation으로 적용하고 Track/Fill/Marquee Brush에 10px intrinsic ImageSize를 명시한다.
- Shield Bar/Icon/Value는 `Shield` token, Integrity Bar/Icon/Value는 `Integrity` token을 공유한다. 새 Gameplay 판정, 새 Defense Ratio 계산, 새 Content Asset은 추가하지 않는다.
- fresh PIE에서 발견된 Defense Bar 높이 0 회귀는 `UCFHUDPresenter v1.25.1`의 10px intrinsic Brush size로 교정했다.
- `CFHUDDataTests v1.34.2`는 viewport에 붙지 않은 transient Widget의 `GetDesiredSize()` 대신 실제 회귀 원인인 ProgressBar Style Background/Fill/Marquee Brush intrinsic height를 직접 검증하며 final `VehicleDefenseBarVisualContract` 1/1 PASS다.
- fresh `M_VehicleDefensePIE` Defense Pawn에서 Snapshot 기준 Shield/Integrity ProgressBar가 각각 591x10으로 동시에 존재하고 Screenshot 기준 Shield Cyan / Integrity Red Bar가 실제 픽셀로 렌더링되는 것을 확인했다. Frame/Speed-RPM/Armor도 보존됐다.
- 따라서 Frame + Shield/Integrity는 AI Technical/Pixel PASS다. 최종 두께·색·가독성·시안 방향은 USER Visual Review Pending으로 유지한다.
- Armor USER PASS는 이 회귀와 무관하므로 보존한다.

### 10.2 Vehicle-specific silhouette Source Authority

2026-08-25 persisted `CFVehicleData`와 `VehicleVisualConfig.ChassisMesh`를 재확인해 HUD silhouette의 형상 Source Authority를 다음 체인으로 고정했다.

```text
Runtime catalog key
= exact CFVehicleData asset identity

Shape Source Authority
= 해당 VehicleData.VehicleVisualConfig.ChassisMesh exact asset identity

Production source
= 위 ChassisMesh 형상을 탑다운·좌향 HUD 규격으로 투영한 mesh-derived silhouette
```

현재 persisted mapping은 다음과 같다.

| VehicleData | ChassisMesh Source Authority | Shape group |
| --- | --- | --- |
| `/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan` | `/Game/CarFight/Vehicles/Meshes/Sedan/Sedan.Sedan` | Sedan |
| `/Game/CarFight/Vehicles/Data/Definitions/DA_TestSUV` | `/Game/CarFight/Vehicles/Meshes/SUV/SUV.SUV` | SUV |
| `/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV` | `/Game/CarFight/Vehicles/Meshes/SUV/SUV.SUV` | SUV |

Current 계약:

- VehicleData는 3종이지만 unique ChassisMesh 형상은 **Sedan/SUV 2종**이다.
- `UCFHUDVisualData.VehicleSilhouettes`의 runtime key는 계속 exact VehicleData identity다. schema나 별도 UI용 VehicleType ID를 추가하지 않는다.
- 같은 `SUV.SUV`를 사용하는 `DA_TestSUV`와 `DA_VehicleDefense_TestSUV`는 catalog entry를 각각 가지되 동일 SUV silhouette Texture를 공유할 수 있다.
- 기존 `SourceArt/UI/HUD/P2/T_UI_VehSil_LF.png`는 `GenerateUIHUDArtP2.ps1`의 고정 좌표 polygon으로 생성된 generic 전투차량 Reference이므로 Sedan/SUV의 형상 Source Authority가 아니다.
- `VT03_VehSil_LF_v2`도 exact ChassisMesh identity와 연결되지 않은 target-derived 후보이므로 특정 VehicleData에 임의 bind하지 않는다.
- authority ChassisMesh에서 생성한 `VT05_VehSil_Sedan.png` / `VT05_VehSil_SUV.png`는 512×256 transparent PNG Source로 Technical 검증됐고 USER의 진행 승인 뒤 Production Assetization에 사용됐다.
- Production Texture는 `/Game/CarFight/UI/HUD/Visual/T_UI_VehSil_Sedan`과 `/Game/CarFight/UI/HUD/Visual/T_UI_VehSil_SUV`이며, fresh persisted AssetDump에서 둘 다 실제 Texture2D Asset으로 확인됐다.
- `DA_CFHUDVisual_Default.VehicleSilhouettes`는 persisted **3 entry**다. `DA_TestSedan → T_UI_VehSil_Sedan`, `DA_TestSUV → T_UI_VehSil_SUV`, `DA_VehicleDefense_TestSUV → T_UI_VehSil_SUV` exact mapping을 사용한다.
- 기존 단일 `VehicleSilhouette = T_UI_VehSil_LF`는 미등록 VehicleData용 compatibility fallback으로 그대로 보존한다.

---

## 11. C++ / Blueprint 소유권

### C++ 소유

- UI Root와 Layer 수명
- Current Controller/Pawn Rebind
- Gameplay Runtime → HUD ViewData 변환
- ViewData → Production Presentation 갱신
- Pause 상태와 Input Mode 전환
- Radar dynamic contact pool과 Runtime positioning
- Target Screen-edge Runtime marker 위치/방향
- AimReticle의 현재 Pawn Aim/FireFeedback 표시 갱신

### Blueprint / Designer 소유

- Production HUD의 정적 패널 위치와 크기
- Anchor / Alignment / Slot
- Image / Text / Material 배치
- Visual Style와 Art 조정
- 재사용 UMG 구성요소의 Designer 편집성

정적 Layout을 C++ 생성 코드에서 강제로 다시 배치해 Designer 소유권을 빼앗지 않는다. 반대로 Blueprint가 Gameplay Component를 Cast해 전투 판정을 다시 계산하는 것도 금지한다.

---

## 12. 검증된 Current 범위

UI-P0-10 Technical Validation과 2026-08-22 post-closure code review remediation 기준:

- Official Build `1a632fcdac38424e9750633a95204d76` PASS.
- UI-P0-10 당시 `CarFight.UI` broad process `2fdf9866d08e4ae2aa464c21e6a373bb`: 43/43 PASS, Failure 0.
- post-closure remediation final Official Build `bd3640c616794cd7a54cc8a8afa4b021`: PASS / Exit 0.
- post-closure focused regression: `CarFight.UI.UI_P0_07` `baca1e3ad76d4889b987211307cd345d` 2/2, `CarFight.UI.UI_P0_08` `dd2e9597ff59486caa116b1c781217f5` 4/4, `AlertStyleLifecycleContract` `33fc6168cae840218a007e15383f2217` 1/1, `SEN_P0_06.HUDSnapshot` `10825286b6d14994a31baa0295486aec` 1/1 PASS.
- post-closure broad regression: `CarFight.UI` `c7bbf0bf32c54d248c7cd3e122d700bc` 44/44, `CarFight.Sensor` `dc4ba42db0fc477aa82112a903b2054b` 14/14 PASS.
- ResolutionLayoutContract:
  - 1920×1080
  - 2560×1440
  - 3440×1440
  - 5120×1440
  - Project DPI Scale 적용
  - Production 6 Panel 화면 내 유지·비겹침
  - ReticleLayer full-stretch
- Ammo:
  - Reload 1/1 PASS
  - HUD/Rebind 1/1 PASS
- Sensor:
  - HUD/DestroyedHold 1/1 PASS
  - ContactLifetime/Reacquire 2/2 PASS
- fresh AI-owned PIE `14c0d695567b4569814a39229e77fcc9`:
  - `TestMap` PIE
  - LocalPlayer 1
  - `CFPlayerController`
  - `BP_CFVehiclePawn`
  - Drive/Aim/Weapon/Ammo/Health/Defense/TargetSelect/Sensor 핵심 Component 확인
- fresh PIE actual pixel evidence:
  - Vehicle HUD 실제 생성
  - Radar 실제 생성
  - Weapon HUD 실제 생성
  - TargetPanel `NO TARGET` 실제 생성
  - 중앙 Reticle 단일 생성
- post-closure Target Identity:
  - `ACFVehiclePawn`의 안정 `TargetId`는 유효한 `VehicleData.PrimaryAssetId.PrimaryAssetName`을 사용한다.
  - VehicleData/PrimaryAssetId가 없으면 `TargetId=None`으로 fail-closed한다.
  - Player-facing 차량 이름 source가 없는 현재 `DisplayName`은 Empty를 유지하며 Actor `GetFName()/GetName()`을 UI/Sensor identity fallback으로 사용하지 않는다.
  - `ContactId`는 개별 Sensor Contact identity를 계속 소유하고 Vehicle TargetId와 역할을 합치지 않는다.
- post-closure Alert lifecycle:
  - 유한 Notice/Warning duration은 Alert 활성 시점이 아니라 실제 Primary로 처음 표시된 Game-Time부터 시작한다.
  - 상위 Priority에 가려진 suppression 시간은 duration을 소비하지 않는다.
  - 한번 만료된 같은 AlertKey는 해당 Gameplay 상태가 해제되기 전 재표시하지 않는다.
- post-closure hot-path:
  - Screen-edge relation은 `GetCurrentViewDataRef()`의 const cache를 읽어 전체 HUD ViewData 복사를 만들지 않는다.
  - 한 HUD Refresh에서 Sensor Snapshot과 Radar Range Profile을 각각 한 번만 캡처해 Target/Radar가 공유한다.
  - AimReticle 자동 Tick은 cache를 일괄 갱신한 뒤 Text/WeaponReticle/Visibility를 한 번씩 반영하고 외부 Apply API 의미는 유지한다.

CF-FQ-039 VehiclePanel Production Presentation 추가 검증:

- 2026-08-25 Armor modular runtime/WBP final Official Editor Build `ea1918cf88b0470e91398e9a87840e6b`: PASS / Exit 0.
- `WBP_CFArmorSector` one-asset migration clean re-run은 Exit 0 / `already_migrated=true / saved_assets=0`으로 idempotent PASS했다. UE 5.8 WidgetVariable GUID 누락 ensure는 교정 후 재발하지 않았다.
- fresh persisted AssetDump에서 `WBP_CFArmorBodyMap`은 기존 `Image_VehicleSilhouette + WBP_ArmorFront/Right/Rear/Left/Top/Bottom` 8-node 구조와 각 Canvas Slot을 그대로 유지하고, `WBP_CFArmorSector`만 `Image_DirectionIcon`이 추가된 10-widget 구조임을 확인했다.
- current focused `CarFight.UI.UI_P0_06`: 17/17 PASS / Fail 0. 첫 실행의 ArmorSector 1건 실패는 구형 direction-baked Plate 기대를 modular Texture-unbound fallback 계약으로 교정한 뒤 최종 PASS했다.
- Armor final Official Editor Build `c8f67e0d88f647db8c4de7bba485be91`: PASS / Exit 0.
- `CarFight.UI.UI_P0_06.ArmorSectorProductionVisualContract`: 1/1 PASS.
- Armor 표시 의도는 USER Visual PASS.
- Frame/Shield/Integrity initial Official Editor Build `5071957595464725b945c9ad2f704b99`: PASS / Exit 0.
- Defense Bar pixel-fix Official Build `d9c33e2a689e42b2a511698fa021df4f`: PASS / Exit 0.
- test-harness final Official Build `87d13671b2aa466bac88a820d5eda72b`: PASS / Exit 0.
- `CarFight.UI.UI_P0_06.VehicleDefenseBarVisualContract`: final 1/1 PASS — persisted 9-Slice Frame resource, Shield/Integrity Ratio·semantic color·2px inset·fill mode + Background/Fill/Marquee intrinsic height 확인.
- `CarFight.UI.UI_P0_03.DefenseRuntimeViewData`: 1/1 PASS.
- `CarFight.UI.UI_P0_06.RpmGaugeVisualBindingContract`: 1/1 PASS.
- fresh `M_VehicleDefensePIE` Defense Pawn pixel evidence: SHIELD 100/100 + 591x10 Bar, INTEGRITY 100/100 + 591x10 Bar 동시 렌더, Shield Cyan / Integrity Red 실제 픽셀 확인. Frame/Speed-RPM/Armor 보존.
- Frame/Shield/Integrity Technical/Pixel slice 당시 신규 Content Asset/Blueprint 저장 mutation은 0이었고 기존 P2 Frame/Armor Texture와 Designer Layout을 재사용했다.
- 2026-08-25 후속 Armor 2-Icon Production Assetization에서 `T_UI_ArmorIcon_Arrow`와 `T_UI_ArmorIcon_Chevron2` Texture2D 2종을 신규 저장하고 `DA_CFHUDVisual_Default.ArmorDirectionArrow/ArmorDirectionChevron2`에 연결했다.
- 두 Source PNG는 `SourceArt/UI/HUD/VT/VT04_ArmorIcon_Arrow.png`, `VT04_ArmorIcon_Chevron2.png`이며 Project External Image Vision readback에서 각각 128×128 우향 단일 Arrow / 이중 Chevron 형상을 확인했다.
- final Official Editor Build `8eed02112b73443a9c4df1d03d9a859b` PASS, `CFUIArmorArt` commandlet Exit 0 / `imported=2 bound=2 saved_packages=3`, fresh AssetDump `adset_v1_af4240b727a9713595181ce45d13e44d.a1c14e4b40026b6373270d82`에서 Texture2종과 DataAsset binding을 persisted 확인했다.
- 후속 common Plate slice에서 `SourceArt/UI/HUD/VT/VT04_ArmorPlate.png`를 exact 98×130 / 1174 bytes / SHA-256 `986635f29722558f5682db633bb8493a50c04b82f1e9d3b2cb21dab33fa5e5be`로 repository Source Binding했고 Project External Image Vision readback도 동일 Source authority를 확인했다.
- `CFUIArmorArt v1.1.0 -PlateOnly`는 기존 Arrow/Chevron2 Texture와 DataAsset binding을 validation-only로 확인한 뒤 `T_UI_ArmorPlate` 1종과 `ArmorCommonPlate`만 추가했다. Official Editor Build `3183bb86d5e7483e9cde64be70c27551` PASS / Commandlet process `19d669f0a2c8415a99305aa13133c59a` Exit 0 / `imported=1 bound=1 saved_packages=2 existing_icons_validated=2 sector_mutation=0 silhouette_catalog_mutation=0`이다.
- fresh AssetDump `adset_v1_653b88c9306c201ab9f785c79b7431ce.908692ddcf890cb552fe879e`에서 `T_UI_ArmorPlate` Texture2D와 `DA_CFHUDVisual_Default.ArmorCommonPlate=/Game/CarFight/UI/HUD/Visual/T_UI_ArmorPlate.T_UI_ArmorPlate`를 persisted 확인했다. Arrow/Chevron2 두 binding도 그대로 유지됐다.
- 따라서 `ArmorCommonPlate + ArmorDirectionArrow + ArmorDirectionChevron2` 세 필드의 non-null 조건은 충족됐다. `HasModularArmorArt()` 함수 자체를 별도로 실행 검증한 것은 아니므로 실행 PASS로 확대하지 않는다. `VehicleSilhouettes` count 0과 6개 Sector의 Designer 위치·크기·Icon 선택·회전은 그대로 유지한다.
- Frame/Shield/Integrity 최종 Visual 품질은 USER review Pending이다.

이 evidence는 **AI Technical Validation**이다. 다음은 기술 PASS로 해석하지 않는다.

- UI-P0-08 Radar/Screen-edge 최종 Visual 품질
- Radar Zoom Feel
- D1-11-ART의 SpeedGauge / VehiclePanel / 전체 Production HUD 최종 Visual Review
- content-dependent Heat/Charge/RPM tuning 체감

위 항목은 사용자가 실제로 판단하기 전 USER PASS로 기록하지 않는다.

---

## 13. 유지보수 규칙

아래 변경이 발생하면 이 문서를 함께 갱신한다.

- `UCFUISubsystem`의 Root/Widget 수명 소유권 변경
- `ECFUILayer` 종류 또는 주요 Widget Layer/ZOrder 변경
- Production HUD가 `UCFHUDDataProvider -> FCFInGameUIViewData -> UCFHUDPresenter` 외의 새 Gameplay 조회 경로를 사용하게 될 때
- AimReticle이 Provider/Presenter 경로로 이관되거나 직접 Pawn 소비 경계가 바뀔 때
- Target Marker와 Production TargetPanel 책임 분리가 바뀔 때
- Pause owner, 실제 World Pause 정책 또는 Controller/Pawn Input Context 소유권이 바뀔 때
- 기본 Style/Density/Layout/Visual/Widget Config가 바뀔 때
- Runtime Source가 없던 VehicleBattery, player-facing WeaponGroup 등의 실제 owner가 구현될 때

---

## 14. Migration

### v1.0.0 -> v1.1.0

- CF-FQ-032 Done 상태를 다시 열지 않고 post-closure code review의 P1 2건(Target Identity, Alert suppression lifecycle)과 P2 3건(Screen-edge ViewData copy, AimReticle duplicate refresh, Provider Snapshot/Profile dedupe)을 Current 구현에 반영한다.
- 차량 TargetId는 VehicleData PrimaryAssetId 기반 안정 ID를 사용하고 Player-facing DisplayName은 명시 source 전까지 Empty를 유지한다. Actor instance 이름 fallback은 금지한다.
- Alert duration은 실제 첫 presentation부터 시작하며 suppression 시간 비소모와 completed-until-clear 계약을 적용한다.
- 성능 교정은 표시 의미·Gameplay ownership·Blueprint layout을 변경하지 않는다.
- Product Content Asset/Blueprint mutation은 0이며 USER Visual/Feel Deferred 상태도 변경하지 않는다.

### v1.0.0

- CF-FQ-032 UI-P0-02~10에서 확정된 LocalPlayer UI Root, Production HUD, AimReticle/Target Marker 단일 수명, Pause와 ViewData/Presenter 계약을 Current Systems로 승격한다.
- 기존 Product Source, Config와 Content Asset을 이 문서 작성 때문에 변경하지 않는다.
- 기존 `AimReticle.md`는 Aim/FireFeedback의 상세 의미 owner로 유지하되 생성·수명 owner는 `UCFUISubsystem` Current 계약에 맞춰 갱신한다.
- 사용자 Visual·UX·조작감 Deferred 항목은 이 승격으로 PASS 처리하지 않는다.

---

## 15. Changelog

### v1.1.10 - 2026-08-25

- authority ChassisMesh 기반 Sedan/SUV Source를 실제 512×256 transparent PNG로 생성·검증하고 USER 진행 승인 뒤 `T_UI_VehSil_Sedan` / `T_UI_VehSil_SUV` Production Texture로 Assetize했다.
- fresh persisted AssetDump에서 `DA_CFHUDVisual_Default.VehicleSilhouettes` 3 entry exact mapping을 확인했다. Sedan은 Sedan Texture, 일반/Defense SUV 두 VehicleData는 동일 SUV Texture를 공유한다.
- 기존 `VehicleSilhouette` fallback과 common Plate·Direction 2-Icon binding, six-sector Designer ownership은 변경하지 않았다.

### v1.1.9 - 2026-08-25

- persisted `CFVehicleData` 3종과 `VehicleVisualConfig.ChassisMesh`를 다시 읽어 `DA_TestSedan → Sedan.Sedan`, `DA_TestSUV → SUV.SUV`, `DA_VehicleDefense_TestSUV → SUV.SUV` exact mapping을 Current 계약으로 고정했다.
- silhouette 형상 Source Authority를 `VehicleData identity → ChassisMesh identity → mesh-derived silhouette Source`로 확정했다. Runtime catalog는 VehicleData-keyed를 유지하고 동일 SUV Mesh를 쓰는 두 VehicleData는 같은 silhouette Texture를 공유할 수 있다.
- P2 generic polygon silhouette와 exact Chassis identity 없는 VT03 후보는 Production Source Authority에서 제외했다. 실제 Sedan/SUV source 승인 전 `VehicleSilhouettes` count 0과 compatibility fallback을 유지하며 Content Import/Bind는 수행하지 않는다.

### v1.1.8 - 2026-08-25

- CF-FQ-039 common Armor Plate Source를 `VT04_ArmorPlate.png`로 exact binding하고 `T_UI_ArmorPlate` Production Texture + `DA_CFHUDVisual_Default.ArmorCommonPlate` persisted binding까지 Technical PASS로 닫았다.
- Official Build `3183bb86d5e7483e9cde64be70c27551`, `CFUIArmorArt -PlateOnly` process `19d669f0a2c8415a99305aa13133c59a` Exit 0과 fresh AssetDump persisted evidence를 기록했다.
- 기존 Arrow/Chevron2와 six-sector Designer ownership은 보존했고 `VehicleSilhouettes` count 0은 유지했다. 다음 Production Art 경계는 vehicle-specific silhouette이며 USER Visual은 Pending이다.

### v1.1.7 - 2026-08-25

- CF-FQ-039 modular Armor의 canonical `Arrow/Chevron2` 2종 Production Texture를 실제 저장하고 `DA_CFHUDVisual_Default`의 두 Direction Icon Soft Reference에 연결했다.
- final Official Build `8eed02112b73443a9c4df1d03d9a859b`, `CFUIArmorArt` Exit 0, fresh persisted AssetDump에서 2 Texture + 2 binding을 Technical PASS로 확인했다.
- `ArmorCommonPlate`와 VehicleData별 `VehicleSilhouettes` Production Art는 아직 비어 있으며, USER Designer-owned Sector 위치·크기·Icon 선택·회전과 USER Visual Gate는 Pending/보존했다.

### v1.1.6 - 2026-08-25

- CF-FQ-039 Armor modular 구조의 현재 구현을 Systems로 승격했다. `WBP_CFArmorSector`에 `Image_DirectionIcon`이 additive 저장됐고 BodyMap의 6개 Sector Designer Slot은 보존됐다.
- `UCFArmorSectorWidget v1.2.1` common/legacy Plate + DirectionIcon/fallback 계약과 `UCFHUDPresenter v1.26.1` VehicleData별 silhouette catalog 소비·stale Brush clear 계약을 Current로 기록했다.
- clean migration Exit 0, fresh AssetDump, final Official Build와 `CarFight.UI.UI_P0_06` 17/17을 Technical PASS로 기록했다. 실제 common Plate/Arrow/Chevron2/차량별 silhouette Production Art Import·Binding과 새 modular USER Visual PASS는 Pending이다.

### v1.1.5 - 2026-08-24

- Defense Bar final pixel fix와 test-harness 교정을 Current 구현으로 승격하고 final `VehicleDefenseBarVisualContract` 1/1 PASS를 반영했다.
- fresh `M_VehicleDefensePIE` Defense Pawn에서 Shield/Integrity가 각각 591x10 Bar로 동시에 실제 렌더되는 것을 Snapshot/Screenshot으로 확인해 Frame + Shield/Integrity를 AI Technical/Pixel PASS로 복구했다.
- Frame/Speed-RPM/Armor 보존과 Content mutation0을 확인했으며 USER Visual 품질 판정과 전체 `VT-VEH-01` 승격은 계속 Pending으로 유지했다.

### v1.1.4 - 2026-08-24

- fresh PIE pixel review에서 Integrity ProgressBar 소실을 발견해 Frame + Shield/Integrity Technical PASS를 재검증 Pending으로 교정했다.
- `CFHUDPresenter v1.25.1`의 Track/Fill/Marquee 10px intrinsic size final fix와 Official Build PASS를 Current 구현에 반영했다.
- exact focused rerun + fresh PIE pixel revalidation이 남아 있으므로 Defense Bar Technical PASS와 USER Visual Gate 승격은 보류했다. Armor USER PASS는 보존한다.

### v1.1.3 - 2026-08-24

- CF-FQ-039 Armor USER PASS와 Frame + Shield/Integrity Production Presentation Technical PASS를 Current 구현에 반영했다.
- 기존 P2 VehiclePanel Frame의 9-Slice persisted 소비를 확인하고 신규 Frame Content를 만들지 않았다.
- `CFHUDPresenter v1.25.0`의 기존 Shield/Integrity ProgressBar runtime styling과 semantic Shield/Integrity color 계약을 기록했다.
- final Build와 VehicleDefenseBar/DefenseRuntime/RPM focused regression PASS를 추가하되 Frame + Shield/Integrity USER Visual Review는 Pending으로 유지했다.

### v1.1.2 - 2026-08-24

- CF-FQ-039 Armor Production Presentation의 Current 구현을 반영했다.
- `CFArmorSectorWidget v1.1.1`의 icon-first/Label fallback과 Ratio 기반 Armor/Caution/Critical tint를 Current 계약으로 기록했다.
- final Official Build와 Armor/Defense/RPM focused regression PASS를 Current technical evidence에 추가했다.
- Content Asset mutation 0과 USER Visual Review Pending 경계를 명시해 Technical PASS를 Visual PASS로 확대하지 않았다.

### v1.1.1 - 2026-08-24

- CF-FQ-039 VehiclePanel editable production 준비 과정에서 persisted `WBP_CFArmorBodyMap`과 `WBP_CFArmorSector` 구조를 AssetDump로 재확인해 Current 구조로 기록했다.
- BodyMap이 탑다운 VehicleSilhouette + 동일 ArmorSector 6개 + 독립 Canvas Slot을 이미 사용하고 있음을 명시했다.
- 기존 `WBP_CFArmorSector`를 canonical reusable Armor element로 유지하고 신규 `WBP_CFArmorSlot` 중복 생성을 Current 구조에서 배제했다.
- 이 감사에서는 Source/Content mutation과 USER Visual PASS를 만들지 않았으며, 실제 Production Visual 목표는 CF-FQ-039 Plan에 남겼다.

### v1.1.0 - 2026-08-22

- post-closure code review remediation P1 2건과 P2 3건을 Current 계약으로 반영했다.
- Vehicle Target Identity를 `VehicleData.PrimaryAssetId.PrimaryAssetName` 기반으로 안정화하고 내부 Actor 이름을 TargetId/DisplayName fallback에서 제거했다.
- Alert Notice/Warning duration을 실제 첫 presentation 시점 기준으로 교정해 높은 Priority suppression 중 수명 소모를 제거했다.
- Screen-edge const ViewData ref, HUD Refresh 단일 Sensor Snapshot/Range Profile, AimReticle 단일 per-frame refresh를 Current hot-path 계약으로 기록했다.
- final Build `bd3640c616794cd7a54cc8a8afa4b021`, focused 2/2 + 4/4 + 1/1 + 1/1, broad UI 44/44 + Sensor 14/14 PASS를 최신 code-review evidence로 추가했다. Content/Blueprint mutation0, USER Visual/Feel 상태 변화0이다.

### v1.0.0 - 2026-08-22

- CarFight 인게임 UI의 Current Systems 문서를 최초 생성했다.
- LocalPlayer `UCFUISubsystem -> UCFUIRootWidget -> 8 Layer` 소유권을 기록했다.
- Production HUD의 `UCFHUDDataProvider -> FCFInGameUIViewData -> UCFHUDPresenter` 경계를 기록했다.
- AimReticle HUD Layer singleton/Rebind, Target Marker Game Layer singleton/Rebind, Pause Menu Menu Layer 수명을 현재 구현으로 기록했다.
- Radar Range/Zoom, Target Screen-edge, Weapon/Ammo/Heat/Charge, ViewMode/Alert/Style의 현재 기술 계약과 미구현 Runtime fail-closed 경계를 기록했다.
- UI-P0-10 Build/Automation/fresh PIE RuntimeRead를 Current 기술 evidence로 연결하고 USER Visual/Feel과 분리했다.

---

## 16. 마지막 확인 기준

- 확인 일시: 2026-08-22
- 기준 엔진: Unreal Engine 5.8 Source Build
- 주요 Source:
  - `UE/Source/CarFight_Re/Public/UI/CFUISubsystem.h` v1.12.0
  - `UE/Source/CarFight_Re/Private/UI/CFUISubsystem.cpp` v1.12.0
  - `UE/Source/CarFight_Re/Public/UI/CFUIRootWidget.h` v1.0.2
  - `UE/Source/CarFight_Re/Private/UI/CFUIRootWidget.cpp` v1.0.2
    - `UE/Source/CarFight_Re/Public/UI/CFHUDDataProvider.h` v1.10.0
  - `UE/Source/CarFight_Re/Private/UI/CFHUDDataProvider.cpp` v1.16.0
  - `UE/Source/CarFight_Re/Public/UI/CFHUDPresenter.h` v1.23.0
  - `UE/Source/CarFight_Re/Private/UI/CFHUDPresenter.cpp` v1.24.0
  - `UE/Source/CarFight_Re/Public/UI/CFHUDViewData.h` v1.14.0
  - `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h` v1.9.0
  - `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp` v1.10.0
  - `UE/Source/CarFight_Re/Public/UI/CFTargetSelectWidget.h` v1.3.0
  - `UE/Source/CarFight_Re/Private/UI/CFTargetSelectWidget.cpp` v1.4.0
  - `UE/Source/CarFight_Re/Public/CFTargetSelectable.h` v1.1.0
  - `UE/Source/CarFight_Re/Private/CFTargetSelectable.cpp` v1.2.0
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h` v2.154.0
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp` v2.156.0
  - `UE/Source/CarFight_Re/Private/CFHUDDataTests.cpp` v1.35.0
  - `UE/Source/CarFight_Re/Private/UI/CFHUDPresenter.cpp` v1.26.1
  - `UE/Source/CarFight_Re/Public/UI/CFArmorSectorWidget.h` v1.2.1
  - `UE/Source/CarFight_Re/Private/UI/CFArmorSectorWidget.cpp` v1.2.1
  - `UE/Source/CarFight_Re/Public/UI/CFUIHUDProdEditorBridge.h` v1.12.1
  - `UE/Source/CarFight_Re/Private/UI/CFUIHUDProdEditorBridge.cpp` v1.15.1
  - `UE/Source/CarFight_Re/Public/CFPlayerController.h` v1.4.0
  - `UE/Config/DefaultGame.ini`
- 대표 Persisted UI:
  - `/Game/CarFight/UI/HUD/WBP_CFInGameHUD`
  - `/Game/CarFight/UI/WBP_AimReticle`
  - `/Game/CarFight/UI/WBP_TargetSelect`
