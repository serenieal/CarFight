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
  - `FCFVehicleDrivingFXConfig`
  - `FCFVehicleCameraAimProfile`
  - `FCFVehicleCameraModeFlags`
  - `FCFVehicleCameraGameplayView`
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
  - `UpdateDrivingMotionState()`
  - `UpdateCameraTransform()`
  - `GetGameplayView()`
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
- `bAimPresentation`
- `bReverse`
- `bAirborne`
- `bDestroyed`
- `bSpectate`

`bAimPresentation`은 Driving FX 감쇠용 명시 플래그이며 Aim Profile 존재 여부에서 자동 추론하지 않는다. 2026-09-28 fresh C++ 검색에서는 `SetCameraModeFlags()`의 Product caller와 `bCombat / bAimPresentation / bAirborne` 실제 producer가 확인되지 않았다. 현재 GoPyMCP를 작업 prerequisite로 사용하지 않는 운영 기준에서는 이 자동 producer 증거 부재를 Camera FX core blocker로 취급하지 않는다. 해당 gameplay 상태가 실제 사용 가능하면 USER 직접 주행/전투 검수에서 최종 화면 결과를 확인하고, 상태 자체가 아직 사용되지 않으면 별도 wiring debt로 남긴다.

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
현재 `VehicleCamera`는 **Presentation tuning owner와 Aim Gameplay owner를 분리**한다.

현재 Presentation tuning 해석 순서:
1. Owner `VehicleData->CameraPresentationDataOverride`가 있으면 해당 자산의 `CameraTuningConfig`만 사용한다.
2. 차량별 override가 없으면 Component의 기존 `VehicleCameraData->CameraTuningConfig`를 사용한다.
3. 둘 다 없으면 `FCFVehicleCameraTuningConfig()` C++ 기본값을 사용한다.

현재 Aim Profile 해석 순서는 기존 계약을 유지한다.
1. `bUseAimProfileOverride == true`면 explicit `AimProfileOverride`
2. 아니면 Component의 기존 `VehicleCameraData->DefaultAimProfile`
3. 없으면 `FCFVehicleCameraAimProfile()` 기본값

즉 `CameraPresentationDataOverride`는 **Presentation 전용**이며 차량별 override가 Aim 제한각/조준 Gameplay owner를 암묵적으로 바꾸지 않는다.

2026-09-29 fresh persisted 기준 현재 대표 카메라 DataAsset은 `/Game/CarFight/Vehicles/Data/Definitions/DA_Cam_Default`다. 해당 자산은 legacy endpoint `MaxSpeedFOVBonus=6`, `MaxSpeedArmLengthBonus=45`를 serialized compatibility 값으로 유지하면서 `DrivingFXConfig.bUseNormalizedDrivingFX=true` 상태로 전환됐다. activation은 full CameraTuningConfig + DefaultAimProfile pre-state scope를 review하고 exact-one-flag만 false→true로 적용한 뒤 별도 disk reload Verify까지 PASS했다.

현재 in-scope `CFVehicleData` exact3의 persisted reference는 `DA_TestSedan=97.8303986 km/h`, `DA_TestSUV=92.7599792 km/h`, `DA_VehicleDefense_TestSUV=89.9538803 km/h`이며 세 차량 모두 `CameraPresentationDataOverride=None`이다. 이 값은 fixed-60Hz Vehicle Builder benchmark의 stable PeakSpeedKmh를 동일 Target path + DefinitionHash + RunId evidence로 결합해 authoring한 값이다.

VCFX-P0-03 USER Driving / Combat Feel Review는 USER 지시에 따라 현재 정상 검수 차량을 `Wagon` 단일 차량으로 사용한다. Wagon은 current TargetHash `0e38269d76e9907d1569facbe0ee10f4`에 다시 결합한 fresh fixed-60Hz benchmark에서 `PeakSpeedKmh=89.175705`, `top_speed_stable=true`, RunId `513827e0-01c9-4529-a5a9-a6c15dfa0606`을 확보했다. persisted `DA_Vehicle_Wagon`은 `ReferenceMaxSpeedKmh=89.175705`와 `CameraPresentationDataOverride=/Game/CarFight/Vehicles/Data/Definitions/DA_Cam_Default`를 명시적으로 가진다. 따라서 Wagon USER review는 Component fallback 존재 여부에 의존하지 않고 shared `DA_Cam_Default`의 normalized Driving FX Presentation tuning을 직접 소비한다.

fresh Blueprint AssetDump에서 `BP_CFVehiclePawn.VehicleCameraComp.VehicleCameraData`가 `/Game/CarFight/Vehicles/Data/Definitions/DA_Cam_Default`를 hard reference함을 확인했다. 따라서 기본 차량 카메라는 shared `DA_Cam_Default`의 normalized Driving FX 설정을 실제 Presentation tuning으로 소비한다.

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
- 현재 상태:
  - C++ 타입 존재
  - `/Game/CarFight/Vehicles/Data/Definitions/DA_Cam_Default` 자산 존재
  - fresh Blueprint AssetDump 기준 `BP_CFVehiclePawn.VehicleCameraComp.VehicleCameraData`가 해당 자산을 hard reference
  - shared `DA_Cam_Default.DrivingFXConfig.bUseNormalizedDrivingFX=true` 활성화 완료

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
   - `UpdateDrivingMotionState()`
   - `UpdateCameraTransform()`
   - `UpdateAimTrace()`
   순서로 갱신

