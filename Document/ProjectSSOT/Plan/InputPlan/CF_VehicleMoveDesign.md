# CF_VehicleMoveDesign

## 문서 목적
본 문서는 `CF_VehicleMoveSpec.md`를 기준으로, 현재 CarFight 네이티브 구조 위에 입력 해석 계층을 어떻게 안전하게 추가할지 정리한 구현 설계서다.

현재 확인된 기준선은 아래다.

- `ACFVehiclePawn`
- `UCFVehicleDriveComp`
- `UCFWheelSyncComp`
- Enhanced Input 바인딩 구조
- `InputDeviceMode` 기반 입력 허용/차단 로직
- Chaos Vehicle `bUseAutoReverse`

핵심 방향은 **기존 Drive 전달 구조를 유지하고, 그 위에 차량 이동 입력 해석 계층을 추가하되, 후진 전환의 최종 결정권은 Chaos Vehicle에 둔다**는 것이다.

---

## 1. 현재 구조 요약

### 1-1. `ACFVehiclePawn` 역할
현재 `ACFVehiclePawn`은 아래 역할을 가진다.

- Enhanced Input Action 바인딩
- 입력 장치 모드 허용/차단 검사
- `Throttle / Steering / Brake / Handbrake`를 `VehicleDriveComp`로 전달
- 2D 이동 입력의 각도/영역 해석
- 디버그 문자열 출력

즉, 입력 수집과 입력 의미 해석은 Pawn에 있고, 실제 차량 입력 적용은 `DriveComp`와 Chaos Vehicle이 담당한다.

### 1-2. `UCFVehicleDriveComp` 역할
현재 `UCFVehicleDriveComp`는 아래 역할을 가진다.

- `ApplyThrottleInput`
- `ApplySteeringInput`
- `ApplyBrakeInput`
- `ApplyHandbrakeInput`
- DriveState 계산
- 반대 방향 입력을 제동으로 보는 규칙(`bTreatOppositeThrottleAsBrake`)

즉, 이번 작업은 `DriveComp`를 새로 뒤엎는 작업이 아니다.
`DriveComp`에 전달하기 전 단계에서 **2D 이동 입력을 해석해 1D 입력들로 분해**하는 작업이다.

### 1-3. Chaos Vehicle 역할
후진 전환의 최종 책임은 아래로 정리한다.

- `bUseAutoReverse = true`
- 브레이크 유지 중 정지 후 후진 전환
- reverse gear 진입 및 실제 후진 물리 반응

즉, **CarFight는 후진 의도를 해석하고 브레이크를 전달하며, 실제 reverse 전환은 Chaos Vehicle이 한다.**

---

## 2. 권장 구현 방향

### 2-1. 기본안
가장 안정적인 기본안은 아래다.

1. 차량 이동용 `Axis2D` 입력 액션을 Pawn에서 받는다.
2. Pawn 내부 해석 함수에서 입력 벡터를 해석한다.
3. 결과를 아래 출력으로 변환한다.
   - Steering
   - Throttle
   - Brake
   - ReverseIntent
4. 최종적으로 기존 `SetVehicleThrottleInput / SetVehicleBrakeInput / SetVehicleSteeringInput` 경로에 연결한다.
5. 후진 의도일 때도 수동 기어 강제나 음수 스로틀 강제를 하지 않고, Chaos Vehicle의 `bUseAutoReverse`에 후진 전환을 맡긴다.

### 2-2. 비권장안
아래는 현재 기준으로 비권장이다.

- `DriveComp` 내부를 직접 2D 입력 해석기로 바꾸는 것
- 기존 `Throttle / Brake / Steering` API를 제거하는 것
- 물리 키/스틱 배치를 C++ 상수로 박는 것
- 수동 기어 강제(`SetTargetGear(-1 / 1)`)를 상시 사용하는 것
- 음수 스로틀을 이용해 reverse를 직접 강제하는 것

---

## 3. 구현 계층 분리

### 3-1. 권장 계층
아래처럼 3계층으로 분리한다.

#### A. 입력 수집 계층
- Enhanced Input
- Input Mapping Context
- Input Action 바인딩

