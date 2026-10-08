# VehicleDebug Panel Schema 설계안

- Version: 0.3.0
- Date: 2026-04-27
- Status: Panel 1차 마감
- Scope: VehicleDebug Panel을 장기 확장 가능한 데이터 기반 구조로 전환하기 위한 설계 기준

---

## 1. 문서 목적

이 문서는 현재의 정적 `WBP_VehicleDebugPanel` 구현을,

- 늘리기 쉽고,
- 관리하기 쉽고,
- 안정적이며,
- 성능 영향을 통제할 수 있고,
- 오래 유지 가능한

**데이터 기반 Panel 구조**로 전환하기 위한 설계 기준을 정리한다.

이 문서의 핵심 한 줄은 아래다.

> VehicleDebug Panel은 개별 Text/Button 위젯을 계속 수동으로 늘리는 구조가 아니라, Section/Field 데이터 모델을 렌더링하는 구조로 전환해야 한다.

---

## 2. 현재 상태 한 줄 요약

현재 Panel은 정적 `VerticalBox_CategoryList` 기반 구조에서 동적 Section 렌더링 구조로 전환됐다.

- `Overview / Drive / Input / Runtime`
- Runtime 하위 섹션
- Overview / Drive의 Transition 하위 섹션
- 공통 `WBP_VehicleDebugSection` 기반 Header 입력
- `VerticalBox_DynamicSectionHost` 기반 반복 렌더링
- `WBP_VehicleDebugFieldRow` 기반 일반 Field Row 렌더링
- 하위 Section 들여쓰기
- `CarFight_ReEditor Win64 Development` 빌드 성공

까지 PIE에서 정상 표시가 확인됐다.

기존 정적 `VerticalBox_CategoryList`는 더 이상 운영 구조가 아니다.
새 섹션이나 하위 섹션은 루트 WBP에 개별 Text/Button/Container를 추가하지 않고,
`Panel ViewData`에 Section 데이터를 추가하는 방식으로 확장한다.

주의:
- `VerticalBox_CategoryList`를 에디터 계층구조 눈 아이콘으로 숨기는 것은 PIE 런타임 숨김을 보장하지 않는다.
- 동적 섹션 표시 확인 후에는 `VerticalBox_CategoryList`를 삭제하고 `ScrollBox_Root` 아래에 `VerticalBox_DynamicSectionHost`만 남긴다.

---

## 3. 이번 설계의 목표

이번 설계의 목표는 아래 5개다.

1. 새 디버그 카테고리 추가 비용을 낮춘다.
2. 하위 섹션, 이벤트, 긴 텍스트를 같은 구조 안에서 처리한다.
3. WBP 자산 폭증과 이벤트 그래프 폭증을 막는다.
4. 현재 Snapshot 기반 데이터 구조와 충돌하지 않게 한다.
5. HUD / Panel / 미래 Event View가 같은 원본을 재사용하게 한다.

---

## 4. 비목표

이번 설계 문서에서 바로 하지 않는 것:

1. 즉시 전체 Panel 구현을 갈아엎지 않는다.
2. Slate 전환은 하지 않는다.
3. 화려한 애니메이션 구조는 먼저 넣지 않는다.
4. 검색/필터/정렬 UI까지 한 번에 넣지 않는다.
5. DataTable 기반 외부 설정화까지 바로 가지 않는다.

이번 문서는 **지속 가능한 기본 구조**를 먼저 정하는 것이 목적이다.

---

## 5. 핵심 문제 정의

현재 정적 Panel 방식의 핵심 문제는 아래다.

### 5.1 구조 중복

카테고리마다 아래가 반복된다.

- Header Button
- Title Text
- Body Container
- Body Text
- Toggle 함수
- 가시성 조건

이 반복은 초반엔 빠르지만, 카테고리가 늘수록 유지보수가 급격히 어려워진다.

### 5.2 WBP 비대화

WBP가 커질수록 아래 비용이 생긴다.

- 위젯 트리 확인 비용 증가
- 이벤트 그래프 확인 비용 증가
- 변수 이름 관리 비용 증가
- 실수 가능성 증가

### 5.3 확장 방식이 “끼워넣기”가 됨

지금 방식에서는 `Recent Events`, `Physics`, `Network`, `Camera`, `AI` 같은 새 영역이 들어올 때마다
기존 Panel에 또 Header와 Body를 덧붙이게 된다.