현재 구조 해석:
- 카메라 기능은 Pawn 종속이지만, 실제 계산 책임은 `VehicleCameraComp`로 분리돼 있다.
- 입력은 `Input` 기능이 받고, 최종 카메라 계산은 `VehicleCameraComp`가 맡는다.
- `UpdateDrivingMotionState()`는 신규 Driving FX용 XY 평면 속도/가속/제동/횡가속 normalized signal을 계산한다.
- `GetGameplayView()`는 신규 Driving Roll/Speed FOV를 제외한 안정된 Origin/Direction/Up/FOV를 TargetSelect/Aim 소비용으로 제공한다.

### 6. VCFX normalized Driving FX 현재 구현

`FCFVehicleCameraTuningConfig` 안의 `FCFVehicleDrivingFXConfig`가 신규 주행 연출 계약을 소유한다.

현재 지원되는 최소 효과:
- `SpeedRatio` → Speed FOV Offset
- `SpeedRatio` → Speed Arm Offset
- normalized longitudinal acceleration → Acceleration Rear Kick
- normalized longitudinal deceleration → Braking Forward Kick
- signed normalized lateral acceleration → 기본 Lateral Roll 요구량 → 실제 차체 Motion intensity × 자유시점 signed ViewAlignment × 상태 감쇠 → 최종 Camera Roll

입력 기준:
- 차량 속도는 3D magnitude가 아니라 XY Planar velocity를 사용한다.
- `ReferenceMaxSpeedKmh`를 cm/s로 변환해 Speed/Accel/Brake/Lateral 기본 normalized 입력의 공통 분모로 사용한다.
- `ReferenceMaxSpeedKmh <= 0`, non-finite velocity/axis/reference에서는 신규 Driving FX 전체가 exact0 fail-safe다.
- 첫 valid frame, 비정상 DeltaTime, hitch, teleport에서는 SpeedRatio는 유효 reference가 있으면 유지할 수 있지만 Accel/Brake/Lateral history 기반 Motion FX는 0으로 reset한다.

P0-03 USER feel correction 이후 Lateral Roll은 횡가속만으로 화면을 직접 기울이지 않는다. `LateralRate`는 좌/우 signed 방향과 기본 Roll 요구량만 결정하고, 실제 Vehicle Mesh의 차체 움직임과 현재 자유조준 시점 방향이 최종 강도/화면 방향을 결정한다.

```text
BodyRollDeg      = NormalizeAxis(VehicleMesh.WorldRotation.Roll)
BodyAngularLocal = InverseTransformVectorNoScale(VehicleMesh.GetPhysicsAngularVelocityInDegrees())
BodyRollRate     = BodyAngularLocal.X
BodyYawRate      = BodyAngularLocal.Z

BodyMotionIntensity = Filter(Max(
    Abs(BodyRollDeg)  / 7.5,
    Abs(BodyRollRate) / 45.0,
    Abs(BodyYawRate)  / 90.0), 0..1)

ViewAlignment = Dot(VehicleForwardXY, CurrentFreeLookViewForwardXY)  // -1..1

FinalLateralRoll = Clamp(LateralRollCurve(LateralRate), ±MaxLateralRollDeg)
                 * BodyMotionIntensity
                 * ViewAlignment
                 * ResolvedMotionFXScale
```

현재 의미:
- 차체가 잔잔하면 `BodyMotionIntensity`가 낮아 저속 완만한 코너의 Camera Roll도 낮다.
- 실제 차체 Roll/Yaw motion이 격해지면 authored Roll headroom을 더 많이 사용한다.
- 정면 자유시점은 `ViewAlignment≈+1`, 측면 ±90°는 `≈0`, 후면은 `<0`이라 화면 기준 Roll을 억제/반전한다.
- 실제 차체 Roll을 Camera Rotation에 직접 복사하지 않는다. `bIsolateCameraFromVehiclePitchRoll=true`의 안정화 책임은 유지한다.
- 자유시점 각도만으로 `bAimPresentation`을 자동 활성화하지 않는다. `bAimPresentation`은 외부 producer가 명시적으로 공급할 별도 Aim 감쇠 상태다.
- Vehicle Mesh body-motion sample이 invalid이면 Lateral body-motion intensity는 exact0으로 fail-safe한다.

출력 안전 경계:
- Speed FOV/Arm과 Accel/Brake Kick은 기존 Curve 출력 뒤 절대 Clamp를 적용한다.
- Lateral Roll은 Curve 기본 요구량을 `±MaxLateralRollDeg`로 clamp한 뒤 BodyMotion/ViewAlignment/MotionScale을 곱하고 최종값을 다시 `±MaxLateralRollDeg`로 clamp한다.
- `bUseNormalizedDrivingFX=false`에서는 기존 `SpeedForMaxBonusKmh` + `MaxSpeedFOVBonus` + `MaxSpeedArmLengthBonus` legacy branch를 유지한다.
- normalized=true에서는 legacy global speed denominator를 fallback으로 사용하지 않는다.

Gameplay / Presentation 분리:
- 실제 SpringArm collision-resolved Camera Origin은 Gameplay View에도 유지한다.
- 신규 Driving Roll 적용 직전 실제 Camera rotation에서 Gameplay Direction/Up을 캡처한다.
- Gameplay FOV는 신규 Speed FOV를 제외한다.
- `CFTargetCandidateSearch::BuildRuntimeSearchView()`는 Gameplay View의 Origin/Direction/Up/FOV를 우선 소비한다.

