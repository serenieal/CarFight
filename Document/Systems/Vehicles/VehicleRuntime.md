# VehicleRuntime

## 문서 목적
이 문서는 현재 프로젝트에서 `VehicleRuntime` 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 자산/클래스/설정 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

## 문서 범위
이 문서에서 말하는 `VehicleRuntime` 기능은 아래 요소를 묶어서 본다.

- lifecycle / observable state Authority: `ACFVehiclePawn`
- runtime orchestration coordinator: `UCFVehicleRuntimeComp`
- 주요 관련 컴포넌트: `UCFVehicleDriveComp`, `UCFWheelSyncComp`, `UCFVehicleFittingComp`, `UCFVehicleAimComp`, `UCFVehicleWeaponComp`, `UCFVehicleAmmoComp`, `UCFLauncherComp`, `UCFVehicleHealthComp`, `UCFVehicleDefenseComp`, `UCFTargetSelectComp`
- 관련 데이터: `UCFVehicleData`, `UCFVehicleFittingData`
- 핵심 Pawn facade / lifecycle 함수:
  - `PreRegisterAllComponents()`
  - `BeginPlay()`
  - `InitializeVehicleRuntime()`
  - `RefreshFittingDependentRuntime()`
  - `PrepareInitialSortieRuntimeMass()`
  - `VerifyInitialSortieRuntimeMass()`
  - `ApplyVehicleDataConfig()`
  - `PrepareWheelSync()`
  - `UpdateVehicleWheelVisuals()`
- 관련 상태:
  - `bAutoInitializeOnBeginPlay`
  - `bVehicleCoreRuntimeReady`
  - `bVehicleCombatRuntimeReady`
  - `bVehicleRuntimeReady` — 기존 호환 상태이며 현재 Core Ready와 동일 의미
  - `LastVehicleRuntimeSummary`
  - `bEnableWheelVisualTick`

즉, 현재 기준 `VehicleRuntime`은 **Pawn이 lifecycle과 상태 Authority를 유지한 채 `UCFVehicleRuntimeComp`에 초기화 순서 실행을 위임하고, VehicleData/Fitting/질량/장비/전투 연결까지 검증해 Core/Combat Ready를 판정하는 차량 런타임 준비 기능 묶음**으로 본다.

## 이 기능이 현재 실제로 하는 일
현재 구현 기준 `VehicleRuntime`의 핵심 역할은 **차량 Pawn의 lifecycle 순서를 보존하면서 VehicleData와 Fitting Snapshot을 실제 차량 런타임에 반영하고, Initial Mass·Drive·WheelSync·Health·Defense·Weapon·Ammo·Launcher·TargetSelect 연결을 검증한 뒤 Core Ready와 Combat Ready를 구분해 판정하는 것**이다.

`ACFVehiclePawn`은 `BeginPlay`/`PreRegisterAllComponents`, Public/BP/Automation facade와 모든 observable 상태를 계속 소유한다. `UCFVehicleRuntimeComp`는 별도 Tick/BeginPlay나 Blueprint 공개 API, 중복 상태를 만들지 않고 내부 실행 순서만 담당한다.

현재 구현 기준으로 보면, 이 기능은 아래 4단계 파이프라인으로 동작한다.

### 1. 런타임 초기화 시도를 시작한다
현재 `ACFVehiclePawn::BeginPlay()`는 아래 순서로 동작한다.

1. `Super::BeginPlay()`
2. `bAutoRegisterInputMappingContext`가 켜져 있으면 입력 매핑 등록 시도
3. `bAutoInitializeOnBeginPlay`가 켜져 있으면 `InitializeVehicleRuntime()` 호출

즉 현재 `VehicleRuntime`은 기본적으로 **BeginPlay 자동 초기화 기능**을 가진다.
별도 외부 호출 없이도 Pawn이 시작되면 스스로 런타임 준비를 시도한다.

그리고 `InitializeVehicleRuntime()`가 시작되면 Pawn facade가 내부 `VehicleRuntimeComp`로 실행을 위임하고, coordinator가 가장 먼저 아래 Pawn-owned 상태를 초기화한다.

- `bVehicleCoreRuntimeReady = false`
- `bVehicleCombatRuntimeReady = false`
- `bVehicleRuntimeReady = false`
- `LastVehicleRuntimeSummary = "VehicleRuntime: InitializeStarted"`

즉 현재 기능은 매 초기화 시도마다 이전 성공 상태를 그대로 신뢰하지 않고 **Core/Combat/compat Ready를 모두 Not Ready로 되돌린 뒤 현재 데이터와 Runtime 연결을 다시 검증하는 보수적 흐름**을 갖는다.

### 2. VehicleData 기반 설정을 실제 런타임 구성에 적용한다
현재 `InitializeVehicleRuntime()`는 가장 먼저 `ApplyVehicleDataConfig()`를 호출한다.

그리고 `ApplyVehicleDataConfig()`는 현재 아래 하위 적용 단계들을 순서대로 호출한다.

1. `ApplyVehicleMovementConfig()`
2. `ApplyVehicleReferenceConfig()`
3. `ApplyVehicleWheelPhysicsConfig()`
4. `ApplyVehicleWheelVisualConfig()`
5. `ApplyVehicleTurretVisualConfig()`
6. `VehicleDriveComp->ApplyDriveStateConfig(VehicleData->DriveStateConfig)`

즉 현재 `VehicleRuntime`은 단일 값 하나를 세팅하는 기능이 아니라,
**VehicleData를 실제 Chaos Vehicle Movement / Wheel Class / Wheel Visual / DriveState 설정으로 풀어서 적용하는 런타임 반영 기능**이다.

