# VehicleRuntime

## 문서 목적
이 문서는 현재 프로젝트에서 `VehicleRuntime` 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 자산/클래스/설정 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

## 문서 범위
이 문서에서 말하는 `VehicleRuntime` 기능은 아래 요소를 묶어서 본다.

- 핵심 주체: `ACFVehiclePawn`
- 관련 컴포넌트: `UCFVehicleDriveComp`, `UCFWheelSyncComp`
- 관련 데이터: `UCFVehicleData`
- 핵심 함수:
  - `BeginPlay()`
  - `InitializeVehicleRuntime()`
  - `ApplyVehicleDataConfig()`
  - `PrepareWheelSync()`
  - `UpdateVehicleWheelVisuals()`
  - `ResolveVehicleMovementComponent()`
- 관련 상태:
  - `bAutoInitializeOnBeginPlay`
  - `bVehicleRuntimeReady`
  - `LastVehicleRuntimeSummary`
  - `bEnableWheelVisualTick`

즉, 현재 기준 `VehicleRuntime`은 **차량 Pawn이 플레이 시작 시 자신의 데이터/주행/휠 시각 구성을 실제 런타임 상태로 준비시키는 기능 묶음**으로 본다.

## 이 기능이 현재 실제로 하는 일
현재 구현 기준 `VehicleRuntime`의 핵심 역할은 **차량 Pawn이 플레이 시작 시 VehicleData를 실제 차량 이동 컴포넌트와 WheelSync 구성에 적용하고, Drive/WheelSync 준비 성공 여부를 검사한 뒤, 최종적으로 이 차량이 런타임 준비 완료 상태인지 판정하는 것**이다.

이 기능은 단순히 `BeginPlay에서 뭔가 초기화한다` 수준이 아니다.
현재 구현 기준으로 보면, 이 기능은 아래 4단계 파이프라인으로 동작한다.

### 1. 런타임 초기화 시도를 시작한다
현재 `ACFVehiclePawn::BeginPlay()`는 아래 순서로 동작한다.

1. `Super::BeginPlay()`
2. `bAutoRegisterInputMappingContext`가 켜져 있으면 입력 매핑 등록 시도
3. `bAutoInitializeOnBeginPlay`가 켜져 있으면 `InitializeVehicleRuntime()` 호출

즉 현재 `VehicleRuntime`은 기본적으로 **BeginPlay 자동 초기화 기능**을 가진다.
별도 외부 호출 없이도 Pawn이 시작되면 스스로 런타임 준비를 시도한다.

그리고 `InitializeVehicleRuntime()`가 시작되면 가장 먼저 아래 상태를 초기화한다.

- `bVehicleRuntimeReady = false`
- `LastVehicleRuntimeSummary = "VehicleRuntime: InitializeStarted"`

즉 현재 기능은 매 초기화 시도마다 먼저 이전 성공 상태를 믿지 않고,
**항상 Not Ready 상태로 되돌린 뒤 다시 준비를 검증하는 보수적 흐름**을 갖는다.

### 2. VehicleData 기반 설정을 실제 런타임 구성에 적용한다
현재 `InitializeVehicleRuntime()`는 가장 먼저 `ApplyVehicleDataConfig()`를 호출한다.

그리고 `ApplyVehicleDataConfig()`는 현재 아래 하위 적용 단계들을 순서대로 호출한다.

1. `ApplyVehicleMovementConfig()`
2. `ApplyVehicleReferenceConfig()`
3. `ApplyVehicleWheelPhysicsConfig()`
4. `ApplyVehicleWheelVisualConfig()`
5. `VehicleDriveComp->ApplyDriveStateConfig(VehicleData->DriveStateConfig)`

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

현재 이 단계가 하는 일:

