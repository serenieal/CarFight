# CF_VehicleMoveChecklist

## 문서 목적
본 문서는 `CF_VehicleMoveSpec.md`, `CF_VehicleMoveDesign.md`, `CF_VehicleMoveTasks.md`, `CF_VehicleMoveWork.md`, `CF_VehicleMoveCpp.md` 기준으로 **현재 구현 진행도와 남은 작업**을 체크리스트 형태로 정리한 진행 점검 문서다.

이 문서의 목적은 아래 두 가지다.

- 현재 무엇이 끝났고 무엇이 아직 덜 끝났는지 한눈에 확인하기
- 다음 작업을 기능 추가가 아니라 **안정화/정리 관점**에서 바로 이어갈 수 있게 만들기

---

## 1. 현재 요약

### 1-1. 현재 판단
- 핵심 기능 구현: **완료**
- 실제 PIE 동작 확인: **완료**
- 전체 계획 기준 진행도: **약 90~95%**
- 남은 작업 성격: **기능 추가보다 회귀 검증 / 산출물 정리 / 체감 튜닝 중심**


### 1-2. 현재 확정 구조
- 입력 각도/영역 해석: **CarFight 커스텀 로직**
- 후진 전환의 최종 결정권: **Chaos Vehicle `bUseAutoReverse`**
- 수동 기어 강제: **미사용**
- 음수 스로틀 기반 reverse 강제: **미사용**

---

## 2. 완료 체크리스트

## 2-1. 입력 자산 및 경로
- [x] 차량 이동용 `Axis2D` Input Action 경로를 사용한다.
- [x] `IA_VehicleMove` 경로가 실제로 연결되어 있다.
- [x] `ACFVehiclePawn`에 `InputAction_VehicleMove` 프로퍼티가 추가되어 있다.
- [x] `SetupPlayerInputComponent()`에 `InputAction_VehicleMove` 바인딩이 추가되어 있다.
- [x] 게임패드는 `IA_VehicleMove` 기반 신규 조작을 사용한다.
- [x] 키보드는 기존 `IA_Throttle / IA_Brake / IA_Steering` 기반 조작을 유지한다.
- [x] IMC를 장치별로 분리했고 실제 동작 확인까지 완료했다.


## 2-2. Pawn 상태/설정 구조
- [x] `FCFVehicleMoveInputConfig`가 추가되어 있다.
- [x] `FCFVehicleMoveInputResult`가 추가되어 있다.
- [x] `LastMoveDirectionIntent`가 추가되어 있다.
- [x] `LastVehicleMoveInputResult`가 추가되어 있다.
- [x] 차량 이동 입력 관련 enum/상태 구조가 헤더에 정리되어 있다.

## 2-3. 2D 입력 해석 로직
- [x] `Magnitude` 계산이 구현되어 있다.
- [x] `AngleDeg` 계산이 구현되어 있다.
- [x] 위=0도, 시계 방향 증가 기준이 구현되어 있다.
- [x] 경계각 포함 비교가 구현되어 있다.
- [x] 쓰로틀 영역 판정이 구현되어 있다.
- [x] 후진 영역 판정이 구현되어 있다.
- [x] 검은 영역 판정이 구현되어 있다.
- [x] `Steering = InputX` 규칙이 구현되어 있다.

## 2-4. 검은 영역 유지 정책
- [x] `HoldDirection_UseCurrentMagnitude` 기본 정책이 구현되어 있다.
- [x] 직전 방향 의도 `Forward` 유지가 구현되어 있다.
- [x] 직전 방향 의도 `Reverse` 유지가 구현되어 있다.
- [x] 방향 이력 없음(`None`)일 때 중립 처리 경로가 있다.
- [x] `bUsedBlackZoneHold` 디버그 항목이 존재한다.

