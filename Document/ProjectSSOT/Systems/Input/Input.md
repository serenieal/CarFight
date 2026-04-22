# Input

## 문서 목적
이 문서는 현재 프로젝트에서 `Input` 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 자산/클래스/설정 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

## 문서 범위
이 문서에서 말하는 `Input` 기능은 아래 요소를 묶어서 본다.

- 입력 자산: `/Game/CarFight/Input/*`
- 기본 입력 매핑 컨텍스트: `/Game/CarFight/Input/IMC_Vehicle_Default`
- 핵심 입력 해석 주체: `ACFVehiclePawn`
- 입력 등록 주체: `ACFVehiclePawn::RegisterDefaultInputMappingContext()`
- 입력 바인딩 주체: `ACFVehiclePawn::SetupPlayerInputComponent()`

즉, 현재 기준 `Input`은 **Input 폴더의 액션 자산만을 뜻하는 것이 아니라, InputAction + InputMappingContext + Pawn 입력 해석 로직을 합친 기능 묶음**으로 본다.

## 이 기능이 현재 실제로 하는 일
현재 구현 기준 `Input` 기능의 핵심 역할은 **차량 조작에 필요한 다양한 장치 입력을 받아, 현재 상황에 맞는 차량 입력값으로 해석하고, 이를 DriveComp와 CameraComp에 전달하는 것**이다.

현재 `Input`은 단순 키 바인딩 묶음이 아니다.
현재 구현을 기준으로 보면, 이 기능은 아래 일을 한다.

### 1. 여러 장치 입력을 하나의 차량 입력 체계로 묶는다
현재 `IMC_Vehicle_Default`는 키보드/마우스, 게임패드, 그리고 일부 XR 계열 입력까지 하나의 기본 매핑 컨텍스트로 묶는다.

현재 확인된 내용:
- `IMC_Vehicle_Default`의 `Default Key Mappings`에는 **40개 매핑**이 존재한다.
- 같은 액션에 대해 장치별 키가 여러 개 연결돼 있다.
- 현재 매핑은 단순 PC 키보드만이 아니라, 게임패드 및 VR/XR 입력까지 포함한다.

대표 예시:
- `IA_Throttle`
  - `W`
  - `Up`
  - `Vive_Right_Trigger_Axis`
  - `MixedReality_Right_Trigger_Axis`
  - `OculusTouch_Right_Trigger_Axis`
  - `ValveIndex_Right_Trigger_Axis`
- `IA_Brake`
  - `S`
  - `Down`
  - `Vive_Left_Trigger_Axis`
  - `MixedReality_Left_Trigger_Axis`
  - `OculusTouch_Left_Trigger_Axis`
  - `ValveIndex_Left_Trigger_Axis`
- `IA_Steering`
  - `D`
  - `A` (`Negate` modifier 확인)
  - 그 외 추가 아날로그 계열 매핑 존재
- `IA_LookAround`
  - `Mouse2D`
  - `Gamepad_Right2D`
- `IA_VehicleMove`
  - `Gamepad_Left2D` (`DeadZone` modifier 확인)
- `IA_Handbrake`
  - `SpaceBar`
  - `Gamepad_LeftThumbstick`
  - 일부 XR Grip / Button 계열 입력
- `IA_ToggleCamera`
  - `Tab`
  - `Gamepad_Special_Left`
- `IA_Reset`
  - `BackSpace`
  - `Gamepad_Special_Right`

즉 현재 `Input` 기능은 **다양한 입력 장치를 하나의 차량 조작 체계로 수렴시키는 통합 입력 레이어** 역할을 한다.

### 2. 차량 조작 입력을 두 개의 경로로 운영한다
현재 `ACFVehiclePawn` 기준 입력 조작 경로는 두 갈래다.

#### A. Legacy 축 입력 경로
현재 바인딩된 액션:
- `InputAction_Throttle`
- `InputAction_Steering`
- `InputAction_Brake`
- `InputAction_Handbrake`

