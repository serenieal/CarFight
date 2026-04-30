# Camera Debug 설계안

- Version: 0.1.0
- Date: 2026-04-30
- Status: Draft
- Scope: 현재 VehicleDebug Navigation + Selected Section 구조에 Camera 디버그 카테고리를 편입하기 위한 설계 기준

---

## 1. 목적

Camera 디버그는 별도 HUD나 별도 WBP가 아니라, 현재 VehicleDebug Panel의 새 TopLevel Section으로 편입한다.

현재 Panel은 `VerticalBox_DynamicSectionHost`에 모든 섹션을 나열하는 구조가 아니라, 아래 구조를 기본으로 한다.

```text
PanelViewData.TopLevelSectionArray
  -> Navigation Item 자동 생성
  -> SelectedSectionId 선택
  -> VerticalBox_SelectedSectionHost에 선택 Section 하나 표시
```

`VerticalBox_DynamicSectionHost`는 Selected Section 구조가 준비되지 않았을 때만 쓰는 fallback으로 본다.

---

## 2. 현재 구조 기준

### VehicleDebug 흐름

```text
ACFVehiclePawn
  -> FCFVehicleDebugSnapshot
  -> GetVehicleDebugOverview / Drive / Input / Runtime
  -> UCFVehicleDebugPanelWidget
  -> FCFVehicleDebugPanelViewData
  -> FCFVehicleDebugSectionViewData
  -> FCFVehicleDebugNavItemViewData
  -> WBP_VehicleDebugNavItem
  -> WBP_VehicleDebugSection
  -> WBP_VehicleDebugFieldRow
```

### Camera 쪽 준비 상태

`UCFVehicleCameraComp`에는 `GetCameraRuntimeState()`가 있고, `FCFVehicleCameraRuntimeState`에는 아래 계열 값이 이미 있다.

- Current / Previous CameraMode
- ActiveAimProfileName
- Accumulated / Clamped Aim Yaw, Pitch
- bAimAtYawLimit, bAimAtPitchLimit
- bAimBlocked, bWeaponCanFireAtCurrentAim
- DesiredArmLength, CurrentArmLength, SolvedArmLength
- DesiredFOV, CurrentFOV
- AimHitLocation, AimTraceDistance

따라서 CameraDebug는 새 계산기를 만들기보다 이 Snapshot을 VehicleDebug 카테고리로 감싸는 방식이 맞다.

---

## 3. 새 데이터 구조

### `FCFVehicleDebugCamera`

권장 위치:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
```

권장 필드:

```cpp
// VehicleCameraComp 보유 여부입니다.
bool bHasVehicleCameraComponent = false;

// VehicleCameraComp가 제공하는 원본 카메라 런타임 스냅샷입니다.
FCFVehicleCameraRuntimeState CameraRuntimeState;

// DesiredArmLength 대비 SolvedArmLength 비율입니다.
float CollisionCompressionRatio = 1.0f;

// 충돌 등으로 카메라가 의미 있게 압축된 상태인지 여부입니다.
bool bCameraCompressedByCollision = false;
```

### Compression 계산 기준

```cpp
const float DesiredArmLength = CameraRuntimeState.DesiredArmLength;
const float SolvedArmLength = CameraRuntimeState.SolvedArmLength;

if (DesiredArmLength > KINDA_SMALL_NUMBER)
{
    CollisionCompressionRatio = FMath::Clamp(SolvedArmLength / DesiredArmLength, 0.0f, 1.0f);
}
else
{
    CollisionCompressionRatio = 1.0f;
}