이 방식은 오래 쓰기 어렵다.

### 5.4 UI 구조와 데이터 구조가 너무 강하게 결합됨

현재는
- `Overview`가 있으니 `Text_OverviewTitle`, `Text_OverviewBody`
- `Drive`가 있으니 `Text_DriveTitle`, `Text_DriveBody`

처럼 UI 이름과 데이터 의미가 강하게 묶여 있다.

이 상태에서는 데이터 구조를 확장할수록 UI 수정량도 같이 커진다.

---

## 6. 권장 목표 구조

권장 목표 구조는 아래 4층이다.

### 6.1 Snapshot Layer

원본 디버그 데이터 구조다.

예:
- `FCFVehicleDebugSnapshot`
- `FCFVehicleDebugOverview`
- `FCFVehicleDebugDrive`
- `FCFVehicleDebugInput`
- `FCFVehicleDebugRuntime`

역할:
- 게임 상태를 구조화해서 담는다.
- UI 구조를 몰라야 한다.

### 6.2 Panel ViewData Layer

Panel 표시용 공통 데이터 구조다.

예:
- `FCFVehicleDebugFieldViewData`
- `FCFVehicleDebugSectionViewData`
- `FCFVehicleDebugPanelViewData`

역할:
- Snapshot을 Panel이 그리기 쉬운 구조로 변환한다.
- “섹션 / 하위 섹션 / 필드 / 긴 본문”을 공통 형태로 다룬다.

### 6.3 Widget Row Layer

반복 렌더링에 사용할 공통 위젯 계층이다.

예:
- `CFVehicleDebugSectionWidget`
- `CFVehicleDebugFieldRowWidget`

역할:
- 개별 카테고리마다 별도 위젯을 만드는 대신 공통 구조를 재사용한다.

### 6.4 Panel Root Layer

실제 `WBP_VehicleDebugPanel` 루트다.

역할:
- Panel ViewData를 받아 Section Widget을 반복 생성한다.
- 상세 표시 정책은 공통 Section Widget에 맡긴다.

---

## 7. 권장 데이터 모델

## 7.1 Field 모델

`Field`는 가장 작은 표시 단위다.

추천 역할:
- 라벨
- 값
- 표시 스타일 힌트
- 숨김 여부

권장 예시 필드:
- `State: Idle`
- `Speed: 10.2 km/h`
- `Input Owner: Player`

### Field 모델에 권장할 값

- `LabelText`
- `ValueText`
- `TooltipText`
- `bIsImportant`
- `bIsMultilineValue`

핵심:
- 단순 `라벨: 값`은 Field로 처리한다.
- 지금 Runtime처럼 긴 본문 블록은 Section 본문으로 처리한다.

## 7.2 Section 모델

`Section`은 Panel의 핵심 단위다.

예:
- `Overview`
- `Drive`
- `Input`
- `Runtime`
- `Overview Last Transition`
- `Drive Transition`
- `Runtime Summary`

즉, 상위 섹션과 하위 섹션은 **같은 타입**으로 다루는 것이 중요하다.

### Section 모델에 권장할 값

- `SectionId`
- `TitleText`
- `BodyText`
- `FieldArray`
- `ChildSectionArray`
- `bDefaultExpanded`
- `bVisible`
- `SectionKind`

### 왜 이렇게 해야 하냐

이 구조를 쓰면 아래가 전부 같은 방식으로 처리된다.

- 일반 카테고리
- 하위 섹션
- Recent Events
- 나중의 Physics / Network / Camera

즉, “새 구조를 또 새로 만들지 않아도 되는 상태”가 된다.

## 7.3 Panel ViewData 모델

`PanelViewData`는 최상위 렌더링 입력이다.

추천 역할:
- 최상위 섹션 배열 보유
- 패널 제목/버전/상태 메타 보유 가능

권장 값:
- `TopLevelSections`
- `PanelTitleText`
- `GeneratedFrameNumber`
- `GeneratedTimeSeconds`

---

## 8. 권장 렌더링 방식

## 8.1 정적 이름 기반 렌더링을 줄인다

현재 방식:
- `Text_OverviewBody`
- `Text_DriveBody`
- `Text_RuntimeSummaryBody`

목표 방식:
- Section Widget 하나를 만들어 여러 번 재사용

