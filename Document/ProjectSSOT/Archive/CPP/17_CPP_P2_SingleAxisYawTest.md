# Archived — 17_CPP_P2_SingleAxisYawTest

> 상태: Archived  
> 원래 경로: `Document/ProjectSSOT/17_CPP_P2_SingleAxisYawTest.md`  
> 현재 기준 문서: `12_CPP_TransitionPlan.md`, `16_CPP_DecisionLog.md`

---

# Phase 2 단일 축 C++ 적용 테스트 체크리스트 (조향 Yaw 전용)

> 역할: **단일 축 C++ 적용 테스트 문서**  
> 언제 읽나: helper 비교 해석이 끝나고, 조향 Yaw만 C++ 적용하는 실제 테스트 단계에 들어갈 때 읽는다.  
> 읽기 관계: 테스트 전 구조/정책은 `14_CPP_P2_Design.md`, 비교 결과 해석은 `15_CPP_P2_HelperCmp.md`, 승인 근거는 `16_CPP_DecisionLog.md`를 먼저 확인한다.

- 상태: Draft v0.1
- 작성일시(Asia/Seoul): 2026-03-04 16:14
- 대상: `BP_ModularVehicle::UpdateWheelVisuals` + `UCFWheelSyncComp` (`v0.4.2`)

## 목적
- 다축(Z/Yaw/Pitch) 전환 전에, **조향 Yaw만 C++ 적용**하는 테스트를 안전하게 수행한다.
- 핵심은 **BP와 C++의 조향 Yaw 이중 적용을 방지**하는 것이다.

---

## 테스트 전 개념 정리 (중요)
현재 기본 구조는 다음과 같다.
- **BP `UpdateWheelVisuals`**: 최종 시각 적용 담당 (기존)
- **`UCFWheelSyncComp`**: 비교/검증 및 C++ 적용 경로 준비

단일 축 테스트에서는 아래처럼 역할을 잠깐 바꾼다.
- **조향 Yaw만 C++가 적용**
- **서스펜션 Z / 스핀 Pitch는 BP가 계속 적용**

즉, 테스트 성공 조건은 다음 한 줄이다.
> **같은 프레임에 같은 조향 Yaw를 BP와 C++가 동시에 적용하지 않기**

---

## 사전 조건 (반드시 먼저 충족)

### 1) helper 비교 모드 해석 완료
아래 값을 먼저 확인하고, 축/부호가 크게 틀리지 않는지 판단한다.
- `LastHelperCompareSummary`
- `LastHelperCompareWarnWheelIndices`
- `LastHelperCompareFrontRearSummary`

### 2) 개발 브랜치에서 작업
- 권장 브랜치 예: `feat/wheel-sync-cpp-yaw-test`
- 이유: BP 그래프 임시 우회는 되돌림이 필요할 수 있음

### 3) BP 백업 복제
- `BP_ModularVehicle` Duplicate
- 예: `BP_ModularVehicle_Bak_YawTest`

---

## C++ 측 설정값 (WheelSyncComp) — 테스트용
`BP_ModularVehicle`에서 `WheelSyncComp` 선택 후 Details 패널에서 설정:

### 필수 설정
- `bEnableApplyTransformsInCpp = true`
- `bEnableSingleAxisSteeringYawCppApplyOnly = true` ✅
- `bUseSteeringYawDebugPipe = true`
- `bUseSuspensionZDebugPipe = false`
- `bUseWheelSpinPitchDebugPipe = false`

### 첫 테스트 권장 설정 (디버그 파이프 기준)
- `bUseRealWheelTransformHelper = false`
- `bHelperOverridesDebugInputs = false`

> 첫 테스트는 helper까지 동시에 켜지 말고, 조향 Yaw C++ 적용 경로만 검증하는 것이 안전하다.

---

## BP에서 해야 할 임시 우회 작업 (핵심)

## 목표
BP `UpdateWheelVisuals` 안에서 **조향 Yaw 적용 부분만 임시로 끄고**,
서스펜션 Z / 스핀 Pitch 관련 적용은 유지한다.

---

## 초보자용 작업 순서 (Unreal Editor 클릭 기준)

### Step 1) `BP_ModularVehicle` 열기
- Content Browser에서 `BP_ModularVehicle` 열기
- 함수/그래프에서 `UpdateWheelVisuals` 찾기

### Step 2) 조향 Yaw 적용 노드 구간 찾기
보통 아래 패턴을 찾으면 된다.
- 전륜 판정 (`WheelIndex == 0`, `1` 또는 `Front Wheel` 판정)
- 조향값 계산 (`Steering Input`, `Yaw` 계산)
- Anchor 회전 적용 (`Set Relative Rotation` 또는 Anchor Rot 수정)

