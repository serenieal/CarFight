# ProjectSSOT 운영 가이드 (CarFight)

> 문서 버전: v1.1.0
> 마지막 정리(Asia/Seoul): 2026-04-15


## 목적
`Document/ProjectSSOT/`는 CarFight의 **상위 방향 / 현재 기준선 / 작업 순서 / 검증 기준 / 결정 로그**를 유지하는 프로젝트 전용 SSOT다.

이 폴더의 목적은 문서를 많이 만드는 것이 아니라,
**지금 팀이 무엇을 목표로 보고 있고, 현재 어디에 서 있으며, 다음에 무엇을 해야 하는지**를 흔들리지 않게 고정하는 것이다.

공통 규칙 원본은 `Document/SSOT/`에 둔다.
여기에는 CarFight에만 필요한 판단만 남긴다.

---

## 현재 활성 문서 최소 집합
앞으로 루트에서 바로 보는 문서는 아래 6개로 줄인다.

1. `README.md`
   - ProjectSSOT 입구 문서
   - 문서 역할 분리와 운영 규칙
2. `03_VisionAlign.md`
   - 최종 방향
   - 현재 기준선
   - 현재 구조 갭
   - 유지할 것 / 교체할 것
3. `00_Handover.md`
   - 지금 실제로 굴러가는 기준선
   - 현재 리스크
   - 임시 운영 편차
4. `01_Roadmap.md`
   - 문서 정렬 이후 실제 작업 순서
   - 완료 조건
5. `08_P0_Verification.md`
   - 현재 기준선 검증
   - DriveState 코어 체크
   - 첫 차량 튜닝 기준
6. `16_CPP_DecisionLog.md`
   - 현재 구조 유지 / 교체 판단 기록

---

## 먼저 읽는 순서
1. `03_VisionAlign.md`
2. `00_Handover.md`
3. `01_Roadmap.md`
4. `08_P0_Verification.md`
5. `16_CPP_DecisionLog.md`

---

## 지금 문서가 고정해야 하는 핵심 문장
아래 4문장은 활성 문서들에서 서로 충돌하면 안 된다.

### 1. 최종 방향
- CarFight의 최종 차량 구조 목표는 `CMVS / Cluster Union / Geometry Collection` 기반의 다차종 차량 전투 구조다.

### 2. 현재 기준선
- 현재 실제 구현 기준선은 `ACFVehiclePawn + UCFVehicleDriveComp + UCFWheelSyncComp + UCFVehicleData + DA_PoliceCar` 조합이다.
- 현재 차량 휠 배치는 `Wheel_Anchor_*` 수동 배치 + `WheelRadius` 기준선을 사용한다.
- `AutoFit / VehicleLayoutConfig / WheelLayout` 레거시 구조는 현재 코드 기준에서 하드 삭제됐다.

### 3. 유지 예정 코어
- `VehicleData`
- `DriveState`
- `WheelSync`
- Thin BP 원칙

### 4. 교체 후보
- Vehicle root 구조
- 차량 조립 방식
- 파괴 / 분리 표현 구조

---

## 문서 운영 규칙
### 1. 최종 방향과 현재 기준선을 섞지 않는다
- 최종 방향은 `03_VisionAlign.md`에서 고정한다.
- 현재 실측 상태는 `00_Handover.md`에서만 확정한다.
- 작업 순서는 `01_Roadmap.md`에서 관리한다.

### 2. 임시 운영 편차와 구조 갭을 구분한다
- 예:
  - `GamepadOnly`는 임시 운영 편차다.
  - `CMVS 미도입`은 구조 갭이다.
- 이 둘을 같은 수준의 문제로 적지 않는다.

### 3. 특정 테스트 차량을 최종 목표처럼 쓰지 않는다
- `DA_PoliceCar`는 현재 기준선이다.
- 최종 목표 차량이 아니다.

### 4. 활성 문서는 적게 유지한다
- 내용이 겹치면 새 문서를 만들지 않고 기존 활성 문서에 흡수한다.
- 역할이 끝난 문서는 루트에 남기지 않고 `Archive/`로 이동한다.

---

## Archive 운영 원칙
루트 문서는 최소 집합만 남기고, 나머지는 `Archive/`로 내린다.

### Archive 하위 분류
- `Archive/LegacyBP/`
  - 구형 BP 기준 절차 / 핫픽스 / 조사 기록
- `Archive/CPP/`
  - 완료된 C++ 전환 단계 문서 / 과도기 비교 기록
- `Archive/Checkpoints/`
  - 일자별 체크포인트 로그
- `Archive/Ops/`
  - 문서 정리 / 이동 실행 기록
- `Archive/Merged/`
  - 활성 문서에 흡수되어 루트에서 내려간 문서

---

## 이번 정리의 결과
이번 정리 이후에는 아래 문서를 루트 기준에서 비활성으로 본다.
- `02_Conventions.md`
- `07_TestMap_Notes.md`
- `12_CPP_TransitionPlan.md`
- `18_DriveState_Tuning.md`
- `19_DriveState_CoreChecklist.md`
- `20_DocCleanup_Run.md`
- `21_Archive_Index.md`
- `22_DirectionGap.md`
- `ProjectSSOT_OpMap.md`

이 문서들은 삭제가 아니라 **흡수 또는 Archive 이동**으로 처리한다.

---

## 보조 실행 계획 문서 (2026-04-15 추가)
루트 활성 문서 최소 집합은 그대로 유지하되, 현재 진행 중인 카메라 작업은 아래 보조 실행 계획 문서를 함께 본다.

- `Document/ProjectSSOT/Plan/CameraPlan/CF_CameraPlan_260410.md`
  - 차량 카메라 기획 회의 정리
- `Document/ProjectSSOT/Plan/CameraPlan/CF_CameraSysSpec_260410.md`
  - 차량 카메라 시스템 세부 설계안
- `Document/ProjectSSOT/Plan/CameraPlan/CF_CamChecklist_260415.md`
  - 차량 카메라 개발 체크리스트 및 현재 구현 상태

이 문서군은 루트 활성 문서를 대체하지 않는다.
즉, 상위 방향 / 현재 기준선 / 우선순위는 계속 루트 활성 문서에서 판단하고,
카메라 작업의 상세 기준과 구현 순서는 `Plan/CameraPlan/` 문서군에서 본다.

---

## 변경 이력
- v1.1.0 (2026-04-15)
  - 문서 버전 / 마지막 정리 날짜를 추가했다.
  - `Plan/CameraPlan/` 문서군을 현재 보조 실행 계획 문서로 연결했다.