2026-09-29 Technical evidence:
- P0-03 Lateral body-motion/free-look correction Official UE 5.8 Build PASS — job `58a7fd0fe6b04ea0834438c8cf61658a`. Admin 동일-request terminal polling budget 소진 후 read-only recovery process `c6324d1cd0844d7c8efe07890cc86840`가 official UBT log에서 `Running=0`, `CFVehicleCameraComp.cpp compile=1`, `CFVehicleCameraFXTests.cpp compile=1`, `Result:Succeeded`, `Failed=0`을 확인했다.
- focused Automation exact8 / 8 PASS — process `5588bff93fdc4694a7485eb9890c02f8`. 신규 `VCFX_P0_03.LateralPresentationMath`가 body motion intensity와 front/side/rear free-look alignment를 직접 검증한다.
- fixed-60Hz Vehicle Builder benchmark exact3 `top_speed_stable=true`: Sedan `97.830399`, TestSUV `92.759979`, DefenseSUV `89.953880 km/h`
- typed Reference migration process `bcbfdd00fa4e4c758e5105931b218831` PASS; TestSUV의 기존 `EQ_PFP_RocketReview` drift는 baseline으로 보존하고 Reference exact1만 Target에 적용
- shared CameraData activation process `5cda072c442242f6877157cd5e0664fd` PASS / terminal `VCFX_CAM_ACTIVATION=PASS`
- Camera activation historical AssetDump `adset_v1_1359b125c5ea74f8f5eb151e4d1dcb8d.3badfb69844be1ee7e008aec`
- P0-03 current CameraData AssetDump `adset_v1_4ea5ab37f8ccf66bb0004a8ff7b616d7.a447ee08dd738529a32da051`: normalized=true, `MaxLateralRollDeg=2.5`, body-motion seeds `6 / 7.5deg / 45deg/s / 90deg/s`, MotionRateInterpSpeed=10, 기존 Lateral/Accel/Brake curves unchanged
- Vehicle exact3 fresh AssetDump `adset_v1_9d16ff0177681371138c203d43019413.4d00b5e31d236084cc22ed14`
- fresh Blueprint AssetDump에서 `BP_CFVehiclePawn.VehicleCameraComp.VehicleCameraData -> DA_Cam_Default` hard reference 확인
- GoPyMCP managed PIE/RuntimeRead는 사용자 운영 결정에 따라 optional deferred technical evidence이며 현재 Camera FX 진행 blocker가 아니다.

따라서 현재 상태는 **VCFX-P0-02 Technical PASS를 기준선으로 유지한 VCFX-P0-03 USER Review 진행 중**이다. Lateral Roll은 actual body-motion + free-look signed alignment Contract Correction 후 Wagon USER 재검수에서 USER PASS했다. Speed FOV/Arm은 Reference 100% 초과 구간에서도 변화가 계속되는 것을 USER가 직접 확인해 Overspeed 항목 USER PASS했다. Acceleration Rear Kick은 Wagon에서 작동 자체를 확인했으며 체감은 둔하지만 현재 추가 튜닝을 보류한다. Braking Forward Kick도 Wagon 급정지에서 작동과 정지 후 baseline 복귀를 확인했지만 급정지 충격 체감은 약했다. Wagon의 실제 제동 성능과 현재 Arm 축소 중심 Presentation이 모두 영향을 줄 수 있으므로 Brake tuning도 현재 보류한다. Camera Pop/Collision은 일반 장애물 접근/이탈의 즉시 Arm 압축 + 부드러운 복귀를 확인했고, 이전에 재현됐던 hit/clear 미세 경계의 빠른 압축↔복귀 Chatter/Jitter도 Collision Recovery 교정 후 동일 조건 재검수에서 사라졌음을 USER가 확인해 USER PASS했다. 조준/Target Lock 상태에서 가속·고속주행·좌우 선회·급브레이크·장애물 근처 Camera Compression을 포함한 USER 검수에서도 Presentation Camera FX 때문에 Aim/Target Lock이 끌리거나 틀어지거나 비정상 해제되는 문제는 확인되지 않아 Aim/Targeting Interference USER PASS다. 전체 VCFX-P0-03 USER Acceptance는 아직 완료 선언하지 않았다. 멀미/피로감 USER 검수는 사용자의 결정으로 Deferred이며 PASS로 간주하지 않는다. Combat/Aim/Airborne 감쇠는 계산 로직과 `SetCameraModeFlags()` API는 존재하지만 fresh C++ 검색에서 Product caller가 없고, current AssetDump의 `/Game/CarFight` Blueprint exact6 전체에서도 관련 graph symbol이 없어 현재 gameplay producer/wiring이 없는 상태다. 따라서 이 감쇠 체감은 현재 USER 검수 불가이며 non-blocking wiring debt로 남긴다. 즉시 실행 가능한 P0-03 USER 검수는 더 없으며 현재 상태를 수용하면 다음 단계는 VCFX-P0-04 Current System Promotion + Closure다.

## 현재 기본 프로젝트 설정 (`BP_CFVehiclePawn.VehicleCameraComp`)
현재 기본값 기준으로 확인된 주요 설정은 아래와 같다.

- Component-level `VehicleCameraData`: fresh Blueprint AssetDump에서 `/Game/CarFight/Vehicles/Data/Definitions/DA_Cam_Default` hard reference 확인
- 현재 대표 카메라 DataAsset: `/Game/CarFight/Vehicles/Data/Definitions/DA_Cam_Default`
- `DA_Cam_Default.DrivingFXConfig.bUseNormalizedDrivingFX = true` — reviewed exact-one-flag activation + disk reload Verify PASS
- `DA_Cam_Default.DrivingFXConfig.MaxLateralRollDeg = 2.5`
- `LateralBodyMotionInterpSpeed = 6.0`
- `LateralBodyRollAngleForFullIntensityDeg = 7.5`
- `LateralBodyRollRateForFullIntensityDegPerSec = 45.0`
- `LateralBodyYawRateForFullIntensityDegPerSec = 90.0`
- VehicleData-level `CameraPresentationDataOverride`: in-scope exact3 모두 `None`
- VehicleData-level `ReferenceMaxSpeedKmh`: Sedan `97.8303986`, TestSUV `92.7599792`, DefenseSUV `89.9538803 km/h`
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

