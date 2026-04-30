# Codex 작업 지시서 - VehicleDebug Panel UX 1차 개선

- Version: 0.2.0
- Date: 2026-04-27
- Status: Completed / C++ Build Passed
- Scope: VehicleDebug Panel을 카테고리 점진 추가에 강한 Navigation + Selected Content 기반으로 전환하기 위한 C++ 1차 작업 지시

---

## 1. 작업 목적

현재 VehicleDebug Panel은 `VerticalBox_DynamicSectionHost` 아래에 모든 최상위 Section을 세로로 나열한다.
현재 카테고리는 `Overview / Drive / Input / Runtime` 정도라서 사용할 수 있지만, 앞으로 `Recent Events / Physics / Network / Camera / Wheel / Input Detail / Runtime Init` 같은 카테고리가 하나씩 추가되면 버튼과 Section이 계속 세로로 늘어나 실제 데이터를 보기 어려워진다.

이번 Codex 작업의 목적은 아래다.

> 최상위 Section 전체를 한 번에 표시하는 구조에서, 선택된 Section 하나만 Content 영역에 표시할 수 있는 C++ 기반을 만든다.

단, 이번 작업에서 모든 UX 최종안을 완성하지 않는다.
이번 작업은 **BP/UMG 자산을 나중에 안전하게 연결할 수 있도록 C++ 기반을 먼저 준비하는 단계**다.

---

## 2. 중요한 전제

## 2.1 카테고리 목록은 확정하지 않는다

이번 작업에서 `Physics / Network / Camera / Wheel` 같은 카테고리를 실제로 전부 추가하지 않는다.

카테고리는 앞으로 필요할 때 하나씩 추가된다.
따라서 구조는 아래 조건을 만족해야 한다.

- 현재 `Overview / Drive / Input / Runtime`만 있어도 정상 동작한다.
- 나중에 새 TopLevel Section 하나가 추가되면 Navigation/Selected Content 구조가 그대로 받아들인다.
- 루트 WBP에 카테고리별 Button/Text를 매번 직접 추가하지 않는 구조를 유지한다.

## 2.2 Codex는 `.uasset` 직접 편집을 하지 않는다

Codex는 Unreal UMG `.uasset`을 직접 수정하지 않는다.
이번 작업은 C++ 코드와 문서만 수정한다.

UMG에서 해야 할 작업은 문서에 수동 체크리스트로 남긴다.

## 2.3 기존 Panel 동작을 깨지 않는다

현재 `WBP_VehicleDebugPanel`에 아직 새 위젯들이 없을 수 있다.
그러므로 C++는 아래 fallback을 반드시 지원해야 한다.

- `VerticalBox_SelectedSectionHost`가 있고 `DynamicSectionWidgetClass`도 유효하면 선택된 Section만 표시한다.
- `VerticalBox_SelectedSectionHost`가 있어도 `DynamicSectionWidgetClass`가 비어 있으면 선택 Section 렌더링 경로를 사용하지 않고 기존 `VerticalBox_DynamicSectionHost` fallback으로 돌아간다.
- `VerticalBox_SelectedSectionHost`가 없고 기존 `VerticalBox_DynamicSectionHost`만 있으면 기존처럼 전체 Section을 표시한다.
- `VerticalBox_NavHost` 또는 `NavItemWidgetClass`가 없으면 Navigation 생성은 건너뛴다.

---

## 3. 참고 문서

먼저 아래 문서를 읽고 작업한다.

```text
Document/ProjectSSOT/Plan/VehicleDebugPlan/CF_VehicleDebugPanelUX.md
Document/ProjectSSOT/Plan/VehicleDebugPlan/CF_VehicleDebugPanelSchema.md
Document/ProjectSSOT/Plan/VehicleDebugPlan/CF_VehicleDebugPanelGuide.md
Document/ProjectSSOT/Systems/UI/VehicleDebug.md
```

특히 `CF_VehicleDebugPanelUX.md`의 아래 섹션을 기준으로 한다.

- `6. 권장 최종 레이아웃`
- `7. Navigation + Selected Content 구조`
- `9. 데이터 모델 확장 방향`
- `10. 선택 상태 관리`
- `15. 권장 구현 단계`
- `16. WBP 이름 기준`
- `18. 성능 원칙`
- `19. 실패 방지 규칙`

---

## 4. 이번 작업 범위

이번 작업 범위는 `Phase UX-1`과 `Phase UX-2 C++ 기반`이다.

## 4.1 반드시 할 것

