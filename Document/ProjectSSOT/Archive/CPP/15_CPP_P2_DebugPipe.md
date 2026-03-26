# Archived — 15_CPP_P2_DebugPipe

> 상태: Archived  
> 원래 경로: `Document/ProjectSSOT/15_CPP_P2_DebugPipe.md`  
> 현재 기준 문서: `12_CPP_TransitionPlan.md`, `16_CPP_DecisionLog.md`

---

# Phase 2 디버그 파이프 연결 (C++ 입력값 비교 단계)

> 역할: **Phase 2 디버그 입력 비교 문서**  
> 언제 읽나: Phase 2 실행 중, BP 디버그 파이프 개념을 C++ 입력 구조와 맞출 때 읽는다.  
> 읽기 관계: 실행 진입은 `14_CPP_P2_Start.md`, helper 비교는 `15_CPP_P2_HelperCmp.md`, 적용 승인은 `16_CPP_DecisionLog.md`를 따른다.

- 상태: Draft v0.1
- 작성일시(Asia/Seoul): 2026-03-04 09:29
- 대상: `UCFWheelSyncComp` v0.3.0

## 목적
- BP `UpdateWheelVisuals`에서 사용하던 디버그 개념(조향/서스펜션 Z/스핀 Pitch)을
- C++ `FCFWheelVisualInput`에 먼저 연결해서, **계산 경로를 C++로 옮길 준비**를 한다.

> 이 단계에서도 기본값은 **C++ 최종 적용 OFF** (`bEnableApplyTransformsInCpp=false`) 이다.
> 즉, 현재는 **C++가 계산/상태 기록**, **BP가 최종 적용** 담당이다.

---

## 이번 변경 요약 (v0.3.0)

### 새로 추가/확장된 디버그 파이프 설정값
`CFWheelSyncComp` Details 패널에서 확인 가능:

### 조향(Yaw) 디버그 파이프
- `bUseSteeringYawDebugPipe` (기본 True)
- `FrontWheelCountForSteering` (기본 2)
- `SteeringYawScaleDeg` (기본 40)
- `SteeringVisualSign` (기본 1)

### 서스펜션 Z 디버그 파이프
- `bUseSuspensionZDebugPipe` (기본 False)
- `SuspensionZDebugOffset` (기본 0)
- `SuspensionVisualSign` (기본 1)

### 스핀 Pitch 디버그 파이프
- `bUseWheelSpinPitchDebugPipe` (기본 False)
- `WheelSpinPitchDebugDeg` (기본 0)
- `WheelSpinVisualSign` (기본 1)

### helper 결합 옵션 (선택)
- `bUseRealWheelTransformHelper` (기본 False)
- `bHelperOverridesDebugInputs` (기본 False)

---

## 초보자용 실행 순서 (오늘 바로 할 것)

### 1) C++ 빌드
- `UE/CarFight_Re.sln` 열기
- `Development Editor / Win64`
- 빌드 실행

### 2) `BP_ModularVehicle`에서 Tick 브리지 확인
권장 Tick 순서 (현재 유지):
1. `Branch(bEnableWheelVisualSync)`
2. `WheelSyncComp -> UpdateWheelVisualsPhase1Stub(DeltaSeconds)`
3. `WheelSyncComp -> UpdateWheelVisualsPhase2(DeltaSeconds)`
4. 기존 BP `UpdateWheelVisuals`

### 3) `WheelSyncComp` Details 설정 (초기 안전값)
- `bEnableApplyTransformsInCpp = false` ✅
- `bUseRealWheelTransformHelper = false` ✅
- `bHelperOverridesDebugInputs = false` ✅
- `bVerboseLog = true` (점검용)

### 4) 조향 디버그 파이프 비교 테스트 (먼저 이것부터)
- `bUseSteeringYawDebugPipe = true`
- `FrontWheelCountForSteering = 2`
- `SteeringYawScaleDeg = 40` (BP 조향각과 맞춰 조정)
- `SteeringVisualSign = 1` 또는 `-1` (축 방향 맞춰 조정)

