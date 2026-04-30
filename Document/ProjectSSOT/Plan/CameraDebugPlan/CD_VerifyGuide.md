# Camera Debug 검증 가이드

- Version: 0.1.0
- Date: 2026-04-30
- Status: Draft
- Scope: Camera Debug 카테고리 편입 후 빌드, PIE, Panel UI 동작을 검증하기 위한 절차

---

## 1. 목적

이 문서는 Camera Debug 편입 후 무엇을 확인해야 하는지 정리한다.

Camera Debug는 현재 VehicleDebug Panel의 Navigation + Selected Section 구조를 사용해야 한다.
따라서 검증도 단순히 값이 생성되는지뿐 아니라, 아래 흐름이 끝까지 이어지는지 확인해야 한다.

```text
VehicleCameraComp
  -> VehicleDebug Camera Snapshot
  -> PanelViewData Camera Section
  -> Navigation Camera Item
  -> SelectedSectionHost Camera Content
```

---

## 2. 빌드 검증

### 2.1 기본 빌드

권장 빌드 대상:

```text
CarFight_ReEditor Win64 Development
```

성공 기준:

- Unreal Header Tool 오류 없음
- `FCFVehicleDebugCamera` 관련 타입 오류 없음
- `FCFVehicleCameraRuntimeState` include 오류 없음
- `GetVehicleDebugCamera()` 링크 오류 없음
- `BuildCameraSectionViewData()` 선언/정의 불일치 없음

---

## 3. C++ 정적 확인

### 3.1 `CFVehiclePawn.h`

확인 항목:

- `FCFVehicleDebugCamera` 구조체가 있다.
- `FCFVehicleDebugCamera`가 `USTRUCT(BlueprintType)`이다.
- 구조체 필드에 `UPROPERTY`와 ToolTip이 있다.
- `FCFVehicleDebugSnapshot`에 `Camera` 필드가 있다.
- `GetVehicleDebugCamera()`가 `BlueprintPure`로 선언되어 있다.

---

### 3.2 `CFVehiclePawn.cpp`

확인 항목:

- `GetVehicleDebugSnapshot()`에서 Camera 필드를 채운다.
- `VehicleCameraComp == nullptr`일 때 안전하다.
- `DesiredArmLength <= 0`일 때 0 나누기를 하지 않는다.
- `CollisionCompressionRatio`가 0~1 범위로 Clamp된다.
- `bCameraCompressedByCollision` 기준이 명확하다.
- `GetVehicleDebugCamera()`가 `GetVehicleDebugSnapshot().Camera`를 반환한다.

---

### 3.3 `CFVehicleDebugPanelWidget.h`

확인 항목:

- `CachedCamera`가 있다.
- `BuildCameraSectionViewData()` 선언이 있다.
- 선언 위치가 기존 `BuildInputSectionViewData()`와 `BuildRuntimeSectionViewData()` 흐름 사이에 있다.

---

### 3.4 `CFVehicleDebugPanelWidget.cpp`

확인 항목:

- `RefreshFromPawn()`에서 `GetVehicleDebugCamera()`를 호출한다.
- `CachedCamera`에 최신 Camera 카테고리를 저장한다.
- `BuildVehicleDebugPanelViewData()`에 Camera TopLevel Section이 추가되어 있다.
- `BuildCameraSectionViewData()`가 구현되어 있다.
- Camera Section에 NavigationGroup / NavigationOrder / bShowInNavigation이 지정되어 있다.
- BadgeText가 AimBlocked / Compressed / Limit 조건으로 설정된다.
- Camera 하위 Section들이 `AddChildSection()`으로 추가된다.

---

## 4. PIE 검증

## 4.1 기본 표시 조건

PIE 전에 확인할 값:

- `bEnableDriveStateOnScreenDebug = true`
- `bShowVehicleDebugPanel = true`
- `bShowVehicleDebugHud`는 이번 검증과 무관하다.
- `bShowVehicleDebugText`는 false여도 된다.

Panel이 안 보이면 먼저 위 값을 확인한다.

---

## 4.2 Navigation 검증

PIE 실행 후 확인한다.

성공 기준:

- 왼쪽 Navigation에 `Camera` 항목이 보인다.
- Camera 항목은 수동으로 WBP에 만든 버튼이 아니라 자동 생성된 NavItem이다.
- Camera 항목의 선택 상태가 기존 NavItem 스타일과 동일하게 표시된다.
- Camera 항목 클릭 시 `SelectedSectionId`가 `Camera`로 바뀐다.

실패 시 확인:

1. `CameraSectionViewData->bShowInNavigation`이 true인지
2. `CameraSectionViewData->bIsVisible`이 true인지
3. `CameraSectionViewData->SectionId`가 `Camera`인지
4. `VerticalBox_NavHost`가 WBP에 정확히 존재하는지
5. `NavItemWidgetClass`가 `WBP_VehicleDebugNavItem`인지

---

## 4.3 Selected Section 검증

Camera Navigation 항목을 클릭한다.

성공 기준:

- 오른쪽 Content 영역에 Camera Section 하나만 보인다.
- `Overview / Drive / Input / Runtime` 전체가 동시에 나열되지 않는다.
- `VerticalBox_SelectedSectionHost`에 Camera Section이 표시된다.
- `VerticalBox_DynamicSectionHost`는 숨겨져 있거나 fallback으로만 쓰인다.