현재 역할:
- 1D 축 입력을 직접 읽는다.
- `SetVehicleThrottleInput()`
- `SetVehicleSteeringInput()`
- `SetVehicleBrakeInput()`
- `SetVehicleHandbrakeInput()`
으로 전달한다.

즉 이 경로는 **전통적인 개별 축 입력 기반 차량 조작 경로**다.

#### B. VehicleMove 2D 입력 경로
현재 바인딩된 액션:
- `InputAction_VehicleMove`

현재 역할:
- `FVector2D` 입력을 한 번에 받는다.
- 입력 벡터의 크기와 각도를 해석한다.
- 이를 조향 / 쓰로틀 / 브레이크 값으로 다시 변환한다.
- 최종 결과를 `ApplyResolvedVehicleMoveInput()`을 통해 차량 입력으로 적용한다.

즉 이 경로는 **하나의 2D 입력으로 전진/후진/브레이크/조향을 동시에 해석하는 통합 조작 경로**다.

현재 `Input` 기능에서 중요한 점은, 이 두 경로가 동시에 존재한다는 것이다.
즉 현재 시스템은 **Legacy 축 입력 체계와 VehicleMove 2D 체계를 병행 운영**하고 있다.

### 3. 현재 입력을 실제 차량 동작으로 해석한다
현재 `VehicleMove` 경로는 단순 스틱 전달이 아니라, 해석 과정을 가진다.

관련 구조:
- `FCFVehicleMoveInputConfig`
- `FCFVehicleMoveInputResult`
- `ResolveVehicleMoveInput()`
- `ConvertMoveInputToAngleDeg()`
- `IsAngleWithinRange()`
- `ResolveDirectionIntentFallback()`

현재 해석 결과로 관리되는 값:
- 원본 2D 입력 벡터
- 입력 강도
- 입력 각도
- 해석된 입력 존(`ResolvedZone`)
- 해석된 방향 의도(`ResolvedDirectionIntent`)
- 최종 조향 출력값
- 최종 쓰로틀 출력값
- 최종 브레이크 출력값
- 검은 영역 유지 정책 사용 여부

즉 현재 `Input` 기능은 키를 직접 함수로 보내는 것이 아니라,
**특히 VehicleMove 2D 입력에 대해서는 "입력 해석 계층"을 거쳐 차량 제어값으로 변환하는 역할**을 수행한다.

### 4. 입력 충돌을 막기 위해 소유권을 관리한다
현재 시스템에는 `CurrentInputOwnership` 개념이 있다.

소유권 상태:
- `None`
- `VehicleMove2D`
- `LegacyAxis`

관련 로직:
- `CanProcessVehicleMoveInput()`
- `CanProcessLegacyAxisInput()`
- `UpdateInputOwnershipFromVehicleMove()`
- `UpdateInputOwnershipFromLegacyAxis()`
- `ReleaseInputOwnershipIfIdle()`
- `InputOwnershipHoldTimeSec`

현재 의미:
- VehicleMove 2D 입력이 주도권을 잡고 있는 동안에는 Legacy 축 입력이 바로 끼어들지 못하게 한다.
- 반대로 Legacy 축 입력이 주도권을 잡고 있는 동안에는 VehicleMove 2D 입력이 바로 끼어들지 못하게 한다.
- 마지막 입력 시각과 `InputOwnershipHoldTimeSec`을 이용해 입력 흔들림을 줄인다.

즉 현재 `Input` 기능은 단순 입력 수집이 아니라,
**복수 입력 경로가 동시에 존재할 때 어떤 입력이 현재 프레임의 주도권을 가지는지 관리하는 입력 충돌 방지 시스템** 역할도 한다.

### 5. 장치 모드 필터링을 수행한다
현재 `ACFVehiclePawn`에는 `InputDeviceMode`가 있다.

모드:
- `Auto`
- `KeyboardMouseOnly`
- `GamepadOnly`