## 2-5. 후진/브레이크 처리
- [x] 전진 중 후진 영역 입력은 브레이크로 해석된다.
- [x] 후진 전환은 Chaos Vehicle `bUseAutoReverse` 기준으로 정리되었다.
- [x] 수동 기어 강제(`SetTargetGear`)는 제거되었다.
- [x] 음수 스로틀 기반 reverse 강제는 제거되었다.
- [x] 실제 PIE에서 후진 동작을 확인했다.

## 2-6. 디버그
- [x] `MoveZone` 디버그가 있다.
- [x] `MoveIntent` 디버그가 있다.
- [x] `MoveRaw` 디버그가 있다.
- [x] `MoveMag` 디버그가 있다.
- [x] `MoveAngle` 디버그가 있다.
- [x] `BlackHold` 디버그가 있다.
- [x] `ForwardSpeed` 디버그가 있다.
- [x] `DriveState` 기반 확인 경로가 유지된다.
- [x] `InputOwner` 디버그가 추가되어 있다.

## 2-7. 입력 충돌 방지(Runtime Input Ownership)
- [x] `VehicleMove2D / LegacyAxis / None` 입력 소유권 enum이 추가되어 있다.
- [x] `CurrentInputOwnership` 상태값이 추가되어 있다.
- [x] `InputOwnershipHoldTimeSec` 유지 시간이 추가되어 있다.
- [x] `LastVehicleMoveInputTimeSec` / `LastLegacyAxisInputTimeSec` 추적값이 추가되어 있다.
- [x] `VehicleMove`와 기존 축 입력 간 코드 레벨 게이트가 구현되어 있다.
- [x] release 경로가 현재 소유권이 아닌 입력을 덮어쓰지 않도록 보강되었다.
- [x] 빌드 성공을 확인했다.
- [x] PIE 실기 테스트에서 정상 동작을 확인했다.

## 2-8. 문서 반영

- [x] `CF_VehicleMoveSpec.md`에 A안 결과가 반영되어 있다.
- [x] `CF_VehicleMoveDesign.md`에 `bUseAutoReverse` 중심 구조가 반영되어 있다.
- [x] `CF_VehicleMoveTest.md`에 후진 검증 기준이 갱신되어 있다.
- [x] `CF_VehicleMoveTasks.md`에 A안 완료 기준이 반영되어 있다.
- [x] `CF_VehicleMoveWork.md`와 `CF_VehicleMoveCpp.md`에 실제 구현 결과가 반영되어 있다.

---

## 3. 부분 완료 체크리스트

## 3-1. 기존 축 입력과 2D 입력 공존 안정화
- [x] `VehicleMove` 활성 시 기존 `Throttle / Steering / Brake` 단일 축 입력을 코드 레벨에서 명확히 무시한다.
- [x] IMC 분리와 별개로 런타임 입력 소유권(Runtime Input Ownership)으로 중복 입력을 제어한다.
- [x] `VehicleMove2D / LegacyAxis / None` 소유권 전환이 실제 플레이에서 정상 동작한다.

### 판단
현재 상태는 **IMC 분리 + 코드 레벨 충돌 방지**가 모두 들어간 상태다.
즉, 이 항목은 P1 완료로 본다.


## 3-2. 검은 영역 후진 유지 체감 튜닝
- [x] 검은 영역에서 직전 `Reverse` 의도 유지 경로는 있다.
- [x] 현재 A안 기준으로 브레이크 전달 + `bUseAutoReverse` 구조가 동작한다.
- [ ] 실제 체감이 가장 좋은지에 대한 추가 튜닝은 아직 안 끝났다.

### 판단
기능은 구현됐지만, **최적 체감값 / 최종 UX 튜닝**까지는 아직 아니다.

## 3-3. 회귀 검증 범위 확대
- [x] 핵심 주행 입력은 검증했다.
- [x] 실제 후진 전환은 검증했다.
- [ ] `KeyboardMouseOnly` / `GamepadOnly`를 체계적으로 다시 검증했다는 증거는 아직 부족하다.
- [ ] IMC 리매핑 변경 후 동일 의미 유지까지 폭넓게 검증하지는 않았다.
- [ ] DriveState 전이 전체 회귀를 한 번씩 모두 점검했다고 보긴 어렵다.

