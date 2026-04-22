# VehicleDebug v2 구현 계획

- Version: 0.1.0
- Date: 2026-04-22
- Status: Draft
- Scope: VehicleDebug 구조 개편과 HUD/Panel 도입의 단계별 계획

---

## 1. 문서 목적

이 문서는 `VehicleDebug`를 한 번에 갈아엎지 않고,

- 현재 기능을 최대한 유지하면서,
- Snapshot 중심 구조로 옮기고,
- Debug HUD와 Debug Panel을 점진적으로 붙이는

실행 순서를 정리한 계획 문서다.

핵심 원칙은 아래와 같다.

> 호환을 유지하면서 내부 원본 데이터를 먼저 바꾸고, 그 다음 표시 레이어를 확장한다.

---

## 2. 전체 전략

전체 전략은 아래 순서를 따른다.

1. 현재 텍스트 출력 유지
2. Snapshot 중심으로 내부 원본 정리
3. Compact HUD 추가
4. Category Panel 추가
5. Recent Events 추가
6. 필요 시 UI 소유권을 Pawn 밖으로 이동

이 순서가 중요한 이유는,

- 현재 사용 중인 디버그 기능을 바로 깨지 않게 하고,
- 단계별로 검증 가능한 상태를 유지하며,
- 어느 단계에서 멈춰도 최소 기능이 남게 하기 위함이다.

---

## 3. 단계별 계획

## 3.1 Phase 0 - 기준선 고정

### 목표
현재 `VehicleDebug` 동작을 기준선으로 고정한다.

### 해야 할 일
- 현재 `VehicleDebug.md` 기준 기능 범위를 다시 확인한다.
- 현재 텍스트 출력 샘플을 저장한다.
- 현재 표시 조건과 생성 흐름을 재확인한다.
- 현재 `FCFVehicleDebugSnapshot` 포함 범위를 점검한다.

### 산출물
- 현재 상태 문서 유지
- 기준 출력 샘플 확보
- 회귀 비교 기준 확보

### 완료 기준
- 기존 `SingleLine / MultiLine` 출력의 기준 샘플이 남아 있음
- 현재 HUD가 없는 상태에서도 기존 기능이 재현 가능함

---

## 3.2 Phase 1 - Snapshot 구조 확장

### 목표
기존 문자열 중심 구조를 건드리지 않으면서, 내부 디버그 원본을 카테고리 구조로 확장한다.

### 해야 할 일
- `FCFVehicleDebugSnapshot`를 카테고리 중심으로 재정리한다.
- `Overview / Drive / Input / Runtime` 하위 구조를 정의한다.
- 현재 문자열 생성 함수가 새 Snapshot을 원본으로 사용하도록 바꾼다.
- 기존 `GetDebugText*` 함수는 유지한다.

### 권장 원칙
- 출력 문자열을 먼저 바꾸지 않는다.
- 필드 이름은 나중에 HUD와 Panel에서 그대로 써도 이해되는 수준으로 둔다.
- 기존 DriveComp 스냅샷과 중복되는 정보는 책임 경계를 정리한다.

### 산출물
- 구조화된 Snapshot 정의
- Snapshot 기반 텍스트 출력
- 기존 기능 호환 유지

### 완료 기준
- 기존 텍스트 출력이 살아 있음
- 텍스트 출력 원본이 새 Snapshot으로 바뀜
- Overview / Drive / Input / Runtime 분리가 완료됨

---

## 3.3 Phase 2 - Compact HUD 도입

### 목표
가장 자주 보는 핵심값을 위한 작은 HUD를 추가한다.

### 해야 할 일
- `WBP_VehicleDebugHud` 작성
- Overview 기반 표시 항목 선정
- HUD 표시 토글 추가
- 기존 Text View와 동시에 공존 가능하게 구성

### 권장 표시 항목
- State
- Speed
- Throttle
- Brake
- Steering
- InputMode
- RuntimeReady

### 설계 포인트
- HUD는 요약값만 보여준다.
- Panel 수준 상세값은 올리지 않는다.
- 값 변화 시 간단한 강조는 허용하되 과한 연출은 피한다.

### 산출물
- Compact HUD 위젯
- HUD 토글 정책
- HUD용 데이터 바인딩

### 완료 기준
- HUD On/Off 가능
- 기존 텍스트 패널과 동시 사용 가능
- 한눈에 핵심 상태 확인 가능

---

## 3.4 Phase 3 - Category Panel 도입

### 목표
긴 문자열 텍스트 대신 카테고리 기반 상세 패널을 도입한다.

### 해야 할 일
- `WBP_VehicleDebugPanel` 작성
- `WBP_DebugCategoryBlock` 공통 블록 작성
- `WBP_DebugFieldRow` 공통 Row 작성
- `Overview / Drive / Input / Runtime` 카테고리 배치
- 카테고리 토글 또는 접기/펼치기 지원

### 설계 포인트
- 항목이 많아도 읽기 쉬운 정렬 구조를 우선한다.
- Overview는 요약, 나머지는 상세 중심으로 배치한다.
- 기존 MultiLine 텍스트와 병행 가능하게 둔다.

### 산출물
- Category Panel 위젯
- 공통 카테고리 블록
- 공통 필드 행 위젯

### 완료 기준
- 카테고리별 정보가 명확히 구분됨
- 긴 텍스트 한 블록보다 가독성이 좋아짐
- 필요한 카테고리만 열어볼 수 있음

