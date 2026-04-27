# VehicleCamera

## 문서 목적
이 문서는 현재 프로젝트에서 `VehicleCamera` 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 자산/클래스/설정 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

## 문서 범위
이 문서에서 말하는 `VehicleCamera` 기능은 아래 요소를 묶어서 본다.

- 핵심 클래스: `UCFVehicleCameraComp`
- 관련 데이터:
  - `UCFVehicleCameraData`
  - `FCFVehicleCameraTuningConfig`
  - `FCFVehicleCameraAimProfile`
  - `FCFVehicleCameraModeFlags`
  - `FCFVehicleCameraRuntimeState`
- 현재 대표 소비 주체: `ACFVehiclePawn`
- 현재 대표 참조 자산/컴포넌트:
  - `CameraPivotRoot`
  - `CameraAimPivot`
  - `CameraBoom`
  - `FollowCamera`
- 핵심 함수:
  - `InitializeCameraRuntime()`
  - `ResolveCameraReferences()`
  - `EvaluateCameraMode()`
  - `BuildResolvedAimProfile()`
  - `UpdateAimState()`
  - `UpdateCameraTransform()`
  - `UpdateAimTrace()`

즉, 현재 기준 `VehicleCamera`는 **차량 기준 자유 조준, 카메라 모드 평가, SpringArm/FOV 보정, Aim Trace 계산을 묶은 차량 카메라 운영 기능**으로 본다.

## 이 기능이 현재 실제로 하는 일
현재 구현 기준 `VehicleCamera`의 핵심 역할은 **차량 이동과 카메라 회전을 분리한 상태에서, Look 입력을 누적해 차량 기준 조준 각도를 계산하고, 현재 카메라 모드와 Aim Profile에 따라 제한각/거리/FOV를 보정한 뒤, 이를 SpringArm과 FollowCamera에 반영하고 Aim Trace까지 계산하는 것**이다.

이 기능은 단순히 카메라를 따라붙게 만드는 수준이 아니다.
현재 구조상 `VehicleCamera`는 아래 다섯 가지 일을 동시에 한다.

### 1. 차량 기준 자유 조준 상태를 유지한다
현재 `VehicleCamera`는 Look 입력을 직접 받아 누적 조준 상태를 만든다.

현재 입력 관련 함수:
- `SetLookInput(FVector2D)`
- `AddLookInput(float, float)`
- `ClearLookInput()`
- `ResetAimToVehicleForward()`

현재 내부 상태:
- `PendingLookInput`
- `AccumulatedAimYaw`
- `AccumulatedAimPitch`

현재 `UpdateAimState()`는 아래 흐름으로 동작한다.

1. `PendingLookInput`를 읽는다.
2. `LookYawSpeedDegPerSec`, `LookPitchSpeedDegPerSec`로 각도 변화량을 만든다.
3. 필요 시 `bScaleLookInputByDeltaTime`에 따라 `DeltaTime` 스케일을 적용한다.
4. 누적 Yaw/Pitch를 계산한다.
5. Aim Profile의 최소/최대 각도로 Clamp 한다.
6. Clamp 결과를 다시 누적 상태에 반영한다.
7. SoftLimitZone 기준으로 현재 리미트 근접 여부를 기록한다.

즉 현재 `VehicleCamera`는 **현재 프레임 입력을 즉시 쓰고 버리는 기능이 아니라, 차량 기준 누적 자유 조준 상태를 계속 유지하고 갱신하는 기능**이다.

현재 런타임 스냅샷에 기록되는 핵심 값:
- `AccumulatedAimYaw`
- `AccumulatedAimPitch`
- `ClampedAimYaw`
- `ClampedAimPitch`
- `bAimAtYawLimit`
- `bAimAtPitchLimit`

### 2. 현재 카메라 모드를 평가한다
현재 `VehicleCamera`는 단일 모드 enum을 외부에서 바로 받지 않고,
`FCFVehicleCameraModeFlags`를 입력으로 받아 현재 대표 모드를 평가한다.

