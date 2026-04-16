# CF_VehicleMoveTasks

## 문서 목적
본 문서는 `CF_VehicleMoveSpec.md`와 `CF_VehicleMoveDesign.md` 기준으로 실제 구현 작업 순서와 완료 조건을 정리한 실행 문서다.

이번 작업의 목표는 아래 한 줄이다.

**기존 `Throttle / Steering / Brake` 전달 구조를 유지한 채, `Axis2D` 기반 차량 이동 입력 해석 계층을 추가하고, 후진 전환은 Chaos Vehicle `bUseAutoReverse`에 맡긴다.**

---

## 1. 작업 범위 요약

### 포함
- 차량 이동용 `Axis2D` 입력 해석 도입
- 각도/반지름 기반 영역 판정
- 검은 영역 유지 정책 도입
- 전진 중 후진 영역 = 브레이크 규칙 도입
- Chaos Vehicle `bUseAutoReverse` 기반 후진 전환 확인
- 디버그 출력 확장

### 제외
- 카메라 입력 재설계
- 핸드브레이크 배치 고정
- 물리 키/스틱 배치 고정
- IMC 데드존 정책 변경 강제
- DriveComp 전체 구조 개편
- 수동 기어 강제 기반 후진 구현

---

## 2. 추천 작업 폴더/파일

### C++ 우선 수정 후보
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

### 검토 후보
- `UE/Source/CarFight_Re/Public/CFVehicleDriveComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleDriveComp.cpp`

### 입력 에셋 후보
- 차량 이동용 `Axis2D` Input Action
- 차량용 IMC 매핑 항목

---

## 3. 작업 단계

### Phase 1. 입력 자산 준비
**목표**
차량 이동용 `Axis2D` 입력 액션과 IMC 경로를 준비한다.

**해야 할 일**
- [x] 차량 이동용 2D Input Action 정의
- [x] 키보드 전/후진/좌우를 해당 액션에 연결
- [x] 게임패드 이동 스틱을 해당 액션에 연결
- [x] 기존 단일 축 입력 자산과 충돌 여부 확인

### Phase 2. Pawn 입력 상태/설정 추가
**목표**
입력 해석에 필요한 설정값과 상태값을 Pawn에 추가한다.

**해야 할 일**
- [x] 입력 해석 설정값 추가
- [x] 최근 방향 의도 상태값 추가
- [x] 마지막 해석 결과 스냅샷 추가
- [x] 디버그용 요약 문자열 준비

### Phase 3. 2D 입력 해석 함수 구현
**목표**
입력 벡터를 차량 의미값으로 변환하는 해석 함수를 구현한다.

**해야 할 일**
- [x] `Magnitude` 계산
- [x] `AngleDeg` 계산
- [x] 쓰로틀/후진/검은 영역 판정 구현
- [x] 경계각 포함 비교 구현
- [x] 검은 영역 `HoldDirection_UseCurrentMagnitude` 구현
- [x] 최근 방향 의도 갱신 규칙 구현

### Phase 4. 기존 차량 적용 API 연결
**목표**
해석 결과를 기존 Drive 전달 구조에 연결한다.

**해야 할 일**
- [x] `Steering` -> 기존 조향 전달 함수 연결
- [x] `Throttle` -> 기존 스로틀 전달 함수 연결
- [x] `Brake` -> 기존 브레이크 전달 함수 연결
- [x] 후진 의도 처리 방식 확정
- [x] 전진 중 후진 영역 = 브레이크 동작 검증

**현재 확정 방향**
- 후진 의도는 입력 해석 계층에서 유지한다.
- 실제 reverse 전환은 Chaos Vehicle `bUseAutoReverse`가 담당한다.
- 수동 기어 강제와 음수 스로틀 기반 reverse 강제는 제거한다.

### Phase 5. 디버그/검증
**목표**
PIE에서 명세와 구현이 일치하는지 검증한다.

**해야 할 일**
- [x] RawInput / Angle / Zone / Intent / Output 디버그 추가
- [x] `CF_VehicleMoveTest.md` 기준 핵심 테스트 수행
- [x] 경계각 포함 검증
- [x] `bUseAutoReverse` 기반 후진 전환 검증
- [x] 장치 모드 차단 검증

---

## 4. 구현 중 의사결정 기준

### 4-1. 구조 변경 최소화
- 기존 구조를 유지하면서 기능을 얹는 방향을 우선한다.
- 새로운 구조를 도입하더라도 기존 함수 시그니처를 불필요하게 바꾸지 않는다.

### 4-2. 하드코딩 금지
- 좌스틱/우스틱 고정 금지
- 물리 키 이름 고정 금지
- 버튼 배치 고정 금지
- 수동 기어 강제 금지

### 4-3. 디버그 우선
- 체감형 기능이므로 구현 즉시 해석 결과를 시각적으로 확인할 수 있어야 한다.
- 후진 여부는 `Throttle < 0` 대신 `DriveState`와 실제 차량 이동으로 확인한다.

---

## 5. 완료 체크리스트

### 명세 일치
- [x] 각도 기준계가 문서와 같다.
- [x] 경계각 포함 규칙이 맞다.
- [x] 검은 영역 유지 정책이 맞다.
- [x] 전진 중 후진 영역은 브레이크다.
- [x] 후진 전환은 Chaos Vehicle `bUseAutoReverse` 기준으로 동작한다.

### 구조 일치
- [x] 기존 `Throttle / Steering / Brake` 적용 API를 유지했다.
- [x] 키/스틱 배치를 코드에 박지 않았다.
- [x] 장치 모드별 차단 정책이 유지된다.
- [x] 수동 기어 강제를 제거했다.

### 검증 일치
- [x] `CF_VehicleMoveTest.md` 기준 핵심 테스트를 통과했다.
- [x] 실제 PIE에서 후진 동작을 확인했다.
- [x] DriveState 회귀 문제가 크지 않다.

---

## 6. 작업 후 권장 산출물
- 수정된 C++ 파일 changelog
- IMC 변경 요약
- PIE 검증 캡처 또는 테스트 로그
- 남은 이슈 목록
- 차후 데이터화 후보 목록

---

## 7. 남은 후속 후보
- 입력 해석 설정의 DataAsset화 또는 Config화
- 차량 입력 해석 디버그 위젯 추가
- AI/리플레이/네트워크 관점 입력 기록 형식 정리
- 카메라/핸드브레이크/전투 입력과의 충돌 점검
- 검은 영역에서의 후진 유지 정책 추가 튜닝 검토

---

## 8. 변경 이력
- v0.2 (2026-04-16)
  - A안 검증 결과 반영.
  - 후진 전환을 Chaos Vehicle `bUseAutoReverse` 중심으로 정리.
  - 수동 기어 강제 및 음수 스로틀 기반 reverse 강제를 제거한 방향으로 체크리스트 갱신.
- v0.1 (2026-04-15)
  - 차량 입력 해석 변경안의 단계별 작업 문서 최초 작성.
