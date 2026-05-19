# VehicleSteering

## 문서 목적
이 문서는 현재 프로젝트에서 `VehicleSteering` 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 클래스/입력/설정 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

## 문서 범위
이 문서에서 말하는 `VehicleSteering` 기능은 아래 요소를 묶어서 본다.

- 핵심 로직 주체: `ACFVehiclePawn`
- 입력 액션 경로: `IA_VehicleMove`
- Legacy 입력 경로: `IA_Steering`
- 실제 적용 함수:
  - `SetVehicleSteeringInput()`
  - `ApplyResolvedVehicleMoveInput()`
  - `UpdateVehicleMoveSteeringInput()`
  - `CalculateVehicleMoveTargetSteering()`
  - `CalculateSteeringReturnRateKmh()`
- 관련 입력 해석 함수:
  - `ResolveVehicleMoveInput()`
  - `HandleVehicleMoveInput()`
  - `HandleVehicleMoveReleased()`
- 관련 상태값:
  - `TargetSteeringInput`
  - `CurrentSteeringInput`
  - `LastSteeringTurnRate`
  - `LastSteeringReturnRate`
  - `bSteeringReturningToCenter`
- 관련 설정값:
  - `SteeringDirectionMinMagnitude`
  - `SteeringLockToLockTimeSec`
  - `SteeringReturnMinSpeedKmh`
  - `SteeringReturnMaxSpeedKmh`
  - `SteeringReturnMinRate`
  - `SteeringReturnMaxRate`

즉, 현재 기준 `VehicleSteering`은 **스틱 방향 기반 목표 조향 계산, 제한 속도 기반 실제 조향 추적, 속도 기반 중립 복귀를 묶은 차량 조향 운영 기능**으로 본다.

## 이 기능이 현재 실제로 하는 일
현재 구현 기준 `VehicleSteering`의 핵심 역할은 **게임패드 VehicleMove 2D 입력에서 스틱 크기와 조향량의 직접 비례 관계를 끊고, 스틱 방향을 목표 조향으로 해석한 뒤 실제 조향값을 제한 속도로 따라가게 하며, 스틱 중립 시 차량 속도에 비례해 바퀴를 중립 복귀시키는 것**이다.

현재 구현 기준으로 보면, 이 기능은 아래 정보를 처리한다.

### 1. 스틱 크기와 스틱 방향을 분리한다
현재 `VehicleSteering`은 `IA_VehicleMove`에서 들어오는 `FVector2D` 입력을 크기와 방향으로 분리해서 사용한다.

- 스틱 기울기 정도: 가속 / 브레이크 강도에 사용
- 스틱 기울어진 방향: 목표 조향값에 사용
- 스틱 X 원본값: 실제 조향량에 직접 비례시키지 않음

이 항목의 현재 의미:
- 스틱을 살짝만 기울여도 방향이 명확하면 목표 조향은 방향 기준으로 계산된다.
- 실제 자동차처럼 핸들을 많이 꺾은 상태에서 가속은 약하게 하는 조작이 가능하다.
- 조향은 입력 강도가 아니라 입력 방향의 문제로 취급한다.

### 2. 목표 조향과 실제 조향을 분리한다
현재 `VehicleSteering`은 목표 조향값과 실제 적용 조향값을 분리한다.

- `TargetSteeringInput`: 스틱 방향에서 계산한 목표 조향값
- `CurrentSteeringInput`: 실제 차량에 전달 중인 조향값

이 항목의 현재 의미:
- 목표 조향은 입력 방향에 따라 빠르게 바뀔 수 있다.
- 실제 조향값은 목표값으로 순간이동하지 않는다.
- 실제 조향값은 제한 속도로 목표값을 따라간다.

### 3. 조향 변화 속도를 제한한다
현재 `VehicleSteering`은 `SteeringLockToLockTimeSec`를 사용해 조향 변화 속도를 제한한다.

현재 기본값:
- `SteeringLockToLockTimeSec = 0.45`

계산 의미:
- 조향값 범위는 `-1.0 ~ +1.0`이다.
- 왼쪽 최대 조향에서 오른쪽 최대 조향까지의 거리는 `2.0`이다.
- 실제 조향 변화 속도는 `2.0 / SteeringLockToLockTimeSec` 기준으로 계산된다.

중요한 이유:
- 플레이어가 스틱을 빠르게 반대 방향으로 넘겨도 바퀴가 순간이동하지 않는다.
- 고속 급조향 자체는 허용하지만, 입력값이 물리적으로 말이 안 되게 튀는 것은 막는다.

### 4. 스틱 중립 시 차량 속도 기반으로 중립 복귀한다
현재 `VehicleSteering`은 스틱 중립 또는 조향 의도 없음 상태에서 `TargetSteeringInput = 0.0`으로 둔다.
다만 실제 `CurrentSteeringInput`은 즉시 0으로 가지 않고, 차량 속도 기반 복귀 속도로 0을 향해 이동한다.

