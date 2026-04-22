# VehicleDebug v2 이름 초안

- Version: 0.1.0
- Date: 2026-04-22
- Status: Draft
- Scope: VehicleDebug v2 구현 시 사용할 구조체, 위젯, 함수, 변수 이름 초안

---

## 1. 문서 목적

이 문서는 VehicleDebug v2를 실제 구현으로 옮길 때,

- 구조체 이름이 흔들리지 않게 하고,
- 위젯 이름이 역할과 맞게 정리되며,
- 함수/변수 이름이 데이터 책임과 표시 책임을 섞지 않도록

초기 이름 기준을 고정하기 위한 문서다.

이번 문서의 목적은 단순 작명 놀이가 아니다.

VehicleDebug는 이번 확장에서

- 텍스트 기반 출력
- Snapshot 기반 데이터
- HUD 표시
- Panel 표시
- 이벤트 표시

가 함께 존재하게 될 가능성이 높기 때문에, 이름이 조금만 흔들려도 구조가 금방 헷갈릴 수 있다.

---

## 2. 이름 설계 원칙

## 2.1 원본 데이터와 표시 결과를 이름에서 구분한다

가장 중요한 원칙은 아래다.

- 원본 데이터: `Snapshot`, `State`, `Entry`, `Event`
- 표시 결과: `Text`, `Hud`, `Panel`, `Row`, `Block`

예를 들어,

- `VehicleDebugSnapshot`은 원본 데이터
- `VehicleDebugText`는 표시 결과

라는 점이 이름만 봐도 드러나야 한다.

## 2.2 카테고리 이름은 책임 기준으로 붙인다

카테고리는 보기 편한 임시 이름보다 실제 책임 기준으로 붙인다.

추천:
- Overview
- Drive
- Input
- Runtime
- Event

비추천:
- Main
- Extra
- Misc
- Common

## 2.3 위젯 이름은 화면 역할이 바로 드러나게 한다

위젯 이름은 `디버그용 화면 조각`이라는 점이 보이게 한다.

추천 패턴:
- `WBP_VehicleDebugHud`
- `WBP_VehicleDebugPanel`
- `WBP_VehicleDebugText`
- `WBP_VehicleDebugEvents`

## 2.4 함수 이름은 데이터 반환과 텍스트 반환을 분리한다

추천:
- `GetVehicleDebugSnapshot()`
- `BuildVehicleDebugTextSingleLine()`
- `BuildVehicleDebugTextMultiLine()`

즉,
- `Get`은 데이터 반환
- `Build`는 가공된 출력 생성

처럼 쓰는 편이 더 읽기 쉽다.

## 2.5 파일 이름은 짧고 역할이 바로 보이게 한다

이번 프로젝트 기준으로 파일 이름은 길게 늘이지 않는다.

추천:
- `CFVehicleDebugTypes.h`
- `CFVehicleDebugText.h`
- `CFVehicleDebugText.cpp`
- `WBP_VehicleDebugHud`
- `WBP_VehicleDebugPanel`

---

## 3. C++ 타입 이름 권장안

## 3.1 최상위 Snapshot

### 유지 권장
- `FCFVehicleDebugSnapshot`

유지 이유:
- 이미 현재 코드와 문서에 존재하는 이름이다.
- 지금 단계에서 굳이 전체 이름을 갈아엎을 이유가 없다.
- 이후 하위 구조만 정리해도 충분히 확장 가능하다.

권장 역할:
- 차량 디버그 전체 원본 데이터의 최상위 묶음

---

## 3.2 카테고리 하위 구조

### 추천 이름
- `FCFVehicleDebugOverview`
- `FCFVehicleDebugDrive`
- `FCFVehicleDebugInput`
- `FCFVehicleDebugRuntime`

권장 역할은 아래와 같다.

### `FCFVehicleDebugOverview`
HUD에 올릴 핵심 요약값 묶음

### `FCFVehicleDebugDrive`
Drive 상태와 입력 관련 주행 핵심값 묶음

### `FCFVehicleDebugInput`
입력 해석, 장치 모드, 입력 소유권 관련 묶음

### `FCFVehicleDebugRuntime`
런타임 준비/검증/실패 원인 관련 묶음

---

## 3.3 이벤트 구조

### 추천 이름
- `ECFVehicleDebugEventType`
- `ECFVehicleDebugSeverity`
- `FCFVehicleDebugEvent`

권장 역할:

### `ECFVehicleDebugEventType`
이벤트 종류

