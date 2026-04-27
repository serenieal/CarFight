# VehicleDebug Panel 구현 가이드

- Version: 0.3.0
- Date: 2026-04-27
- Status: Panel 1차 마감
- Scope: `CFVehicleDebugPanelWidget` C++ 부모 클래스와 `WBP_VehicleDebugPanel` 자식 위젯을 안전하게 도입하기 위한 구현 가이드

---

## 1. 문서 목적

이 문서는 VehicleDebug v2의 Phase 3에서,

- `WBP_VehicleDebugPanel`을 어디에 만들고
- 어떤 카테고리를 어떤 구조로 올리고
- 어떤 C++ 부모 클래스를 기반으로 붙이며
- 어떤 방식으로 갱신해야 하는지

를 초보자도 그대로 따라 만들 수 있도록 정리한 실무 가이드다.

이번 단계의 목표는 화려한 패널이 아니라,
**HUD보다 자세하지만 문자열 덩어리보다 읽기 쉬운 카테고리형 상세 패널을 안전하게 붙이고, 이후 확장이 쉬운 동적 Section 구조로 고정하는 것**이다.

---

## 2. 현재 상태 한 줄 요약

현재 C++ 기준으로는 아래 준비가 끝난 상태다.

- `FCFVehicleDebugSnapshot`에 `Overview / Drive / Input / Runtime` 카테고리 존재
- `GetVehicleDebugOverview()` 공개 API 존재
- `GetVehicleDebugDrive()` 공개 API 존재
- `GetVehicleDebugInput()` 공개 API 존재
- `GetVehicleDebugRuntime()` 공개 API 존재
- `ShouldShowVehicleDebugPanel()` 공개 API 존재
- `UCFVehicleDebugPanelWidget` C++ 부모 위젯 클래스 존재
- `FCFVehicleDebugPanelViewData` 기반 ViewData 구조 존재
- `UCFVehicleDebugSectionWidget` 공통 Section 부모 위젯 클래스 존재
- `WBP_VehicleDebugSection` 공통 Section 자식 위젯 존재
- `UCFVehicleDebugFieldRowWidget` 공통 Field Row 부모 위젯 클래스 존재
- `WBP_VehicleDebugFieldRow` 공통 Field Row 자식 위젯 존재
- `WBP_VehicleDebugPanel`은 `VerticalBox_DynamicSectionHost`를 통해 동적 Section을 표시한다.

즉,
현재 단계는 **정적 CategoryList Panel에서 동적 Section Panel로 전환 완료된 상태**다.

---

## 3. 이번 단계 목표

이번 `WBP_VehicleDebugPanel` 1차 목표는 아래다.

1. `Overview / Drive / Input / Runtime`를 카테고리별로 분리해 보여준다.
2. HUD보다 상세하지만 구조는 단순하게 유지한다.
3. 패널 갱신 로직은 C++ 부모 클래스에 둔다.
4. 레거시 `WBP_VehicleDebug` 없이도 상세 진단이 가능해야 한다.
5. 카테고리별 접기/펼치기가 가능해야 한다.
6. 새 섹션 추가 시 루트 WBP에 Button/Text/Container를 계속 추가하지 않아야 한다.

이번 단계에서 하지 않는 것:

- 접기/펼치기 애니메이션
- 검색 기능
- Recent Events 표시
- 색상 규칙 자동화

---

## 4. 권장 자산 경로

### 신규
- `UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h`
- `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp`
- `/Game/CarFight/UI/WBP_VehicleDebugPanel`

최종 운영 구조는 아래 조합을 사용한다.

- `CFVehicleDebugPanelWidget` + `WBP_VehicleDebugPanel`
- `CFVehicleDebugSectionWidget` + `WBP_VehicleDebugSection`
- `CFVehicleDebugFieldRowWidget` + `WBP_VehicleDebugFieldRow`
- `FCFVehicleDebugPanelViewData` / `FCFVehicleDebugSectionViewData`

주의:
- 기존 정적 `VerticalBox_CategoryList`는 동적 섹션 표시 확인 후 삭제한다.
- 에디터 계층구조의 눈 아이콘으로 숨기는 것은 PIE 런타임 숨김을 보장하지 않으므로 제거 기준으로 삼지 않는다.

---

## 5. Panel에 올릴 값

이번 1차 권장 카테고리와 항목은 아래다.

### 5.1 Overview
- `bRuntimeReady`
- `CurrentDriveState`
- `SpeedKmh`
- `ForwardSpeedKmh`
- `DeviceMode`
- `InputOwner`