현재 모드 플래그:
- `bCombat`
- `bReverse`
- `bAirborne`
- `bDestroyed`
- `bSpectate`

현재 `EvaluateCameraMode()`의 우선순위는 아래와 같다.

1. `Destroyed`
2. `Spectate`
3. `Reverse`
4. `Airborne`
5. `Combat`
6. 그 외 `Normal`

즉 현재 카메라 기능은 단순한 “전투 모드 bool”이 아니라,
**외부 시스템이 여러 상태 플래그를 주면 그 우선순위에 따라 현재 대표 카메라 모드를 결정하는 기능**을 가진다.

현재 런타임 스냅샷에 기록되는 값:
- `CurrentCameraMode`
- `PreviousCameraMode`
- `bCameraModeChangedThisFrame`

### 3. 현재 Aim Profile을 해석하고 제한각을 결정한다
현재 카메라 회전 가능 범위는 `FCFVehicleCameraAimProfile`이 결정한다.

현재 `BuildResolvedAimProfile()`의 우선순위는 아래와 같다.

1. `bUseAimProfileOverride == true`면 외부에서 주입한 `AimProfileOverride`
2. 그렇지 않으면 `VehicleCameraData->DefaultAimProfile`
3. 그것도 없으면 구조체 기본값

즉 현재 `VehicleCamera`는 **기본 Aim Profile로 동작하되, 외부 시스템이 무기/상황에 따라 임시 프로필을 덮어쓸 수 있는 구조**다.

현재 Aim Profile이 담당하는 핵심 값:
- `MinYawDeg`
- `MaxYawDeg`
- `MinPitchDeg`
- `MaxPitchDeg`
- `YawSoftLimitZoneDeg`
- `PitchSoftLimitZoneDeg`
- `FOVOffset`
- `ArmLengthOffset`
- `HeightOffset`
- `SideOffset`

즉 현재 `VehicleCamera`는 단순 조준 제한만 하는 게 아니라,
**프로필에 따라 카메라 거리/FOV/프레이밍까지 같이 바꾸는 기능**도 가진다.

### 4. 현재 속도와 카메라 모드에 따라 SpringArm과 FOV를 갱신한다
현재 `UpdateCameraTransform()`는 `VehicleCamera`의 중심 계산 함수다.

현재 이 함수는 아래 요소를 함께 사용한다.
- 현재 차량 속도(`GetVehicleSpeedKmh()`)
- 현재 카메라 모드
- `FCFVehicleCameraTuningConfig`
- 현재 해석된 `AimProfile`
- 현재 Clamp된 Aim Yaw/Pitch

현재 계산 흐름은 아래와 같다.

#### A. 기본 목표값 계산
기본값으로 시작하는 항목:
- `DesiredArmLength = BaseArmLength`
- `DesiredFOV = BaseFOV`
- `ResolvedHeightOffset = BaseHeightOffset + AimProfile.HeightOffset`
- `ResolvedSideOffset = BaseSideOffset + AimProfile.SideOffset`

#### B. 모드별 보정
현재 모드에 따라 아래 보정이 추가된다.

- `Combat`
  - `CombatArmLengthOffset`
  - `CombatFOVOffset`
- `Reverse`
  - `ReverseArmLengthOffset`
  - `ReverseFOVOffset`
- `Airborne`
  - `AirborneFOVOffset`

즉 현재 카메라는 **모드에 따라 거리/FOV를 바꾸는 상태 기반 카메라**다.

#### C. 속도 기반 보정
현재 설정이 켜져 있으면 속도에 따라 보정이 추가된다.

- `bUseSpeedBasedArmLength`
- `bUseSpeedBasedFOV`
- `SpeedForMaxBonusKmh`
- `MaxSpeedArmLengthBonus`
- `MaxSpeedFOVBonus`

즉 현재 카메라는 **차량이 빨라질수록 더 넓고 멀게 보는 속도 기반 카메라 보정**을 가진다.