1. Panel ViewData에 Navigation용 최소 데이터 구조를 추가한다.
2. `UCFVehicleDebugPanelWidget`에 `SelectedSectionId` 상태를 추가한다.
3. 선택된 Section만 표시하는 경로를 추가한다.
4. 기존 `VerticalBox_DynamicSectionHost` fallback을 유지한다.
5. Navigation Item C++ 부모 위젯 클래스를 추가한다.
6. `VerticalBox_NavHost`와 `NavItemWidgetClass`가 준비된 경우 Navigation 항목을 동적으로 생성한다.
7. README 또는 관련 문서에 BP/UMG 수동 연결 체크리스트를 남긴다.

## 4.2 이번 작업에서 하지 말 것

1. `.uasset` 직접 수정 금지
2. 모든 예상 카테고리 구현 금지
3. Recent Events 실제 이벤트 로그 시스템 구현 금지
4. Top Summary Bar 실제 구현 금지
5. Event Strip 실제 구현 금지
6. 검색/필터 UI 구현 금지
7. 기존 Field Row / Section 구조 대규모 재작성 금지
8. 현재 레거시 fallback 삭제 금지

---

## 5. 수정 대상 파일

## 5.1 수정할 기존 파일

```text
UE/Source/CarFight_Re/Public/UI/CFDebugPanelViewData.h
UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
Document/ProjectSSOT/Plan/VehicleDebugPlan/CF_VehicleDebugPanelGuide.md
Document/ProjectSSOT/Plan/VehicleDebugPlan/CF_VehicleDebugPanelUX.md
```

## 5.2 새로 추가할 C++ 파일

```text
UE/Source/CarFight_Re/Public/UI/CFVehicleDebugNavItemWidget.h
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugNavItemWidget.cpp
```

파일명 길이 제한을 지킨다.

## 5.3 선택적으로 수정할 파일

README 갱신이 필요하면 아래 파일도 수정한다.

```text
Document/ProjectSSOT/Plan/VehicleDebugPlan/README.md
```

---

## 6. 코드 스타일 규칙

## 6.1 버전 주석

수정하는 C++ 파일 상단의 Version/Date/Description을 갱신한다.

예:

```cpp
// Version: 1.7.0
// Date: 2026-04-27
// Description: VehicleDebug Panel Navigation + Selected Section 표시 기반 추가
```

## 6.2 주석 규칙

새로 추가하는 주요 변수와 함수에는 반드시 한국어 주석을 단다.

예:

```cpp
// [v1.7.0] 현재 Panel Content 영역에 표시할 선택된 최상위 SectionId입니다.
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="선택된 Section ID (SelectedSectionId)", ToolTip="현재 Panel Content 영역에 표시할 최상위 SectionId입니다. 비어 있거나 유효하지 않으면 첫 번째 표시 가능한 Section으로 대체됩니다."))
FString SelectedSectionId = TEXT("Overview");
```

## 6.3 Blueprint 노출 규칙

Blueprint에서 보일 변수/함수는 `DisplayName`, `ToolTip`, `Category`를 명확히 작성한다.

## 6.4 이름 규칙

직관적인 이름을 사용한다.
축약어를 과도하게 쓰지 않는다.

좋은 예:

```text
SelectedSectionId
NavItemWidgetClass
VerticalBox_SelectedSectionHost
RefreshNavigationItems
RefreshSelectedSectionWidget
ResolveSelectedSectionViewData
```

나쁜 예:

```text
SelId
NavCls
TmpSec
DoRefresh
```

---

## 7. 데이터 구조 작업

파일:

```text
UE/Source/CarFight_Re/Public/UI/CFDebugPanelViewData.h
```

## 7.1 Navigation Group enum 추가

`FCFVehicleDebugSectionViewData` 근처에 아래 enum을 추가한다.

```cpp
// VehicleDebug Panel Navigation에서 사용할 카테고리 그룹입니다.
enum class ECFVehicleDebugNavGroup : uint8
{
	// 핵심 주행/입력/런타임 정보를 묶는 기본 그룹입니다.
	Core,

	// 차량 하위 시스템 정보를 묶는 그룹입니다.
	Vehicle,

	// 문제 추적과 검증 정보를 묶는 그룹입니다.
	Diagnostics,

	// 네트워크, 소유권, AI 등 고급 정보를 묶는 그룹입니다.
	Advanced
};
```