### 5.2 Drive
- `CurrentDriveState`
- `PreviousDriveState`
- `bDriveStateChangedThisFrame`
- `SpeedKmh`
- `ForwardSpeedKmh`
- `Throttle`
- `Brake`
- `Steering`
- `bHandbrake`

### 5.3 Input
- `DeviceMode`
- `InputOwner`
- `MoveZone`
- `MoveIntent`
- `MoveRaw`
- `MoveMagnitude`
- `MoveAngle`
- `bUsedBlackZoneHold`

### 5.4 Runtime
- `bRuntimeReady`
- `bHasDriveComponent`
- `bHasWheelSyncComponent`
- `RuntimeSummary`
- `LastInitAttemptSummary`
- `LastValidationSummary`

권장 표시 방식:
- `Overview` 상위 카테고리에서는 긴 `LastTransitionShortText`를 직접 노출하지 않는다.
- `Drive` 상위 카테고리에서는 긴 `DriveStateTransitionSummary`를 직접 노출하지 않는다.
- `Overview Last Transition`은 Overview 내부 하위 섹션으로 분리한다.
- `Drive Transition`은 Drive 내부 하위 섹션으로 분리한다.
- `Runtime` 상위 카테고리에는 짧은 상태만 둔다.
- `Runtime Summary`
- `Last Init`
- `Last Validation`
위 3개는 Runtime 내부 하위 섹션으로 분리하고 각각 접기/펼치기를 둔다.

---

## 6. 권장 UI 구조

## 6.1 루트 방향

권장 루트:
- `CanvasPanel`

권장 배치:
- 화면 좌측 또는 우측 가장자리
- HUD보다 크고, 전체 화면을 덮지 않는 패널
- 세로 스크롤 가능한 구조

## 6.2 최종 위젯 트리

```text
CanvasPanel
└── Border_RootPanel
    └── ScrollBox_Root
        └── VerticalBox_DynamicSectionHost
```

## 6.3 위젯 변수 이름 권장

- `Border_RootPanel`
- `ScrollBox_Root`
- `VerticalBox_DynamicSectionHost`

주의:
- `VerticalBox_DynamicSectionHost` 이름은 C++의 `BindWidgetOptional` 이름과 정확히 일치해야 한다.
- 기존 `VerticalBox_CategoryList`와 그 하위 정적 위젯은 최종 운영 구조에서 사용하지 않는다.

## 6.4 `WBP_VehicleDebugSection` 위젯 트리

```text
CanvasPanel
└── VerticalBox_Root
    ├── Button_Header
    │   └── Text_Title
    ├── Container_Body
    │   ├── VerticalBox_FieldHost
    │   └── Text_Body
    └── VerticalBox_ChildSectionHost
```

주의:
- `WBP_VehicleDebugSection` 부모 클래스는 `CFVehicleDebugSectionWidget`이다.
- `Button_Header`의 `OnClicked` 이벤트는 이벤트 그래프에서 `ToggleSectionExpanded`로 연결한다.
- `Container_Body`는 팔레트 이름이 아니라 이름을 바꾼 `Vertical Box`다.
- `VerticalBox_FieldHost`는 일반 `라벨: 값` Field Row를 붙이는 호스트다.
- `Text_Body`는 긴 본문 문자열 전용 fallback/본문 표시용으로 유지한다.
- `ChildSectionWidgetClass`는 비워둬도 C++가 현재 Section 위젯 클래스를 fallback으로 사용한다.
- 하위 섹션은 C++에서 `ChildSectionIndentLeft` / `ChildSectionTopPadding` 슬롯 여백을 자동 적용해 상위 카테고리와 시각적으로 구분한다.

## 6.5 `WBP_VehicleDebugFieldRow` 위젯 트리

```text
CanvasPanel
└── HorizontalBox_Root
    ├── Text_Label
    └── Text_Value
```

대안:
- 더 단순하게 시작하려면 `Text_Combined` 하나만 둬도 된다.
- `Text_Label` / `Text_Value`가 있으면 라벨과 값을 나눠 정렬할 수 있다.
- `Text_Combined`가 있으면 `Label: Value` 형태로 합쳐 표시한다.

주의:
- `WBP_VehicleDebugFieldRow` 부모 클래스는 `CFVehicleDebugFieldRowWidget`이다.
- `Text_Label`, `Text_Value`, `Text_Combined`는 모두 선택 사항이지만, 최소 하나는 있어야 화면에 값이 보인다.