## VehicleDebug 연동 상태

현재 `VehicleCamera`는 `FCFVehicleCameraRuntimeState`를 통해 카메라 런타임 상태를 외부에 제공한다.
이 RuntimeState는 `VehicleDebug Panel`의 Camera 카테고리에서 표시된다.

현재 연결 흐름은 아래와 같다.

```text
UCFVehicleCameraComp
  -> FCFVehicleCameraRuntimeState
  -> FCFVehicleDebugCamera
  -> FCFVehicleDebugSnapshot.Camera
  -> ACFVehiclePawn::GetVehicleDebugCamera()
  -> UCFVehicleDebugPanelWidget::CachedCamera
  -> BuildCameraSectionViewData()
  -> Camera Navigation Item
  -> Selected Camera Section
```

이 구조에서 `VehicleCamera`는 카메라 상태를 계산하고 RuntimeState로 제공하는 책임을 가진다.
반대로 Camera Debug를 화면에 어떻게 배치하고 어떤 라벨로 보여줄지는 `VehicleDebug Panel` 책임이다.

현재 `VehicleDebug Panel`에서 Camera 카테고리로 확인할 수 있는 주요 항목은 아래와 같다.

- 상태 요약
- 현재 카메라 모드
- 이전 카메라 모드
- 조준 프로필
- 조준 막힘 여부
- 발사 가능 여부
- 누적 Aim Yaw / Pitch
- 제한 적용 Aim Yaw / Pitch
- Yaw / Pitch 제한 여부
- Desired / Current / Solved ArmLength
- Desired / Current FOV
- 충돌 압축 비율
- Aim Trace 거리
- Aim Trace 명중 위치

Camera Debug 표시 정책:

- 내부 `FieldId`, `SectionId`, enum 값, Snapshot 필드는 영문을 유지한다.
- 화면에 보이는 섹션 제목, 필드 라벨, 상태값은 한국어로 표시한다.
- `Blocked`, `Compressed`, `Limit` 같은 상태 문구는 Camera 탭 제목에 붙이지 않는다.
- 상태는 Camera Section 내부의 `상태 요약` 필드로 표시한다.
- 표시 언어 정책은 `Document/Systems/UI/DisplayTextPolicy.md`를 따른다.
- 최신 패널 구조는 `Document/Systems/UI/VehicleDebugPanel.md`를 기준으로 본다.

현재 Camera 상태 요약 우선순위는 아래와 같다.

```text
조준 막힘
카메라 압축
조준 제한
정상
```

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

- `CameraModeFlags`의 Combat/Aim/Airborne producer는 현재 Product 경로에 연결되어 있지 않다. fresh C++ 검색에서 `SetCameraModeFlags()` Product caller가 0건이고, current AssetDump `/Game/CarFight` Blueprint exact6 전체에서도 관련 graph symbol이 0건이다. 감쇠 계산/API는 구현돼 있으나 실제 gameplay state producer 연결은 후속 wiring debt다.
- 외부 무기 시스템이 `AimProfileOverride`를 실제 gameplay에서 어느 범위까지 사용하는지 여부.
- `CF-FQ-057 / VCFX-P0-04` Current System Promotion이 완료됐다. USER가 직접 수용한 범위는 Lateral Roll, Overspeed Speed FOV/Arm, Camera Pop/Collision, Aim/Targeting Interference다. Acceleration Rear Kick과 Braking Forward Kick은 기능 동작과 baseline 복귀를 확인했지만 추가 feel tuning은 각각 고성능/강제동 차량 이후로 Deferred이며 feel PASS로 확대하지 않는다. 멀미/피로감은 USER 결정으로 Deferred다. Combat/Aim/Airborne attenuation은 계산/API는 구현됐으나 Product producer가 없어 USER 체감 검수 불가인 non-blocking wiring debt로 보존한다.

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
- 현재 문서 버전: `1.3.0`
- 문서 상태: `Current`
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
### v1.3.0 - 2026-10-02
- `CF-FQ-057 Vehicle Camera Driving FX`의 `VCFX-P0-04 Current System Promotion + Closure`를 Current 계약에 반영했다.
- Normalized Driving FX, Gameplay/Presentation View 분리, body-motion/free-look Lateral Presentation, Overspeed FOV/Arm, Accel/Brake Kick, collision recovery와 Aim/Targeting 보호 경계를 현재 구현으로 승격했다.
- USER PASS는 Lateral Roll, Overspeed, Camera Pop/Collision, Aim/Targeting Interference 범위다. Rear/Brake는 기능 확인 후 tuning Deferred, Comfort는 Deferred로 보존하며 PASS로 확대하지 않는다.
- Combat/Aim/Airborne attenuation은 계산/API가 존재하지만 현재 Product producer가 없는 non-blocking wiring debt로 명시한다.
- 기존 official UE 5.8 Build 성공과 focused Automation exact8/8 PASS를 closure technical baseline으로 유지한다. 이번 promotion에서 Source/DataAsset mutation은 없다.

### v1.2.17 - 2026-10-02
- Combat/Aim/Airborne Camera Mode attenuation producer/wiring을 fresh audit했다.
- C++ 전체에서 `SetCameraModeFlags()` Product caller는 0건이며 선언/정의만 확인됐다.
- current AssetDump `/Game/CarFight` Blueprint exact6 전체 `bp_search_index`에서도 관련 함수/플래그 graph symbol은 0건이었다.
- 따라서 Mode attenuation은 계산 로직/API는 구현돼 있지만 현재 gameplay producer가 없는 non-blocking wiring debt이며 USER feel 검수 불가 상태다.
- Comfort는 Deferred 상태를 유지한다. 현재 P0-03에서 즉시 실행 가능한 USER 검수는 더 없으며 상태 수용 시 다음 단계는 VCFX-P0-04다.

