# CF_SteerDesign

## 문서 목적
본 문서는 `CF_SteerSpec.md`를 기준으로 CarFight 차량 조향 변경안을 C++ 구조에 어떻게 반영할지 정리한 설계서다.

현재 기준 적용 대상은 `ACFVehiclePawn`의 게임패드 신규 조작 경로다.

---

## 1. 현재 코드 기준
현재 `ResolveVehicleMoveInput()`에서 조향값은 아래와 유사하게 계산된다.

```cpp
ResolvedMoveInput.SteeringValue = FMath::Clamp(MoveInputVector.X, -1.0f, 1.0f);
```

이 구조는 스틱의 좌우 성분 크기가 곧 바퀴 조향량이 된다.
따라서 스틱을 조금만 움직이면 바퀴도 조금만 꺾인다.

이번 변경은 이 구조를 아래처럼 바꾸는 것이다.

```text
Raw Stick Magnitude -> Throttle / Brake
Raw Stick Direction -> Target Steering
Current Steering    -> Target Steering을 제한 속도로 추적
```

---

## 2. 설계 원칙

### 2-1. 가감속과 조향 분리
- `Magnitude`는 기존처럼 가속 / 브레이크 강도로 사용한다.
- 조향은 `MoveInputVector.X` 원본값이 아니라 정규화된 방향의 X값을 사용한다.

### 2-2. 목표 조향과 실제 조향 분리
- `TargetSteeringInput`은 입력 방향으로부터 계산한다.
- `CurrentSteeringInput`은 실제 차량에 전달하는 값이다.
- `CurrentSteeringInput`은 매 프레임 제한 속도로 `TargetSteeringInput`을 따라간다.

### 2-3. 중립 복귀는 속도 기반
- 스틱 중립 또는 조향 의도 없음이면 목표 조향은 `0.0`이다.
- 이때 복귀 속도는 차량 속도에 비례한다.

### 2-4. 고속 급조향 보정은 하지 않음
- 고속에서 급격한 조향 입력으로 차량이 불안정해지는 것은 자연스러운 리스크로 둔다.
- 다만 입력값이 물리적으로 순간이동하지 않도록 조향 변화 속도 제한은 적용한다.

---

## 3. 권장 데이터 구조

### 3-1. 설정값
`ACFVehiclePawn` 또는 차량 입력 설정 구조체에 아래 값을 추가하는 것을 권장한다.

```cpp
/** 조향 방향 계산을 허용할 최소 스틱 입력 크기입니다. */
float SteeringDirectionMinMagnitude = 0.10f;

/** 왼쪽 최대 조향에서 오른쪽 최대 조향까지 이동하는 데 걸리는 시간입니다. */
float SteeringLockToLockTimeSec = 0.45f;

/** 중립 복귀 속도 보간 시작 차량 속도(km/h)입니다. */
float SteeringReturnMinSpeedKmh = 3.0f;

/** 중립 복귀 속도 보간 완료 차량 속도(km/h)입니다. */
float SteeringReturnMaxSpeedKmh = 80.0f;

/** 복귀 시작 속도 직후의 중립 복귀 속도입니다. 기본값 0은 정지/극저속에서 자동 복귀하지 않는다는 뜻입니다. */
float SteeringReturnMinRate = 0.0f;


/** 고속에서의 중립 복귀 속도입니다. */
float SteeringReturnMaxRate = 6.0f;
```

블루프린트 노출 시에는 `EditAnywhere`, `BlueprintReadWrite`, `ToolTip`, `ClampMin`을 붙이는 것을 권장한다.

### 3-2. 런타임 상태값
`ACFVehiclePawn`에 아래 상태값을 추가하는 것을 권장한다.

```cpp
/** 실제 차량에 적용 중인 조향 입력값입니다. */
float CurrentSteeringInput = 0.0f;

/** 이번 프레임에 입력 방향에서 계산된 목표 조향값입니다. */
float TargetSteeringInput = 0.0f;
```

디버그용으로 아래도 있으면 좋다.

```cpp
float LastSteeringTurnRate = 0.0f;
float LastSteeringReturnRate = 0.0f;
bool bLastSteeringReturningToCenter = false;
```

---

## 4. 목표 조향 계산

### 4-1. 입력 방향 계산
```cpp
FVector2D SteeringDirection = FVector2D::ZeroVector;
if (MoveInputMagnitude >= SteeringDirectionMinMagnitude)
{
    SteeringDirection = MoveInputVector / MoveInputMagnitude;
}
```

### 4-2. 목표 조향 계산
```cpp
TargetSteeringInput = FMath::Clamp(SteeringDirection.X, -1.0f, 1.0f);
```

### 4-3. 조향 의도 없음
입력 크기가 최소값보다 작으면 아래처럼 처리한다.

```cpp
TargetSteeringInput = 0.0f;
```

---

## 5. 실제 조향값 갱신
조향 갱신은 `ApplyResolvedVehicleMoveInput()` 또는 별도 helper 함수에서 처리하는 것을 권장한다.

권장 함수명:

```cpp
float UpdateCurrentSteeringInput(float TargetSteeringInput, float DeltaSeconds);
```