---

## 7. C++ 부모 위젯 구조

## 7.1 C++ 클래스 기준

코드 클래스 이름:
- `UCFVehicleDebugPanelWidget`

에디터 부모 클래스 검색 이름:
- `CFVehicleDebugPanelWidget`

주의:
- 에디터에서는 보통 `U` 접두사를 빼고 보게 된다.
- 부모 클래스 검색이나 타입 검색을 안내할 때는 에디터 표시 이름 기준으로 설명한다.

## 7.2 C++ 부모 위젯 변수

`CFVehicleDebugPanelWidget` 안에는 아래 핵심 상태가 이미 있다.

- `VehiclePawnRef`
- `CachedOverview`
- `CachedDrive`
- `CachedInput`
- `CachedRuntime`
- `bAutoRefreshEveryTick`
- `bAutoEnterInteractionModeWhenVisible`
- `bIsPanelInteractionModeActive`
- `bIsOverviewExpanded`
- `bIsDriveExpanded`
- `bIsInputExpanded`
- `bIsRuntimeExpanded`
- `bIsRuntimeSummaryExpanded`
- `bIsRuntimeLastInitExpanded`
- `bIsRuntimeLastValidationExpanded`

## 7.3 C++ 부모 위젯 함수

아래 함수는 이미 C++ 부모 위젯에 있다.

- `SetVehiclePawnRef`
- `RefreshFromPawn`
- `ApplyVehicleDebugOverview`
- `ApplyVehicleDebugDrive`
- `ApplyVehicleDebugInput`
- `ApplyVehicleDebugRuntime`
- `UpdatePanelVisibility`
- `EnterPanelInteractionMode`
- `ExitPanelInteractionMode`
- `ToggleOverviewExpanded`
- `ToggleDriveExpanded`
- `ToggleInputExpanded`
- `ToggleRuntimeExpanded`
- `ToggleRuntimeSummaryExpanded`
- `ToggleRuntimeLastInitExpanded`
- `ToggleRuntimeLastValidationExpanded`

주의:
- 위 `ToggleOverviewExpanded` 계열 함수는 정적 Panel fallback 호환용으로 남아 있다.
- 최종 운영 입력은 `WBP_VehicleDebugSection`의 `Button_Header -> ToggleSectionExpanded` 연결을 기준으로 한다.

---

## 8. 데이터 연결 방식 권장안

이번 단계에서는 **Panel 본문 문자열 조합과 표시 갱신을 C++ 부모 위젯에서 처리하는 것**을 권장한다.

권장 흐름은 아래다.

1. `BP_CFVehiclePawn`에서 Panel 생성
2. `SetVehiclePawnRef(self)` 호출
3. `CFVehicleDebugPanelWidget`이 내부에서 `GetVehicleDebugOverview()` 호출
4. `CFVehicleDebugPanelWidget`이 내부에서 `GetVehicleDebugDrive()` 호출
5. `CFVehicleDebugPanelWidget`이 내부에서 `GetVehicleDebugInput()` 호출
6. `CFVehicleDebugPanelWidget`이 내부에서 `GetVehicleDebugRuntime()` 호출
7. `CFVehicleDebugPanelWidget`이 내부에서 카테고리별 본문 텍스트 반영
8. `CFVehicleDebugPanelWidget`이 내부에서 `ShouldShowVehicleDebugPanel()` 호출

이 방식을 권장하는 이유:

- Panel은 HUD보다 필드 수가 많아 BP 그래프가 빨리 비대해진다.
- 카테고리별 형식 변경을 C++에서 더 안전하게 관리할 수 있다.

중요:
- 카테고리 Header 버튼 입력은 `WBP_VehicleDebugSection` 이벤트 그래프에서 `ToggleSectionExpanded`를 호출하는 구조를 기준으로 한다.
- `WBP_VehicleDebugPanel` 루트는 동적 섹션 호스트만 제공하고, 개별 섹션 입력은 공통 Section 위젯이 처리한다.
- 일반 `라벨: 값` 항목은 `WBP_VehicleDebugFieldRow`가 처리하고, 긴 본문은 `Text_Body`가 처리한다.
- Field Row 스타일을 바꾸더라도 데이터 원본을 다시 손대지 않아도 된다.

## 8.1 Header 입력 연결 기준

현재 동적 Section 기준 Header 입력 연결은 아래 방식이 가장 안전하다.

