# VehicleDebug Panel UX 설계안

- Version: 0.1.0
- Date: 2026-04-27
- Status: Draft
- Scope: VehicleDebug Panel이 카테고리를 하나씩 점진 추가해도 보기 좋은 UX 구조를 유지하기 위한 설계 기준

---

## 1. 문서 목적

이 문서는 현재 `WBP_VehicleDebugPanel`이 동적 Section 구조로 전환된 이후, 카테고리가 점진적으로 늘어날 때 UI가 길게 나열되어 가독성이 떨어지는 문제를 막기 위한 UX 설계 기준을 정리한다.

현재 Panel은 `VerticalBox_DynamicSectionHost` 아래에 `Overview / Drive / Input / Runtime` 상위 Section을 동적으로 표시한다.
이 구조는 정적 WBP 확장보다 훨씬 좋지만, 앞으로 아래 같은 카테고리가 하나씩 추가되면 다른 문제가 생길 수 있다.

- Recent Events
- Physics
- Network
- Camera
- Wheel
- Input Detail
- Runtime Init
- Owner
- Weapon
- AI

중요한 전제는 아래다.

> VehicleDebug Panel의 최종 카테고리 목록은 지금 확정하지 않는다. 새 카테고리는 필요할 때 하나씩 추가할 수 있어야 한다.

따라서 이 문서의 목표는 특정 카테고리 목록을 고정하는 것이 아니라, **카테고리를 하나씩 추가해도 루트 WBP 구조와 사용성이 무너지지 않는 Panel UX 구조**를 정하는 것이다.

---

## 2. 현재 구조 요약

현재 기준 Panel 구조는 아래 흐름을 따른다.

```text
ACFVehiclePawn
  -> FCFVehicleDebugSnapshot
  -> Overview / Drive / Input / Runtime
  -> FCFVehicleDebugPanelViewData
  -> FCFVehicleDebugSectionViewData
  -> FCFVehicleDebugFieldViewData
  -> WBP_VehicleDebugPanel
  -> WBP_VehicleDebugSection
  -> WBP_VehicleDebugFieldRow
```

현재 운영 UI 구조는 아래에 가깝다.

```text
WBP_VehicleDebugPanel
└── Border_RootPanel
    └── ScrollBox_Root
        └── VerticalBox_DynamicSectionHost
            ├── WBP_VehicleDebugSection(Overview)
            ├── WBP_VehicleDebugSection(Drive)
            ├── WBP_VehicleDebugSection(Input)
            └── WBP_VehicleDebugSection(Runtime)
```

현재 구조의 장점:
- 새 Section을 데이터로 추가할 수 있다.
- 일반 필드는 `WBP_VehicleDebugFieldRow`로 재사용된다.
- 긴 문자열은 `Text_Body` 경로로 fallback 가능하다.
- 하위 Section을 같은 타입으로 다룰 수 있다.

현재 구조의 한계:
- 상위 카테고리가 늘수록 Header 버튼이 세로로 길게 누적된다.
- 접힌 상태여도 Header 자체가 화면을 많이 차지한다.
- 실제로 보고 싶은 데이터 영역보다 카테고리 버튼 목록이 더 커질 수 있다.
- Recent Events처럼 항상 보여야 하는 정보가 일반 카테고리 사이에 묻힐 수 있다.

---

## 3. 핵심 UX 문제 정의

현재 구조의 문제는 데이터 구조가 아니라 표시 방식이다.

`Section / Field / ViewData` 구조는 계속 유지해도 된다.
다만 최상위 Section을 전부 세로 Accordion으로 렌더링하는 방식은 카테고리 수가 늘어날수록 불리하다.

문제 상황 예시:

```text
Overview
Drive
Input
Input Detail
Runtime
Runtime Init
Recent Events
Physics
Network
Camera
Wheel
Owner
Weapon
AI
```

이런 목록이 전부 한 ScrollBox 안에 Header + Body 형태로 들어가면 아래 문제가 생긴다.

1. 원하는 카테고리를 찾는 시간이 길어진다.
2. 실제 디버그 값보다 Header가 더 많이 보인다.
3. Runtime / Warning / Recent Events 같은 우선 정보가 묻힌다.
4. 상위 Section과 하위 Section의 시각적 계층이 복잡해진다.
5. 새 카테고리를 추가할수록 UX 품질이 떨어진다.

따라서 개선 방향은 아래로 잡는다.

