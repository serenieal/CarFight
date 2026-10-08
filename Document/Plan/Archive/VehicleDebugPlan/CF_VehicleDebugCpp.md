# VehicleDebug v2 C++ 반영 초안

- Version: 0.2.1
- Date: 2026-04-23
- Status: Draft
- Scope: VehicleDebug v2를 실제 C++ / BP / UMG에 반영하기 위한 타입, 함수, 파일 분리 초안

---

## 1. 문서 목적

이 문서는 VehicleDebug v2 설계안을 실제 구현으로 옮길 때,

- 어떤 파일을 만들고,
- 어떤 타입을 어디에 두며,
- 어떤 함수는 유지하고 어떤 함수는 새로 추가하고,
- BP와 UMG는 어떤 책임만 맡게 할지

실행 수준으로 정리한 C++ 반영 초안이다.

이번 문서의 목적은 설계 문서를 다시 반복하는 것이 아니라,

- 실제 작업 시작 시 손이 바로 갈 수준의 구조 초안
- 헤더/CPP 분리 기준
- 공개 API와 내부 구현 함수 경계
- BP 노출 범위와 ToolTip 방향

을 먼저 고정하는 데 있다.

---

## 2. 구현 목표 재확인

이번 C++ 반영의 1차 목표는 아래 네 가지다.

1. 레거시 `WBP_VehicleDebug` 의존을 제거한다.
2. 내부 원본 데이터를 Snapshot 구조로 재편한다.
3. Compact HUD가 같은 Snapshot을 읽게 만든다.
4. Overview / Drive / Input / Runtime 패널이 같은 Snapshot을 공유하게 만든다.

즉, 이번 작업은 새로운 UI를 만드는 일만이 아니라,
**텍스트, HUD, Panel이 같은 원본 데이터를 보게 만드는 구조 개편**이다.

---

## 3. 권장 파일 분리

파일은 한 번에 너무 잘게 쪼개지 않는 편이 좋다.

1차 구현 기준 권장안은 아래다.

### 3.1 타입 정의 파일
- `UE/Source/CarFight_Re/Public/CFVehicleDebugTypes.h`

권장 포함 항목:
- `ECFVehicleDebugEventType`
- `ECFVehicleDebugSeverity`
- `FCFVehicleDebugOverview`
- `FCFVehicleDebugDrive`
- `FCFVehicleDebugInput`
- `FCFVehicleDebugRuntime`
- `FCFVehicleDebugEvent`
- 필요 시 확장된 `FCFVehicleDebugSnapshot`

### 3.2 텍스트 빌드 파일
- `UE/Source/CarFight_Re/Public/CFVehicleDebugText.h`
- `UE/Source/CarFight_Re/Private/CFVehicleDebugText.cpp`

권장 포함 항목:
- Snapshot -> SingleLine Text 변환
- Snapshot -> MultiLine Text 변환
- 표시용 짧은 요약 문자열 생성

### 3.3 Pawn 기존 파일
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

권장 역할:
- 실제 디버그 데이터 수집
- Snapshot 조합
- 기존 공개 API 유지
- BP와 위젯에서 읽을 수 있는 함수 제공

### 3.4 HUD 부모 위젯 파일
- `UE/Source/CarFight_Re/Public/UI/CFVehicleDebugHudWidget.h`
- `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugHudWidget.cpp`

권장 역할:
- `Overview` 카테고리 읽기
- HUD 텍스트 갱신
- HUD 가시성 갱신
- `WBP_VehicleDebugHud` 자식이 재사용할 공통 부모 제공

---

## 4. 타입 정의 초안

## 4.1 이벤트 enum

### `ECFVehicleDebugEventType`

권장 항목:
- `None`
- `DriveStateChanged`
- `RuntimeReinitialized`
- `InputOwnerChanged`
- `DeviceModeChanged`
- `WarningRaised`

설계 이유:
- 최근 이벤트 목록에 표시할 변화를 분류하기 위함
- 나중에 색/배지/필터 기준으로 재사용 가능

### `ECFVehicleDebugSeverity`

권장 항목:
- `Info`
- `Warning`
- `Error`

설계 이유:
- HUD/Panel/Event Strip에서 강조 정도를 나누기 위함

---

## 4.2 Overview 구조

### `FCFVehicleDebugOverview`