#### B. 입력 해석 계층
- 2D 입력 벡터 -> 차량 의미값 변환
- 각도/반지름 계산
- 영역 판정
- 검은 영역 유지 정책 적용
- 장치 모드 허용/차단 반영

#### C. 차량 적용 계층
- `SetVehicleThrottleInput`
- `SetVehicleBrakeInput`
- `SetVehicleSteeringInput`
- `SetVehicleHandbrakeInput`
- Chaos Vehicle `bUseAutoReverse`

---

## 4. 구현 위치 권장안

### 4-1. 1차 권장안: `ACFVehiclePawn` 내부 헬퍼 추가
현재 구조를 최대한 덜 흔들기 위해 1차 구현은 `ACFVehiclePawn` 내부 헬퍼 함수 추가를 권장한다.

이유는 아래와 같다.

- 입력 액션 바인딩이 이미 Pawn에 있다.
- `InputDeviceMode` 검사도 Pawn에 있다.
- `DriveComp` 전달 API도 Pawn에 이미 정리돼 있다.
- 작은 범위에서 기능을 붙이고 검증하기 좋다.
- 실제 검증 결과, 후진 전환은 엔진 기본 기능에 맡기는 쪽이 더 안정적이었다.

### 4-2. 향후 확장안
입력 해석 규칙이 더 커지면 별도 구조로 분리할 수 있다.

예시:
- `FCFVehicleMoveInputConfig`
- `FCFVehicleMoveInputState`
- `FCFVehicleMoveInputResult`
- `UCFVehicleInputInterpreter` 같은 별도 객체/컴포넌트

하지만 현재 단계에서는 과분할보다 **Pawn 내부 헬퍼 + 명확한 구조체**가 안전하다.

---

## 5. 권장 데이터 구조

### 5-1. 설정 구조체
입력 해석 수치를 묶는 설정 구조체를 하나 두는 것을 권장한다.

예시 필드:
- `ThrottleStartAngleDeg`
- `ThrottleEndAngleDeg`
- `ReverseStartAngleDeg`
- `ReverseEndAngleDeg`
- `bUseInclusiveAngleBoundary`
- `bEnableBlackZoneDirectionHold`
- `bUseCurrentMagnitudeInBlackZone`

### 5-2. 런타임 결과 구조체
입력 해석 결과도 구조체로 묶는 편이 좋다.

예시 필드:
- `RawInputVector`
- `InputMagnitude`
- `InputAngleDeg`
- `SteeringValue`
- `ThrottleValue`
- `BrakeValue`
- `ResolvedZone`
- `ResolvedDirectionIntent`
- `bUsedBlackZoneFallback`
- `DebugSummary`

이 구조체는 디버그와 테스트에서 매우 유용하다.

---

## 6. 최근 방향 의도 저장 위치
최근 방향 의도는 Pawn에 두는 것을 권장한다.

이유:
- 검은 영역 정책은 입력 해석 책임에 가깝다.
- DriveState와는 연관되지만 동일 개념은 아니다.
- 입력 장치 모드, 액션값, 해석 규칙과 더 밀접하다.

권장 상태값:
- `None`
- `Forward`
- `Reverse`

---

## 7. 후진 처리 방식 권장안

### 7-1. 검토했던 방식
#### A안. Signed Throttle 사용
- 전진 = `+Throttle`
- 후진 = `-Throttle`
- 브레이크는 별도

#### B안. ReverseIntent를 해석 후 기존 API로 변환
- 상위 해석기에서는 `ReverseIntent`를 유지
- 실제 적용 시점에 `Throttle / Brake`로 변환

### 7-2. 실제 검증 결과
실제 구현 및 테스트 결과, 아래 방식이 안정적이었다.

- 후진 영역의 의미 해석은 CarFight가 담당
- 전진 중 뒤 입력은 `Brake = Magnitude`
- 정지 근처 및 정지 후에도 수동 기어 강제를 하지 않음
- 음수 스로틀로 reverse를 직접 강제하지 않음
- **Chaos Vehicle `bUseAutoReverse=true`가 브레이크 유지 후 후진 전환을 담당**

### 7-3. 현재 권장안
현재 단계의 권장안은 아래 한 줄이다.