#### 찾은 뒤 해야 할 것
- 해당 구간 전체를 **Comment Box**로 감싸기
- 코멘트 이름 예시:
  - `BP_YAW_APPLY_BLOCK` (원본 조향 적용 구간)
  - `CPP_YAW_TEST_BYPASS` (임시 우회 표시)

### Step 3) BP 조향 적용 구간 임시 우회(비활성화)
초보자에게 안전한 방법 2가지 중 하나를 사용한다.

#### 방법 A (권장) — Branch로 우회
1. 조향 Yaw 적용 직전 실행선에 `Branch` 추가
2. 새 bool 변수 생성 (BP 변수)
   - 이름 예: `bUseCppSteeringYawTest`
   - 기본값: `false`
3. `bUseCppSteeringYawTest == true`일 때는 **BP 조향 적용 구간을 건너뛰기**
4. `false`일 때는 기존 BP 조향 적용 구간으로 연결

> 장점: 테스트 끝나면 bool만 끄면 원복 가능

#### 방법 B — 실행선 임시 분리 (빠르지만 비권장)
- 조향 적용 노드 실행선을 끊고 우회 연결
- 테스트 후 원복을 잊기 쉬움

### Step 4) 서스펜션/스핀 적용은 그대로 유지 확인
- BP `UpdateWheelVisuals`에서 아래는 그대로 남겨둔다.
  - 서스펜션 Z 관련 위치 적용
  - 스핀 Pitch 관련 Mesh 회전 적용

> 이번 테스트는 **조향 Yaw만 C++로 이동**하는 단계이므로 나머지는 BP 유지가 맞다.

### Step 5) Tick 흐름 확인
권장 순서(이미 연결된 흐름 기준):
1. `Branch(bEnableWheelVisualSync)`
2. `WheelSyncComp -> UpdateWheelVisualsPhase1Stub(DeltaSeconds)`
3. `WheelSyncComp -> UpdateWheelVisualsPhase2(DeltaSeconds)`
4. 기존 BP `UpdateWheelVisuals`

> BP `UpdateWheelVisuals`는 남겨두되, **조향 Yaw 블록만 우회**된 상태여야 한다.

---

## PIE 테스트 체크리스트 (조향 Yaw 전용)

### 시작 전
- [ ] `bUseCppSteeringYawTest = true` (BP 우회 활성)
- [ ] `bEnableApplyTransformsInCpp = true`
- [ ] `bEnableSingleAxisSteeringYawCppApplyOnly = true`
- [ ] `bUseSteeringYawDebugPipe = true`
- [ ] `bUseSuspensionZDebugPipe = false`
- [ ] `bUseWheelSpinPitchDebugPipe = false`

### 테스트 중 확인
- [ ] 전륜 조향이 동작한다 (좌/우 회전)
- [ ] 조향 시 떨림/이중 적용 느낌이 없다
- [ ] 후륜 조향이 생기지 않는다
- [ ] 서스펜션 Z / 스핀 Pitch는 기존 BP 동작 유지
- [ ] `SingleAxisCppApplyTestSummary`가 갱신된다

### 이상 현상 발생 시 빠른 점검
- [ ] BP 조향 적용 블록이 정말 우회됐는지 (`bUseCppSteeringYawTest` 분기 확인)
- [ ] `FrontWheelCountForSteering = 2`인지
- [ ] `SteeringYawScaleDeg`, `SteeringVisualSign` 값 확인
- [ ] `bEnableSingleAxisSteeringYawCppApplyOnly`가 true인지

---

## 테스트 종료 후 원복 절차 (매우 중요)

### 원복 A (권장)
- `bUseCppSteeringYawTest = false`
- `bEnableApplyTransformsInCpp = false`
- `bEnableSingleAxisSteeringYawCppApplyOnly = false`

### 원복 B (실험 계속할 때)
- BP 우회는 유지하되, 브랜치 코멘트/메모를 남긴다
- `00_Handover.md` 또는 `16_CPP_DecisionLog.md`에 테스트 상태 기록

---

## 결과 기록 템플릿 (복붙용)
- 날짜/브랜치:
- 설정값:
  - `bEnableApplyTransformsInCpp=`
  - `bEnableSingleAxisSteeringYawCppApplyOnly=`
  - `bUseRealWheelTransformHelper=`
  - `SteeringYawScaleDeg=` / `SteeringVisualSign=`
- BP 우회 여부:
  - `bUseCppSteeringYawTest=`
- 관찰 결과:
  - 전륜 조향 정상/비정상
  - 떨림 여부
  - 후륜 오동작 여부
  - 비고

---

## 운영 메모
- 전환 원칙/브리지 정책: `Document/ProjectSSOT/12_CPP_TransitionPlan.md`
- helper 비교 문서: `Document/ProjectSSOT/15_CPP_P2_HelperCmp.md`
- Decision Log: `Document/ProjectSSOT/16_CPP_DecisionLog.md`
