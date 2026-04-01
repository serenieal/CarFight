# DriveState 튜닝 가이드

> 역할: `UCFVehicleData::DriveStateConfig` 값을 실제 자산에 넣을 때 보는 **첫 차량 튜닝 작업 문서**  
> 상위 방향 문서: `03_VisionAlign.md`  
> 현재 기준 테스트 자산: `DA_PoliceCar`  
> 마지막 정리(Asia/Seoul): 2026-03-20

---

## 이 문서의 목적
이 문서는 손맛을 끝까지 다듬는 문서가 아니다.

현재 목적은 아래 3가지다.
- 첫 차량에 들어갈 `DriveStateConfig` 시작값을 잡는다.
- 공통 코어를 깨지 않는 범위에서 차량별 차이를 연다.
- 이후 다른 차량으로 확장할 때 재사용 가능한 조정 순서를 남긴다.

---

## 먼저 이해할 것
`DriveStateConfig`는 코드값이 아니라 `UCFVehicleData` 안에 들어가는 **차량별 튜닝 데이터**다.

즉, 앞으로는
- 경찰차는 경찰차 느낌으로
- 다른 차량은 다른 차량 느낌으로
상태 판정을 다르게 열 수 있다.

하지만 지금 단계에서는
**공통 코어 문제를 덮지 않는 범위에서만** 차량별 차이를 연다.

---

## 현재 적용 경로
- `VehicleData->DriveStateConfig`
- `ACFVehiclePawn::ApplyVehicleDataConfig()`
- `UCFVehicleDriveComp::ApplyDriveStateConfig()`

이 경로로 실제 적용된다.

---

## 에디터에서 열 자산
### 자산 경로
- `/Game/CarFight/Data/Cars/DA_PoliceCar`

### 열기 순서
1. Content Browser에서 `DA_PoliceCar` 열기
2. Details 패널에서 `DriveStateConfig` 찾기
3. 아래 권장 시작값 입력
4. 저장
5. `BP_CFVehiclePawn` 또는 테스트맵에서 PIE 확인

---

## 권장 시작값
아래 값은 **첫 시작점**이다.
처음부터 완벽한 값이 아니라, 현재 구조에서 테스트를 시작하기 좋은 기준이다.

### 기본 토글
- `bUseDriveStateOverrides = true`
- `bEnableDriveStateHysteresis = true`
- `bUsePerStateHoldTimes = true`
- `bTreatOppositeThrottleAsBrake = true`

### 입력 판정
- `ActiveInputThreshold = 0.05`

### Idle 판정
- `IdleEnterSpeedThresholdKmh = 0.75`
- `IdleExitSpeedThresholdKmh = 1.50`
- `IdleStateMinimumHoldTimeSeconds = 0.10`

### Reverse 판정
- `ReverseEnterSpeedThresholdKmh = 1.25`
- `ReverseExitSpeedThresholdKmh = 0.75`
- `ReversingStateMinimumHoldTimeSeconds = 0.18`

### Airborne 판정
- `AirborneMinSpeedThresholdKmh = 4.00`
- `AirborneVerticalSpeedThresholdCmPerSec = 120.0`
- `AirborneStateMinimumHoldTimeSeconds = 0.08`

### 공통 HoldTime
- `DriveStateMinimumHoldTimeSeconds = 0.12`

---

## 조정 순서
반드시 아래 순서로만 조정한다.

### 1단계 — Idle
먼저 확인할 것:
- 정지 근처에서 `Idle` 진입이 늦지 않은가
- `Idle ↔ Coasting`이 경계에서 과하게 튀지 않는가

먼저 건드릴 값:
- `IdleEnterSpeedThresholdKmh`
- `IdleExitSpeedThresholdKmh`
- `IdleStateMinimumHoldTimeSeconds`

### 2단계 — Reversing
다음 확인할 것:
- 정지 후 후진 진입이 자연스러운가
- `Braking ↔ Reversing` 경계가 흔들리지 않는가

먼저 건드릴 값:
- `ReverseEnterSpeedThresholdKmh`
- `ReverseExitSpeedThresholdKmh`
- `ReversingStateMinimumHoldTimeSeconds`

### 3단계 — Airborne
마지막으로 확인할 것:
- 작은 요철에서 너무 쉽게 뜨지 않는가
- 큰 점프/낙하에서 너무 늦게 뜨지 않는가

먼저 건드릴 값:
- `AirborneMinSpeedThresholdKmh`
- `AirborneVerticalSpeedThresholdCmPerSec`
- `AirborneStateMinimumHoldTimeSeconds`

---

## PIE에서 확인할 것
### Pawn 디버그 옵션
`BP_CFVehiclePawn` 또는 배치된 Pawn에서 아래 옵션을 켠다.
- `bEnableDriveStateOnScreenDebug = true`
- `bShowDriveStateTransitionSummary = true`

### 확인해야 하는 디버그 값
- `DriveState`
- `Prev`
- `Candidate`
- `Hold`
- `Speed`
- `Forward`
- `Grounded`

---

## 이상할 때 조정 규칙
### A. 정지 근처에서 `Idle`이 너무 늦게 들어온다
- `IdleEnterSpeedThresholdKmh`를 조금 올린다.

### B. `Idle`과 `Coasting`이 자꾸 튄다
- `IdleExitSpeedThresholdKmh`를 조금 올린다.
- `IdleStateMinimumHoldTimeSeconds`를 조금 올린다.

### C. 후진 진입이 너무 늦다
- `ReverseEnterSpeedThresholdKmh`를 조금 내린다.

### D. 후진 진입 직후 `Braking ↔ Reversing`이 자꾸 튄다
- `ReversingStateMinimumHoldTimeSeconds`를 조금 올린다.

### E. 작은 턱에서 너무 쉽게 `Airborne`이 뜬다
- `AirborneMinSpeedThresholdKmh`를 올린다.
- `AirborneVerticalSpeedThresholdCmPerSec`를 올린다.

### F. 큰 점프인데 `Airborne`이 너무 늦게 뜬다
- `AirborneMinSpeedThresholdKmh`를 내린다.
- `AirborneVerticalSpeedThresholdCmPerSec`를 내린다.

---

## 기록 규칙
값을 바꿀 때는 아래 형식으로 짧게 남긴다.

- 변경 항목:
- 이전 값:
- 새 값:
- 확인한 현상:
- 결과:

예:
- 변경 항목: `ReverseEnterSpeedThresholdKmh`
- 이전 값: `1.25`
- 새 값: `1.00`
- 확인한 현상: 후진 진입이 늦음
- 결과: 정지 후 후진 진입이 더 빨라짐

---

## 금지 사항
- 한 번에 여러 항목을 크게 바꾸기
- `Idle`, `Reverse`, `Airborne`를 섞어서 동시에 조정하기
- 공통 코어 문제를 차량별 수치로 덮어버리기
- 경찰차 느낌 완성 자체를 이번 단계의 목표로 착각하기

---

## 이번 단계의 완료 조건
- [ ] `DA_PoliceCar`에 시작값이 들어가 있다.
- [ ] 조정 순서가 기록되어 있다.
- [ ] 변경 이유와 결과가 남아 있다.
- [ ] 공통 코어 문제와 차량별 튜닝 문제가 분리되어 있다.
