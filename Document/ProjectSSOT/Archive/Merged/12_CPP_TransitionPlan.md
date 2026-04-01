# CarFight — 12_CorePlan

> 파일명은 기존 연속성을 위해 유지하지만, 이 문서의 현재 역할은 **공통 차량 코어 구조와 확장 기준**을 정리하는 것이다.  
> 상위 방향 문서: `03_VisionAlign.md`  
> 마지막 정리(Asia/Seoul): 2026-03-20

---

## 문서 역할
이 문서는 더 이상 전환 계획 문서가 아니다.

현재 역할은 아래와 같다.
- 현재 차량 코어 구조를 공통 기준선으로 정리한다.
- 어떤 부분을 공통 규칙으로 유지할지 정한다.
- 어떤 부분을 차량별 데이터로 열어둘지 정한다.
- 이후 다차종 차량 추가가 가능하도록 기준을 고정한다.

---

## 현재 공통 차량 코어
### Native 중심
- `ACFVehiclePawn`
- `UCFVehicleDriveComp`
- `UCFWheelSyncComp`
- `UCFVehicleData`

### Thin BP
- `BP_CFVehiclePawn`
  - 시각 / 조립 / 반응 계층 유지
  - 복잡한 상태 판단 / 규칙 / 전이 로직은 두지 않음

---

## 공통 코어에서 고정할 것
### 1) 판단은 C++ 코어
- DriveState 상태 판정
- 상태 전이 규칙
- Wheel Sync 기준 해석
- VehicleData 적용 해석

### 2) BP는 표현 계층
- 조립
- 시각 반응
- 디버그 보조 노출
- 후속 연출 연결 지점

### 3) 데이터는 VehicleData 축으로 분리
- 공통 규칙은 코드에 둔다.
- 차량별 차이는 `UCFVehicleData`에 둔다.
- 테스트 차량은 `DA_PoliceCar`를 기준으로 시작한다.

---

## 차량별로 열어둘 것
아래 항목은 차량별 차이를 허용한다.
- DriveStateConfig 시작값
- 차량별 감각 차이
- 향후 카메라 반응 계수
- 향후 충돌/전투 반응 계수

아래 항목은 먼저 공통 규칙으로 유지한다.
- 기본 상태 전이 구조
- 반대 입력 처리 원칙
- 공중 상태 판정의 큰 방향
- Wheel Sync 적용 구조

---

## 현재 단계의 핵심 질문
현재 단계에서는 아래 질문만 본다.

1. 이 구조가 다른 차량에도 옮겨갈 수 있는가?
2. 공통 코어와 차량별 튜닝이 문서상 분리되어 있는가?
3. 현재 기준 테스트 차량이 임시 종착점처럼 굳어지지 않았는가?

---

## 지금 해야 할 일
### Priority A — DriveState 공통 기준 확정
- 공통 기본값 기준으로 상태 판정이 안정적인지 확인
- 차량별 override는 코어 통과 뒤 연다

### Priority B — Wheel Sync 공통 기준 확정
- 특정 테스트 상황이 아니라 공통 차량 구조에서 읽히는지 확인
- 과도기 문서는 Archive로 정리

### Priority C — VehicleData 분화 준비
- 공통값과 차량별 값을 구분
- 다음 차량 추가 절차를 문서화

---

## 하지 말 것
- 테스트 차량을 최종 기준 차량처럼 취급하기
- 상태 판정 로직을 BP에 다시 키우기
- 코어 문제를 차량별 튜닝으로 덮어버리기
- 완료된 전환 문서를 활성 기준 문서처럼 계속 유지하기

---

## 관련 활성 문서
- `03_VisionAlign.md`
- `00_Handover.md`
- `01_Roadmap.md`
- `08_P0_Verification.md`
- `16_CPP_DecisionLog.md`
- `18_DriveState_Tuning.md`
- `19_DriveState_CoreChecklist.md`