> 최상위 카테고리는 전부 펼침/접힘 목록으로 표시하지 말고, Navigation에서 선택하고 선택된 카테고리만 Content 영역에 표시한다.

---

## 4. 설계 목표

## 4.1 카테고리 미확정 전제 유지

이 문서는 최종 카테고리 목록을 확정하지 않는다.

좋은 구조는 아래 조건을 만족해야 한다.

- 지금은 `Overview / Drive / Input / Runtime`만 있어도 동작한다.
- 나중에 `Wheel` 하나만 추가해도 기존 UI를 크게 수정하지 않는다.
- 그 다음 `Camera` 하나만 추가해도 같은 방식으로 붙는다.
- `Recent Events`가 나중에 추가되어도 별도 대공사를 하지 않는다.

## 4.2 새 카테고리 추가 비용 최소화

새 카테고리 추가 시 기본 작업은 아래 정도로 끝나야 한다.

1. Snapshot 또는 관련 데이터 원본 추가
2. Panel ViewData Builder에 새 Section 생성 함수 추가
3. `TopLevelSectionArray`에 새 Section 추가
4. 필요 시 Navigation 그룹/정렬값만 지정

하지 말아야 할 것:
- 루트 WBP에 새 Button을 직접 추가
- 루트 WBP에 새 TextBlock을 직접 추가
- 카테고리별 이벤트 그래프를 계속 추가
- 전체 Panel 레이아웃을 다시 설계

## 4.3 작은 카테고리 수와 많은 카테고리 수 모두 지원

카테고리가 4개일 때도 과하게 복잡하지 않아야 하고,
카테고리가 12개 이상이 되어도 무너지지 않아야 한다.

따라서 표시 구조는 아래 두 단계를 모두 고려한다.

- 초기: 현재 Accordion 구조 유지 가능
- 확장: Navigation + Selected Content 구조로 전환

다만 장기 기본안은 `Navigation + Selected Content` 구조다.

---

## 5. 비목표

이번 문서에서 바로 하지 않는 것:

1. 최종 카테고리 목록 확정
2. 모든 예상 카테고리의 실제 구현
3. 복잡한 검색/필터 UI 선도입
4. Slate 기반 Editor Tool 전환
5. 그래프/차트 기반 시각화
6. 모든 Debug 시스템 통합 Manager 설계

이번 문서는 VehicleDebug Panel이 **점진 확장 가능한 UX 뼈대**를 갖도록 하는 데 집중한다.

---

## 6. 권장 최종 레이아웃

장기적으로 권장하는 Panel 레이아웃은 아래다.

```text
WBP_VehicleDebugPanel
└── Border_RootPanel
    └── VerticalBox_Root
        ├── Border_TopSummary
        │   └── HorizontalBox_SummaryHost
        │
        ├── HorizontalBox_Main
        │   ├── ScrollBox_Navigation
        │   │   └── VerticalBox_NavHost
        │   │
        │   └── ScrollBox_Content
        │       └── VerticalBox_SelectedSectionHost
        │
        └── Border_EventStrip
            └── Text_RecentEventSummary
```

각 영역의 책임은 아래와 같다.

| 영역 | 책임 |
|---|---|
| `Border_TopSummary` | 항상 봐야 하는 핵심 요약 표시 |
| `VerticalBox_NavHost` | 최상위 카테고리 Navigation 표시 |
| `VerticalBox_SelectedSectionHost` | 현재 선택된 카테고리의 Section 표시 |
| `Border_EventStrip` | 최근 이벤트 1~3개 요약 표시 |

핵심은 아래다.

> 최상위 Section 전체를 한 번에 렌더링하지 않고, 선택된 Section 하나를 넓게 표시한다.

---

## 7. Navigation + Selected Content 구조

## 7.1 개념

현재 구조:

```text
TopLevelSectionArray 전체를 VerticalBox_DynamicSectionHost에 순서대로 렌더링
```

권장 구조:

```text
TopLevelSectionArray는 전체 카테고리 원본으로 유지
Navigation은 TopLevelSectionArray에서 항목만 추출해 표시
Content는 SelectedSectionId에 해당하는 Section만 표시
```

예시:

```text
TopLevelSectionArray
├── Overview
├── Drive
├── Input
├── Runtime
├── Wheel
├── Camera
└── Physics

SelectedSectionId = "Drive"

Content에는 Drive Section만 표시
```

