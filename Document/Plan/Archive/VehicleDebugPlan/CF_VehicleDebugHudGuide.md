# VehicleDebug HUD 구현 가이드

- Version: 0.2.1
- Date: 2026-04-23
- Status: Draft
- Scope: `CFVehicleDebugHudWidget` C++ 부모 클래스와 `WBP_VehicleDebugHud` 자식 위젯을 안전하게 도입하기 위한 구현 가이드

---

## 1. 문서 목적

이 문서는 VehicleDebug v2의 Phase 2에서,

- `WBP_VehicleDebugHud`를 어디에 만들고
- 어떤 값만 HUD에 올리고
- 어떤 함수로 데이터를 받아오며
- 어떤 방식으로 갱신해야 하는지

를 초보자도 그대로 따라 만들 수 있도록 정리한 실무 가이드다.

이 문서의 목표는 화려한 HUD가 아니라,
**기존 Text View를 깨지 않으면서 바로 실전에서 읽을 수 있는 작은 요약 HUD를 안정적으로 붙이는 것**이다.

---

## 2. 현재 상태 한 줄 요약

현재 C++ 기준으로는 아래 준비가 끝난 상태다.

- `FCFVehicleDebugSnapshot`에 `Overview / Drive / Input / Runtime` 카테고리 존재
- `GetVehicleDebugOverview()` 공개 API 존재
- `ShouldShowVehicleDebugHud()` 공개 API 존재
- `UCFVehicleDebugHudWidget` C++ 부모 위젯 클래스 존재
- 기존 Text 디버그는 제거 대상으로 분리 중

즉,
이번 단계는 **C++ 부모 위젯 위에 얇은 `WBP_VehicleDebugHud` 자식을 얹는 단계**다.

---

## 3. 이번 단계 목표

이번 `WBP_VehicleDebugHud` 1차 목표는 아래다.

1. 화면 구석에 작은 요약 HUD를 만든다.
2. `Overview` 카테고리만 읽는다.
3. HUD 갱신 로직은 C++ 부모 클래스에 둔다.
4. 기존 `WBP_VehicleDebug`를 제거해도 HUD만으로 핵심 상태를 읽을 수 있게 한다.
5. 레이아웃이 무겁지 않고 한눈에 읽힌다.

이번 단계에서 하지 않는 것:

- 복잡한 애니메이션
- 그래프/게이지
- 상세 카테고리 표시
- Recent Events
- Panel 수준 정보 노출

---

## 4. 권장 자산 경로

### 신규
- `UE/Source/CarFight_Re/Public/UI/CFVehicleDebugHudWidget.h`
- `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugHudWidget.cpp`
- `/Game/CarFight/UI/WBP_VehicleDebugHud`

### 선택적 후속
- `/Game/CarFight/UI/WBP_DebugValueTile`
- `/Game/CarFight/UI/WBP_DebugStateBadge`

이번 1차에서는 공통 하위 위젯 없이
`CFVehicleDebugHudWidget` + `WBP_VehicleDebugHud` 조합으로 먼저 시작해도 된다.

이유:
- HUD 갱신 로직을 BP 그래프에 쌓지 않아도 된다.
- 공통 위젯 분리는 HUD가 실제로 안정화된 뒤 해도 늦지 않다.

---

## 5. HUD에 올릴 값

이번 1차 권장 표시 항목은 아래 6개다.

1. `CurrentDriveState`
2. `SpeedKmh`
3. `ForwardSpeedKmh`
4. `DeviceMode`
5. `InputOwner`
6. `bRuntimeReady`

선택 추가 가능 항목:

7. `LastTransitionShortText`

주의:
- `Throttle / Brake / Steering`까지 HUD에 바로 올릴 수도 있지만,
  첫 버전은 너무 빽빽해질 가능성이 있다.
- 가장 안전한 기본안은 위 6개다.

---

## 6. 권장 UI 구조

## 6.1 루트 방향

권장 루트:
- `CanvasPanel`

권장 배치:
- 우상단 또는 좌상단 고정
- 적당한 패딩
- 너무 넓지 않은 세로형 카드