1. `WBP_VehicleDebugSection` 열기
2. `Button_Header` 선택
3. 이벤트(On Clicked) 추가
4. `ToggleSectionExpanded` 호출 노드 연결

이유:
- 현재 프로젝트에서 실제로 동작 확인된 경로는 WBP 이벤트 그래프의 `On Clicked` 이벤트다.
- C++ 자동 바인딩에 기대지 않고, 공통 Section 위젯 내부에서 직접 호출하면 디버깅 경로가 단순하다.
- 루트 Panel에 섹션별 버튼 이벤트를 계속 추가하지 않아도 된다.

## 8.2 Field Row 렌더링 기준

현재 Field Row 입력 연결은 C++에서 처리한다.

1. `WBP_VehicleDebugSection`의 `FieldRowWidgetClass`에 `WBP_VehicleDebugFieldRow` 지정
2. `WBP_VehicleDebugSection` 내부에 `VerticalBox_FieldHost` 배치
3. C++가 `FCFVehicleDebugSectionViewData.FieldArray`를 순회
4. 필요한 개수만큼 `WBP_VehicleDebugFieldRow` 생성
5. 이후 Tick에서는 기존 Row 위젯을 재사용하고 텍스트만 갱신

Fallback:
- `VerticalBox_FieldHost` 또는 `FieldRowWidgetClass`가 없으면 기존처럼 FieldArray를 `Text_Body` 문자열로 합쳐 표시한다.

---

## 9. 에디터 작업 순서

## 9.1 사용자 작업

1. 콘텐츠 브라우저(`Content Browser`)에서 `/Game/CarFight/UI`로 이동
2. 우클릭 -> 사용자 인터페이스(`User Interface`) -> 위젯 블루프린트(`Widget Blueprint`)
3. 이름을 `WBP_VehicleDebugPanel`로 지정
4. 방금 만든 자산 열기
5. 클래스 설정(`Class Settings`) 선택
6. 부모 클래스(`Parent Class`)가 `CFVehicleDebugPanelWidget`인지 확인
7. `CanvasPanel` 루트 유지
8. `Border` + `ScrollBox` + `VerticalBox` 기반으로 패널 레이아웃 구성
9. `ScrollBox_Root` 아래에 `VerticalBox_DynamicSectionHost`만 둔다.
10. 기존 `VerticalBox_CategoryList`가 남아 있다면 삭제한다.
11. 디테일(Details)에서 `DynamicSectionWidgetClass`를 `WBP_VehicleDebugSection`으로 지정한다.

## 9.2 `WBP_VehicleDebugFieldRow` 사용자 작업

1. 콘텐츠 브라우저(`Content Browser`)에서 `/Game/CarFight/UI`로 이동
2. 우클릭 -> 사용자 인터페이스(`User Interface`) -> 위젯 블루프린트(`Widget Blueprint`)
3. 이름을 `WBP_VehicleDebugFieldRow`로 지정
4. 방금 만든 자산 열기
5. 클래스 설정(`Class Settings`) 선택
6. 부모 클래스(`Parent Class`)를 `CFVehicleDebugFieldRowWidget`으로 지정
7. `Horizontal Box`를 추가하고 이름을 `HorizontalBox_Root`로 변경
8. `HorizontalBox_Root` 아래에 `Text Block` 2개 추가
9. 첫 번째 이름을 `Text_Label`로 변경
10. 두 번째 이름을 `Text_Value`로 변경
11. 컴파일(Compile) / 저장(Save)

## 9.3 `WBP_VehicleDebugSection` 추가 설정

1. `WBP_VehicleDebugSection` 열기
2. `Container_Body` 아래에 `Vertical Box` 추가
3. 이름을 `VerticalBox_FieldHost`로 변경
4. `Text_Body`는 삭제하지 말고 긴 본문 표시용으로 유지
5. 디테일(Details)에서 `FieldRowWidgetClass`를 `WBP_VehicleDebugFieldRow`로 지정
6. 컴파일(Compile) / 저장(Save)

## 9.2 Codex가 이미 준비한 코드 연결점

현재 C++에서 바로 사용할 수 있는 함수:

- `GetVehicleDebugOverview()`
- `GetVehicleDebugDrive()`
- `GetVehicleDebugInput()`
- `GetVehicleDebugRuntime()`
- `ShouldShowVehicleDebugPanel()`
- `SetVehiclePawnRef()`
- `RefreshFromPawn()`
- `ApplyVehicleDebugOverview()`
- `ApplyVehicleDebugDrive()`
- `ApplyVehicleDebugInput()`
- `ApplyVehicleDebugRuntime()`
- `UpdatePanelVisibility()`
- `EnterPanelInteractionMode()`
- `ExitPanelInteractionMode()`
- `ToggleOverviewExpanded()`
- `ToggleDriveExpanded()`
- `ToggleInputExpanded()`
- `ToggleRuntimeExpanded()`

즉, BP에서 Panel 함수 그래프를 따로 만들지 않아도
카테고리 읽기와 Panel 표시 조건 분리는 바로 가능하다.

주의:
- 위 목록은 정적 fallback 경로와 일부 호환 함수를 포함한다.
- 현재 운영 구조의 섹션 접기/펼치기는 `WBP_VehicleDebugSection`의 `ToggleSectionExpanded()`가 담당한다.

---

## 10. 블루프린트 구현 순서

## 10.1 `WBP_VehicleDebugPanel` 디자이너 작업

`WBP_VehicleDebugPanel`에는 개별 카테고리 텍스트를 직접 배치하지 않는다.
카테고리 표시는 `WBP_VehicleDebugSection` 인스턴스가 동적으로 담당한다.

주의:
- 각 `Header`는 `WBP_VehicleDebugSection` 내부의 `Button_Header`로 둔다.
- 본문 컨테이너는 `Container_Body`라는 이름의 `Vertical Box`로 둔다.
- 본문 텍스트는 `Text_Body`에 `라벨: 값` 멀티라인 문자열로 표시한다.
- 줄 간격과 패딩만 맞아도 첫 버전 가독성은 충분히 확보할 수 있다.
- 패널 바깥 클릭 종료는 `Border_RootPanel` 바깥 위치를 C++ 부모 위젯이 직접 감지하는 현재 구조를 기준으로 한다.

---

## 11. `BP_CFVehiclePawn` 연결 권장안

현재 구조를 크게 흔들지 않는 가장 안전한 연결은 아래다.

1. `Create Widget` 노드 추가
2. 클래스는 `WBP_VehicleDebugPanel` 선택
3. 생성 결과를 `VehicleDebugPanelRef` 변수에 저장
4. `VehicleDebugPanelRef -> SetVehiclePawnRef(self)` 호출
5. `VehicleDebugPanelRef -> Add to Viewport`

주의:
- `SetVehiclePawnRef`는 `WBP_VehicleDebugPanel`의 부모가 `CFVehicleDebugPanelWidget`일 때만 노드로 보인다.
- 초기에는 HUD와 Panel을 동시에 띄워도 되지만, 위치 충돌은 피해야 한다.
- 현재 동적 Section 구조에서는 Header 버튼 입력을 `WBP_VehicleDebugSection` 이벤트 그래프에서 `ToggleSectionExpanded`로 연결한다.
- 패널 바깥 클릭 종료는 `Border_RootPanel` 기준 바깥 영역 클릭을 C++ 부모 위젯이 직접 감지한다.

---

## 12. 스타일 권장안

## 12.1 첫 버전 기본안

- 반투명 배경
- 섹션 제목은 조금 더 굵게
- 본문은 작은 밝은 회색 텍스트
- 카테고리 사이에 여백 분리
- 스크롤바는 너무 튀지 않게 얇게 유지

## 12.2 피해야 할 것

- 제목/본문 색을 너무 많이 섞는 것
- 한 카테고리를 너무 넓게 벌리는 것
- 첫 버전부터 카드 내부에 또 카드 구조를 중첩하는 것

핵심:
- 1차 Panel은 “자세하지만 단순한 진단 패널”이어야 한다.

---

## 13. 테스트 방법

## 13.1 기능 테스트

1. `BP_CFVehiclePawn`에서 `bEnableDriveStateOnScreenDebug = true`
2. `bShowVehicleDebugPanel = true`
3. PIE 실행
4. Panel이 화면에 보이는지 확인
5. 주행/정지 상태에서 `Drive` 값이 변하는지 확인
6. 입력 변경 시 `Input` 값이 변하는지 확인
7. 런타임 준비 여부가 `Runtime` 섹션에 반영되는지 확인
8. 각 Header 버튼 클릭 시 해당 카테고리가 접히고 다시 펴지는지 확인
9. 패널 바깥 클릭 시 마우스 커서가 사라지고 게임 입력으로 복귀하는지 확인
10. `VerticalBox_CategoryList`가 삭제되어 기존 정적 텍스트가 중복 표시되지 않는지 확인
11. 일반 항목이 `Text_Body` 긴 문자열이 아니라 Field Row 단위로 표시되는지 확인

