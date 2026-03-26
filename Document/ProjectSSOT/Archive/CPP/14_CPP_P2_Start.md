# Archived — 14_CPP_P2_Start

> 상태: Archived  
> 원래 경로: `Document/ProjectSSOT/14_CPP_P2_Start.md`  
> 현재 기준 문서: `12_CPP_TransitionPlan.md`, `16_CPP_DecisionLog.md`

---

# Phase 2 시작 체크리스트 (C++ 디버그 파이프 연결)

> 역할: **Phase 2 실행 시작 체크리스트**  
> 언제 읽나: `14_CPP_P2_Design.md`에서 구조와 브리지 정책을 확인한 뒤, 실제 Tick 브리지/설정값 적용을 시작할 때 읽는다.  
> 읽기 관계: 디버그 입력 세부 점검은 `15_CPP_P2_DebugPipe.md`, helper 비교는 `15_CPP_P2_HelperCmp.md`, 결정 기록은 `16_CPP_DecisionLog.md`로 이어진다.

- 상태: Draft v0.2
- 작성일시(Asia/Seoul): 2026-03-04 09:27
- 대상: `UCFWheelSyncComp` v0.3.0 + `BP_ModularVehicle` Tick 브리지

## 목적
- BP `UpdateWheelVisuals`에서 쓰던 디버그 개념(조향 / 서스펜션 Z / 스핀 Pitch)을
  **C++ `FCFWheelVisualInput` 구조체로 먼저 옮겨서** 계산 결과를 비교 가능하게 만든다.
- 아직 C++가 최종 Transform 적용을 담당하지 않는다. (기본값은 C++ 적용 OFF)

---

## 이번 변경 요약 (코드 v0.3.0)

### 추가/확장된 기능
- `BuildWheelVisualInputsFromDebugPipe()` 추가
  - `GetSteeringInput` 기반 전륜 Yaw 입력 생성
  - `SuspensionZDebugOffset`, `SuspensionVisualSign` 개념 반영
  - `WheelSpinPitchDebugDeg`, `WheelSpinVisualSign` 개념 반영
- `BuildWheelVisualInputsPhase2()`가 이제:
  1) 디버그 파이프 입력 생성
  2) 옵션으로 helper(`GetRealWheelTransform`) 값 반영
  순서로 동작

### 신규/확장된 설정값 (Details 패널)
#### 안전 토글
- `bEnableApplyTransformsInCpp` (**기본 False**) ✅
- `bUseRealWheelTransformHelper` (**기본 False**) ✅
- `bHelperOverridesDebugInputs` (**기본 False**) ✅

#### 디버그 파이프 설정 (BP 개념 대응)
- `bUseSteeringYawDebugPipe` (기본 True)
- `FrontWheelCountForSteering` (기본 2)
- `SteeringYawScaleDeg` (기본 40)
- `SteeringVisualSign` (기본 1)
- `bUseSuspensionZDebugPipe` (기본 False)
- `SuspensionZDebugOffset` (기본 0)
- `SuspensionVisualSign` (기본 1)
- `bUseWheelSpinPitchDebugPipe` (기본 False)
- `WheelSpinPitchDebugDeg` (기본 0)
- `WheelSpinVisualSign` (기본 1)

---

## 초보자용 실행 순서 (오늘 바로 할 것)

### 1) C++ 빌드
- `UE/CarFight_Re.sln` 열기
- `Development Editor / Win64`
- 빌드 실행

### 2) 에디터에서 `BP_ModularVehicle` 열기
- `WheelSyncComp` (`CFWheelSyncComp`) 선택
- 아래 값부터 확인 (중요):
  - `bEnableApplyTransformsInCpp = false` ✅
  - `bUseRealWheelTransformHelper = false` ✅
  - `bHelperOverridesDebugInputs = false` ✅
  - `bVerboseLog = true` ✅

### 3) Tick 브리지 순서 확인
권장 Tick 순서:
1. `Branch(bEnableWheelVisualSync)`
2. `WheelSyncComp -> UpdateWheelVisualsPhase1Stub(DeltaSeconds)`
3. `WheelSyncComp -> UpdateWheelVisualsPhase2(DeltaSeconds)`
4. 기존 BP `UpdateWheelVisuals`

> 핵심: **C++는 입력 계산/상태 기록, BP는 아직 최종 적용 담당**

---