즉,
- `Overview`도 Section Widget
- `Drive`도 Section Widget
- `Runtime Summary`도 Section Widget

으로 처리한다.

## 8.2 루트 Panel은 섹션 리스트만 관리한다

최종 루트 구조:

```text
WBP_VehicleDebugPanel
└── ScrollBox_Root
    └── VerticalBox_DynamicSectionHost
        ├── SectionWidget(Overview)
        ├── SectionWidget(Drive)
        ├── SectionWidget(Input)
        └── SectionWidget(Runtime)
```

각 SectionWidget 내부는 다시

```text
WBP_VehicleDebugSection
├── Button_Header
├── Container_Body
│   ├── VerticalBox_FieldHost
│   └── Text_Body
└── VerticalBox_ChildSectionHost
```

형태를 권장한다.

이렇게 하면 WBP 루트는 더 이상 디버그 항목 수만큼 비대해지지 않는다.

## 8.3 입력 책임은 계속 WBP가 가져도 된다

현재 실측 기준으로 가장 안정적인 입력 경로는 아래였다.

- Header 클릭 입력: WBP 이벤트 그래프 또는 공통 Section Widget 내부 BP 이벤트
- 상태 변경/본문 생성: C++

이 원칙은 유지해도 된다.

즉, “전부 C++로 옮기기”가 목표가 아니다.

목표는:
- 입력 진입점은 UI 친화적으로
- 데이터 생성은 C++ 친화적으로

분리하는 것이다.

---

## 9. 권장 구현 전략

가장 안전한 기본안은 **병행 전환 후 정적 구조 제거**다.

## 9.1 1단계 - ViewData 타입만 추가

먼저 아래 타입만 추가한다.

- `FieldViewData`
- `SectionViewData`
- `PanelViewData`

이 단계에서는 기존 Panel을 아직 유지한다.

목적:
- 새 렌더링 구조의 입력 타입을 먼저 고정

## 9.2 2단계 - 현재 Panel 데이터를 ViewData로도 생성

예:
- `BuildOverviewSectionViewData()`
- `BuildDriveSectionViewData()`
- `BuildRuntimeSectionViewData()`

이 단계에서도 기존 정적 Panel은 아직 유지한다.

목적:
- Snapshot -> ViewData 변환 계층을 먼저 안정화

## 9.3 3단계 - 공통 Section Widget 도입

신규 자산 권장:
- `CFVehicleDebugSectionWidget`
- `WBP_VehicleDebugSection`

역할:
- Header 버튼
- Body Host
- Child Section Host

이 단계부터 동적 렌더링 전환을 시작한다.

## 9.4 4단계 - Panel Root를 동적 생성 구조로 전환

루트 Panel은 `TopLevelSections`를 순회하면서
SectionWidget을 생성해 붙인다.

이 단계가 끝나면,
기존 `Text_OverviewBody` 같은 정적 위젯 의존을 제거할 수 있다.

현재 구현 상태:
- `VerticalBox_DynamicSectionHost`에 최상위 Section Widget을 동적으로 생성한다.
- `DynamicSectionWidgetClass`에는 `WBP_VehicleDebugSection`을 지정한다.
- `VerticalBox_CategoryList`는 운영 구조에서 제거한다.
- 동적 경로가 준비되지 않았을 때만 기존 정적 경로가 fallback으로 남아 있다.

## 9.5 5단계 - Recent Events / 신규 카테고리 추가

이제부터는 새 기능이 들어와도
“UI를 다시 설계”하지 않고
“Section 하나 더 추가”
수준으로 끝나야 한다.

---

## 10. 성능 원칙

## 10.1 매 프레임 전체 재생성 금지

가장 중요한 원칙이다.

동적 구조로 간다고 해서 매 Tick마다
- Section Widget 전부 Destroy/Create
- FieldRow 전부 재생성

하면 안 된다.

권장 방식:
- ViewData 갱신
- 기존 위젯 재사용
- 변경된 텍스트만 업데이트

## 10.2 “생성”과 “갱신”을 분리한다

권장 함수 분리 예시:

- `BuildPanelViewData()`
- `EnsureSectionWidgets()`
- `RefreshSectionWidgetsFromViewData()`

즉,
- 처음 한 번 생성
- 이후엔 내용만 갱신

구조로 가야 한다.