주의:
- 이번 단계에서는 `UENUM(BlueprintType)`로 만들 필요는 없다.
- C++ 내부 ViewData용으로 충분하다.
- 나중에 BP에서 직접 다룰 필요가 생기면 별도 작업으로 승격한다.

## 7.2 NavItem ViewData 추가

아래 struct를 추가한다.

```cpp
// VehicleDebug Panel의 Navigation 표시용 항목 ViewData입니다.
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

## 7.3 SectionViewData에 Navigation 힌트 추가

`FCFVehicleDebugSectionViewData`에 아래 필드를 추가한다.

```cpp
// Navigation에서 사용할 그룹입니다.
ECFVehicleDebugNavGroup NavigationGroup = ECFVehicleDebugNavGroup::Core;

// 같은 Navigation 그룹 안에서의 표시 순서입니다.
int32 NavigationOrder = 0;

// 이 Section을 Navigation 목록에 표시할지 여부입니다.
bool bShowInNavigation = true;

// Navigation 항목 우측에 표시할 짧은 배지 문자열입니다.
FString BadgeText;
```

## 7.4 MakeSection overload 또는 기본 설정 함수 추가

현재 `MakeSection()`을 크게 깨지 말고, 아래 중 하나로 구현한다.

선택 A: 기존 `MakeSection()` 유지 + 생성 후 필드 직접 설정

```cpp
TSharedRef<FCFVehicleDebugSectionViewData> SectionViewData = FCFVehicleDebugSectionViewData::MakeSection(...);
SectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Core;
SectionViewData->NavigationOrder = 10;
```

선택 B: 새 helper 함수 추가

```cpp
void ConfigureNavigation(
	ECFVehicleDebugNavGroup InNavigationGroup,
	int32 InNavigationOrder,
	bool bInShowInNavigation = true,
	const FString& InBadgeText = FString());
```

권장: 선택 A.
이유: 이번 단계에서는 구조 변경을 최소화한다.

---

## 8. Panel Widget Header 작업

파일:

```text
UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h
```

## 8.1 Forward Declaration 추가

```cpp
class UCFVehicleDebugNavItemWidget;
```

## 8.2 Public 함수 추가

아래 BlueprintCallable 함수를 추가한다.

```cpp
// [v1.7.0] 표시할 최상위 Section을 SectionId 기준으로 선택하고 Panel을 갱신합니다.
UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Section 선택 (SelectSectionById)", ToolTip="Navigation 또는 Blueprint에서 호출해 현재 Panel Content 영역에 표시할 최상위 Section을 선택합니다."))
void SelectSectionById(const FString& InSectionId);

// [v1.7.0] 현재 선택된 SectionId를 반환합니다.
UFUNCTION(BlueprintPure, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="선택된 Section ID 반환 (GetSelectedSectionId)", ToolTip="현재 Panel Content 영역에 표시 중인 최상위 SectionId를 반환합니다."))
FString GetSelectedSectionId() const;
```

## 8.3 BindWidgetOptional 추가

기존 `VerticalBox_DynamicSectionHost` 옆에 아래 호스트를 추가한다.

```cpp
// [v1.7.0] Navigation 항목을 붙일 Vertical Box 호스트입니다.
UPROPERTY(meta=(BindWidgetOptional))
TObjectPtr<UVerticalBox> VerticalBox_NavHost = nullptr;

// [v1.7.0] 선택된 최상위 Section 하나를 붙일 Vertical Box 호스트입니다.
UPROPERTY(meta=(BindWidgetOptional))
TObjectPtr<UVerticalBox> VerticalBox_SelectedSectionHost = nullptr;
```

## 8.4 설정 변수 추가

```cpp
// [v1.7.0] Navigation 항목 생성에 사용할 위젯 클래스입니다.
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Navigation Item 위젯 클래스 (NavItemWidgetClass)", ToolTip="Navigation 항목 생성 시 사용할 공통 Nav Item 위젯 클래스입니다. 보통 WBP_VehicleDebugNavItem을 지정합니다."))
TSubclassOf<UCFVehicleDebugNavItemWidget> NavItemWidgetClass;

// [v1.7.0] 현재 Panel Content 영역에 표시할 선택된 최상위 SectionId입니다.
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="선택된 Section ID (SelectedSectionId)", ToolTip="현재 Panel Content 영역에 표시할 최상위 SectionId입니다. 비어 있거나 유효하지 않으면 첫 번째 표시 가능한 Section으로 대체됩니다."))
FString SelectedSectionId = TEXT("Overview");
```

## 8.5 캐시 배열 추가

```cpp
// [v1.7.0] 생성해 둔 Navigation Item 위젯 캐시 배열입니다.
UPROPERTY(Transient)
TArray<TObjectPtr<UCFVehicleDebugNavItemWidget>> NavigationItemWidgetArray;

