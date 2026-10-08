# VehicleDebug v2 설계안

- Version: 0.1.0
- Date: 2026-04-22
- Status: Draft
- Scope: Snapshot 기반 VehicleDebug 구조, HUD/Panel 구성, 소유권 분리 설계

---

## 1. 문서 목적

이 문서는 현재 텍스트 기반 `VehicleDebug`를,

- 구조화된 디버그 데이터 기반으로 재편하고,
- Debug HUD와 Debug Panel을 함께 지원하며,
- 이후 카테고리 증가에도 버티는 구조로 확장하기 위한

설계 기준안을 정리한다.

이번 문서의 핵심은 아래 한 줄이다.

> VehicleDebug의 원본은 문자열이 아니라 Snapshot 구조체이며, HUD / Panel / Text 출력은 모두 같은 Snapshot을 공유한다.

---

## 2. 현재 구조 요약

현재 문서 기준 구조는 대략 아래와 같다.

1. `ACFVehiclePawn`이 런타임/주행/입력 상태를 읽는다.
2. `BuildVehicleDebugSummary()` 계열 함수가 이를 문자열로 조합한다.
3. `WBP_VehicleDebug`가 그 문자열을 Text로 표시한다.
4. `BP_CFVehiclePawn`이 로컬 제어 차량일 때 위젯을 생성해 뷰포트에 붙인다.

이 구조의 장점은 단순하고 빠르다는 점이다.

하지만 아래 확장 요구에는 약하다.

- HUD와 Panel을 동시에 지원
- 카테고리별 접기/펼치기
- 값 기반 강조나 상태 배지
- 최근 이벤트 분리 표시
- 카테고리 증가에 따른 유지보수

---

## 3. 목표 구조 개요

권장 구조는 아래 4층으로 나눈다.

### 3.1 Data Layer

실제 차량/입력/런타임 상태를 수집하는 계층이다.

예:
- Pawn
- DriveComp
- WheelSyncComp
- CameraComp
- 입력 해석 계층

### 3.2 Snapshot Layer

Data Layer에서 수집한 상태를 화면 친화적인 구조체로 묶는 계층이다.

예:
- `FCFVehicleDebugSnapshot`
- 카테고리별 하위 Snapshot 구조체
- 최근 이벤트 구조체

### 3.3 ViewModel Layer

표시 정책을 담당하는 계층이다.

예:
- 어떤 카테고리를 어떤 순서로 노출할지
- HUD에 어떤 필드만 보여줄지
- 값 강조 여부
- 숨김/정렬/접기 상태

### 3.4 Widget Layer

실제 화면 표시 계층이다.

예:
- Compact HUD
- Debug Panel
- Recent Event Strip
- Legacy Text View

---

## 4. 핵심 설계 원칙

## 4.1 문자열은 원본이 아니라 출력물이다

기존에는 문자열이 사실상 중심이었다.

v2에서는 아래 원칙으로 바꾼다.

- 원본 = Snapshot 구조체
- 출력 = HUD / Panel / Text

즉, 텍스트는 유지할 수 있어도 그것은 마지막 표현 단계다.

## 4.2 현재 값과 최근 이벤트를 분리한다

현재 값과 이벤트는 성격이 다르다.

### 현재 값 예시
- 현재 속도
- 현재 Drive State
- 현재 입력값
- 현재 Runtime Ready

### 최근 이벤트 예시
- Idle -> Accelerating 전이
- InputOwner 변경
- Runtime 재초기화 성공/실패

이 둘을 같은 TextBlock 한 덩어리로만 다루지 않는다.

## 4.3 HUD와 Panel은 역할이 다르다

### HUD 역할
- 지금 당장 중요한 값 빠르게 보기
- 한눈에 읽기
- 상시 표시

### Panel 역할
- 상세 원인 분석
- 카테고리별 점검
- 접기/펼치기
- 많은 정보 소화

## 4.4 카테고리는 책임 기준으로 자른다

카테고리는 보기 편하다는 이유보다, 실제 책임 단위에 맞춰 나눈다.

좋은 예:
- Drive
- Input
- Runtime
- Wheel

피해야 할 예:
- Extra
- Misc
- MoreInfo

---

## 5. Snapshot 구조 권장안

## 5.1 최상위 구조

기본 최상위는 기존 구조를 유지하되, 하위 구조를 카테고리별로 나누는 방향을 권장한다.