## 7.2 장점

- 카테고리가 늘어도 오른쪽 데이터 영역이 유지된다.
- 사용자는 필요한 카테고리를 빠르게 선택할 수 있다.
- 새 카테고리 추가가 Navigation 자동 항목 생성으로 끝난다.
- Runtime, Wheel, Camera처럼 상세 필드가 많은 카테고리도 넓게 표시할 수 있다.
- Recent Events를 일반 카테고리와 별도 영역으로 뺄 수 있다.

---

## 8. 카테고리 그룹 기준

카테고리 목록은 확정하지 않지만, Navigation에는 그룹 기준이 있으면 좋다.

초기 권장 그룹은 아래다.

```text
Core
├── Overview
├── Drive
├── Input
└── Runtime

Vehicle
├── Wheel
├── Physics
└── Camera

Diagnostics
├── Recent Events
├── Runtime Init
└── Validation

Advanced
├── Network
├── Owner
├── Weapon
└── AI
```

주의:
- 위 목록은 최종 구현 목록이 아니다.
- 실제로 필요한 카테고리만 하나씩 추가한다.
- 그룹은 UI 정렬과 탐색을 돕기 위한 기준이다.

## 8.1 그룹 없이 시작해도 된다

처음부터 그룹 UI를 만들 필요는 없다.

초기에는 아래처럼 단순 Navigation만 둬도 된다.

```text
Overview
Drive
Input
Runtime
```

카테고리가 6개 이상이 되면 그룹 표시를 추가한다.

---

## 9. 데이터 모델 확장 방향

현재 `FCFVehicleDebugSectionViewData`에는 아래 정보가 있다.

```text
SectionId
TitleText
BodyText
FieldArray
ChildSectionArray
bDefaultExpanded
bIsVisible
SectionKind
```

Navigation 구조를 위해 아래 정보를 추가하는 방향을 권장한다.

```text
NavigationGroup
NavigationOrder
bShowInNavigation
BadgeText
bShowInTopSummary
```

## 9.1 권장 필드 의미

| 필드 | 의미 |
|---|---|
| `NavigationGroup` | Core / Vehicle / Diagnostics / Advanced 같은 탐색 그룹 |
| `NavigationOrder` | 같은 그룹 안에서의 표시 순서 |
| `bShowInNavigation` | 왼쪽 Navigation에 표시할지 여부 |
| `BadgeText` | Warning 수, Error 수, 상태 같은 짧은 배지 |
| `bShowInTopSummary` | 상단 요약 영역에 일부 표시할지 여부 |

이 필드는 카테고리를 확정하기 위한 값이 아니다.
**카테고리가 하나씩 추가될 때, UI가 그 카테고리를 어떻게 배치할지 알려주는 힌트**다.

## 9.2 별도 NavItem ViewData 선택안

`FCFVehicleDebugSectionViewData`가 너무 UI 탐색 정보까지 많이 알게 되는 것이 부담되면, 별도 Navigation ViewData를 둘 수 있다.

권장 타입명:

```text
FCFVehicleDebugNavItemViewData
```

권장 파일명:

```text
CFDebugPanelNavData.h
```

예시 구조:

```cpp
// VehicleDebug Panel Navigation에서 사용할 카테고리 그룹입니다.
enum class ECFVehicleDebugNavGroup : uint8
{
	Core,
	Vehicle,
	Diagnostics,
	Advanced
};

// VehicleDebug Panel의 Navigation 표시용 항목입니다.
struct FCFVehicleDebugNavItemViewData
{
	// Navigation 항목과 연결될 Section 고유 ID입니다.
	FString SectionId;

	// Navigation 버튼에 표시할 이름입니다.
	FString DisplayText;

	// Navigation 항목이 속한 그룹입니다.
	ECFVehicleDebugNavGroup NavigationGroup = ECFVehicleDebugNavGroup::Core;

	// 같은 그룹 안에서 표시될 순서입니다.
	int32 NavigationOrder = 0;

	// 버튼 우측에 표시할 짧은 배지 문자열입니다.
	FString BadgeText;

	// 현재 항목을 Navigation에 표시할지 여부입니다.
	bool bIsVisible = true;

	// 현재 선택된 항목인지 여부입니다.
	bool bIsSelected = false;
};
```

초기 구현에서는 별도 파일을 만들지 않고 SectionViewData에서 바로 생성해도 된다.
다만 카테고리 수가 많아지면 별도 NavItem ViewData로 분리하는 편이 유지보수에 좋다.

