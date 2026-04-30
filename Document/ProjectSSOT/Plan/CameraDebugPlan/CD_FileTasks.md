# Camera Debug 파일별 작업 목록

- Version: 0.1.0
- Date: 2026-04-30
- Status: Draft
- Scope: Camera 디버그 카테고리를 현재 VehicleDebug Panel 구조에 편입하기 위한 파일 단위 작업 지시서

---

## 1. 목적

이 문서는 Camera Debug 편입 작업을 실제 수정 파일 기준으로 쪼갠다.

목표는 아래다.

```text
CameraComp RuntimeState
  -> VehicleDebug Camera Category
  -> PanelViewData Camera Section
  -> Navigation Camera Item
  -> Selected Camera Content
```

---

## 2. 수정 대상 파일 요약

| 순서 | 파일 | 작업 성격 |
|---|---|---|
| 1 | `CFVehiclePawn.h` | Camera Debug 데이터 타입과 Getter 선언 추가 |
| 2 | `CFVehiclePawn.cpp` | Camera Debug 데이터 채우기 구현 |
| 3 | `CFVehicleDebugPanelWidget.h` | CachedCamera와 Builder 선언 추가 |
| 4 | `CFVehicleDebugPanelWidget.cpp` | Camera Section ViewData 생성 및 Panel 편입 |
| 5 | WBP 자산 | 원칙적으로 수정 없음. 연결이 누락된 경우만 수동 확인 |

---

## 3. `CFVehiclePawn.h` 작업