// [v1.7.0] 선택된 Section 표시용 위젯 캐시입니다.
UPROPERTY(Transient)
TObjectPtr<UCFVehicleDebugSectionWidget> SelectedSectionWidget = nullptr;
```

## 8.6 Private 함수 추가

```cpp
// [v1.7.0] Navigation 레이아웃 준비 여부를 반환합니다.
bool IsNavigationLayoutReady() const;

// [v1.7.0] 선택 Section 레이아웃 준비 여부를 반환합니다. 반드시 VerticalBox_SelectedSectionHost와 DynamicSectionWidgetClass가 모두 유효할 때만 true를 반환해야 합니다.
bool IsSelectedSectionLayoutReady() const;

// [v1.7.0] 현재 CachedPanelViewData에서 Navigation 항목 배열을 생성합니다.
TArray<FCFVehicleDebugNavItemViewData> BuildNavigationItemViewDataArray() const;

// [v1.7.0] 현재 SelectedSectionId가 유효한지 확인하고 필요하면 fallback SectionId로 보정합니다.
void ResolveSelectedSectionId();

// [v1.7.0] 현재 SelectedSectionId에 해당하는 Section ViewData를 찾습니다.
TSharedPtr<FCFVehicleDebugSectionViewData> FindSelectedSectionViewData() const;

// [v1.7.0] Navigation Item 위젯 개수를 현재 Navigation 항목 수에 맞게 준비합니다.
void EnsureNavigationItemWidgets(const TArray<FCFVehicleDebugNavItemViewData>& InNavigationItemViewDataArray);

// [v1.7.0] 현재 Navigation 항목 ViewData 기준으로 Navigation Item 위젯을 갱신합니다.
void RefreshNavigationItems();

// [v1.7.0] 선택된 Section 하나를 Content 영역에 갱신합니다.
void RefreshSelectedSectionWidget();

// [v1.7.0] 기존 전체 Section 렌더링 fallback을 사용할지 여부를 반환합니다.
bool ShouldUseLegacyFullSectionRendering() const;
```

---

## 9. Panel Widget CPP 작업

파일:

```text
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
```

## 9.1 Include 추가

```cpp
#include "UI/CFVehicleDebugNavItemWidget.h"
```

## 9.2 ResolveWidgetReferences 보강

이름 기준 보강을 추가한다.

```cpp
if (!VerticalBox_NavHost)
{
	VerticalBox_NavHost = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("VerticalBox_NavHost")));
}

if (!VerticalBox_SelectedSectionHost)
{
	VerticalBox_SelectedSectionHost = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("VerticalBox_SelectedSectionHost")));
}
```

## 9.3 RefreshFromPawn 흐름 변경

현재는 동적 Section 레이아웃이 준비되면 `RefreshDynamicSectionWidgets()`로 전체 Section을 갱신한다.
새 흐름은 아래가 되어야 한다.

```text
RefreshFromPawn()
  -> Pawn 데이터 읽기
  -> CachedPanelViewData 생성
  -> ResolveSelectedSectionId()
  -> RefreshNavigationItems()
  -> if IsSelectedSectionLayoutReady()
       RefreshSelectedSectionWidget()
     else if IsDynamicSectionLayoutReady()
       RefreshDynamicSectionWidgets()
     else
       기존 정적 Text fallback
