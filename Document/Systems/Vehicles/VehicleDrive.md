# VehicleDrive

## 문서 목적
이 문서는 현재 프로젝트에서 `VehicleDrive` 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 자산/클래스/설정 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

## 문서 범위
이 문서에서 말하는 `VehicleDrive` 기능은 아래 요소를 묶어서 본다.

- 핵심 클래스: `UCFVehicleDriveComp`
- 상태 설정 구조: `FCFVehicleDriveStateConfig`
- 관련 상태 구조:
  - `FCFVehicleInputState`
  - `FCFVehicleDriveStateSnapshot`
- 현재 대표 소비 주체: `ACFVehiclePawn`
- 현재 대표 설정 공급 주체: `UCFVehicleData::DriveStateConfig`
- 핵심 함수:
  - `CacheVehicleMovementComponent()`
  - `ApplyThrottleInput()`
  - `ApplySteeringInput()`
  - `ApplyBrakeInput()`
  - `ApplyHandbrakeInput()`
  - `BuildDriveStateSnapshot()`
  - `EvaluateDriveState()`
  - `ShouldEnterAirborneState()`
  - `CanApplyStateTransition()`
  - `UpdateDriveStateTransitionSummary()`
  - `ApplyDriveStateConfig()`

즉, 현재 기준 `VehicleDrive`는 **차량 입력을 Chaos Vehicle Movement에 전달하는 기능과, 현재 주행 상태를 계산하고 상태 전이를 관리하는 기능을 묶은 주행 상태 운영 시스템**으로 본다.

## 이 기능이 현재 실제로 하는 일
현재 구현 기준 `VehicleDrive`의 핵심 역할은 **차량 Pawn이 넘겨준 스로틀/조향/브레이크/핸드브레이크 입력을 실제 Chaos Vehicle Movement에 적용하고, 현재 속도/진행 방향/접지 상태/입력 상태를 바탕으로 DriveState를 계산한 뒤, 히스테리시스와 최소 유지 시간을 적용해서 최종 주행 상태를 안정적으로 관리하는 것**이다.

이 기능은 단순히 입력 전달만 하지 않는다.
현재 구현 기준으로 보면, 이 기능은 아래 다섯 가지 일을 동시에 한다.

### 1. 입력을 실제 VehicleMovement에 전달한다
현재 `UCFVehicleDriveComp`는 네 가지 입력 채널을 직접 다룬다.

- `ApplyThrottleInput(float)`
- `ApplySteeringInput(float)`
- `ApplyBrakeInput(float)`
- `ApplyHandbrakeInput(bool)`

현재 각 함수는 먼저 `CurrentInputState`를 갱신한 뒤,
캐시된 `UChaosWheeledVehicleMovementComponent`에 실제 입력을 전달한다.

현재 전달 방식:
- `Throttle` → `SetThrottleInput()`
- `Steering` → `SetSteeringInput()`
- `Brake` → `SetBrakeInput()`
- `Handbrake` → `SetHandbrakeInput()`

즉 현재 `VehicleDrive`는 **차량 입력의 최종 적용 지점**이다.
`Input` 기능이 값을 해석해도, 실제 VehicleMovement에 입력이 들어가는 마지막 단계는 현재 `VehicleDrive`다.

### 2. 마지막 입력 상태를 항상 보관한다
현재 `VehicleDrive`는 단순히 입력을 흘려보내지 않고,
`FCFVehicleInputState CurrentInputState`에 마지막 적용 입력을 보관한다.

현재 보관 항목:
- `ThrottleInput`
- `SteeringInput`
- `BrakeInput`
- `bHandbrakePressed`

현재 의미:
- DriveState 계산 시 어떤 입력이 들어왔는지 함께 본다.
- Debug/상태 스냅샷에서도 같은 입력 상태를 읽을 수 있다.

즉 현재 `VehicleDrive`는 **입력 적용기**이면서 동시에 **입력 상태 보관기**다.

### 3. 현재 주행 상태 스냅샷을 계산한다
현재 `BuildDriveStateSnapshot()`는 매 상태 갱신 시점에 `FCFVehicleDriveStateSnapshot`을 새로 만든다.