예:
- `DriveStateChanged`
- `RuntimeReinitialized`
- `InputOwnerChanged`
- `DeviceModeChanged`
- `WarningRaised`

### `ECFVehicleDebugSeverity`
이벤트 중요도

예:
- `Info`
- `Warning`
- `Error`

### `FCFVehicleDebugEvent`
최근 이벤트 한 건

---

## 3.4 표시 정책 enum / 설정 이름

현재 `ECFVehicleDebugDisplayMode`는 이미 존재하므로 즉시 폐기하지 않는다.

### 유지 권장
- `ECFVehicleDebugDisplayMode`

다만 역할은 아래처럼 정리하는 편이 좋다.

- Legacy Text View 중심 모드
- 전체 표시 정책의 유일한 스위치가 아님

### 새 표시 스위치 추천
- `bShowVehicleDebugHud`
- `bShowVehicleDebugPanel`
- `bShowVehicleDebugText`
- `bShowVehicleDebugEvents`

이름 원칙:
- `DriveState`보다 `VehicleDebug`를 붙여 범위를 분명히 한다.
- 표시 대상을 이름만 보고 알 수 있게 한다.

---

## 4. C++ 함수 이름 권장안

## 4.1 데이터 반환 함수

### 유지 / 추천
- `GetVehicleDebugSnapshot()`

### 신규 추천
- `BuildVehicleDebugSnapshot()`
- `BuildVehicleDebugOverview()`
- `BuildVehicleDebugDrive()`
- `BuildVehicleDebugInput()`
- `BuildVehicleDebugRuntime()`

권장 해석:
- `Get...()` : 외부에서 읽는 공개 반환 함수
- `Build...()` : 내부에서 실제 값 조합하는 함수

---

## 4.2 텍스트 생성 함수

현재 이름:
- `GetDebugTextSingleLine()`
- `GetDebugTextMultiLine()`
- `GetDebugTextByDisplayMode()`

호환 때문에 당장 바꾸지 않아도 되지만, 장기적으로는 아래 이름이 더 분명하다.

### 장기 권장 이름
- `BuildVehicleDebugTextSingleLine()`
- `BuildVehicleDebugTextMultiLine()`
- `BuildVehicleDebugTextByDisplayMode()`

단, 지금 프로젝트에서는 기존 API 사용처가 있을 수 있으므로,

- 기존 함수는 유지
- 내부 구현은 새 `Build...` 함수 호출

형태가 가장 안전하다.

---

## 4.3 이벤트 관련 함수

### 추천 이름
- `PushVehicleDebugEvent()`
- `AddVehicleDebugEvent()`
- `GetRecentVehicleDebugEvents()`
- `ClearVehicleDebugEvents()`

권장 해석:
- `Push` 또는 `Add` : 새 이벤트 기록
- `GetRecent...` : 표시용 최근 이벤트 반환

---

## 5. C++ 변수 이름 권장안

## 5.1 최상위 캐시/보관 변수

### 추천
- `CachedVehicleDebugSnapshot`
- `RecentVehicleDebugEvents`

### 비추천
- `DebugData`
- `DebugInfo`
- `TempDebug`

이유:
- 너무 넓고 역할이 모호하다.

---

## 5.2 HUD / Panel 표시 토글

### 추천
- `bShowVehicleDebugHud`
- `bShowVehicleDebugPanel`
- `bShowVehicleDebugText`
- `bShowVehicleDebugEvents`

### 카테고리 토글 추천
- `bShowVehicleDebugOverview`
- `bShowVehicleDebugDrive`
- `bShowVehicleDebugInput`
- `bShowVehicleDebugRuntime`

이렇게 하면 BP 변수 목록에서 봐도 무엇을 켜고 끄는지 바로 보인다.

---

## 5.3 시간 / 개수 설정값

### 추천
- `VehicleDebugEventMaxCount`
- `VehicleDebugMessageDuration`
- `VehicleDebugRefreshInterval`

주의:
현재 있는 `DriveStateDebugMessageDuration`은 범위가 좁게 느껴질 수 있다.
VehicleDebug 전체 기능으로 확장한다면 더 일반화한 이름이 나중에 자연스럽다.

---

## 6. UMG / Widget 이름 권장안

## 6.1 최상위 조합 위젯

### 추천
- `WBP_VehicleDebugRoot`

역할:
- HUD / Panel / Text / Events를 한곳에서 조합하는 루트 위젯

---

## 6.2 핵심 위젯