## 10.3 기본 펼침 상태는 데이터에서 결정한다

예:
- `Overview` = 기본 펼침
- `Drive` = 기본 펼침
- `Input` = 기본 접힘 가능
- `Recent Events` = 기본 접힘

이걸 UI 하드코딩이 아니라 `SectionViewData.bDefaultExpanded`로 주는 게 좋다.

## 10.4 최대 보관 개수/최대 표시 개수 제한

나중에 `Recent Events` 같은 리스트형 섹션이 들어오면 반드시 제한이 필요하다.

예:
- 최근 이벤트 최대 20개 보관
- 패널에서는 최근 5개만 기본 표시

---

## 11. 안정성 원칙

## 11.1 Snapshot과 ViewData를 분리한다

Snapshot은 게임 상태의 구조화다.
ViewData는 UI 표시용 구조다.

이 둘을 섞으면 나중에
- HUD
- Panel
- Text
- Events

가 서로 영향을 주기 쉬워진다.

## 11.2 WBP 자산 이름 증가를 통제한다

지금처럼 섹션마다
- `Text_X`
- `Button_X`
- `Container_X`

를 계속 늘리면 자산 유지비가 커진다.

공통 Widget 기반으로 바꾸면
루트 WBP는 훨씬 단순해진다.

## 11.3 문서는 구조 기준으로 적는다

앞으로 문서도
“어떤 버튼 이름을 계속 추가해라”보다
“Section 모델에 어떤 데이터가 들어가고, 공통 Widget이 어떻게 그리느냐”
를 기준으로 옮겨가야 한다.

---

## 12. 권장 신규 산출물

이번 설계 이후 권장 산출물은 아래다.

### 문서
- `CF_VehicleDebugPanelSchema.md` (현재 문서)
- `CF_VehicleDebugPanelRefactorPlan.md` (후속 구현 계획 문서)

### C++
- `CFVehicleDebugPanelViewData.h`
- `CFVehicleDebugSectionWidget.h`
- `CFVehicleDebugSectionWidget.cpp`
- `CFVehicleDebugFieldRowWidget.h`
- `CFVehicleDebugFieldRowWidget.cpp`

### WBP
- `WBP_VehicleDebugSection`
- `WBP_VehicleDebugFieldRow`

---

## 13. 추천 전환 순서

1. ViewData 타입 정의
2. Snapshot -> ViewData 빌더 추가
3. 공통 Section Widget 추가
4. Panel Root 동적 렌더링 전환
5. 기존 정적 Panel 의존 제거
6. 신규 섹션 추가는 전부 ViewData 방식으로만 진행

이 순서를 권장하는 이유:

- 현재 잘 되는 구조를 바로 깨지 않는다.
- 각 단계마다 검증이 가능하다.
- 중간에 멈춰도 프로젝트가 망가지지 않는다.

---

## 14. 완료 판단 기준

아래가 충족되면 구조 전환이 성공한 것으로 본다.

1. 새 상위 카테고리 추가 시 루트 WBP에 새 Text/Button/Container를 직접 추가하지 않아도 된다.
2. 새 하위 섹션 추가 시 공통 Section Widget을 재사용할 수 있다.
3. `Recent Events`가 같은 구조 안에 자연스럽게 들어간다.
4. Tick마다 위젯을 재생성하지 않는다.
5. 현재 HUD / Panel / Snapshot 구조와 충돌하지 않는다.
6. 일반 `라벨: 값` 필드는 `WBP_VehicleDebugFieldRow`로 표시된다.
7. 하위 섹션은 상위 섹션보다 들여쓰기되어 계층 관계를 구분할 수 있다.

---

## 15. Changelog

### v0.3.0 - 2026-04-27
- Panel 1차 마감 상태를 반영했다.
- `WBP_VehicleDebugFieldRow` 기반 일반 Field Row 렌더링이 실제 운영 구조에 포함된 것을 명시했다.
- 하위 Section 들여쓰기 적용과 빌드 성공 상태를 완료 기준에 반영했다.
- `WBP_VehicleDebugPanel`, `WBP_VehicleDebugSection`, `WBP_VehicleDebugFieldRow` AssetDump 확인 결과를 기준으로 마이그레이션 지침을 정리했다.

