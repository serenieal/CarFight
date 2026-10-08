# CarFight C++ 중심 전환 계획서 v0.2 (정리본)

- 작성일: 2026-03-18
- 대상 프로젝트: `UE/CarFight_Re`
- 문서 상태: **정리본 / 종료 기준 포함**
- 문서 목적: CarFight 차량 구조를 **BP 중심 구조에서 C++ 중심 구조로 재기준화**한 결과를 정리하고, 이 문서를 어디까지 마무리하면 프로젝트 로드맵으로 넘어갈 수 있는지 명확히 고정한다.
- 기준 문서:
  - `Document/SSOT/UE_SSOT/UE_CPP_SSOT/UE_AI_Cpp_SSOT_v0_1.md`
  - `Document/SSOT/UE_SSOT/UE_CPP_SSOT/UE_BP2Cpp_Check_v0_1.md`
  - `Document/ProjectSSOT/12_CPP_TransitionPlan.md`
  - `Document/ProjectSSOT/16_CPP_DecisionLog.md`
  - `Document/ProjectSSOT/19_DriveState_CoreChecklist.md`

---

## 0. 이 문서의 현재 역할

이 문서는 더 이상 “처음부터 전환을 어떻게 시작할까”를 고민하는 초안이 아니다.

지금 이 문서의 역할은 아래 3가지다.

1. **CarFight 차량 구조의 현재 기준 구현을 고정한다.**
2. **C++ 전환 계획에서 이미 끝난 것 / 아직 남은 것을 구분한다.**
3. **이 문서를 어디까지 마무리하면 프로젝트 로드맵(`01_Roadmap.md`)으로 넘어갈 수 있는지 종료 기준을 정한다.**

즉, 지금부터 이 문서는 **전환 설계 초안**이 아니라 **전환 정리본 + 종료 문서**로 취급한다.

---

## 1. 현재 기준 구현 고정

### 1.1 현재 기준 구현
현재 CarFight 차량의 기준 구현은 아래로 고정한다.

- Native 기준 Pawn: `ACFVehiclePawn`
- Thin BP 기준 Pawn: `BP_CFVehiclePawn`
- 입력/주행 코어: `UCFVehicleDriveComp`
- 휠 시각 동기화 코어: `UCFWheelSyncComp`
- 차량 데이터 기준: `UCFVehicleData`
- DriveState 공용 설정 구조체: `FCFVehicleDriveStateConfig`

### 1.2 과거 기준 자산 처리
- `BP_ModularVehicle`는 현재 기준 구현이 아니다.
- `BP_ModularVehicle`는 이미 삭제되었으며, 과거 문서에 남아 있는 내용은 **역사 기록**으로만 본다.
- 따라서 이 문서에서 더 이상 `BP_ModularVehicle`를 현재 작업 대상으로 다루지 않는다.

### 1.3 구조 원칙 재확인
- **판단은 C++**
- **표현은 BP**
- **BP는 Thin BP 유지**
- **핵심 규칙/상태/전이는 Native 기준으로만 고정**

---

## 2. 이번 전환에서 실제로 끝난 것

아래 항목은 더 이상 계획 단계가 아니라, 현재 프로젝트에서 **이미 구조가 세워졌거나 1차 검증이 끝난 항목**으로 본다.

### 2.1 기준 자산 재고정 완료
- `ACFVehiclePawn` / `BP_CFVehiclePawn` / `UCFVehicleDriveComp` / `UCFWheelSyncComp` / `UCFVehicleData` 기준으로 재고정 완료
- `BP_ModularVehicle` 관련 현재형 설명 제거 완료

### 2.2 Thin BP 방향 고정 완료
- `BP_CFVehiclePawn`는 표현/조립용 Thin BP 기준으로 유지
- 핵심 상태 판단 로직을 BP에 다시 넣지 않는 방향 고정 완료

### 2.3 DriveState 코어 1차 도입 완료
`UCFVehicleDriveComp` 기준으로 아래가 들어가 있다.

- `ECFVehicleDriveState`
- `FCFVehicleInputState`
- `FCFVehicleDriveStateSnapshot`
- 입력 캐시
- 속도/방향 기반 상태 판정
- 상태 전이 추적
- 상태 전이 요약 문자열
- 히스테리시스
- 상태별 HoldTime
- Airborne 접촉 힌트 폴백
- DataAsset 기반 DriveState 설정 적용 경로

