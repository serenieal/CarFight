# CarFight — 00_Handover

> 역할: CarFight 프로젝트의 **현재 상태 / 확정 기준 / 현재 리스크**를 고정한다.  
> 공통 규칙 원본: `Document/SSOT/`  
> 마지막 정리(Asia/Seoul): 2026-03-20

---

## 먼저 확인할 방향
- CarFight의 상위 방향은 `03_VisionAlign.md`를 기준으로 본다.
- 이 문서는 그 방향 아래에서 **지금 실제로 굴러가는 기준선**을 고정한다.

---

## 현재 구현 기준
- 현재 기준 플레이 차량: `BP_CFVehiclePawn`
- 현재 기준 Native Pawn: `ACFVehiclePawn`
- 현재 기준 주행 코어: `UCFVehicleDriveComp`
- 현재 기준 휠 시각 동기화 코어: `UCFWheelSyncComp`
- 현재 기준 데이터 축: `UCFVehicleData`
- 현재 기준 테스트 자산: `DA_PoliceCar`

---

## 현재 상태 요약
### 입력 / 주행
- Enhanced Input 기반 기본 조작은 현재 기준 구조에서 동작한다.
- `ACFVehiclePawn` + `UCFVehicleDriveComp` 조합으로 기본 주행이 가능한 상태다.

### DriveState 코어
- DriveState는 현재 활성 개발 축이다.
- 현재 목표는 단순 유지가 아니라, **다차종 확장을 견딜 수 있는 공통 상태 판단 코어 확보**다.
- 관련 활성 문서:
  - `18_DriveState_Tuning.md`
  - `19_DriveState_CoreChecklist.md`
  - `16_CPP_DecisionLog.md`

### Wheel Sync
- 휠 시각 동기화의 현재 기준 코어는 `UCFWheelSyncComp`다.
- 목표는 한 차량 전용 임시 처리 정리가 아니라, **공통 차량 코어 위에서 재사용 가능한 시각 동기화 기준선 확보**다.

### VehicleData
- `UCFVehicleData`는 차량별 차이를 담는 데이터 축이다.
- 현재 테스트 기준은 `DA_PoliceCar`지만, 장기 목표는 여러 차량을 같은 구조 위에 올릴 수 있게 만드는 것이다.

---

## 확정 기준
### 구현 기준
- 판단 / 상태 / 규칙은 C++ 코어를 우선 기준으로 둔다.
- `BP_CFVehiclePawn`는 Thin BP로 유지한다.
- 새 상태 판정이나 복잡 규칙을 BP에 다시 키우지 않는다.

### 문서 기준
- 활성 문서는 현재 기준 자산 이름만 사용한다.
- 현재 기준선과 장기 목표를 함께 설명한다.
- 특정 테스트 차량을 최종 목표처럼 쓰지 않는다.
- 완료된 과도기 문서는 `Archive/` 하위로 이동한다.

### 검증 기준
- 최종 PASS/FAIL 판정은 `08_P0_Verification.md`를 기준으로 한다.
- DriveState 공통 코어 검증은 `19_DriveState_CoreChecklist.md`를 기준으로 한다.
- 차량별 튜닝은 공통 코어 기준이 흔들리지 않는 범위에서 진행한다.

---

## 현재 리스크
- DriveState 코어가 특정 테스트 차량 기준값에 너무 묶이면 다차종 확장성이 떨어질 수 있다.
- Wheel Sync가 현재 차량 기준 임시 처리 위주로 굳어지면 공통 규칙으로 확장하기 어려워질 수 있다.
- 활성 문서가 구현 세부만 남기고 상위 방향을 잊으면, 문서가 다시 축소 해석될 수 있다.

---

## 후속 세션에서 먼저 볼 순서
1. `03_VisionAlign.md`
2. `README.md`
3. `00_Handover.md`
4. `01_Roadmap.md`
5. `08_P0_Verification.md`

---

## Archive 처리 원칙
아래 부류는 활성 기준이 아니라 역사 기록으로 본다.
- 완료된 C++ 전환 단계 문서
- helper/debug pipe 비교 기록
- 단일 축 전환 테스트 기록
- 일자별 체크포인트 로그