PIE에서 확인:
- [ ] 좌/우 조향 시 `LastWheelVisualInputs[0/1].SteeringYawDeg` 값이 바뀜
- [ ] 후륜(`2/3`)의 `SteeringYawDeg`는 0 유지
- [ ] 기존 BP 조향 시각 동작은 그대로 유지

### 5) 서스펜션 Z 디버그 파이프 비교 테스트 (옵션)
- `bUseSuspensionZDebugPipe = true`
- `SuspensionZDebugOffset` 값을 5~20 정도로 바꿔보기
- `SuspensionVisualSign` = 1 / -1 테스트

PIE에서 확인:
- [ ] `LastWheelVisualInputs[*].SuspensionOffsetZ` 값이 동일하게 변경됨
- [ ] (현재 C++ 적용 OFF이므로) 화면 결과는 기존 BP가 담당함

### 6) 스핀 Pitch 디버그 파이프 비교 테스트 (옵션)
- `bUseWheelSpinPitchDebugPipe = true`
- `WheelSpinPitchDebugDeg` 값을 10~30 정도로 바꿔보기
- `WheelSpinVisualSign` = 1 / -1 테스트

PIE에서 확인:
- [ ] `LastWheelVisualInputs[*].SpinPitchDeg` 값이 변경됨
- [ ] 화면 결과는 여전히 기존 BP `UpdateWheelVisuals`

---

## 왜 이렇게 하나? (초보자용 설명)

지금 바로 C++가 화면까지 바꾸게 만들면,
- BP `UpdateWheelVisuals`도 같은 프레임에 돌고 있으므로
- **같은 Transform을 두 번 적용**해서 떨림/덮어쓰기 버그가 날 수 있다.

그래서 지금 단계에서는:
- **C++ = 계산/상태 기록 담당**
- **BP = 최종 시각 적용 담당**
으로 나눠서 안전하게 비교한다.

이게 안정화되면 그 다음 단계에서 C++를 최종 적용 담당으로 넘긴다.

---

## 다음 단계 (권장)

### A. helper 경로 값만 비교 (아직 적용 X)
- `bUseRealWheelTransformHelper = true`
- `bHelperOverridesDebugInputs = false`
- 목적: helper 경로가 크래시 없이 호출되는지, 값이 상식적인지 확인

### B. helper 값으로 디버그 입력 덮어쓰기 비교 (아직 적용 X)
- `bUseRealWheelTransformHelper = true`
- `bHelperOverridesDebugInputs = true`
- `bEnableApplyTransformsInCpp = false` 유지
- 목적: C++ 입력 구조체가 실데이터 기반으로도 채워지는지 확인

### C. 단일 축/단일 휠부터 C++ 적용 ON 테스트
- `bEnableApplyTransformsInCpp = true` 전,
- BP 쪽 해당 축/해당 휠 적용을 잠시 끄거나 우회한 뒤 테스트

---

## 빠른 실패 점검표

### 증상: 값은 바뀌는데 화면이 안 바뀜
- 정상일 수 있음 ✅
- 현재는 `bEnableApplyTransformsInCpp=false`라서 BP가 화면 담당

### 증상: 화면이 떨림/중복 적용처럼 보임
- [ ] `bEnableApplyTransformsInCpp`가 실수로 True인지 확인
- [ ] BP `UpdateWheelVisuals`가 동시에 같은 축 적용 중인지 확인

### 증상: 조향 방향이 반대임
- [ ] `SteeringVisualSign` 값을 1 ↔ -1로 변경
- [ ] `SteeringYawScaleDeg` 크기 과다 여부 확인

---

## 운영 메모
- 전환 기준/판정/로드맵: `Document/ProjectSSOT/12_CPP_TransitionPlan.md`
- Phase 1 시작: `Document/ProjectSSOT/13_CPP_P1_Start.md`
- Phase 2 시작: `Document/ProjectSSOT/14_CPP_P2_Start.md`
- Phase 2 디버그 파이프(본 문서): `Document/ProjectSSOT/15_CPP_P2_DebugPipe.md`
