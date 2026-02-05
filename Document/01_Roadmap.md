# CarFight — 01_Roadmap (v1.0)

> 기준: `Document/00_Handover.md`(v1.0) / 원본 `이관.txt`  
> 마지막 정리: 2026-02-05 (Asia/Seoul)

---

## 목차
- [0. 운영 원칙](#0-운영-원칙)
- [1. 백로그 한눈에 보기(P0/P1/P2)](#1-백로그-한눈에-보기p0p1p2)
- [2. P0 상세(필수)](#2-p0-상세필수)
- [3. P1 상세(안정화/품질)](#3-p1-상세안정화품질)
- [4. 리스크/확인 필요 항목](#4-리스크확인-필요-항목)
- [5. 1주 스프린트 플랜(추천)](#5-1주-스프린트-플랜추천)

---

## 0. 운영 원칙

- “P0가 돌아가야 다음이 있다.”  
  → **입력 + 휠 시각 동기화**가 끝나기 전에는 전투/터렛/UI 확장을 하지 않는다.
- 모든 작업은 **완료 조건(Definition of Done)** 과 **재현 절차**를 같이 작성한다.
- 커밋 단위: “바로 롤백 가능한 크기”로 쪼갠다(기능 1개 = 커밋 1~2개).

---

## 1. 백로그 한눈에 보기(P0/P1/P2)

| ID | 우선순위 | 작업명 | 결과물 | 완료 조건(요약) |
|---|---|---|---|---|
| CF-P0-001 | P0 | Enhanced Input 기본 조작 | IA/IMC + BP 연결 | WASD/Space/Shift로 실제 주행/제동/핸브 동작 |
| CF-P0-002 | P0 | 휠 시각 동기화(Tick) | Event Tick 그래프 | 주행 시 휠 회전+조향+서스펜션 스트로크가 보임 |
| CF-P0-003 | P0 | Physics Asset(기본) 확인/생성 | PhysicsAsset 또는 대체 충돌 | Spawn 시 차량이 바닥을 뚫지 않음 |
| CF-P0-004 | P0 | TestMap 주행 테스트 루틴 | 테스트 맵/체크리스트 | 누구나 같은 절차로 재현 가능 |
| CF-P1-001 | P1 | Camera Lag(스프링암) | 카메라 세팅 | 조작감이 과하게 튀지 않음 |
| CF-P1-002 | P1 | Collision Filtering | 충돌 설정 | 차체/바퀴/부품 지터 감소 |
| CF-P2-001 | P2 | (보류) 전투/터렛 프로토 | TBD | P0 안정화 후 정의 |

---

## 2. P0 상세(필수)

### CF-P0-001 — Enhanced Input 기본 조작

**목표**  
WASD/Space/Shift 입력이 `VehicleMovementComponent` 입력으로 들어가서 “운전이 된다”.

**생성/수정 대상(예상)**  
- `/Game/Input/IA_Throttle`
- `/Game/Input/IA_Steer`
- `/Game/Input/IA_Brake`
- `/Game/Input/IA_Handbrake`
- `/Game/Input/IMC_Default`
- `BP_ModularVehicle`(또는 PlayerController/PlayerPawn 입력 처리)

**완료 조건(Definition of Done)**  
- [ ] PIE에서 W/S 입력 → 전/후진(Throttle 값 변화)  
- [ ] PIE에서 A/D 입력 → 조향(Steer 값 변화)  
- [ ] Space 입력 → 브레이크가 체감됨  
- [ ] LeftShift 입력 → 핸드브레이크가 체감됨  
- [ ] 디버그(예: Print String)로 각 입력 값이 들어오는 것을 확인 가능

**검증 절차(재현)**  
1) TestMap에서 BP_ModularVehicle 조종(Possess)  
2) W를 누른 상태에서 속도가 증가하는지 확인  
3) A/D로 조향 시 차량 방향이 꺾이는지 확인  
4) Space/Shift로 제동이 걸리는지 확인

**리스크/주의**  
- 컨텍스트 주입 시점(BeginPlay/OnPossessed 등) 선택이 중요(중복 Add/우선순위 꼬임 방지)

---

### CF-P0-002 — 휠 시각 동기화(Tick)

**목표**  
물리 휠 상태를 읽어 `Anchor_*`(또는 Wheel_Mesh_*)에 적용하여, 시각 휠이 실제로 굴러간다.

**수정 대상(예상)**  
- `BP_ModularVehicle` → Event Graph
- `Anchor_FL/FR/RL/RR`, `Wheel_Mesh_FL/FR/RL/RR`

**완료 조건(Definition of Done)**  
- [ ] 정지 상태에서 바퀴가 이상하게 떨지 않음(심한 지터 X)  
- [ ] 주행 시 바퀴가 회전(구름)  
- [ ] 조향 시 전륜이 조향각 반영  
- [ ] 서스펜션이 눌리거나 튀는 변화가 시각적으로 보임(최소한의 스트로크)

**검증 절차(재현)**  
1) TestMap에서 BP_ModularVehicle 조종  
2) 저속으로 직진하면서 바퀴 회전 확인  
3) 좌/우 조향하면서 전륜 각도 변화 확인  
4) 언덕/턱을 넘을 때 서스펜션 변화 확인(있으면)

