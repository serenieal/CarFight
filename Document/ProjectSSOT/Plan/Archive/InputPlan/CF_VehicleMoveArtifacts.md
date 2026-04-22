# CF_VehicleMoveArtifacts

## 문서 목적
본 문서는 `CF_VehicleMoveSpec.md`, `CF_VehicleMoveTasks.md`, `CF_VehicleMoveWork.md`, `CF_VehicleMoveCpp.md`, `CF_VehicleMoveRegression.md` 기준으로 **이번 차량 신규 조작 작업의 산출물**을 정리한 문서다.

이 문서의 목적은 아래와 같다.

- 실제로 무엇이 바뀌었는지 한 번에 확인하기
- IMC 변경과 C++ 변경을 분리해서 기록하기
- 빌드 / PIE / 회귀 검증 문서 연결점을 남기기
- 아직 남아 있는 이슈를 분리해서 다음 단계로 넘기기

---

## 1. 산출물 요약
현재 기준 산출물은 아래처럼 정리된다.

### 1-1. 코드 산출물
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

### 1-2. 계획 / 설계 / 구현 문서 산출물
- `CF_VehicleMoveSpec.md`
- `CF_VehicleMoveDesign.md`
- `CF_VehicleMoveTasks.md`
- `CF_VehicleMoveWork.md`
- `CF_VehicleMoveCpp.md`
- `CF_VehicleMoveChecklist.md`
- `CF_VehicleMoveTest.md`
- `CF_VehicleMoveRegression.md`
- `README.md`

### 1-3. 실행 결과 산출물
- 빌드 성공 확인
- PIE 실기 테스트 확인
- 회귀 검증 범위 문서화 완료

---

## 2. C++ 변경 요약

### 2-1. 신규 조작 핵심 구현
아래 기능이 `ACFVehiclePawn` 기준으로 반영되었다.

- `IA_VehicleMove` 기반 2D 입력 해석
- 각도 / 강도 계산
- 쓰로틀 / 후진 / 검은 영역 판정
- 검은 영역 유지 정책
- 후진 영역 브레이크 처리
- Chaos Vehicle `bUseAutoReverse` 기반 후진 전환

### 2-2. 입력 충돌 방지(P1)
후속 안정화 단계에서 아래가 반영되었다.

- `VehicleMove2D / LegacyAxis / None` 입력 소유권 enum
- `CurrentInputOwnership`
- `InputOwnershipHoldTimeSec`
- `LastVehicleMoveInputTimeSec`
- `LastLegacyAxisInputTimeSec`
- `VehicleMove` 입력 소유권 획득 로직
- 기존 축 입력 게이트
- release 경로 덮어쓰기 방지
- `InputOwner` 디버그

### 2-3. 제거된 방식
아래 방식은 현재 기준 사용하지 않는다.

- 수동 기어 강제 (`SetTargetGear`)
- 음수 스로틀 기반 reverse 강제

---

## 3. IMC 변경 요약

### 3-1. 운영 기준
현재 입력 자산 운영 기준은 아래다.

- 게임패드: `IA_VehicleMove`
- 키보드: `IA_Throttle / IA_Brake / IA_Steering`

### 3-2. 적용 방식
이 분리는 코드 하드코딩이 아니라 **IMC 분리**로 적용했다.

### 3-3. 확인 결과
아래를 실제로 확인했다.

- 게임패드 신규 조작 정상 동작
- 키보드 기존 조작 정상 동작
- IMC 분리 후 상호 충돌 없이 동작
- `Auto` 상태에서 입력 소유권 정책 기준으로 전환 동작 확인

---

## 4. 빌드 / 테스트 요약

### 4-1. 빌드
- `CarFight_ReEditor / Win64 / Development` 빌드 성공 확인
- P1 입력 충돌 방지 적용 후 빌드 성공 확인

### 4-2. PIE 실기 테스트
아래를 실제 플레이 기준으로 확인했다.

- 전진 동작
- 좌 / 우 조향 동작
- 뒤 입력 시 브레이크 후 후진 전환
- 게임패드 신규 조작 동작
- 키보드 기존 조작 동작
- 입력 충돌 방지 동작
- `InputOwner` 디버그 확인

### 4-3. 회귀 검증 문서
회귀 검증 범위와 실행 순서는 아래 문서에 정리했다.

- `CF_VehicleMoveRegression.md`

---

## 5. 관련 문서 연결

### 명세
- `CF_VehicleMoveSpec.md`

### 설계
- `CF_VehicleMoveDesign.md`

### 테스트 기준
- `CF_VehicleMoveTest.md`
- `CF_VehicleMoveRegression.md`

### 실행 / 구현
- `CF_VehicleMoveTasks.md`
- `CF_VehicleMoveWork.md`
- `CF_VehicleMoveCpp.md`

### 진행 상태
- `CF_VehicleMoveChecklist.md`

---

## 6. 남은 이슈 목록
현재 기준 남은 이슈는 아래다.

### 6-1. 회귀 검증 결과표 미작성
회귀 검증 범위 문서는 만들었지만, 실제 결과표는 아직 비어 있다.

### 6-2. PIE 캡처 / 로그 미정리
실기 확인은 했지만, 별도 캡처나 결과 로그 파일은 아직 묶어두지 않았다.

### 6-3. 검은 영역 후진 유지 체감 튜닝
기능은 정상이나, 체감 최적화는 아직 후속 검토 대상이다.

### 6-4. DataAsset / Config화 미진행
입력 해석 설정값은 아직 코드/프로퍼티 중심이고, 별도 데이터화는 하지 않았다.

---

## 7. 현재 완료 판정
현재 산출물 기준으로는 아래처럼 정리할 수 있다.

- 핵심 기능 구현: 완료
- 입력 충돌 방지(P1): 완료
- 문서 기준선 정리: 완료
- 회귀 검증 범위 정리: 완료
- 회귀 검증 실제 결과표 작성: 미완료
- PIE 증빙 캡처 / 로그 정리: 미완료
- 체감 튜닝: 미완료

---

## 8. 다음 단계 권장 순서
1. 회귀 검증 실제 결과표 작성
2. PIE 캡처 또는 로그 정리
3. 남은 이슈 목록 후속 분기
4. 검은 영역 체감 튜닝 여부 판단

---

## 9. 변경 이력
- v0.1 (2026-04-17)
  - 차량 신규 조작 작업의 코드/문서/IMC/테스트 산출물을 한 번에 묶어 정리한 문서 최초 작성.
