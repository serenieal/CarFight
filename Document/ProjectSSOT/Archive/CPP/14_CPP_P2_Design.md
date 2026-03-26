# Archived — 14_CPP_P2_Design

> 상태: Archived  
> 원래 경로: `Document/ProjectSSOT/14_CPP_P2_Design.md`  
> 현재 기준 문서: `12_CPP_TransitionPlan.md`, `16_CPP_DecisionLog.md`

---

# Phase 2 설계/브리지 계획 (C++ Tick 계산 이전 준비)

> 역할: **Phase 2 설계 문서**  
> 언제 읽나: Phase 2를 바로 실행하기 전, 구조/브리지 전략/중복 적용 방지 정책을 먼저 확인할 때 읽는다.  
> 읽기 관계: 실제 실행 절차는 `14_CPP_P2_Start.md`, 디버그 입력 비교는 `15_CPP_P2_DebugPipe.md`, 결정 기록은 `16_CPP_DecisionLog.md`로 이어진다.

- 상태: Draft v0.1
- 작성일시(Asia/Seoul): 2026-03-04 09:24
- 대상: `UCFWheelSyncComp` / `BP_ModularVehicle::UpdateWheelVisuals`

## 목적
- BP의 `UpdateWheelVisuals`에 있는 반복/분기/상태 가드 로직을 C++로 이전하기 위한 **안전한 중간 단계**를 정의한다.
- 초기 단계에서는 **계산만 C++**, 실제 Transform 적용은 옵션으로 잠가서 중복 적용을 방지한다.

---

## 이번 코드 반영 내용 (v0.2.0 Preview)

### 추가 구조체 (C++)
- `FCFWheelVisualInput`
  - 휠 인덱스, 전륜 여부, 서스펜션 Z, 조향 Yaw, 스핀 Pitch, 입력 유효성
- `FCFWheelVisualState`
  - 계산된 Anchor 위치/회전, Mesh 회전, 적용 가능 여부

### 추가 옵션/캐시 (C++)
- `bEnableCppWheelVisualApply` (기본 `false`)
  - C++가 실제 Transform을 적용할지 여부
- `bGuardAgainstDualApply` (기본 `true`)
  - BP/C++ 중복 적용 방지 가드
- `CachedWheelVisualStates`
  - 계산 결과 캐시 (디버그/검증용)

### 추가 함수 (C++)
- `BuildWheelVisualInputsPhase2(...)`
- `EvaluateWheelVisualsPhase2(...)`
- `ApplyWheelVisualStatesPhase2(...)`
- `UpdateWheelVisualsPhase2(...)` (통합 진입점)

> 현재 버전은 **Preview/Stub** 이며, 실제 Snapshot/Chaos 물리값 연결은 아직 하지 않음.

---

## 핵심 전략 (중복 적용 방지)

### 원칙: 한 프레임 한 책임자
- **BP가 최종 Transform 적용 중이면** → C++는 계산만 수행
- **C++가 최종 Transform 적용을 시작하면** → BP 최종 적용 경로를 끈다

### 기본값 정책 (전환 중)
- `bEnableCppWheelVisualApply = false`
- `bGuardAgainstDualApply = true`

즉, 처음에는 `UpdateWheelVisualsPhase2()`를 Tick에 연결해도 **계산/캐시만 수행**하게 만든다.

---

## BP 브리지 변경 순서 (초보자용, 안전 순서)

### Step 1) 기존 BP 적용 경로 유지
- 기존 `UpdateWheelVisuals` BP 적용 로직은 **그대로 둔다**
- 이유: 아직 C++ 실제값 소스/축/부호 검증이 끝나지 않음

### Step 2) Tick에 C++ Phase2 통합 함수 추가 (계산만)
- `Event Tick`에서 기존 경로와 별도로:
  - `WheelSyncComp -> UpdateWheelVisualsPhase2(DeltaSeconds)` 호출
- 이때 `bEnableCppWheelVisualApply=false` 유지

### Step 3) 계산 결과 확인 (Details 패널)
- PIE 중 `WheelSyncComp` 선택
- `CachedWheelVisualStates` 값 확인
  - 인덱스별 상태가 생성되는지
  - Yaw/Pitch/Z 값 흐름이 예상대로 들어오는지

### Step 4) 입력 소스 교체 (실제값 연결 전 준비)
- 현재 `BuildWheelVisualInputsPhase2()`는 0값 스텁 입력만 생성
- 후속 작업에서 아래 순서로 교체:
  1. 디버그 파이프 값 연결 (BP에서 검증된 부호/축 기준)
  2. Chaos 노드 기반 실제값 연결
  3. Snapshot 경로는 보류 유지 (별도 격리 검증 후)

### Step 5) C++ 실제 적용 전환 (마지막 단계)
이 단계는 아래 조건을 만족할 때만 진행:
- [ ] `CachedWheelVisualStates` 계산값이 신뢰 가능
- [ ] BP/C++ 축/부호 일치 검증 완료
- [ ] BP 최종 Transform 적용 경로를 끌 준비 완료

전환 순서:
1. BP 최종 Transform 적용 노드 비활성/우회
2. `bGuardAgainstDualApply = false`
3. `bEnableCppWheelVisualApply = true`
4. PIE 리그레션 테스트

---

## 체크포인트 (실패 방지)

### 체크포인트 A — 중복 적용 금지
- 증상: 바퀴 떨림/튀기/오버라이드
- 원인: BP와 C++가 같은 프레임에 같은 Transform 수정
- 대응: `bEnableCppWheelVisualApply=false` 유지 + `bGuardAgainstDualApply=true`

### 체크포인트 B — Snapshot 보류 유지
- 증상: Tick 평가 시 `ChaosVehicles.dll` 크래시 가능성
- 대응: Phase 2 Preview에서는 Snapshot 사용 금지

### 체크포인트 C — 축/부호 검증 후 실제 적용
- 우측 바퀴는 Mesh Yaw 180 구조라 회전 부호가 다르게 보일 수 있음
- C++ 실제 적용 전 반드시 디버그 파이프 기준과 비교 검증

---

## 다음 구현 작업 (권장 순서)
1. `BuildWheelVisualInputsPhase2()`에 **디버그 입력 소스 연결**
2. `EvaluateWheelVisualsPhase2()`에서 **BaseAnchor/BaseMesh 캐시 반영 계산**
3. `ApplyWheelVisualStatesPhase2()`에 **부분 적용(한 축씩)** 검증 모드 추가
4. 이후에만 실제 소스(Chaos/Snapshot 대체 경로) 검토

---

## 관련 문서
- `Document/ProjectSSOT/12_CPP_TransitionPlan.md` (전환 원칙/판정/로드맵)
- `Document/ProjectSSOT/13_CPP_P1_Start.md` (Phase 1 시작 체크리스트)
- `Document/ProjectSSOT/01_Roadmap.md` (프로젝트 우선순위)