---

## 3.5 Phase 4 - Recent Events 도입

### 목표
현재 값과 최근 변화를 분리해서 추적한다.

### 해야 할 일
- 최근 이벤트 구조 정의
- 이벤트 Push 시점 정리
- `WBP_VehicleDebugEvents` 작성
- 최근 N개 유지 규칙 추가

### 추천 이벤트
- DriveStateChanged
- InputOwnerChanged
- DeviceModeChanged
- RuntimeReinitialized
- WarningRaised

### 설계 포인트
- 모든 변화 이벤트를 무조건 다 쌓지 않는다.
- 디버깅 가치가 큰 변화만 남긴다.
- 이벤트 텍스트는 짧고 즉시 읽혀야 한다.

### 완료 기준
- 현재 값만 봐서는 놓치는 순간 변화가 따로 보임
- 최근 5~10개 이벤트 정도를 안정적으로 유지함

---

## 3.6 Phase 5 - UI 소유권 정리

### 목표
필요할 경우, Pawn 직접 생성 구조를 더 상위 레벨로 옮긴다.

### 후보 방향
- `PlayerController` 관리
- 전용 Debug HUD Manager
- LocalPlayer 기반 Debug Layer

### 이 단계가 필요한 조건
- Possess 대상 전환 시 HUD 유지가 필요함
- 여러 시스템 디버그를 하나의 입력 체계로 묶고 싶음
- 차량 외 디버그와 통합 표시가 필요함

### 설계 포인트
- 이 단계는 선택적이다.
- 초기 v2 성공 조건에는 반드시 포함되지 않는다.

---

## 4. 우선순위

이번 작업의 우선순위는 아래와 같이 둔다.

### 최우선
1. Snapshot 구조 확장
2. 기존 텍스트 호환 유지
3. Compact HUD 추가

### 중간 우선
4. Category Panel 추가
5. 카테고리별 표시 토글

### 후속 우선
6. Recent Events 추가
7. UI 소유권 재배치

---

## 5. 주요 리스크와 대응

## 5.1 문자열 호환 깨짐

### 리스크
기존 디버그 텍스트를 사용하는 흐름이 깨질 수 있다.

### 대응
- `GetDebugText*` 함수는 즉시 제거하지 않는다.
- Snapshot -> Text 변환 레이어를 따로 둔다.
- 기준선 샘플과 비교한다.

## 5.2 HUD와 Panel이 같은 값을 중복 구현

### 리스크
HUD용 데이터 가공과 Panel용 데이터 가공이 따로 생기면 유지보수가 나빠진다.

### 대응
- 같은 Snapshot을 공유한다.
- HUD는 Overview에서, Panel은 상세 카테고리에서 읽게 한다.

## 5.3 카테고리 경계 모호함

### 리스크
어떤 값이 Overview인지 Drive인지 Runtime인지 모호해질 수 있다.

### 대응
- 카테고리는 책임 기준으로 분리한다.
- 같은 값이 여러 곳에 필요하면 원본은 한 곳에 두고, Overview에는 요약만 둔다.

## 5.4 Pawn 직접 UI 생성 구조의 확장 문제

### 리스크
향후 전역 디버그와 통합 시 구조가 막힐 수 있다.

### 대응
- 이번 단계에서는 데이터 구조를 먼저 안정화한다.
- UI 소유권 이동은 후속 단계로 분리한다.

---

## 6. 검증 계획

각 단계 완료 후 아래를 확인한다.

### 기능 검증
- 기존 텍스트 출력 유지 여부
- HUD 표시 여부
- Panel 표시 여부
- 카테고리 값 누락 여부
- 이벤트 발생/유지 여부

### 사용성 검증
- HUD가 한눈에 읽히는지
- Panel이 문자열 덩어리보다 읽기 쉬운지
- 필요한 값 찾는 시간이 줄었는지

### 유지보수 검증
- 새 카테고리 추가가 쉬운지
- 새 값 추가가 한 함수 비대화로 이어지지 않는지

---

## 7. 이번 단계 권장 작업 묶음

실무 기준으로는 아래 3묶음으로 나누는 것을 추천한다.

### 작업 묶음 A
- Snapshot 구조 정리
- 텍스트 호환 유지

### 작업 묶음 B
- Compact HUD 추가
- HUD 토글 정책 추가

### 작업 묶음 C
- Category Panel 추가
- Recent Events 추가

이렇게 나누면 한 번에 너무 많은 영역을 동시에 건드리지 않게 된다.

---

## 8. 완료 정의

이번 VehicleDebug v2의 1차 완료는 아래 조건으로 정의한다.

1. Snapshot 기반 구조가 도입됨
2. 기존 텍스트 디버그가 유지됨
3. Compact HUD가 추가됨
4. Overview / Drive / Input / Runtime Panel이 작동함
5. 이후 카테고리 확장 가능한 구조가 됨

Recent Events와 UI 소유권 이동은 1차 완료 이후 확장 항목으로 둬도 된다.

---

## 9. 결론

이번 계획의 핵심은,

- 먼저 내부 원본을 바꾸고,
- 그 다음 표시 레이어를 늘리며,
- 기존 기능을 깨지 않는 방향으로 이동하는 것

이다.

즉, 이번 개량은 `텍스트를 더 꾸미는 작업`이 아니라,
`VehicleDebug를 앞으로도 계속 늘릴 수 있는 구조로 재편하는 작업`으로 봐야 한다.