관련 로직:
- `ShouldAcceptActionInput()`
- `HasActiveMappedKeyForDevice()`
- `IsMappedKeyCurrentlyActive()`
- `InputDeviceAnalogThreshold`

현재 동작:
- `Auto`면 입력을 모두 허용한다.
- `KeyboardMouseOnly` 또는 `GamepadOnly`면,
  - 현재 액션에 연결된 키 중
  - 지정 장치 타입과 일치하는 키가
  - 실제로 눌렸거나 아날로그 임계값 이상인지 확인한 뒤에만 입력을 허용한다.

즉 현재 `Input` 기능은 **어떤 장치의 입력을 받을 것인지까지 런타임에서 필터링하는 장치 게이트 역할**도 한다.

### 6. 카메라 입력도 함께 처리한다
현재 `InputAction_Look`은 차량 조작과 별도로 `VehicleCameraComp`에 전달된다.

관련 함수:
- `HandleLookInput()`
- `HandleLookReleased()`

현재 동작:
- 입력 중에는 `VehicleCameraComp->SetLookInput()` 호출
- 입력 종료 시 `VehicleCameraComp->ClearLookInput()` 호출

즉 현재 `Input` 기능은 차량 이동 입력만이 아니라,
**차량 카메라 자유 조준 입력도 같은 입력 시스템 안에서 함께 처리**한다.

## 현재 기준 기능의 성격 정리
현재 구현을 종합하면 `Input`은 아래 역할을 가진다.

1. **입력 통합 레이어**
   - 키보드/마우스, 게임패드, XR 입력을 하나의 IMC로 묶음
   - 같은 액션에 여러 장치 입력을 연결함

2. **차량 조작 해석 레이어**
   - Legacy 축 입력을 DriveComp 입력으로 전달함
   - VehicleMove 2D 입력을 조향/쓰로틀/브레이크로 재해석함

3. **입력 충돌 제어 레이어**
   - `VehicleMove2D`와 `LegacyAxis` 사이의 주도권 관리
   - 장치 모드 필터링
   - 아날로그 임계값 기준 유효 입력 판정

4. **카메라 조작 입력 레이어**
   - LookAround 입력을 VehicleCameraComp로 전달

따라서 현재 이 기능은 단순한 `InputAction` 모음이 아니라,
**차량 조작 입력을 등록하고, 장치별로 필터링하고, 충돌을 제어하고, 최종 차량/카메라 입력으로 변환하는 현재 입력 운영 시스템**이라고 보는 것이 맞다.

## 현재 동작 방식
현재 `Input`은 아래 방식으로 동작한다.

### 1. 기본 매핑 등록
`ACFVehiclePawn::RegisterDefaultInputMappingContext()`가 현재 컨트롤러의 `LocalPlayer`에서 `UEnhancedInputLocalPlayerSubsystem`를 가져와,
`DefaultInputMappingContext`를 `InputMappingPriority`로 등록한다.

즉 현재 입력은 **Pawn이 자신의 기본 IMC를 직접 등록하는 구조**다.

### 2. 액션 바인딩
`ACFVehiclePawn::SetupPlayerInputComponent()`에서 현재 확인된 액션이 바인딩된다.

현재 직접 바인딩된 액션:
- `InputAction_VehicleMove`
- `InputAction_Throttle`
- `InputAction_Steering`
- `InputAction_Brake`
- `InputAction_Look`
- `InputAction_Handbrake`

바인딩 방식:
- `VehicleMove / Throttle / Steering / Brake / Look`는 Triggered + Completed 기반
- `Handbrake`는 Started + Completed 기반

즉 현재 핸드브레이크는 **누르는 동안 On, 떼면 Off**인 디지털성 입력으로 처리된다.

### 3. 입력 종료 처리
현재 구현은 입력 시작만이 아니라 종료도 함께 처리한다.

예:
- `HandleVehicleMoveReleased()`
- `HandleThrottleReleased()`
- `HandleSteeringReleased()`
- `HandleBrakeReleased()`
- `HandleLookReleased()`
- `HandleHandbrakeCompleted()`