#### 2-1. Movement 설정 적용
`ApplyVehicleMovementConfig()`는 현재 `VehicleData->VehicleMovementConfig`를 읽어,
`UChaosWheeledVehicleMovementComponent`에 아래 설정을 적용한다.

- `ChassisHeight`
- `DragCoefficient`
- `DownforceCoefficient`
- `bEnableCenterOfMassOverride`
- `CenterOfMassOverride`
- `EngineSetup.MaxTorque`
- `EngineSetup.MaxRPM`
- `EngineSetup.EngineIdleRPM`
- `EngineSetup.EngineBrakeEffect`
- `EngineSetup.EngineRevUpMOI`
- `EngineSetup.EngineRevDownRate`
- `DifferentialSetup.DifferentialType`
- `DifferentialSetup.FrontRearSplit`
- `SteeringSetup.SteeringType`
- `SteeringSetup.AngleRatio`
- `bLegacyWheelFrictionPosition`

즉 현재 `VehicleRuntime`은 **차량이 어떤 주행 성격을 가지는지**를 실제 VehicleMovementComponent에 적용하는 역할을 한다.

현재 이 단계가 끝나면 `LastVehicleRuntimeSummary`에는 대략 아래 의미의 문자열이 기록된다.

- MovementProfile 이름
- MaxTorque
- MaxRPM
- DifferentialType
- SteeringType

즉 현재 기능은 단순 적용만 하는 것이 아니라,
**무슨 주행 프로파일을 적용했는지 런타임 요약 문자열로 함께 남긴다.**

#### 2-2. Reference 설정 적용
`ApplyVehicleReferenceConfig()`는 현재 `VehicleData->VehicleReferenceConfig` 기준으로,
앞/뒤 휠 클래스 참조 정보를 읽고 요약 문자열을 만든다.

현재 이 단계는 직접 Chaos 설정을 크게 만지기보다,
**현재 차량 참조 자산이 무엇인지 확정하고 요약 문자열로 남기는 기능**의 성격이 더 강하다.

현재 남기는 요약 예시는 아래 의미를 가진다.

- FrontWheelClass 이름
- RearWheelClass 이름

#### 2-3. Wheel Physics 설정 적용
`ApplyVehicleWheelPhysicsConfig()`는 현재 `VehicleData->VehicleMovementConfig`와 `VehicleReferenceConfig`를 함께 사용한다.

현재 이 단계가 하는 일:

- `ResolvedVehicleMovementComponent->WheelSetups`를 순회한다.
- 본 이름 기준으로 앞바퀴/뒷바퀴를 구분한다.
- 앞/뒤 각각에 맞는 WheelClass를 할당한다.
- 앞/뒤 AdditionalOffset을 적용한다.
- `bUseMovementOverrides`가 켜져 있으면 WheelClass CDO에 런타임 휠 튜닝을 반영했다가 복원하는 흐름을 수행한다.

즉 현재 `VehicleRuntime`은 **차량 휠 물리 구성을 WheelSetup 레벨까지 실제 런타임 상태로 맞춰주는 기능**을 가진다.

현재 이 단계가 끝나면 `LastVehicleRuntimeSummary`에는 대략 아래 의미가 기록된다.

- Runtime Wheel Physics Override 사용 여부
- FrontWheelClass
- RearWheelClass
- FrontWheelAdditionalOffset
- RearWheelAdditionalOffset

#### 2-4. Wheel Visual 설정 적용
`ApplyVehicleWheelVisualConfig()`는 현재 `WheelSyncComp`와 `VehicleData`가 모두 있어야 동작한다.

CF-FQ-040 WSA 완료 이후 이 단계는 단순 Mesh 지정뿐 아니라 **현재 VehicleData만으로 Wheel Visual state를 deterministic하게 재구성하는 경계**다.

현재 이 단계가 하는 일:

- `WheelSyncComp->ExpectedWheelCount` 적용
- `WheelSyncComp->FrontWheelCountForSteering` 적용
- `Wheel_Mesh_FL`에 FL 휠 메쉬 적용
- `Wheel_Mesh_FR`에 FR 휠 메쉬 적용
- `Wheel_Mesh_RL`에 RL 휠 메쉬 적용
- `Wheel_Mesh_RR`에 RR 휠 메쉬 적용
- 최초 Wheel Visual mutation 전에 `Wheel_Mesh_FL/FR/RL/RR`의 BP-authored base RelativeTransform(Location/Rotation/Scale)을 캡처
- 매 Apply 시작 시 authored base full transform을 복원해 이전 VehicleData의 scale/center/orientation 잔류를 제거
- `bUseWheelSocketScale=true`이면 USER-authored `Wheel_Anchor_*` Socket RelativeScale을 `Wheel_Mesh_*` 시각 Scale로 exact 적용
- FR/RR explicit Mesh가 없고 FL Mesh를 null fallback으로 재사용하는 Right slot에만 local Roll(X) 180° orientation compensation 적용
- 같은 fallback source에서 WheelSync에 FR/RR per-wheel spin handedness `-1` 전달, Left/explicit Right는 `+1`
- Legacy `WheelVisualConfig.bAutoScaleWheelMeshToRadius`가 켜져 있으면 각 휠 StaticMesh 바운드 반지름을 측정해 `FrontWheelRadius` / `RearWheelRadius` 기준 Uniform Scale 적용
- Legacy `WheelVisualConfig.bAutoCenterWheelMeshBoundsToOrigin`이 켜져 있으면 최종 orientation/scale 기준 StaticMesh 바운드 중심을 `Wheel_Mesh_*` 원점에 맞춰 시각 휠 중심을 물리 휠 중심과 정렬

