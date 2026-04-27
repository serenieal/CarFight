# VehicleDebugPlan

- Version: 0.3.7
- Date: 2026-04-27
- Status: Draft / Panel UX 설계 추가
- Scope: VehicleDebug 확장 설계 문서 묶음 안내

---

## 1. 문서 묶음 목적

이 폴더는 현재 텍스트 기반으로 동작하는 `VehicleDebug` 기능을,

- 구조화된 디버그 데이터 시스템으로 정리하고,
- Debug HUD와 Debug Panel을 함께 지원할 수 있는 형태로 확장하며,
- 이후 카테고리 증가에도 유지보수가 가능한 구조로 옮기기 위한

설계/계획 문서를 모아 둔 작업 폴더다.

기준이 되는 현재 상태 문서는 아래다.

- `Document/ProjectSSOT/Systems/UI/VehicleDebug.md`

이 폴더의 문서들은 위 현재 상태 문서를 바탕으로, 앞으로의 확장 방향과 구현 순서를 정리한다.

---

## 2. 문서 목록

### 2.1 `CF_VehicleDebugSpec.md`

이번 확장에서 무엇을 만들고 무엇을 만들지 않는지, 어떤 목표를 우선으로 삼는지 정리한 요구/범위 문서다.

추천 독서 순서:
1. 가장 먼저 읽기
2. 설계와 구현의 기준선 확인

### 2.2 `CF_VehicleDebugDesign.md`

VehicleDebug v2의 목표 구조를 정리한 설계 문서다.

주요 내용:
- 데이터 계층
- ViewModel 계층
- HUD / Panel 계층
- 카테고리 구조
- 이벤트 로그 구조
- 소유권과 생성 위치 권장안

### 2.3 `CF_VehicleDebugPlan.md`

실제 작업을 어떤 단계로 나눠 진행할지 정리한 구현 계획 문서다.

주요 내용:
- 단계별 도입 순서
- 각 단계 산출물
- 리스크와 대응
- 완료 판단 기준

### 2.4 `CF_VehicleDebugBaseline.md`

VehicleDebug v2 착수 전에 현재 텍스트 디버그 기준선을 고정하기 위한 Phase 0 문서다.

주요 내용:
- 현재 코드 기준 출력 항목
- 현재 `FCFVehicleDebugSnapshot` 포함 범위
- 회귀 비교 포인트
- PIE 실측 샘플 채우기 절차

### 2.5 `CF_VehicleDebugHudGuide.md`

`CFVehicleDebugHudWidget` 부모 클래스와 `WBP_VehicleDebugHud` 자식 위젯을 실제 BP/UMG에서 어떻게 만들고 연결할지 정리한 구현 가이드다.

주요 내용:
- 권장 자산 경로
- HUD 표시 항목
- C++ 부모 위젯 구조
- Pawn 연결 방식
- 테스트 절차

### 2.6 `CF_VehicleDebugChecklist.md`

설계/구현/검증 시 빠뜨리기 쉬운 항목을 점검하기 위한 체크리스트다.

주요 사용 시점:
- 구현 시작 전
- 단계 완료 후
- 리팩터링 직전

### 2.7 `CF_VehicleDebugPanelGuide.md`

`CFVehicleDebugPanelWidget` 부모 클래스와 `WBP_VehicleDebugPanel` 자식 위젯을 실제 BP/UMG에서 어떻게 만들고 연결할지 정리한 구현 가이드다.

주요 내용:
- 권장 자산 경로
- Panel 카테고리 구성
- C++ 부모 위젯 구조
- `VerticalBox_DynamicSectionHost` 기반 동적 Section 구조
- `WBP_VehicleDebugSection` 공통 위젯 구성
- `WBP_VehicleDebugFieldRow` 공통 위젯 구성
- Pawn 연결 방식
- 테스트 절차

### 2.8 `CF_VehicleDebugTasks.md`

바로 작업 단위로 옮길 수 있도록 정리한 실무용 작업 목록이다.