- `WheelSyncComp->ExpectedWheelCount` 적용
- `WheelSyncComp->FrontWheelCountForSteering` 적용
- `Wheel_Mesh_FL`에 FL 휠 메쉬 적용
- `Wheel_Mesh_FR`에 FR 휠 메쉬 적용
- `Wheel_Mesh_RL`에 RL 휠 메쉬 적용
- `Wheel_Mesh_RR`에 RR 휠 메쉬 적용

즉 현재 `VehicleRuntime`은 **휠이 몇 개인지, 앞바퀴가 몇 개인지, 실제 시각 휠 메쉬가 무엇인지까지 런타임에 반영하는 기능**도 포함한다.

현재 이 단계가 끝나면 `LastVehicleRuntimeSummary`에는 대략 아래 의미가 기록된다.

- ExpectedWheelCount
- FrontWheelCountForSteering

#### 2-5. Drive 상태 설정 적용
`ApplyVehicleDataConfig()` 마지막에는,
현재 `VehicleDriveComp`와 `VehicleData`가 모두 있을 때만 `VehicleDriveComp->ApplyDriveStateConfig(VehicleData->DriveStateConfig)`를 호출한다.

즉 현재 `VehicleRuntime`은 단순 Movement와 Wheel만 준비하는 기능이 아니라,
**Drive 상태 머신이 어떤 설정으로 동작할지도 런타임 초기화 단계에서 함께 적용하는 기능**이다.

### 3. Drive 준비와 WheelSync 준비를 검증한다
현재 `InitializeVehicleRuntime()`는 VehicleData 적용 이후,
아래 두 준비 상태를 별도로 검사한다.

- `bDriveReady`
- `bWheelSyncReady`

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

### 4. 최종 Ready 상태를 판정하고 요약을 남긴다
현재 `InitializeVehicleRuntime()`의 최종 성공 조건은 단순하다.

- `bVehicleRuntimeReady = bDriveReady && bWheelSyncReady`

즉 현재 런타임 준비 완료는

- VehicleData 존재만으로도 아니고,
- Movement 적용만으로도 아니고,
- Drive 준비만으로도 아니고,
- WheelSync 준비만으로도 아니다.

**Drive 준비와 WheelSync 준비가 둘 다 성공해야만 Ready**가 된다.

그리고 마지막에는 아래 의미의 최종 요약 문자열이 기록된다.

- VehicleData가 있는지 (`Data=Present/Missing`)
- Drive가 준비됐는지 (`Drive=Ready/Missing`)
- WheelSync가 준비됐는지 (`WheelSync=Ready/Missing`)
- 최종 Ready가 true인지 (`Ready=True/False`)
- 그 직전에 수행된 DataConfigSummary

즉 현재 `VehicleRuntime`은 단순 성공/실패 bool만 남기지 않고,
**이번 초기화 시도에서 무엇이 있었고 무엇이 부족했는지를 한 줄 요약으로 남기는 진단 기능**도 함께 가진다.

## 현재 기준 기능의 성격 정리
현재 구현을 종합하면 `VehicleRuntime`은 아래 역할을 가진다.

1. **차량 데이터 반영 기능**
   - VehicleData의 주행/참조/휠 물리/휠 시각/DriveState 설정을 실제 런타임 상태에 적용함

2. **차량 준비 상태 검증 기능**
   - DriveComp가 실제 VehicleMovementComponent를 확보했는지 검증함
   - WheelSyncComp가 실제 준비를 마쳤는지 검증함

3. **차량 준비 완료 판정 기능**
   - `bVehicleRuntimeReady`로 최종 Ready 상태를 결정함

4. **런타임 진단 요약 기능**
   - `LastVehicleRuntimeSummary`에 현재 초기화/적용/실패 상태를 문자열로 남김

5. **런타임 이후 WheelVisual 업데이트 게이트 기능**
   - Ready가 아니면 WheelVisual Tick을 막고, Ready일 때만 휠 시각 갱신을 진행함

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
- 현재 역할: 런타임 준비 파이프라인의 오케스트레이터
- 현재 기능: BeginPlay 자동 시작, 데이터 적용 호출, Ready 판정, 요약 문자열 관리

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