즉 현재 입력 시스템은 **입력 해제 시 상태/축 값을 0으로 되돌리는 복귀 처리**까지 포함한다.

## 현재 표시 조건 / 실행 조건
현재 `Input` 기능이 실제로 동작하려면 아래 조건이 중요하다.

- `PlayerInputComponent`가 `UEnhancedInputComponent`여야 함
- `DefaultInputMappingContext`가 유효해야 함
- `GetController()` → `PlayerController` → `LocalPlayer` → `EnhancedInputLocalPlayerSubsystem` 경로가 유효해야 함
- 장치 모드가 `Auto`가 아니면, 현재 입력값이 `InputDeviceAnalogThreshold` 이상이어야 함
- 장치 모드가 제한되어 있으면, 실제 눌린 키가 해당 장치 타입과 일치해야 함

즉 현재 입력 시스템은 자산만 있다고 살아 있는 것이 아니라,
**로컬 플레이어 컨텍스트와 Enhanced Input 서브시스템이 정상 구성돼야 실제 동작**한다.

## 현재 자산 / 클래스 역할
### `IMC_Vehicle_Default`
- 종류: `InputMappingContext`
- 현재 역할: 차량 기본 입력 매핑의 중심 컨텍스트
- 현재 특징: 40개 매핑을 보유하며, 키보드/게임패드/XR 계열 입력을 함께 묶는다.

### `IA_VehicleMove`
- 종류: `InputAction`
- 현재 역할: 2D 차량 이동 입력
- 현재 타입: `Axis2D`
- 현재 특징: `TakeHighestAbsoluteValue` 누적 방식 사용

### `IA_Throttle`
- 종류: `InputAction`
- 현재 역할: 개별 쓰로틀 축 입력
- 현재 타입: `Axis1D`

### `IA_Brake`
- 종류: `InputAction`
- 현재 역할: 개별 브레이크 축 입력
- 현재 특징: 현재 IMC에서 키보드/트리거 계열로 연결됨

### `IA_Steering`
- 종류: `InputAction`
- 현재 역할: 개별 조향 축 입력
- 현재 특징: `D`, `A(Negate)` 등으로 좌우 방향 입력 구성

### `IA_LookAround`
- 종류: `InputAction`
- 현재 역할: 카메라 Look 입력
- 현재 특징: `Mouse2D`, `Gamepad_Right2D` 매핑 확인

### `IA_Handbrake`
- 종류: `InputAction`
- 현재 역할: 핸드브레이크 입력
- 현재 특징: SpaceBar / Gamepad_LeftThumbstick / XR grip 계열 매핑 확인

### `ACFVehiclePawn`
- 종류: `C++ Pawn`
- 현재 역할: 입력 등록, 바인딩, 해석, 주도권 관리, 최종 전달의 중심 주체

### `UCFVehicleDriveComp`
- 종류: `C++ Component`
- 현재 역할: 최종 차량 주행 입력 적용 대상

### `UCFVehicleCameraComp`
- 종류: `C++ Component`
- 현재 역할: Look 입력 적용 대상

## 현재 생성 및 연결 구조
현재 입력의 연결 구조는 아래와 같다.

1. Pawn BeginPlay / SetupPlayerInputComponent 진입
2. `RegisterDefaultInputMappingContext()`로 기본 IMC 등록 시도
3. `SetupPlayerInputComponent()`에서 InputAction 바인딩
4. 액션 발생 시 각 핸들러 호출
5. 장치 모드 필터링 및 유효 입력 판정
6. 필요 시 입력 소유권 갱신
7. DriveComp 또는 CameraComp에 최종 입력 적용

현재 구조 해석:
- 입력 시스템은 현재 `ACFVehiclePawn` 종속 구조다.
- 전역 InputManager가 따로 해석하는 구조가 아니라, Pawn이 직접 입력을 소유하고 처리한다.
- 기본 IMC 등록도 Pawn이 직접 수행한다.
- 최종 출력은 DriveComp / CameraComp에 분기 전달된다.