예시 개념:

- `FCFVehicleDebugSnapshot`
  - `Overview`
  - `Drive`
  - `Input`
  - `Runtime`
  - `RecentEvents`

초기에는 아래 4개를 먼저 확정한다.

- Overview
- Drive
- Input
- Runtime

---

## 5.2 Overview 카테고리

HUD에 가장 자주 올라갈 핵심 요약값을 모아 둔다.

추천 항목:
- `bRuntimeReady`
- `CurrentDriveState`
- `SpeedKmh`
- `ForwardSpeedKmh`
- `DeviceMode`
- `InputOwnerSummary`
- `LastTransitionShortText`

설계 의도:
- 상시 보는 핵심값만 따로 모아 HUD가 쉽게 가져가게 한다.

---

## 5.3 Drive 카테고리

현재 주행과 상태 전이를 자세히 본다.

추천 항목:
- `CurrentDriveState`
- `PreviousDriveState`
- `bDriveStateChangedThisFrame`
- `SpeedKmh`
- `ForwardSpeedKmh`
- `Throttle`
- `Brake`
- `Steering`
- `bHandbrake`
- `DriveStateTransitionSummary`

설계 의도:
- 현재 DriveComp 스냅샷과 상태 전이를 같은 카테고리에서 읽게 한다.

---

## 5.4 Input 카테고리

입력 해석 상태를 본다.

추천 항목:
- `DeviceMode`
- `InputOwner`
- `MoveZone`
- `MoveIntent`
- `MoveRawX`
- `MoveRawY`
- `MoveMagnitude`
- `MoveAngle`
- `bUsedBlackZoneHold`

설계 의도:
- 단순 입력값이 아니라, 입력 해석 결과까지 한 그룹에서 확인하게 한다.

---

## 5.5 Runtime 카테고리

초기화와 진단 결과를 본다.

추천 항목:
- `bRuntimeReady`
- `RuntimeSummary`
- `bHasDriveComponent`
- `bHasWheelSyncComponent`
- `LastInitAttemptSummary`
- `LastValidationSummary`

설계 의도:
- 지금 차량이 왜 정상/비정상인지 빠르게 원인을 찾게 한다.

---

## 5.6 Recent Event 구조

현재 문서 기준 상태 전이 요약이 이미 있으므로, 이를 일반화한 이벤트 리스트 구조를 권장한다.

권장 이벤트 종류 예시:
- DriveStateChanged
- RuntimeReinitialized
- InputOwnerChanged
- DeviceModeChanged
- WarningRaised

권장 보유 정보 예시:
- `EventType`
- `ShortText`
- `DetailText`
- `TimeSeconds`
- `Severity`

설계 의도:
- 현재 값만으로는 보이지 않는 순간 변화를 따로 추적한다.

---

## 6. 표시 구조 권장안

## 6.1 Compact HUD

화면 구석에 항상 보이는 작은 디버그 HUD다.

권장 표시 항목:
- State
- Speed
- Throttle
- Brake
- Steering
- Input Mode
- Runtime Ready

권장 특징:
- 항목 수 적음
- 큰 요약값 위주
- 텍스트/배지 중심
- 상태 변화 시 짧은 강조 가능

---

## 6.2 Debug Panel

카테고리 기반 상세 패널이다.

권장 섹션:
- Overview
- Drive
- Input
- Runtime
- Recent Events

권장 특징:
- 접기/펼치기
- 스크롤 가능
- 카테고리별 구분 뚜렷함
- 라벨/값 정렬
- 긴 요약 문자열도 표시 가능

---

## 6.3 Legacy Text View

기존 `SingleLine / MultiLine` 표시도 남겨 둔다.

권장 역할:
- 빠른 복붙
- 스크린샷 기반 공유
- 기존 사용자 습관 유지
- 초기 마이그레이션 호환

권장 원칙:
- 직접 원본 데이터가 되지 않음
- Snapshot -> Text 변환 결과로만 존재

---

## 7. 소유권 및 생성 위치 권장안

## 7.1 현재 구조

현재는 `BP_CFVehiclePawn`이 디버그 위젯을 직접 생성한다.

초기에는 이 구조를 완전히 부정하지 않는다.
다만 장기적으로는 소유권을 더 상위 레이어로 올리는 것이 유리하다.

## 7.2 권장 방향