### 추천
- `WBP_VehicleDebugHud`
- `WBP_VehicleDebugPanel`
- `WBP_VehicleDebugText`
- `WBP_VehicleDebugEvents`

역할:

### `WBP_VehicleDebugHud`
Compact HUD 표시

### `WBP_VehicleDebugPanel`
카테고리 기반 상세 패널

### `WBP_VehicleDebugText`
Legacy Text View

### `WBP_VehicleDebugEvents`
최근 이벤트 목록 표시

---

## 6.3 공통 하위 위젯

### 추천
- `WBP_DebugCategoryBlock`
- `WBP_DebugFieldRow`
- `WBP_DebugStateBadge`
- `WBP_DebugValueTile`

역할:

### `WBP_DebugCategoryBlock`
카테고리 제목 + 본문 묶음

### `WBP_DebugFieldRow`
라벨 / 값 한 줄 표시

### `WBP_DebugStateBadge`
Ready / Warning / Error 같은 작은 상태 배지

### `WBP_DebugValueTile`
HUD용 요약 숫자/문자 블록

---

## 6.4 위젯 내부 변수 이름 권장안

### 예시
- `Text_CategoryTitle`
- `Text_FieldLabel`
- `Text_FieldValue`
- `Border_StateBadge`
- `VerticalBox_FieldList`
- `ScrollBox_CategoryList`

원칙:
- 위젯 타입 접두어를 붙여서 검색성을 높인다.
- `Thing1`, `Thing2` 같은 이름은 쓰지 않는다.

---

## 7. 블루프린트 함수 이름 권장안

### 추천
- `RefreshFromSnapshot`
- `ApplyVehicleDebugSnapshot`
- `ApplyVehicleDebugOverview`
- `ApplyVehicleDebugDrive`
- `ApplyVehicleDebugInput`
- `ApplyVehicleDebugRuntime`
- `ApplyVehicleDebugEvents`

원칙:
- `Apply`는 받은 데이터를 UI에 반영하는 함수
- `Refresh`는 최신 Snapshot을 다시 읽어오는 함수

---

## 8. 현재 구조에서 이름 마이그레이션 권장안

## 8.1 유지 권장 이름

가능하면 바로 바꾸지 않는 것이 좋은 이름들:
- `FCFVehicleDebugSnapshot`
- `ECFVehicleDebugDisplayMode`
- `GetVehicleDebugSnapshot()`
- `GetDebugTextSingleLine()`
- `GetDebugTextMultiLine()`
- `GetDebugTextByDisplayMode()`

이유:
- 현재 문서와 코드의 호환성 유지
- 회귀 리스크 감소

## 8.2 새로 추가 권장 이름

이번 v2에서 새로 넣기 좋은 이름들:
- `FCFVehicleDebugOverview`
- `FCFVehicleDebugDrive`
- `FCFVehicleDebugInput`
- `FCFVehicleDebugRuntime`
- `FCFVehicleDebugEvent`
- `WBP_VehicleDebugHud`
- `WBP_VehicleDebugPanel`
- `WBP_VehicleDebugEvents`
- `WBP_VehicleDebugText`

---

## 9. 가장 먼저 고정하면 좋은 이름 세트

당장 구현 시작 전에 우선 고정할 이름은 아래다.

### C++
- `FCFVehicleDebugOverview`
- `FCFVehicleDebugDrive`
- `FCFVehicleDebugInput`
- `FCFVehicleDebugRuntime`
- `FCFVehicleDebugEvent`

### Widget
- `WBP_VehicleDebugHud`
- `WBP_VehicleDebugPanel`
- `WBP_VehicleDebugText`
- `WBP_VehicleDebugEvents`

### Bool / Config
- `bShowVehicleDebugHud`
- `bShowVehicleDebugPanel`
- `bShowVehicleDebugText`
- `bShowVehicleDebugEvents`

이 세트만 먼저 고정해도 이후 문서/코드/위젯 이름이 크게 흔들리지 않는다.

---

## 10. 결론

이번 이름 초안의 핵심은 아래와 같다.

> Snapshot, Text, Hud, Panel, Event라는 단어를 역할별로 엄격히 나눠 쓰고, 카테고리는 Overview / Drive / Input / Runtime 기준으로 고정한다.

이 원칙만 지켜도,

- 데이터 구조 이름
- 위젯 이름
- 함수 이름
- 토글 변수 이름

이 서로 충돌하거나 의미가 섞일 가능성이 크게 줄어든다.
