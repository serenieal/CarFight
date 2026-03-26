# CarFight — 02_Conventions

> 역할: CarFight 프로젝트에서 사용하는 **프로젝트 특화 규칙 / 문서 운영 규칙**만 기록한다.  
> 공통 규칙 원본: `Document/SSOT/`  
> 마지막 정리(Asia/Seoul): 2026-03-20

---

## 1. 문서 역할 분리
### 공통 SSOT
- `Document/SSOT/`는 공통 규칙 원본이다.
- 공통 UE 규칙 / 공통 MCP 절차 / 범용 작성 규칙은 이쪽을 참조한다.

### 프로젝트 SSOT
- `Document/ProjectSSOT/`는 CarFight 전용 기준 문서다.
- CarFight의 방향, 현재 기준 자산, 현재 상태, 현재 검증, 현재 결정을 기록한다.

---

## 2. 현재 기준 규칙
- 현재 기준 플레이 차량: `BP_CFVehiclePawn`
- 현재 기준 Native Pawn: `ACFVehiclePawn`
- 현재 기준 주행 코어: `UCFVehicleDriveComp`
- 현재 기준 휠 시각 동기화 코어: `UCFWheelSyncComp`
- 현재 기준 데이터 축: `UCFVehicleData`
- 현재 기준 테스트 자산: `DA_PoliceCar`

---

## 3. 상위 방향 규칙
- 상위 방향은 `03_VisionAlign.md`를 우선 기준으로 둔다.
- 활성 문서는 현재 구현만 적더라도, 그것이 CarFight의 상위 목표와 어떻게 연결되는지 드러내야 한다.
- 특정 테스트 차량을 프로젝트의 최종 목표처럼 쓰지 않는다.
- 다차종 확장 가능성을 해치는 표현을 피한다.

---

## 4. 구현 규칙
- 판단 / 상태 / 규칙은 C++ 코어를 우선 기준으로 둔다.
- `BP_CFVehiclePawn`는 Thin BP로 유지한다.
- 새 상태 판정 / 복잡 분기 / 중복 규칙을 BP에 다시 키우지 않는다.
- 공통 코어와 차량별 차이를 문서에서 구분한다.

---

## 5. 문서 작성 규칙
- 활성 문서는 현재 기준 자산 이름만 사용한다.
- 실존 경로를 우선 참조한다.
- 현재 기준선과 장기 목표를 혼동하지 않는다.
- 완료된 과도기 문서는 `Archive/` 하위로 이동한다.
- 역사 기록은 남겨도 되지만, 활성 문서 안에서 현재 기준처럼 보이게 두지 않는다.

### 활성 문서 우선순위
- 상위 방향: `03_VisionAlign.md`
- 현재 상태 / 확정 기준: `00_Handover.md`
- 해야 할 일 / 우선순위: `01_Roadmap.md`
- 검증 판정: `08_P0_Verification.md`
- 결정 근거: `16_CPP_DecisionLog.md`

---

## 6. Archive 규칙
### LegacyBP
- 구형 BP 기준 절차 / 핫픽스 / 조사 기록

### CPP
- 완료된 C++ 전환 단계 문서 / 과도기 비교 기록

### Checkpoints
- 일자별 점검 로그

### Ops
- 문서 정리 / 이동 실행 기록

---

## 7. 변경 시 반영 순서
1. `03_VisionAlign.md`
2. `01_Roadmap.md`
3. `00_Handover.md`
4. `08_P0_Verification.md`
5. `02_Conventions.md`
6. 필요 시 `16_CPP_DecisionLog.md`
7. 완료 문서 Archive 이동 검토
