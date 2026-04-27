# VehicleData

## 문서 목적
이 문서는 현재 프로젝트에서 `VehicleData` 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 자산/클래스/설정 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

## 문서 범위
이 문서에서 말하는 `VehicleData` 기능은 아래 요소를 묶어서 본다.

- 핵심 데이터 클래스: `UCFVehicleData`
- 데이터 정의 파일: `UE/Source/CarFight_Re/Public/CFVehicleData.h`
- 현재 대표 데이터 자산: `/Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar`
- 현재 기본 소비 주체: `/Game/CarFight/Vehicles/BP_CFVehiclePawn`
- 현재 직접 소비 함수:
  - `ApplyVehicleDataConfig()`
  - `ApplyVehicleMovementConfig()`
  - `ApplyVehicleReferenceConfig()`
  - `ApplyVehicleWheelPhysicsConfig()`
  - `ApplyVehicleWheelVisualConfig()`
  - `VehicleDriveComp->ApplyDriveStateConfig(...)`

즉, 현재 기준 `VehicleData`는 **차량 설정값을 저장만 하는 DataAsset이 아니라, VehicleRuntime이 실제 차량 주행/휠/시각/Drive 상태 구성으로 풀어 적용하는 루트 차량 데이터 기능**으로 본다.

## 이 기능이 현재 실제로 하는 일
현재 구현 기준 `VehicleData`의 핵심 역할은 **차량 하나의 시각 자산, 주행 설정, 휠 물리 설정, 휠 시각 설정, Wheel Class 참조, DriveState 판정 설정을 하나의 루트 데이터 자산으로 묶어두고, VehicleRuntime이 이를 읽어 실제 차량 런타임 구성에 반영할 수 있게 하는 것**이다.

이 기능은 단순히 `차량 스펙을 저장한다` 정도가 아니다.
현재 구조상 `VehicleData`는 아래 다섯 개의 하위 데이터 축을 한 자산에 묶는다.

### 1. 차량 시각 자산을 정의한다
`UCFVehicleData`의 `VehicleVisualConfig`는 현재 차량의 시각 자산 참조를 가진다.

현재 포함 항목:
- `ChassisMesh`
- `WheelMeshFL`
- `WheelMeshFR`
- `WheelMeshRL`
- `WheelMeshRR`

현재 의미:
- 차체에 어떤 Static Mesh를 사용할지 정한다.
- 앞/뒤, 좌/우 휠에 어떤 Static Mesh를 사용할지 정한다.
- VehicleRuntime의 `ApplyVehicleWheelVisualConfig()`에서 실제 Pawn의 `Wheel_Mesh_*` 컴포넌트에 반영된다.

즉 현재 `VehicleData`는 **차량 외형을 구성하는 핵심 Static Mesh 세트를 제공하는 시각 자산 루트**다.

### 2. 차량 주행 및 휠 물리 기본값을 정의한다
`VehicleMovementConfig`는 현재 `VehicleData`에서 가장 큰 비중을 차지하는 설정 묶음이다.

현재 포함되는 주요 범주:

#### A. Wheel Class 기본 물리 보정값
- `FrontWheelMaxSteerAngle`
- `FrontWheelMaxBrakeTorque`
- `RearWheelMaxBrakeTorque`
- `RearWheelMaxHandBrakeTorque`
- `FrontWheelRadius`
- `RearWheelRadius`
- `FrontWheelWidth`
- `RearWheelWidth`
- `FrontWheelFrictionForceMultiplier`
- `RearWheelFrictionForceMultiplier`
- `FrontWheelCorneringStiffness`
- `RearWheelCorneringStiffness`
- `FrontWheelLoadRatio`
- `RearWheelLoadRatio`
- `FrontWheelSpringRate`
- `RearWheelSpringRate`
- `FrontWheelSpringPreload`
- `RearWheelSpringPreload`
- `FrontWheelSuspensionMaxRaise`
- `RearWheelSuspensionMaxRaise`
- `FrontWheelSuspensionMaxDrop`
- `RearWheelSuspensionMaxDrop`
- `bFrontWheelAffectedByEngine`
- `bRearWheelAffectedByEngine`
- `FrontWheelSweepShape`
- `RearWheelSweepShape`

