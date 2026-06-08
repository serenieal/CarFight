# ProjectSSOT 운영 가이드 (CarFight)

> 문서 버전: v2.0.0  
> 마지막 정리(Asia/Seoul): 2026-06-02  
> 문서 상태: Active  
> 역할: `Document/ProjectSSOT/`의 읽기 순서, 문서 역할, 생명주기를 고정한다.

---

## 1. 목적

`Document/ProjectSSOT/`는 CarFight의 **프로젝트 전용 판단 기준**을 유지하는 SSOT다.

이 폴더의 목적은 문서를 많이 모으는 것이 아니라,
아래 질문에 흔들리지 않고 답할 수 있게 하는 것이다.

```text
- 이 프로젝트가 최종적으로 어디로 가는가?
- 지금 실제로 어디까지 구현되어 있는가?
- 다음에 무엇을 해야 하는가?
- 어떤 기능을 언제 착수할 것인가?
- 왜 그런 결정을 했는가?
- 완료된 기능을 어떻게 회귀 테스트할 것인가?
```

공통 규칙 원본은 `Document/SSOT/`에 둔다.
CarFight에만 필요한 판단은 `Document/ProjectSSOT/`에 둔다.

---

## 2. Document 폴더 역할 분리

CarFight 문서 구조는 아래처럼 분리한다.

| 위치 | 역할 |
|---|---|
| `Document/ProjectSSOT/` | 프로젝트 현재 판단 기준 |
| `Document/Plan/` | 앞으로 개발할 기능의 상세 계획 |
| `Document/Systems/` | 개발 완료된 기능의 현재 구현 기준 |
| `Document/SSOT/` | 여러 프로젝트에 공통 적용되는 기준 |
| `Document/ProjectSSOT/Archive/` | 비활성 / 역사 기록 |

중요 원칙:

```text
ProjectSSOT = 판단 기준
Plan = 진행 중 계획
Systems = 완료된 현재 구현
Archive = 역사 기록
```

---

## 3. 현재 활성 문서 최소 집합

`Document/ProjectSSOT/` 루트에서 바로 보는 활성 문서는 아래 6개와 `README.md`로 제한한다.

| 순서 | 문서 | 역할 |
|---:|---|---|
| 0 | `00_Vision.md` | 최종 방향 / 현재 구조 갭 / 유지·교체 원칙 |
| 1 | `01_ProjectState.md` | 현재 실제 기준선 / 임시 운영 편차 / 현재 리스크 |
| 2 | `02_Roadmap.md` | 큰 진행 순서 / 우선순위 / 완료 조건 |
| 3 | `03_FeatureQueue.md` | 기능 후보 / 착수 판단 / 클라·서버·관리툴 필요성 |
| 4 | `04_ProjectDecisions.md` | 프로젝트 전체 결정 로그 |
| 5 | `05_TestChecklist.md` | 완료된 Systems 기준 최소 회귀 테스트 |

루트에 새 문서를 추가하기 전에는 반드시 아래를 먼저 검토한다.

```text
- 기존 00~05 문서에 흡수 가능한가?
- 진행 중 기능 계획이면 Document/Plan/에 둬야 하는가?
- 완료된 기능 기록이면 Document/Systems/에 둬야 하는가?
- 역사 기록이면 Archive로 보내야 하는가?
```

---

## 4. 먼저 읽는 순서

AI나 사용자가 CarFight 현재 상태를 파악할 때는 아래 순서로 읽는다.

```text
1. Document/ProjectSSOT/README.md
2. Document/ProjectSSOT/00_Vision.md
3. Document/ProjectSSOT/01_ProjectState.md
4. Document/ProjectSSOT/02_Roadmap.md
5. Document/ProjectSSOT/03_FeatureQueue.md
6. Document/ProjectSSOT/04_ProjectDecisions.md
7. Document/ProjectSSOT/05_TestChecklist.md
```

세부 구현 기준이 필요하면 그 다음에 `Document/Systems/`를 본다.
진행 중 기능의 상세 계획이 필요하면 그때만 `Document/Plan/`을 본다.

---

## 5. 핵심 기준 문장

아래 문장은 활성 문서들에서 서로 충돌하면 안 된다.

### 5.1 최종 방향

```text
CarFight는 차량 전투와 물리 상호작용을 중심으로,
장기적으로 CMVS / Cluster Union / Geometry Collection 기반의
다차종 차량 전투 구조를 목표로 한다.
```

### 5.2 현재 기준선

```text
현재 실제 구현 기준선은
ACFVehiclePawn + UCFVehicleDriveComp + UCFWheelSyncComp + UCFVehicleData + DA_PoliceCar 조합이다.
```

현재 차량 구조는 최종 구조가 아니라,
`ChaosWheeledVehicle` 기반 하이브리드 구조다.

### 5.3 유지 예정 코어

```text
- VehicleData
- DriveState
- WheelSync
- Thin BP 원칙
- 서버 Spawn/Possess 최소 흐름
```

### 5.4 교체 / 확장 후보

