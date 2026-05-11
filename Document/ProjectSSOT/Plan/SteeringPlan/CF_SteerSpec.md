# CF_SteerSpec

## 문서 목적
본 문서는 CarFight 차량 조향 입력 변경안의 **SSOT 명세서 v0.1**이다.

이번 변경의 핵심은 기존 게임패드 2D 입력에서 조향과 가감속이 같은 입력값에 묶여 있던 문제를 분리하는 것이다.

---

## 1. 현재 문제
현재 게임패드 신규 조작에서는 조향값이 스틱의 X값에 직접 비례한다.

예시:

- 스틱을 살짝 오른쪽으로 기울임: 바퀴도 조금만 꺾임
- 스틱을 오른쪽 끝까지 기울임: 바퀴가 끝까지 꺾임

이 구조는 CarFight의 목표 조작감과 맞지 않는다.

---

## 2. 목표 조작 철학
이번 조향 변경의 목표는 아래와 같다.

- 스틱의 기울기 정도는 가속 / 브레이크 강도에 사용한다.
- 스틱이 기울어진 방향은 조향 목표에 사용한다.
- 스틱을 덜 기울였다고 바퀴도 덜 꺾이는 구조는 사용하지 않는다.
- 실제 자동차처럼 핸들을 끝까지 꺾은 상태에서 가속은 살살 하는 상황이 가능해야 한다.

즉, 스틱 입력은 아래처럼 분해해서 해석한다.

```text
Stick Magnitude  -> Throttle / Brake Strength
Stick Direction  -> Target Steering
```

---

## 3. 조향 목표값 규칙

### 3-1. 기존 방식
기존 방식은 아래와 같다.

```text
TargetSteering = RawStickX
```

이 방식은 스틱 입력 크기가 작으면 조향도 작아진다.

### 3-2. 변경 방식
변경 방식은 아래와 같다.

```text
NormalizedStickDirection = RawStickInput / StickMagnitude
TargetSteering = NormalizedStickDirection.X
```

즉, 스틱의 크기가 아니라 **방향**을 사용해 조향 목표를 만든다.

---

## 4. 조향 입력 최소값
정규화 방식은 작은 입력 노이즈를 크게 증폭할 수 있다.

따라서 조향 방향 계산에는 별도 최소 입력값을 둔다.

기본값:

```text
SteeringDirectionMinMagnitude = 0.10
```

규칙:

- `StickMagnitude < SteeringDirectionMinMagnitude`이면 조향 의도 없음
- 조향 의도 없음이면 `TargetSteering = 0.0`
- `StickMagnitude >= SteeringDirectionMinMagnitude`이면 정규화 방향으로 조향 목표 계산

---

## 5. 실제 조향값 규칙
조향 목표값은 즉시 차량에 적용하지 않는다.

아래 두 값을 분리한다.

```text
TargetSteeringInput
CurrentSteeringInput
```

- `TargetSteeringInput`: 스틱 방향에서 계산한 목표 조향값
- `CurrentSteeringInput`: 실제 Chaos Vehicle에 전달하는 조향값

`CurrentSteeringInput`은 매 프레임 `TargetSteeringInput`을 제한 속도로 따라간다.

---

## 6. 조향 제한 속도
조향 제한 속도는 실제 운전자가 핸들을 한쪽 끝에서 반대쪽 끝까지 돌릴 수 있는 최대 속도에 해당한다.

기본 튜닝값:

```text
SteeringLockToLockTimeSec = 0.45
```

조향값 범위는 `-1.0 ~ +1.0`이므로 왼쪽 끝에서 오른쪽 끝까지의 거리는 `2.0`이다.

계산:

```text
SteeringTurnRate = 2.0 / SteeringLockToLockTimeSec
```

---

## 7. 스틱 중립 시 중립 복귀
스틱이 중립이면 바퀴가 즉시 중립으로 돌아가지 않는다.

대신 실제 자동차처럼 제한된 속도로 중립을 향해 복귀한다.

중립 복귀 목표:

```text
TargetSteeringInput = 0.0
```

실제 적용값:

```text
CurrentSteeringInput -> 0.0
```

---

## 8. 차량 속도 기반 중립 복귀
중립 복귀 속도는 차량 속도에 비례한다.

기본 튜닝값:

```text
SteeringReturnMinSpeedKmh = 0.0
SteeringReturnMaxSpeedKmh = 80.0
SteeringReturnMinRate = 0.4
SteeringReturnMaxRate = 6.0
```

계산 개념:

```text
SpeedAlpha = Clamp((SpeedKmh - MinSpeedKmh) / (MaxSpeedKmh - MinSpeedKmh), 0.0, 1.0)
ReturnRate = Lerp(SteeringReturnMinRate, SteeringReturnMaxRate, SpeedAlpha)
```

의미:

- 정지 / 극저속: 매우 느리게 중립 복귀
- 중속: 자연스럽게 중립 복귀
- 고속: 빠르게 중립 복귀

---

## 9. 고속 급조향에 대한 정책
고속에서 플레이어가 급조향하면 차량이 불안정해질 수 있다.

이 현상은 실제 차량에서도 발생 가능한 자연스러운 리스크로 본다.
따라서 이번 1차 설계에서는 고속 조향 입력을 별도로 약화하지 않는다.

단, 아래는 막아야 한다.

- 입력 노이즈 때문에 조향이 튀는 현상
- 실제 조향값이 목표값으로 순간이동하는 현상
- release / ownership 전환 과정에서 조향값이 부정확하게 덮어써지는 현상

---

## 10. 적용 범위
이번 변경은 기본적으로 **게임패드 신규 조작(`IA_VehicleMove`)**에 적용한다.

키보드 기존 조작(`IA_Throttle / IA_Brake / IA_Steering`)은 별도 정책을 유지한다.
단, 향후 키보드 조향에도 같은 조향 속도 제한을 적용할지는 별도 검토한다.

---

## 11. 변경 이력
- v0.1 (2026-05-11)
  - 조향과 가감속 분리 규칙 정의.
  - 방향 기반 목표 조향과 속도 제한 기반 실제 조향 규칙 정의.
  - 차량 속도 기반 중립 복귀 규칙 정의.