#### B. VehicleMovementComponent 본체 설정값
- `ChassisHeight`
- `DragCoefficient`
- `DownforceCoefficient`
- `bEnableCenterOfMassOverride`
- `CenterOfMassOverride`
- `EngineMaxTorque`
- `EngineMaxRPM`
- `EngineIdleRPM`
- `EngineBrakeEffect`
- `EngineRevUpMOI`
- `EngineRevDownRate`
- `DifferentialType`
- `FrontRearSplit`
- `SteeringType`
- `SteeringAngleRatio`
- `bLegacyWheelFrictionPosition`
- `FrontWheelAdditionalOffset`
- `RearWheelAdditionalOffset`

#### C. 런타임 적용 스위치
- `bUseMovementOverrides`
- `MovementProfileName`

현재 의미:
- 차가 얼마나 강하게 가속하는지
- 최고 회전수와 엔진 브레이크가 어떤지
- 조향이 어떤 방식인지
- 앞/뒤 바퀴가 어떤 물리 특성을 가지는지
- WheelSetup의 위치 보정이 어떻게 되는지

를 한 묶음으로 정의한다.

즉 현재 `VehicleData`는 **차량이 어떤 주행 성격과 어떤 바퀴 물리 특성을 가지는지 정의하는 주행/물리 데이터 루트**다.

### 3. 휠 시각 동기화 기준값을 정의한다
`WheelVisualConfig`는 현재 휠 시각 동기화에 필요한 최소 기준값을 가진다.

현재 포함 항목:
- `bUseWheelVisualOverrides`
- `ExpectedWheelCount`
- `FrontWheelCountForSteering`

현재 의미:
- 이 차량이 바퀴를 몇 개로 보는지
- 그중 몇 개를 조향 바퀴로 보는지

현재 `ApplyVehicleWheelVisualConfig()`는 이 값을 `WheelSyncComp`에 반영한다.

즉 현재 `VehicleData`는 **휠 시각 시스템이 차량을 어떤 형태로 해석해야 하는지 알려주는 WheelSync 입력 데이터**도 함께 가진다.

### 4. 런타임 참조형 자산을 정의한다
`VehicleReferenceConfig`는 현재 앞/뒤 Wheel Class 참조를 가진다.

현재 포함 항목:
- `FrontWheelClass`
- `RearWheelClass`

현재 의미:
- 앞바퀴에 어떤 Wheel Blueprint/Class를 쓸지
- 뒷바퀴에 어떤 Wheel Blueprint/Class를 쓸지

이 값은 `ApplyVehicleWheelPhysicsConfig()`에서 `WheelSetups`에 반영된다.

즉 현재 `VehicleData`는 **수치만 담는 데이터가 아니라, 실제 런타임에 사용할 Wheel Class 참조 자산도 함께 가진다.**

### 5. Drive 상태 판정 기준값을 정의한다
`DriveStateConfig`는 현재 차량별 Drive 상태 판정 규칙을 가진다.

현재 포함 항목:
- `ActiveInputThreshold`
- `AirborneMinSpeedThresholdKmh`
- `AirborneStateMinimumHoldTimeSeconds`
- `AirborneVerticalSpeedThresholdCmPerSec`
- `DriveStateMinimumHoldTimeSeconds`
- `IdleEnterSpeedThresholdKmh`
- `IdleExitSpeedThresholdKmh`
- `IdleStateMinimumHoldTimeSeconds`
- `ReverseEnterSpeedThresholdKmh`
- `ReverseExitSpeedThresholdKmh`
- `ReversingStateMinimumHoldTimeSeconds`
- `bEnableDriveStateHysteresis`
- `bTreatOppositeThrottleAsBrake`
- `bUseDriveStateOverrides`
- `bUsePerStateHoldTimes`

