# CF_VehicleMoveWork

## 문서 목적
본 문서는 `CF_VehicleMoveSpec.md`, `CF_VehicleMoveDesign.md`, `CF_VehicleMoveTest.md`, `CF_VehicleMoveTasks.md`를 기준으로 바로 구현에 들어가기 위한 **구현 작업 문서 v0.2**이다.

본 문서는 아래를 하나로 묶는다.

- 이번 작업의 실제 구현 목표
- 우선 수정 파일과 검토 파일
- 단계별 구현 순서
- 각 단계의 산출물
- 디버그/검증 체크포인트
- 완료 기준
- 작업 중 지켜야 하는 금지사항
- 실제 검증 결과 요약

---

## 1. 기준 문서
1. `CF_VehicleMoveSpec.md`
2. `CF_VehicleMoveDesign.md`
3. `CF_VehicleMoveTest.md`
4. `CF_VehicleMoveTasks.md`

구현 도중 위 문서와 충돌이 발생하면, 임의로 코드를 바꾸지 말고 먼저 문서 기준을 다시 확인한다.

---

## 2. 이번 구현의 핵심 목표
**기존 `Throttle / Steering / Brake` 전달 구조를 유지하면서, 차량 이동용 `Axis2D` 입력 해석 계층을 `ACFVehiclePawn` 상단에 추가한다.**

단, 후진 전환의 최종 결정권은 아래로 둔다.

- **UE Chaos Vehicle `bUseAutoReverse = true`**

즉 이번 작업은 아래를 하지 않는다.

- `DriveComp` 전체 재설계
- 물리 키 하드코딩
- 좌스틱/우스틱 고정
- 카메라 입력 재설계
- 핸드브레이크 입력 재설계
- 수동 기어 강제 기반 후진 구현

---

## 3. 구현 범위

### 포함 범위
- 차량 이동용 `Axis2D` 입력 해석
- 입력 벡터의 각도/강도 계산
- 쓰로틀 / 후진 / 검은 영역 판정
- 검은 영역 `HoldDirection_UseCurrentMagnitude` 정책 적용
- 전진 중 후진 영역 입력을 브레이크로 해석
- `bUseAutoReverse` 기반 후진 전환 검증
- 장치 모드별 입력 허용/차단 유지
- 디버그 출력 확장

### 제외 범위
- 카메라 입력 로직 개편
- 물리 키/스틱 위치 고정
- 핸드브레이크 바인딩 고정
- IMC 데드존 설계 변경 강제
- `UCFVehicleDriveComp`의 DriveState 전체 개편
- 수동 역기어 강제

---

## 4. 우선 작업 파일

### 직접 수정 대상
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

### 검토 대상
- `UE/Source/CarFight_Re/Public/CFVehicleDriveComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleDriveComp.cpp`

### 입력 에셋 대상
- 차량 이동용 `Axis2D` Input Action
- 차량용 IMC

---

## 5. 구현 원칙

### 5-1. 구조 원칙
- 입력 수집은 Pawn에 둔다.
- 입력 해석도 1차 구현은 Pawn 내부 헬퍼로 둔다.
- 차량 적용은 기존 `DriveComp` 전달 구조를 유지한다.
- 후진 전환은 Chaos Vehicle `bUseAutoReverse`에 맡긴다.

### 5-2. 하드코딩 금지 원칙
- 특정 물리 키 이름 박기 금지
- 특정 스틱 위치 박기 금지
- 카메라 입력 배치 고정 금지
- 핸드브레이크 버튼 배치 고정 금지
- 데드존 수치 코드 강제 고정 금지
- 수동 기어 강제 금지

### 5-3. 안정성 원칙
- 기존 함수 시그니처를 말 없이 바꾸지 않는다.
- 기존 축 입력 경로를 바로 삭제하지 않는다.
- 디버그를 먼저 붙이고 체감 확인 후 정리한다.

---

## 6. 권장 구현 순서

### Phase 0. 입력 경로 정리
- 차량 이동용 2D Input Action 준비
- 키보드/게임패드 공용 입력 연결
- 현재 차량용 IMC와 충돌 확인

### Phase 1. Pawn 상태/설정 추가
- 각도 범위 설정값 추가
- 최근 방향 의도 상태 추가
- 마지막 해석 결과 보관용 상태 추가
- 디버그 출력용 문자열/상태 추가

### Phase 2. 2D 입력 해석 함수 구현
- `Magnitude` 계산
- `AngleDeg` 계산
- 쓰로틀/후진/검은 영역 판정
- `Steering = InputX` 적용

### Phase 3. 검은 영역 유지 정책 적용
- 최근 방향 의도 `None / Forward / Reverse` 관리
- 검은 영역 진입 시 직전 방향 유지
- 강도는 현재 `Magnitude` 사용