권장 필드:
- `bool bRuntimeReady`
- `ECFVehicleDriveState CurrentDriveState`
- `float SpeedKmh`
- `float ForwardSpeedKmh`
- `FString DeviceModeText`
- `FString InputOwnerText`
- `FString LastTransitionShortText`

권장 의도:
- HUD에서 자주 읽을 핵심값만 별도 요약

권장 UPROPERTY 방향:
- `VisibleAnywhere`
- `BlueprintReadOnly`
- `Category="CarFight|VehicleDebug|Overview"`
- `meta=(DisplayName="...", ToolTip="...")`

---

## 4.3 Drive 구조

### `FCFVehicleDebugDrive`

권장 필드:
- `ECFVehicleDriveState CurrentDriveState`
- `ECFVehicleDriveState PreviousDriveState`
- `bool bDriveStateChangedThisFrame`
- `float SpeedKmh`
- `float ForwardSpeedKmh`
- `float Throttle`
- `float Brake`
- `float Steering`
- `bool bHandbrake`
- `FString DriveStateTransitionSummary`

권장 의도:
- Drive 상태와 주행 입력 상태를 한 그룹에서 읽게 함

---

## 4.4 Input 구조

### `FCFVehicleDebugInput`

권장 필드:
- `FString DeviceModeText`
- `FString InputOwnerText`
- `FString MoveZoneText`
- `FString MoveIntentText`
- `FVector2D MoveRaw`
- `float MoveMagnitude`
- `float MoveAngle`
- `bool bUsedBlackZoneHold`

권장 의도:
- 입력값 그 자체보다 입력 해석 결과를 읽기 좋게 제공

주의:
- UI 편의를 위해 enum을 바로 노출하기보다, 일부는 표시 친화 문자열을 함께 들고 있는 것도 허용 가능

---

## 4.5 Runtime 구조

### `FCFVehicleDebugRuntime`

권장 필드:
- `bool bRuntimeReady`
- `bool bHasDriveComponent`
- `bool bHasWheelSyncComponent`
- `FString RuntimeSummary`
- `FString LastInitAttemptSummary`
- `FString LastValidationSummary`

권장 의도:
- 왜 정상인지/왜 비정상인지 찾는 카테고리

---

## 4.6 이벤트 구조

### `FCFVehicleDebugEvent`

권장 필드:
- `ECFVehicleDebugEventType EventType`
- `ECFVehicleDebugSeverity Severity`
- `FString ShortText`
- `FString DetailText`
- `float TimestampSeconds`

권장 의도:
- Event Strip과 Panel Recent Events 영역 공통 원본

---

## 4.7 최상위 Snapshot

### `FCFVehicleDebugSnapshot`

권장 구성:
- `FCFVehicleDebugOverview Overview`
- `FCFVehicleDebugDrive Drive`
- `FCFVehicleDebugInput Input`
- `FCFVehicleDebugRuntime Runtime`
- `TArray<FCFVehicleDebugEvent> RecentEvents`

주의:
- 기존 필드를 남길지, 하위 구조로 완전히 옮길지는 구현 난이도와 호환 범위를 보고 결정
- 1차에서는 기존 필드 일부를 유지하면서 하위 구조를 추가하는 절충도 가능

---

## 5. ACFVehiclePawn 권장 공개 API

## 5.1 유지 권장 API

가능하면 아래 함수는 유지한다.

- `GetVehicleDebugSnapshot()`
- `GetDebugTextSingleLine()`
- `GetDebugTextMultiLine()`
- `GetDebugTextByDisplayMode()`
- `ShouldShowDebugWidget()`  : 레거시 제거 전환 중에는 숨김 고정 API로 취급
- `GetDebugWidgetVisibility()` : 레거시 제거 전환 중에는 `Collapsed` 고정 API로 취급

이유:
- 기존 BP/UMG/문서와 호환성 유지
- 회귀 리스크 감소

---

## 5.2 신규 권장 공개 API

### Snapshot 세부 접근
- `GetVehicleDebugOverview()`
- `GetVehicleDebugDrive()`
- `GetVehicleDebugInput()`
- `GetVehicleDebugRuntime()`
- `GetRecentVehicleDebugEvents()`

이유:
- HUD나 Panel이 최상위 스냅샷 전체를 몰라도 필요한 범위만 읽을 수 있게 함