즉 현재 `VehicleRuntime`은 **휠이 몇 개인지, 앞바퀴가 몇 개인지, 실제 시각 휠 메쉬가 무엇인지까지 런타임에 반영하는 기능**도 포함한다.
자동 스케일은 `WheelSyncComp->TryPrepareWheelSync()`가 기준 회전/위치를 캡처하기 전에 적용된다.

현재 이 단계가 끝나면 `LastVehicleRuntimeSummary`에는 대략 아래 의미가 기록된다.

- ExpectedWheelCount
- FrontWheelCountForSteering
- AutoScale On/Off와 휠별 Scale / Center / CenterFix / InvalidRadius / MeshMissing 요약

#### 2-5. Drive 상태 설정 적용
`ApplyVehicleDataConfig()` 마지막에는,
현재 `VehicleDriveComp`와 `VehicleData`가 모두 있을 때만 `VehicleDriveComp->ApplyDriveStateConfig(VehicleData->DriveStateConfig)`를 호출한다.

즉 현재 `VehicleRuntime`은 단순 Movement와 Wheel만 준비하는 기능이 아니라,
**Drive 상태 머신이 어떤 설정으로 동작할지도 런타임 초기화 단계에서 함께 적용하는 기능**이다.

### 3. Core / Combat Runtime 준비 조건을 검증한다
현재 `InitializeVehicleRuntime()`는 VehicleData 적용 이후 Drive/WheelSync만 확인하지 않는다. 먼저 Drive/WheelSync/Health와 같은 차량 기본 Runtime을 준비하고, 같은 Prepared Fitting Snapshot의 Initial Mass 검증 → Weapon/Defense Commit → Ammo/Launcher/Aim/TargetSelect 연결까지 순서대로 확인한다.

대표적인 준비 상태는 다음과 같다.

- `bDriveReady`
- `bWheelSyncReady`
- `bHealthReady`
- `bInitialMassReady`
- `bFittingApplied`
- `bAimReady`
- `bWeaponReady`
- `bAmmoReady`
- `bLauncherReady`
- `bTargetSelectReady`
- Defense Component 존재 및 실제 DefenseData 초기화 상태

#### 3-1. Drive 준비 검사
현재 구현 기준 `bDriveReady`는 아래 조건으로 판정된다.

- `VehicleDriveComp != nullptr`
- `VehicleDriveComp->CacheVehicleMovementComponent()` 성공

즉 현재 Drive 준비는 단순히 DriveComp 포인터 존재 여부가 아니라,
**DriveComp가 실제 VehicleMovementComponent를 캐시할 수 있어야 Ready**로 본다.

#### 3-2. WheelSync 준비 검사
현재 구현 기준 `bWheelSyncReady`는 `PrepareWheelSync()`의 반환값을 사용한다.

`PrepareWheelSync()`는 현재 아래처럼 동작한다.

- `WheelSyncComp`가 없으면
  - `LastVehicleRuntimeSummary = "VehicleRuntime: WheelSyncComp is null."`
  - `false` 반환
- 있으면
  - `WheelSyncComp->TryPrepareWheelSync()` 호출 결과 반환

즉 현재 WheelSync 준비는 **컴포넌트 존재 + WheelSync 자체 준비 성공 여부**를 함께 본다.

### 4. Core Ready / Combat Ready를 분리해 판정하고 요약을 남긴다
현재 `InitializeVehicleRuntime()`의 최종 판정은 두 단계다.

```text
CoreReady
= DriveReady
&& WheelSyncReady
&& HealthReady
&& DefenseComponentReady
&& FittingApplied

CombatReady
= CoreReady
&& AimReady
&& WeaponReady
&& AmmoReady
&& LauncherReady
&& TargetSelectReady

bVehicleRuntimeReady = CoreReady   // 기존 호환 의미
InitializeVehicleRuntime() 반환값 = CoreReady
```

장비 hot apply 이후 호출되는 `RefreshFittingDependentRuntime()`은 이미 Commit된 Fitting을 기준으로 Ammo/TurretVisual/Launcher와 Combat 연결만 다시 구성하며 **반환값은 CombatReady**다. 이 비대칭은 의도된 현재 계약이다.

최종 `LastVehicleRuntimeSummary`에는 Data/Fitting/Mass/Drive/WheelSync/Aim/Weapon/Ammo/Launcher/Health/Defense/TargetSelect/OwnerVisual과 CoreReady/CombatReady, Fitting·InitialMass·DataConfig·Layout·TurretVisual 요약이 함께 기록된다.

즉 현재 `VehicleRuntime`은 단순 성공/실패 bool 하나가 아니라 **차량 코어 준비와 실제 전투 준비를 분리해 진단하고, 기존 호환 Ready는 Core 의미로 유지하는 구조**다.

## 현재 기준 기능의 성격 정리
현재 구현을 종합하면 `VehicleRuntime`은 아래 역할을 가진다.

1. **차량 데이터 반영 기능**
   - VehicleData의 주행/참조/휠 물리/휠 시각/터렛 시각/DriveState 설정을 실제 런타임 상태에 적용함

2. **Initial Sortie Fitting / Mass 준비와 검증 기능**
   - PreRegister의 Super 이전에 Prepared Snapshot Target Mass를 Chaos Movement에 적용하고 BeginPlay 초기화에서 실제 VehicleMesh Physics Mass를 검증함
   - 질량 검증을 통과한 같은 Snapshot으로 Weapon/Defense Runtime을 Commit함

3. **차량 Core / Combat 준비 상태 검증 기능**
   - Drive/WheelSync/Health/Defense/Fitting을 Core Ready 조건으로 검증함
   - Aim/Weapon/Ammo/Launcher/TargetSelect를 Combat Ready 조건으로 추가 검증함