### v1.2.16 - 2026-10-02
- USER가 멀미/피로감 검수를 현재는 넘어가기로 결정해 해당 항목을 Deferred로 기록했다. USER PASS로 확대하지 않는다.
- 기존 Aim/Targeting Interference USER PASS 및 이전 Camera FX 검증 결과는 유지한다.
- Source/DataAsset 변경은 없다. 다음 기술 확인 대상은 Combat/Aim/Airborne 감쇠 상태의 실제 producer/wiring이다.

### v1.2.15 - 2026-10-02
- Camera Pop/Collision USER PASS 이후 조준/Target Lock 상태에서 가속·고속주행·좌우 선회·급브레이크·장애물 근처 Camera Compression 조건을 USER가 추가 검수했다.
- USER가 `별 문제 없어`로 확인했으며 Presentation Camera FX 때문에 Aim/Target Lock이 끌리거나 틀어지거나 비정상 해제되는 문제는 재현되지 않았다.
- Aim/Targeting Interference 항목을 USER PASS로 확정한다. Source/DataAsset 변경은 없고 기존 Build + focused exact8 PASS evidence를 유지한다.
- 전체 VCFX-P0-03 USER Acceptance는 계속 진행 중이며 다음 exact USER 항목은 멀미/피로감이다.

### v1.2.14 - 2026-10-02
- Collision Chatter 교정 후 USER가 이전 문제와 동일한 hit/clear 미세 경계에서 재검수했고 `지터링 사라졌어`로 확인했다.
- 일반 접근/이탈의 즉시 Arm 압축 + 부드러운 복귀와 경계 안정성까지 확인되어 Camera Pop/Collision 항목을 USER PASS로 확정했다.
- v1.2.12~13의 root cause, Collision Recovery 구조, USER 재빌드 성공, focused Automation exact8/8 PASS evidence는 보존한다. 이번 USER 판정으로 Source/DataAsset 추가 변경이나 재빌드는 없다.
- 전체 VCFX-P0-03 USER Acceptance는 계속 진행 중이며 다음 exact USER 항목은 조준/타겟팅 방해 여부다.

### v1.2.13 - 2026-10-02
- Camera Collision Chatter 교정본에 대해 USER가 공식 UE 5.8 재빌드 성공을 보고했다. 최초 `LNK1104`는 실행 중 Editor의 DLL 점유에 의한 runtime-build interference로 확정했다.
- 기존 `Tools/RunVCFXP002Tests.ps1` 재실행 process `ebf0a8c24cd244719cdd2b27eb2759e2`에서 exact8/8 PASS, failure/missing/unexpected/duplicate terminal exact0을 확인했다.
- `FrozenDefaults`가 collision release hold/padding/recovery interpolation 새 기본 seed를 검증한다. 기존 Camera intent precedence, Sensor precedence, TargetSelect와 Lateral Presentation 회귀도 모두 PASS했다.
- 따라서 Camera Collision Chatter Correction은 Technical PASS다. Camera Pop USER PASS는 아직 보류하며 동일 hit/clear 경계에서 시각적 chatter 제거를 USER가 직접 확인해야 한다.

### v1.2.12 - 2026-10-02
- v1.2.11에서 일반 접근/이탈만으로 수용했던 Camera Pop USER PASS를 취소했다. USER가 hit/clear 미세 경계에서 시점을 움직일 때 카메라가 빠르게 압축↔복귀를 반복하는 Collision Chatter/Jitter를 재현했다.
- root cause는 SpringArm collision으로 줄어든 `SolvedArmLength`를 Presentation 상태인 `CurrentArmLength`에 역주입하고 다음 frame부터 다시 확장하는 feedback loop였다.
- 현재 구현은 `CurrentArmLength`를 Presentation 목표로 유지하고 별도 `CollisionRecoveryArmLength`를 사용한다. 전체 Presentation 목표 경로를 `ECC_Camera` Sphere Sweep으로 검사하고 blocking 동안 outward expansion을 금지한다.
- collision release는 `CollisionReleaseHoldTimeSec=0.10s`, `CollisionReleaseProbePaddingCm=4cm`, `CollisionRecoveryInterpSpeed=6`을 사용해 연속 Clear 확인 후 부드럽게 복귀한다. SpringArm built-in collision은 최종 안전망으로 유지한다.
- 최초 UE 5.8 Build job `4327e342b8b74c87acddaa0ed79ee6b4`는 수정 Source compile을 통과했지만 실행 중인 `UnrealEditor.exe`의 DLL 점유로 최종 link가 `LNK1104`에서 차단됐다. 이후 USER가 Editor 종료 후 동일 수정본 재빌드 성공을 보고했다. 새 binary 기준 focused Automation process `ebf0a8c24cd244719cdd2b27eb2759e2`는 exact8/8 PASS이며 실패/누락/예상외/중복 terminal이 모두 0이다. 따라서 Collision Chatter 교정은 Technical PASS이고, 다음 USER 검수는 동일 hit/clear 경계 재현 케이스다.

### v1.2.11 - 2026-10-02
- Wagon 주변 장애물 접근/이탈 USER 검수에서 Camera Pop/충돌 체감을 확인했다.
- 장애물 접촉 시 CameraBoom이 빠르게 압축되는 반응은 장애물 관통을 막기 위한 정상 collision response로 수용했다.
- 장애물에서 빠져나올 때 카메라가 부드럽게 원래 거리로 복귀하는 것을 확인해 Camera Pop 항목을 USER PASS로 기록했다.
- Source/DataAsset/Collision 설정은 변경하지 않았다. 다음 USER 검수는 조준/타겟팅 방해 여부다.