---

## 10. 선택 상태 관리

Panel Root에는 현재 선택된 Section을 나타내는 상태가 필요하다.

권장 변수:

```text
SelectedSectionId
```

권장 기본값:

```text
Overview
```

선택 상태 규칙:

1. `SelectedSectionId`가 비어 있으면 첫 번째 표시 가능한 Section을 선택한다.
2. `SelectedSectionId`가 더 이상 존재하지 않으면 첫 번째 표시 가능한 Section으로 fallback한다.
3. 사용자가 Navigation 항목을 클릭하면 `SelectedSectionId`를 해당 SectionId로 바꾼다.
4. `SelectedSectionId`에 해당하는 Section만 `VerticalBox_SelectedSectionHost`에 표시한다.
5. 선택되지 않은 최상위 Section은 생성하지 않거나 Collapsed 처리한다.

---

## 11. Recent Events 표시 기준

Recent Events는 일반 카테고리와 성격이 다르다.

일반 카테고리:
- 현재 상태
- 현재 수치
- 현재 입력
- 현재 런타임 상태

Recent Events:
- 최근 변화
- 상태 전이
- 경고 발생
- 입력 주도권 변경
- Runtime 재초기화

따라서 Recent Events는 두 위치를 권장한다.

## 11.1 하단 Event Strip

하단에는 최근 1~3개만 짧게 표시한다.

예:

```text
Recent: DriveState Idle -> Driving | InputOwner VehicleMove2D | Runtime Ready
```

장점:
- 어떤 카테고리를 보고 있어도 최근 변화를 놓치지 않는다.
- 이벤트가 일반 Section 사이에 묻히지 않는다.

## 11.2 Events 상세 카테고리

Navigation에는 `Events` 또는 `Recent Events` 카테고리를 둘 수 있다.
이 카테고리를 선택하면 이벤트 목록 전체를 상세히 본다.

예:

```text
Recent Events
├── Latest
├── Warning
├── Runtime
├── Input
└── Drive
```

주의:
- Event Strip은 요약이다.
- Events 카테고리는 상세 목록이다.
- 두 역할을 분리한다.

---

## 12. 카테고리 추가 규칙

새 카테고리를 추가할 때는 아래 순서를 따른다.

## 12.1 새 상위 카테고리 추가

예: `Wheel`

작업 순서:

1. 필요한 원본 데이터 구조를 만든다.
2. Snapshot 또는 관련 접근 API를 추가한다.
3. `BuildWheelSectionViewData()` 같은 Builder 함수를 추가한다.
4. `PanelViewData.TopLevelSectionArray`에 Wheel Section을 추가한다.
5. `NavigationGroup`과 `NavigationOrder`를 지정한다.
6. WBP 루트에는 새 Button/Text를 직접 추가하지 않는다.

## 12.2 기존 카테고리의 하위 섹션 추가

예: `Runtime` 안에 `Runtime Init` 추가

작업 순서:

1. `BuildRuntimeSectionViewData()` 안에서 하위 Section을 만든다.
2. `RuntimeSectionViewData->AddChildSection()`으로 추가한다.
3. 하위 섹션은 `SectionKind = Subsection`으로 둔다.
4. 별도 루트 Navigation 항목으로 올릴 필요가 있는지 판단한다.

판단 기준:
- 자주 독립적으로 봐야 하면 상위 Navigation 항목
- 특정 카테고리 안에서만 의미가 있으면 하위 Section

## 12.3 일반 Field 추가

예: Drive에 `Current Gear` 추가

작업 순서:

1. Drive Snapshot에 값 추가
2. `BuildDriveSectionViewData()`에 Field 추가
3. `WBP_VehicleDebugFieldRow`가 자동으로 표시

루트 WBP 수정은 하지 않는다.

---

## 13. 상위 카테고리와 하위 섹션 판단 기준

상위 카테고리로 올릴 것:

- 독립적으로 자주 확인하는 정보
- 필드 수가 많아 별도 페이지가 필요한 정보
- 다른 카테고리와 책임이 명확히 다른 정보
- 문제 원인 분석에서 진입점이 되는 정보

하위 섹션으로 둘 것:

- 특정 카테고리에 종속된 상세 정보
- 긴 Summary 문자열
- 가끔만 펼쳐 보는 진단 텍스트
- 상위 카테고리 없이 보면 맥락이 약한 정보