## 13.2 가시성 테스트

1. `bShowVehicleDebugPanel = false`
2. PIE 실행
3. Panel이 숨겨지는지 확인
4. `bShowVehicleDebugHud = true`면 HUD는 계속 남는지 확인

## 13.3 사용성 테스트

1. HUD만으로 부족한 정보가 Panel에서 보완되는지 확인
2. `Drive / Input / Runtime` 원인 파악이 문자열 덩어리보다 쉬운지 확인
3. 긴 요약 문자열이 레이아웃을 깨지 않는지 확인
4. `Runtime Summary / Last Init / Last Validation`을 각각 개별로 열고 닫을 수 있는지 확인

## 13.4 상호작용 테스트

1. PIE 실행 후 패널 위 클릭 시 마우스가 유지되는지 확인
2. 패널 바깥 클릭 시 `ExitPanelInteractionMode()`가 동작해 마우스가 사라지고 게임 입력으로 복귀하는지 확인
3. 패널을 다시 표시했을 때 `EnterPanelInteractionMode()`가 다시 동작하는지 확인

---

## 14. 예상 결과

정상 결과는 아래와 같다.

1. HUD보다 자세한 Panel이 별도로 보인다.
2. `Overview / Drive / Input / Runtime`가 명확히 분리된다.
3. 레거시 텍스트 없이도 현재 상태와 원인 분석이 가능해진다.
4. 이후 새 섹션은 ViewData에 추가하는 방식으로 확장 가능하다.
5. 루트 WBP에 정적 Button/Text/Container를 계속 추가하지 않아도 된다.
6. 일반 `라벨: 값` 항목은 Field Row로 분리되고 긴 본문만 `Text_Body`에 남는다.

---

## 15. 다음 단계

Panel 1차가 붙으면 다음은 아래 순서를 권장한다.

1. Recent Events 구조 설계 시작
2. 표시 정책 정리
3. 필요 시 Field Row 스타일 개선
4. 최종 현재 상태 문서를 `Systems/UI/VehicleDebug.md`로 승격 반영

---

## 15.1 Panel 1차 완료 기준

아래가 모두 충족되면 Panel 1차를 완료로 본다.

1. `Overview / Drive / Input / Runtime` 상위 카테고리가 접기/펼치기 된다.
2. `Runtime Summary / Last Init / Last Validation` 하위 섹션이 개별 접기/펼치기 된다.
3. 상위 카테고리 입력은 `WBP_VehicleDebugPanel` 이벤트 그래프에서 `Toggle...` 함수 호출로 연결된다.
4. `Overview Last Transition`과 `Drive Transition`은 각각 하위 섹션으로 분리된다.
5. Runtime 상위 본문은 짧은 상태만 유지하고, 긴 텍스트는 하위 섹션으로 분리된다.
6. HUD와 Panel이 동시에 있어도 역할이 겹치지 않고, Panel이 상세 진단을 담당한다.
7. 일반 `라벨: 값` 항목은 `WBP_VehicleDebugFieldRow`로 렌더링된다.
8. 하위 섹션은 들여쓰기되어 상위 카테고리와 구분된다.

## 15.2 Panel 1차 마감 결과

아래 항목은 실제 PIE와 AssetDump로 확인했다.

1. `WBP_VehicleDebugPanel` 부모 클래스는 `CFVehicleDebugPanelWidget`이다.
2. `WBP_VehicleDebugPanel`은 `VerticalBox_DynamicSectionHost`를 사용한다.
3. `DynamicSectionWidgetClass`는 `WBP_VehicleDebugSection`이다.
4. 기존 정적 `VerticalBox_CategoryList`는 제거되었다.
5. `WBP_VehicleDebugSection` 부모 클래스는 `CFVehicleDebugSectionWidget`이다.
6. `Button_Header -> ToggleSectionExpanded` 연결이 존재한다.
7. `FieldRowWidgetClass`는 `WBP_VehicleDebugFieldRow`이다.
8. `WBP_VehicleDebugFieldRow` 부모 클래스는 `CFVehicleDebugFieldRowWidget`이다.
9. 일반 필드는 Row 단위로 표시되고, 긴 Summary는 `Text_Body` 경로로 표시된다.
10. 하위 섹션 들여쓰기 동작이 확인되었다.
11. `CarFight_ReEditor Win64 Development` 빌드가 성공했다.