### v1.2.10 - 2026-10-02
- Wagon 급정지 USER 검수에서 Braking Forward Kick이 작동하고 정지 후 baseline으로 복귀하는 것을 확인했다.
- 다만 `급정지한다`는 체감은 약했다. 현재 Brake FX는 실제 longitudinal deceleration을 normalized 입력으로 사용하므로 Wagon의 약한 제동 성능이 입력 강도에 영향을 줄 수 있다.
- 현재 Brake Presentation은 `BrakingArmKickCurve`에 따른 Arm 축소 중심이며 최대 출력도 보수적으로 제한되고 최종 Arm 길이에는 보간이 적용되므로, 차량 물리 외에도 Presentation 자체가 충격감을 완화할 수 있다.
- 현재 Source/DataAsset는 수정하지 않고 Brake tuning을 보류한다. 향후 제동력이 강한 차량에서도 동일 체감이 재현될 때만 Curve/별도 scale/다른 Presentation 축을 재평가한다.
- 다음 USER 검수는 Camera Pop이다.

### v1.2.9 - 2026-10-02
- Wagon USER 재검수에서 Acceleration Rear Kick의 작동 자체는 확인했다. 다만 Wagon의 느린 가속 특성 때문에 뒤로 당겨지는 체감은 둔하게 느껴졌다.
- 현재는 `AccelerationRearKickScale=1.25`와 기존 AccelerationArmKickCurve shape를 유지하고 추가 튜닝하지 않는다.
- 향후 고성능 차량에서도 Rear Kick이 부족하거나 과도한 문제가 재현될 때만 tuning을 다시 연다.
- 전체 VCFX-P0-03 USER Acceptance는 아직 진행 중이며 다음 USER 항목은 Braking Forward Kick이다.

### v1.2.8 - 2026-10-02
- Wagon USER 재검수에서 `100 km/h` 이후에도 Speed FOV/Arm 변화가 계속되는 것을 직접 확인했다.
- Overspeed 고속 지속가속 Presentation 항목을 USER PASS로 기록한다. 전체 VCFX-P0-03 USER Acceptance는 아직 진행 중이다.
- Source/Product 추가 변경은 없으며 기존 0~100% Curve, ReferenceMaxSpeedKmh, 차량 물리, Brake/Lateral, Gameplay View/TargetSelect 계약을 그대로 유지한다.
- 다음 USER 검수는 Acceleration Rear Kick의 초반 급가속 방향·강도다.

### v1.2.7 - 2026-09-30
- P0-03 Wagon USER feedback에서 초반 Acceleration Rear Kick이 약하고 `ReferenceMaxSpeedKmh` 초과 후 Speed FOV/Arm이 1.0 ratio에 포화되어 고속 지속가속감이 줄어드는 문제를 교정했다.
- `ReferenceMaxSpeedKmh`의 performance-reference 의미는 유지하고 CameraData에 `MaxSpeedPresentationRatio=1.5`, `MaxOverspeedFOVBonusDeg=4`, `MaxOverspeedArmBonusCm=30`, `AccelerationRearKickScale=1.25`를 추가했다.
- Runtime SpeedRatio는 명시적 Max ratio까지 허용하지만 기존 Speed Curve에는 0..1 기본 ratio만 넣는다. Reference 100% 초과분은 별도 `OverspeedPhase`로 계산해 FOV/Arm headroom만 추가하므로 기존 0~100% USER 체감을 보존한다.
- 기존 AccelerationArmKickCurve shape는 유지하고 출력에 `AccelerationRearKickScale`을 곱한다. Braking/Lateral, 차량 물리, Gameplay View/TargetSelect 의미는 변경하지 않는다.
- corrected official UE 5.8 Build `c17a0f712ea741e3bcbb4e6568cf71f0` PASS. 선행 build `fa3d1d7d01e64cb4b4e8fb16b3237823`의 실패는 테스트 파일 tail/`#endif` 누락 `C1070`였고 복구 후 compile/link/UBT success로 닫았다.
- focused Automation `4c661b1277fa477aa9ac27d74ffdbb27` exact8/8 PASS.
- Product Python tune는 `VCFX_SPEED_ACCEL_TUNE_PASS`와 exact4 저장을 확인했다. wrapper stale marker는 v1.2.1로 교정했고 재적용은 하지 않았다.
- fresh AssetDump `adset_v1_6317bedee790f503c072f871aa0b1e7f.67430b6d68b9cefb0e867c06`에서 overspeed/rear-kick exact4와 기존 Curve 보존을 재확인했다.
- Overspeed 고속 지속가속은 Wagon USER 재검수에서 100 km/h 이후에도 변화가 계속되는 것을 확인해 USER PASS했다. Acceleration Rear Kick은 작동 자체를 확인했으며 Wagon의 느린 가속 특성 때문에 체감은 둔했지만 현재 추가 튜닝은 보류한다. USER Acceptance는 아직 진행 중이며 다음은 Braking Forward Kick 재검수다.

### v1.2.6 - 2026-09-29
- P0-03 Lateral Contract Correction을 Wagon으로 USER 재검수했고 `크게 이상한 것 못 느낌 / 일단 넘어가도 됨`으로 수용됐다.
- Lateral Roll 항목을 USER PASS로 기록한다. 전체 VCFX-P0-03 USER Acceptance는 아직 닫지 않는다.
- Source/Product 값은 v1.2.5 Technical PASS 상태 그대로이며 이번 기록에서 Build/Automation/Asset mutation은 0이다.
- 다음 USER 검수는 Acceleration Rear Kick이다.