### 2.4 Pawn 조회 API 정리 완료
`ACFVehiclePawn` 기준으로 아래 조회 API가 존재한다.

- `GetVehicleSpeed()`
- `GetDriveState()`
- `GetDriveStateSnapshot()`

### 2.5 코어 검증 보조 경로 완료
- 온스크린 디버그 표시 경로 추가 완료
- `Document/ProjectSSOT/19_DriveState_CoreChecklist.md` 작성 완료
- 공통 코어 검증 단계에서는 `DriveStateConfig` override를 끄고 검증한다는 원칙 고정 완료

### 2.6 데이터 저장 위치 분리 시작 완료
- `UCFVehicleData`에 `DriveStateConfig` 슬롯 추가 완료
- 차량별 DriveState override 경로는 열어두되, 현재 단계에서는 기본적으로 사용 보류

---

## 3. 이번 전환에서 아직 남은 것

이 문서를 깔끔하게 마무리하려면, 남은 항목을 “계속 깊게 파야 하는 것”이 아니라 **정리/인계 관점**에서 봐야 한다.

### 3.1 WheelSync 마감 검증
`UCFWheelSyncComp`의 운영 경로 정리는 완료되었다.

반영 완료:
- DebugMode 기반으로 helper / helper compare / 관측용 상태를 개발용 영역으로 격리
- 레거시 단일축 필드를 `WheelSync|Legacy`로 격리
- `Phase1Stub` 제거 완료
- 현재 기준 Pawn / Wheel BP에서 BP 중복 적용 흔적 없음 확인
- 정리 후 빌드 성공 확인

남은 핵심:
- 실제 서스펜션 값 소스 연결 최종화
- 실제 스핀 값 소스 연결 최종화
- TestMap 기준 마감 검증 루틴 연결

### 3.2 Camera 판단 Native 분리
현재 카메라는 별도 Native 판단 컴포넌트가 없다.

남은 핵심:
- `UCFVehicleCameraComp` 설계 또는 후속 분리 계획 확정
- 카메라 상태 Enum 초안
- BP 리그와 Native 판단의 책임 분리

### 3.3 검증 루틴 고정
코어는 기본적으로 작동하지만, “누가 해도 같은 결과가 나오는가”까지는 아직 문서 기반 루틴으로 더 고정해야 한다.

남은 핵심:
- TestMap 기준 재현 절차 고정
- 통과 / 보류 / 실패 기록 방식 고정
- WheelSync 포함 공통 검증 루틴 연결

---

## 4. Phase별 현재 판정

| Phase | 원래 목표 | 현재 판정 | 메모 |
|---|---|---|---|
| Phase 0 | 사전 고정 | 완료 | 현재 기준 구현과 문서 기준 재고정 완료 |
| Phase 1 | 기존 BP 기능 분해 | 종료 처리 | 과거 BP 직접 분해보다, 현재 기준 구현 재정의가 우선 수행됨 |
| Phase 2 | 새 차량 아키텍처 설계 | 완료 | 현재 기준 구조와 책임표가 사실상 고정됨 |
| Phase 3 | 새 C++ Pawn 골격 생성 | 완료 | `ACFVehiclePawn`가 기준 Pawn 역할 수행 |
| Phase 4 | 입력 처리 C++ 이전 | 1차 완료 | 입력 진입점과 전달/해석 구조가 Native 기준으로 올라옴 |
| Phase 5 | 이동 상태/차량 규칙 이전 | 1차 완료 | DriveState / 히스테리시스 / 디버그 / 전이 추적까지 반영 |
| Phase 6 | 휠 동기화 C++ 정식화 | 마감 검증 단계 | 운영 경로 정리 완료, 남은 일은 실제 값 소스 연결과 검증 루틴 고정 |
| Phase 7 | 카메라 로직 분리 | 미완료 | 후속 Roadmap 이전 정리 필요 |
| Phase 8 | Thin BP 재작성 | 완료 | `BP_CFVehiclePawn` 기준으로 방향 고정 |
| Phase 9 | 데이터 소유권 정리 | 진행 중 | DriveStateConfig까지는 분리, 전체 차량 튜닝값 정리는 후속 |
| Phase 10 | 기존 BP 퇴역 | 완료 처리 | `BP_ModularVehicle`는 현재 기준 구현에서 제외 및 삭제 |
| Phase 11 | 문서/운영 동기화 | 완료 | 현재 정리본 기준으로 마감, 남은 일은 Roadmap 인계 항목으로 분리 완료 |

---

## 5. 이 문서를 닫는 기준