실패 시 확인:

1. `VerticalBox_SelectedSectionHost` 이름이 정확한지
2. `DynamicSectionWidgetClass`가 지정되어 있는지
3. `FindSelectedSectionViewData()`가 Camera Section을 찾는지
4. `SelectedSectionWidget->SetSectionViewData()`가 호출되는지

---

## 5. Camera 값 검증

## 5.1 상위 Camera 필드

Camera Section에서 아래가 보이는지 확인한다.

- Has CameraComp
- Mode
- Aim Profile
- Aim Blocked
- Can Fire
- Compression
- Current FOV

성공 기준:

- `Has CameraComp`가 true로 나온다.
- `Mode`가 현재 CameraMode enum 문자열로 나온다.
- `Compression`이 0~1 범위로 나온다.
- `Current FOV`가 실제 카메라 FOV 변화와 유사하게 움직인다.

---

## 5.2 Aim 하위 Section

확인 필드:

- Accum Yaw
- Accum Pitch
- Clamped Yaw
- Clamped Pitch
- At Yaw Limit
- At Pitch Limit

테스트 방법:

1. 마우스/패드로 좌우 시점을 움직인다.
2. `Accum Yaw`와 `Clamped Yaw`가 변하는지 확인한다.
3. 제한각 끝까지 이동했을 때 `At Yaw Limit`가 true가 되는지 확인한다.
4. 상하 시점 이동도 같은 방식으로 확인한다.

---

## 5.3 View 하위 Section

확인 필드:

- Desired Arm
- Current Arm
- Solved Arm
- Desired FOV
- Current FOV
- Compression

테스트 방법:

1. 평지에서 주행한다.
2. 벽에 후방 카메라가 가까워지도록 차량을 붙인다.
3. 좁은 골목이나 기둥 근처에서 카메라가 압축되는지 본다.

성공 기준:

- 평지에서는 `Compression`이 1.0에 가깝다.
- 벽에 가까워지면 `Solved Arm`이 `Desired Arm`보다 작아진다.
- 의미 있게 눌리면 `Compression`이 0.9 아래로 내려간다.
- 충돌 해제 후 `Current Arm`과 `Solved Arm`이 자연스럽게 복귀한다.

---

## 5.4 Trace 하위 Section

확인 필드:

- Aim Blocked
- Can Fire
- Trace Distance
- Hit Location

테스트 방법:

1. 벽을 바라본다.
2. 차량 주변 장애물을 향해 조준한다.
3. 멀리 열린 공간을 바라본다.

성공 기준:

- 장애물에 가깝게 막히면 Aim Blocked가 true가 된다.
- 열린 공간에서는 Trace Distance가 커진다.
- Hit Location이 상황에 따라 변한다.

---

## 5.5 Mode 하위 Section

확인 필드:

- Current
- Previous
- Changed This Frame
- Aim Profile

현재는 Normal 중심이어도 된다.
후속 Reverse / Airborne / Destroyed 작업이 들어오면 이 Section에서 전환 검증을 한다.

---

## 6. Badge 검증

Camera Navigation 항목의 Badge를 확인한다.

권장 동작:

- Aim Blocked 상태: `Blocked`
- 충돌 압축 상태: `Compressed`
- 제한각 상태: `Limit`
- 정상 상태: 빈 배지

우선순위는 `Blocked > Compressed > Limit`이다.

---

## 7. 회귀 검증

Camera 추가 후 기존 카테고리를 확인한다.

- Overview 선택 가능
- Drive 선택 가능
- Input 선택 가능
- Runtime 선택 가능
- 각 카테고리 FieldRow 표시 유지
- 하위 Section 접기/펼치기 유지
- Navigation 선택 색상 유지
- Panel 바깥 클릭 시 상호작용 모드 종료 유지

---

## 8. 실패별 진단표

| 증상 | 우선 확인 |
|---|---|
| 빌드 실패 | include, 타입 선언 위치, UHT 오류 |
| Camera Nav 없음 | bShowInNavigation, NavHost, NavItemWidgetClass |
| Camera 클릭 안 됨 | NavItem 클릭 이벤트, RequestSelectThisNavItem |
| 클릭해도 Content 안 바뀜 | SelectedSectionId, FindSelectedSectionViewData |
| Camera 값 전부 0 | VehicleCameraComp null 여부, GetCameraRuntimeState 호출 |
| Compression 이상 | DesiredArmLength 0 처리, Solved/Desired 계산식 |
| 기존 카테고리 안 보임 | TopLevelSectionArray 생성 순서, SelectedSection fallback |

---

## 9. 완료 판정

아래를 모두 만족하면 1차 검증 완료다.

- 빌드 성공
- Navigation에 Camera 자동 표시
- Camera 선택 가능
- Selected Content에 Camera Section 표시
- 상위 Camera 필드 표시
- Aim / View / Trace / Mode 하위 Section 표시
- 벽 근처에서 Compression 변화 확인
- 기존 Overview / Drive / Input / Runtime 회귀 없음

---

## 10. Changelog

### v0.1.0 - 2026-04-30
- Camera Debug 편입 후 필요한 빌드/PIE/Panel 검증 절차를 신규 작성했다.