## 6.2 위젯 트리 권장 예시

```text
CanvasPanel
└── Border_RootCard
    └── VerticalBox_Root
        ├── Text_Title
        ├── Text_RuntimeReady
        ├── Text_DriveState
        ├── Text_Speed
        ├── Text_ForwardSpeed
        ├── Text_DeviceMode
        ├── Text_InputOwner
        └── Text_LastTransition
```

## 6.3 위젯 변수 이름 권장

- `Border_RootCard`
- `VerticalBox_Root`
- `Text_Title`
- `Text_RuntimeReady`
- `Text_DriveState`
- `Text_Speed`
- `Text_ForwardSpeed`
- `Text_DeviceMode`
- `Text_InputOwner`
- `Text_LastTransition`

주의:
- `Text1`, `Text2` 같은 이름은 쓰지 않는다.
- 나중에 Panel과 공통 패턴으로 갈 때도 이 이름들이 그대로 도움이 된다.

---

## 7. C++ 부모 위젯 구조

## 7.1 C++ 클래스 기준

코드 클래스 이름:
- `UCFVehicleDebugHudWidget`

에디터 부모 클래스 검색 이름:
- `CFVehicleDebugHudWidget`

주의:
- 에디터에서는 보통 `U` 접두사를 빼고 보게 된다.
- 부모 클래스 검색이나 타입 검색을 안내할 때는 에디터 표시 이름 기준으로 설명한다.

## 7.2 C++ 부모 위젯 변수

`CFVehicleDebugHudWidget` 안에는 아래 핵심 상태가 이미 있다.

- `VehiclePawnRef`
  - 역할: HUD가 디버그 정보를 읽어올 차량 Pawn 참조

- `CachedOverview`
  - 역할: 현재 HUD에 적용 중인 최신 Overview 캐시

- `bAutoRefreshEveryTick`
  - 역할: `NativeTick`에서 자동 갱신할지 결정

## 7.3 C++ 부모 위젯 함수

아래 함수는 이미 C++ 부모 위젯에 있다.

- `SetVehiclePawnRef`
  - 역할: HUD가 읽을 Pawn 참조를 설정하고 첫 갱신 수행

- `RefreshFromPawn`
  - 역할: `VehiclePawnRef`에서 최신 Overview를 읽어옴

- `ApplyVehicleDebugOverview`
  - 역할: 전달받은 `FCFVehicleDebugOverview` 값을 실제 텍스트 위젯에 반영

- `UpdateHudVisibility`
  - 역할: `ShouldShowVehicleDebugHud()` 기준으로 HUD 표시/숨김 처리

---

## 8. 데이터 연결 방식 권장안

이번 단계에서는 **UMG Text 바인딩 남용을 권장하지 않는다.**

권장 흐름은 아래다.

1. `BP_CFVehiclePawn`에서 HUD 생성
2. `SetVehiclePawnRef(self)` 호출
3. `CFVehicleDebugHudWidget`이 내부에서 `GetVehicleDebugOverview()` 호출
4. `CFVehicleDebugHudWidget`이 내부에서 텍스트 반영
5. `CFVehicleDebugHudWidget`이 내부에서 `ShouldShowVehicleDebugHud()` 호출
6. `CFVehicleDebugHudWidget`이 내부에서 `Set Visibility` 적용

이 방식을 권장하는 이유:

- 바인딩 함수가 많아지면 디버그가 어려워진다.
- 이후 Panel까지 갈 때 갱신 흐름을 통일하기 쉽다.
- HUD 갱신 로직이 C++에 있어 다음 세션에서도 구조가 덜 흔들린다.

---

## 9. 에디터 작업 순서

## 9.1 사용자 작업