#### D. 충돌 시 시야 보조
현재는 SpringArm 충돌로 카메라가 많이 당겨졌을 때 가시성을 보완하는 보조 규칙도 있다.

관련 설정:
- `bUseCollisionViewAssist`
- `CollisionViewAssistStartRatio`
- `MaxCollisionHeightAssist`
- `MaxCollisionFOVAssist`

현재 동작:
- 이전 프레임 해결 거리와 목표 거리 비율을 `CollisionCompressionRatio`로 계산
- 이 비율이 시작 임계값보다 작으면
  - 높이 보조 추가
  - FOV 보조 추가

즉 현재 카메라는 단순 SpringArm 충돌 테스트만 쓰는 것이 아니라,
**충돌로 카메라가 심하게 눌릴 때 높이/FOV 보조로 시야를 살리는 기능**도 가진다.

#### E. 실제 컴포넌트 반영
현재 최종 반영 방식:
- `CameraAimPivot`가 있으면 월드 위치/회전 반영
- `CameraBoom`이 있으면
  - 충돌 테스트 설정 반영
  - 월드 위치/회전 반영
  - `TargetArmLength` 반영
  - `SocketOffset` 반영
- `CameraBoom`이 없고 `FollowCamera`만 있으면
  - 수동 위치/회전 계산 후 `FollowCamera`에 직접 적용
- `FollowCamera`가 있으면 `FieldOfView` 반영

즉 현재 `VehicleCamera`는 **SpringArm 우선 구조**지만,
Boom이 없을 때는 **직접 카메라 위치를 계산하는 폴백 경로**도 가진다.

### 5. 현재 Aim Trace를 계산하고 조준 가능 여부를 기록한다
현재 `UpdateAimTrace()`는 카메라 기준 조준점 계산을 담당한다.

현재 흐름:
1. Trace 시작점 결정
   - `FollowCamera`가 있으면 카메라 위치
   - 없으면 피벗 위치
2. `GetCurrentAimDirection()`으로 방향 벡터 계산
3. `AimTraceLength`만큼 `ECC_Visibility` 라인트레이스 수행
4. 결과를 런타임 스냅샷에 기록

현재 기록되는 값:
- `bAimBlocked`
- `AimHitLocation`
- `AimTraceDistance`
- `bWeaponCanFireAtCurrentAim`

현재 규칙상:
- 막히면 `bAimBlocked = true`
- 막히지 않으면 최대 거리 끝점을 적중 위치로 사용
- 현재 1차 구현에서는 `bWeaponCanFireAtCurrentAim = !bAimBlocked`

즉 현재 `VehicleCamera`는 단순 시점 계산 기능이 아니라,
**현재 조준선이 어디를 보고 있고 지금 바로 막혀 있는지까지 계산하는 조준 정보 기능**도 가진다.

## 현재 기준 기능의 성격 정리
현재 구현을 종합하면 `VehicleCamera`는 아래 역할을 가진다.

1. **자유 조준 기능**
   - Look 입력을 누적해서 차량 기준 Aim Yaw/Pitch를 유지함

2. **카메라 모드 평가 기능**
   - 전투/후진/공중/파괴/관전자 상태를 대표 모드로 해석함

3. **Aim Profile 해석 기능**
   - 현재 조준 제한각과 프레이밍/FOV 보정값을 결정함

4. **카메라 위치/거리/FOV 계산 기능**
   - SpringArm과 FollowCamera에 현재 목표값을 반영함

5. **충돌 대응 시야 보정 기능**
   - 카메라 압축 시 높이/FOV 보조를 적용함

6. **Aim Trace 계산 기능**
   - 현재 조준점과 가림 상태를 계산해 런타임 스냅샷에 기록함

따라서 현재 이 기능은 단순한 `CameraBoom 설정`이 아니라,
**차량 기준 자유 조준과 카메라 상태 계산을 담당하는 현재 차량 카메라 운영 기능**이라고 보는 것이 맞다.

## 현재 동작 방식
현재 `VehicleCamera`는 아래 방식으로 동작한다.