현재 의미:
- 언제 Idle로 볼지
- 언제 Reversing으로 볼지
- 언제 Airborne으로 볼지
- 상태 전이에 히스테리시스를 사용할지
- 반대 방향 throttle을 brake처럼 처리할지

를 차량별로 조정할 수 있다.

즉 현재 `VehicleData`는 **Drive 상태 머신의 민감도와 판정 규칙까지 정의하는 주행 상태 정책 데이터 루트**이기도 하다.

## 현재 기준 기능의 성격 정리
현재 구현을 종합하면 `VehicleData`는 아래 역할을 가진다.

1. **차량 자산 루트 데이터**
   - 차체/휠 시각 자산을 한 곳에서 관리함

2. **주행 특성 정의 데이터**
   - 엔진, 디퍼렌셜, 조향, 중심질량, 휠 물리 특성을 한 곳에서 관리함

3. **휠 시각/휠 클래스 정의 데이터**
   - WheelSync 기준값과 Front/Rear Wheel Class를 함께 관리함

4. **Drive 상태 정책 데이터**
   - 상태 판정 임계값과 히스테리시스 정책을 차량 단위로 관리함

5. **VehicleRuntime 입력 데이터**
   - 저장용 데이터가 아니라, 실제 런타임 초기화 함수들이 직접 소비하는 데이터 세트임

따라서 현재 이 기능은 단순한 데이터 에셋이라기보다,
**차량 하나의 런타임 구성 전체를 묶어서 공급하는 루트 차량 구성 데이터 기능**이라고 보는 것이 맞다.

## 현재 동작 방식
현재 `VehicleData`는 아래 방식으로 동작한다.

### 1. Pawn이 VehicleData를 참조한다
현재 `BP_CFVehiclePawn`의 `VehicleData` 기본값은 아래 자산으로 확인된다.

- `/Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar.DA_PoliceCar`

즉 현재 기본 차량 Pawn은 **`DA_PoliceCar`를 자신의 기본 차량 데이터 세트로 사용**한다.

### 2. VehicleRuntime이 VehicleData를 풀어 적용한다
현재 `ACFVehiclePawn::ApplyVehicleDataConfig()`는 `VehicleData`를 직접 읽어서,
런타임 대상에 아래 순서로 분해 적용한다.

1. VehicleMovementComponent 설정 적용
2. Front/Rear WheelClass 참조 반영
3. WheelSetups 물리/오프셋 반영
4. WheelSyncComp와 Wheel_Mesh_* 시각 구성 반영
5. VehicleDriveComp에 DriveStateConfig 반영

즉 현재 `VehicleData`는 데이터 테이블처럼 조회만 되는 것이 아니라,
**런타임 함수에 의해 실제 차량 구성으로 해체되어 적용되는 실행 입력 데이터**다.

### 3. PostLoad에서 레거시 값 보정을 수행한다
`UCFVehicleData`는 현재 `PostLoad()`를 오버라이드한다.

헤더 설명 기준 현재 의미:
- 레거시 VehicleMovement 실험값 세트를 현재 프로젝트 기준값으로 보정한다.

즉 현재 `VehicleData`는 단순 정적 저장 구조가 아니라,
**로딩 시점에 구버전 값 보정 책임까지 일부 갖는 데이터 자산**이다.

## 현재 자산 / 클래스 역할
### `UCFVehicleData`
- 종류: `PrimaryDataAsset`
- 현재 역할: 차량 루트 데이터 자산 클래스
- 현재 기능: 시각/주행/휠/참조/DriveState 설정을 한 곳에 묶는다.