4. **장비 변경 후 부분 Runtime 재구성 기능**
   - `RefreshFittingDependentRuntime()`으로 Core를 유지한 채 장비 의존 Ammo/TurretVisual/Launcher/CombatReady만 재구성함

5. **런타임 진단 요약 기능**
   - `LastVehicleRuntimeSummary`에 초기화/질량/Fitting/Core/Combat 상태를 문자열로 남김

6. **런타임 이후 WheelVisual 업데이트 게이트 기능**
   - compat `bVehicleRuntimeReady`(Core Ready)가 아니면 WheelVisual Tick을 막고, Ready일 때만 휠 시각 갱신을 진행함

따라서 현재 이 기능은 단순한 `초기화 함수`라기보다,
**차량 데이터 적용, 주행 준비 검증, 휠 시각 준비, 최종 Ready 판정, 그리고 런타임 진단 요약까지 담당하는 차량 런타임 준비 기능**이라고 보는 것이 맞다.

## 현재 동작 방식
현재 `VehicleRuntime`은 아래 방식으로 동작한다.

### 1. 자동 시작 여부
현재 기본 동작은 `BeginPlay`에서 자동 시작이다.

관련 스위치:
- `bAutoInitializeOnBeginPlay`

현재 의미:
- `true`면 BeginPlay에서 자동으로 `InitializeVehicleRuntime()` 실행
- `false`면 자동 실행되지 않음

### 2. Ready 전 Tick 동작
현재 `Tick()`은 아래 조건으로 동작한다.

- `!bEnableWheelVisualTick || !bVehicleRuntimeReady`
  - `DisplayDriveStateOnScreenDebug()`만 호출하고 리턴
- 그 외
  - `UpdateVehicleWheelVisuals(DeltaSeconds)` 호출
  - 이후 `DisplayDriveStateOnScreenDebug()` 호출

즉 현재 구조에서 `VehicleRuntime`은 **Ready 상태가 되기 전까지 휠 시각 업데이트를 막는 게이트** 역할도 한다.

### 3. WheelVisual 갱신 방식
현재 `UpdateVehicleWheelVisuals()`는 아래처럼 동작한다.

- `WheelSyncComp`가 없으면 실패
- 있으면 `WheelSyncComp->UpdateWheelVisualsPhase2(DeltaSeconds)` 호출
- 성공 시 `AppendWheelSyncRuntimeSummary()` 호출
- 실패 시 `LastVehicleRuntimeSummary = "VehicleRuntime: Wheel visual update failed."`

즉 현재 런타임 기능은 초기화 시점만 보는 게 아니라,
**초기화 이후 프레임 단위 WheelVisual 동작 결과도 런타임 요약 문자열에 연결**한다.

## 현재 표시 조건 / 실행 조건
현재 `VehicleRuntime` 기능이 실제로 제대로 성립하려면 아래 조건이 중요하다.

- `bAutoInitializeOnBeginPlay`가 켜져 있거나 외부에서 `InitializeVehicleRuntime()`를 직접 호출해야 함
- `VehicleDriveComp`가 존재해야 함
- `VehicleDriveComp->CacheVehicleMovementComponent()`가 성공해야 함
- `WheelSyncComp`가 존재해야 함
- `WheelSyncComp->TryPrepareWheelSync()`가 성공해야 함
- VehicleData가 있어야 Movement / Wheel Physics / Wheel Visual / DriveStateConfig 적용이 정상적으로 의미를 가짐

즉 현재 구조에서 `VehicleRuntime`은 단순히 Pawn이 존재한다고 자동으로 완성되는 기능이 아니라,
**DriveComp, WheelSyncComp, VehicleMovementComponent, VehicleData가 모두 일정 수준 이상 정상 연결되어야 완성되는 기능**이다.

## 현재 자산 / 클래스 역할
### `ACFVehiclePawn`
- 종류: `C++ Pawn`
- 현재 역할: VehicleRuntime lifecycle / Public·BP·Automation facade / observable state Authority
- 현재 기능: PreRegister·BeginPlay 진입, RuntimeComp 호출, Core/Combat/compat Ready와 `LastVehicleRuntimeSummary` 소유, Tick의 WheelVisual gate 유지
- 현재 Debug 계약: `GetVehicleDebug*()` / `ShouldShowVehicleDebug*()` Public·Blueprint facade와 Debug text aggregation behavior를 계속 소유하며 reflected DTO 선언 자체는 전용 type header로 분리됨

### `CFVehicleInputTypes.h`
- 종류: `C++ Public reflected type header`
- 현재 역할: Vehicle Pawn 입력 enum/struct declaration의 단일 owner
- 현재 범위: `ECFVehicleInputDeviceMode`, `ECFVehicleMoveDirectionIntent`, `ECFVehicleMoveZone`, `ECFVehicleInputOwnership`, `FCFVehicleMoveInputConfig`, `FCFVehicleMoveInputResult`
- 현재 비책임: 입력 behavior/state Authority 이동 없음, 새 Input Component 없음

### `CFVehicleDebugTypes.h`
- 종류: `C++ Public reflected type header`
- 현재 역할: VehicleDebug enum/DTO declaration의 단일 owner
- 현재 범위: `ECFVehicleDebugDisplayMode`와 Overview/Drive/Input/Runtime/Camera/Aim/Target/Weapon/Snapshot Debug struct
- 현재 dependency 경계: Pawn을 include하지 않고 필요한 Input/Drive/Aim/Camera/Damage/Weapon/TargetSelect type owner를 직접 소비한다. DataAsset object pointer는 forward declaration을 사용한다.
- 현재 소비 경계: Debug HUD/Panel과 `CarFightVehicleUtils.h`는 DebugTypes를 직접 include하고, 실제 Pawn 메서드 호출이 필요한 Private translation unit만 `CFVehiclePawn.h`를 명시적으로 include한다.
- 현재 호환 계약: reflected 이름/필드/Blueprint 노출, Pawn Debug facade와 RuntimeApply `GetVehicleDebugRuntime()` readback 의미는 변경하지 않는다.