### 판단
핵심 시나리오 기준으론 충분하지만, **전체 회귀 검증 문서화**는 아직 덜 끝났다.

## 3-4. 작업 산출물 정리
- [x] 수정된 C++ 파일 changelog를 별도 정리했다.
- [x] IMC 변경 요약을 별도 정리했다.
- [ ] PIE 검증 캡처 또는 테스트 로그를 폴더 산출물로 남겼다.
- [x] 남은 이슈 목록을 별도 문서로 정리했다.
- [x] 산출물 정리 문서 `CF_VehicleMoveArtifacts.md`를 작성했다.

### 판단
핵심 산출물 문서화는 끝났고, **남은 건 PIE 캡처 또는 로그 같은 증빙 파일 정리**다.


---

## 4. 아직 안 한 것

아래 항목은 현재 핵심 범위 밖이거나 후속 과제로 남아 있다.

- [ ] 입력 해석 설정의 DataAsset화 또는 Config화
- [ ] 차량 입력 해석 디버그 위젯 추가
- [ ] AI/리플레이/네트워크 관점 입력 기록 형식 정리
- [ ] 카메라/핸드브레이크/전투 입력과의 충돌 점검
- [ ] 검은 영역 후진 유지 정책 추가 튜닝안 정리

---

## 5. 우선순위가 높은 남은 작업

### Priority 1. 회귀 검증 정리
- [ ] `KeyboardMouseOnly` / `GamepadOnly` / `Auto` 모드 검증 결과를 체크리스트나 테스트 로그로 남긴다.
- [ ] IMC 리매핑 변경 시나리오를 최소 1회 이상 문서화한다.
- [ ] DriveState 전이 회귀를 한 번씩 점검하고 기록한다.

### Priority 2. 산출물 정리
- [x] IMC 변경 요약 작성
- [ ] PIE 검증 캡처 또는 로그 저장
- [x] 수정된 C++ changelog 작성
- [x] 남은 이슈 목록 작성
- [x] 산출물 정리 문서 `CF_VehicleMoveArtifacts.md` 작성


### Priority 3. 체감 튜닝
- [ ] 검은 영역 후진 유지 체감을 다시 점검한다.
- [ ] 각도값과 브레이크 유지 감각의 추가 튜닝 필요 여부를 판단한다.


---

## 6. 현재 완료 판정

### 6-1. 기능 완료 여부
- **예**

핵심 목표였던 아래는 끝났다.

- 2D 입력 기반 차량 이동 해석
- 전진/조향 동작
- 후진 영역 브레이크 처리
- `bUseAutoReverse` 기반 후진 전환
- 실제 PIE 동작 확인

### 6-2. 실사용 가능 여부
- **예**

현재 기준으로는 아래까지 확인됐다.

- IMC 분리 적용 완료
- 코드 레벨 충돌 방지 완료
- 빌드 성공 확인
- PIE 실기 테스트 확인


### 6-3. 전체 계획 100% 완료 여부
- **아직 아님**

기능 자체보다 **안정화 / 회귀 검증 / 정리 산출물**이 남아 있다.

---

## 7. 다음 작업 권장 순서

1. 기존 축 입력과 `VehicleMove` 중복 방지 정책 확정
2. 장치 모드/리매핑/회귀 검증 로그 정리
3. IMC 변경 요약 및 PIE 검증 산출물 저장
4. 남은 이슈 목록 작성

---

## 8. 변경 이력
- v0.1 (2026-04-16)
  - 현재 구현 진행도를 `완료 / 부분 완료 / 미완료`로 정리한 체크리스트 문서 최초 작성.
  - 실제 PIE 동작 확인 결과와 문서/코드 대조 결과를 반영.