### `DA_PoliceCar`
- 종류: `CFVehicleData` 인스턴스
- 현재 역할: 현재 기본 차량용 실제 데이터 세트
- 현재 특징:
  - 차체 메쉬: `Combined_Body`
  - 휠 메쉬: `Wheel_FL/FR/RL/RR`
  - 앞 WheelClass: `BP_Wheel_Front_C`
  - 뒤 WheelClass: `BP_Wheel_Rear_C`
  - `bUseMovementOverrides = true`
  - `bUseWheelVisualOverrides = true`
  - `bUseDriveStateOverrides = true`

### `BP_CFVehiclePawn`
- 종류: `Blueprint`
- 현재 역할: VehicleData 소비 주체
- 현재 특징: 기본 `VehicleData`로 `DA_PoliceCar`를 직접 참조한다.

### `ACFVehiclePawn`
- 종류: `C++ Pawn`
- 현재 역할: VehicleData를 실제 런타임에 적용하는 중심 실행 주체

## 현재 대표 데이터 세트: `DA_PoliceCar`
현재 프로젝트에서 확인된 대표 VehicleData는 `DA_PoliceCar`다.

현재 확인된 주요 값은 아래와 같다.

### 시각 자산
- `ChassisMesh` → `Combined_Body`
- `WheelMeshFL` → `Wheel_FL`
- `WheelMeshFR` → `Wheel_FR`
- `WheelMeshRL` → `Wheel_RL`
- `WheelMeshRR` → `Wheel_RR`

즉 현재 기본 차량 시각 데이터는 **경찰차용 결합 차체 메쉬와 4개 개별 휠 메쉬**를 사용한다.

### Movement 기본값
현재 확인된 대표값:
- `bUseMovementOverrides = true`
- `DifferentialType = RearWheelDrive`
- `EngineMaxTorque = 750`
- `EngineMaxRPM = 7000`
- `EngineIdleRPM = 900`
- `FrontWheelMaxSteerAngle = 35`
- `FrontWheelRadius = 40`
- `RearWheelRadius = 40`
- `SteeringType = AngleRatio`
- `SteeringAngleRatio = 0.7`
- `bLegacyWheelFrictionPosition = true`

즉 현재 기본 차량은 **후륜구동 기반, 750 토크 / 7000 RPM / 전륜 35도 조향** 성격의 기본 세트를 사용한다.

### WheelVisual 기준값
- `bUseWheelVisualOverrides = true`
- `ExpectedWheelCount = 4`
- `FrontWheelCountForSteering = 2`

즉 현재 기본 차량은 **4륜 차량, 전륜 2개 조향** 기준으로 WheelSync를 구성한다.

### Reference 자산
- `FrontWheelClass` → `BP_Wheel_Front_C`
- `RearWheelClass` → `BP_Wheel_Rear_C`

즉 현재 기본 차량은 **앞/뒤 Wheel Class를 분리**해서 사용한다.

### DriveState 기준값
현재 확인된 대표값:
- `ActiveInputThreshold = 0.05`
- `IdleEnterSpeedThresholdKmh = 0.75`
- `IdleExitSpeedThresholdKmh = 1.5`
- `ReverseEnterSpeedThresholdKmh = 1.25`
- `ReverseExitSpeedThresholdKmh = 0.75`
- `AirborneMinSpeedThresholdKmh = 3`
- `AirborneVerticalSpeedThresholdCmPerSec = 100`
- `bEnableDriveStateHysteresis = true`
- `bTreatOppositeThrottleAsBrake = true`
- `bUseDriveStateOverrides = true`
- `bUsePerStateHoldTimes = true`

즉 현재 기본 차량 데이터는 **Drive 상태 전이에 히스테리시스와 상태별 홀드 타임을 적극 사용하는 설정**을 가진다.

## 현재 생성 및 연결 구조
현재 `VehicleData` 연결 구조는 아래와 같다.

1. `BP_CFVehiclePawn`이 `VehicleData` 자산 참조를 가짐
2. 현재 기본값은 `DA_PoliceCar`
3. `BeginPlay` 이후 `InitializeVehicleRuntime()` 진입
4. `ApplyVehicleDataConfig()`가 `VehicleData`를 읽음
5. Movement / Reference / WheelPhysics / WheelVisual / DriveState 설정으로 분해 적용
6. 이후 Drive 준비 / WheelSync 준비 검증 수행

