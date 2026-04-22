# CF_VehicleMoveCpp

## 문서 목적
본 문서는 `CF_VehicleMoveSpec.md`, `CF_VehicleMoveDesign.md`, `CF_VehicleMoveWork.md` 기준으로 정리한 **C++ 수정안 및 실제 반영 결과 요약 v0.2**이다.

이 문서는 아래를 기록한다.

- 수정 대상 파일
- 실제 반영된 핵심 로직
- A안 검증 결과
- 유지된 구조와 제거된 방식
- 후속 정리 포인트

---

## 1. 변경 목표
이번 변경의 핵심 목표는 아래 한 줄이다.

**기존 `Throttle / Steering / Brake` 전달 경로는 유지하고, `ACFVehiclePawn`에 차량 이동용 `Axis2D` 입력 해석 계층을 추가한다. 단, 후진 전환의 최종 결정권은 Chaos Vehicle `bUseAutoReverse`에 둔다.**

---

## 2. 실제 수정 대상 파일

### 직접 수정
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

### 검토
- `UE/Source/CarFight_Re/Public/CFVehicleDriveComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleDriveComp.cpp`

---

## 3. 실제 반영된 핵심 변경

### 3-1. Pawn 입력 해석 계층 추가
`ACFVehiclePawn`에 아래 요소를 추가했다.

- `InputAction_VehicleMove`
- 차량 이동 입력 설정 구조
- 차량 이동 입력 결과 구조
- 최근 방향 의도 상태
- 2D 입력 해석 헬퍼 함수
- 디버그 출력 확장

### 3-2. 2D 입력 해석 구현
`CFVehiclePawn.cpp`에서 아래를 구현했다.

- `Magnitude` 계산
- `AngleDeg` 계산
- 쓰로틀 / 후진 / 검은 영역 판정
- 검은 영역 `HoldDirection_UseCurrentMagnitude`
- `Steering = InputX`

### 3-3. 후진 처리 방식 최종 정리
최종 검증 결과, 현재 프로젝트에서는 아래 방식이 유효했다.

- 전진 중 후진 영역 입력 = 브레이크
- 후진 의도는 입력 해석 계층에서 유지
- 실제 reverse 전환은 Chaos Vehicle `bUseAutoReverse`가 담당

즉, **후진은 브레이크 기반 전환**으로 정리했다.

### 3-4. P1 입력 충돌 방지(Runtime Input Ownership)
후속 안정화 단계에서 아래 로직을 추가했다.

- `VehicleMove2D / LegacyAxis / None` 입력 소유권 enum 추가
- `CurrentInputOwnership` 상태값 추가
- `InputOwnershipHoldTimeSec` 유지 시간 추가
- `LastVehicleMoveInputTimeSec / LastLegacyAxisInputTimeSec` 추적값 추가
- `VehicleMove` 입력 소유권 획득 로직 추가
- 기존 축 입력 게이트 추가
- release 경로 덮어쓰기 방지 로직 추가
- `InputOwner` 디버그 추가

즉, 이제는 `IA_VehicleMove`와 기존 축 입력이 동시에 살아 있어도 **한 프레임에 하나의 입력 경로만 차량 입력을 지배**한다.

---

## 4. 제거한 방식

아래 방식은 실제 테스트 후 제거했다.

### 제거 1. 음수 스로틀 기반 reverse 강제
- `ThrottleValue = -Magnitude`
- 이유: 현재 프로젝트의 Chaos Vehicle 설정과 충돌 가능성이 높았음

### 제거 2. 수동 기어 강제
- `SetTargetGear(-1, true)`
- `SetTargetGear(1, true)`
- 이유: `bUseAutoReverse`와 충돌하며, 실제 후진 동작을 오히려 불안정하게 만들었음

---

## 5. 최종 적용 흐름
현재 기준 최종 흐름은 아래다.

1. `IA_VehicleMove`에서 2D 입력 수신
2. `ACFVehiclePawn`이 각도/영역/강도 해석
3. 전진 영역이면 `Throttle`
4. 후진 영역이면 `Brake`
5. 검은 영역이면 최근 방향 의도 기준 유지
6. `SetVehicleThrottleInput / SetVehicleBrakeInput / SetVehicleSteeringInput`로 전달
7. 후진 전환은 Chaos Vehicle `bUseAutoReverse`가 수행

---

## 6. A안 검증 결과
실제 PIE 기준으로 아래를 확인했다.

- 앞/좌/우 입력 동작 확인
- 전진 중 뒤 입력 시 브레이크 동작 확인
- `bUseAutoReverse = true` 환경에서 후진 전환 확인
- 수동 기어 강제 없이 후진 확인

즉 현재 구현 기준 결론은 아래다.

**입력 해석은 CarFight, 후진 전환은 Chaos Vehicle**

---

## 7. 디버그 확인 포인트
현재 구조에서는 아래 값이 중요하다.

- `MoveZone`
- `MoveIntent`
- `Throttle`
- `Brake`
- `ForwardSpeed`
- `DriveState`

특히 후진 여부는 `Throttle < 0`보다 **실제 차량 이동 + `DriveState`**로 보는 것이 맞다.

---

## 8. 유지 원칙
- 기존 `Throttle / Brake / Steering` API 유지
- 기존 축 입력 경로 즉시 삭제 금지
- 물리 키/스틱 하드코딩 금지
- 후진 전환에서 수동 기어 강제 재도입 금지

---

## 9. 변경 이력
- v0.2 (2026-04-16)
  - A안 검증 결과 반영.
  - 수동 기어 강제와 음수 스로틀 기반 reverse 강제를 제거한 최종 방향 기록.
  - `bUseAutoReverse` 기반 후진 전환 확인 결과 문서화.
- v0.1 (2026-04-15)
  - `ACFVehiclePawn` 상단에 2D 차량 이동 입력 해석 계층 추가 초안 작성.
  - 신규 `InputAction_VehicleMove` 추가안 제시.
  - 디버그 확장 항목 정의.