```

`IsSelectedSectionLayoutReady()`는 반드시 아래 조건과 같은 의미로 동작해야 한다.

```cpp
return VerticalBox_SelectedSectionHost != nullptr && DynamicSectionWidgetClass != nullptr;
```

`VerticalBox_SelectedSectionHost`만 있고 `DynamicSectionWidgetClass`가 비어 있으면 선택 Section 경로로 들어가면 안 된다.
이 경우 기존 `VerticalBox_DynamicSectionHost` fallback이 보여야 한다.

주의:
- 기존 fallback 경로를 삭제하지 않는다.
- `VerticalBox_SelectedSectionHost`가 없는 기존 WBP에서는 지금처럼 전체 Section이 계속 보여야 한다.

## 9.4 SelectSectionById 구현

```cpp
void UCFVehicleDebugPanelWidget::SelectSectionById(const FString& InSectionId)
{
	// [v1.7.0] 빈 SectionId는 무시합니다.
	if (InSectionId.IsEmpty())
	{
		return;
	}

	SelectedSectionId = InSectionId;
	ResolveSelectedSectionId();
	RefreshNavigationItems();
	RefreshSelectedSectionWidget();
}
```

단, `CachedPanelViewData`가 아직 비어 있을 수 있으므로 Null/Empty 방어를 넣는다.

## 9.5 ResolveSelectedSectionId 구현 규칙

규칙:

1. `CachedPanelViewData.TopLevelSectionArray`가 비어 있으면 `SelectedSectionId`를 비우고 return.
2. `SelectedSectionId`와 일치하는 표시 가능한 Section이 있으면 유지.
3. 없으면 첫 번째 `bIsVisible == true` Section의 `SectionId`로 변경.
4. 표시 가능한 Section이 없으면 빈 문자열로 설정.

## 9.6 RefreshSelectedSectionWidget 구현 규칙

- `VerticalBox_SelectedSectionHost`가 없으면 return.
- 선택된 Section ViewData가 없으면 Host를 ClearChildren하고 return.
- `SelectedSectionWidget`이 없으면 하나 생성해서 Host에 추가한다.
- 생성에 사용할 클래스는 기존 `DynamicSectionWidgetClass`를 사용한다.
- `DynamicSectionWidgetClass`가 없으면 이 함수 경로로 들어오면 안 된다. `IsSelectedSectionLayoutReady()`가 false를 반환해 기존 DynamicHost fallback으로 돌아가야 한다.
- 매 Tick마다 새로 만들지 않는다.
- 기존 위젯에 `SetSectionViewData()`만 호출한다.

## 9.7 기존 DynamicSection Host와의 관계

`VerticalBox_SelectedSectionHost`가 준비되어 있으면 `VerticalBox_DynamicSectionHost`는 사용하지 않는다.
가능하면 숨긴다.

```cpp
if (VerticalBox_DynamicSectionHost && IsSelectedSectionLayoutReady())
{
	VerticalBox_DynamicSectionHost->SetVisibility(ESlateVisibility::Collapsed);
}
```

반대로 Selected Host가 없으면 기존 DynamicSectionHost를 그대로 사용한다.

---

## 10. NavItem Widget 새 클래스 작업

## 10.1 Header 파일

새 파일:

```text
UE/Source/CarFight_Re/Public/UI/CFVehicleDebugNavItemWidget.h
```

필수 내용:

- `UUserWidget` 상속
- `SetNavItemViewData()` 함수
- `RefreshFromNavItemViewData()` 함수
- `OnNavItemClicked()` 함수
- WBP 이벤트 그래프 fallback에서 호출할 `RequestSelectThisNavItem()` BlueprintCallable 함수
- `OwningPanelWidget`는 소유 관계가 아니므로 `TWeakObjectPtr<UCFVehicleDebugPanelWidget>` 사용
- `Text_Display`, `Text_Badge` BindWidgetOptional
- `Button_Root` BindWidgetOptional

권장 설계:

```cpp
class UButton;
class UTextBlock;
class UCFVehicleDebugPanelWidget;

UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API UCFVehicleDebugNavItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// [v1.0.0] 이 NavItem이 선택 요청을 전달할 Panel 위젯 참조를 설정합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|NavItem", meta=(DisplayName="Panel 위젯 참조 설정 (SetOwningPanelWidget)", ToolTip="이 Navigation Item이 클릭되었을 때 선택 요청을 전달할 Panel 위젯 참조를 설정합니다."))
	void SetOwningPanelWidget(UCFVehicleDebugPanelWidget* InOwningPanelWidget);

	// [v1.0.0] 현재 Navigation Item ViewData를 설정하고 표시를 갱신합니다.
	void SetNavItemViewData(const FCFVehicleDebugNavItemViewData& InNavItemViewData);

	// [v1.0.0] 현재 캐시된 Navigation Item ViewData 기준으로 표시를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|NavItem", meta=(DisplayName="Navigation Item 갱신 (RefreshFromNavItemViewData)", ToolTip="현재 캐시된 Navigation Item ViewData 기준으로 표시 텍스트, 배지, 선택 상태를 갱신합니다."))
	void RefreshFromNavItemViewData();

	// [v1.0.0] 이 NavItem이 가리키는 Section 선택을 Panel에 요청합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|NavItem", meta=(DisplayName="이 Navigation Item 선택 요청 (RequestSelectThisNavItem)", ToolTip="이 Navigation Item이 가진 SectionId를 사용해 Panel에 선택 변경을 요청합니다. WBP 이벤트 그래프 fallback에서도 이 함수를 호출합니다."))
	void RequestSelectThisNavItem();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> Button_Root = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Display = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Badge = nullptr;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UCFVehicleDebugPanelWidget> OwningPanelWidget;

	FCFVehicleDebugNavItemViewData CachedNavItemViewData;

	UFUNCTION()
	void HandleRootButtonClicked();
};
```

## 10.2 CPP 파일

새 파일:

```text
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugNavItemWidget.cpp
```

구현 규칙:

- `NativeConstruct()`에서 `Button_Root->OnClicked.AddDynamic()` 연결
- 중복 바인딩 방지를 위해 `RemoveDynamic()` 후 `AddDynamic()` 사용
- `OwningPanelWidget`는 약참조이므로 호출 전 `IsValid()` 또는 `Get()` 검사를 수행한다.
- `HandleRootButtonClicked()`는 `RequestSelectThisNavItem()`만 호출한다.
- `RequestSelectThisNavItem()`에서 유효한 Panel에 `SelectSectionById(CachedNavItemViewData.SectionId)`를 호출한다.
- C++ 바인딩만으로 클릭이 동작하지 않으면, `WBP_VehicleDebugNavItem` 이벤트 그래프에서 `Button_Root.OnClicked`를 직접 받아 `RequestSelectThisNavItem()`을 호출한다.
- `Text_Display`에는 선택 상태를 간단히 반영한다.

초기 선택 표시 방식은 단순 문자열로 충분하다.

```text
> Drive
  Input
```

즉, 선택된 항목이면 DisplayText 앞에 `> `를 붙인다.
스타일 색상/브러시 변경은 이번 작업 범위 밖이다.

---

## 11. Navigation 생성 규칙

`UCFVehicleDebugPanelWidget::BuildNavigationItemViewDataArray()`는 아래 기준으로 작성한다.

1. `CachedPanelViewData.TopLevelSectionArray`를 순회한다.
2. 유효하지 않은 Section은 제외한다.
3. `bIsVisible == false`인 Section은 제외한다.
4. `bShowInNavigation == false`인 Section은 제외한다.
5. SectionId, TitleText, NavigationGroup, NavigationOrder, BadgeText를 NavItem에 복사한다.
6. `bIsSelected`는 `SectionId == SelectedSectionId`로 설정한다.
7. `NavigationGroup`, `NavigationOrder`, `DisplayText` 순서로 정렬한다.

주의:
- 그룹 Header UI는 이번 작업에서 만들지 않아도 된다.
- 정렬만 적용해 둔다.

## 11.1 Navigation Item 재생성 금지 규칙

`RefreshNavigationItems()` 안에서 매번 `VerticalBox_NavHost->ClearChildren()` 후 전체 NavItem을 다시 만들면 안 된다.

반드시 아래 흐름을 따른다.

```text
RefreshNavigationItems()
  -> BuildNavigationItemViewDataArray()
  -> EnsureNavigationItemWidgets(NavItemViewDataArray)
  -> 각 기존 NavItemWidget에 SetNavItemViewData() 호출