### `UCFVehicleRuntimeComp`
- 종류: `C++ ActorComponent`
- 현재 역할: 상태를 새로 소유하지 않는 내부 Runtime orchestration coordinator
- 현재 기능: Initialize/Refresh, Initial Mass prepare/verify, VehicleData 적용 순서를 실행하고 결과를 Pawn-owned 상태에 기록
- 현재 비노출 계약: 독립 Tick/BeginPlay 없음, Blueprint 공개 API 없음, Product Asset 수동 추가/저장 불필요

### `UCFVehicleData`
- 종류: `DataAsset`
- 현재 역할: 차량 런타임 적용의 원본 데이터
- 현재 기능: Movement / Reference / WheelVisual / DriveState 설정 공급

### `UCFVehicleDriveComp`
- 종류: `C++ Component`
- 현재 역할: VehicleMovementComponent 접근과 DriveState 설정 적용의 중심 컴포넌트
- 현재 기능: `CacheVehicleMovementComponent()`, `ApplyDriveStateConfig()`

### `UCFWheelSyncComp`
- 종류: `C++ Component`
- 현재 역할: 휠 시각 준비와 프레임 갱신의 중심 컴포넌트
- 현재 기능: `TryPrepareWheelSync()`, `UpdateWheelVisualsPhase2()`

### `UChaosWheeledVehicleMovementComponent`
- 종류: `Chaos Vehicle Movement Component`
- 현재 역할: 실제 주행 물리 설정의 최종 적용 대상

## 현재 생성 및 연결 구조
현재 런타임 준비 흐름은 아래와 같다.

1. Pawn 생성 시 `VehicleRuntimeComp`를 포함한 Runtime 기본 서브오브젝트들을 자동 생성
2. `PreRegisterAllComponents()`에서 Initial Sortie Fitting을 준비하고 필요 시 Snapshot Target Mass를 **Super 호출 전에** Chaos Movement에 적용
3. `BeginPlay()` 진입 후 `bAutoInitializeOnBeginPlay`가 켜져 있으면 Pawn facade `InitializeVehicleRuntime()` 호출
4. Pawn facade가 `VehicleRuntimeComp`에 실행을 위임
5. VehicleVisual/VehicleData 적용 → OwnerVisual/Layout/Turret → Drive/WheelSync/Aim/Health 준비
6. Prepared Fitting의 실제 VehicleMesh Mass 검증 뒤 **같은 Snapshot**으로 Weapon/Defense Commit
7. Ammo/Turret/Launcher/CombatFx/Sensor 연결 후 CoreReady와 CombatReady를 각각 판정
8. compat `bVehicleRuntimeReady`는 CoreReady와 동일하게 유지하며 Tick의 WheelVisual gate에 계속 사용
9. Runtime Equipment Apply처럼 이미 Commit된 Fitting이 바뀐 경로는 Pawn facade `RefreshFittingDependentRuntime()`로 장비 의존 Runtime과 CombatReady만 재구성

현재 구조 해석:
- 이 기능은 전역 시스템이 아니라 Pawn 종속 런타임 기능이다.
- Pawn은 lifecycle과 최종 state Authority를 유지하고, `VehicleRuntimeComp`는 behavior/orchestration만 담당한다.
- VehicleData 적용, Initial Mass/Fitting 검증, Domain Component 연결과 준비 판정이 같은 초기화 계약 안에 묶여 있다.
- Runtime Vehicle/Equipment Apply의 rollback/readback 진입점은 기존 Pawn facade를 계속 사용한다.

## Fitting / Mass Current 계약

현재 `VehicleRuntime`은 VehicleData의 정적 주행 설정뿐 아니라 **검증된 Fitting Snapshot을 실제 출격 차량 런타임으로 반영하는 경계**도 소유한다.

현재 실제 Source 기준 핵심 흐름은 다음과 같다.

```text
VehicleData + VehicleFittingData + ActiveMountProfile
→ UCFVehicleFittingComp::PrepareInitialSortieFitting()
→ VehicleFittingSnapshot.TotalVehicleMassKg
→ Pre-Physics UChaosWheeledVehicleMovementComponent::Mass 적용
→ 기존 Mass 기록 / 적용 실패 시 Legacy 입력 복구
→ BeginPlay에서 Configured Movement Mass + VehicleMesh Actual Mass + Physics State 검증
→ 같은 Prepared Snapshot으로 Weapon / Defense Runtime Commit
→ AppliedFittingSnapshot 보존
```

`ACFVehiclePawn::PrepareInitialSortieRuntimeMass()`는 Physics State 생성 전에 Snapshot의 목표 질량을 Chaos Vehicle Movement에 적용한다. 적용 준비 또는 기록에 실패하면 기존 Movement Mass를 복원하고 허용된 Legacy 입력으로 fail-safe 전환한다.

`ACFVehiclePawn::VerifyInitialSortieRuntimeMass()`는 BeginPlay 이후 Configured Movement Mass와 `VehicleMesh->GetMass()`가 보고하는 실제 Body Mass, Physics State 생성 여부, Simulation 여부와 PhysicsAsset 존재 여부를 함께 검증한다. 이 검증을 통과하지 않은 Snapshot은 Weapon·Defense Commit의 전제가 되지 않는다.