현재 기본값:
- `SteeringReturnMinSpeedKmh = 3.0`
- `SteeringReturnMaxSpeedKmh = 80.0`
- `SteeringReturnMinRate = 0.0`
- `SteeringReturnMaxRate = 6.0`

현재 동작:
- `0 ~ 3km/h` 이하: 자동 중립 복귀 없음. 바퀴 방향 유지
- `3km/h` 초과: 차량 속도에 비례해 중립 복귀 시작
- `80km/h` 이상: 최대 중립 복귀 속도 사용

중요한 이유:
- 정지 상태에서 바퀴가 스스로 중앙으로 돌아오는 비현실적인 동작을 피한다.
- 차량이 움직일 때만 자연스럽게 바퀴가 정렬되는 느낌을 만든다.
- 저속과 고속의 조향 복귀 체감 차이를 만든다.

### 5. 최종 출력 / 적용 결과
현재 `VehicleSteering`은 최종적으로 아래 값을 차량에 적용한다.

- `CurrentSteeringInput`

적용 경로:
1. `HandleVehicleMoveInput()`에서 `IA_VehicleMove` 값을 받는다.
2. `ResolveVehicleMoveInput()`에서 입력 크기/각도/조향 목표를 해석한다.
3. `ApplyResolvedVehicleMoveInput()`에서 `TargetSteeringInput`을 갱신한다.
4. `Tick()`에서 `UpdateVehicleMoveSteeringInput()`을 호출한다.
5. `UpdateVehicleMoveSteeringInput()`이 `CurrentSteeringInput`을 갱신한다.
6. `SetVehicleSteeringInput(CurrentSteeringInput)`으로 실제 조향을 적용한다.

## 현재 기준 기능의 성격 정리
현재 구현을 종합하면 `VehicleSteering`은 아래 역할을 가진다.

1. **방향 기반 조향 해석 기능**
   - VehicleMove 2D 입력의 원본 X값을 조향량으로 직접 쓰지 않는다.
   - 입력 벡터의 정규화 방향을 목표 조향값으로 사용한다.

2. **조향 속도 제한 기능**
   - 목표 조향과 실제 조향을 분리한다.
   - 실제 조향값은 제한 속도로 목표 조향값을 따라간다.

3. **속도 기반 중립 복귀 기능**
   - 스틱 중립 시 실제 조향값은 속도 기반으로 0을 향해 이동한다.
   - 정지/극저속에서는 자동 복귀하지 않는다.

따라서 현재 이 기능은 단순히 `Steering Input`이라기보다,
**게임패드 VehicleMove 입력을 실제 차량 바퀴 조향에 맞게 해석하고 시간적으로 보정하는 차량 조향 운영 기능**이라고 보는 것이 맞다.

## 현재 동작 방식
현재 `VehicleSteering`은 입력 상태에 따라 아래처럼 동작한다.

- VehicleMove 조향 의도 있음: 목표 조향을 제한 속도로 추적
- VehicleMove 조향 의도 없음: 차량 속도 기반으로 중립 복귀
- 차량 속도 `0 ~ 3km/h` 이하: 자동 중립 복귀 없음
- 차량 속도 `3km/h` 초과: 중립 복귀 시작
- 차량 속도 `80km/h` 이상: 최대 복귀 속도
- LegacyAxis 조향: 기존 키보드 조작 경로 유지

즉 현재 `VehicleSteering`은 **게임패드 신규 조작에는 방향 기반 조향 보정을 적용하고, 키보드 Legacy 조작은 기존 경로를 유지하는 구조**다.

## 현재 표시 조건 / 실행 조건
현재 이 기능이 실제로 적용되는 조건은 아래와 같다.

- `ACFVehiclePawn` 런타임이 준비되어 있어야 한다.
- `IA_VehicleMove` 경로가 활성 입력으로 들어와야 한다.
- `CurrentInputOwnership`이 `VehicleMove2D` 계열 흐름을 허용해야 한다.
- `Tick()`이 실행되어야 실제 조향값이 계속 갱신된다.

조건 해석:
- 입력 이벤트가 없는 동안에도 `Tick()`에서 중립 복귀가 계속 처리된다.
- `HandleVehicleMoveReleased()`는 조향을 즉시 0으로 리셋하지 않고 `TargetSteeringInput = 0.0`으로 둔다.
- 실제 복귀는 `UpdateVehicleMoveSteeringInput()`에서 처리한다.

## 현재 자산 / 클래스 역할

### `ACFVehiclePawn`
- 종류: C++ Pawn
- 현재 역할: VehicleMove 조향 해석, 목표/현재 조향 상태 관리, Tick 기반 조향 갱신, 최종 조향 입력 전달

### `IA_VehicleMove`
- 종류: Enhanced Input Action
- 현재 역할: 게임패드 신규 조작용 2D 입력 제공