주요 내용:
- 선행 작업
- C++ 작업
- 블루프린트 작업
- UI 작업
- 테스트 작업

### 2.9 `CF_VehicleDebugPanelSchema.md`

정적 `WBP_VehicleDebugPanel`을 장기 확장 가능한 데이터 기반 Panel 구조로 전환하기 위한 설계/완료 기준 문서다.

주요 내용:
- Section / Field / Panel ViewData 구조
- 공통 Section Widget 방향
- 정적 Panel에서 동적 Panel로의 전환 전략과 제거 기준
- 성능/안정성/장기 유지보수 원칙

### 2.10 `CF_VehicleDebugNaming.md`

VehicleDebug v2 구현 시 사용할 구조체, 위젯, 함수, 변수 이름 초안을 정리한 이름 기준 문서다.

주요 내용:
- Snapshot / Text / Hud / Panel / Event 명명 원칙
- 카테고리 구조체 권장 이름
- HUD / Panel / Event 위젯 권장 이름
- 표시 토글 변수 권장 이름
- 현재 구조에서 유지할 이름과 새로 추가할 이름 구분

### 2.11 `CF_VehicleDebugPanelUX.md`

VehicleDebug Panel이 카테고리를 하나씩 점진 추가해도 보기 좋은 UX 구조를 유지하기 위한 설계 문서다.

주요 내용:
- 카테고리 미확정 전제
- Navigation + Selected Content 구조
- Top Summary / Event Strip 배치 방향
- 새 카테고리 추가 규칙
- 단계별 Panel UX 전환 계획

---

## 3. 추천 읽기 순서

처음 읽을 때는 아래 순서를 추천한다.

1. `CF_VehicleDebugSpec.md`
2. `CF_VehicleDebugDesign.md`
3. `CF_VehicleDebugNaming.md`
4. `CF_VehicleDebugPlan.md`
5. `CF_VehicleDebugBaseline.md`
6. `CF_VehicleDebugHudGuide.md`
7. `CF_VehicleDebugPanelGuide.md`
8. `CF_VehicleDebugPanelSchema.md`
9. `CF_VehicleDebugPanelUX.md`
10. `CF_VehicleDebugTasks.md`
11. `CF_VehicleDebugChecklist.md`

이 순서대로 보면,

- 왜 바꾸는지
- 무엇으로 바꾸는지
- 어떤 이름으로 고정할지
- 어떤 순서로 바꿀지
- 당장 뭘 해야 하는지
- 무엇을 놓치면 안 되는지

를 자연스럽게 따라갈 수 있다.

---

## 4. 이번 문서 묶음의 핵심 방향

이번 확장의 핵심 방향은 아래 한 줄로 정리한다.

> VehicleDebug의 원본 데이터를 문자열이 아니라 구조화된 Snapshot 기반으로 재편하고, 그 위에 Debug HUD와 Debug Panel을 함께 얹을 수 있는 구조로 확장한다.

즉, 단순히 텍스트 패널을 더 예쁘게 바꾸는 것이 아니라,

- 데이터 구조를 먼저 정리하고,
- 표시용 문자열은 최종 출력 단계로 내리고,
- HUD / Panel / Event 표시가 같은 원본 데이터를 공유하게 만드는 것

이 이번 설계 묶음의 핵심이다.

---

## 5. 문서 관리 원칙

- 이 폴더 문서는 현재 상태 문서가 아니라 앞으로의 작업 기준 문서다.
- 구현이 바뀌면 계획 문서도 함께 갱신한다.
- 실제 구현 확정 후에는 일부 내용이 `Systems` 계열 문서로 승격될 수 있다.
- 작업이 크게 달라지면 `Version`과 `Status`를 함께 갱신한다.

---

## 6. 문서 버전 관리

- 현재 문서 버전: `0.3.7`
- 문서 상태: `Draft / Panel UX 설계 추가`