**ReverseIntent는 입력 해석 계층에서 유지하되, 실제 후진 전환은 Chaos Vehicle `bUseAutoReverse`에 맡긴다.**

---

## 8. 장치 모드 처리
현재 `ShouldAcceptActionInput()`는 `Auto`일 때 사실상 모두 허용한다.

이번 변경에서도 이 원칙은 유지하되, 새 `Axis2D` 차량 이동 액션에도 동일한 정책을 적용해야 한다.

### Auto
- 현재 입력 액션값을 그대로 수용

### KeyboardMouseOnly
- 게임패드 계열 매핑 무시

### GamepadOnly
- 키보드/마우스 계열 매핑 무시

중요한 점은, 장치 배치 판정은 **물리 키 이름 하드코딩이 아니라 현재 매핑된 키가 게임패드 키인지 여부**로 판단해야 한다.

---

## 9. 디버그 설계
이번 기능은 체감형 기능이라 디버그가 중요하다.

최소한 아래 항목을 즉시 볼 수 있어야 한다.

- Raw 2D Input
- Magnitude
- AngleDeg
- ResolvedZone
- LastDirectionIntent
- SteeringOutput
- ThrottleOutput
- BrakeOutput
- VehicleSpeedKmh
- ForwardSpeedKmh
- DeviceMode
- `DriveState`

특히 현재 구조에서는 **후진 여부를 `Throttle < 0`가 아니라 `DriveState`와 실제 차량 이동**으로 확인해야 한다.

---

## 10. 파일/함수 작업 후보

### 우선 수정 후보
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

### 검토 후보
- `UE/Source/CarFight_Re/Public/CFVehicleDriveComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleDriveComp.cpp`

### 에셋/입력 설정 후보
- 차량 이동용 `Axis2D` Input Action
- 기존 IMC 차량 입력 매핑

---

## 11. 단계별 구현 순서

### 1단계
- 차량 이동용 `Axis2D` 입력 액션 정의
- IMC에서 키보드/게임패드 공용 입력 경로 구성

### 2단계
- Pawn에 입력 해석용 설정/상태 구조 추가
- 최근 방향 의도 상태 추가

### 3단계
- 2D 입력 해석 함수 추가
- 각도/강도/영역 판정 구현
- 검은 영역 유지 정책 구현

### 4단계
- 해석 결과를 기존 `Throttle / Brake / Steering` 적용 함수로 연결
- 후진 전환은 `bUseAutoReverse`와 충돌하지 않도록 수동 기어 강제를 제거

### 5단계
- Pawn 디버그 문자열 확장
- PIE 검증

---

## 12. 구현 시 주의사항
- 기존 `Throttle / Brake / Steering` 단일 축 경로를 갑자기 제거하지 않는다.
- 기존 함수 시그니처를 말 없이 바꾸지 않는다.
- 장치/키 배치를 코드 상수로 직접 박지 않는다.
- 검은 영역을 단순 0입력으로 처리하지 않는다.
- 전진 중 후진 영역에서 즉시 역추진하지 않는다.
- `bUseAutoReverse`를 쓰는 동안 수동 기어 강제를 다시 넣지 않는다.

---

## 13. 권장 결론
현재 가장 안전한 구현 방향은 아래 한 줄로 요약된다.

**`ACFVehiclePawn`에 2D 차량 이동 입력 해석 계층을 추가하고, 결과를 기존 `UCFVehicleDriveComp` 전달 API로 연결하되, 후진 전환의 최종 결정권은 Chaos Vehicle `bUseAutoReverse`에 둔다.**

---

## 14. 변경 이력
- v0.2 (2026-04-16)
  - A안 검증 결과를 반영.
  - 수동 기어 강제 및 음수 스로틀 기반 후진 강제를 제거하는 방향으로 정리.
  - 후진 전환의 최종 결정권을 Chaos Vehicle `bUseAutoReverse`로 고정.
- v0.1 (2026-04-15)
  - 현재 CarFight 네이티브 구조 기준 입력 해석 계층 추가 방향 고정.
  - 별도 2D 입력 해석 계층 + 기존 Drive 전달 API 유지 원칙 기록.