현재 스냅샷에 들어가는 주요 값:
- `CurrentDriveState`
- `CurrentSpeedKmh`
- `ForwardSpeedKmh`
- `bIsGrounded`
- `CurrentInputState`

현재 계산 방식:
- `OwnerActor->GetVelocity()`로 현재 속도 벡터 확보
- `GetActorForwardVector()`와 내적으로 전방 기준 속도 계산
- `ConvertCmPerSecToKmh()`로 km/h 변환
- `ShouldEnterAirborneState()` 결과를 뒤집어 접지 여부 판단

즉 현재 `VehicleDrive`는 단순 enum만 계산하는 게 아니라,
**현재 프레임의 주행 상태 판단 근거를 하나의 스냅샷 구조로 먼저 정리한 뒤 그 스냅샷으로 상태를 평가하는 구조**다.

### 4. 현재 DriveState를 판정한다
현재 `EvaluateDriveState()`는 스냅샷을 바탕으로 아래 상태 중 하나를 결정한다.

- `Idle`
- `Accelerating`
- `Coasting`
- `Braking`
- `Reversing`
- `Airborne`
- `Disabled`

현재 판정에 쓰는 핵심 값:
- 전체 속도 `CurrentSpeedKmh`
- 전방 기준 속도 `ForwardSpeedKmh`
- 접지 여부 `bIsGrounded`
- 스로틀 입력 존재 여부
- 브레이크/핸드브레이크 존재 여부
- 전진 의도 / 후진 의도
- 반대 스로틀을 브레이크로 볼지 여부

현재 판정 흐름을 요약하면 아래와 같다.

#### A. 공중 상태 우선 판정
- 접지 상태가 아니면 즉시 `Airborne`

#### B. 거의 정지 상태 판정
- 현재 속도가 Idle 임계값 이하이면
  - 후진 입력이면 `Reversing`
  - 전진 입력이면 `Accelerating`
  - 입력 없으면 `Idle`

#### C. 제동 상태 판정
- 브레이크 입력이 있거나
- 진행 방향 반대 스로틀이 브레이크 의도로 해석되면
  - `Braking`

#### D. 후진 이동 상태 판정
- 실제 후진 이동 중이면
  - 후진 입력이 있거나 입력이 없으면 `Reversing`
  - 반대 의도 입력이면 `Braking`

#### E. 전진 입력 상태 판정
- 전진 스로틀이면 `Accelerating`

#### F. 나머지 입력/무입력 판정
- 입력은 있는데 현재 상황상 제동 의도로만 읽히면 `Braking`
- 아무것도 아니면 `Coasting`

즉 현재 `VehicleDrive`는 단순 속도 기반 상태 머신이 아니라,
**속도 + 방향 + 접지 + 입력 의도까지 함께 해석하는 주행 상태 판정기**다.

### 5. 상태 전이를 그대로 적용하지 않고 안정화한다
현재 `VehicleDrive`의 가장 중요한 특징 중 하나는,
계산된 `CandidateDriveState`를 바로 쓰지 않는다는 점이다.

현재 `UpdateDriveStateTransitionSummary()`는 아래 흐름으로 동작한다.

1. 스냅샷에서 `CandidateDriveState`를 읽음
2. `CanApplyStateTransition()`로 현재 시점에 전이가 허용되는지 검사
3. 허용되면 `ApprovedDriveState = CandidateDriveState`
4. 허용되지 않으면 `ApprovedDriveState = CurrentDriveState` 유지
5. `PreviousDriveState`, `CurrentDriveState`, `bDriveStateChangedThisFrame` 갱신
6. 상태가 바뀌었으면 변경 시각/카운트 증가
7. `LastDriveStateTransitionSummary` 문자열 갱신

즉 현재 `VehicleDrive`는 **상태를 계산하는 기능**과 **상태 튐을 줄이기 위해 전이를 억제하는 기능**을 동시에 가진다.

## 현재 기준 기능의 성격 정리
현재 구현을 종합하면 `VehicleDrive`는 아래 역할을 가진다.

