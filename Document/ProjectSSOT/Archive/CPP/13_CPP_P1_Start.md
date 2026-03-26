# Archived — 13_CPP_P1_Start

> 상태: Archived  
> 원래 경로: `Document/ProjectSSOT/13_CPP_P1_Start.md`  
> 현재 기준 문서: `12_CPP_TransitionPlan.md`, `16_CPP_DecisionLog.md`, `19_DriveState_CoreChecklist.md`

---

# Phase 1 시작 체크리스트 (DriveState 정식화)

> 역할: **현재 기준 Phase 1 실행 체크리스트**  
> 언제 읽나: `12_CPP_TransitionPlan.md`에서 Phase 1 착수로 판단됐을 때만 본다.  
> 읽기 관계: 설계 변경이 필요하면 `12`로 돌아가고, 실행 결과 결정 기록은 `16_CPP_DecisionLog.md`에 남긴다.

- 상태: Draft v0.2
- 작성일시(Asia/Seoul): 2026-03-18 07:57
- 대상: `ACFVehiclePawn` + `UCFVehicleDriveComp` (Phase 1)

## 목적
- `UCFVehicleDriveComp`를 단순 입력 전달 컴포넌트에서 **주행 상태 판단 컴포넌트**로 올리기 시작한다.
- 현재 기준 Pawn(`ACFVehiclePawn` / `BP_CFVehiclePawn`)을 유지한 채, 상태/판정/디버그 기준을 Native에 먼저 세운다.

## 생성/수정 대상 C++ 파일 (이번 작업)
- `UE/Source/CarFight_Re/Public/CFVehicleDriveComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleDriveComp.cpp`
- 필요 시:
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

> 현재 목표(v0.2.0)는 **DriveState 골격 도입**이다.  
> 실제 차량 감각 튜닝 전체를 한 번에 바꾸는 단계가 아니며, 먼저 상태 판정/조회 기준을 세우는 데 집중한다.

---

## 초보자용 실행 순서 (오늘 바로 할 것)

### 1) Visual Studio / Rider에서 프로젝트 열기
- 파일: `UE/CarFight_Re.sln`
- 빌드 구성: `Development Editor`
- 플랫폼: `Win64`

### 2) 현재 기준 클래스 열기
- `CFVehicleDriveComp.h/.cpp`
- `CFVehiclePawn.h/.cpp`

먼저 확인할 것:
- `UCFVehicleDriveComp`가 현재 어떤 입력값만 전달하는지
- `ACFVehiclePawn`가 DriveComp를 어떻게 소유/호출하는지

### 3) DriveState 타입 추가
먼저 헤더에 아래 3개를 설계한다.

#### A. `ECFVehicleDriveState`
권장 초안:
- `Idle`
- `Accelerating`
- `Coasting`
- `Braking`
- `Reversing`
- `Airborne`
- `Disabled`

#### B. `FCFVehicleInputState`
권장 항목:
- `ThrottleInput`
- `SteeringInput`
- `BrakeInput`
- `bHandbrakePressed`

#### C. `FCFVehicleDriveStateSnapshot`
권장 항목:
- `CurrentDriveState`
- `CurrentSpeedKmh`
- `ForwardSpeedKmh`
- `bIsGrounded`
- `CurrentInputState`

> 주의: 이름만 보고 역할이 드러나게 작성하고, BP 노출 타입/함수에는 Tooltip을 붙인다.

### 4) DriveComp에 상태 보관 변수 추가
권장 추가 항목:
- `CurrentInputState`
- `CurrentDriveState`
- `LastDriveStateSnapshot`
- 필요 시 `bDriveStateReady`

### 5) 입력 전달 함수에서 입력 상태도 같이 저장
현재 있는 함수:
- `ApplyThrottleInput()`
- `ApplySteeringInput()`
- `ApplyBrakeInput()`
- `ApplyHandbrakeInput()`

이번 단계에서 같이 할 일:
- VehicleMovement로 전달하기 전에 `CurrentInputState`를 갱신한다.
- 아직 큰 리팩터링은 하지 말고, **입력 캐시 + 상태 판정 진입점 확보**에 집중한다.

### 6) 상태 판정 함수 추가
권장 함수 초안:
- `UpdateDriveState()`
- `BuildDriveStateSnapshot()`
- `GetDriveState()`
- `GetDriveStateSnapshot()`

권장 초기 판정 기준:
- 속도가 거의 0이고 입력이 없으면 `Idle`
- 스로틀 입력이 있으면 `Accelerating`
- 브레이크 입력이 있으면 `Braking`
- 속도는 있는데 입력이 거의 없으면 `Coasting`
- 음수 전진 속도 또는 후진 조건이면 `Reversing`
- 접지 아님이면 `Airborne`

> 처음부터 완벽하게 만들려고 하지 말고, **읽기 쉬운 기준 상태**를 먼저 만든다.

### 7) Pawn 조회 API 추가
`ACFVehiclePawn`에 아래 조회 함수를 추가 후보로 둔다.
- `GetVehicleSpeed()`
- `GetDriveState()`
- `GetDriveStateSnapshot()`

원칙:
- Pawn은 직접 상태를 계산하지 않는다.
- DriveComp가 계산한 결과를 **얇게 노출만** 한다.

### 8) 컴파일
- `CarFight_Re` 빌드 실행
- 기대 결과: 타입/함수 선언 및 기본 구현 컴파일 성공

### 9) PIE 테스트 체크리스트
- [ ] 에디터가 정상 실행됨
- [ ] 기존 입력/주행이 깨지지 않음
- [ ] DriveComp가 입력 캐시를 유지함
- [ ] 상태 조회 함수 호출 시 기본값/현재값이 정상 반환됨
- [ ] `BP_CFVehiclePawn`에 새 복잡 로직을 추가하지 않음

---

## 흔한 실패 원인 (초보자용 빠른 점검)

### A. 상태가 계속 `Idle`만 나올 때
확인 순서:
1. 입력 함수에서 `CurrentInputState`를 실제로 갱신했는지
2. `UpdateDriveState()`가 호출되는지
3. 속도 값을 어느 컴포넌트에서 읽는지 확인했는지

### B. 상태가 너무 자주 튀거나 애매할 때
확인 순서:
1. 속도 기준 임계값(거의 0 판정)이 너무 낮지 않은지
2. 브레이크/스로틀 동시 입력 시 우선순위를 정했는지
3. 처음 단계에서는 세밀한 튜닝보다 **읽기 쉬운 단순 규칙**을 썼는지

### C. Pawn과 DriveComp 둘 다 상태를 들고 있을 때
- 이번 단계에서는 **DriveComp만 상태 계산/보관**한다.
- Pawn은 조회/전달만 담당한다.

---

## 다음 단계 (Phase 2 진입 조건)
다음 조건을 만족하면 Pawn API 정리와 WheelSync 최종화 단계로 넘어간다.
- [ ] `ECFVehicleDriveState`가 코드상 공식 기준으로 존재함
- [ ] 입력 상태 구조체와 상태 스냅샷 구조체가 존재함
- [ ] DriveComp가 상태를 계산하고 Pawn은 조회만 수행함
- [ ] `BP_CFVehiclePawn`에 새 상태 판단 분기를 추가하지 않음

---

## 운영 메모
- 상세 전환 판정/브리지 정책/로드맵은 `Document/ProjectSSOT/12_CPP_TransitionPlan.md`를 기준으로 관리
- `BP_ModularVehicle`는 삭제되었으므로 본 문서의 실행 대상이 아니다.
- 현재 Phase 1의 핵심은 **DriveState 기준을 Native에 고정하는 것**이다.