### Phase 4. 브레이크/후진 전환 처리
- 전진 중 후진 영역 = 브레이크
- 후진 의도는 유지하되 음수 스로틀/수동 기어 강제는 제거
- 실제 후진 전환은 `bUseAutoReverse`로 처리

### Phase 5. 기존 적용 함수 연결
- Steering 결과를 기존 조향 적용 경로에 연결
- Throttle 결과를 기존 스로틀 적용 경로에 연결
- Brake 결과를 기존 브레이크 적용 경로에 연결
- 장치 모드 허용/차단 정책 유지 확인

### Phase 6. 디버그/PIE 검증
- RawInputX / RawInputY 출력
- Magnitude / AngleDeg 출력
- ResolvedZone 출력
- LastDirectionIntent 출력
- Steering / Throttle / Brake 출력
- BlackZoneHold 사용 여부 출력
- VehicleSpeedKmh / ForwardSpeedKmh 출력
- DriveState 출력
- `bUseAutoReverse` 기반 후진 전환 확인

---

## 7. 디버그 체크포인트

### 입력 해석
- 위 입력이 `0°`로 계산되는가
- 오른쪽 입력이 `90°`로 계산되는가
- 아래 입력이 `180°`로 계산되는가
- 왼쪽 입력이 `270°`로 계산되는가

### 영역 판정
- `260° ~ 100°`가 쓰로틀인가
- `135° ~ 225°`가 후진인가
- 나머지가 검은 영역인가
- 경계각이 포함되는가

### 유지 정책
- 전진 후 검은 영역에서 전진 유지되는가
- 후진 의도 후 검은 영역에서 후진 의도 유지가 되는가
- 방향 이력 없음 + 검은 영역에서 중립인가

### 속도 분기
- 전진 중 후진 영역 입력이 브레이크인가
- `bUseAutoReverse` 기준으로 실제 후진 전환이 되는가

---

## 8. 실제 검증 결과
현재 기준 실제 검증 결과는 아래다.

- `IA_VehicleMove` 경로 동작 확인
- 앞/좌/우 입력 동작 확인
- 수동 기어 강제 방식은 제거
- 음수 스로틀 기반 후진 강제는 제거
- **Chaos Vehicle `bUseAutoReverse = true` 환경에서 실제 후진 동작 확인**
- 게임패드 신규 조작 / 키보드 기존 조작 IMC 분리 확인
- **P1 입력 충돌 방지(Runtime Input Ownership) 구현 완료**
- `InputOwner` 디버그 확인 완료
- 빌드 성공 및 PIE 실기 테스트 완료

즉 현재 구현 기준으로는:

**입력 해석은 CarFight 커스텀 로직, 후진 전환은 Chaos Vehicle 기본 기능, 입력 충돌 방지는 Runtime Input Ownership 계층**


이 역할 분리가 맞다.

---

## 9. 완료 조건(Definition of Done)
아래가 모두 만족되면 이번 작업은 1차 완료로 본다.

- `Axis2D` 차량 이동 입력이 안정적으로 들어온다.
- 각도 기준계가 명세와 일치한다.
- 경계각 포함 규칙이 맞다.
- 검은 영역 유지 정책이 명세대로 동작한다.
- 전진 중 후진 영역은 브레이크다.
- `bUseAutoReverse = true` 환경에서 실제 후진 전환이 된다.
- 장치 모드별 차단 정책이 유지된다.
- 기존 `DriveComp` 전달 구조를 유지했다.
- 물리 키/스틱 배치를 코드에 하드코딩하지 않았다.
- `CF_VehicleMoveTest.md` 핵심 항목을 통과했다.

---

## 10. 작업 중 주의사항
- 기존 축 입력 경로를 바로 삭제하지 말 것
- 테스트 전에 디버그부터 붙일 것
- IMC 변경과 C++ 변경을 같은 세션에서 섞을 때 기준 문서를 다시 확인할 것
- 후진 구현 방식을 다시 바꿀 때 `bUseAutoReverse`와 충돌 여부를 먼저 검토할 것

---

## 11. 후속 작업 후보
- 입력 해석 설정값의 데이터화
- 입력 해석 결과 디버그 위젯화
- 카메라/전투 입력과의 충돌 점검
- 리플레이/네트워크 입력 기록 포맷 검토
- 검은 영역 후진 유지 정책 추가 튜닝

---

## 12. 변경 이력
- v0.2 (2026-04-16)
  - A안 검증 결과 반영.
  - 후진 전환을 Chaos Vehicle `bUseAutoReverse` 기준으로 정리.
  - 실제 PIE 후진 확인 결과를 작업 문서에 기록.
- v0.1 (2026-04-15)
  - 차량 이동 입력 구현에 바로 착수하기 위한 실행형 작업 문서 최초 작성.