### 표시 정책
- `ShouldShowVehicleDebugHud()`
- `ShouldShowVehicleDebugPanel()`
- `ShouldShowVehicleDebugText()`
- `ShouldShowVehicleDebugEvents()`

이유:
- Legacy Text 정책을 HUD/Panel 정책과 분리하고, 최종적으로는 Legacy Text 정책 자체를 제거하기 위함

추가 권장 토글:
- `bEnableDriveStateOnScreenDebug`
  - HUD/Panel UI 표시용 메인 토글
- `bEnableVehicleDebugOnScreenMessage`
  - `AddOnScreenDebugMessage` 기반 개발용 문자열 출력 토글

---

## 6. ACFVehiclePawn 권장 내부 함수

실제 구현은 아래처럼 내부 `Build...` 계열 함수로 나누는 것이 좋다.

### 권장 내부 함수
- `BuildVehicleDebugSnapshot()`
- `BuildVehicleDebugOverview()`
- `BuildVehicleDebugDrive()`
- `BuildVehicleDebugInput()`
- `BuildVehicleDebugRuntime()`
- `BuildVehicleDebugTextSingleLine(const FCFVehicleDebugSnapshot& Snapshot)`
- `BuildVehicleDebugTextMultiLine(const FCFVehicleDebugSnapshot& Snapshot)`
- `BuildVehicleDebugTextByDisplayMode(const FCFVehicleDebugSnapshot& Snapshot)`

설계 이유:
- `Get...()`는 바깥에서 읽는 API
- `Build...()`는 내부 조합 로직
- 역할이 이름만 봐도 구분됨

---

## 7. 이벤트 처리 권장안

## 7.1 보관 변수

`ACFVehiclePawn` 또는 전용 디버그 보관 영역에 아래 변수를 둔다.

### 추천 변수
- `TArray<FCFVehicleDebugEvent> RecentVehicleDebugEvents`
- `int32 VehicleDebugEventMaxCount = 8`

## 7.2 권장 함수

- `AddVehicleDebugEvent(...)`
- `ClearVehicleDebugEvents()`
- `TrimVehicleDebugEvents()`

## 7.3 기록 시점 예시

- Drive 상태가 바뀌었을 때
- InputOwner가 바뀌었을 때
- DeviceMode가 바뀌었을 때
- Runtime 재초기화 성공/실패 시

주의:
- Tick마다 쌓이지 않게 한다.
- 진짜 의미 있는 변화만 넣는다.

---

## 8. 텍스트 출력 호환 전략

기존 Text 출력은 바로 제거하지 않는다.

### 권장 흐름
1. `GetVehicleDebugSnapshot()` 호출
2. 내부에서 Snapshot 생성
3. 텍스트 함수는 그 Snapshot을 인자로 받아 문자열 생성

즉,
- 이전: 직접 문자열 조합
- 이후: Snapshot -> Text 변환

이 방식이면 HUD, Panel, Text가 동일 원본을 공유한다.

---

## 9. BP / UMG 반영 권장안

## 9.1 신규 위젯 권장

- `CFVehicleDebugHudWidget` 부모 클래스
- `WBP_VehicleDebugHud`
- `WBP_VehicleDebugPanel`
- `WBP_VehicleDebugEvents`
- `WBP_VehicleDebugRoot`

## 9.2 위젯 역할

### `WBP_VehicleDebugHud`
- 부모 클래스는 `CFVehicleDebugHudWidget` 사용
- Overview 핵심값만 표시
- 레이아웃과 스타일만 담당

### `WBP_VehicleDebugPanel`
- Overview / Drive / Input / Runtime 카테고리 표시

### `WBP_VehicleDebugEvents`
- 최근 이벤트 표시

### `WBP_VehicleDebugRoot`
- 위 네 요소를 묶는 루트 위젯

---

## 9.3 BP 함수 이름 권장

- `ApplyVehicleDebugSnapshot`
- `ApplyVehicleDebugOverview`
- `ApplyVehicleDebugDrive`
- `ApplyVehicleDebugInput`
- `ApplyVehicleDebugRuntime`
- `ApplyVehicleDebugEvents`
- `RefreshFromSnapshot`

원칙:
- BP는 계산보다 적용/표시에 집중
- 이름만 보고 UI 반영 함수임이 드러나야 함