## 16. Changelog

### v0.3.0 - 2026-04-27
- Panel 1차 마감 결과를 문서에 고정했다.
- AssetDump/PIE 기준 확인된 `Panel -> Section -> FieldRow` 운영 구조를 완료 기준으로 정리했다.
- 정적 fallback 설명과 현재 운영 구조가 섞이지 않도록 주의 문구를 보강했다.
- 다음 단계를 Recent Events 구조 설계와 표시 정책 정리로 갱신했다.

### v0.2.3 - 2026-04-27
- 하위 섹션 버튼이 상위 카테고리 버튼과 같은 정렬로 보여 구분이 어려운 문제를 반영했다.
- `CFVehicleDebugSectionWidget` v1.2.0 기준으로 하위 Section 슬롯에 기본 왼쪽 들여쓰기와 위쪽 여백을 적용하는 운영 기준을 추가했다.

### v0.2.2 - 2026-04-27
- AssetDump 확인 결과 `ChildSectionWidgetClass`가 비어 있을 수 있음을 반영했다.
- `CFVehicleDebugSectionWidget` v1.1.1 기준으로 하위 Section 위젯 클래스가 비어 있으면 현재 Section 위젯 클래스를 fallback으로 사용하는 운영 기준을 추가했다.

### v0.2.1 - 2026-04-27
- `CFVehicleDebugFieldRowWidget`와 `WBP_VehicleDebugFieldRow` 도입 절차를 추가했다.
- `WBP_VehicleDebugSection`에 `VerticalBox_FieldHost`와 `FieldRowWidgetClass`를 설정하는 기준을 문서화했다.
- Field Row 준비가 안 된 경우 기존 `Text_Body` 문자열 렌더링으로 fallback되는 운영 기준을 추가했다.

### v0.2.0 - 2026-04-24
- `WBP_VehicleDebugPanel` 최종 구조를 `VerticalBox_DynamicSectionHost` 단일 호스트 기준으로 갱신했다.
- `WBP_VehicleDebugSection` 공통 Section 위젯과 `DynamicSectionWidgetClass` 설정 절차를 가이드에 반영했다.
- 기존 정적 `VerticalBox_CategoryList`는 동적 섹션 표시 확인 후 삭제하는 것으로 마이그레이션 기준을 고정했다.

### v0.1.8 - 2026-04-24
- `Overview Last Transition`과 `Drive Transition`을 상위 본문에서 분리해 각각 하위 섹션으로 여는 구조를 가이드에 반영했다.
- `WBP_VehicleDebugPanel` 위젯 트리, 위젯 이름, Header 입력 연결 규칙을 새 하위 섹션 기준으로 갱신했다.

### v0.1.7 - 2026-04-24
- 실제 `WBP_VehicleDebugPanel` 어셋덤프 결과를 기준으로 Header 입력 연결 구조를 문서에 다시 고정했다.
- `## 8.1 Header 입력 연결 기준` 섹션을 본문 흐름 안으로 이동해 문서 순서를 바로잡았다.
- 현재 실제 WBP 구조와 맞지 않는 중간 설명 대신, 덤프 기준 확인 가능한 경로를 문서에 반영했다.

### v0.1.6 - 2026-04-24
- `Button_Backdrop` 의존 경로를 제거하고, 패널 바깥 클릭은 `Border_RootPanel` 바깥 위치를 C++ 부모 위젯이 직접 감지하는 현재 구조로 문서를 정리했다.
- 현재 실제 동작 경로와 무관한 백드롭 배치/이벤트 설명을 삭제했다.

### v0.1.5 - 2026-04-23
- Panel 1차 마감 기준을 문서에 추가했다.
- Header 버튼 입력 책임을 `WBP_VehicleDebugPanel` 이벤트 그래프로 정리하고, C++ 부모 위젯은 상태/표시 책임에 집중하는 현재 구조를 문서에 명시했다.
- Runtime 3분할 하위 섹션 구조를 포함한 현재 운영 방식을 완료 기준으로 고정했다.

### v0.1.3 - 2026-04-23
- Panel 상호작용 모드 가이드를 추가했다.
- `EnterPanelInteractionMode()` / `ExitPanelInteractionMode()` 사용 방식을 문서에 반영했다.
- PIE에서 마우스 커서 유지/복귀 흐름을 테스트 항목과 마이그레이션 지침에 반영했다.