### v1.2.5 - 2026-09-29
- P0-03 USER feedback에서 `MaxLateralRollDeg`만 줄이는 방식으로는 차체가 잔잔한데 카메라만 휙 꺾이는 이질감을 근본적으로 해결하기 어렵고, CarFight의 자유 조준 시점까지 고려해야 한다는 점을 Current System에 반영했다.
- Lateral Roll을 `signed LateralRate 기본 요구량 × 실제 Vehicle Mesh BodyMotionIntensity × signed free-look ViewAlignment × ResolvedMotionFXScale`로 교정했다. BodyMotionIntensity는 차체 Roll angle / local Roll angular rate / local Yaw angular rate의 normalized max를 사용한다.
- 자유시점은 차량 전방과 현재 WorldAim 수평 전방 dot으로 정면 `+1`, 측면 `0`, 후면 `-1`을 연속 계산한다. 자유시점 자체를 `bAimPresentation`으로 추론하지 않으며 explicit Aim/Combat attenuation 계약은 유지한다.
- 실제 차체 Roll을 Camera Rotation에 복사하지 않고 Motion intensity로만 사용해 `bIsolateCameraFromVehiclePitchRoll=true`와 Gameplay View pre-roll capture를 보존했다. 따라서 Aim/TargetSelect Gameplay direction/up/FOV 의미는 변경하지 않는다.
- current seed는 `LateralBodyMotionInterpSpeed=6`, Roll full-intensity `7.5deg`, RollRate `45deg/s`, YawRate `90deg/s`이다.
- Official UE 5.8 Build job `58a7fd0fe6b04ea0834438c8cf61658a` PASS. terminal polling budget 소진 후 read-only recovery `c6324d1cd0844d7c8efe07890cc86840`가 official UBT `Result:Succeeded`, 핵심 CameraComp/CameraFXTests compile, running process 0을 확인했다.
- focused Automation `5588bff93fdc4694a7485eb9890c02f8` exact8/8 PASS. 신규 `VCFX_P0_03.LateralPresentationMath`가 calm/violent body intensity, front/side/rear alignment, invalid exact0, scale multiplication을 검증한다.
- 임시 완화값 `MaxLateralRollDeg=1.25`는 새 modulation Technical PASS 후 `2.5deg` headroom으로 복구했다. Product tune process `fe8172dcb3a94644abbc442de3d9314e` PASS, fresh AssetDump `adset_v1_4ea5ab37f8ccf66bb0004a8ff7b616d7.a447ee08dd738529a32da051`에서 normalized=true, MaxRoll=2.5, body-motion seeds `6/7.5/45/90`, 기존 Lateral/Accel/Brake curve 유지 상태를 확인했다.
- USER Acceptance는 아직 Pending이다. 다음 직접 검수는 Wagon 전방 저속 완만한 회전 → 전방 더 격한 회전 → 약 90° 측면 자유시점 순서다.

### v1.2.4 - 2026-09-29
- VCFX-P0-03 USER 저속 회전 피드백에 따라 shared `DA_Cam_Default`의 `DrivingFXConfig.MaxLateralRollDeg`를 `3.0 -> 1.25 deg`로 교정했다.
- 이번 교정은 Lateral Roll clamp exact-one-field만 변경한다. `MotionRateInterpSpeed=10`, LateralRollCurve, AccelerationArmKickCurve, BrakingArmKickCurve, normalized flag는 그대로 유지한다.
- fresh persisted AssetDump `adset_v1_8e146e597214d9a99493f69391be0f73.2dadf797df157b5767bf6f08`에서 현재값을 재확인했다.

### v1.2.3 - 2026-09-29
- VCFX-P0-03 USER Driving / Combat Feel Review의 실제 검수 차량을 Wagon 단일 차량으로 고정한 현재 Product 상태를 반영했다.
- Wagon current TargetHash `0e38269d76e9907d1569facbe0ee10f4`에 fresh technical benchmark를 다시 결합해 `ReferenceMaxSpeedKmh=89.175705`, `top_speed_stable=true`, RunId `513827e0-01c9-4529-a5a9-a6c15dfa0606`을 확보했다.
- `DA_Recipe_Wagon` + `DA_Vehicle_Wagon`에 `ReferenceMaxSpeedKmh=89.175705`와 `CameraPresentationDataOverride=DA_Cam_Default` exact2 selective migration을 적용했다. unrelated Target field와 Recipe `AppliedState`는 보존했고 process `060797a8b98e4021a0c71d8f504086fc`가 Preview/Apply/fresh disk Verify를 모두 PASS했다.
- fresh persisted AssetDump `adset_v1_798a56aa52a87f86beba97426c54fef0.2a2f7eb1f411847d9dffae55`에서 Wagon exact2 값을 재확인했다.
- CameraData-only dependency dataset의 referencer exact0은 외부 Blueprint binding 부재를 단독 증명하지 못하므로 current binding 판단 근거로 사용하지 않는다. Wagon은 explicit VehicleData Presentation override를 가지므로 USER review에서는 fallback ambiguity가 없다.

