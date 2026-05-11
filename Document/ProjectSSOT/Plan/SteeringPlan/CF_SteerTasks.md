# CF_SteerTasks

## 문서 목적
본 문서는 `CF_SteerSpec.md`, `CF_SteerDesign.md`, `CF_SteerRisk.md` 기준으로 실제 구현 작업 순서와 완료 조건을 정리한 실행 문서다.

---

## 1. 작업 목표
이번 작업의 목표는 아래 한 줄이다.

**게임패드 신규 조작에서 스틱 크기와 조향량의 직접 비례 관계를 끊고, 스틱 방향 기반 목표 조향과 제한 속도 기반 실제 조향을 구현한다.**

---

## 2. 직접 수정 후보 파일

### C++
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

### 검토 후보
- `UE/Source/CarFight_Re/Public/CFVehicleDriveComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleDriveComp.cpp`

---

## 3. 구현 단계

### Phase 1. 현재 Tick / Release 구조 확인
- [ ] `ACFVehiclePawn`에서 Tick 사용 여부 확인
- [ ] `HandleVehicleMoveReleased()`에서 조향값 즉시 0 리셋 여부 확인
- [ ] 기존 Runtime Input Ownership과 조향 release 경로 충돌 가능성 확인
- [ ] `UCFVehicleDriveComp::ApplySteeringInput()` 내부 보간 여부 확인

### Phase 2. 설정값 추가
- [ ] `SteeringDirectionMinMagnitude` 추가
- [ ] `SteeringLockToLockTimeSec` 추가
- [ ] `SteeringReturnMinSpeedKmh` 추가
- [ ] `SteeringReturnMaxSpeedKmh` 추가
- [ ] `SteeringReturnMinRate` 추가
- [ ] `SteeringReturnMaxRate` 추가

권장 기본값:

```text
SteeringDirectionMinMagnitude = 0.10
SteeringLockToLockTimeSec = 0.45
SteeringReturnMinSpeedKmh = 0.0
SteeringReturnMaxSpeedKmh = 80.0
SteeringReturnMinRate = 0.4
SteeringReturnMaxRate = 6.0
```

### Phase 3. 런타임 상태값 추가
- [ ] `CurrentSteeringInput` 추가
- [ ] `TargetSteeringInput` 추가
- [ ] `LastSteeringTurnRate` 추가
- [ ] `LastSteeringReturnRate` 추가
- [ ] `bLastSteeringReturningToCenter` 추가

### Phase 4. 목표 조향 계산 변경
- [ ] 기존 `SteeringValue = MoveInputVector.X` 직접 비례 구조 제거
- [ ] `MoveInputVector / Magnitude` 기반 방향 계산 추가
- [ ] `NormalizedDirection.X`를 목표 조향값으로 사용
- [ ] `Magnitude < SteeringDirectionMinMagnitude`이면 목표 조향 0 처리

### Phase 5. 실제 조향값 보간 추가
- [ ] `UpdateCurrentSteeringInput()` 또는 동등한 helper 추가
- [ ] 조향 의도 있음: `SteeringLockToLockTimeSec` 기반 turn rate 적용
- [ ] 조향 의도 없음: 속도 기반 return rate 적용
- [ ] `CurrentSteeringInput`을 `SetVehicleSteeringInput()`에 전달

### Phase 6. 중립 복귀 처리 위치 확정
- [ ] 입력 이벤트가 없는 동안에도 중립 복귀가 진행되도록 처리
- [ ] Tick에서 처리할지 기존 입력 갱신 루프에서 처리할지 확정
- [ ] release 시 조향 즉시 0 리셋 금지

### Phase 7. 디버그 추가
- [ ] `SteerTarget` 출력
- [ ] `SteerCurrent` 출력
- [ ] `SteerTurnRate` 출력
- [ ] `SteerReturnRate` 출력
- [ ] `SteerReturning` 출력

### Phase 8. 빌드 / PIE 테스트
- [ ] `CarFight_ReEditor / Win64 / Development` 빌드
- [ ] PIE에서 조향/가속 분리 확인
- [ ] 스틱 중립 시 속도 기반 중립 복귀 확인
- [ ] Runtime Input Ownership 회귀 확인

---

## 4. 완료 조건
아래를 모두 만족하면 1차 완료로 본다.

- 스틱을 살짝 기울여도 방향이 명확하면 목표 조향이 방향 기준으로 계산된다.
- 스틱 기울기 정도는 가속 / 브레이크 강도에만 영향을 준다.
- 실제 바퀴 조향은 목표값으로 즉시 점프하지 않는다.
- 스틱 중립 시 바퀴가 속도 기반으로 천천히 중립 복귀한다.
- 고속 급조향은 억지로 약화하지 않는다.
- 입력 노이즈로 인한 조향 튐은 최소 입력값으로 방지한다.
- 빌드가 성공한다.
- PIE에서 기본 주행이 정상 동작한다.

---

## 5. 변경 이력
- v0.1 (2026-05-11)
  - 조향 변경안의 단계별 구현 작업 문서 최초 작성.