1. **입력 적용 기능**
   - 스로틀/조향/브레이크/핸드브레이크를 VehicleMovement에 전달함

2. **주행 상태 스냅샷 생성 기능**
   - 속도, 전방 속도, 접지, 입력 상태를 하나의 스냅샷으로 정리함

3. **DriveState 판정 기능**
   - Idle / Accelerating / Coasting / Braking / Reversing / Airborne / Disabled 상태를 계산함

4. **상태 전이 안정화 기능**
   - 히스테리시스와 최소 유지 시간을 적용해 상태 튐을 줄임

5. **주행 상태 진단 기능**
   - 이전 상태, 현재 상태, 변경 여부, 전이 요약 문자열을 유지함

따라서 현재 이 기능은 단순한 `입력 전달 컴포넌트`라기보다,
**차량 주행 입력을 적용하고 현재 DriveState를 계산·안정화·기록하는 주행 상태 운영 기능**이라고 보는 것이 맞다.

## 현재 동작 방식
현재 `VehicleDrive`는 아래 방식으로 동작한다.

### 1. BeginPlay 시 준비
현재 `BeginPlay()`에서는 아래를 수행한다.

- `CacheVehicleMovementComponent()`
- `UpdateDriveState()`

즉 시작 시점부터 현재 VehicleMovement를 찾아 캐시하고,
초기 DriveState 한 번을 계산한다.

### 2. Tick 기반 자동 상태 갱신
현재 `TickComponent()`는 `bEnableDriveStateUpdates`가 `true`면 매 프레임 `UpdateDriveState()`를 호출한다.

즉 현재 DriveState는 **입력 이벤트 시점에만 갱신되는 것이 아니라, Tick 기준으로 계속 재평가되는 구조**다.

### 3. 입력 적용 시 즉시 상태 갱신
현재 각 입력 함수도 실제 입력 적용 후 상태를 다시 갱신한다.

즉 현재 `VehicleDrive`는
- Tick으로도 갱신하고
- 입력 변경 시점에도 갱신한다.

그래서 현재 상태 반영이 비교적 즉각적이다.

## 현재 표시 조건 / 실행 조건
현재 `VehicleDrive`가 제대로 동작하려면 아래 조건이 중요하다.

- `OwnerActor`가 존재해야 함
- `UChaosWheeledVehicleMovementComponent`를 찾아 캐시할 수 있어야 함
- `bEnableDriveStateUpdates`가 켜져 있어야 DriveState 계산이 살아 있음
- `CurrentInputState`가 유효하게 갱신되어야 입력 의도 기반 상태 판정이 의미를 가짐

현재 `CacheVehicleMovementComponent()` 실패 시:
- `CachedVehicleMovementComponent = nullptr`
- `CurrentDriveState = Disabled`
- `false` 반환

즉 현재 구조에서 `VehicleDrive`는 VehicleMovement 캐시를 확보하지 못하면,
**주행 상태 기능을 `Disabled`로 떨어뜨리는 안전한 실패 경로**를 가진다.

## 현재 접지 / Airborne 판정 방식
현재 공중 상태 진입은 `ShouldEnterAirborneState()`가 담당한다.

현재 판정 방식:
- `bUseWheelContactSurfaceHint`가 켜져 있고
- 휠 중 하나라도 `GetPhysMaterial()`이 있으면
  - 접지 중으로 보고 `Airborne` 진입 안 함
- 그렇지 않으면
  - 수직 속도 `>` `AirborneVerticalSpeedThresholdCmPerSec`
  - 전체 속도 `>` `AirborneMinSpeedThresholdKmh`
  둘 다 만족할 때 `Airborne`

즉 현재 Airborne 판정은 **휠 접촉 힌트 우선, 실패 시 속도 기반 폴백** 구조다.

## 현재 히스테리시스 / 상태 유지 방식
현재 상태 전이 안정화는 아래 설정으로 동작한다.

- `bEnableDriveStateHysteresis`
- `bUsePerStateHoldTimes`
- `DriveStateMinimumHoldTimeSeconds`
- `IdleStateMinimumHoldTimeSeconds`
- `ReversingStateMinimumHoldTimeSeconds`
- `AirborneStateMinimumHoldTimeSeconds`