추가 원칙:
- HUD 갱신 로직은 가능하면 C++ 부모 위젯에 둔다.
- 에디터 안내 시 부모 클래스 검색 이름은 `CFVehicleDebugHudWidget`처럼 접두사 제거된 표시 이름 기준으로 적는다.

---

## 10. UPROPERTY / UFUNCTION 메타 방향

사용자와 BP 가독성을 위해 ToolTip과 DisplayName을 적극 붙이는 것을 권장한다.

예를 들어 구조체 필드는 아래 방향이 좋다.

- `DisplayName="현재 Drive 상태 (CurrentDriveState)"`
- `ToolTip="현재 VehicleDriveComp 기준 Drive 상태입니다."`

신규 함수도 같은 방향을 권장한다.

예:
- `ToolTip="HUD 표시용 Overview 디버그 스냅샷을 반환합니다."`
- `ToolTip="최근 VehicleDebug 이벤트 목록을 반환합니다."`

중요:
- ToolTip은 구현 설명이 아니라 BP 사용자가 읽는 사용 설명에 가깝게 적는다.

---

## 11. 1차 반영 순서 권장

### 1단계
- `CFVehicleDebugTypes.h` 추가
- 하위 구조체와 이벤트 타입 정의

### 2단계
- `ACFVehiclePawn`에 `BuildVehicleDebug...` 계열 추가
- 기존 `GetDebugText*`가 새 Snapshot을 쓰도록 변경

### 3단계
- `CFVehicleDebugText.h/.cpp`로 문자열 생성 분리

### 4단계
- `CFVehicleDebugHudWidget` 추가
- `WBP_VehicleDebugHud`와 `WBP_VehicleDebugPanel` 추가

### 5단계
- 최근 이벤트 구조 도입
- `WBP_VehicleDebugEvents` 추가

이 순서의 장점:
- 회귀를 줄임
- 구조를 먼저 안정화함
- UI 작업이 뒤늦게 와도 데이터 구조가 안 흔들림

---

## 12. 1차 완료 정의

아래 조건을 만족하면 C++ 반영 1차 완료로 본다.

1. `FCFVehicleDebugSnapshot`가 카테고리 구조를 가진다.
2. 텍스트 출력이 Snapshot 기반으로 바뀐다.
3. HUD가 Overview를 읽는다.
4. Panel이 Drive / Input / Runtime을 읽는다.
5. 레거시 `WBP_VehicleDebug`를 제거해도 HUD/Panel 구조가 단독으로 동작한다.

---

## 13. 결론

이번 C++ 반영의 핵심은 아래 한 줄이다.

> `ACFVehiclePawn`은 디버그 데이터 원본을 조합하고, 텍스트/HUD/Panel은 그 원본을 각자 다른 방식으로 표시한다.

즉, 앞으로의 VehicleDebug는

- Pawn이 만든 긴 문자열 하나를 뿌리는 구조가 아니라,
- Pawn이 만든 구조화된 Snapshot을 여러 표시 레이어가 공유하는 구조

로 가야 한다.

---

## 14. Changelog

### v0.2.2 - 2026-04-23
- `CFVehicleDebugPanelWidget`에 카테고리별 접기/펼치기와 백드롭 기반 상호작용 모드가 추가된 점을 반영했다.
- Panel이 긴 요약 문자열을 Panel 전용 멀티라인 포맷으로 가공한다는 점을 구현 원칙에 반영했다.
- UI 표시 토글과 온스크린 디버그 메시지 토글이 분리된 현재 구조를 문서에 반영했다.

### v0.2.1 - 2026-04-23
- 레거시 `WBP_VehicleDebug` 제거 방향을 반영해 목표/위젯 목록/표시 정책 문구 정리
- `ShouldShowDebugWidget()`와 `GetDebugWidgetVisibility()`를 레거시 숨김 고정 API로 취급하는 원칙 추가

### v0.2.0 - 2026-04-23
- HUD 구현 기본안을 순수 BP HUD에서 `C++ 부모 위젯 + WBP 자식` 구조로 보강
- `CFVehicleDebugHudWidget` 파일 위치와 책임 추가
- 에디터 안내 시 접두사 제거된 표시 이름을 기준으로 적는 원칙 추가
- 레거시 `WBP_VehicleDebug` 제거 방향과 숨김 고정 API 처리 원칙 추가

### v0.1.0 - 2026-04-22
- VehicleDebug v2 C++ 반영 초안 신규 작성
