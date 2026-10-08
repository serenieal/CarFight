# VehicleDebug Phase 0 기준선

- Version: 0.1.0
- Date: 2026-04-22
- Status: Draft
- Scope: VehicleDebug v2 착수 전 현재 텍스트 디버그 기준선 고정

---

## 1. 문서 목적

이 문서는 VehicleDebug v2 작업을 시작하기 전에,

- 현재 `VehicleDebug`가 실제로 어떤 데이터를 보여주는지
- 현재 `SingleLine / MultiLine` 출력이 어떤 형식인지
- 무엇이 코드로 확인되었고 무엇이 PIE 실측이 아직 필요한지

를 한 문서에 고정하기 위한 Phase 0 기준선 문서다.

이 문서는 미래 설계 문서가 아니라,
**v2 구현 전에 회귀 비교용으로 붙잡아 둘 현재 기준선 문서**다.

---

## 2. 기준 근거

이번 기준선은 아래 근거를 사용해 고정했다.

- `Document/ProjectSSOT/Systems/UI/VehicleDebug.md`
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
- `UE/Source/CarFight_Re/Public/CarFightVehicleUtils.h`
- `UE/Source/CarFight_Re/Private/CarFightVehicleUtils.cpp`

주의:
- 이번 문서는 **코드 기준선 확인 결과**를 우선 반영한다.
- 실제 PIE 화면 문자열 샘플은 아직 이 문서에서 실측값으로 고정하지 않았다.
- PIE 실측은 아래 수동 검증 절차를 따라 추후 채운다.

---

## 3. 현재 구조 한 줄 요약

현재 기준 `VehicleDebug`는
`ACFVehiclePawn`이 차량/입력/런타임 상태를 문자열로 직접 조합하고,
`WBP_VehicleDebug`가 그 문자열을 표시하는 텍스트 기반 디버그 구조다.

즉, 아직 v2 목표인
`Snapshot -> HUD / Panel / Text`
구조로 옮겨지지 않았다.

---

## 4. 현재 코드 기준 확인 결과

## 4.1 표시 스위치

현재 표시 조건은 아래다.

- `bEnableDriveStateOnScreenDebug == true`
- `DriveStateDebugDisplayMode != Off`

현재 기본값은 아래다.

- `bEnableDriveStateOnScreenDebug = false`
- `DriveStateDebugDisplayMode = SingleLine`
- `bShowDriveStateTransitionSummary = true`

정리:
- 디버그 표시를 명시적으로 켜지 않으면 위젯은 숨김 상태가 기본이다.

---

## 4.2 현재 공개 API

현재 기준 공개 API는 아래와 같다.

- `GetVehicleDebugSnapshot()`
- `GetDebugTextSingleLine()`
- `GetDebugTextMultiLine()`
- `GetDebugTextByDisplayMode()`
- `ShouldShowDebugWidget()`
- `GetDebugWidgetVisibility()`

정리:
- v2 문서에서 유지 대상으로 잡은 API 이름은 현재 코드에도 실제로 존재한다.

---

## 4.3 현재 실제 문자열 생성 경로

현재 텍스트 출력은 아래 함수가 직접 만든다.

- `ACFVehiclePawn::BuildVehicleDebugSummary(...)`

현재 호출 흐름은 아래다.

1. `GetDebugTextSingleLine()` -> `BuildVehicleDebugSummary(false, ...)`
2. `GetDebugTextMultiLine()` -> `BuildVehicleDebugSummary(true, ...)`
3. `GetDebugTextByDisplayMode()` -> 현재 DisplayMode를 보고 위 두 함수 중 하나 선택

정리:
- 현재 텍스트 출력은 아직 `Snapshot -> Text` 변환 구조가 아니다.
- v2 Phase 1에서 가장 먼저 바꿔야 할 지점이 이 부분이다.

---

## 4.4 현재 `FCFVehicleDebugSnapshot` 포함 범위

현재 `FCFVehicleDebugSnapshot`는 존재하지만 범위가 아직 좁다.

현재 포함 필드:
- `bRuntimeReady`
- `RuntimeSummary`
- `bHasDriveComponent`
- `bHasWheelSyncComponent`
- `CurrentDriveState`
- `PreviousDriveState`
- `bDriveStateChangedThisFrame`
- `DriveStateTransitionSummary`
- `DriveStateSnapshot`