### v1.2.2 - 2026-09-29
- CF-FQ-057 VCFX-P0-02를 Technical PASS 상태로 갱신했다. Source/Build/Automation뿐 아니라 per-Vehicle reference migration과 shared CameraData normalized activation까지 persisted Product 경로에 반영됐다.
- stable benchmark 기반 `ReferenceMaxSpeedKmh` exact3를 Sedan `97.8303986`, TestSUV `92.7599792`, DefenseSUV `89.9538803 km/h`로 기록했다.
- TestSUV에는 기존 PFP `EQ_PFP_RocketReview` Target drift가 있어 full DefinitionApply를 사용하지 않고 baseline drift 보존 + reviewed Reference exact-one-field selective migration을 사용했으며 fresh AssetDump에서 기존 RocketReview 보존을 확인했다.
- `BP_CFVehiclePawn.VehicleCameraComp.VehicleCameraData`가 `/Game/CarFight/Vehicles/Data/Definitions/DA_Cam_Default`를 hard reference함을 fresh persisted evidence로 확정했다.
- `DA_Cam_Default.DrivingFXConfig.bUseNormalizedDrivingFX=true`를 full config/Aim pre-state review, exact-one-flag Apply, package rollback 보호, 별도 disk Verify를 거쳐 활성화했다.
- latest Build `556bcd7e5b704253a78dcdd1b11d1ce9` PASS, activation 후 focused Automation `8ec5c2a8fc5c4d5482895aeed7d2eea0` exact6/6 PASS를 기록했다.
- next gate는 VCFX-P0-03 USER Driving / Combat Feel Review다. GoPyMCP PIE/RuntimeRead는 optional deferred evidence로 유지하며 USER 시각·주행감 판단을 차단하지 않는다.

### v1.2.1 - 2026-09-28
- GoPyMCP가 현재 작업 prerequisite가 아니라는 운영 결정을 반영해 managed PIE/RuntimeRead failure를 Camera FX blocking boundary에서 optional deferred evidence로 재분류했다.
- 사람이 직접 확인 가능한 Speed FOV/Arm, Accel/Brake Kick, Lateral Roll, Camera Pop, 조준 방해와 실제 사용 가능한 상태 감쇠는 USER direct review로 판정하도록 Current validation boundary를 교정했다.
- `ReferenceMaxSpeedKmh=0` exact3와 accepted Benchmark evidence 부재는 실제 normalized Product 활성을 막으므로 현재 Product migration blocker로 유지했다.

### v1.2.0 - 2026-09-28
- CF-FQ-057 VCFX-P0-02 Source minimum implementation을 Current System에 반영했다.
- `FCFVehicleDrivingFXConfig`, VehicleData `ReferenceMaxSpeedKmh` / `CameraPresentationDataOverride`, normalized XY motion/fail-safe, Speed FOV/Arm, Accel/Brake Kick, Lateral Roll을 추가했다.
- 신규 Driving Presentation 효과가 TargetSelect/Aim geometry를 바꾸지 않도록 `FCFVehicleCameraGameplayView`와 `GetGameplayView()` 소비 경계를 반영했다.
- `bUseNormalizedDrivingFX=false` legacy compatibility와 invalid reference exact0 fail-safe를 Current contract로 기록했다.
- fresh AssetDump로 `DA_Cam_Default`의 normalized=false, legacy FOV +6 / Arm +45, in-scope VehicleData exact3의 Reference=0을 확인해 Product migration HOLD 상태를 기록했다.
- `SetCameraModeFlags()` Product caller와 Combat/Aim/Airborne producer 미확정은 자동증명 한계로 기록하되 core blocker에서 제외했다. GoPyMCP PIE lifecycle interpreter identity mismatch 역시 optional deferred technical evidence로 내리고, 직접 확인 가능한 화면/주행 결과는 USER review로 이관했다.

### v1.1.0 - 2026-06-02
- `VehicleCameraData` 연결 상태 갱신
- `/Game/CarFight/Vehicles/Data/Camera/DA_Cam_Default` 자산 존재 상태 반영
- `BP_CFVehiclePawn.VehicleCameraComp.VehicleCameraData = null` 상태를 최신 확인 기준으로 유지
- 미확인 항목을 “자산 존재 여부”에서 “기본 차량 BP에 연결할지 여부”로 정리

### v1.0.0 - 2026-04-23
- `VehicleCamera` 문서 최초 작성
- `UCFVehicleCameraComp`, `UCFVehicleCameraData`, `CFVehicleCameraTypes` 기준으로 현재 기능 정리
- 자유 조준, 모드 평가, Aim Profile 해석, SpringArm/FOV 갱신, AimTrace 계산 중심으로 본문 작성
- `BP_CFVehiclePawn.VehicleCameraComp` 기본 설정과 현재 `VehicleCameraData = null` 상태 반영

## 마지막 확인 기준
- 확인 일시: 2026-10-02
- 확인 근거:
  - Official UE 5.8 Build job `58a7fd0fe6b04ea0834438c8cf61658a` + read-only UBT recovery `c6324d1cd0844d7c8efe07890cc86840` PASS
  - Focused Automation process `5588bff93fdc4694a7485eb9890c02f8` / exact8 PASS
  - Current CameraData AssetDump `adset_v1_4ea5ab37f8ccf66bb0004a8ff7b616d7.a447ee08dd738529a32da051`
  - Wagon persisted activation AssetDump `adset_v1_798a56aa52a87f86beba97426c54fef0.2a2f7eb1f411847d9dffae55`
  - CFVehicleData prior exact3 AssetDump `adset_v1_9d16ff0177681371138c203d43019413.4d00b5e31d236084cc22ed14`
  - `UE/Source/CarFight_Re/Private/CFTargetCandidateSearch.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehicleData.h`
  - `UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleAuthoringTypes.h`
  - `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleFieldRegistry.cpp`
  - `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleResolver.cpp`
  - 기존 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h`
  - `UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehicleCameraData.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleCameraTypes.h`
  - `/Game/CarFight/Vehicles/BP_CFVehiclePawn`
  - `/Game/CarFight/Vehicles/Data/Camera/DA_Cam_Default`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
---

## 변경 이력
### 2026-06-19 - 링크 경로 정정
- 이전 ProjectSSOT Plan/Systems 참조를 현재 Document/Plan 및 Document/Systems 경로로 정정했다.
- 구버전 ProjectSSOT 파일명 참조를 현재 `00_Vision` ~ `05_TestChecklist` 기준 또는 Archive 경로로 정정했다.