### v0.1.4 - 2026-04-23
- `Runtime Summary`, `Last Init`, `Last Validation`을 Runtime 내부 하위 섹션으로 분리하는 2단계 접기/펼치기 구조를 반영했다.
- Runtime 상위 카테고리에는 짧은 상태만 남기고, 긴 런타임 요약은 하위 섹션 버튼으로 여는 구조를 가이드에 추가했다.

### v0.1.2 - 2026-04-23
- 카테고리별 접기/펼치기 구조를 문서에 반영했다.
- `Button_OverviewHeader`, `Button_DriveHeader`, `Button_InputHeader`, `Button_RuntimeHeader` 이름 규칙을 추가했다.
- `Vertical Box`를 추가한 뒤 `Container_OverviewBody`, `Container_DriveBody`, `Container_InputBody`, `Container_RuntimeBody`로 이름을 바꾸는 작업 방식을 명시했다.

### v0.1.1 - 2026-04-23
- 긴 Runtime/Transition 문자열이 Panel 레이아웃을 깨지 않도록 Panel 전용 멀티라인 포맷 방침을 추가했다.
- `Input` / `Runtime` 기본 펼침 상태를 테스트 가능한 기본값으로 보는 운영 판단을 문서에 반영했다.

### v0.1.0 - 2026-04-23
- `CFVehicleDebugPanelWidget + WBP_VehicleDebugPanel` 구현 가이드 신규 작성
- Panel 1차 목표, 카테고리 표시 항목, 위젯 구조, BP 연결 방식 정리
- 현재 C++ 공개 API와 Panel 부모 위젯 연결 방식 정리
- 테스트 방법과 예상 결과 추가

---

## 17. 마이그레이션 지침

- 현재 Panel은 `Overview / Drive / Input / Runtime`를 동적 Section 위젯으로 표시하는 것을 기본으로 한다.
- Panel 로직은 `CFVehicleDebugPanelWidget` 부모 클래스에 두고, `WBP_VehicleDebugPanel`은 레이아웃만 맡긴다.
- 상호작용 모드는 `EnterPanelInteractionMode()` / `ExitPanelInteractionMode()`와 패널 바깥 클릭 감지로 제어한다.
- 접기/펼치기 상태는 Section ViewData와 공통 Section 위젯 상태로 관리한다.
- `VerticalBox_CategoryList`는 운영 구조에서 제거하고 `VerticalBox_DynamicSectionHost`만 유지한다.
- 일반 필드는 `WBP_VehicleDebugFieldRow`로 표시하고, 긴 본문은 `Text_Body` 멀티라인 표시로 유지한다.
- 나중에 `PlayerController` 소유 구조로 옮기더라도, `GetVehicleDebugDrive()` / `GetVehicleDebugInput()` / `GetVehicleDebugRuntime()` 연결 방식은 그대로 재사용할 수 있다.

---

## 18. AssetDump 검증 메모

검증 출력 파일:
- `UE/Saved/BPDumpVehicleDebug/WBP_VehicleDebugPanel.dump.json`
- `UE/Saved/BPDumpVehicleDebug/WBP_VehicleDebugSection.dump.json`
- `UE/Saved/BPDumpVehicleDebug/WBP_VehicleDebugFieldRow.dump.json`

확인 결과:
- `WBP_VehicleDebugPanel` 부모 클래스는 `CFVehicleDebugPanelWidget`이다.
- `WBP_VehicleDebugPanel`에는 `VerticalBox_DynamicSectionHost`가 있다.
- `DynamicSectionWidgetClass`는 `WBP_VehicleDebugSection`으로 설정되어 있다.
- `VerticalBox_CategoryList`는 더 이상 존재하지 않는다.
- `WBP_VehicleDebugSection` 부모 클래스는 `CFVehicleDebugSectionWidget`이다.
- `Button_Header -> ToggleSectionExpanded` 이벤트 그래프 연결이 있다.
- `WBP_VehicleDebugSection`에는 `VerticalBox_FieldHost`가 있다.
- `FieldRowWidgetClass`는 `WBP_VehicleDebugFieldRow`로 설정되어 있다.
- `WBP_VehicleDebugFieldRow` 부모 클래스는 `CFVehicleDebugFieldRowWidget`이다.
- `WBP_VehicleDebugFieldRow`에는 `Text_Label`, `Text_Value`, `Text_Combined`가 있다.