현재 아직 없는 것:
- `Overview` 하위 구조
- `Drive` 하위 구조
- `Input` 하위 구조
- `Runtime` 하위 구조
- `RecentEvents`

정리:
- 현재 구조는 “최소 디버그 스냅샷” 단계다.
- v2 문서가 요구하는 카테고리형 Snapshot과는 아직 다르다.

---

## 4.5 현재 문자열 출력 항목

현재 `BuildVehicleDebugSummary(...)` 기준으로 출력되는 핵심 항목은 아래다.

### 항상 포함
- `Ready`
- `State`
- `Speed`
- `ForwardSpeed`
- `Throttle`
- `Brake`
- `Steering`
- `Handbrake`

### 입력 상태 포함 시
- `DeviceMode`
- `InputOwner`
- `MoveZone`
- `MoveIntent`
- `MoveRaw`
- `MoveMag`
- `MoveAngle`
- `BlackHold`

### 옵션 포함 시
- 마지막 Drive 상태 전이 요약
- 마지막 런타임 요약 문자열

정리:
- 현재 `VehicleDebug.md`가 설명한 출력 항목 범위는 코드와 대체로 일치한다.

---

## 5. 현재 코드 기준 샘플 포맷

이 절은 **실측값이 아니라 현재 코드 기준 포맷 샘플**이다.

## 5.1 SingleLine 포맷 샘플

```text
Ready=True | State=ECFVehicleDriveState::Idle | Speed=0.0 km/h | ForwardSpeed=0.0 km/h | Throttle=0.00 | Brake=0.00 | Steering=0.00 | Handbrake=Off | DeviceMode=ECFVehicleInputDeviceMode::GamepadOnly | InputOwner=ECFVehicleInputOwnership::None | MoveZone=ECFVehicleMoveInputZone::None | MoveIntent=ECFVehicleMoveDirectionIntent::None | MoveRaw=(0.00, 0.00) | MoveMag=0.00 | MoveAngle=0.0 | BlackHold=False | DriveStateTransition: None | VehicleRuntime: ...
```

주의:
- enum 문자열은 현재 `UEnum::GetValueAsString(...)` 결과를 그대로 사용한다.
- 실제 PIE에서는 현재 상태에 따라 각 값이 달라진다.

## 5.2 MultiLine 포맷 샘플

```text
Ready=True
State=ECFVehicleDriveState::Idle
Speed=0.0 km/h
ForwardSpeed=0.0 km/h
Throttle=0.00
Brake=0.00
Steering=0.00
Handbrake=Off
DeviceMode=ECFVehicleInputDeviceMode::GamepadOnly
InputOwner=ECFVehicleInputOwnership::None
MoveZone=ECFVehicleMoveInputZone::None
MoveIntent=ECFVehicleMoveDirectionIntent::None
MoveRaw=(0.00, 0.00)
MoveMag=0.00
MoveAngle=0.0
BlackHold=False
DriveStateTransition: None
VehicleRuntime: ...
```

주의:
- 현재 MultiLine은 항목 내용이 크게 다른 것이 아니라, 구분자가 ` | `에서 줄바꿈으로 바뀌는 구조다.

---

## 6. 현재 확인된 회귀 비교 포인트

v2 작업 후 아래가 깨지면 회귀로 본다.

1. `GetDebugTextByDisplayMode()`가 `Off / SingleLine / MultiLine` 의미를 유지하지 못함
2. `ShouldShowDebugWidget()`의 표시 조건이 달라짐
3. 기존 문자열에서 아래 핵심 항목이 빠짐
   - `Ready`
   - `State`
   - `Speed`
   - `ForwardSpeed`
   - 입력 해석 정보
   - 상태 전이 요약
   - 런타임 요약
4. 로컬 제어 차량 기준 표시 정책이 깨짐

---

## 7. PIE 실측 샘플 채우기 절차

이 절은 실제 에디터/PIE에서 사용자가 채울 절차다.

## 7.1 준비

