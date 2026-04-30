# Camera Debug 체크리스트

- Version: 0.1.0
- Date: 2026-04-30
- Status: Draft
- Scope: Camera Debug 편입 작업 누락 방지용 체크리스트

---

## 1. 데이터 계층

- [ ] `CFVehicleCameraTypes.h` include 필요 여부를 확인했다.
- [ ] `FCFVehicleDebugCamera` 구조체를 추가했다.
- [ ] `FCFVehicleDebugCamera`는 `USTRUCT(BlueprintType)`이다.
- [ ] `bHasVehicleCameraComponent` 필드를 추가했다.
- [ ] `CameraRuntimeState` 필드를 추가했다.
- [ ] `CollisionCompressionRatio` 필드를 추가했다.
- [ ] `bCameraCompressedByCollision` 필드를 추가했다.
- [ ] 각 필드에 `UPROPERTY`와 ToolTip을 추가했다.
- [ ] `FCFVehicleDebugSnapshot`에 `Camera` 필드를 추가했다.
- [ ] `GetVehicleDebugCamera()`를 선언했다.

---

## 2. Pawn 구현

- [ ] `GetVehicleDebugSnapshot()`에서 `VehicleCameraComp` 유효성을 확인한다.
- [ ] `VehicleCameraComp`가 유효하면 `GetCameraRuntimeState()`를 읽는다.
- [ ] `DesiredArmLength > 0`일 때만 압축 비율을 계산한다.
- [ ] 압축 비율은 0~1 범위로 Clamp한다.
- [ ] `DesiredArmLength <= 0`이면 압축 비율을 1.0으로 둔다.
- [ ] `bCameraCompressedByCollision` 기준값을 0.90으로 둔다.
- [ ] `GetVehicleDebugCamera()`를 구현했다.
- [ ] `VehicleCameraComp == nullptr`일 때도 안전하게 기본값이 반환된다.

---

## 3. Panel Widget 헤더

- [ ] `CachedCamera`를 추가했다.
- [ ] `BuildCameraSectionViewData()` 선언을 추가했다.
- [ ] 선언 위치가 기존 Builder 흐름과 맞다.
- [ ] 필요한 타입 include가 정리되어 있다.

---

## 4. Panel Widget 구현

- [ ] `RefreshFromPawn()`에서 `GetVehicleDebugCamera()`를 호출한다.
- [ ] `CachedCamera`에 최신 값을 저장한다.
- [ ] `BuildVehicleDebugPanelViewData()`에 Camera Section을 추가했다.
- [ ] Camera Section 순서는 `Input` 다음, `Runtime` 전이다.
- [ ] `BuildCameraSectionViewData()`를 구현했다.
- [ ] Camera SectionId는 `Camera`다.
- [ ] Camera TitleText는 `Camera`다.
- [ ] Camera SectionKind는 `Category`다.
- [ ] `NavigationGroup`을 지정했다.
- [ ] `NavigationOrder`를 지정했다.
- [ ] `bShowInNavigation`이 true다.
- [ ] Badge 우선순위가 `Blocked > Compressed > Limit`이다.

---

## 5. Camera Section 필드

### 상위 Camera 필드

- [ ] Has CameraComp
- [ ] Mode
- [ ] Aim Profile
- [ ] Aim Blocked
- [ ] Can Fire
- [ ] Compression
- [ ] Current FOV

### Aim 하위 Section

- [ ] Accum Yaw
- [ ] Accum Pitch
- [ ] Clamped Yaw
- [ ] Clamped Pitch
- [ ] At Yaw Limit
- [ ] At Pitch Limit

### View 하위 Section

- [ ] Desired Arm
- [ ] Current Arm
- [ ] Solved Arm
- [ ] Desired FOV
- [ ] Current FOV
- [ ] Compression

### Trace 하위 Section

- [ ] Aim Blocked
- [ ] Can Fire
- [ ] Trace Distance
- [ ] Hit Location

### Mode 하위 Section