### 1. 런타임 초기화 방식
현재 `BeginPlay()`는 `bAutoInitializeOnBeginPlay == true`면 `InitializeCameraRuntime()`를 호출한다.

그리고 `TickComponent()`는 아직 준비가 안 된 상태라면,
매 프레임 `InitializeCameraRuntime()`를 다시 시도한다.

즉 현재 카메라 기능은 **BeginPlay 1회 시도만 하는 게 아니라, 준비 실패 시 Tick에서 재시도하는 복구형 초기화 구조**다.

현재 `InitializeCameraRuntime()`가 하는 일:
- Owner가 `ACFVehiclePawn`인지 캐시
- 카메라 참조 검색
- `VehicleCameraData`가 있으면 그 튜닝값 사용, 없으면 구조체 기본값 사용
- `CurrentArmLength`, `CurrentFOV` 초기화
- `CameraRuntimeState` 초기값 기록
- `bCameraRuntimeReady = true`

### 2. 참조 검색 방식
현재 `ResolveCameraReferences()`는 비어 있는 참조를 이름으로 자동 검색한다.

현재 기본 이름:
- `DefaultPivotRootName = CameraPivotRoot`
- `DefaultAimPivotName = CameraAimPivot`
- `DefaultCameraBoomName = CameraBoom`
- `DefaultFollowCameraName = FollowCamera`

현재 `BP_CFVehiclePawn` 기준으로도 이 이름에 맞는 컴포넌트가 존재한다.

즉 현재 카메라 기능은 **BP에 카메라 관련 컴포넌트가 준비돼 있으면, 이름 규칙만으로 자동 연결하는 구조**다.

### 3. 데이터 사용 방식
현재 구조상 `VehicleCameraData`는 선택적이다.

현재 `BP_CFVehiclePawn.VehicleCameraComp` 기본값 기준:
- `VehicleCameraData = null`

즉 현재 기본 프로젝트 구성에서는 **카메라 DataAsset이 연결되어 있지 않다.**
그 대신 `Tick`/`InitializeCameraRuntime`에서 `VehicleCameraData`가 없으면
- `FCFVehicleCameraTuningConfig()` 기본 구조체 값
- `FCFVehicleCameraAimProfile()` 기본 구조체 값

으로 폴백한다.

즉 현재 카메라 기능은 **DataAsset 없이도 코드 기본값으로 동작 가능한 폴백 구조**다.

## 현재 표시 조건 / 실행 조건
현재 `VehicleCamera`가 제대로 동작하려면 아래 조건이 중요하다.

- `bCameraRuntimeReady == true`여야 함
- `ResolveCameraReferences()`가 최소한 `CameraBoom` 또는 `FollowCamera`를 찾아야 함
- `Owner`가 존재해야 함
- AimTrace는 `World`와 `Owner`가 있어야 정상 동작함

현재 `ResolveCameraReferences()` 성공 조건은 아래처럼 완화되어 있다.

- `CameraBoom != nullptr || FollowCamera != nullptr`

즉 현재 구조에서는 Pivot/AimPivot가 없어도 일부 계산은 진행할 수 있고,
최소한 **Boom 또는 Camera만 있으면 카메라 기능을 살아 있는 것으로 본다.**

## 현재 자산 / 클래스 역할
### `UCFVehicleCameraComp`
- 종류: `C++ Component`
- 현재 역할: 자유 조준, 모드 평가, SpringArm/FOV 갱신, AimTrace 계산의 중심 컴포넌트

### `UCFVehicleCameraData`
- 종류: `PrimaryDataAsset`
- 현재 역할: 카메라 튜닝값과 기본 Aim Profile 공급
- 현재 상태: 기본 `BP_CFVehiclePawn`에는 연결되지 않음

### `FCFVehicleCameraTuningConfig`
- 종류: `USTRUCT`
- 현재 역할: Pivot/FOV/ArmLength/속도 보정/충돌 보정/AimTrace 길이 설정 공급