`InitializeVehicleRuntime()`의 Core Ready 판정에는 현재 Fitting Apply 성공이 포함된다. 따라서 Fitting/Mass는 별도 UI용 부가 데이터가 아니라 차량 코어 런타임 준비 계약의 일부다.

필드 장착 변경에서는 `FCFFieldFitCoordinator`가 후보 Fitting Snapshot, Runtime 장비 상태와 질량 변경을 같은 완료 트랜잭션 안에서 다루며, Runtime 또는 Inventory Commit 실패 시 이전 Applied Runtime과 이전 질량을 복구한다. 이 Current 계약은 과거 `CF-FQ-034`의 독립 Feature gate가 아니라 현재 VehicleRuntime/Fitting 구현의 유지 계약으로 취급한다.

질량에 따른 **주행감이 좋은지 나쁜지**는 이 문서의 Technical Ready 판정이 아니다. 향후 질량·장비 변화의 체감 튜닝이 필요하면 `VehicleBuilder.md`의 `Technical Benchmark + USER Feel Pair` 및 `Measurement Gap / Benchmark Extension Ownership` 절차를 사용하며, 과거 `FIT-P0-07D`를 USER PASS로 간주하지 않는다.

## 현재 기능 책임
현재 구현 기준에서 `VehicleRuntime`의 책임은 아래와 같다.

- PreRegister/BeginPlay lifecycle 경계를 유지하면서 차량 런타임 준비를 자동 시작한다.
- VehicleData의 Movement / Reference / WheelPhysics / WheelVisual / TurretVisual / DriveState 설정을 런타임에 반영한다.
- Prepared Fitting Snapshot의 Initial Mass를 Physics 등록 전 적용하고 BeginPlay에서 실제 VehicleMesh 물리 질량을 검증한다.
- 같은 Fitting Snapshot으로 Weapon/Defense Runtime을 Commit하고 Ammo/Launcher/Aim/TargetSelect를 연결한다.
- `bVehicleCoreRuntimeReady`와 `bVehicleCombatRuntimeReady`를 분리해서 관리한다.
- 기존 `bVehicleRuntimeReady`는 Core Ready 호환 상태로 유지한다.
- 장비 변경 후 `RefreshFittingDependentRuntime()`으로 장비 의존 Runtime과 CombatReady를 부분 재구성한다.
- 초기화 및 런타임 상태를 `LastVehicleRuntimeSummary`로 요약한다.
- Core Ready 이전에는 WheelVisual Tick을 막고, Ready 이후에만 휠 시각 갱신을 허용한다.

## 현재 기준 비책임 항목
현재 구현상 `VehicleRuntime`의 직접 책임으로 보지 않는 항목은 아래와 같다.

- 실제 주행 상태 계산 자체
  - 현재 Drive 상태 계산 자체는 `VehicleDriveComp` 책임
- 휠 시각 동기화 알고리즘 자체
  - 실제 휠 시각 계산은 `WheelSyncComp` 책임
- 입력 처리 자체
  - 입력 등록/해석은 `Input` 기능 책임
- 디버그 UI 표시 자체
  - 화면 출력은 `VehicleDebug` 기능 책임
- 카메라 조준 처리 자체
  - 카메라 Look 계산은 `VehicleCameraComp` 책임

즉 현재 `VehicleRuntime`은 계산기나 표시기라기보다,
**차량 런타임 준비를 조립하고 검증하는 준비/판정 기능**에 가깝다.

## Vehicle Pawn Slimming 완료 후 Current 책임 경계

`CF-FQ-048 Vehicle Pawn Slimming` 완료 기준으로 `ACFVehiclePawn`은 더 이상 Visual/Fire/Runtime 세부 orchestration을 한 파일에서 직접 수행하지 않지만, 아래 책임은 계속 Pawn이 소유한다.

```text
Lifecycle owner
- OnConstruction
- PreRegisterAllComponents
- BeginPlay
- EndPlay
- Tick
- SetupPlayerInputComponent

Composition root
- Default Subobject identity / 생성
- BP/SCS component composition
- component 간 조립 진입

Input entry
- Enhanced Input mapping / binding
- Handle* input entry
- Reload / Weapon Select / Active Scan 같은 Pawn-level command facade

Target identity
- ICFTargetSelectable 구현
- TargetPointComp composition
- Target display / location / track identity

Compatibility facade
- 기존 Public / Blueprint / Automation / RuntimeApply seam

Contract state Authority
- Vehicle/Fitting data pointer
- Core / Combat / compatibility Runtime Ready
- LastVehicleRuntimeSummary
- Fire / Turret / Debug 등 observable·serialized·shared state
```

행동 분리는 다음 Current coordinator 경계를 사용한다.

- `UCFVehicleVisualComp`: Chassis/Wheel/Layout/Turret/Owner Visual behavior.
- `UCFVehicleFireComp`: Fire validation, Muzzle/Aim resolve, HitScan/Projectile/Launcher 후속 발사 orchestration.
- `UCFVehicleRuntimeComp`: Initialize/Refresh, Initial Sortie Mass prepare/verify, VehicleData apply orchestration.

반대로 Debug facade/aggregation, `ConfigureVehicleVisualHitCollision()`의 gameplay collision 계약, `WITH_EDITOR` authoring seam, VehicleMove input ownership은 단순 line-count 감소를 위해 다른 coordinator로 이동하지 않는다. P0-05 final facade audit에서 `private-only + consumer0 + Freeze seam 아님 + state Authority 아님 + coordinator complete owner` 5조건을 모두 만족한 추가 삭제 대상은 0건이었다.

