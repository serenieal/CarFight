# ProjectSSOT 운영 가이드 (CarFight)

## 목적
`Document/ProjectSSOT/`는 **CarFight 프로젝트 전용 기준 문서 묶음**을 보관한다.

이 폴더의 문서는 단순 운영 메모가 아니라,
**CarFight의 상위 비전과 현재 구현 기준을 함께 유지하기 위한 프로젝트 SSOT**다.

- 공통 SSOT 원본은 `Document/SSOT/`에 있다.
- 이 폴더는 CarFight에만 필요한 **방향 / 현재 상태 / 운영 규칙 / 검증 기준 / 결정 로그**를 기록한다.
- 공통 규칙은 복붙하지 않고 `Document/SSOT/`를 참조한다.

---

## 먼저 볼 문서
1. `03_VisionAlign.md`
2. `README.md`
3. `00_Handover.md`
4. `01_Roadmap.md`
5. `08_P0_Verification.md`

---

## 현재 구현 기준
- 현재 기준 플레이 차량: `BP_CFVehiclePawn`
- 현재 기준 Native Pawn: `ACFVehiclePawn`
- 현재 기준 주행 코어: `UCFVehicleDriveComp`
- 현재 기준 휠 시각 동기화 코어: `UCFWheelSyncComp`
- 현재 기준 데이터 축: `UCFVehicleData`
- 현재 기준 테스트 자산: `DA_PoliceCar`

이 기준은 **현재 실제 구현 기준선**이며,
CarFight의 상위 목표를 축소하거나 한 대의 차량으로 제한한다는 뜻이 아니다.

---

## 문서 묶음의 중심 방향
CarFight 문서는 아래 흐름을 따라야 한다.

1. **차량 전투와 물리 상호작용**이라는 상위 목표를 잊지 않는다.
2. **다양한 차량을 공통 코어 위에 올릴 수 있는 구조**를 지향한다.
3. 현재 구현은 `BP_CFVehiclePawn` / `ACFVehiclePawn` / `UCFVehicleDriveComp` / `UCFWheelSyncComp` / `UCFVehicleData` 조합을 기준으로 본다.
4. 문서는 현재 기준선과 장기 목표를 함께 설명해야 한다.

---

## 활성 핵심 문서
- `03_VisionAlign.md`
  - 상위 비전 / 현재 구현 / 다음 확장 방향
- `00_Handover.md`
  - 현재 상태 / 확정 기준 / 현재 리스크
- `01_Roadmap.md`
  - 현재 해야 할 일 / 우선순위 / 완료 조건
- `02_Conventions.md`
  - CarFight 프로젝트 특화 규칙 / 문서 운영 규칙
- `08_P0_Verification.md`
  - 현재 기준 기능 검증 / PASS-FAIL 판정

---

## 활성 보조 문서
- `07_TestMap_Notes.md`
  - 테스트맵 변경/검증 메모 템플릿
- `12_CPP_TransitionPlan.md`
  - 파일명은 유지하지만, 현재는 **차량 코어 구조 유지와 확장 기준 문서**로 사용한다.
- `16_CPP_DecisionLog.md`
  - 현재 차량 코어 구조와 운영 결정을 기록한다.
- `18_DriveState_Tuning.md`
  - VehicleData 기반 DriveState 튜닝 가이드
- `19_DriveState_CoreChecklist.md`
  - 공통 코어 검증 체크리스트
- `20_DocCleanup_Run.md`
  - 문서 정리 실행표 / 처리 기준

---

## 문서 작성 원칙
- 활성 문서는 현재 기준 자산 이름만 쓴다.
- 현재 기준선과 장기 목표를 혼동하지 않는다.
- 특정 테스트 차량을 최종 목표처럼 쓰지 않는다.
- 실존 경로를 우선 참조한다.
- 완료된 과도기 문서는 `Archive/` 하위로 이동한다.

---

## Archive 운영 원칙
완료된 작업 문서, 과도기 문서, 체크포인트 로그는 `Archive/` 하위로 이동한다.

### Archive 하위 분류
- `Archive/LegacyBP/`
  - 구형 BP 기준 절차 / 핫픽스 / 조사 기록
- `Archive/CPP/`
  - 완료된 C++ 전환 단계 문서 / 과도기 비교 기록
- `Archive/Checkpoints/`
  - 일자별 체크포인트 로그
- `Archive/Ops/`
  - 문서 정리 / 이동 실행 기록