## 현재 기능 책임
현재 구현 기준에서 `Input` 기능의 책임은 아래와 같다.

- 기본 입력 매핑 컨텍스트를 로컬 플레이어에 등록한다.
- 차량 조작 액션을 바인딩한다.
- 장치별 입력을 하나의 차량 조작 체계로 통합한다.
- VehicleMove 2D 입력을 차량 조작값으로 해석한다.
- Legacy 축 입력과 VehicleMove 2D 입력 사이의 충돌을 제어한다.
- 장치 모드와 아날로그 임계값 기준으로 입력 허용 여부를 판정한다.
- 최종 입력을 DriveComp / CameraComp에 전달한다.

## 현재 기준 비책임 항목
현재 구현상 `Input` 기능의 직접 책임으로 보지 않는 항목은 아래와 같다.

- 차량 물리 상태 계산 자체
  - 현재 Drive 상태 계산 자체는 `VehicleDriveComp` 책임
- 카메라 실제 시점 계산 자체
  - Look 입력 전달 이후의 카메라 해석은 `VehicleCameraComp` 책임
- UI 디버그 표시 자체
  - 입력 결과를 화면에 보여주는 것은 `VehicleDebug` 계층 책임
- 입력 자산 전체의 프로젝트 전역 사용 여부 확정
  - 예: `IA_Headlights`, `IA_ShifterUp`, `IA_ShifterDown`, `IA_ToggleCamera`, `IA_Reset`는 IMC에 존재하지만, 현재 확인한 `ACFVehiclePawn` 바인딩 범위에서는 직접 사용이 확인되지 않았다.

## 현재 문서 기준의 핵심 결론
현재 `Input` 기능은,

**차량용 입력 자산과 기본 매핑 컨텍스트를 바탕으로, 여러 장치 입력을 수집하고, 현재 장치 모드와 입력 소유권 규칙에 따라 이를 해석한 뒤, 최종 차량 주행 입력과 카메라 입력으로 전달하는 현재 입력 운영 시스템**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `Input`은 현재 차량 조작에 필요한 입력을 등록하고, 장치별로 필터링하고, 2D/Legacy 입력 충돌을 제어하면서, 최종 차량 조작값으로 변환해 적용하는 현재 상태 기능이다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- `IA_Headlights`, `IA_ShifterUp`, `IA_ShifterDown`, `IA_ToggleCamera`, `IA_Reset`의 프로젝트 전체 실제 소비 지점
- `IA_VehicleMove`의 추가 비게임패드 매핑 존재 여부 전체 확인
- 블루프린트 레벨에서 입력을 추가 소비하는 로직 존재 여부
- Pawn 외 다른 입력 소비 주체가 별도로 있는지 여부

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `IMC_Vehicle_Default` 매핑 구성 변경
- `SetupPlayerInputComponent()` 바인딩 액션 변경
- `RegisterDefaultInputMappingContext()` 등록 방식 변경
- `VehicleMove` 입력 해석 규칙 변경
- `InputDeviceMode` / `InputOwnership` 규칙 변경
- 입력 최종 전달 대상이 바뀌는 경우

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
- `Input` 문서 최초 작성
- `IMC_Vehicle_Default`, 개별 `IA_*`, `ACFVehiclePawn` 기준으로 현재 입력 기능 정리
- 단순 키 목록이 아니라, 현재 입력 등록/해석/장치 필터링/소유권 관리 역할 중심으로 본문 작성
- 현재 사용 중인 입력과 현재 `CFVehiclePawn` 바인딩 범위에서 직접 사용 확인되지 않은 입력을 구분해 기록

## 마지막 확인 기준
- 확인 일시: 2026-04-22
- 확인 근거:
  - `UE/Content/CarFight/Input/*`
  - `/Game/CarFight/Input/IMC_Vehicle_Default`
  - `/Game/CarFight/Input/IA_VehicleMove`
  - `/Game/CarFight/Input/IA_LookAround`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