1. Pawn 생성 시 `VehicleDriveComp`, `WheelSyncComp`, `VehicleCameraComp` 생성
2. `BeginPlay()` 진입
3. `bAutoInitializeOnBeginPlay`가 켜져 있으면 `InitializeVehicleRuntime()` 호출
4. `ApplyVehicleDataConfig()`로 VehicleData 반영
5. `VehicleDriveComp->CacheVehicleMovementComponent()`로 Drive 준비 검사
6. `PrepareWheelSync()`로 WheelSync 준비 검사
7. 두 검사 모두 성공하면 `bVehicleRuntimeReady = true`
8. 이후 Tick에서 Ready일 때만 `UpdateVehicleWheelVisuals()` 수행

현재 구조 해석:
- 이 기능은 전역 시스템이 아니라 Pawn 종속 런타임 기능이다.
- 실제 하위 작업은 각 컴포넌트로 위임하지만, 최종 준비 상태 판단은 Pawn이 가진다.
- VehicleData 적용과 준비 검증이 같은 초기화 파이프라인 안에 묶여 있다.

## 현재 기능 책임
현재 구현 기준에서 `VehicleRuntime`의 책임은 아래와 같다.

- BeginPlay 기준 차량 런타임 준비를 자동 시작한다.
- VehicleData의 Movement / Reference / WheelPhysics / WheelVisual / DriveState 설정을 런타임에 반영한다.
- DriveComp가 실제 VehicleMovementComponent를 사용할 준비가 됐는지 검증한다.
- WheelSyncComp가 실제 휠 시각 동기화 준비를 마쳤는지 검증한다.
- 최종 Ready 상태를 `bVehicleRuntimeReady`로 관리한다.
- 초기화 및 런타임 상태를 `LastVehicleRuntimeSummary`로 요약한다.
- Ready 이전에는 WheelVisual Tick을 막고, Ready 이후에만 휠 시각 갱신을 허용한다.

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

## 현재 문서 기준의 핵심 결론
현재 `VehicleRuntime` 기능은,

**차량 Pawn이 BeginPlay 시점에 VehicleData를 실제 주행/휠/Drive 설정에 반영하고, Drive와 WheelSync 준비를 검증한 뒤, 이 차량이 실제 런타임 준비 완료 상태인지 판정하고 그 결과를 요약 문자열로 유지하는 현재 런타임 준비 기능**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `VehicleRuntime`은 현재 차량이 실제 플레이 가능한 준비 상태인지 판단하기 위해, 데이터 적용과 준비 검증을 수행하고 최종 Ready 상태를 관리하는 현재 상태 기능이다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- `WheelSyncComp->TryPrepareWheelSync()` 내부 검증 상세
- `UpdateWheelVisualsPhase2()` 내부 알고리즘 상세
- `VehicleDriveComp->ApplyDriveStateConfig()`의 내부 적용 범위 상세
- 런타임 초기화를 외부에서 재호출하는 운영 경로가 별도로 있는지 여부

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `InitializeVehicleRuntime()`의 준비 순서 변경
- `bVehicleRuntimeReady` 판정 조건 변경
- `ApplyVehicleDataConfig()` 하위 적용 단계 변경
- `PrepareWheelSync()` 준비 조건 변경
- `Tick()`의 WheelVisual 실행 게이트 조건 변경
- `LastVehicleRuntimeSummary` 요약 포맷 변경

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
### v1.0.0 - 2026-04-22
- `VehicleRuntime` 문서 최초 작성
- `ACFVehiclePawn` 기준 런타임 준비 기능 정리
- VehicleData 적용, Drive/WheelSync 준비 검증, Ready 판정, WheelVisual Tick 게이트 역할 중심으로 본문 작성
- 현재 실패 요약 문자열과 현재 운영 스위치까지 포함해 문서화

## 마지막 확인 기준
- 확인 일시: 2026-04-22
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