bCameraCompressedByCollision = CollisionCompressionRatio < 0.90f;
```

---

## 4. Snapshot 편입

`FCFVehicleDebugSnapshot`에 `Camera`를 추가한다.

```cpp
// [v2.15.0] 카메라 상세 카테고리입니다.
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Camera 카테고리 (Camera)", ToolTip="차량 카메라 런타임 상태와 충돌 압축 정보를 담는 VehicleDebug Camera 카테고리입니다."))
FCFVehicleDebugCamera Camera;
```

Camera는 `Overview / Drive / Input / Runtime`과 같은 레벨의 정식 카테고리다.

---

## 5. Pawn API

추가 공개 API:

```cpp
UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="상세 패널 표시용 VehicleDebug Camera 카테고리를 반환합니다."))
FCFVehicleDebugCamera GetVehicleDebugCamera() const;
```

1차 구현에서는 현재 패턴에 맞춰 `GetVehicleDebugSnapshot()` 안에서 Camera를 채운다.
장기적으로는 `BuildVehicleDebugCamera()` 내부 함수로 분리한다.

---

## 6. Panel 편입

`UCFVehicleDebugPanelWidget`에 `CachedCamera`를 추가한다.

```cpp
FCFVehicleDebugCamera CachedCamera;
```

`RefreshFromPawn()`에서 `VehiclePawnRef->GetVehicleDebugCamera()`를 읽어 캐시한다.

그리고 아래 Builder를 추가한다.

```cpp
TSharedRef<FCFVehicleDebugSectionViewData> BuildCameraSectionViewData(const FCFVehicleDebugCamera& InCamera) const;
```

`BuildVehicleDebugPanelViewData()`에는 `Camera`를 TopLevelSection으로 추가한다.

권장 순서:

```text
Overview
Drive
Input
Camera
Runtime
```

---

## 7. Camera Navigation 메타데이터

Camera TopLevel Section은 Navigation에 자동 표시되어야 한다.

권장값:

```cpp
CameraSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Vehicle;
CameraSectionViewData->NavigationOrder = 30;
CameraSectionViewData->bShowInNavigation = true;
```

권장 Badge 우선순위:

1. `bAimBlocked == true` -> `Blocked`
2. `bCameraCompressedByCollision == true` -> `Compressed`
3. `bAimAtYawLimit || bAimAtPitchLimit` -> `Limit`
4. 정상 -> 빈 문자열

---

## 8. Camera Section 구성

### 상위 Camera 필드

| FieldId | Label | 값 |
|---|---|---|
| `camera_has_comp` | Has CameraComp | bHasVehicleCameraComponent |
| `camera_mode` | Mode | CurrentCameraMode |
| `camera_profile` | Aim Profile | ActiveAimProfileName |
| `camera_aim_blocked` | Aim Blocked | bAimBlocked |
| `camera_can_fire` | Can Fire | bWeaponCanFireAtCurrentAim |
| `camera_compression` | Compression | CollisionCompressionRatio |
| `camera_current_fov` | Current FOV | CurrentFOV |

### 하위 Section: `CameraAim`

| FieldId | Label | 값 |
|---|---|---|
| `camera_aim_accum_yaw` | Accum Yaw | AccumulatedAimYaw |
| `camera_aim_accum_pitch` | Accum Pitch | AccumulatedAimPitch |
| `camera_aim_clamped_yaw` | Clamped Yaw | ClampedAimYaw |
| `camera_aim_clamped_pitch` | Clamped Pitch | ClampedAimPitch |
| `camera_aim_yaw_limit` | At Yaw Limit | bAimAtYawLimit |
| `camera_aim_pitch_limit` | At Pitch Limit | bAimAtPitchLimit |

### 하위 Section: `CameraView`

| FieldId | Label | 값 |
|---|---|---|
| `camera_view_desired_arm` | Desired Arm | DesiredArmLength |
| `camera_view_current_arm` | Current Arm | CurrentArmLength |
| `camera_view_solved_arm` | Solved Arm | SolvedArmLength |
| `camera_view_desired_fov` | Desired FOV | DesiredFOV |
| `camera_view_current_fov` | Current FOV | CurrentFOV |
| `camera_view_compression` | Compression | CollisionCompressionRatio |

### 하위 Section: `CameraTrace`

| FieldId | Label | 값 |
|---|---|---|
| `camera_trace_blocked` | Aim Blocked | bAimBlocked |
| `camera_trace_can_fire` | Can Fire | bWeaponCanFireAtCurrentAim |
| `camera_trace_distance` | Trace Distance | AimTraceDistance |
| `camera_trace_hit` | Hit Location | AimHitLocation |

### 하위 Section: `CameraMode`

| FieldId | Label | 값 |
|---|---|---|
| `camera_mode_current` | Current | CurrentCameraMode |
| `camera_mode_previous` | Previous | PreviousCameraMode |
| `camera_mode_changed` | Changed This Frame | bCameraModeChangedThisFrame |
| `camera_mode_profile` | Aim Profile | ActiveAimProfileName |

---

## 9. HUD 승격 기준

1차에서는 HUD를 수정하지 않는다.

Panel에서 Camera Debug가 안정화된 뒤, 아래 후보 중 정말 자주 보는 값만 Overview/HUD로 승격한다.

- CurrentCameraMode
- bAimBlocked
- bWeaponCanFireAtCurrentAim
- CurrentFOV
- CollisionCompressionRatio

---

## 10. 금지 사항

- `WBP_VehicleDebugPanel`에 `Button_Camera`를 수동 추가하지 않는다.
- `WBP_VehicleDebugPanel` 이벤트 그래프에 Camera 선택 이벤트를 추가하지 않는다.
- Camera 전용 `WBP_VehicleDebugCameraPanel`을 만들지 않는다.
- Camera 표시용 TextBlock을 루트 Panel에 직접 추가하지 않는다.
- Camera 데이터 문자열을 BP 바인딩에서 직접 조립하지 않는다.
- `VerticalBox_DynamicSectionHost` 기준으로만 검증하지 않는다.

---

## 11. 완료 기준

- `FCFVehicleDebugCamera`가 추가되어 있다.
- `FCFVehicleDebugSnapshot.Camera`가 추가되어 있다.
- `GetVehicleDebugCamera()`가 BlueprintPure로 제공된다.
- `UCFVehicleDebugPanelWidget`이 Camera 카테고리를 캐시한다.
- `BuildCameraSectionViewData()`가 Camera TopLevel Section을 만든다.
- Camera Section에 Navigation 메타데이터가 지정되어 있다.
- Navigation에 Camera 항목이 자동 생성된다.
- Camera 선택 시 `VerticalBox_SelectedSectionHost`에 Camera Section 하나가 표시된다.
- Aim / View / Trace / Mode 하위 Section 값이 표시된다.
- 빌드가 성공한다.

---

## 12. Changelog

### v0.1.0 - 2026-04-30
- 현재 VehicleDebug Navigation + Selected Section 구조 기준으로 Camera 디버그 편입 설계를 신규 작성했다.