CF-FQ-048 closure evidence는 P0-04 Official UE 5.8 Build `241ef0b05aa9455c94ca562fc93d81cc` PASS, final affected exact28 process `6431180d7e7d4b11ac9f8dc51a25b5c2` 28/28 PASS, USER representative regression smoke PASS다. RuntimeRead T0는 UE MCP unavailable 때문에 `Waived / Deferred Observation`으로 남아 있으며, 미관측 runtime 내부값을 PASS로 간주하지 않는다.

USER smoke 중 확인된 Vehicle Builder 신규 차량의 기본 Sensor/Active Scan baseline 누락은 current scanner-less 0-range Sensor 계약과 상위 기본 센서 기획의 별도 불일치다. VPS가 새로 만든 회귀라는 evidence가 없으므로 이 Current VehicleRuntime slimming closure를 다시 열지 않으며, Sensor/Vehicle Builder correction은 별도 lifecycle에서 다룬다.

## 현재 문서 기준의 핵심 결론
현재 `VehicleRuntime` 기능은,

**차량 Pawn이 lifecycle과 상태 Authority를 유지하면서 내부 `VehicleRuntimeComp`에 실행 순서를 위임하고, VehicleData + Prepared Fitting Snapshot + Initial Mass + Domain Runtime 연결을 검증해 Core Ready와 Combat Ready를 구분해서 관리하는 현재 차량 런타임 준비 기능**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `VehicleRuntime`은 Pawn facade를 깨지 않고 차량 데이터·질량·Fitting·장비 Runtime을 하나의 검증된 준비 흐름으로 조립해 Core/Combat 준비 상태를 판정하는 Current System이다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- `WheelSyncComp->TryPrepareWheelSync()` 내부 검증 상세
- `UpdateWheelVisualsPhase2()` 내부 알고리즘 상세
- `VehicleDriveComp->ApplyDriveStateConfig()`의 내부 적용 범위 상세
- 질량 차이에 대한 USER 주행감 적합성 — Technical Runtime Ready와 분리된 관찰/튜닝 범위

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `InitializeVehicleRuntime()`의 준비 순서 변경
- `bVehicleRuntimeReady` 판정 조건 변경
- `ApplyVehicleDataConfig()` 하위 적용 단계 변경
- `PrepareWheelSync()` 준비 조건 변경
- `Tick()`의 WheelVisual 실행 게이트 조건 변경
- `LastVehicleRuntimeSummary` 요약 포맷 변경

## 문서 버전 관리
- 현재 문서 버전: `1.6.0`
- 문서 상태: `Vehicle Pawn Slimming Complete + Runtime Coordinator + Fitting/Mass + Wheel Runtime Current`
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
### v1.6.0 - 2026-09-15
- `CF-FQ-048 Vehicle Pawn Slimming`의 VPS-P0-00~05 완료를 Current System에 최종 승격했다.
- Pawn의 최종 owner를 Lifecycle / Composition root / Input entry / Target identity / Compatibility facade / Contract state Authority로 고정하고 Visual/Fire/Runtime coordinator의 behavior responsibility를 명시했다.
- Debug aggregation, gameplay hit collision, WITH_EDITOR authoring, VehicleMove input은 Slimming line-count 정리 대상으로 이동하지 않는 Current 경계이며 P0-05 final audit에서 추가 facade 삭제 대상 0건을 확인했다.
- 최종 evidence는 P0-04 Official UE 5.8 Build `241ef0b05aa9455c94ca562fc93d81cc` PASS, final exact28 `6431180d7e7d4b11ac9f8dc51a25b5c2` 28/28 PASS, USER representative regression smoke PASS다. RuntimeRead T0는 UE MCP unavailable로 `Waived / Deferred Observation`이며 미관측 내부값은 PASS로 확대하지 않는다.
- Vehicle Builder-created vehicle의 기본 Sensor/Active Scan baseline 누락은 VPS 회귀가 아닌 별도 Sensor/Builder 설계-구현 불일치로 분리했으며 이 문서의 Slimming closure를 다시 열지 않는다.
- 대표 Historical Plan은 `Document/Plan/VehiclePawnSlimming/VehiclePawnSlimmingPlan.md v0.7.0`이다.

Migration: 기존 Blueprint/Product Asset resave는 필요하지 않다. 기존 Pawn Public/BP/Automation/RuntimeApply facade를 계속 사용하며, 내부 구현 위치를 이유로 Consumer 호출 경로를 임의 변경하지 않는다. Sensor/Active Scan baseline correction은 별도 lifecycle로 수행한다.

### v1.5.0 - 2026-09-14
- `CF-FQ-048 / VPS-P0-04 Debug / Header Cleanup`의 현재 타입 owner와 Public include 경계를 반영했다.
- Vehicle input reflected declaration은 `CFVehicleInputTypes.h`, VehicleDebug reflected declaration은 `CFVehicleDebugTypes.h`가 단일 owner이며 `CFVehiclePawn.h`는 두 type header를 소비하는 facade/composition owner로 정리됐다.
- Debug HUD/Panel Public header와 `CarFightVehicleUtils.h`는 Pawn 전체 header 대신 DebugTypes를 직접 소비하고, Pawn 함수 호출이 필요한 Private `.cpp`만 Pawn을 explicit include한다.
- Pawn의 `GetVehicleDebug*` / `ShouldShowVehicleDebug*`, Debug text aggregation과 RuntimeApply `GetVehicleDebugRuntime()` readback 계약은 그대로 유지한다. Runtime behavior, Product Blueprint/SCS/Asset migration은 없다.
- 상세 구현 및 Official UE 5.8 Build + affected exact12 Technical PASS evidence는 대표 `VehiclePawnSlimmingPlan.md v0.6.0`이 소유한다.