예시:

```text
Input
├── Summary Fields
├── Input Detail
└── Ownership Detail
```

```text
Runtime
├── Runtime State Fields
├── Runtime Init
├── Last Validation
└── Component State
```

```text
Vehicle
├── Wheel
├── Physics
└── Camera
```

---

## 14. Adaptive 표시 전략

카테고리 수가 적을 때부터 Navigation 구조를 강제할 필요는 없다.
그러나 장기적으로는 Navigation 구조가 유리하다.

권장 전환 기준:

| 카테고리 수 | 권장 표시 방식 |
|---|---|
| 1~4개 | 현재 Accordion 방식도 허용 |
| 5~7개 | Navigation + Selected Content 전환 권장 |
| 8개 이상 | Navigation 구조 필수 |

다만 구현 안정성을 생각하면, 카테고리가 적더라도 미리 Navigation 구조로 옮기는 편이 좋다.
그렇게 하면 나중에 카테고리가 하나씩 늘어날 때 전환 비용이 없다.

---

## 15. 권장 구현 단계

## 15.1 Phase UX-0 - 현재 구조 유지

목표:
- 현재 `VerticalBox_DynamicSectionHost` 구조를 유지한다.
- 문서 기준만 먼저 고정한다.

완료 기준:
- Panel UX 설계 문서가 추가된다.
- 다음 구현 방향이 Selected Section 구조로 합의된다.

## 15.2 Phase UX-1 - Selected Section 상태 추가

목표:
- `SelectedSectionId`를 추가한다.
- `TopLevelSectionArray` 중 선택된 하나만 표시하는 구조를 만든다.

권장 추가 위젯 이름:

```text
VerticalBox_SelectedSectionHost
```

완료 기준:
- 선택된 Section 하나만 오른쪽 Content 영역에 표시된다.
- 기본 선택은 `Overview`다.
- 선택 대상이 없으면 첫 번째 표시 가능한 Section으로 fallback한다.

## 15.3 Phase UX-2 - Navigation 목록 추가

목표:
- 왼쪽 Navigation 영역을 만든다.
- `TopLevelSectionArray`에서 NavItem을 자동 생성한다.

권장 추가 위젯:

```text
WBP_VehicleDebugNavItem
```

권장 추가 호스트:

```text
VerticalBox_NavHost
```

완료 기준:
- NavItem 클릭 시 `SelectedSectionId`가 바뀐다.
- 새 TopLevel Section이 추가되면 Navigation에도 자동으로 항목이 생긴다.

## 15.4 Phase UX-3 - Top Summary 추가

목표:
- 항상 봐야 하는 핵심값을 상단에 고정한다.

권장 표시값:
- Runtime Ready
- Drive State
- Speed
- Input Owner
- Warning Count 또는 Event Count

완료 기준:
- 어떤 카테고리를 선택해도 핵심 상태는 상단에 유지된다.

## 15.5 Phase UX-4 - Event Strip 추가

목표:
- Recent Events 최신 1~3개를 하단에 표시한다.

권장 추가 위젯:

```text
WBP_VehicleDebugEventStrip
```

완료 기준:
- 카테고리를 바꿔도 최근 이벤트 요약은 유지된다.
- Events 상세 카테고리와 역할이 분리된다.

## 15.6 Phase UX-5 - Group / Badge 추가

목표:
- Navigation 항목이 많아졌을 때 그룹과 배지로 탐색성을 높인다.

완료 기준:
- Core / Vehicle / Diagnostics / Advanced 같은 그룹 표시가 가능하다.
- Warning, Error, Count 같은 짧은 배지가 NavItem에 표시된다.

---

## 16. WBP 이름 기준

새로 추가할 가능성이 있는 위젯 이름은 아래를 권장한다.

```text
WBP_VehicleDebugNavItem
WBP_VehicleDebugEventStrip
```

Panel 내부 위젯 이름은 아래를 권장한다.

```text
VerticalBox_Root
Border_TopSummary
HorizontalBox_SummaryHost
HorizontalBox_Main
ScrollBox_Navigation
VerticalBox_NavHost
ScrollBox_Content
VerticalBox_SelectedSectionHost
Border_EventStrip
Text_RecentEventSummary
```