현재 구조 해석:
- VehicleData는 전역 데이터베이스가 아니라 Pawn 종속 참조 자산이다.
- 현재 기본 차량은 `DA_PoliceCar` 한 세트를 중심으로 운영된다.
- 실제 적용은 VehicleRuntime 파이프라인에서 이뤄진다.

## 현재 기능 책임
현재 구현 기준에서 `VehicleData`의 책임은 아래와 같다.

- 차량 시각 자산 참조를 제공한다.
- 차량 Movement 기본값을 제공한다.
- WheelClass 및 WheelSetup 관련 참조/보정값을 제공한다.
- WheelSync 시각 구성 기준값을 제공한다.
- Drive 상태 판정 기준값을 제공한다.
- VehicleRuntime이 실제 차량 구성을 적용할 수 있도록 루트 데이터 세트를 제공한다.
- 일부 레거시 값 보정을 위해 PostLoad 보정 지점을 가진다.

## 현재 기준 비책임 항목
현재 구현상 `VehicleData`의 직접 책임으로 보지 않는 항목은 아래와 같다.

- 실제 주행 상태 계산 자체
  - 계산은 `VehicleDriveComp` 책임
- 실제 휠 시각 동기화 계산 자체
  - 계산은 `WheelSyncComp` 책임
- 데이터 적용 실행 자체
  - 적용 실행은 `ACFVehiclePawn` / VehicleRuntime 책임
- 입력 처리 자체
  - `Input` 기능 책임
- 디버그 UI 표시 자체
  - `VehicleDebug` 기능 책임

즉 현재 `VehicleData`는 계산기나 실행기라기보다,
**차량 런타임 구성을 공급하는 데이터 루트**다.

## 현재 문서 기준의 핵심 결론
현재 `VehicleData` 기능은,

**차량 하나의 외형, 주행 성격, 휠 물리, 휠 시각 구성, Wheel Class 참조, Drive 상태 판정 기준을 하나의 루트 DataAsset으로 묶고, VehicleRuntime이 이를 실제 차량 구성에 반영할 수 있게 공급하는 현재 차량 구성 데이터 기능**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `VehicleData`는 현재 차량이 어떤 외형과 어떤 주행 특성, 어떤 Wheel/Drive 정책으로 동작할지를 한 자산으로 정의해서 VehicleRuntime에 공급하는 현재 상태 기능이다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- `PostLoad()`의 실제 레거시 보정 로직 상세 구현
- `PDA_VehicleArchetype`가 현재 `UCFVehicleData`와 어떤 관계를 갖는지
- `S_WheelConfig` 자산이 현재 런타임 경로에서 직접 쓰이는지 여부
- `DA_PoliceCar` 외 추가 차량 데이터 세트가 실제 게임플레이 경로에 연결돼 있는지 여부

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `CFVehicleData.h` 구조 변경
- `VehicleData` 하위 config 항목 변경
- `BP_CFVehiclePawn`의 기본 `VehicleData` 참조 변경
- `ApplyVehicleDataConfig()` 소비 방식 변경
- `DA_PoliceCar` 주요 값 변경
- `PostLoad()` 보정 정책 변경

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
- `VehicleData` 문서 최초 작성
- `UCFVehicleData` 구조와 `DA_PoliceCar` 실값 기준으로 현재 기능 정리
- 단순 데이터 필드 목록이 아니라, VehicleRuntime이 실제로 어떻게 소비하는지 중심으로 본문 작성
- 현재 기본 차량 데이터 세트가 `DA_PoliceCar`임을 문서화

## 마지막 확인 기준
- 확인 일시: 2026-04-22
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFVehicleData.h`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `/Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar`
  - `/Game/CarFight/Vehicles/BP_CFVehiclePawn`
