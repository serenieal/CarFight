# Archived — 15_CPP_P2_HelperCmp

> 상태: Archived  
> 원래 경로: `Document/ProjectSSOT/15_CPP_P2_HelperCmp.md`  
> 현재 기준 문서: `12_CPP_TransitionPlan.md`, `16_CPP_DecisionLog.md`

---

# Phase 2 Helper 비교 모드 체크리스트 (착수 초안)

> 역할: **helper 비교 해석 문서**  
> 언제 읽나: 디버그 입력 비교 이후, `GetRealWheelTransform` helper 경로를 바로 적용하지 않고 delta 기반으로 안전 점검할 때 읽는다.  
> 읽기 관계: 비교 해석 결과로 정책 확정이 필요하면 `16_CPP_DecisionLog.md`, 단일 축 적용 테스트로 넘어가면 `17_CPP_P2_SingleAxisYawTest.md`를 따른다.

- 상태: Draft v0.2 (구현 반영)
- 작성일시(Asia/Seoul): 2026-03-04 09:34
- 대상: `UCFWheelSyncComp` (Phase 2 helper compare)

## 목적
- `GetRealWheelTransform` helper 값을 바로 적용하지 않고,
- 먼저 디버그 파이프 입력(`FCFWheelVisualInput`)과 **차이(delta)** 를 비교해서 축/부호/스케일 문제를 안전하게 점검한다.

## 목표 상태 (구현 예정 항목)
> 구현 반영(2026-03-04): `UCFWheelSyncComp` v0.4.0 핵심 패치로 helper 비교 상태/요약/임계값 경고 계산 경로를 추가함.

- `LastHelperCompareStates` 배열 추가 (휠별 delta 상태)
- `LastHelperCompareSummary` 문자열 추가 (최대 delta / 경고 카운트)
- 비교 모드 토글
  - `bEnableHelperCompareMode`
  - `bLogHelperCompareDetails`
- 경고 임계값 설정
  - `HelperCompareWarnDeltaDeg`
  - `HelperCompareWarnDeltaZ`

## 초보자용 테스트 계획 (구현 후 바로 실행)
1. `bEnableApplyTransformsInCpp = false` 유지
2. `bUseRealWheelTransformHelper = true`
3. `bHelperOverridesDebugInputs = false`
4. `bEnableHelperCompareMode = true`
5. 주행/조향하면서 `LastHelperCompareSummary` 확인
6. 필요 시 `bLogHelperCompareDetails = true`로 휠별 delta 로그 확인

## 판정 기준 예시 (초안)
### v0.4.1 추가 확인값 (C++ 상태 문자열)
- `LastHelperCompareWarnWheelIndices`
  - 임계값 초과 휠 인덱스 목록 (예: `0,1` / `None`)
- `LastHelperCompareFrontRearSummary`
  - 전륜/후륜 경고 개수 및 최대 delta 요약
  - 예: `FrontWarn=2 RearWarn=0 ...`

### 해석 가이드 (추천)
- **전륜 Yaw delta만 크다** → 조향 스케일/부호(`SteeringYawScaleDeg`, `SteeringVisualSign`) 우선 점검
- **후륜 Yaw delta가 크다** → 전륜 판정(`FrontWheelCountForSteering`) 또는 helper 축 해석 의심
- **모든 휠 Pitch delta가 크다** → 스핀 축(Pitch vs Roll) 또는 부호(`WheelSpinVisualSign`) 점검
- **모든 휠 Z delta가 크다** → 기준 위치/서스펜션 부호(`SuspensionVisualSign`) 점검

- 조향(Yaw) delta가 전륜에서 지속적으로 큰 경우 → 축/부호/스케일 재검토
- 스핀(Pitch) delta가 비정상적으로 큰 경우 → 로컬축(Pitch/Roll) 매핑 재검토
- 서스펜션 Z delta가 큰 경우 → 기준 위치/부호(`SuspensionVisualSign`) 재검토

## 주의사항
## 단일 축 C++ 적용 테스트 (조향 Yaw만) — v0.4.2 준비 완료

> 이 단계는 **비교 모드 해석이 끝난 뒤** 진행한다.

### 사전 조건
- `bEnableApplyTransformsInCpp = false` 상태에서 helper 비교 결과를 먼저 확인
- `LastHelperCompareWarnWheelIndices`, `LastHelperCompareFrontRearSummary` 해석 완료
- BP 쪽 조향 적용을 임시로 우회/비활성화할 준비 완료

### 테스트 설정 (추천)
1. `bEnableApplyTransformsInCpp = true`
2. `bEnableSingleAxisSteeringYawCppApplyOnly = true`
3. `bUseRealWheelTransformHelper = false` (첫 테스트는 디버그 파이프 기준 권장)
4. `bUseSteeringYawDebugPipe = true`
5. `bUseSuspensionZDebugPipe = false`
6. `bUseWheelSpinPitchDebugPipe = false`

### 기대 결과
- C++ 적용 경로에서 **Anchor Yaw만 적용**됨
- 서스펜션 Z / 스핀 Pitch는 C++에서 건드리지 않음
- `SingleAxisCppApplyTestSummary` 문자열로 현재 테스트 경로 확인 가능

### 실패 시 우선 점검
- BP 조향 적용이 동시에 살아있는지 (이중 적용 여부)
- `FrontWheelCountForSteering` 값이 전륜 구성과 맞는지
- `SteeringYawScaleDeg`, `SteeringVisualSign` 부호/스케일이 맞는지

- 비교 모드 단계에서는 **helper 값을 최종 입력에 덮어쓰지 않는다** (`bHelperOverridesDebugInputs=false`)
- 비교 모드 단계에서는 **C++ 최종 적용 OFF 유지** (`bEnableApplyTransformsInCpp=false`)
- 기존 BP `UpdateWheelVisuals`는 그대로 유지

## 운영 메모
- 단일 축 C++ 적용 테스트 절차 문서: `Document/ProjectSSOT/17_CPP_P2_SingleAxisYawTest.md`

- 전환 원칙/판정/브리지 정책: `Document/ProjectSSOT/12_CPP_TransitionPlan.md`
- Phase 2 시작 문서: `Document/ProjectSSOT/14_CPP_P2_Start.md`