- [ ] Current
- [ ] Previous
- [ ] Changed This Frame
- [ ] Aim Profile

---

## 6. WBP / UMG 확인

원칙적으로 새 WBP 수정은 하지 않는다.

- [ ] `WBP_VehicleDebugPanel`에 Camera 전용 버튼을 추가하지 않았다.
- [ ] `WBP_VehicleDebugPanel` 이벤트 그래프에 Camera 전용 OnClicked를 추가하지 않았다.
- [ ] `WBP_VehicleDebugSection`을 Camera 전용으로 복제하지 않았다.
- [ ] `WBP_VehicleDebugFieldRow`를 Camera 전용으로 복제하지 않았다.
- [ ] `VerticalBox_NavHost`가 있다.
- [ ] `VerticalBox_SelectedSectionHost`가 있다.
- [ ] `NavItemWidgetClass`가 지정되어 있다.
- [ ] `DynamicSectionWidgetClass`가 지정되어 있다.

---

## 7. 빌드 검증

- [ ] `CarFight_ReEditor Win64 Development` 빌드가 성공했다.
- [ ] Unreal Header Tool 오류가 없다.
- [ ] include 오류가 없다.
- [ ] 선언/정의 불일치가 없다.
- [ ] enum 그룹 이름 오류가 없다.

---

## 8. PIE 검증

- [ ] `bEnableDriveStateOnScreenDebug = true` 상태에서 Panel이 보인다.
- [ ] `bShowVehicleDebugPanel = true` 상태에서 Panel이 보인다.
- [ ] Navigation에 Camera 항목이 자동 표시된다.
- [ ] Camera 항목을 클릭할 수 있다.
- [ ] Camera 클릭 시 선택 표시가 바뀐다.
- [ ] Camera 클릭 시 오른쪽 Selected Content가 Camera로 바뀐다.
- [ ] 전체 Section이 동시에 나열되지 않는다.
- [ ] Camera 상위 Field가 보인다.
- [ ] Aim 하위 Section이 보인다.
- [ ] View 하위 Section이 보인다.
- [ ] Trace 하위 Section이 보인다.
- [ ] Mode 하위 Section이 보인다.

---

## 9. 동작 검증

- [ ] 시점 입력 시 Aim Yaw/Pitch 값이 변한다.
- [ ] 제한각 근처에서 Yaw/Pitch Limit 값이 변한다.
- [ ] 벽 근처에서 Solved Arm이 Desired Arm보다 작아진다.
- [ ] 벽 근처에서 Compression 값이 낮아진다.
- [ ] 의미 있는 압축에서 Badge가 `Compressed`로 표시된다.
- [ ] Aim Blocked 상황에서 Badge가 `Blocked`로 표시된다.
- [ ] 열린 공간에서 Trace Distance가 길어진다.
- [ ] Hit Location이 상황에 따라 변한다.

---

## 10. 회귀 검증

- [ ] Overview가 여전히 선택 가능하다.
- [ ] Drive가 여전히 선택 가능하다.
- [ ] Input이 여전히 선택 가능하다.
- [ ] Runtime이 여전히 선택 가능하다.
- [ ] 기존 FieldRow 표시가 깨지지 않았다.
- [ ] 기존 하위 Section 표시가 깨지지 않았다.
- [ ] Navigation 선택 색상이 유지된다.
- [ ] Panel 상호작용 모드가 유지된다.

---

## 11. 문서 반영

- [ ] `CameraDebugPlan` 문서가 생성되어 있다.
- [ ] 설계 문서가 현재 구조 기준과 맞다.
- [ ] 파일별 작업 문서가 실제 수정 대상과 맞다.
- [ ] 검증 가이드가 실제 Panel 구조와 맞다.
- [ ] 구현 완료 후 필요한 경우 `VehicleDebugPlan` 쪽 문서에도 Camera 카테고리 추가 사실을 반영한다.

---

## 12. Changelog

### v0.1.0 - 2026-04-30
- Camera Debug 편입 작업용 체크리스트를 신규 작성했다.
