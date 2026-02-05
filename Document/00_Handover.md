# CarFight — 00_Handover (v1.0)

> 목적: **다른 세션/다른 AI로 100% 이관**해도 개발 맥락이 끊기지 않도록, 현재까지의 *결정/구조/완료/미완료/검증*을 “재현 가능한 형태”로 고정한다.  
> 기준 문서: `이관.txt` (원본)  
> 마지막 정리: 2026-02-05 (Asia/Seoul)

---

## 목차
- [1. TL;DR](#1-tldr)
- [2. 프로젝트 목표와 게임 루프](#2-프로젝트-목표와-게임-루프)
- [3. 기술 스택과 전제](#3-기술-스택과-전제)
- [4. 리포지토리 구조](#4-리포지토리-구조)
- [5. CMVS(노-리깅 차량 파이프라인) 개요](#5-cmvs노-리깅-차량-파이프라인-개요)
- [6. BP_ModularVehicle 상세](#6-bp_modularvehicle-상세)
- [7. 데이터 에셋/구조체 스키마](#7-데이터-에셋구조체-스키마)
- [8. 구현 완료 체크리스트](#8-구현-완료-체크리스트)
- [9. 검증 시나리오(재현 절차)](#9-검증-시나리오재현-절차)
- [10. 진행 중 / 다음 작업(P0~)](#10-진행-중--다음-작업p0)
- [11. Known Issues / 리스크](#11-known-issues--리스크)
- [12. 누적 패치 번들(요약)](#12-누적-패치-번들요약)
- [13. 마이그레이션 가이드(처음부터 세팅)](#13-마이그레이션-가이드처음부터-세팅)

---

## 1. TL;DR

- 엔진/프로젝트: **Unreal Engine 5.7 기반**(향후 UE 최신버전 유지), PC 타겟
- 목표: TPS 드라이빙 액션(트위스티드 메탈 감성) — **운전(물리적 재미) + 무기(전투)** 결합
- 핵심 파이프라인: 외부 DCC 리깅 없이 **Static Mesh 차량 에셋을 엔진에서 바로 주행 가능**하게 만드는 “No-Rigging” 파이프라인
- 현재까지 완료:
  - `BP_ModularVehicle`에서 **Construction Script 자동 조립/배치(Auto-Fit) 로직 완료**
  - 차체/바퀴 메시 할당 + 바퀴 폴백(Smart Fallback) + 우측 바퀴 시각 미러링 완료
  - `ChaosVehicleWheel` 기반 `BP_Wheel_Front / BP_Wheel_Rear` 생성 및 MovementComponent 슬롯(0~3) 등록 완료
- 지금 당장 필요한 P0:
  - [ ] **Event Tick 로직**: 물리 휠 상태 → 시각 Anchor(및 Wheel Mesh) 동기화
  - [ ] **Enhanced Input 세팅**: WASD/Space/Shift 입력 연결

---

## 2. 프로젝트 목표와 게임 루프

### 2.1 장르/방향
- 장르: **TPS 드라이빙 액션**
- 플레이 감각: 실제차처럼 “완전 리얼”보다는, **조작성 + 그럴듯한 물리(하중이동/서스펜션/드리프트)** 를 목표

### 2.2 게임 루프(프로토타입 기준)
1) 차량 탑승/주행  
2) 적/오브젝트 회피 또는 접근  
3) 상황에 맞는 무기(터렛 등) 사용(향후)  
4) 보상/강화/교체(향후)

> 전투/터렛/UI는 아직 본 문서 범위 밖(정의만 있음). 현재는 **차량 파이프라인(P0)** 이 최우선.

---

## 3. 기술 스택과 전제

### 3.1 엔진/플러그인
- Engine: Unreal Engine 5.7 기반(향후 UE 최신버전 유지)
- Platform: PC
- 주요 플러그인(필수/사용 중)
  - `ChaosVehiclesPlugin` (Enabled) — 차량 물리
  - `ModelingToolsEditorMode` (Enabled) — 에셋 정규화(피벗/축 등)를 엔진 내에서 처리

### 3.2 개발 원칙
- 외부 DCC(Blender/Maya 등) 없이 가능하면 엔진 내 해결(“No-Rigging” 원칙)
- 초기 속도: **Blueprint 우선**, 병목/복잡도가 커지면 C++로 이관

---

## 4. 리포지토리 구조

> 문서 허브는 `Document/` 폴더로 고정한다.

권장 트리(이관.txt 기준):
```text
/Content
  /Blueprints
    BP_ModularVehicle.uasset
    BP_Wheel_Front.uasset
    BP_Wheel_Rear.uasset
  /Data
    /Base
      S_WheelConfig.uasset
      PDA_VehicleArchetype.uasset
    /Assets
      DA_MyFirstCar.uasset
  /Meshes
    SM_Chassis_*.uasset
    SM_Wheel_*.uasset
  /Maps
    (TestMap)
```

---

## 5. CMVS(노-리깅 차량 파이프라인) 개요

### 5.1 의도
- “차량 스켈레탈 리깅/바퀴 본 세팅” 없이도,
- **Static Mesh 기반 차량**을 DataAsset로 지정하면,
- 에디터에서 즉시 **차체/바퀴가 배치되고**, 런타임에 물리와 동기화해서 **주행 가능**하게 만든다.

### 5.2 핵심 패턴: Anchor 분리
- 물리/시각 분리를 위해 `Anchor_*`(Scene Component) 를 사용한다.
- 시각 바퀴 메시(`Wheel_Mesh_*`)는 Anchor의 자식으로 두고 **(기본은 0,0,0 / 0,0,0)** 를 유지한다.
- 우측 바퀴 미러링은 **Anchor가 아니라 Mesh만 회전**한다(Anchor 회전은 물리 꼬임 위험).

---

## 6. BP_ModularVehicle 상세

### 6.1 역할
- 차량 Pawn(Chaos)으로서:
  - 에디터(Construction Script)에서 차체/바퀴 외형을 조립
  - 런타임(Event Tick 예정)에서 물리 휠 상태를 읽어 **시각 Anchor/바퀴를 동기화**

### 6.2 컴포넌트 계층 구조(현 상태)
```text
[Root] Mesh (Inherited, Skeletal Mesh) - Ghost Root(렌더링 목적 아님)
  ├── VehicleMovementComponent (Inherited)
  └── SM_Chassis (Static Mesh) - 실제 차체 외형
      ├── Anchor_FL (Scene)
      │   └── Wheel_Mesh_FL (Static Mesh)
      ├── Anchor_FR (Scene)
      │   └── Wheel_Mesh_FR (Static Mesh)  // 우측: Mesh만 Yaw 180
      ├── Anchor_RL (Scene)
      │   └── Wheel_Mesh_RL (Static Mesh)
      └── Anchor_RR (Scene)
          └── Wheel_Mesh_RR (Static Mesh)  // 우측: Mesh만 Yaw 180
```

### 6.3 주요 변수
- `VehicleData` (Type: `PDA_VehicleArchetype`)
  - Instance Editable: True
  - Expose on Spawn: True

### 6.4 Vehicle Movement Wheel Setups(등록 완료)
- Index 0: `BP_Wheel_Front` (FL)
- Index 1: `BP_Wheel_Front` (FR)
- Index 2: `BP_Wheel_Rear`  (RL)
- Index 3: `BP_Wheel_Rear`  (RR)

### 6.5 Construction Script 로직(요약 + 정확한 단계)

#### Step 1) 유효성 검사 & 차체 메시 세팅
- `IsValid(VehicleData)`
  - Valid → 계속
  - Not Valid → 종료
- `Set Static Mesh(SM_Chassis, VehicleData.ChassisMesh)`

#### Step 2) 바퀴 메시 세팅(Smart Fallback)
- FL은 Master로 무조건 사용
- FR/RL/RR은 각 값이 비어있으면 FL로 폴백
- 구현 포인트:
  - 각 휠에 `Select` + `IsValid(해당 WheelMesh)`를 사용해 폴백 분기

#### Step 3) Auto-Fit(앵커 위치 계산)
- `Get Local Bounds(SM_Chassis)`에서 `Max` 핀을 **Split**해서 `Max.X/Y/Z`를 사용
- `WheelConfig`를 Split해서 `SideMargin / ForwardMargin / HeightOffset` 사용
- 계산식:
  - `Loc_X = MaxX - ForwardMargin`
  - `Loc_Y = MaxY + SideMargin`
  - `Loc_Z = HeightOffset`
- 적용:
  - FL: `( Loc_X, -Loc_Y, Loc_Z )`
  - FR: `( Loc_X, +Loc_Y, Loc_Z )`
  - RL: `( -Loc_X, -Loc_Y, Loc_Z )`
  - RR: `( -Loc_X, +Loc_Y, Loc_Z )`

#### Step 4) 우측 바퀴 시각 미러링
- FR/RR은 `Wheel_Mesh_*`만 `Set Relative Rotation`으로 Yaw 180
- **Anchor는 0도 유지**

---

## 7. 데이터 에셋/구조체 스키마

### 7.1 `S_WheelConfig` (User Defined Struct)
- `bAutoFit` (Bool): 자동 배치 사용 여부
- `SideMargin` (Float): 좌우 폭 보정
- `ForwardMargin` (Float): 전후 길이 보정(차체 끝에서 안쪽으로)
- `HeightOffset` (Float): 지상고(높이) 보정

### 7.2 `PDA_VehicleArchetype` (Primary Data Asset)
| 변수명 | 타입 | 의미 |
|---|---|---|
| `ChassisMesh` | StaticMesh | 차체 외형 |
| `WheelMesh_FL` | StaticMesh | 기본 바퀴(마스터) |
| `WheelMesh_FR` | StaticMesh | 우측 앞(없으면 FL) |
| `WheelMesh_RL` | StaticMesh | 좌측 뒤(없으면 FL) |
| `WheelMesh_RR` | StaticMesh | 우측 뒤(없으면 FL) |
| `WheelConfig` | `S_WheelConfig` | 배치 마진/오프셋 |

### 7.3 `DA_MyFirstCar` (데이터 인스턴스)
- 실제 테스트용 차량 구성 데이터

---

## 8. 구현 완료 체크리스트

### 8.1 에셋/데이터
- [x] 에셋 정규화(모델링 모드에서 피벗/전방축 정리)
- [x] `S_WheelConfig` 생성
- [x] `PDA_VehicleArchetype` 생성
- [x] `DA_MyFirstCar` 생성 및 값 채움

### 8.2 BP_ModularVehicle(Construction Script)
- [x] 차체 메시 할당
- [x] 바퀴 메시 할당 + Smart Fallback
- [x] Auto-Fit(바퀴 앵커 위치 자동 배치)
- [x] 우측 바퀴 시각 미러링(Anchor 유지, Mesh만 회전)

### 8.3 물리 데이터(휠)
- [x] `BP_Wheel_Front` 생성(조향 O)
- [x] `BP_Wheel_Rear` 생성(조향 X, 핸드브레이크 O)
- [x] VehicleMovementComponent WheelSetup 0~3 슬롯 등록

---

## 9. 검증 시나리오(재현 절차)

> 목적: “완료” 표기가 실제로 맞는지, 누구나 재현 가능한 절차로 확인한다.

### 9.1 에셋 정규화 검증(차체/바퀴)
- 사전 조건: 메시를 선택한 상태
- 절차:
  1) 모델링 모드(Modeling Mode)로 전환(단축키가 다를 수 있음)
  2) 피벗이 Center/Bottom인지 확인
  3) 전방(Front)이 +X 방향인지 확인
- 기대 결과:
  - 차체: 바닥 기준으로 놓았을 때 “땅에 맞닿는” 위치가 자연스럽다.
  - 바퀴: 회전 축이 의도대로 보인다(축 확인은 디버그에서 1차 체크).

### 9.2 Construction Script 검증(에디터 뷰포트)
- 사전 조건: 레벨에 `BP_ModularVehicle` 배치
- 절차:
  1) Details에서 `VehicleData = DA_MyFirstCar` 지정
  2) 컴파일/저장 없이도 뷰포트에서 즉시 변화를 확인
- 기대 결과:
  - 차체 메시가 바뀐다.
  - 4개 바퀴 메시가 들어간다(비어있는 슬롯은 FL로 폴백).
  - 앵커가 차체 네 귀퉁이로 이동한다.
  - 우측 바퀴의 림이 바깥을 향한다(미러링이 정상).

### 9.3 WheelSetup 등록 검증(클래스 기본값)
- 사전 조건: `BP_ModularVehicle` 열기
- 절차:
  1) VehicleMovementComponent 선택
  2) Wheel Setups(인덱스 0~3) 확인
- 기대 결과:
  - 0~1은 Front, 2~3은 Rear가 지정되어 있다.

---

## 10. 진행 중 / 다음 작업(P0~)

### P0 (막히면 다음 작업이 전부 멈춤)
- [ ] **Event Tick 휠 동기화**
  - 목표: 물리 휠 상태(서스펜션/조향/회전)를 읽어, `Anchor_*`와 `Wheel_Mesh_*`가 따라가게 만든다.
  - 완료 조건:
    - 정지 상태에서도 서스펜션 높이가 안정적(심한 튐/꼬임 없음)
    - 주행 시 휠이 회전하고, 조향 시 전륜이 방향 전환이 보인다
- [ ] **Enhanced Input Setup**
  - 목표: `IA_Throttle/Steer/Brake/Handbrake` + `IMC_Default`로 기본 조작을 연결한다.
  - 완료 조건:
    - WASD/Space/Shift 입력이 VehicleMovementComponent 입력으로 들어간다
    - 디버그(예: Print String)로 값이 들어오는 것 확인 가능

### P1 (P0 안정화 후)
- [ ] Camera Lag(스프링암 기반)
- [ ] Collision Filtering(차체-바퀴/부품간 불필요 충돌 제거, 지터 억제)

---

## 11. Known Issues / 리스크

- [x] Anchor vs Mesh Rotation 문제(해결됨)  
  - Anchor 회전은 유지(0도), 우측 바퀴는 Mesh만 회전하는 구조로 확정
- [x] GetLocalBounds 핀 이슈(해결됨)  
  - Return Value 없이 Max를 Split하는 방식으로 고정
- [ ] **Physics Asset 리스크(확인 필요)**  
  - Ghost Root(Skeletal Mesh)에 Physics Asset이 없으면 차가 땅을 뚫고 떨어질 수 있음  
  - 확인 포인트:
    - PIE에서 Spawn 시 차체가 바닥과 정상적으로 충돌하는지
    - 충돌/질량/COM이 이상하지 않은지

---

## 12. 누적 패치 번들(요약)

> “처음부터 다시 만들기”가 필요할 때의 재현 가능한 정보 요약.

### 12.1 `*.uproject` 플러그인 활성화(v1.0)
- ChaosVehiclesPlugin Enabled
- ModelingToolsEditorMode Enabled

### 12.2 에셋 목록
- `/Game/Data/Base/S_WheelConfig` (Struct)
- `/Game/Data/Base/PDA_VehicleArchetype` (Primary Data Asset Blueprint)
- `/Game/Data/Assets/DA_MyFirstCar` (Instance)
- `/Game/Blueprints/BP_Wheel_Front` (ChaosVehicleWheel)
- `/Game/Blueprints/BP_Wheel_Rear` (ChaosVehicleWheel)
- `/Game/Blueprints/BP_ModularVehicle` (WheeledVehiclePawn(Chaos))

### 12.3 Wheel 기본값(요약)
- Front:
  - Radius 32 / Width 20
  - Steer Angle 40
  - Affected by Engine True
- Rear:
  - Radius 32 / Width 20
  - Steer Angle 0
  - Affected by Engine True
  - Affected by Handbrake True

---

## 13. 마이그레이션 가이드(처음부터 세팅)

1) 프로젝트 플러그인
- `ChaosVehiclesPlugin`, `ModelingToolsEditorMode` 활성화 확인

2) 에셋 생성 순서
1. `S_WheelConfig`
2. `PDA_VehicleArchetype`
3. `BP_Wheel_Front`, `BP_Wheel_Rear`
4. `BP_ModularVehicle`(컴포넌트 계층 구성 + Construction Script)

3) 검증
- 레벨에 `BP_ModularVehicle` 배치
- `VehicleData = DA_MyFirstCar` 지정
- 바퀴 앵커 자동 배치 + 우측 미러링 확인

---

> 다음 문서: `Document/01_Roadmap.md`에서 P0/P1 작업을 “완료 조건”까지 포함해 쪼개서 관리한다.