`CarFight_CPP_RePlan.md`는 아래 조건을 만족하면 **마무리된 문서**로 본다.

### 종료 조건 A — 기준 구현이 흔들리지 않는다
- 현재 기준 구현이 `ACFVehiclePawn` / `BP_CFVehiclePawn` / `UCFVehicleDriveComp` / `UCFWheelSyncComp` / `UCFVehicleData`로 고정되어 있다.
- 삭제된 과거 BP가 현재 작업 기준으로 다시 등장하지 않는다.

### 종료 조건 B — 운전 코어는 “더 파기보다 넘어가도 되는 수준”이다
- 차량이 기본적으로 잘 움직인다.
- DriveState 코어는 공통 기본값 기준으로 1차 동작 확인이 끝났다.
- 지금 단계의 우선순위가 차종별 감각 튜닝이 아님이 문서에 반영되어 있다.

### 종료 조건 C — 남은 일은 계획 초안이 아니라 인계 항목으로 표현된다
- WheelSync 잔여 작업이 별도 작업 항목으로 표현된다.
- Camera Native 분리 항목이 후속 작업으로 분리된다.
- 검증 루틴 문서가 존재한다.

### 종료 조건 D — 프로젝트 로드맵으로 넘길 목록이 분명하다
- 이 문서에서 더 깊게 파지 않을 항목과, `01_Roadmap.md`로 넘길 항목이 명확히 구분된다.

### 현재 판정 — 종료 조건 충족
- 종료 조건 A ~ D를 현재 정리본 기준으로 충족한 것으로 본다.
- 따라서 `CarFight_CPP_RePlan.md`는 **계획 진행 문서가 아니라 마감된 정리 문서**로 취급한다.
- 이후 남은 작업 추적은 `01_Roadmap.md` 기준으로 관리한다.

---

## 6. 이 문서에서 Project Roadmap으로 넘길 작업

이 문서를 닫고 나면, 다음 우선순위는 `01_Roadmap.md`에서 관리한다.

### Roadmap으로 넘길 1순위
- `UCFWheelSyncComp` 실제 서스펜션 / 스핀 값 소스 최종 연결
- TestMap 기준 WheelSync 마감 검증 루틴 고정

### Roadmap으로 넘길 2순위
- TestMap 기준 공통 코어 검증 루틴 고정
- 재현 가능한 체크리스트와 PASS/FAIL 기록 방식 연결

### Roadmap으로 넘길 3순위
- Camera Lag / Collision Filtering / 후속 P1 안정화 항목
- Camera Native 분리 설계

### 지금 넘기지 않을 것
- 차종별 감각 튜닝
- 경찰차/기타 차량별 DriveState 수치 세부 조정
- 코어 검증 전에 차종 개성을 먼저 잡는 작업

---

## 7. 현재 단계 운영 원칙

### 7.1 지금은 차종 튜닝 단계가 아니다
- `DriveStateConfig` override는 구조상 열려 있지만,
- 현재 단계에서는 기본적으로 `bUseDriveStateOverrides = false` 기준으로 검증한다.

### 7.2 지금은 코어 안정화 단계다
- 공통 주행 코어가 안정적인지 먼저 본다.
- 차종별 세부값은 코어 검증 종료 후 다시 연다.

### 7.3 지금은 문서도 같이 닫아야 한다
이 문서를 닫는다는 것은 단순히 파일 하나를 끝내는 것이 아니라,
- 현재 기준 구현 고정
- 남은 일 인계
- 후속 로드맵 연결
까지 끝내는 것을 뜻한다.

---

## 8. 최종 판단

이번 CarFight C++ 전환은 이제 더 이상 “BP_ModularVehicle를 기준으로 새 구조를 구상하는 단계”가 아니다.

현재 단계의 본질은 아래와 같다.

- 현재 기준 구현은 이미 `ACFVehiclePawn` / `BP_CFVehiclePawn` 중심으로 재고정되었다.
- DriveState와 주행 코어는 1차 동작 수준까지 올라왔다.
- Thin BP 방향도 고정되었다.
- 남은 것은 구조 구상이 아니라, WheelSync / Camera / 검증 루틴의 **정리와 인계**다.

따라서 `CarFight_CPP_RePlan.md`는 이 정리본 기준으로 **종료 처리**하고,
이후의 실제 우선순위 관리는 **프로젝트 로드맵(`01_Roadmap.md`)** 으로 넘기는 것으로 확정한다.