### `FCFVehicleCameraAimProfile`
- 종류: `USTRUCT`
- 현재 역할: 조준 제한각과 카메라 보정값 공급

### `FCFVehicleCameraModeFlags`
- 종류: `USTRUCT`
- 현재 역할: 외부 시스템이 현재 카메라 상태 Modifier를 전달하는 입력 구조

### `FCFVehicleCameraRuntimeState`
- 종류: `USTRUCT`
- 현재 역할: 현재 카메라 상태를 HUD/디버그용으로 묶어 보관하는 런타임 스냅샷

### `BP_CFVehiclePawn`
- 종류: `Blueprint`
- 현재 역할: 카메라 관련 Scene/SpringArm/Camera 컴포넌트를 실제로 보유한 소유자

## 현재 생성 및 연결 구조
현재 카메라 연결 구조는 아래와 같다.

1. Pawn 생성 시 `VehicleCameraComp` 존재
2. `BP_CFVehiclePawn`가 `CameraPivotRoot`, `CameraAimPivot`, `CameraBoom`, `FollowCamera` 컴포넌트를 보유
3. BeginPlay 또는 Tick에서 `InitializeCameraRuntime()` 호출
4. `ResolveCameraReferences()`로 이름 기반 참조 연결
5. Tick마다
   - `BuildResolvedAimProfile()`
   - `EvaluateCameraMode()`
   - `UpdateAimState()`
   - `UpdateCameraTransform()`
   - `UpdateAimTrace()`
   순서로 갱신

현재 구조 해석:
- 카메라 기능은 Pawn 종속이지만, 실제 계산 책임은 `VehicleCameraComp`로 분리돼 있다.
- 입력은 `Input` 기능이 받고, 최종 카메라 계산은 `VehicleCameraComp`가 맡는다.

## 현재 기본 프로젝트 설정 (`BP_CFVehiclePawn.VehicleCameraComp`)
현재 기본값 기준으로 확인된 주요 설정은 아래와 같다.

- `VehicleCameraData = null`
- `CameraPivotRoot -> BP_CFVehiclePawn:CameraPivotRoot`
- `CameraAimPivot -> BP_CFVehiclePawn:CameraAimPivot`
- `CameraBoom -> BP_CFVehiclePawn:CameraBoom`
- `FollowCamera -> BP_CFVehiclePawn:FollowCamera`
- `bAutoInitializeOnBeginPlay = true`
- `DefaultPivotRootName = CameraPivotRoot`
- `DefaultAimPivotName = CameraAimPivot`
- `DefaultCameraBoomName = CameraBoom`
- `DefaultFollowCameraName = FollowCamera`
- `CameraModeFlags` 기본값은 전부 `false`
- `bDrawAimTraceDebug = false`

즉 현재 기본 차량은 **BP가 카메라 참조 컴포넌트를 직접 보유하고, VehicleCameraComp가 이를 자동 연결해서 사용하는 구조**다.

## 현재 기능 책임
현재 구현 기준에서 `VehicleCamera`의 책임은 아래와 같다.

- 카메라 런타임 초기화를 수행한다.
- 카메라 관련 컴포넌트 참조를 자동 검색하고 연결한다.
- Look 입력을 누적해 차량 기준 자유 조준 상태를 갱신한다.
- 현재 카메라 모드를 평가한다.
- 현재 Aim Profile을 해석한다.
- SpringArm 길이, FOV, 높이/좌우 오프셋을 계산해 반영한다.
- 충돌 압축 시 시야 보조를 계산한다.
- 현재 AimTrace 적중 위치와 가림 상태를 계산한다.
- 현재 카메라 런타임 스냅샷을 유지한다.

## 현재 기준 비책임 항목
현재 구현상 `VehicleCamera`의 직접 책임으로 보지 않는 항목은 아래와 같다.

- Look 입력 해석 자체
  - 입력 수집/해제는 `Input` 기능 책임
- 차량 주행 상태 계산 자체
  - 속도/주행 상태 계산은 `VehicleDrive` 책임