1. 콘텐츠 브라우저(`Content Browser`)에서 `/Game/CarFight/UI`로 이동
2. 우클릭 -> 사용자 인터페이스(`User Interface`) -> 위젯 블루프린트(`Widget Blueprint`)
3. 이름을 `WBP_VehicleDebugHud`로 지정
4. 방금 만든 자산 열기
5. 클래스 설정(`Class Settings`) 선택
6. 부모 클래스(`Parent Class`)가 `CFVehicleDebugHudWidget`인지 확인
7. `CanvasPanel` 루트 유지
8. `Border` + `VerticalBox` 기반으로 작은 카드 레이아웃 구성
9. 아래 텍스트 위젯 이름을 C++와 정확히 맞춤
   - `Text_Title`
   - `Text_RuntimeReady`
   - `Text_DriveState`
   - `Text_Speed`
   - `Text_ForwardSpeed`
   - `Text_DeviceMode`
   - `Text_InputOwner`
   - `Text_LastTransition`

## 9.2 Codex가 이미 준비한 코드 연결점

현재 C++에서 바로 사용할 수 있는 함수:

- `GetVehicleDebugOverview()`
- `ShouldShowVehicleDebugHud()`
- `GetVehicleDebugSnapshot()`
- `SetVehiclePawnRef()`
- `RefreshFromPawn()`
- `ApplyVehicleDebugOverview()`
- `UpdateHudVisibility()`

즉, BP에서 HUD 함수 그래프를 따로 만들지 않아도
`Overview` 읽기와 HUD 표시 조건 분리는 바로 가능하다.

---

## 10. 블루프린트 구현 순서

## 10.1 `WBP_VehicleDebugHud` 디자이너 작업

아래 텍스트 권장 포맷에 맞춰 텍스트 블록만 배치한다.

- `Text_Title`
  - `Vehicle Debug HUD`

- `Text_RuntimeReady`
  - `Runtime: Ready` 또는 `Runtime: Not Ready`

- `Text_DriveState`
  - `State: <enum 문자열>`

- `Text_Speed`
  - `Speed: <값> km/h`

- `Text_ForwardSpeed`
  - `Forward: <값> km/h`

- `Text_DeviceMode`
  - `Input Mode: <enum 문자열>`

- `Text_InputOwner`
  - `Input Owner: <enum 문자열>`

- `Text_LastTransition`
  - `Last: <문자열>`

주의:
- 첫 버전은 enum 문자열을 그대로 써도 된다.
- 보기 불편하면 후속 단계에서 표시 전용 문자열 정리를 해도 된다.
- 텍스트 위젯 이름이 다르면 `BindWidgetOptional` 자동 연결이 되지 않는다.

---

## 11. `BP_CFVehiclePawn` 연결 권장안

현재 구조를 크게 흔들지 않는 가장 안전한 연결은 아래다.

1. 기존 `WBP_VehicleDebug` 생성 흐름은 제거 대상으로 본다.
2. 같은 위치 또는 바로 다음에 `Create Widget` 노드 추가
3. 클래스는 `WBP_VehicleDebugHud` 선택
4. 생성 결과를 `VehicleDebugHudRef` 변수에 저장
5. `VehicleDebugHudRef -> SetVehiclePawnRef(self)` 호출
6. `VehicleDebugHudRef -> Add to Viewport`

주의:
- 이번 단계에서는 UI 소유권을 `PlayerController`로 옮기지 않는다.
- 아직은 `BP_CFVehiclePawn` 직접 생성 구조를 유지하는 편이 안전하다.
- `SetVehiclePawnRef`는 `WBP_VehicleDebugHud`의 부모가 `CFVehicleDebugHudWidget`일 때만 노드로 보인다.
- 최종 정리 단계에서는 기존 `WBP_VehicleDebug` 생성/보관 노드를 제거한다.

---

## 12. 스타일 권장안

## 12.1 첫 버전 기본안

- 반투명 어두운 배경
- 작은 흰색/회색 텍스트
- 제목은 조금 더 밝게
- `Runtime Ready`는 색으로 약간 구분

예:
- Ready: 청록/녹색 계열
- Not Ready: 주황/빨강 계열

## 12.2 피해야 할 것

- 너무 큰 패널
- 과한 애니메이션
- 과도한 외곽선
- Panel과 거의 같은 정보량

핵심:
- HUD는 “한눈에 보는 카드”여야 한다.

---

## 13. 테스트 방법

## 13.1 기능 테스트