장기 권장안은 아래와 같다.

- 데이터 원본: `ACFVehiclePawn`
- UI 소유/관리: 로컬 플레이어 기준 상위 계층
  - 후보: `PlayerController`
  - 후보: 전용 Debug HUD Manager
  - 후보: LocalPlayer 기반 Debug Layer

설계 이유:
- Possess 대상 변경 대응이 쉬움
- HUD 지속성이 좋아짐
- 다른 디버그와 공존하기 쉬움
- 토글 입력 관리가 쉬움

## 7.3 점진적 마이그레이션 권장

처음부터 UI 소유권을 크게 옮기지 않아도 된다.

초기 권장 순서:
1. 데이터 구조 먼저 Snapshot 중심으로 전환
2. HUD / Panel 위젯 추가
3. 이후 필요 시 생성 위치를 Pawn 밖으로 이동

---

## 8. 표시 정책 권장안

현재는 `Off / SingleLine / MultiLine` 중심이다.

v2에서는 아래처럼 표시 정책을 분리하는 편이 좋다.

### 8.1 표시 스위치 분리

추천:
- `bShowDebugHud`
- `bShowDebugPanel`
- `bShowDebugTextView`
- `bShowRecentEvents`

이유:
- HUD와 Panel 역할이 다르기 때문
- 하나의 enum으로 모든 조합을 관리하면 곧 복잡해짐

### 8.2 카테고리 표시 분리

추천:
- `bShowOverviewCategory`
- `bShowDriveCategory`
- `bShowInputCategory`
- `bShowRuntimeCategory`

이유:
- 디버깅 목적에 따라 필요한 카테고리가 다름
- Drive 확인 중 Input을 숨길 수 있어야 함

---

## 9. 권장 위젯 구성

가능한 한 작은 위젯을 조합하는 형태를 추천한다.

### 9.1 최상위
- `WBP_VehicleDebugRoot`

### 9.2 하위
- `WBP_VehicleDebugHud`
- `WBP_VehicleDebugPanel`
- `WBP_VehicleDebugEvents`
- `WBP_VehicleDebugText`
- `WBP_DebugCategoryBlock`
- `WBP_DebugFieldRow`

이 구조의 장점:
- 카테고리 추가가 쉬움
- 공통 Row 스타일 재사용 가능
- HUD와 Panel을 분리 관리 가능

---

## 10. C++와 BP 책임 분리 권장안

## 10.1 C++ 책임

- Snapshot 구조 정의
- 디버그 데이터 수집
- 이벤트 기록
- 표시용 최소 ViewModel 또는 가공 함수 제공

## 10.2 블루프린트 책임

- 위젯 배치
- 표시 토글
- 시각 스타일
- 접기/펼치기
- 로컬 플레이어 연결

이유:
- 데이터 계산은 C++이 안정적
- UI 레이아웃은 BP/UMG가 빠름

---

## 11. 성능 및 운영 메모

### 11.1 모든 값을 매 프레임 문자열로 재조합하지 않기

가능하면 Snapshot은 구조체로 유지하고,
문자열 생성은 실제 필요할 때만 한다.

### 11.2 HUD에는 핵심값만 올리기

HUD에 너무 많은 값을 넣으면 Panel과 차별점이 사라진다.

### 11.3 이벤트 개수 제한 두기

최근 이벤트는 예를 들어 최근 5~10개 정도만 유지하도록 제한하는 편이 좋다.

---

## 12. 권장 마이그레이션 원칙

1. 기존 `GetDebugText*` 계열은 바로 제거하지 않는다.
2. `GetVehicleDebugSnapshot()`를 메인 원본으로 승격한다.
3. HUD는 Overview 중심으로 먼저 붙인다.
4. Panel은 Drive / Input / Runtime 순으로 확장한다.
5. 이벤트 스트립은 마지막에 붙여도 된다.

---

## 13. 설계 결론

이번 설계의 핵심 결론은 아래와 같다.

> VehicleDebug v2는 `Pawn -> String -> TextBlock` 구조에서 `Pawn/Comp -> Snapshot -> ViewModel -> HUD/Panel/Text` 구조로 이동해야 한다.

이렇게 해야,

- 현재 텍스트 디버그를 유지하면서도,
- 새로운 HUD와 Panel을 얹을 수 있고,
- 이후 카테고리 증가에도 구조가 버틸 수 있다.