현재 동작:
- 히스테리시스가 꺼져 있으면 상태 전이는 즉시 허용
- 켜져 있으면 마지막 상태 변경 이후 충분한 시간이 지나야 다른 상태로 바뀜
- 상태별 유지 시간이 켜져 있으면
  - `Idle`, `Reversing`, `Airborne`은 각자 다른 최소 유지 시간 사용
  - 나머지는 기본 유지 시간 사용

즉 현재 `VehicleDrive`는 **상태 머신이 너무 민감하게 흔들리지 않도록 시간 기반 안정화 규칙**을 갖는다.

## 현재 차량별 튜닝 방식
현재 `ApplyDriveStateConfig()`는 `FCFVehicleDriveStateConfig`를 받아 DriveComp 내부 설정값에 복사한다.

단, 현재는 아래 조건이 붙는다.

- `InDriveStateConfig.bUseDriveStateOverrides == true`일 때만 적용

즉 현재 차량별 DriveState 튜닝은,
**VehicleData가 오버라이드 사용을 명시한 차량에만 반영되는 옵트인 구조**다.

현재 `DA_PoliceCar`에서 확인된 대표 DriveState 값은 아래와 같다.

- `bUseDriveStateOverrides = true`
- `bEnableDriveStateHysteresis = true`
- `bUsePerStateHoldTimes = true`
- `DriveStateMinimumHoldTimeSeconds = 0.12`
- `IdleStateMinimumHoldTimeSeconds = 0.10`
- `ReversingStateMinimumHoldTimeSeconds = 0.18`
- `AirborneStateMinimumHoldTimeSeconds = 0.08`
- `IdleEnterSpeedThresholdKmh = 0.75`
- `IdleExitSpeedThresholdKmh = 1.5`
- `ReverseEnterSpeedThresholdKmh = 1.25`
- `ReverseExitSpeedThresholdKmh = 0.75`
- `AirborneMinSpeedThresholdKmh = 3`
- `AirborneVerticalSpeedThresholdCmPerSec = 100`
- `ActiveInputThreshold = 0.05`
- `bTreatOppositeThrottleAsBrake = true`

즉 현재 기본 차량은 **상태 튐 방지와 반대 스로틀 브레이크 해석을 적극 사용하는 DriveState 정책**으로 운영된다.

## 현재 자산 / 클래스 역할
### `UCFVehicleDriveComp`
- 종류: `C++ Component`
- 현재 역할: 입력 적용 + DriveState 계산 + 전이 관리의 중심 컴포넌트

### `FCFVehicleInputState`
- 종류: `USTRUCT`
- 현재 역할: 마지막 적용 입력 스냅샷 보관

### `FCFVehicleDriveStateSnapshot`
- 종류: `USTRUCT`
- 현재 역할: 현재 상태 판정 근거를 구조화한 스냅샷

### `FCFVehicleDriveStateConfig`
- 종류: `USTRUCT`
- 현재 역할: 차량별 DriveState 판정/히스테리시스 튜닝값 공급

### `UChaosWheeledVehicleMovementComponent`
- 종류: `Chaos Vehicle Movement Component`
- 현재 역할: 실제 주행 입력의 최종 적용 대상

### `ACFVehiclePawn`
- 종류: `C++ Pawn`
- 현재 역할: Input 계층에서 해석한 입력을 VehicleDriveComp에 전달하는 상위 소비 주체

## 현재 생성 및 연결 구조
현재 `VehicleDrive` 연결 구조는 아래와 같다.

1. Pawn 생성 시 `VehicleDriveComp` 생성
2. BeginPlay에서 `CacheVehicleMovementComponent()` 시도
3. VehicleRuntime에서 필요 시 `ApplyDriveStateConfig()` 호출
4. Input 계층 또는 Pawn 함수가 `ApplyThrottle/Steering/Brake/HandbrakeInput()` 호출
5. DriveComp가 VehicleMovement에 입력 적용
6. DriveComp가 현재 스냅샷 생성
7. Candidate 상태 계산
8. 히스테리시스/유지 시간 검사 후 최종 상태 승인
9. 전이 요약 문자열과 카운트 갱신