주의:
- C++ `BindWidgetOptional`로 연결할 이름은 정확히 일치해야 한다.
- 기존 `VerticalBox_DynamicSectionHost`는 전환 기간 fallback으로 유지할 수 있다.
- 최종적으로는 `VerticalBox_SelectedSectionHost`가 선택된 Section 표시를 담당한다.

---

## 17. C++ 책임 분리 기준

## 17.1 Panel Root 책임

`UCFVehicleDebugPanelWidget` 책임:

- Pawn에서 최신 데이터 읽기
- PanelViewData 생성
- Navigation 항목 생성
- `SelectedSectionId` 관리
- 선택된 Section 위젯 생성/갱신
- Summary / Event Strip 갱신
- Panel Visibility / Interaction Mode 관리

## 17.2 Section Widget 책임

`UCFVehicleDebugSectionWidget` 책임:

- Section ViewData 적용
- Header 접기/펼치기
- Field Row 생성/갱신
- Child Section 생성/갱신
- Section 내부 표시 책임

## 17.3 NavItem Widget 책임

`UCFVehicleDebugNavItemWidget`을 만들 경우 책임:

- Navigation 항목 텍스트 표시
- 선택 상태 표시
- Badge 표시
- 클릭 시 Panel Root에 선택 요청 전달

주의:
- NavItem은 데이터를 계산하지 않는다.
- NavItem은 어떤 카테고리가 중요한지 판단하지 않는다.
- 판단은 ViewData 또는 Panel Root에서 한다.

---

## 18. 성능 원칙

1. 매 Tick마다 모든 Section 위젯을 Destroy/Create하지 않는다.
2. 선택된 Section만 Content 영역에서 갱신한다.
3. Navigation 항목은 Section 개수가 바뀔 때만 재생성한다.
4. Field Row는 현재처럼 개수 변화가 있을 때만 재생성하고, 값만 갱신한다.
5. Event Strip은 표시 개수를 제한한다.
6. Recent Events 전체 목록은 선택된 경우에만 상세 렌더링한다.

---

## 19. 실패 방지 규칙

아래 작업은 피한다.

- 새 카테고리마다 루트 WBP에 새 Button 추가
- 새 카테고리마다 루트 WBP 이벤트 그래프에 OnClicked 추가
- 모든 카테고리를 항상 펼쳐진 상태로 표시
- Recent Events를 일반 긴 TextBlock 하나에만 밀어 넣기
- Navigation 그룹을 enum으로 너무 일찍 과도하게 고정하기
- 검색/필터를 Panel 기본 구조보다 먼저 구현하기

---

## 20. 완료 판단 기준

아래 조건을 만족하면 Panel UX 구조 개선이 성공한 것으로 본다.

1. 새 TopLevel Section을 추가해도 루트 WBP 구조를 수정하지 않는다.
2. 카테고리가 8개 이상이어도 실제 데이터 영역이 확보된다.
3. 선택된 카테고리 하나를 넓게 볼 수 있다.
4. Recent Events는 항상 요약으로 확인할 수 있다.
5. Events 상세는 별도 카테고리로 확장 가능하다.
6. Navigation 항목은 ViewData에서 자동 생성된다.
7. Field Row / Child Section 기존 구조는 그대로 재사용된다.
8. 기존 Panel 1차 구조와 단계적으로 병행 전환할 수 있다.

---

## 21. 결론

VehicleDebug Panel의 다음 개선은 데이터 구조 재작성보다 UX 렌더링 방식 개선이 핵심이다.

현재 `Section / FieldRow / ViewData` 구조는 유지한다.
다만 최상위 Section을 전부 세로로 나열하는 방식은 장기적으로 불리하므로, 다음 방향으로 전환한다.

> TopLevel Section은 Navigation 항목으로 보여주고, Content 영역에는 선택된 Section 하나만 표시한다.

이렇게 하면 카테고리 목록이 확정되지 않아도, 새 카테고리를 하나씩 추가하면서 Panel UX를 안정적으로 유지할 수 있다.

---

## 22. Changelog

### v0.1.0 - 2026-04-27
- VehicleDebug Panel이 카테고리를 하나씩 점진 추가할 수 있도록 UX 설계 기준을 신규 작성했다.
- 현재 Accordion 방식의 한계와 Navigation + Selected Content 전환 방향을 정리했다.
- Recent Events를 Event Strip과 Events 상세 카테고리로 분리하는 기준을 추가했다.
- 새 카테고리 추가 시 루트 WBP를 직접 수정하지 않는 규칙을 명시했다.