```

`EnsureNavigationItemWidgets()` 구현 규칙:

1. 현재 `NavigationItemWidgetArray.Num()`과 필요한 NavItem 수가 같으면 새로 생성하지 않는다.
2. 개수가 같으면 `VerticalBox_NavHost->ClearChildren()`을 호출하지 않는다.
3. 개수가 달라졌을 때만 Host를 비우고 필요한 개수만큼 재생성한다.
4. 선택 상태 변경만 발생한 경우에는 기존 위젯에 `SetNavItemViewData()`만 호출한다.
5. 매 Tick마다 NavItem 위젯이 Destroy/Create 되지 않게 한다.

---

## 12. Section별 NavigationOrder 권장값

현재 Builder 함수에서 아래 값을 설정한다.

```text
Overview: Core, 10
Drive:    Core, 20
Input:    Core, 30
Runtime:  Core, 40
```

파일:

```text
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
```

수정 위치:

```text
BuildOverviewSectionViewData()
BuildDriveSectionViewData()
BuildInputSectionViewData()
BuildRuntimeSectionViewData()
```

각 Section 생성 직후 NavigationGroup / NavigationOrder를 설정한다.

예:

```cpp
OverviewSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Core;
OverviewSectionViewData->NavigationOrder = 10;
OverviewSectionViewData->bShowInNavigation = true;
```

---

## 13. 문서 갱신 작업

## 13.1 PanelGuide 갱신

파일:

```text
Document/ProjectSSOT/Plan/VehicleDebugPlan/CF_VehicleDebugPanelGuide.md
```

추가할 내용:

- `VerticalBox_SelectedSectionHost` 추가 예정
- `VerticalBox_NavHost` 추가 예정
- `WBP_VehicleDebugNavItem` 추가 예정
- 기존 `VerticalBox_DynamicSectionHost`는 fallback으로 유지
- BP/UMG 수동 작업 체크리스트

## 13.2 PanelUX 갱신

파일:

```text
Document/ProjectSSOT/Plan/VehicleDebugPlan/CF_VehicleDebugPanelUX.md
```

추가할 내용:

- Phase UX-1/2 C++ 기반 작업이 착수/완료되면 Changelog 갱신
- 실제 구현된 파일 목록 추가
- fallback 정책 명시

## 13.3 README 갱신

필요하면 README의 Changelog에 이번 작업을 추가한다.

---

## 14. BP/UMG 수동 작업 체크리스트

Codex는 `.uasset`을 직접 수정하지 않는다.
아래 내용은 문서에 남긴다.

수동으로 해야 할 작업:

1. `WBP_VehicleDebugPanel` 루트에 `VerticalBox_Root`가 없으면 추가한다.
2. 기존 `ScrollBox_Root`만 쓰는 구조라면, 전환 단계에서는 `VerticalBox_Root` 아래에 `HorizontalBox_Main`을 만들고 그 안에 Navigation 영역과 Content 영역을 나란히 둔다.
3. `HorizontalBox_Main` 왼쪽에는 `ScrollBox_Navigation`을 두고, 그 안에 `VerticalBox_NavHost`를 배치한다.
4. `HorizontalBox_Main` 오른쪽에는 `ScrollBox_Content`를 두고, 그 안에 `VerticalBox_SelectedSectionHost`를 배치한다.
5. 기존 `VerticalBox_DynamicSectionHost`는 fallback 확인 전까지 제거하지 않는다.
6. 기존 `VerticalBox_DynamicSectionHost`는 가능하면 `ScrollBox_Content` 내부 또는 같은 Content 영역에 임시 fallback으로 남긴다.
7. `WBP_VehicleDebugNavItem` 생성
8. `WBP_VehicleDebugNavItem` 부모 클래스를 `CFVehicleDebugNavItemWidget`으로 설정
9. `WBP_VehicleDebugNavItem` 내부에 `Button_Root`, `Text_Display`, `Text_Badge` 생성
10. `WBP_VehicleDebugPanel`의 `NavItemWidgetClass`에 `WBP_VehicleDebugNavItem` 지정
11. `WBP_VehicleDebugPanel`의 `DynamicSectionWidgetClass`가 기존 `WBP_VehicleDebugSection`을 가리키는지 확인
12. `DynamicSectionWidgetClass`가 비어 있으면 `VerticalBox_SelectedSectionHost`가 있어도 선택 Section 경로가 아니라 기존 DynamicHost fallback이 보여야 한다.
13. C++ 바인딩만으로 `Button_Root` 클릭이 동작하지 않으면, `WBP_VehicleDebugNavItem` 이벤트 그래프에서 `Button_Root.OnClicked`를 직접 연결한다.
14. 이벤트 그래프 fallback 연결 시에는 `Button_Root.OnClicked -> RequestSelectThisNavItem()`만 호출한다. `CachedNavItemViewData`는 C++ private 캐시이므로 BP에서 직접 접근하지 않는다.
15. Navigation Item 클릭 후 `SelectedSectionId`가 바뀌고 오른쪽 Content 영역의 Section도 바뀌는지 확인한다.

---

## 15. 빌드 검증

반드시 아래 빌드를 통과시킨다.

```text
CarFight_ReEditor Win64 Development
```

가능하면 Unreal Editor에서 아래도 확인한다.

1. 기존 WBP 상태에서 Panel이 여전히 표시되는지 확인
2. `VerticalBox_SelectedSectionHost`가 없는 경우 기존 전체 Section 렌더링 fallback 확인
3. 새 Host를 추가한 뒤 선택된 Section 하나만 표시되는지 확인
4. Navigation Item 클릭 시 SelectedSectionId가 바뀌는지 확인
5. 매 Tick 중 위젯이 계속 재생성되지 않는지 확인

---

## 16. 완료 기준

이번 Codex 작업 완료 기준은 아래다.

1. C++ 빌드가 성공한다.
2. 기존 `WBP_VehicleDebugPanel`이 새 Host 없이도 깨지지 않는다.
3. `SelectedSectionId`가 추가되어 선택된 Section 하나를 표시할 수 있다.
4. `VerticalBox_SelectedSectionHost`와 `DynamicSectionWidgetClass`가 모두 유효하면 선택 Section 렌더링 경로를 사용한다.
5. `VerticalBox_NavHost`와 `NavItemWidgetClass`가 있으면 Navigation Item이 생성된다.
6. Navigation Item 클릭으로 `SelectSectionById()`가 호출된다.
7. 기존 `WBP_VehicleDebugSection` / `WBP_VehicleDebugFieldRow` 구조는 유지된다.
8. 새 카테고리 추가 시 루트 WBP에 새 Button/Text를 직접 추가하지 않는 방향이 유지된다.
9. 문서에 BP/UMG 수동 작업 체크리스트가 남아 있다.

---

## 17. 실패 시 되돌릴 기준

아래 문제가 생기면 작업을 멈추고 되돌린다.

1. 기존 Panel이 새 WBP 작업 전부터 표시되지 않는다.
2. `VerticalBox_DynamicSectionHost` fallback이 깨진다.
3. 매 Tick마다 Section/NavItem 위젯이 계속 새로 생성된다.
4. 기존 `RefreshFromPawn()`의 정적 Text fallback이 사라진다.
5. UMG 자산을 직접 수정하려고 한다.
6. 예상 카테고리들을 한 번에 구현하려고 한다.

---

## 18. Codex 작업 요약

이번 작업의 핵심은 아래 한 줄이다.

> VehicleDebug Panel에 `SelectedSectionId`와 Navigation C++ 기반을 추가하되, 기존 전체 Section 렌더링 fallback을 유지한다.

이번 작업은 UX 최종 완성이 아니라, 앞으로 카테고리를 하나씩 추가할 수 있게 만드는 안정적인 1차 기반 작업이다.

---

## 19. Changelog

### v0.2.0 - 2026-04-27
- 지시서 기준 Phase UX-1/2 C++ 기반 작업을 완료했다.
- `CFVehicleDebugNavItemWidget` 추가, `SelectedSectionId` 기반 선택 Section 렌더링, Navigation Item 생성, 기존 DynamicHost fallback 유지가 구현되었다.
- `CarFight_ReEditor Win64 Development` 빌드 성공 결과를 반영했다.

### v0.1.2 - 2026-04-27
- WBP 이벤트 그래프 fallback에서 private `CachedNavItemViewData`를 직접 접근하지 않도록 `RequestSelectThisNavItem()` 공개 함수를 추가하는 기준을 반영했다.
- `HandleRootButtonClicked()`는 `RequestSelectThisNavItem()`만 호출하도록 클릭 처리 경로를 단일화했다.
- 완료 기준의 선택 Section 렌더링 조건을 `VerticalBox_SelectedSectionHost`와 `DynamicSectionWidgetClass`가 모두 유효한 경우로 보정했다.

### v0.1.1 - 2026-04-27
- 검토 결과를 반영해 `IsSelectedSectionLayoutReady()` 조건을 `VerticalBox_SelectedSectionHost`와 `DynamicSectionWidgetClass` 모두 유효한 경우로 명확히 고정했다.
- `DynamicSectionWidgetClass` 누락 시 선택 Section 경로가 아니라 기존 DynamicHost fallback으로 돌아가야 한다는 규칙을 추가했다.
- NavItem 클릭 C++ 바인딩 실패 시 WBP 이벤트 그래프 fallback 연결 규칙을 추가했다.
- NavItem 위젯 캐시는 개수가 같으면 재생성하지 않고 `SetNavItemViewData()`만 호출하도록 명시했다.
- `OwningPanelWidget` 참조는 소유 관계가 아니므로 `TWeakObjectPtr` 사용을 권장안으로 고정했다.
- BP/UMG 수동 체크리스트에 Navigation/Content 배치 구조와 이벤트 검증 절차를 보강했다.

### v0.1.0 - 2026-04-27
- VehicleDebug Panel UX 1차 개선을 위한 Codex 작업 지시서를 신규 작성했다.
- Phase UX-1/2 C++ 기반 작업 범위, 수정 파일, 금지 사항, 검증 기준을 정리했다.
- UMG `.uasset` 직접 편집 금지와 BP/UMG 수동 연결 체크리스트를 분리했다.