1. `BP_CFVehiclePawn` 열기
2. 세부 정보(`Details`)에서 아래 값 확인
   - `화면 DriveState 디버그 사용 (bEnableDriveStateOnScreenDebug)` = `True`
   - `DriveState 디버그 표시 모드 (DriveStateDebugDisplayMode)` = `SingleLine` 또는 `MultiLine`
3. 테스트 맵에서 로컬 플레이어가 해당 Pawn을 조종하도록 배치 확인

## 7.2 SingleLine 실측

1. PIE 실행
2. 정지 상태에서 화면 문자열 1회 캡처
3. 전진 입력 상태에서 화면 문자열 1회 캡처
4. 브레이크 또는 후진 전환 상태에서 화면 문자열 1회 캡처

권장 저장 형식:
- `Idle_SingleLine`
- `Forward_SingleLine`
- `ReverseOrBrake_SingleLine`

## 7.3 MultiLine 실측

1. `DriveStateDebugDisplayMode`를 `MultiLine`으로 변경
2. PIE 실행
3. 정지 상태 문자열 1회 캡처
4. 주행 상태 문자열 1회 캡처

권장 저장 형식:
- `Idle_MultiLine`
- `Driving_MultiLine`

## 7.4 저장 위치 권장

실제 실측 문자열은 이 문서 하단의 `부록 A. PIE 실측 샘플` 절에 붙여 넣는 것을 권장한다.

이유:
- 설계 문서와 분리되어 기준선 문서 하나만 보면 된다.
- 이후 회귀 비교 시 같은 문서에서 바로 대조 가능하다.

---

## 8. 아직 실측이 필요한 항목

아래는 현재 코드 확인만 되었고, 에디터 실측이 아직 남아 있는 항목이다.

- `WBP_VehicleDebug`의 현재 Text 바인딩이 실제로 여전히 `GetDebugTextByDisplayMode()`인지
- `BP_CFVehiclePawn` 이벤트 그래프의 실제 위젯 생성 흐름이 여전히 유지되는지
- PIE 중 실제 enum 문자열 길이가 UI 가독성에 어떤 영향을 주는지

정리:
- 이 3개는 Phase 0 마감 전에 확인하면 가장 좋다.
- 다만 C++ 구조 개편 Phase 1을 시작하는 데 치명적 블로커는 아니다.

---

## 9. Phase 0 완료 정의

아래를 만족하면 Phase 0 기준선 고정이 완료된 것으로 본다.

1. 현재 코드 기준 출력 포맷이 문서화되었다.
2. 현재 `FCFVehicleDebugSnapshot` 범위가 문서화되었다.
3. 현재 표시 조건이 문서화되었다.
4. PIE 실측 샘플을 나중에 같은 위치에 추가할 수 있는 절차가 정리되었다.

---

## 10. 부록 A. PIE 실측 샘플

아직 미작성.

추후 아래 형식으로 채운다.

### Idle_SingleLine
- 여기에 실제 화면 문자열 붙여 넣기

### Forward_SingleLine
- 여기에 실제 화면 문자열 붙여 넣기

### ReverseOrBrake_SingleLine
- 여기에 실제 화면 문자열 붙여 넣기

### Idle_MultiLine
- 여기에 실제 화면 문자열 붙여 넣기

### Driving_MultiLine
- 여기에 실제 화면 문자열 붙여 넣기

---

## 11. Changelog

### v0.1.0 - 2026-04-22
- VehicleDebug v2 착수 전 Phase 0 기준선 문서 추가
- 현재 `BuildVehicleDebugSummary(...)` 기준 출력 항목 정리
- 현재 `FCFVehicleDebugSnapshot` 실제 포함 범위 정리
- PIE 실측 샘플을 같은 문서에 누적할 수 있는 절차 추가

---

## 12. 마이그레이션 지침

- 앞으로 Phase 1 이상 작업에서 텍스트 출력 구조를 바꾸면, 이 문서를 먼저 회귀 비교 기준으로 사용한다.
- 실제 구현이 `Snapshot -> Text` 구조로 바뀌면, 이 문서는 “구 구현 기준선” 문서로 남기고 `VehicleDebug.md`는 새 현재 상태 기준으로 갱신한다.
- PIE 실측 문자열을 채운 뒤에는, 회귀 체크 시 코드 비교와 함께 이 문서의 부록 A도 같이 본다.