## 디버그 파이프 점검 순서 (축/부호 확인용)

### Step A — 조향(Yaw)만 먼저 확인
1. `bUseSteeringYawDebugPipe = true`
2. `SteeringYawScaleDeg = 40`
3. `SteeringVisualSign = 1` (이상하면 -1로 테스트)
4. `bUseSuspensionZDebugPipe = false`
5. `bUseWheelSpinPitchDebugPipe = false`

#### 기대 결과
- `LastWheelVisualInputs[0]`, `[1]`(전륜)의 `SteeringYawDeg` 값이 변함
- 후륜 `[2]`, `[3]`는 `SteeringYawDeg = 0`
- 기존 BP 조향 시각 동작은 그대로 유지됨

### Step B — 서스펜션 Z 디버그 파이프 확인
1. `bUseSuspensionZDebugPipe = true`
2. `SuspensionZDebugOffset` 값을 `+5`, `-5`처럼 바꿔보기
3. `SuspensionVisualSign`가 반대면 `-1`로 테스트

#### 기대 결과
- `LastWheelVisualInputs[*].SuspensionOffsetZ` 값이 바뀜
- (현재 C++ 적용 OFF이므로 실제 시각 변화는 BP가 담당)

### Step C — 스핀 Pitch 디버그 파이프 확인
1. `bUseWheelSpinPitchDebugPipe = true`
2. `WheelSpinPitchDebugDeg` 값을 `10`, `30`, `90` 등으로 테스트
3. `WheelSpinVisualSign` 부호 확인 (필요 시 `-1`)

#### 기대 결과
- `LastWheelVisualInputs[*].SpinPitchDeg` 값이 바뀜
- BP의 디버그 스핀 파이프와 개념이 일치하는지 비교 가능

---

## PIE 테스트 체크리스트 (Phase 2 중반)
- [ ] 크래시 없이 PIE 시작됨
- [ ] `TryPrepareWheelSync` 성공 (`IsWheelSyncReady == true`)
- [ ] `UpdateWheelVisualsPhase2` 호출 후 `LastWheelVisualInputs` 길이 = 4
- [ ] 조향 입력 시 전륜(0,1)의 `SteeringYawDeg`만 변화
- [ ] `bEnableApplyTransformsInCpp=false` 상태에서 기존 BP 시각 동작 유지
- [ ] 떨림/이중 적용 없음

---

## 절대 먼저 하지 말 것 (중요)
- `bEnableApplyTransformsInCpp = true` 먼저 켜기 ❌
  - BP와 C++가 같은 Transform을 동시에 건드릴 수 있음
- `bUseRealWheelTransformHelper = true`부터 켜고 디버그 파이프 검증 생략 ❌
  - 축/부호/기준값 검증이 어려워짐
- 기존 BP `UpdateWheelVisuals` 삭제 ❌
  - 아직은 병행 검증 단계

---

## 다음 단계 (권장)

### Phase 2 다음 작업 1 — helper 값 비교 모드- 구현 상태(2026-03-04 09:31): **착수 중** (다음 변경에서 `LastHelperCompareStates`, `LastHelperCompareSummary`, delta 임계값 경고 추가 예정)

- `bUseRealWheelTransformHelper = true`
- `bHelperOverridesDebugInputs = false`
- 목적: helper 값을 읽되, 아직 입력 덮어쓰지 않고 비교 로깅/상태 확인만

### Phase 2 다음 작업 2 — 단일 축만 C++ 적용 테스트
- `bEnableApplyTransformsInCpp = true`는 테스트맵에서만
- 단, 먼저 BP 쪽 동일 축 적용을 임시로 끄거나 우회
- 추천 시작: 전륜 조향(Yaw)만 C++ 적용

### Phase 2 다음 작업 3 — BP 최종 적용 단계 분리
- BP `UpdateWheelVisuals` 안에서
  - 조향 적용 / 서스펜션 적용 / 스핀 적용을 분리 가능한 구조로 정리
- 그래야 축 단위로 C++ 담당 전환 가능

---

## 운영 메모
- 전환 원칙/판정/브리지 정책: `Document/ProjectSSOT/12_CPP_TransitionPlan.md`
- Phase 1 시작 문서: `Document/ProjectSSOT/13_CPP_P1_Start.md`
- Phase 2 시작 문서(본 문서): `Document/ProjectSSOT/14_CPP_P2_Start.md`