### 버전 증가 기준
- `Major`
  - 설계 방향 자체가 바뀔 때
  - Snapshot 중심 구조를 포기하거나 다른 축으로 재설계할 때
- `Minor`
  - 새로운 카테고리/패널/이벤트 구조가 추가될 때
  - 구현 단계나 목표 범위가 확장될 때
  - 구현용 이름 기준 문서가 추가되어 실무 범위가 넓어질 때
- 기준선 고정용 Phase 0 문서가 추가될 때
- HUD 실제 구현 가이드 문서가 추가될 때
- Panel 실제 구현 가이드 문서가 추가될 때
- HUD 구현 기본안이 순수 BP에서 C++ 부모 위젯 구조로 바뀔 때
- `Patch`
  - 표현 정리
  - 누락 항목 보강
  - 예시와 설명 추가

---

## 7. 작성 메모

이번 문서 묶음은 아래 현재 정보에 기반해 작성했다.

- `Document/ProjectSSOT/Systems/UI/VehicleDebug.md`
- `ACFVehiclePawn` 공개 API 표면
- 현재 `FCFVehicleDebugSnapshot` 존재 여부
- 현재 `GetDebugTextSingleLine / MultiLine / ByDisplayMode()` 구조
- 현재 `BP_CFVehiclePawn` 기반 로컬 위젯 생성 흐름
- v2 구현 시 이름이 흔들리지 않도록 하기 위한 구조/명명 정리
- Phase 0 회귀 비교용 기준선 정리
- HUD 실제 적용 순서와 BP 작업 가이드 정리
- Panel 실제 적용 순서와 상호작용 모드 가이드 정리

---

## 8. Changelog

### v0.3.7 - 2026-04-27
- `CF_VehicleDebugPanelUX.md`를 추가해 카테고리 미확정/점진 추가 전제의 Panel UX 설계 기준을 문서 묶음에 연결했다.
- Navigation + Selected Content, Top Summary, Event Strip, 새 카테고리 추가 규칙을 상위 안내에 반영했다.

### v0.3.6 - 2026-04-27
- Panel 1차 마감 상태를 문서 묶음 상위 안내에 반영했다.
- 동적 Section, Field Row, 하위 Section 들여쓰기, 정적 `VerticalBox_CategoryList` 제거 완료 흐름을 최신 기준으로 정리했다.

### v0.3.5 - 2026-04-27
- Panel 문서 묶음에 `CFVehicleDebugFieldRowWidget`와 `WBP_VehicleDebugFieldRow` 기반 Field Row 렌더링 전환을 반영했다.

### v0.3.4 - 2026-04-24
- `WBP_VehicleDebugPanel`이 `VerticalBox_DynamicSectionHost`와 `WBP_VehicleDebugSection` 기반 동적 Section 구조로 전환된 상태를 문서 묶음 안내에 반영했다.
- 기존 정적 `VerticalBox_CategoryList` 제거 기준을 Panel 문서 묶음의 최신 방향으로 기록했다.

### v0.3.3 - 2026-04-24
- `CF_VehicleDebugPanelSchema.md`를 추가해 Panel을 데이터 기반 구조로 전환하기 위한 설계 문서를 문서 묶음에 연결했다.
- README도 다른 구현 문서와 동일하게 체인지로그를 유지해야 한다는 관리 기준을 반영했다.

### v0.3.2 - 2026-04-23
- `CF_VehicleDebugPanelGuide.md`의 접기/펼치기와 백드롭 상호작용 모드 반영에 맞춰 문서 묶음 안내를 최신 상태로 정리했다.

### v0.3.1 - 2026-04-23
- Panel 실제 구현 가이드 문서 추가를 반영했다.
- HUD 구현 기본안이 순수 BP에서 C++ 부모 위젯 구조로 바뀐 점을 버전 기준에 반영했다.

### v0.3.0 - 2026-04-22
- Phase 0 기준선 문서 `CF_VehicleDebugBaseline.md` 추가를 반영했다.