### `IA_Steering`
- 종류: Enhanced Input Action
- 현재 역할: 키보드 Legacy 조향 입력 제공

### `UCFVehicleDriveComp`
- 종류: C++ Component
- 현재 역할: Pawn에서 전달한 최종 조향 입력을 차량 Drive 입력 계층으로 전달하는 하위 컴포넌트

## 현재 생성 및 연결 구조
현재 연결 흐름은 아래와 같다.

1. `SetupPlayerInputComponent()`에서 `InputAction_VehicleMove`를 바인딩한다.
2. 게임패드 입력이 들어오면 `HandleVehicleMoveInput()`이 호출된다.
3. `ResolveVehicleMoveInput()`이 입력 크기, 각도, 조향 목표, 쓰로틀, 브레이크를 해석한다.
4. `ApplyResolvedVehicleMoveInput()`이 쓰로틀/브레이크와 목표 조향을 갱신한다.
5. `Tick()`에서 `UpdateVehicleMoveSteeringInput()`이 현재 실제 조향값을 갱신한다.
6. `SetVehicleSteeringInput()`이 최종 조향값을 차량 입력으로 전달한다.

현재 구조 해석:
- 이 기능은 특정 Pawn 종속 로컬 차량 조작 기능이다.
- 입력 해석과 실제 적용이 모두 `ACFVehiclePawn` 안에서 이루어진다.
- 조향 보정값은 현재 차량 Pawn의 프로퍼티로 튜닝한다.

## 현재 기능 책임
현재 구현 기준에서 이 기능의 책임은 아래다.

- VehicleMove 2D 입력에서 조향 목표 계산
- 스틱 크기와 조향량의 직접 비례 관계 제거
- 실제 조향값 제한 속도 적용
- 스틱 중립 시 속도 기반 중립 복귀
- 정지/극저속 상태에서 자동 중립 복귀 방지
- VehicleMove release 시 조향 즉시 0 리셋 방지

## 현재 기준 비책임 항목
현재 구현상 이 기능의 직접 책임이 아닌 것은 아래다.

- 키보드 Legacy 조향 체감 보간
- 카메라 조작
- 차량 엔진/기어/브레이크 물리 자체
- 휠 물리 MaxSteerAngle 설정 자체
- 고속 급조향으로 인한 차량 불안정성 보정
- 네트워크 동기화 설계
- AI 주행 조향 로직

중요:
- 현재 이 기능은 물리 차량 모델 전체가 아니라 **플레이어 입력에서 실제 조향 입력값으로 변환되는 중간 조향 운영 계층**이다.

## 현재 문서 기준의 핵심 결론
현재 `VehicleSteering` 기능은,

**게임패드 VehicleMove 2D 입력의 방향을 목표 조향으로 해석하고, 실제 조향값을 제한 속도와 차량 속도 기반 중립 복귀 규칙으로 보정해 차량에 적용하는 현재 상태 기능**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `VehicleSteering`은 현재 스틱 크기는 가감속에, 스틱 방향은 조향에 사용하고, 실제 바퀴 조향은 제한 속도와 차량 속도 기반 복귀 규칙을 거쳐 적용하게 해주는 차량 조향 운영 기능이다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- 키보드 Legacy 조향에도 동일한 제한 속도/중립 복귀를 적용할지 여부
- 차량별 DataAsset 또는 Config로 조향 튜닝값을 분리할지 여부
- 속도 기반 중립 복귀에 전후 방향 속도 대신 전체 속도를 계속 사용할지 장기 정책
- 드리프트/미끄러짐 상태에서 중립 복귀를 별도 보정할지 여부

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `ResolveVehicleMoveInput()` 조향 계산 규칙 변경
- `UpdateVehicleMoveSteeringInput()` 조향 보간 규칙 변경
- `CalculateSteeringReturnRateKmh()` 속도 기반 복귀 계산 변경
- 조향 튜닝 기본값 변경
- 키보드 Legacy 조향까지 기능 범위가 확장될 때
- 차량별 DataAsset/Config로 조향 튜닝값이 이동할 때

## 문서 버전 관리
- 현재 문서 버전: `1.0.0`
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
### v1.0.0 - 2026-05-11
- `VehicleSteering` 문서 최초 작성
- 게임패드 VehicleMove 2D 조향 해석 현재 구현 상태 문서화
- 방향 기반 목표 조향, 제한 속도 추적, 속도 기반 중립 복귀 규칙 문서화
- 빌드 성공 및 PIE 실기 검증 완료 상태 반영

## 마지막 확인 기준
- 확인 일시: `2026-05-11`
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `Document/ProjectSSOT/Plan/SteeringPlan/CF_SteerSpec.md`
  - `Document/ProjectSSOT/Plan/SteeringPlan/CF_SteerTest.md`
  - `CarFight_ReEditor / Win64 / Development` 빌드 성공: `build_bc511ba17bd92716`
  - PIE 실기 동작 확인 완료