**리스크/주의**  
- 축(Roll/Pitch) 방향이 메시 로컬 축과 맞지 않으면 회전이 이상해짐  
- 우측 바퀴는 Mesh가 Yaw 180이므로, 회전 부호/축이 반대로 보일 수 있음(검증 필요)

---

### CF-P0-003 — Physics Asset 확인/생성

**목표**  
Spawn 시 차량이 바닥을 뚫고 떨어지지 않게 “최소 충돌/물리 바디”를 확보한다.

**완료 조건(Definition of Done)**  
- [ ] PIE 시작 시 차량이 지면 위에서 정상적으로 정지/주행  
- [ ] 충돌이 아예 없는 상태(Free fall)로 보이지 않음

**검증 절차(재현)**  
1) TestMap에 바닥 Static Mesh(충돌 있음) 배치  
2) 차량 Spawn  
3) 차량이 바닥을 관통하지 않고 얹혀있는지 확인

---

### CF-P0-004 — TestMap 주행 테스트 루틴

**목표**  
누구나 같은 절차로 현재 기능을 검증할 수 있는 “테스트 루틴”을 고정한다.

**완료 조건(Definition of Done)**  
- [ ] TestMap이 존재한다(또는 동일 목적의 맵)  
- [ ] `00_Handover`의 검증 시나리오가 Map 기준으로 재현 가능하다  
- [ ] 테스트 결과를 체크박스로 남길 수 있다

---

## 3. P1 상세(안정화/품질)

### CF-P1-001 — Camera Lag(스프링암 기반)
- 목표: 운전 카메라가 ‘뒤늦게 따라오는’ 자연스러운 추적을 제공
- 완료 조건:
  - [ ] 급가속/급회전에도 카메라가 과하게 흔들리지 않음
  - [ ] 플레이어가 멀미나지 않는 수준의 Lag 값 확보

### CF-P1-002 — Collision Filtering(지터 감소)
- 목표: 차체/바퀴/부품 간 불필요 충돌 제거
- 완료 조건:
  - [ ] 정지 상태 떨림 감소
  - [ ] 주행 중 튀는 현상 감소

---

## 4. 리스크/확인 필요 항목

- [ ] Physics Asset 존재 여부(또는 대체 충돌 세팅) 확인 필요  
- [ ] Wheel Radius/Width 값이 실제 메시와 일치하는지(체감 주행에 영향 큼)  
- [ ] 우측 바퀴 시각 미러링(Yaw 180)로 인해 회전 방향이 뒤집혀 보이지 않는지

---

## 5. 1주 스프린트 플랜(추천)

> “P0 2개 완성 + 기본 주행 확인”이 목표.

Day 1  
- CF-P0-001(Enhanced Input) 구현 + 디버그 확인

Day 2  
- CF-P0-002(휠 Tick 동기화) 1차 구현(회전/조향/서스펜션 중 2개 이상)

Day 3  
- CF-P0-003(Physics Asset) 확인/보강 + 지면 관통/튐 체크

Day 4  
- CF-P0-004(TestMap 루틴) 정리 + 체크리스트화

Day 5  
- 튜닝(휠 반지름/조향각/마진) + 리그레션 테스트