Migration: 기존 Blueprint와 Product Asset은 resave가 필요하지 않다. C++에서 입력/Debug DTO만 필요한 Public consumer는 `CFVehiclePawn.h` 대신 각각 `CFVehicleInputTypes.h` / `CFVehicleDebugTypes.h`를 직접 include하고, 실제 Pawn 메서드를 호출하는 translation unit만 Pawn header를 include한다.

### v1.4.0 - 2026-09-14
- `CF-FQ-048 / VPS-P0-03 Runtime Behavior Extraction` 결과를 Current System에 반영했다.
- Pawn이 lifecycle/Public·BP·Automation facade와 observable state Authority를 유지하고 `UCFVehicleRuntimeComp`가 Initialize/Refresh, Initial Mass prepare/verify, VehicleData orchestration 실행만 담당하는 composition 경계를 기록했다.
- Core Ready와 Combat Ready를 분리하고 기존 `bVehicleRuntimeReady`는 Core Ready 호환 의미로 유지하는 현재 판정 계약을 반영했다.
- `ApplyVehicleDataConfig()`의 현재 순서를 Movement → Reference → WheelPhysics → WheelVisual → TurretVisual → DriveState로 교정했다.
- Runtime Vehicle/Equipment Apply가 기존 Pawn facade와 rollback/readback 계약을 계속 재사용하는 경계를 반영했다. P0-03의 Official UE 5.8 Build와 affected exact9 Technical PASS 상세 evidence는 대표 `VehiclePawnSlimmingPlan.md v0.5.0`이 소유한다.

Migration: 기존 Blueprint/Product Asset에는 `VehicleRuntimeComp`를 수동 추가하거나 저장할 필요가 없다. `ACFVehiclePawn`의 기존 Public/BP/Automation 진입점과 Ready/Summary 상태 이름은 유지되며 내부 실행만 coordinator로 분리됐다.

### v1.3.0 - 2026-09-14
- `CF-FQ-034` Rebaseline에서 실제 Source의 Initial Sortie Fitting/Mass 적용 계약을 Current VehicleRuntime으로 승격했다.
- Prepared Fitting Snapshot의 `TotalVehicleMassKg`를 Physics 등록 전에 Chaos Movement Mass에 적용하고, BeginPlay에서 Configured/Actual Mass와 Physics State를 검증한 뒤 같은 Snapshot의 Weapon·Defense Runtime을 Commit하는 경계를 기록했다.
- Field Fitting의 후보 Runtime/Mass 원자 적용과 실패 시 이전 Runtime/Mass 복구 경계를 Current 계약으로 명시했다.
- 질량 체감 비교는 Technical Ready를 대신하지 않으며 향후 필요 시 VehicleBuilder의 `Technical Benchmark + USER Feel Pair` 절차로 수행하도록 ownership을 정리했다. Source/Asset 변경과 신규 USER PASS는 없다.

Migration: 과거 `CF-FQ-034 / FIT-P0-07D`는 Current VehicleRuntime을 여는 필수 gate가 아니다. 기존 quantitative evidence는 Historical로 보존하고, 새 질량·피팅 회귀는 현재 Source와 VehicleRuntime 계약을 기준으로 판단한다.

### v1.2.0 - 2026-09-01
- WSA-P0-07 USER PASS를 Current VehicleRuntime 계약으로 승격했다.
- Socket Scale authority, FL-only shared Wheel Right fallback orientation, per-wheel spin handedness 전달을 Wheel Visual 적용 단계에 반영했다.
- authored Wheel_Mesh full RelativeTransform을 먼저 복원해 Legacy AutoScale/AutoCenter → SocketScale → Manual hot-reinit에서 stale Location/Scale/Rotation을 남기지 않는 current 동작을 기록했다.
- `OnConstruction()`에서는 SCS-authored Wheel transform cache를 fresh recapture하고 runtime hot-reinit에서는 동일 authored base를 유지하는 경계를 반영했다.

### v1.1.0 - 2026-07-08
- `ApplyVehicleWheelVisualConfig()`가 WheelRadius 기준 휠 메시 자동 스케일도 처리할 수 있음을 추가했다.
- 자동 스케일 적용 시점이 WheelSync 준비 캡처 이전임을 명시했다.
- 자동 스케일 후 메시 바운드 중심 보정이 가능함을 추가했다.
- `LastVehicleRuntimeSummary`에 AutoScale / CenterFix 결과 요약이 포함됨을 반영했다.

### v1.0.0 - 2026-04-22
- `VehicleRuntime` 문서 최초 작성
- `ACFVehiclePawn` 기준 런타임 준비 기능 정리
- VehicleData 적용, Drive/WheelSync 준비 검증, Ready 판정, WheelVisual Tick 게이트 역할 중심으로 본문 작성
- 현재 실패 요약 문자열과 현재 운영 스위치까지 포함해 문서화

## 마지막 확인 기준
- 확인 일시: 2026-09-15
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h` v2.171.0
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp` v2.170.0
  - `UE/Source/CarFight_Re/Public/CFVehicleInputTypes.h` v1.0.0
  - `UE/Source/CarFight_Re/Public/CFVehicleDebugTypes.h` v1.0.0
  - `UE/Source/CarFight_Re/Public/CFVehicleRuntimeComp.h` v1.0.0
  - `UE/Source/CarFight_Re/Private/CFVehicleRuntimeComp.cpp` v1.0.0
  - `Document/Plan/VehiclePawnSlimming/VehiclePawnSlimmingPlan.md` v0.7.0
  - P0-04 Official UE 5.8 Build `241ef0b05aa9455c94ca562fc93d81cc` PASS
  - VPS final affected exact28 process `6431180d7e7d4b11ac9f8dc51a25b5c2` 28/28 PASS
  - 2026-09-15 USER representative regression smoke PASS