- 차량 런타임 전체 준비 판정 자체
  - 전체 Ready 조합은 `VehicleRuntime` 책임
- 무기 시스템의 실제 조준 정책 결정 자체
  - 현재 AimProfile override를 주입하는 외부 무기 시스템 책임
- 디버그 UI 표시 자체
  - 카메라 런타임 상태를 화면에 보여주는 것은 별도 UI 책임

즉 현재 `VehicleCamera`는 입력기나 UI라기보다,
**차량 기준 카메라 상태를 계산하고 반영하는 카메라 운영 기능**에 가깝다.

## 현재 문서 기준의 핵심 결론
현재 `VehicleCamera` 기능은,

**Look 입력을 차량 기준 누적 조준 상태로 변환하고, 현재 카메라 모드와 Aim Profile, 속도, 충돌 상태를 반영해 SpringArm/FOV/AimTrace를 계산하고 이를 실제 카메라 컴포넌트에 적용하는 현재 차량 카메라 운영 기능**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `VehicleCamera`는 현재 차량이 어떤 시점과 어떤 조준 상태로 세상을 보게 되는지를 매 프레임 계산해서 실제 카메라에 반영하는 현재 상태 기능이다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- 현재 프로젝트에 별도 `VehicleCameraData` 자산이 존재하는지 여부와 실제 연결 계획
- 외부 시스템이 `CameraModeFlags`를 실제로 어디서 갱신하는지 전체 경로
- 외부 무기 시스템이 `AimProfileOverride`를 실제 사용 중인지 여부
- HUD/디버그에서 `CameraRuntimeState`를 직접 소비하는 경로 존재 여부

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `UpdateAimState()`의 Clamp/입력 누적 규칙 변경
- `EvaluateCameraMode()` 우선순위 변경
- `BuildResolvedAimProfile()` 우선순위 변경
- `UpdateCameraTransform()`의 Arm/FOV/충돌 보정 규칙 변경
- `UpdateAimTrace()`의 Trace 규칙 변경
- `BP_CFVehiclePawn.VehicleCameraComp` 기본 참조/설정 변경
- `VehicleCameraData` 실제 연결 상태 변경

## 문서 버전 관리
- 현재 문서 버전: `1.0.0`
- 문서 상태: `Initial`
- 관리 원칙:
  - 이 문서는 한 번 작성하고 끝내는 문서가 아니라, 기능의 현재 상태가 바뀌면 함께 갱신한다.
  - 기능 설명 본문이 바뀌면 체인지로그도 같이 갱신한다.
  - 구현 변경 없이 표현만 다듬은 경우와, 기능 이해에 영향을 주는 내용 변경을 구분해서 기록한다.

### 버전 증가 기준
- `Major`
  - 기능 해석 자체가 바뀌는 수준의 대규모 재작성
  - 문서 범위가 다른 기능 묶음까지 확장되거나 재정의될 때
- `Minor`
  - 현재 기능 설명에 중요한 항목이 추가될 때
  - 새로운 표시 항목, 동작 조건, 연결 구조가 확인되어 본문 의미가 확장될 때
- `Patch`
  - 오탈자 수정
  - 표현 명확화
  - 근거 보강
  - 본문 의미는 유지한 채 설명 정밀도만 올라갈 때

## 체인지로그
### v1.0.0 - 2026-04-23
- `VehicleCamera` 문서 최초 작성
- `UCFVehicleCameraComp`, `UCFVehicleCameraData`, `CFVehicleCameraTypes` 기준으로 현재 기능 정리
- 자유 조준, 모드 평가, Aim Profile 해석, SpringArm/FOV 갱신, AimTrace 계산 중심으로 본문 작성
- `BP_CFVehiclePawn.VehicleCameraComp` 기본 설정과 현재 `VehicleCameraData = null` 상태 반영

## 마지막 확인 기준
- 확인 일시: 2026-04-23
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h`
  - `UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehicleCameraData.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleCameraTypes.h`
  - `/Game/CarFight/Vehicles/BP_CFVehiclePawn`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