현재 구조 해석:
- Drive 기능은 Pawn 종속이지만, 실제 주행 상태 계산 책임은 Component로 분리돼 있다.
- 입력 계산은 Pawn/Input 쪽에서 하고, 최종 적용과 상태 판정은 DriveComp가 맡는다.

## 현재 기능 책임
현재 구현 기준에서 `VehicleDrive`의 책임은 아래와 같다.

- 차량 입력을 VehicleMovement에 적용한다.
- 마지막 입력 상태를 보관한다.
- 현재 속도/전방 속도/접지 상태를 계산한다.
- 현재 DriveState를 판정한다.
- 차량별 DriveState 설정을 적용한다.
- 히스테리시스와 최소 유지 시간으로 상태 전이를 안정화한다.
- 이전 상태, 현재 상태, 상태 변경 여부, 전이 카운트, 전이 요약 문자열을 관리한다.

## 현재 기준 비책임 항목
현재 구현상 `VehicleDrive`의 직접 책임으로 보지 않는 항목은 아래와 같다.

- 입력 해석 자체
  - 2D VehicleMove 해석과 입력 소유권 관리는 `Input` / Pawn 계층 책임
- 차량 런타임 준비 자체
  - DriveComp 준비 성공 판정은 `VehicleRuntime` 책임
- WheelVisual 동기화 자체
  - `WheelSyncComp` 책임
- 차량 시각 자산 적용 자체
  - `VehicleData` + `VehicleRuntime` 책임
- 디버그 UI 출력 자체
  - `VehicleDebug` 기능 책임

즉 현재 `VehicleDrive`는 입력 계산기라기보다,
**입력 적용과 주행 상태 운영을 담당하는 런타임 주행 기능**에 가깝다.

## 현재 문서 기준의 핵심 결론
현재 `VehicleDrive` 기능은,

**차량 입력을 실제 Chaos Vehicle Movement에 적용하고, 현재 속도/방향/접지/입력 상태를 바탕으로 DriveState를 계산하며, 히스테리시스와 최소 유지 시간으로 상태 전이를 안정화하고 그 결과를 스냅샷과 전이 요약 문자열로 유지하는 현재 주행 상태 기능**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `VehicleDrive`는 현재 차량이 어떻게 움직이고 있는지를 입력과 속도 기반으로 해석해서, 실제 주행 입력 적용과 DriveState 운영을 동시에 수행하는 현재 상태 기능이다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- `ApplyMovementInputAndRefreshState()` 내부 헬퍼 구현의 전체 상세
- `UCarFightVehicleUtils::MakeDriveStateSnapshotDebugString()`가 만드는 문자열 포맷 전체
- 네트워크 환경에서 DriveState 동기화/복제 정책 존재 여부
- `Disabled` 상태가 런타임 중 어떤 운영 시나리오에서 의도적으로 사용되는지 상세

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `EvaluateDriveState()` 판정 규칙 변경
- `ShouldEnterAirborneState()` 판정 방식 변경
- `CanApplyStateTransition()` 전이 승인 규칙 변경
- `FCFVehicleDriveStateConfig` 구조 변경
- `ApplyDriveStateConfig()` 적용 항목 변경
- `DA_PoliceCar` 대표 DriveState 튜닝값 변경

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
- `VehicleDrive` 문서 최초 작성
- `UCFVehicleDriveComp`와 `FCFVehicleDriveStateConfig` 기준으로 현재 기능 정리
- 입력 적용, 상태 스냅샷 생성, DriveState 판정, 히스테리시스 기반 전이 제어 중심으로 본문 작성
- 현재 대표 차량 데이터 `DA_PoliceCar`의 DriveState 튜닝값 반영

## 마지막 확인 기준
- 확인 일시: 2026-04-23
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFVehicleDriveComp.h`
  - `UE/Source/CarFight_Re/Public/CFVehicleDriveStateConfig.h`
  - `UE/Source/CarFight_Re/Private/CFVehicleDriveComp.cpp`
  - `/Game/CarFight/Vehicles/Data/Cars/DA_PoliceCar`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