다만 `ApplyResolvedVehicleMoveInput()` 현재 시그니처가 `DeltaSeconds`를 받지 않기 때문에, 아래 둘 중 하나를 선택해야 한다.

### A안. `HandleVehicleMoveInput()`에서 DeltaSeconds 계산 후 전달
- 장점: 명시적이다.
- 단점: 함수 시그니처 변경이 조금 늘어난다.

### B안. Pawn 내부에서 `GetWorld()->GetDeltaSeconds()` 사용
- 장점: 수정 범위가 작다.
- 단점: 테스트 시 명시성이 떨어진다.

1차 구현은 B안을 권장한다.

---

## 6. 조향 속도 계산

### 6-1. 입력이 있는 경우
입력 방향에 따른 목표 조향을 따라갈 때는 `SteeringLockToLockTimeSec`를 사용한다.

```cpp
const float SteeringTurnRate = 2.0f / FMath::Max(SteeringLockToLockTimeSec, 0.01f);
CurrentSteeringInput = FMath::FInterpConstantTo(CurrentSteeringInput, TargetSteeringInput, DeltaSeconds, SteeringTurnRate);
```

### 6-2. 입력이 없는 경우
중립 복귀 시에는 차량 속도 기반 복귀 속도를 사용한다.

```cpp
const float SpeedAlpha = FMath::Clamp(
    (ForwardSpeedKmh - SteeringReturnMinSpeedKmh) / FMath::Max(SteeringReturnMaxSpeedKmh - SteeringReturnMinSpeedKmh, 1.0f),
    0.0f,
    1.0f);

const float SteeringReturnRate = FMath::Lerp(SteeringReturnMinRate, SteeringReturnMaxRate, SpeedAlpha);
CurrentSteeringInput = FMath::FInterpConstantTo(CurrentSteeringInput, 0.0f, DeltaSeconds, SteeringReturnRate);
```

차량 속도는 전후 방향 속도 절댓값 또는 차량 전체 속도 km/h 중 하나를 사용할 수 있다.
1차 구현은 기존 디버그에 있는 `ForwardSpeedKmh` 또는 차량 속도 계산 helper를 재사용한다.

---

## 7. 적용 위치

### 7-1. `ResolveVehicleMoveInput()`
기존 `SteeringValue = MoveInputVector.X` 계산을 변경한다.

변경 후 `SteeringValue`는 목표 조향값으로 쓰거나, 별도 필드 `TargetSteeringValue`로 분리할 수 있다.

권장:

- `FCFVehicleMoveInputResult::SteeringValue`는 목표 조향값으로 유지
- 실제 적용값은 `CurrentSteeringInput` 상태값으로 관리

### 7-2. `ApplyResolvedVehicleMoveInput()`
기존에는 바로 아래처럼 적용했다.

```cpp
SetVehicleSteeringInput(ResolvedMoveInput.SteeringValue);
```

변경 후에는 아래 흐름으로 바꾼다.

```cpp
const float SmoothedSteeringInput = UpdateCurrentSteeringInput(ResolvedMoveInput.SteeringValue);
SetVehicleSteeringInput(SmoothedSteeringInput);
```

### 7-3. Release 처리
`HandleVehicleMoveReleased()`에서 조향을 즉시 0으로 리셋하지 않도록 주의한다.

스틱 release 시 목표는 0이 되어야 하지만, 실제 `CurrentSteeringInput`은 속도 기반 복귀로 천천히 0으로 이동해야 한다.

따라서 release 경로는 아래 중 하나로 정리한다.

- `CurrentSteeringInput`은 즉시 0으로 만들지 않는다.
- 목표 조향을 0으로 만들고, 이후 Tick 또는 입력 처리 경로에서 중립 복귀를 계속 수행한다.

이 지점은 구현 시 별도 확인이 필요하다.

---

## 8. Tick 필요 여부
스틱 입력이 들어오지 않는 동안에도 바퀴가 중립으로 돌아와야 한다.

따라서 `HandleVehicleMoveInput()`이 호출되는 프레임에만 조향을 갱신하면 부족할 수 있다.

권장안:

- `Tick()` 또는 차량 입력 갱신 루프에서 `CurrentSteeringInput` 중립 복귀를 계속 수행한다.
- 이미 Pawn이 Tick을 사용 중이면 그곳에 `UpdateSteeringInput()`을 넣는다.
- Tick이 없다면 이번 기능을 위해 Tick 활성화가 필요할 수 있다.

이 항목은 구현 전 반드시 현재 Pawn의 Tick 사용 여부를 확인한다.

---

## 9. 디버그 항목
아래 디버그 항목을 추가하는 것을 권장한다.

- `SteerTarget`
- `SteerCurrent`
- `SteerTurnRate`
- `SteerReturnRate`
- `SteerReturning`
- `SteerMinMagnitude`

---

## 10. 변경 이력
- v0.1 (2026-05-11)
  - 방향 기반 목표 조향 계산 설계.
  - 제한 속도 기반 실제 조향 적용 설계.
  - 차량 속도 기반 중립 복귀 설계.