```text
- Vehicle root 구조
- 차량 조립 방식
- 파괴 / 분리 표현 구조
- 서버 권한 전투 처리
- 로드아웃 / 인벤토리 / 보상 저장 구조
- 관리툴 / 운영 도구
```

---

## 6. 문서 생명주기

## 6.1 기능 후보 단계

기능 후보는 `03_FeatureQueue.md`에 등록한다.

```text
기능 후보
→ Document/ProjectSSOT/03_FeatureQueue.md
```

이 단계에서는 상세 설계를 하지 않는다.
클라이언트 / 서버 / 관리툴 / 데이터 / 테스트 필요성만 판단한다.

---

## 6.2 개발 착수 단계

착수하기로 결정한 기능은 `Document/Plan/<기능명>/`에 상세 계획을 만든다.

```text
개발 착수
→ Document/Plan/<기능명>/
```

Plan 문서는 앞으로 개발할 기능의 계획만 담는다.
완료된 구현의 기준 문서로 쓰지 않는다.

---

## 6.3 개발 완료 단계

기능 개발이 완료되면 현재 구현 기준을 `Document/Systems/<분류>/<기능명>.md`에 기록한다.

```text
개발 완료
→ Document/Systems/<분류>/<기능명>.md
```

완료된 Plan 문서는 필요하면 `Document/Plan/Archive/`로 이동한다.

---

## 6.4 역사 기록 단계

더 이상 현재 판단 기준이 아닌 문서는 Archive로 보낸다.

```text
비활성 / 역사 기록
→ Document/ProjectSSOT/Archive/
```

Archive 문서는 삭제가 아니라 보존이다.
다만 현재 판단 기준으로 읽지 않는다.

---

## 7. Archive 운영 원칙

`Archive/`는 역사 기록 보존 구역이다.

| 위치 | 역할 |
|---|---|
| `Archive/LegacyBP/` | 구형 BP 기준 절차 / 핫픽스 / 조사 기록 |
| `Archive/CPP/` | 완료된 C++ 전환 단계 문서 / 과도기 비교 기록 |
| `Archive/Checkpoints/` | 일자별 체크포인트 / 기준선 판정 기록 |
| `Archive/Ops/` | 문서 정리 / 이동 실행 기록 |
| `Archive/Merged/` | 활성 문서에 흡수된 문서 |

Archive를 읽을 때는 다음 원칙을 따른다.

```text
- 현재 기준은 루트 00~05 문서를 우선한다.
- Archive 문서는 과거 판단의 근거로만 본다.
- Archive 내용이 현재 Systems 문서와 충돌하면 Systems 문서를 우선한다.
```

---

## 8. Systems 사용 원칙

`Document/Systems/`는 완료된 기능의 현재 구현 기준이다.

예:

```text
Document/Systems/Vehicles/VehicleDrive.md
Document/Systems/Vehicles/WheelSync.md
Document/Systems/Vehicles/VehicleCoreDecisions.md
Document/Systems/Network/ServerSpawn.md
Document/Systems/Input/Input.md
Document/Systems/UI/AimReticle.md
```

기능 구현을 수정할 때는 관련 `Systems` 문서를 먼저 확인한다.
수정 후 기능의 책임이나 동작 방식이 바뀌면 해당 `Systems` 문서도 갱신한다.

---

## 9. Plan 사용 원칙

`Document/Plan/`은 앞으로 개발할 기능의 상세 계획 구역이다.

Plan은 현재 구현 기준이 아니다.

```text
Plan에 있는 내용 = 앞으로 할 계획
Systems에 있는 내용 = 현재 완료된 구현
ProjectSSOT에 있는 내용 = 프로젝트 판단 기준
```

AI가 현재 구현을 확인할 때 Plan을 기준으로 단정하면 안 된다.

---

## 10. 문서 정리 결과

2026-06-02 정리 결과:

```text
03_VisionAlign.md        → 00_Vision.md
00_Handover.md           → 01_ProjectState.md
01_Roadmap.md            → 02_Roadmap.md
02_FeatureQueue.md       → 03_FeatureQueue.md
03_ProjectDecisionLog.md → 04_ProjectDecisions.md
04_TestChecklist.md      → 05_TestChecklist.md
08_P0_Verification.md    → Archive/Checkpoints/P0_Verification_20260401.md
16_CPP_DecisionLog.md    → Document/Systems/Vehicles/VehicleCoreDecisions.md
```

삭제한 문서는 없다.
루트 SSOT 역할을 벗어난 문서는 보존 위치로 이동했다.

---

## 11. 변경 이력

### v2.0.0 - 2026-06-02

```text
- ProjectSSOT 루트 활성 문서를 00~05 체계로 재정렬
- ProjectSSOT / Plan / Systems / Archive 생명주기 명확화
- P0 상세 검증 문서를 Archive/Checkpoints로 이동한 사실 기록
- 차량 C++ 코어 결정 로그를 Systems/Vehicles로 이관한 사실 기록
- 기존 ProjectSSOT/Plan 경로 설명 제거
```

### v1.1.0 - 2026-04-15

```text
- 문서 버전 / 마지막 정리 날짜 추가
- 당시 CameraPlan 보조 실행 계획 문서 연결
```