1. `BP_CFVehiclePawn`에서 `bEnableDriveStateOnScreenDebug = true`
2. `bShowVehicleDebugHud = true`
3. PIE 실행
4. HUD가 화면에 보이는지 확인
5. 정지 상태에서 `Speed / State / Runtime` 확인
6. 전진 입력 시 `Speed / Forward / State` 값 변화 확인
7. 입력 경로 변경 시 `InputOwner` 변화 확인

## 13.2 가시성 테스트

1. `bShowVehicleDebugHud = false`
2. PIE 실행
3. HUD가 숨겨지는지 확인
4. 기존 `WBP_VehicleDebug`가 더 이상 화면에 보이지 않는지 확인

주의:
- 개발용 온스크린 문자열 메시지는 `bEnableVehicleDebugOnScreenMessage`로 별도 제어한다.
- `bEnableDriveStateOnScreenDebug`를 켰다고 해서 `AddOnScreenDebugMessage` 기반 문자열이 자동으로 나오지 않아야 정상이다.

## 13.3 회귀 테스트

1. HUD 추가 후에도 기존 `WBP_VehicleDebug`가 다시 나타나지 않는지 확인
2. 로컬 제어 차량에서만 나타나는지 확인
3. `bEnableDriveStateOnScreenDebug`를 끄면 HUD도 같이 사라지는지 확인

---

## 14. 예상 결과

정상 결과는 아래와 같다.

1. 화면 구석에 작은 HUD 카드가 보인다.
2. 속도, 현재 상태, 입력 모드, 런타임 준비 상태를 빠르게 읽을 수 있다.
3. 기존 `WBP_VehicleDebug`를 제거해도 HUD만으로 핵심 상태를 읽을 수 있다.
4. 이후 Panel을 붙여도 HUD는 그대로 유지 가능한 구조가 된다.

---

## 15. 다음 단계

HUD 1차 버전이 붙으면 다음은 아래 순서를 권장한다.

1. `WBP_VehicleDebugHud` 실제 동작 확인
2. `Overview` 값 중 과한 항목 정리
3. `WBP_VehicleDebugPanel` 설계 시작
4. 공통 `FieldRow / CategoryBlock` 도입

---

## 16. Changelog

### v0.2.1 - 2026-04-23
- 기존 `WBP_VehicleDebug`와의 공존 표현을 제거하고 최종 제거 방향으로 문구 정리
- HUD 단독 진단 가능 여부와 레거시 위젯 비표시 검증 항목 추가

### v0.2.0 - 2026-04-23
- HUD 구현 기본안을 순수 BP 함수 방식에서 `C++ 부모 위젯 + 얇은 WBP 자식` 방식으로 전환
- `CFVehicleDebugHudWidget` 부모 클래스 기준 작업 순서 추가
- `BindWidgetOptional` 전제 조건인 텍스트 위젯 이름 규칙 보강
- `BP_CFVehiclePawn` 연결 단계를 부모 클래스 기반 흐름으로 수정

### v0.1.0 - 2026-04-22
- `WBP_VehicleDebugHud` 구현 가이드 신규 작성
- HUD 1차 목표, 권장 표시 항목, 위젯 구조, BP 함수 흐름 정리
- 현재 C++ 공개 API와 연결하는 방식 정리
- 테스트 방법과 예상 결과 추가

---

## 17. 마이그레이션 지침

- 현재 HUD는 `Overview` 카테고리만 읽는 것을 기본으로 한다.
- HUD 로직은 `CFVehicleDebugHudWidget` 부모 클래스에 두고, `WBP_VehicleDebugHud`는 레이아웃만 맡긴다.
- `WBP_VehicleDebug`는 최종적으로 제거 대상이며, 이 문서는 더 이상 Legacy Text 공존을 기본 전제로 두지 않는다.
- HUD에 Drive/Input 상세를 무리하게 넣지 말고, 상세 정보는 이후 `Panel`로 보낸다.
- 나중에 `PlayerController` 소유 구조로 옮기더라도, `GetVehicleDebugOverview()`와 `ShouldShowVehicleDebugHud()` 연결 방식은 그대로 재사용할 수 있다.