### v0.2.1 - 2026-04-27
- `CFVehicleDebugFieldRowWidget` 기반 Field Row 렌더링 계층을 설계 산출물에 반영했다.
- 일반 `라벨: 값` 필드는 Field Row로 표시하고, 긴 본문 문자열은 Section의 `BodyText` / `Text_Body` 경로로 유지하는 기준을 추가했다.
- `VerticalBox_FieldHost`와 `FieldRowWidgetClass`가 준비되지 않은 경우 기존 문자열 렌더링으로 fallback되는 전환 방식을 반영했다.

### v0.2.2 - 2026-04-27
- `ChildSectionWidgetClass`가 비어 있어도 현재 `WBP_VehicleDebugSection` 클래스가 하위 섹션 생성 fallback으로 사용되는 기준을 반영했다.

### v0.2.3 - 2026-04-27
- 하위 섹션이 상위 섹션과 같은 정렬로 보여 계층 구분이 어려운 문제를 반영했다.
- 공통 Section Widget에서 하위 Section 슬롯에 들여쓰기 여백을 적용해 `Last Transition`, `Runtime Summary` 같은 하위 섹션을 시각적으로 구분하는 기준을 추가했다.

### v0.2.0 - 2026-04-24
- `WBP_VehicleDebugPanel`의 운영 루트가 `VerticalBox_DynamicSectionHost`로 전환된 것을 반영했다.
- `WBP_VehicleDebugSection` 공통 위젯 기반 동적 섹션 표시가 PIE에서 확인된 상태로 문서 상태를 `Implemented`로 변경했다.
- 기존 정적 `VerticalBox_CategoryList`는 동적 섹션 확인 후 삭제하는 것으로 마이그레이션 기준을 고정했다.

### v0.1.2 - 2026-04-24
- 공통 Section 렌더링 기반으로 `UE/Source/CarFight_Re/Public/UI/CFVehicleDebugSectionWidget.h`와 대응 cpp를 추가했다.
- 공통 Section Widget은 제목/본문/하위 섹션 렌더링을 담당하고, Header 입력은 추후 WBP에서 `ToggleSectionExpanded()`를 호출하는 방식으로 두는 현재 기준을 반영했다.

### v0.1.1 - 2026-04-24
- 첫 코드 기반 초안으로 `UE/Source/CarFight_Re/Public/UI/CFDebugPanelViewData.h`를 추가했다.
- `CFVehicleDebugPanelWidget`에 `PanelViewData` 캐시와 Snapshot -> ViewData 빌더 함수 초안을 추가하는 방향을 문서 기준과 맞췄다.

### v0.1.0 - 2026-04-24
- 정적 Panel을 데이터 기반 Panel 구조로 전환하기 위한 설계 기준 문서를 신규 작성했다.
- Section/Field/ViewData 중심 구조와 공통 Section Widget 방향을 정의했다.
- 확장성, 안정성, 성능, 장기 유지보수 관점의 원칙을 정리했다.

---

## 16. 마이그레이션 지침

- `VerticalBox_DynamicSectionHost`와 `DynamicSectionWidgetClass`가 정상 설정된 것을 PIE에서 확인한 뒤 `VerticalBox_CategoryList`를 삭제한다.
- `ScrollBox_Root` 아래에는 최종적으로 `VerticalBox_DynamicSectionHost`만 둔다.
- 앞으로 새 디버그 기능은 정적 WBP 확장이 아니라 ViewData/Section 기반 추가를 우선한다.
- 일반 `라벨: 값` 필드는 `WBP_VehicleDebugFieldRow`로 분리하고, 긴 본문은 `Text_Body`에 남긴다.
- `WBP_VehicleDebugSection`에는 `VerticalBox_FieldHost`를 추가하고 `FieldRowWidgetClass`에 `WBP_VehicleDebugFieldRow`를 지정한다.
- `ChildSectionWidgetClass`가 비어 있어도 현재 Section 클래스 fallback으로 하위 섹션이 생성되지만, 명시 설정이 필요하면 `WBP_VehicleDebugSection`을 지정한다.
- 하위 섹션 계층감이 부족하면 `ChildSectionIndentLeft` 값을 먼저 조정한다.
- 동적 경로가 비어 보이면 `VerticalBox_DynamicSectionHost` 이름, `DynamicSectionWidgetClass`, `WBP_VehicleDebugSection` 부모 클래스와 필수 위젯 이름을 먼저 확인한다.