경로:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
```

### 3.1 include 확인

`FCFVehicleCameraRuntimeState`를 사용해야 하므로 `CFVehicleCameraTypes.h`가 include되어 있는지 확인한다.

없다면 추가한다.

```cpp
#include "CFVehicleCameraTypes.h"
```

주의:
- 이미 다른 경로를 통해 include되어 있으면 중복 추가하지 않는다.
- Unreal Header Tool이 구조체를 읽을 수 있는 위치여야 한다.

---

### 3.2 `FCFVehicleDebugCamera` 구조체 추가

`FCFVehicleDebugRuntime` 아래, `FCFVehicleDebugSnapshot` 위에 추가하는 것을 권장한다.

```cpp
/**
 * VehicleDebug 카메라 상세 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugCamera
{
    GENERATED_BODY()

    // [v2.15.0] VehicleCameraComp 보유 여부입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Camera", meta=(DisplayName="VehicleCameraComp 보유 여부 (bHasVehicleCameraComponent)", ToolTip="현재 Pawn이 VehicleCameraComp를 보유하고 있는지 여부입니다."))
    bool bHasVehicleCameraComponent = false;

    // [v2.15.0] VehicleCameraComp가 제공하는 원본 카메라 런타임 스냅샷입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Camera", meta=(DisplayName="카메라 런타임 상태 (CameraRuntimeState)", ToolTip="VehicleCameraComp가 계산한 현재 카메라 런타임 스냅샷입니다."))
    FCFVehicleCameraRuntimeState CameraRuntimeState;

    // [v2.15.0] 목표 Arm 길이 대비 실제 해결 Arm 길이 비율입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Camera", meta=(DisplayName="충돌 압축 비율 (CollisionCompressionRatio)", ToolTip="DesiredArmLength 대비 SolvedArmLength 비율입니다. 1에 가까울수록 압축이 적고, 0에 가까울수록 크게 눌린 상태입니다."))
    float CollisionCompressionRatio = 1.0f;

    // [v2.15.0] 카메라가 충돌 등으로 의미 있게 압축된 상태인지 여부입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Camera", meta=(DisplayName="충돌 압축 상태 (bCameraCompressedByCollision)", ToolTip="True이면 현재 카메라가 목표 Arm 길이보다 의미 있게 짧아진 상태입니다."))
    bool bCameraCompressedByCollision = false;
};
```

---

### 3.3 `FCFVehicleDebugSnapshot`에 Camera 필드 추가

`Input`과 `Runtime` 사이 또는 `Runtime` 앞에 추가한다.
권장 순서는 Panel 표시 순서와 맞춰 `Input` 다음이다.

```cpp
// [v2.15.0] 카메라 상세 카테고리입니다.
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Camera 카테고리 (Camera)", ToolTip="차량 카메라 런타임 상태와 충돌 압축 정보를 담는 VehicleDebug Camera 카테고리입니다."))
FCFVehicleDebugCamera Camera;
```

---

### 3.4 `GetVehicleDebugCamera()` 선언 추가

기존 Getter들 근처에 추가한다.

위치 권장:

```text
GetVehicleDebugInput()
GetVehicleDebugCamera()
GetVehicleDebugRuntime()
```

선언:

```cpp
UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="상세 패널 표시용 VehicleDebug Camera 카테고리를 반환합니다."))
FCFVehicleDebugCamera GetVehicleDebugCamera() const;
```

---

## 4. `CFVehiclePawn.cpp` 작업

경로:

```text
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

### 4.1 `GetVehicleDebugSnapshot()`에 Camera 채우기 추가

현재 `GetVehicleDebugSnapshot()`는 다음을 직접 채운다.

- Runtime
- Input
- Overview
- Drive

여기에 Camera를 추가한다.

권장 위치:

```text
Input 채우기 이후
Overview 채우기 이전 또는 이후
Drive 처리와 독립된 위치
```

권장 구현:

```cpp
// [v2.15.0] Camera 카테고리 채우기입니다.
DebugSnapshot.Camera.bHasVehicleCameraComponent = (VehicleCameraComp != nullptr);

if (DebugSnapshot.Camera.bHasVehicleCameraComponent)
{
    DebugSnapshot.Camera.CameraRuntimeState = VehicleCameraComp->GetCameraRuntimeState();

    const float DesiredArmLength = DebugSnapshot.Camera.CameraRuntimeState.DesiredArmLength;
    const float SolvedArmLength = DebugSnapshot.Camera.CameraRuntimeState.SolvedArmLength;

    if (DesiredArmLength > KINDA_SMALL_NUMBER)
    {
        DebugSnapshot.Camera.CollisionCompressionRatio = FMath::Clamp(SolvedArmLength / DesiredArmLength, 0.0f, 1.0f);
    }
    else
    {
        DebugSnapshot.Camera.CollisionCompressionRatio = 1.0f;
    }

    DebugSnapshot.Camera.bCameraCompressedByCollision = DebugSnapshot.Camera.CollisionCompressionRatio < 0.90f;
}
```

주의:
- `VehicleCameraComp`가 null이면 기본값을 유지한다.
- 여기서 `CameraRuntimeState`를 새로 계산하지 않는다.
- 카메라 디버그 계산은 표시용 파생값 정도로 제한한다.

---

### 4.2 `GetVehicleDebugCamera()` 구현 추가

기존 카테고리 Getter 근처에 추가한다.

```cpp
FCFVehicleDebugCamera ACFVehiclePawn::GetVehicleDebugCamera() const
{
    // [v2.15.0] 상세 패널이 필요한 Camera 카테고리만 직접 읽을 수 있도록 반환합니다.
    return GetVehicleDebugSnapshot().Camera;
}
```

---

## 5. `CFVehicleDebugPanelWidget.h` 작업

경로:

```text
UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h
```

### 5.1 CachedCamera 추가

기존 캐시들 근처에 추가한다.

예상 위치:

```cpp
FCFVehicleDebugOverview CachedOverview;
FCFVehicleDebugDrive CachedDrive;
FCFVehicleDebugInput CachedInput;
FCFVehicleDebugRuntime CachedRuntime;
```

권장 추가:

```cpp
// [v1.8.0] 현재 Panel에 적용 중인 Camera 카테고리 캐시입니다.
FCFVehicleDebugCamera CachedCamera;
```

주의:
- 이 타입을 쓰려면 `CFVehiclePawn.h`에서 선언된 타입이 보이는지 확인한다.
- 기존 파일이 이미 `CFVehiclePawn.h`를 include하고 있으면 별도 include가 필요 없을 수 있다.

---

### 5.2 Builder 선언 추가

기존 Builder 선언 근처에 추가한다.

```cpp
// [v1.8.0] Camera 카테고리용 Section ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> BuildCameraSectionViewData(const FCFVehicleDebugCamera& InCamera) const;
```

권장 위치:

```text
BuildInputSectionViewData()
BuildCameraSectionViewData()
BuildRuntimeSectionViewData()
```

---

## 6. `CFVehicleDebugPanelWidget.cpp` 작업

경로:

```text
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
```

### 6.1 `RefreshFromPawn()`에서 Camera 읽기

현재 흐름:

```cpp
const FCFVehicleDebugOverview LatestOverview = VehiclePawnRef->GetVehicleDebugOverview();
const FCFVehicleDebugDrive LatestDrive = VehiclePawnRef->GetVehicleDebugDrive();
const FCFVehicleDebugInput LatestInput = VehiclePawnRef->GetVehicleDebugInput();
const FCFVehicleDebugRuntime LatestRuntime = VehiclePawnRef->GetVehicleDebugRuntime();
```

권장 추가:

```cpp
const FCFVehicleDebugCamera LatestCamera = VehiclePawnRef->GetVehicleDebugCamera();
```

캐시 반영:

```cpp
CachedCamera = LatestCamera;
```

권장 위치:

```text
LatestInput 다음
LatestRuntime 전 또는 직후
CachedInput 다음
CachedRuntime 전 또는 직후
```

Panel 표시 순서와 맞춰 Input 다음, Runtime 전을 추천한다.

---

### 6.2 `BuildVehicleDebugPanelViewData()`에 Camera 추가

현재 흐름:

```cpp
PanelViewData.AddTopLevelSection(BuildOverviewSectionViewData(CachedOverview));
PanelViewData.AddTopLevelSection(BuildDriveSectionViewData(CachedDrive));
PanelViewData.AddTopLevelSection(BuildInputSectionViewData(CachedInput));
PanelViewData.AddTopLevelSection(BuildRuntimeSectionViewData(CachedRuntime));
```

권장 변경:

```cpp
PanelViewData.AddTopLevelSection(BuildOverviewSectionViewData(CachedOverview));
PanelViewData.AddTopLevelSection(BuildDriveSectionViewData(CachedDrive));
PanelViewData.AddTopLevelSection(BuildInputSectionViewData(CachedInput));
PanelViewData.AddTopLevelSection(BuildCameraSectionViewData(CachedCamera));
PanelViewData.AddTopLevelSection(BuildRuntimeSectionViewData(CachedRuntime));
```

---

### 6.3 `BuildCameraSectionViewData()` 구현 추가

구현 위치:

```text
BuildInputSectionViewData() 다음
BuildRuntimeSectionViewData() 전
```

핵심 구현 흐름:

```cpp
TSharedRef<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::BuildCameraSectionViewData(const FCFVehicleDebugCamera& InCamera) const
{
    const FCFVehicleCameraRuntimeState& CameraState = InCamera.CameraRuntimeState;

    TSharedRef<FCFVehicleDebugSectionViewData> CameraSectionViewData =
        FCFVehicleDebugSectionViewData::MakeSection(TEXT("Camera"), TEXT("Camera"), ECFVehicleDebugSectionKind::Category, true);

    CameraSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Vehicle;
    CameraSectionViewData->NavigationOrder = 30;
    CameraSectionViewData->bShowInNavigation = true;

    if (CameraState.bAimBlocked)
    {
        CameraSectionViewData->BadgeText = TEXT("Blocked");
    }
    else if (InCamera.bCameraCompressedByCollision)
    {
        CameraSectionViewData->BadgeText = TEXT("Compressed");
    }
    else if (CameraState.bAimAtYawLimit || CameraState.bAimAtPitchLimit)
    {
        CameraSectionViewData->BadgeText = TEXT("Limit");
    }

    // 상위 요약 필드 추가
    // Aim / View / Trace / Mode 하위 Section 추가

    return CameraSectionViewData;
}
```

---

### 6.4 Field 포맷 규칙

float 값은 처음에는 단순 문자열 포맷을 사용한다.

권장 예:

```cpp
FString::Printf(TEXT("%.1f"), Value)
FString::Printf(TEXT("%.2f"), Ratio)
FString::Printf(TEXT("(%.1f, %.1f, %.1f)"), Vector.X, Vector.Y, Vector.Z)
```

bool 값은 현재 패널 기존 스타일과 맞춰 `True / False` 또는 `On / Off` 중 하나로 통일한다.
현재 Runtime/Overview 쪽은 `True / False`를 쓰므로 Camera도 `True / False`를 권장한다.

---

## 7. WBP 작업

원칙적으로 새 WBP 작업은 없다.

현재 구조가 정상이라면 아래가 이미 존재해야 한다.

- `WBP_VehicleDebugPanel`
- `VerticalBox_NavHost`
- `VerticalBox_SelectedSectionHost`
- `NavItemWidgetClass = WBP_VehicleDebugNavItem`
- `DynamicSectionWidgetClass = WBP_VehicleDebugSection`
- `WBP_VehicleDebugSection`
- `WBP_VehicleDebugFieldRow`
- `WBP_VehicleDebugNavItem`

### 수동 확인만 필요한 경우

Camera가 Navigation에 안 보이면 아래를 확인한다.

1. `VerticalBox_NavHost` 이름이 정확한지
2. `NavItemWidgetClass`가 지정되어 있는지
3. `CameraSectionViewData->bShowInNavigation`이 true인지
4. `CameraSectionViewData->bIsVisible`이 true인지
5. `CameraSectionViewData->SectionId`가 비어 있지 않은지

Camera를 눌러도 오른쪽에 안 보이면 아래를 확인한다.

1. `VerticalBox_SelectedSectionHost` 이름이 정확한지
2. `DynamicSectionWidgetClass`가 지정되어 있는지
3. `SelectedSectionId`가 `Camera`로 바뀌는지
4. `FindSelectedSectionViewData()`가 Camera를 찾는지

---

## 8. 구현 순서

1. `CFVehiclePawn.h`에 `FCFVehicleDebugCamera` 추가
2. `FCFVehicleDebugSnapshot`에 `Camera` 필드 추가
3. `GetVehicleDebugCamera()` 선언 추가
4. `CFVehiclePawn.cpp`에서 Camera Snapshot 채우기
5. `GetVehicleDebugCamera()` 구현 추가
6. `CFVehicleDebugPanelWidget.h`에 `CachedCamera` 추가
7. `BuildCameraSectionViewData()` 선언 추가
8. `CFVehicleDebugPanelWidget.cpp`에서 Camera 카테고리 읽기
9. `BuildVehicleDebugPanelViewData()`에 Camera Section 추가
10. `BuildCameraSectionViewData()` 구현
11. 빌드
12. PIE에서 Navigation / Selected Section 확인

---

## 9. 예상 빌드 이슈

### 이슈 1: `FCFVehicleCameraRuntimeState` 알 수 없음

원인:
- `CFVehicleCameraTypes.h` include 누락

해결:
- `CFVehiclePawn.h` 또는 타입 분리 파일에 include 추가

### 이슈 2: `FCFVehicleDebugCamera` 알 수 없음

원인:
- PanelWidget 헤더에서 타입 선언을 볼 수 없음

해결:
- 기존처럼 `CFVehiclePawn.h`를 include하거나 타입 정의를 별도 파일로 분리

1차에서는 기존 구조를 따라 `CFVehiclePawn.h` 내 정의 유지가 안전하다.

### 이슈 3: `ECFVehicleDebugNavGroup::Vehicle` 없음

원인:
- 현재 enum에 Vehicle 그룹이 없거나 이름이 다름

해결:
- `CFDebugPanelViewData.h`의 `ECFVehicleDebugNavGroup` 실제 항목을 확인한다.
- Vehicle이 있으면 그대로 사용한다.
- 없으면 Camera를 일단 `Core` 또는 가장 가까운 그룹에 배치하고 문서에 기록한다.

---

## 10. 완료 기준

- 컴파일 성공
- Navigation에 Camera 항목 표시
- Camera 선택 가능
- Camera 선택 시 오른쪽 Selected Section에 Camera 표시
- FieldRow로 Camera 상위 필드 표시
- Aim / View / Trace / Mode 하위 Section 표시
- 기존 Overview / Drive / Input / Runtime 표시 회귀 없음

---

## 11. Changelog

### v0.1.0 - 2026-04-30
- Camera Debug 편입 작업을 파일 단위로 분해한 작업 지시서를 신규 작성했다.
