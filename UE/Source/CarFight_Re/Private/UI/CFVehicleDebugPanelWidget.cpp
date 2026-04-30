// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.7.0
// Date: 2026-04-27
// Description: VehicleDebug Panel용 C++ 부모 위젯 클래스 구현입니다.
// Scope: VehicleDebug Overview / Drive / Input / Runtime 카테고리를 읽어 Navigation + Selected Section 기반 표시와 기존 fallback 표시를 안정적으로 지원합니다.

#include "UI/CFVehicleDebugPanelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "UI/CFVehicleDebugNavItemWidget.h"
#include "UI/CFVehicleDebugSectionWidget.h"
#include "Input/Reply.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"

// [v1.0.0] 기본 `라벨: 값` 필드 ViewData를 생성합니다.
FCFVehicleDebugFieldViewData FCFVehicleDebugFieldViewData::MakeLabelValueField(
	const FString& InFieldId,
	const FString& InLabelText,
	const FString& InValueText,
	bool bInIsImportant,
	const FString& InTooltipText)
{
	// [v1.0.0] 생성할 필드 ViewData 결과입니다.
	FCFVehicleDebugFieldViewData FieldViewData;
	FieldViewData.FieldId = InFieldId;
	FieldViewData.LabelText = InLabelText;
	FieldViewData.ValueText = InValueText;
	FieldViewData.TooltipText = InTooltipText;
	FieldViewData.FieldStyle = bInIsImportant ? ECFVehicleDebugFieldStyle::Important : ECFVehicleDebugFieldStyle::Default;
	FieldViewData.bIsImportant = bInIsImportant;
	FieldViewData.bIsVisible = true;
	return FieldViewData;
}

// [v1.0.0] 여러 줄 값 표시용 필드 ViewData를 생성합니다.
FCFVehicleDebugFieldViewData FCFVehicleDebugFieldViewData::MakeMultilineField(
	const FString& InFieldId,
	const FString& InLabelText,
	const FString& InValueText,
	const FString& InTooltipText)
{
	// [v1.0.0] 생성할 여러 줄 필드 ViewData 결과입니다.
	FCFVehicleDebugFieldViewData FieldViewData;
	FieldViewData.FieldId = InFieldId;
	FieldViewData.LabelText = InLabelText;
	FieldViewData.ValueText = InValueText;
	FieldViewData.TooltipText = InTooltipText;
	FieldViewData.FieldStyle = ECFVehicleDebugFieldStyle::Multiline;
	FieldViewData.bIsImportant = false;
	FieldViewData.bIsVisible = true;
	return FieldViewData;
}

// [v1.0.0] 새로운 섹션 ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> FCFVehicleDebugSectionViewData::MakeSection(
	const FString& InSectionId,
	const FString& InTitleText,
	ECFVehicleDebugSectionKind InSectionKind,
	bool bInDefaultExpanded)
{
	// [v1.0.0] 생성할 섹션 ViewData 결과입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> SectionViewData = MakeShared<FCFVehicleDebugSectionViewData>();
	SectionViewData->SectionId = InSectionId;
	SectionViewData->TitleText = InTitleText;
	SectionViewData->SectionKind = InSectionKind;
	SectionViewData->bDefaultExpanded = bInDefaultExpanded;
	SectionViewData->bIsVisible = true;
	return SectionViewData;
}

// [v1.0.0] 현재 섹션에 필드 하나를 추가합니다.
void FCFVehicleDebugSectionViewData::AddField(const FCFVehicleDebugFieldViewData& InFieldViewData)
{
	FieldArray.Add(InFieldViewData);
}

// [v1.0.0] 현재 섹션에 하위 섹션 하나를 추가합니다.
void FCFVehicleDebugSectionViewData::AddChildSection(const TSharedPtr<FCFVehicleDebugSectionViewData>& InChildSectionViewData)
{
	if (InChildSectionViewData.IsValid())
	{
		ChildSectionArray.Add(InChildSectionViewData);
	}
}

// [v1.0.0] 현재 Panel ViewData를 초기화합니다.
void FCFVehicleDebugPanelViewData::Reset()
{
	PanelId.Reset();
	PanelTitleText.Reset();
	GeneratedFrameNumber = 0;
	GeneratedTimeSeconds = 0.0;
	TopLevelSectionArray.Reset();
}

// [v1.0.0] 최상위 섹션 하나를 추가합니다.
void FCFVehicleDebugPanelViewData::AddTopLevelSection(const TSharedPtr<FCFVehicleDebugSectionViewData>& InSectionViewData)
{
	if (InSectionViewData.IsValid())
	{
		TopLevelSectionArray.Add(InSectionViewData);
	}
}

void UCFVehicleDebugPanelWidget::SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef)
{
	// [v1.0.0] Panel이 읽을 차량 Pawn 참조를 저장합니다.
	VehiclePawnRef = InVehiclePawnRef;

	RefreshFromPawn();
	UpdatePanelVisibility();
}

void UCFVehicleDebugPanelWidget::RefreshFromPawn()
{
	// [v1.0.0] 유효한 Pawn이 없으면 Panel을 숨기고 갱신을 중단합니다.
	if (!IsValid(VehiclePawnRef))
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// [v1.0.0] 현재 Pawn 기준 최신 Overview 카테고리를 가져옵니다.
	const FCFVehicleDebugOverview LatestOverview = VehiclePawnRef->GetVehicleDebugOverview();

	// [v1.0.0] 현재 Pawn 기준 최신 Drive 카테고리를 가져옵니다.
	const FCFVehicleDebugDrive LatestDrive = VehiclePawnRef->GetVehicleDebugDrive();

	// [v1.0.0] 현재 Pawn 기준 최신 Input 카테고리를 가져옵니다.
	const FCFVehicleDebugInput LatestInput = VehiclePawnRef->GetVehicleDebugInput();

	// [v1.0.0] 현재 Pawn 기준 최신 Runtime 카테고리를 가져옵니다.
	const FCFVehicleDebugRuntime LatestRuntime = VehiclePawnRef->GetVehicleDebugRuntime();

	// [v1.0.0] 현재 Panel 캐시에 최신 Overview를 저장합니다.
	CachedOverview = LatestOverview;

	// [v1.0.0] 현재 Panel 캐시에 최신 Drive를 저장합니다.
	CachedDrive = LatestDrive;

	// [v1.0.0] 현재 Panel 캐시에 최신 Input을 저장합니다.
	CachedInput = LatestInput;

	// [v1.0.0] 현재 Panel 캐시에 최신 Runtime을 저장합니다.
	CachedRuntime = LatestRuntime;

	// [v1.5.0] 현재 Snapshot 기준 최신 Panel ViewData를 생성해 캐시에 저장합니다.
	CachedPanelViewData = BuildVehicleDebugPanelViewData();

	// [v1.7.0] 현재 선택된 SectionId를 유효한 최상위 Section 기준으로 보정합니다.
	ResolveSelectedSectionId();

	// [v1.7.0] Navigation 레이아웃이 준비된 경우 Navigation 항목 표시를 갱신합니다.
	RefreshNavigationItems();

	// [v1.7.0] 선택 Section 레이아웃이 준비된 경우 선택된 Section 하나만 갱신합니다.
	if (IsSelectedSectionLayoutReady())
	{
		RefreshSelectedSectionWidget();
	}
	else if (ShouldUseLegacyFullSectionRendering())
	{
		RefreshDynamicSectionWidgets();
	}
	else
	{
		ApplyVehicleDebugOverview(CachedOverview);
		ApplyVehicleDebugDrive(CachedDrive);
		ApplyVehicleDebugInput(CachedInput);
		ApplyVehicleDebugRuntime(CachedRuntime);
		UpdateCategoryHeaderTexts();
	}
}

void UCFVehicleDebugPanelWidget::SelectSectionById(const FString& InSectionId)
{
	// [v1.7.0] 빈 SectionId는 선택 요청으로 처리하지 않습니다.
	if (InSectionId.IsEmpty())
	{
		return;
	}

	SelectedSectionId = InSectionId;
	ResolveSelectedSectionId();
	RefreshNavigationItems();

	if (IsSelectedSectionLayoutReady())
	{
		RefreshSelectedSectionWidget();
	}
}

FString UCFVehicleDebugPanelWidget::GetSelectedSectionId() const
{
	return SelectedSectionId;
}

void UCFVehicleDebugPanelWidget::ApplyVehicleDebugOverview(const FCFVehicleDebugOverview& InOverview)
{
	// [v1.0.0] Overview 섹션 본문 문자열입니다.
	const FString OverviewBodyText = BuildOverviewBodyText(InOverview);

	// [v1.4.0] Overview Last Transition 하위 섹션 본문 문자열입니다.
	const FString OverviewLastTransitionBodyText = BuildOverviewLastTransitionBodyText(InOverview);

	// [v1.0.0] Overview 섹션 본문 텍스트를 반영합니다.
	if (Text_OverviewBody)
	{
		Text_OverviewBody->SetText(FText::FromString(OverviewBodyText));
	}

	// [v1.4.0] Overview Last Transition 본문 텍스트를 반영합니다.
	if (Text_OverviewLastTransitionBody)
	{
		Text_OverviewLastTransitionBody->SetText(FText::FromString(OverviewLastTransitionBodyText));
	}
}

void UCFVehicleDebugPanelWidget::ApplyVehicleDebugDrive(const FCFVehicleDebugDrive& InDrive)
{
	// [v1.0.0] Drive 섹션 본문 문자열입니다.
	const FString DriveBodyText = BuildDriveBodyText(InDrive);

	// [v1.4.0] Drive Transition 하위 섹션 본문 문자열입니다.
	const FString DriveTransitionBodyText = BuildDriveTransitionBodyText(InDrive);

	// [v1.0.0] Drive 섹션 본문 텍스트를 반영합니다.
	if (Text_DriveBody)
	{
		Text_DriveBody->SetText(FText::FromString(DriveBodyText));
	}

	// [v1.4.0] Drive Transition 본문 텍스트를 반영합니다.
	if (Text_DriveTransitionBody)
	{
		Text_DriveTransitionBody->SetText(FText::FromString(DriveTransitionBodyText));
	}
}

void UCFVehicleDebugPanelWidget::ApplyVehicleDebugInput(const FCFVehicleDebugInput& InInput)
{
	// [v1.0.0] Input 섹션 본문 문자열입니다.
	const FString InputBodyText = BuildInputBodyText(InInput);

	// [v1.0.0] Input 섹션 본문 텍스트를 반영합니다.
	if (Text_InputBody)
	{
		Text_InputBody->SetText(FText::FromString(InputBodyText));
	}
}

void UCFVehicleDebugPanelWidget::ApplyVehicleDebugRuntime(const FCFVehicleDebugRuntime& InRuntime)
{
	// [v1.0.0] Runtime 섹션 본문 문자열입니다.
	const FString RuntimeBodyText = BuildRuntimeBodyText(InRuntime);

	// [v1.3.0] Runtime Summary 하위 섹션 본문 문자열입니다.
	const FString RuntimeSummaryBodyText = BuildRuntimeSummaryBodyText(InRuntime);

	// [v1.3.0] Last Init 하위 섹션 본문 문자열입니다.
	const FString RuntimeLastInitBodyText = BuildRuntimeLastInitBodyText(InRuntime);

	// [v1.3.0] Last Validation 하위 섹션 본문 문자열입니다.
	const FString RuntimeLastValidationBodyText = BuildRuntimeLastValidationBodyText(InRuntime);

	// [v1.0.0] Runtime 섹션 본문 텍스트를 반영합니다.
	if (Text_RuntimeBody)
	{
		Text_RuntimeBody->SetText(FText::FromString(RuntimeBodyText));
	}

	// [v1.3.0] Runtime Summary 본문 텍스트를 반영합니다.
	if (Text_RuntimeSummaryBody)
	{
		Text_RuntimeSummaryBody->SetText(FText::FromString(RuntimeSummaryBodyText));
	}

	// [v1.3.0] Last Init 본문 텍스트를 반영합니다.
	if (Text_RuntimeLastInitBody)
	{
		Text_RuntimeLastInitBody->SetText(FText::FromString(RuntimeLastInitBodyText));
	}

	// [v1.3.0] Last Validation 본문 텍스트를 반영합니다.
	if (Text_RuntimeLastValidationBody)
	{
		Text_RuntimeLastValidationBody->SetText(FText::FromString(RuntimeLastValidationBodyText));
	}
}

void UCFVehicleDebugPanelWidget::UpdatePanelVisibility()
{
	// [v1.0.0] 유효한 Pawn이 없으면 Panel을 숨깁니다.
	if (!IsValid(VehiclePawnRef))
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// [v1.0.0] Pawn의 Panel 표시 정책을 계산한 결과입니다.
	const bool bShouldShowPanel = VehiclePawnRef->ShouldShowVehicleDebugPanel();

	// [v1.3.3] Panel 루트가 NativeOnPreviewMouseButtonDown에서 바깥 클릭을 감지할 수 있도록 Visible을 사용합니다.
	SetVisibility(bShouldShowPanel ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// [v1.2.0] Panel이 표시될 때 자동 상호작용 진입 옵션이 켜져 있으면 상호작용 모드를 켭니다.
	if (bShouldShowPanel && bAutoEnterInteractionModeWhenVisible && !bIsPanelInteractionModeActive)
	{
		EnterPanelInteractionMode();
	}

	// [v1.2.0] Panel이 숨겨지면 상호작용 모드도 함께 종료합니다.
	if (!bShouldShowPanel && bIsPanelInteractionModeActive)
	{
		ExitPanelInteractionMode();
	}
}

void UCFVehicleDebugPanelWidget::ToggleOverviewExpanded()
{
	// [v1.1.0] Overview 카테고리 펼침 상태를 반전합니다.
	bIsOverviewExpanded = !bIsOverviewExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleOverviewLastTransitionExpanded()
{
	// [v1.4.0] Overview Last Transition 하위 섹션 펼침 상태를 반전합니다.
	bIsOverviewLastTransitionExpanded = !bIsOverviewLastTransitionExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleDriveExpanded()
{
	// [v1.1.0] Drive 카테고리 펼침 상태를 반전합니다.
	bIsDriveExpanded = !bIsDriveExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleDriveTransitionExpanded()
{
	// [v1.4.0] Drive Transition 하위 섹션 펼침 상태를 반전합니다.
	bIsDriveTransitionExpanded = !bIsDriveTransitionExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleInputExpanded()
{
	// [v1.1.0] Input 카테고리 펼침 상태를 반전합니다.
	bIsInputExpanded = !bIsInputExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleRuntimeExpanded()
{
	// [v1.1.0] Runtime 카테고리 펼침 상태를 반전합니다.
	bIsRuntimeExpanded = !bIsRuntimeExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleRuntimeSummaryExpanded()
{
	// [v1.3.0] Runtime Summary 하위 섹션 펼침 상태를 반전합니다.
	bIsRuntimeSummaryExpanded = !bIsRuntimeSummaryExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleRuntimeLastInitExpanded()
{
	// [v1.3.0] Last Init 하위 섹션 펼침 상태를 반전합니다.
	bIsRuntimeLastInitExpanded = !bIsRuntimeLastInitExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleRuntimeLastValidationExpanded()
{
	// [v1.3.0] Last Validation 하위 섹션 펼침 상태를 반전합니다.
	bIsRuntimeLastValidationExpanded = !bIsRuntimeLastValidationExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::EnterPanelInteractionMode()
{
	// [v1.2.0] Panel 상호작용 모드를 활성화합니다.
	bIsPanelInteractionModeActive = true;

	// [v1.2.0] 상호작용 입력을 적용할 PlayerController입니다.
	APlayerController* PlayerController = ResolveOwningPlayerController();

	if (!PlayerController)
	{
		return;
	}

	// [v1.2.0] Panel 위젯에 포커스를 주는 Game and UI 입력 모드입니다.
	FInputModeGameAndUI GameAndUiInputMode;
	GameAndUiInputMode.SetWidgetToFocus(TakeWidget());
	GameAndUiInputMode.SetHideCursorDuringCapture(false);
	GameAndUiInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetShowMouseCursor(true);
	PlayerController->SetInputMode(GameAndUiInputMode);
}

void UCFVehicleDebugPanelWidget::ExitPanelInteractionMode()
{
	// [v1.2.0] Panel 상호작용 모드를 비활성화합니다.
	bIsPanelInteractionModeActive = false;

	// [v1.2.0] 상호작용 입력을 적용할 PlayerController입니다.
	APlayerController* PlayerController = ResolveOwningPlayerController();

	if (!PlayerController)
	{
		return;
	}

	// [v1.2.0] 게임 입력 전용 입력 모드입니다.
	FInputModeGameOnly GameOnlyInputMode;

	PlayerController->SetShowMouseCursor(false);
	PlayerController->SetInputMode(GameOnlyInputMode);
}

// [v1.5.0] 현재 Snapshot 기준 Panel ViewData 캐시를 반환합니다.
const FCFVehicleDebugPanelViewData& UCFVehicleDebugPanelWidget::GetCachedPanelViewData() const
{
	return CachedPanelViewData;
}

void UCFVehicleDebugPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// [v1.2.3] 디자이너에서 변수 체크가 빠졌더라도 이름 기준으로 위젯 참조를 다시 찾습니다.
	ResolveWidgetReferences();

	// [v1.2.6] 카테고리 헤더 버튼 입력은 WBP 이벤트 그래프에서 Toggle 함수 호출로 연결하는 것을 기본안으로 사용합니다.

	RefreshFromPawn();
	if (!IsDynamicSectionLayoutReady())
	{
		UpdateCategoryBodyVisibility();
	}
	UpdateCategoryHeaderTexts();
	UpdatePanelVisibility();
}

void UCFVehicleDebugPanelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// [v1.0.0] 자동 갱신이 꺼져 있으면 매 프레임 갱신을 건너뜁니다.
	if (!bAutoRefreshEveryTick)
	{
		return;
	}

	RefreshFromPawn();
	if (!IsDynamicSectionLayoutReady())
	{
		UpdateCategoryBodyVisibility();
	}
	UpdatePanelVisibility();
}

FReply UCFVehicleDebugPanelWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// [v1.3.2] 현재 상호작용 모드가 아니면 기본 입력 흐름을 그대로 사용합니다.
	if (!bIsPanelInteractionModeActive)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	// [v1.3.2] 실제 Panel 루트 위젯이 없으면 기본 입력 흐름을 그대로 사용합니다.
	if (!Border_RootPanel)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	// [v1.3.2] 현재 마우스 클릭 위치의 스크린 좌표입니다.
	const FVector2D ScreenSpacePosition = InMouseEvent.GetScreenSpacePosition();

	// [v1.3.2] 현재 클릭이 실제 Panel 본체 내부인지 여부입니다.
	const bool bIsClickInsidePanel = Border_RootPanel->GetCachedGeometry().IsUnderLocation(ScreenSpacePosition);

	// [v1.3.2] Panel 바깥 클릭이면 상호작용 모드를 종료하고 이번 클릭을 처리 완료로 반환합니다.
	if (!bIsClickInsidePanel)
	{
		ExitPanelInteractionMode();
		return FReply::Handled();
	}

	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

FString UCFVehicleDebugPanelWidget::ConvertEnumValueToDisplayString(const TCHAR* EnumValueString) const
{
	// [v1.0.0] Panel에 표시할 enum 문자열 원본입니다.
	FString DisplayString = EnumValueString ? EnumValueString : TEXT("");

	// [v1.0.0] 마지막 `::` 뒤 값만 남기기 위한 구분 인덱스입니다.
	int32 ValueSeparatorIndex = INDEX_NONE;

	// [v1.0.0] enum 풀네임에서 실제 값 이름 시작 위치를 찾습니다.
	if (DisplayString.FindLastChar(TEXT(':'), ValueSeparatorIndex) && ValueSeparatorIndex + 1 < DisplayString.Len())
	{
		return DisplayString.Mid(ValueSeparatorIndex + 1);
	}

	return DisplayString;
}

FString UCFVehicleDebugPanelWidget::BuildOverviewBodyText(const FCFVehicleDebugOverview& InOverview) const
{
	// [v1.0.0] Runtime 준비 상태 표시 문자열입니다.
	const FString RuntimeReadyText = InOverview.bRuntimeReady ? TEXT("True") : TEXT("False");

	// [v1.0.0] Overview 본문 문자열입니다.
	const FString OverviewBodyText = FString::Printf(
		TEXT("Ready: %s\nState: %s\nSpeed: %.1f km/h\nForward: %.1f km/h\nInput Mode: %s\nInput Owner: %s"),
		*RuntimeReadyText,
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.CurrentDriveState)),
		InOverview.SpeedKmh,
		InOverview.ForwardSpeedKmh,
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.DeviceMode)),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.InputOwner)));

	return OverviewBodyText;
}

FString UCFVehicleDebugPanelWidget::BuildOverviewLastTransitionBodyText(const FCFVehicleDebugOverview& InOverview) const
{
	// [v1.4.0] Panel 표시용 Overview Last Transition 문자열입니다.
	const FString OverviewLastTransitionPanelText = FormatLongSummaryForPanel(InOverview.LastTransitionShortText, TEXT("DriveStateTransition: "));

	return OverviewLastTransitionPanelText;
}

FString UCFVehicleDebugPanelWidget::BuildDriveBodyText(const FCFVehicleDebugDrive& InDrive) const
{
	// [v1.0.0] Drive 상태 변경 여부 표시 문자열입니다.
	const FString DriveChangedText = InDrive.bDriveStateChangedThisFrame ? TEXT("True") : TEXT("False");

	// [v1.0.0] Handbrake 표시 문자열입니다.
	const FString HandbrakeText = InDrive.bHandbrake ? TEXT("On") : TEXT("Off");

	// [v1.0.0] Drive 본문 문자열입니다.
	const FString DriveBodyText = FString::Printf(
		TEXT("Current State: %s\nPrevious State: %s\nChanged This Frame: %s\nSpeed: %.1f km/h\nForward: %.1f km/h\nThrottle: %.2f\nBrake: %.2f\nSteering: %.2f\nHandbrake: %s"),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InDrive.CurrentDriveState)),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InDrive.PreviousDriveState)),
		*DriveChangedText,
		InDrive.SpeedKmh,
		InDrive.ForwardSpeedKmh,
		InDrive.Throttle,
		InDrive.Brake,
		InDrive.Steering,
		*HandbrakeText);

	return DriveBodyText;
}

FString UCFVehicleDebugPanelWidget::BuildDriveTransitionBodyText(const FCFVehicleDebugDrive& InDrive) const
{
	// [v1.4.0] Panel 표시용 Drive Transition 문자열입니다.
	const FString DriveTransitionPanelText = FormatLongSummaryForPanel(InDrive.DriveStateTransitionSummary, TEXT("DriveStateTransition: "));

	return DriveTransitionPanelText;
}

FString UCFVehicleDebugPanelWidget::BuildInputBodyText(const FCFVehicleDebugInput& InInput) const
{
	// [v1.0.0] 검은 영역 유지 정책 사용 여부 표시 문자열입니다.
	const FString BlackZoneHoldText = InInput.bUsedBlackZoneHold ? TEXT("True") : TEXT("False");

	// [v1.0.0] Input 본문 문자열입니다.
	const FString InputBodyText = FString::Printf(
		TEXT("Device Mode: %s\nInput Owner: %s\nMove Zone: %s\nMove Intent: %s\nMove Raw: (%.2f, %.2f)\nMove Magnitude: %.2f\nMove Angle: %.1f\nBlack Hold: %s"),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.DeviceMode)),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.InputOwner)),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.MoveZone)),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.MoveIntent)),
		InInput.MoveRaw.X,
		InInput.MoveRaw.Y,
		InInput.MoveMagnitude,
		InInput.MoveAngle,
		*BlackZoneHoldText);

	return InputBodyText;
}

FString UCFVehicleDebugPanelWidget::BuildRuntimeBodyText(const FCFVehicleDebugRuntime& InRuntime) const
{
	// [v1.0.0] Runtime 준비 상태 표시 문자열입니다.
	const FString RuntimeReadyText = InRuntime.bRuntimeReady ? TEXT("True") : TEXT("False");

	// [v1.0.0] Drive 컴포넌트 존재 여부 표시 문자열입니다.
	const FString HasDriveComponentText = InRuntime.bHasDriveComponent ? TEXT("True") : TEXT("False");

	// [v1.0.0] WheelSync 컴포넌트 존재 여부 표시 문자열입니다.
	const FString HasWheelSyncComponentText = InRuntime.bHasWheelSyncComponent ? TEXT("True") : TEXT("False");

	// [v1.0.0] Runtime 본문 문자열입니다.
	const FString RuntimeBodyText = FString::Printf(
		TEXT("Ready: %s\nHas DriveComp: %s\nHas WheelSyncComp: %s"),
		*RuntimeReadyText,
		*HasDriveComponentText,
		*HasWheelSyncComponentText);

	return RuntimeBodyText;
}

FString UCFVehicleDebugPanelWidget::BuildRuntimeSummaryBodyText(const FCFVehicleDebugRuntime& InRuntime) const
{
	// [v1.3.0] Panel 표시용 Runtime Summary 문자열입니다.
	const FString RuntimeSummaryPanelText = FormatLongSummaryForPanel(InRuntime.RuntimeSummary, TEXT("VehicleRuntime: "));

	return RuntimeSummaryPanelText;
}

FString UCFVehicleDebugPanelWidget::BuildRuntimeLastInitBodyText(const FCFVehicleDebugRuntime& InRuntime) const
{
	// [v1.3.0] Panel 표시용 Last Init 문자열입니다.
	const FString RuntimeLastInitPanelText = FormatLongSummaryForPanel(InRuntime.LastInitAttemptSummary, TEXT("VehicleRuntime: "));

	return RuntimeLastInitPanelText;
}

FString UCFVehicleDebugPanelWidget::BuildRuntimeLastValidationBodyText(const FCFVehicleDebugRuntime& InRuntime) const
{
	// [v1.3.0] Panel 표시용 Last Validation 문자열입니다.
	const FString RuntimeLastValidationPanelText = FormatLongSummaryForPanel(InRuntime.LastValidationSummary, TEXT("VehicleRuntime: "));

	return RuntimeLastValidationPanelText;
}

FString UCFVehicleDebugPanelWidget::FormatLongSummaryForPanel(const FString& InSummaryText, const FString& InKnownPrefix) const
{
	// [v1.1.0] Panel 표시용으로 정리할 원본 요약 문자열입니다.
	FString FormattedSummaryText = InSummaryText;

	// [v1.1.0] 패널에서 제거할 수 있는 공통 접두사 길이입니다.
	const int32 KnownPrefixLength = InKnownPrefix.Len();

	// [v1.1.0] 공통 접두사가 있으면 제거해 본문만 보이게 합니다.
	if (!InKnownPrefix.IsEmpty() && FormattedSummaryText.StartsWith(InKnownPrefix))
	{
		FormattedSummaryText.RightChopInline(KnownPrefixLength, EAllowShrinking::No);
	}

	// [v1.1.0] 파이프 구분자를 패널용 줄바꿈으로 바꿉니다.
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT(" | "), TEXT("\n  - "));

	// [v1.1.0] 쉼표 구분자를 패널용 줄바꿈으로 바꿉니다.
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT(", "), TEXT("\n  - "));

	// [v1.1.0] 첫 줄 앞에도 동일한 들여쓰기 스타일을 맞춥니다.
	FormattedSummaryText = FString::Printf(TEXT("  - %s"), *FormattedSummaryText);

	return FormattedSummaryText;
}

void UCFVehicleDebugPanelWidget::ResolveWidgetReferences()
{
	// [v1.2.3] WidgetTree가 없으면 참조 보강을 수행할 수 없습니다.
	if (!WidgetTree)
	{
		return;
	}

	// [v1.2.3] Overview 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_OverviewBody)
	{
		Container_OverviewBody = WidgetTree->FindWidget(TEXT("Container_OverviewBody"));
	}

	// [v1.6.0] 동적 Section 호스트 참조를 이름 기준으로 보강합니다.
	if (!VerticalBox_DynamicSectionHost)
	{
		VerticalBox_DynamicSectionHost = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("VerticalBox_DynamicSectionHost")));
	}

	// [v1.7.0] Navigation 항목 호스트 참조를 이름 기준으로 보강합니다.
	if (!VerticalBox_NavHost)
	{
		VerticalBox_NavHost = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("VerticalBox_NavHost")));
	}

	// [v1.7.0] 선택 Section 호스트 참조를 이름 기준으로 보강합니다.
	if (!VerticalBox_SelectedSectionHost)
	{
		VerticalBox_SelectedSectionHost = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("VerticalBox_SelectedSectionHost")));
	}

	// [v1.3.2] 실제 Panel 본체 루트 위젯 참조를 이름 기준으로 보강합니다.
	if (!Border_RootPanel)
	{
		Border_RootPanel = WidgetTree->FindWidget(TEXT("Border_RootPanel"));
	}

	// [v1.2.3] Overview 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_OverviewTitle)
	{
		Text_OverviewTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_OverviewTitle")));
	}

	// [v1.2.3] Overview 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_OverviewBody)
	{
		Text_OverviewBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_OverviewBody")));
	}

	// [v1.4.0] Overview Last Transition 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_OverviewLastTransitionBody)
	{
		Container_OverviewLastTransitionBody = WidgetTree->FindWidget(TEXT("Container_OverviewLastTransitionBody"));
	}

	// [v1.4.0] Overview Last Transition 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_OverviewLastTransitionTitle)
	{
		Text_OverviewLastTransitionTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_OverviewLastTransitionTitle")));
	}

	// [v1.4.0] Overview Last Transition 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_OverviewLastTransitionBody)
	{
		Text_OverviewLastTransitionBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_OverviewLastTransitionBody")));
	}

	// [v1.2.3] Drive 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_DriveBody)
	{
		Container_DriveBody = WidgetTree->FindWidget(TEXT("Container_DriveBody"));
	}

	// [v1.2.3] Drive 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_DriveTitle)
	{
		Text_DriveTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_DriveTitle")));
	}

	// [v1.2.3] Drive 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_DriveBody)
	{
		Text_DriveBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_DriveBody")));
	}

	// [v1.4.0] Drive Transition 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_DriveTransitionBody)
	{
		Container_DriveTransitionBody = WidgetTree->FindWidget(TEXT("Container_DriveTransitionBody"));
	}

	// [v1.4.0] Drive Transition 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_DriveTransitionTitle)
	{
		Text_DriveTransitionTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_DriveTransitionTitle")));
	}

	// [v1.4.0] Drive Transition 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_DriveTransitionBody)
	{
		Text_DriveTransitionBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_DriveTransitionBody")));
	}

	// [v1.2.3] Input 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_InputBody)
	{
		Container_InputBody = WidgetTree->FindWidget(TEXT("Container_InputBody"));
	}

	// [v1.2.3] Input 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_InputTitle)
	{
		Text_InputTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_InputTitle")));
	}

	// [v1.2.3] Input 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_InputBody)
	{
		Text_InputBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_InputBody")));
	}

	// [v1.2.3] Runtime 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_RuntimeBody)
	{
		Container_RuntimeBody = WidgetTree->FindWidget(TEXT("Container_RuntimeBody"));
	}

	// [v1.2.3] Runtime 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeTitle)
	{
		Text_RuntimeTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeTitle")));
	}

	// [v1.2.3] Runtime 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeBody)
	{
		Text_RuntimeBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeBody")));
	}

	// [v1.3.0] Runtime Summary 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_RuntimeSummaryBody)
	{
		Container_RuntimeSummaryBody = WidgetTree->FindWidget(TEXT("Container_RuntimeSummaryBody"));
	}

	// [v1.3.0] Runtime Summary 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeSummaryTitle)
	{
		Text_RuntimeSummaryTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeSummaryTitle")));
	}

	// [v1.3.0] Runtime Summary 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeSummaryBody)
	{
		Text_RuntimeSummaryBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeSummaryBody")));
	}

	// [v1.3.0] Last Init 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_RuntimeLastInitBody)
	{
		Container_RuntimeLastInitBody = WidgetTree->FindWidget(TEXT("Container_RuntimeLastInitBody"));
	}

	// [v1.3.0] Last Init 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeLastInitTitle)
	{
		Text_RuntimeLastInitTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeLastInitTitle")));
	}

	// [v1.3.0] Last Init 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeLastInitBody)
	{
		Text_RuntimeLastInitBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeLastInitBody")));
	}

	// [v1.3.0] Last Validation 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_RuntimeLastValidationBody)
	{
		Container_RuntimeLastValidationBody = WidgetTree->FindWidget(TEXT("Container_RuntimeLastValidationBody"));
	}

	// [v1.3.0] Last Validation 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeLastValidationTitle)
	{
		Text_RuntimeLastValidationTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeLastValidationTitle")));
	}

	// [v1.3.0] Last Validation 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeLastValidationBody)
	{
		Text_RuntimeLastValidationBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeLastValidationBody")));
	}
}

void UCFVehicleDebugPanelWidget::UpdateCategoryBodyVisibility()
{
	// [v1.3.5] 현재 WBP 구조에서는 본문 텍스트가 컨테이너 안에 있으므로 컨테이너 가시성만 제어합니다.
	if (Container_OverviewBody)
	{
		Container_OverviewBody->SetVisibility(bIsOverviewExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.3.5] Drive 본문 컨테이너가 있으면 펼침 상태를 반영합니다.
	if (Container_DriveBody)
	{
		Container_DriveBody->SetVisibility(bIsDriveExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.4.0] Overview Last Transition 하위 본문 컨테이너는 Overview와 자신 둘 다 펼쳐졌을 때만 보입니다.
	if (Container_OverviewLastTransitionBody)
	{
		Container_OverviewLastTransitionBody->SetVisibility((bIsOverviewExpanded && bIsOverviewLastTransitionExpanded) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.4.0] Drive Transition 하위 본문 컨테이너는 Drive와 자신 둘 다 펼쳐졌을 때만 보입니다.
	if (Container_DriveTransitionBody)
	{
		Container_DriveTransitionBody->SetVisibility((bIsDriveExpanded && bIsDriveTransitionExpanded) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.3.5] Input 본문 컨테이너가 있으면 펼침 상태를 반영합니다.
	if (Container_InputBody)
	{
		Container_InputBody->SetVisibility(bIsInputExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.3.5] Runtime 본문 컨테이너가 있으면 펼침 상태를 반영합니다.
	if (Container_RuntimeBody)
	{
		Container_RuntimeBody->SetVisibility(bIsRuntimeExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.3.5] Runtime Summary 하위 본문 컨테이너는 Runtime과 자신 둘 다 펼쳐졌을 때만 보입니다.
	if (Container_RuntimeSummaryBody)
	{
		Container_RuntimeSummaryBody->SetVisibility((bIsRuntimeExpanded && bIsRuntimeSummaryExpanded) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.3.5] Last Init 하위 본문 컨테이너는 Runtime과 자신 둘 다 펼쳐졌을 때만 보입니다.
	if (Container_RuntimeLastInitBody)
	{
		Container_RuntimeLastInitBody->SetVisibility((bIsRuntimeExpanded && bIsRuntimeLastInitExpanded) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.3.5] Last Validation 하위 본문 컨테이너는 Runtime과 자신 둘 다 펼쳐졌을 때만 보입니다.
	if (Container_RuntimeLastValidationBody)
	{
		Container_RuntimeLastValidationBody->SetVisibility((bIsRuntimeExpanded && bIsRuntimeLastValidationExpanded) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UCFVehicleDebugPanelWidget::UpdateCategoryHeaderTexts()
{
	// [v1.2.4] Overview 펼침 상태를 제목 문자열에 반영합니다.
	const FString OverviewHeaderText = FString::Printf(TEXT("%s Overview"), bIsOverviewExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.2.4] Drive 펼침 상태를 제목 문자열에 반영합니다.
	const FString DriveHeaderText = FString::Printf(TEXT("%s Drive"), bIsDriveExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.4.0] Overview Last Transition 펼침 상태를 제목 문자열에 반영합니다.
	const FString OverviewLastTransitionHeaderText = FString::Printf(TEXT("%s Last Transition"), bIsOverviewLastTransitionExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.4.0] Drive Transition 펼침 상태를 제목 문자열에 반영합니다.
	const FString DriveTransitionHeaderText = FString::Printf(TEXT("%s Transition"), bIsDriveTransitionExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.2.4] Input 펼침 상태를 제목 문자열에 반영합니다.
	const FString InputHeaderText = FString::Printf(TEXT("%s Input"), bIsInputExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.2.4] Runtime 펼침 상태를 제목 문자열에 반영합니다.
	const FString RuntimeHeaderText = FString::Printf(TEXT("%s Runtime"), bIsRuntimeExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.3.0] Runtime Summary 펼침 상태를 제목 문자열에 반영합니다.
	const FString RuntimeSummaryHeaderText = FString::Printf(TEXT("%s Runtime Summary"), bIsRuntimeSummaryExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.3.0] Last Init 펼침 상태를 제목 문자열에 반영합니다.
	const FString RuntimeLastInitHeaderText = FString::Printf(TEXT("%s Last Init"), bIsRuntimeLastInitExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.3.0] Last Validation 펼침 상태를 제목 문자열에 반영합니다.
	const FString RuntimeLastValidationHeaderText = FString::Printf(TEXT("%s Last Validation"), bIsRuntimeLastValidationExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.2.4] Overview 제목 텍스트를 갱신합니다.
	if (Text_OverviewTitle)
	{
		Text_OverviewTitle->SetText(FText::FromString(OverviewHeaderText));
	}

	// [v1.4.0] Overview Last Transition 제목 텍스트를 갱신합니다.
	if (Text_OverviewLastTransitionTitle)
	{
		Text_OverviewLastTransitionTitle->SetText(FText::FromString(OverviewLastTransitionHeaderText));
	}

	// [v1.2.4] Drive 제목 텍스트를 갱신합니다.
	if (Text_DriveTitle)
	{
		Text_DriveTitle->SetText(FText::FromString(DriveHeaderText));
	}

	// [v1.4.0] Drive Transition 제목 텍스트를 갱신합니다.
	if (Text_DriveTransitionTitle)
	{
		Text_DriveTransitionTitle->SetText(FText::FromString(DriveTransitionHeaderText));
	}

	// [v1.2.4] Input 제목 텍스트를 갱신합니다.
	if (Text_InputTitle)
	{
		Text_InputTitle->SetText(FText::FromString(InputHeaderText));
	}

	// [v1.2.4] Runtime 제목 텍스트를 갱신합니다.
	if (Text_RuntimeTitle)
	{
		Text_RuntimeTitle->SetText(FText::FromString(RuntimeHeaderText));
	}

	// [v1.3.0] Runtime Summary 제목 텍스트를 갱신합니다.
	if (Text_RuntimeSummaryTitle)
	{
		Text_RuntimeSummaryTitle->SetText(FText::FromString(RuntimeSummaryHeaderText));
	}

	// [v1.3.0] Last Init 제목 텍스트를 갱신합니다.
	if (Text_RuntimeLastInitTitle)
	{
		Text_RuntimeLastInitTitle->SetText(FText::FromString(RuntimeLastInitHeaderText));
	}

	// [v1.3.0] Last Validation 제목 텍스트를 갱신합니다.
	if (Text_RuntimeLastValidationTitle)
	{
		Text_RuntimeLastValidationTitle->SetText(FText::FromString(RuntimeLastValidationHeaderText));
	}
}

APlayerController* UCFVehicleDebugPanelWidget::ResolveOwningPlayerController() const
{
	// [v1.2.0] 위젯 소유 플레이어를 우선 사용합니다.
	APlayerController* PlayerController = GetOwningPlayer();

	if (PlayerController)
	{
		return PlayerController;
	}

	// [v1.2.0] 위젯 소유 플레이어가 없으면 Pawn의 컨트롤러를 fallback으로 사용합니다.
	return VehiclePawnRef ? Cast<APlayerController>(VehiclePawnRef->GetController()) : nullptr;
}

bool UCFVehicleDebugPanelWidget::IsDynamicSectionLayoutReady() const
{
	return VerticalBox_DynamicSectionHost != nullptr && DynamicSectionWidgetClass != nullptr;
}

bool UCFVehicleDebugPanelWidget::IsNavigationLayoutReady() const
{
	return VerticalBox_NavHost != nullptr && NavItemWidgetClass != nullptr;
}

bool UCFVehicleDebugPanelWidget::IsSelectedSectionLayoutReady() const
{
	return VerticalBox_SelectedSectionHost != nullptr && DynamicSectionWidgetClass != nullptr;
}

void UCFVehicleDebugPanelWidget::EnsureDynamicSectionWidgets()
{
	// [v1.6.0] 동적 Section 레이아웃 준비가 안 되었으면 생성을 중단합니다.
	if (!IsDynamicSectionLayoutReady())
	{
		return;
	}

	// [v1.6.0] 현재 필요한 최상위 섹션 개수입니다.
	const int32 RequiredSectionCount = CachedPanelViewData.TopLevelSectionArray.Num();

	if (DynamicSectionWidgetArray.Num() == RequiredSectionCount)
	{
		return;
	}

	VerticalBox_DynamicSectionHost->ClearChildren();
	DynamicSectionWidgetArray.Reset();

	for (int32 SectionIndex = 0; SectionIndex < RequiredSectionCount; ++SectionIndex)
	{
		// [v1.6.0] 새로 생성할 동적 Section 위젯 인스턴스입니다.
		UCFVehicleDebugSectionWidget* DynamicSectionWidget = nullptr;

		if (APlayerController* OwningPlayerController = GetOwningPlayer())
		{
			DynamicSectionWidget = CreateWidget<UCFVehicleDebugSectionWidget>(OwningPlayerController, DynamicSectionWidgetClass);
		}
		else if (WidgetTree)
		{
			DynamicSectionWidget = CreateWidget<UCFVehicleDebugSectionWidget>(WidgetTree, DynamicSectionWidgetClass);
		}

		if (!DynamicSectionWidget)
		{
			continue;
		}

		DynamicSectionWidgetArray.Add(DynamicSectionWidget);
		VerticalBox_DynamicSectionHost->AddChildToVerticalBox(DynamicSectionWidget);
	}
}

void UCFVehicleDebugPanelWidget::RefreshDynamicSectionWidgets()
{
	// [v1.6.0] 동적 Section 레이아웃 준비가 안 되었으면 갱신을 중단합니다.
	if (!IsDynamicSectionLayoutReady())
	{
		return;
	}

	if (VerticalBox_DynamicSectionHost)
	{
		VerticalBox_DynamicSectionHost->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (VerticalBox_SelectedSectionHost)
	{
		VerticalBox_SelectedSectionHost->SetVisibility(ESlateVisibility::Collapsed);
	}

	EnsureDynamicSectionWidgets();

	for (int32 SectionIndex = 0; SectionIndex < DynamicSectionWidgetArray.Num(); ++SectionIndex)
	{
		// [v1.6.0] 현재 갱신할 동적 Section 위젯 인스턴스입니다.
		UCFVehicleDebugSectionWidget* DynamicSectionWidget = DynamicSectionWidgetArray[SectionIndex];

		// [v1.6.0] 현재 위젯에 대응하는 최상위 섹션 ViewData입니다.
		const TSharedPtr<FCFVehicleDebugSectionViewData> TopLevelSectionViewData =
			CachedPanelViewData.TopLevelSectionArray.IsValidIndex(SectionIndex)
			? CachedPanelViewData.TopLevelSectionArray[SectionIndex]
			: nullptr;

		if (!DynamicSectionWidget || !TopLevelSectionViewData.IsValid())
		{
			continue;
		}

		DynamicSectionWidget->SetSectionViewData(*TopLevelSectionViewData);
	}
}

TArray<FCFVehicleDebugNavItemViewData> UCFVehicleDebugPanelWidget::BuildNavigationItemViewDataArray() const
{
	// [v1.7.0] 생성할 Navigation Item ViewData 배열입니다.
	TArray<FCFVehicleDebugNavItemViewData> NavItemViewDataArray;

	for (const TSharedPtr<FCFVehicleDebugSectionViewData>& TopLevelSectionViewData : CachedPanelViewData.TopLevelSectionArray)
	{
		if (!TopLevelSectionViewData.IsValid() || !TopLevelSectionViewData->bIsVisible || !TopLevelSectionViewData->bShowInNavigation)
		{
			continue;
		}

		// [v1.7.0] 현재 최상위 Section에서 만든 Navigation 항목입니다.
		FCFVehicleDebugNavItemViewData NavItemViewData;
		NavItemViewData.SectionId = TopLevelSectionViewData->SectionId;
		NavItemViewData.DisplayText = TopLevelSectionViewData->TitleText;
		NavItemViewData.NavigationGroup = TopLevelSectionViewData->NavigationGroup;
		NavItemViewData.NavigationOrder = TopLevelSectionViewData->NavigationOrder;
		NavItemViewData.BadgeText = TopLevelSectionViewData->BadgeText;
		NavItemViewData.bIsVisible = true;
		NavItemViewData.bIsSelected = TopLevelSectionViewData->SectionId == SelectedSectionId;
		NavItemViewDataArray.Add(NavItemViewData);
	}

	NavItemViewDataArray.Sort([](const FCFVehicleDebugNavItemViewData& Left, const FCFVehicleDebugNavItemViewData& Right)
	{
		if (Left.NavigationGroup != Right.NavigationGroup)
		{
			return static_cast<uint8>(Left.NavigationGroup) < static_cast<uint8>(Right.NavigationGroup);
		}

		if (Left.NavigationOrder != Right.NavigationOrder)
		{
			return Left.NavigationOrder < Right.NavigationOrder;
		}

		return Left.DisplayText < Right.DisplayText;
	});

	return NavItemViewDataArray;
}

void UCFVehicleDebugPanelWidget::ResolveSelectedSectionId()
{
	if (CachedPanelViewData.TopLevelSectionArray.IsEmpty())
	{
		SelectedSectionId.Reset();
		return;
	}

	for (const TSharedPtr<FCFVehicleDebugSectionViewData>& TopLevelSectionViewData : CachedPanelViewData.TopLevelSectionArray)
	{
		if (TopLevelSectionViewData.IsValid() &&
			TopLevelSectionViewData->bIsVisible &&
			TopLevelSectionViewData->SectionId == SelectedSectionId)
		{
			return;
		}
	}

	for (const TSharedPtr<FCFVehicleDebugSectionViewData>& TopLevelSectionViewData : CachedPanelViewData.TopLevelSectionArray)
	{
		if (TopLevelSectionViewData.IsValid() && TopLevelSectionViewData->bIsVisible)
		{
			SelectedSectionId = TopLevelSectionViewData->SectionId;
			return;
		}
	}

	SelectedSectionId.Reset();
}

TSharedPtr<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::FindSelectedSectionViewData() const
{
	for (const TSharedPtr<FCFVehicleDebugSectionViewData>& TopLevelSectionViewData : CachedPanelViewData.TopLevelSectionArray)
	{
		if (TopLevelSectionViewData.IsValid() &&
			TopLevelSectionViewData->bIsVisible &&
			TopLevelSectionViewData->SectionId == SelectedSectionId)
		{
			return TopLevelSectionViewData;
		}
	}

	return nullptr;
}

void UCFVehicleDebugPanelWidget::EnsureNavigationItemWidgets(const TArray<FCFVehicleDebugNavItemViewData>& InNavigationItemViewDataArray)
{
	// [v1.7.0] Navigation 레이아웃 준비가 안 되었으면 생성을 중단합니다.
	if (!IsNavigationLayoutReady())
	{
		return;
	}

	// [v1.7.0] 기존 Navigation Item 캐시에 비어 있는 항목이 있는지 확인합니다.
	bool bHasInvalidCachedNavItem = false;
	for (const TObjectPtr<UCFVehicleDebugNavItemWidget>& NavigationItemWidget : NavigationItemWidgetArray)
	{
		if (!NavigationItemWidget)
		{
			bHasInvalidCachedNavItem = true;
			break;
		}
	}

	// [v1.7.0] 개수가 같고 캐시가 유효하면 기존 위젯을 재사용합니다.
	if (NavigationItemWidgetArray.Num() == InNavigationItemViewDataArray.Num() && !bHasInvalidCachedNavItem)
	{
		return;
	}

	VerticalBox_NavHost->ClearChildren();
	NavigationItemWidgetArray.Reset();

	for (int32 NavItemIndex = 0; NavItemIndex < InNavigationItemViewDataArray.Num(); ++NavItemIndex)
	{
		// [v1.7.0] 새로 생성할 Navigation Item 위젯 인스턴스입니다.
		UCFVehicleDebugNavItemWidget* NavigationItemWidget = nullptr;

		if (APlayerController* OwningPlayerController = GetOwningPlayer())
		{
			NavigationItemWidget = CreateWidget<UCFVehicleDebugNavItemWidget>(OwningPlayerController, NavItemWidgetClass);
		}
		else if (WidgetTree)
		{
			NavigationItemWidget = CreateWidget<UCFVehicleDebugNavItemWidget>(WidgetTree, NavItemWidgetClass);
		}

		if (!NavigationItemWidget)
		{
			continue;
		}

		NavigationItemWidget->SetOwningPanelWidget(this);
		NavigationItemWidgetArray.Add(NavigationItemWidget);
		VerticalBox_NavHost->AddChildToVerticalBox(NavigationItemWidget);
	}
}

void UCFVehicleDebugPanelWidget::RefreshNavigationItems()
{
	if (!IsNavigationLayoutReady())
	{
		return;
	}

	// [v1.7.0] 현재 Panel ViewData 기준으로 생성한 Navigation 항목 배열입니다.
	const TArray<FCFVehicleDebugNavItemViewData> NavItemViewDataArray = BuildNavigationItemViewDataArray();

	EnsureNavigationItemWidgets(NavItemViewDataArray);

	for (int32 NavItemIndex = 0; NavItemIndex < NavigationItemWidgetArray.Num(); ++NavItemIndex)
	{
		// [v1.7.0] 현재 갱신할 Navigation Item 위젯입니다.
		UCFVehicleDebugNavItemWidget* NavigationItemWidget = NavigationItemWidgetArray[NavItemIndex];

		if (!NavigationItemWidget || !NavItemViewDataArray.IsValidIndex(NavItemIndex))
		{
			continue;
		}

		NavigationItemWidget->SetOwningPanelWidget(this);
		NavigationItemWidget->SetNavItemViewData(NavItemViewDataArray[NavItemIndex]);
	}
}

void UCFVehicleDebugPanelWidget::RefreshSelectedSectionWidget()
{
	// [v1.7.0] 선택 Section 레이아웃 준비가 안 되었으면 갱신을 중단합니다.
	if (!IsSelectedSectionLayoutReady())
	{
		return;
	}

	if (VerticalBox_SelectedSectionHost)
	{
		VerticalBox_SelectedSectionHost->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (VerticalBox_DynamicSectionHost)
	{
		VerticalBox_DynamicSectionHost->SetVisibility(ESlateVisibility::Collapsed);
	}

	// [v1.7.0] 현재 선택된 Section ViewData입니다.
	const TSharedPtr<FCFVehicleDebugSectionViewData> SelectedSectionViewData = FindSelectedSectionViewData();

	if (!SelectedSectionViewData.IsValid())
	{
		VerticalBox_SelectedSectionHost->ClearChildren();
		SelectedSectionWidget = nullptr;
		return;
	}

	if (!SelectedSectionWidget)
	{
		if (APlayerController* OwningPlayerController = GetOwningPlayer())
		{
			SelectedSectionWidget = CreateWidget<UCFVehicleDebugSectionWidget>(OwningPlayerController, DynamicSectionWidgetClass);
		}
		else if (WidgetTree)
		{
			SelectedSectionWidget = CreateWidget<UCFVehicleDebugSectionWidget>(WidgetTree, DynamicSectionWidgetClass);
		}

		if (SelectedSectionWidget)
		{
			VerticalBox_SelectedSectionHost->ClearChildren();
			VerticalBox_SelectedSectionHost->AddChildToVerticalBox(SelectedSectionWidget);
		}
	}

	if (SelectedSectionWidget)
	{
		SelectedSectionWidget->SetSectionViewData(*SelectedSectionViewData);
	}
}

bool UCFVehicleDebugPanelWidget::ShouldUseLegacyFullSectionRendering() const
{
	return !IsSelectedSectionLayoutReady() && IsDynamicSectionLayoutReady();
}

// [v1.5.0] 현재 Snapshot 기준 최신 Panel ViewData를 생성합니다.
FCFVehicleDebugPanelViewData UCFVehicleDebugPanelWidget::BuildVehicleDebugPanelViewData() const
{
	// [v1.5.0] 생성할 Panel ViewData 결과입니다.
	FCFVehicleDebugPanelViewData PanelViewData;
	PanelViewData.PanelId = TEXT("VehicleDebugPanel");
	PanelViewData.PanelTitleText = TEXT("Vehicle Debug Panel");
	PanelViewData.GeneratedFrameNumber = GFrameCounter;
	PanelViewData.GeneratedTimeSeconds = FPlatformTime::Seconds();
	PanelViewData.AddTopLevelSection(BuildOverviewSectionViewData(CachedOverview));
	PanelViewData.AddTopLevelSection(BuildDriveSectionViewData(CachedDrive));
	PanelViewData.AddTopLevelSection(BuildInputSectionViewData(CachedInput));
	PanelViewData.AddTopLevelSection(BuildRuntimeSectionViewData(CachedRuntime));
	return PanelViewData;
}

// [v1.5.0] Overview 카테고리용 Section ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::BuildOverviewSectionViewData(const FCFVehicleDebugOverview& InOverview) const
{
	// [v1.5.0] 생성할 Overview 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> OverviewSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("Overview"), TEXT("Overview"), ECFVehicleDebugSectionKind::Category, bIsOverviewExpanded);
	OverviewSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Core;
	OverviewSectionViewData->NavigationOrder = 10;
	OverviewSectionViewData->bShowInNavigation = true;

	OverviewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("overview_runtime_ready"), TEXT("Ready"), InOverview.bRuntimeReady ? TEXT("True") : TEXT("False"), true));
	OverviewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("overview_state"), TEXT("State"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.CurrentDriveState))));
	OverviewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("overview_speed"), TEXT("Speed"), FString::Printf(TEXT("%.1f km/h"), InOverview.SpeedKmh)));
	OverviewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("overview_forward_speed"), TEXT("Forward"), FString::Printf(TEXT("%.1f km/h"), InOverview.ForwardSpeedKmh)));
	OverviewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("overview_device_mode"), TEXT("Input Mode"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.DeviceMode))));
	OverviewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("overview_input_owner"), TEXT("Input Owner"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.InputOwner))));

	// [v1.5.0] Overview Last Transition 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> OverviewLastTransitionSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("OverviewLastTransition"), TEXT("Last Transition"), ECFVehicleDebugSectionKind::Subsection, bIsOverviewLastTransitionExpanded);
	OverviewLastTransitionSectionViewData->BodyText = BuildOverviewLastTransitionBodyText(InOverview);
	OverviewSectionViewData->AddChildSection(OverviewLastTransitionSectionViewData);

	return OverviewSectionViewData;
}

// [v1.5.0] Drive 카테고리용 Section ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::BuildDriveSectionViewData(const FCFVehicleDebugDrive& InDrive) const
{
	// [v1.5.0] 생성할 Drive 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> DriveSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("Drive"), TEXT("Drive"), ECFVehicleDebugSectionKind::Category, bIsDriveExpanded);
	DriveSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Core;
	DriveSectionViewData->NavigationOrder = 20;
	DriveSectionViewData->bShowInNavigation = true;

	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_current_state"), TEXT("Current State"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InDrive.CurrentDriveState))));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_previous_state"), TEXT("Previous State"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InDrive.PreviousDriveState))));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_changed_this_frame"), TEXT("Changed This Frame"), InDrive.bDriveStateChangedThisFrame ? TEXT("True") : TEXT("False")));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_speed"), TEXT("Speed"), FString::Printf(TEXT("%.1f km/h"), InDrive.SpeedKmh)));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_forward_speed"), TEXT("Forward"), FString::Printf(TEXT("%.1f km/h"), InDrive.ForwardSpeedKmh)));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_throttle"), TEXT("Throttle"), FString::Printf(TEXT("%.2f"), InDrive.Throttle)));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_brake"), TEXT("Brake"), FString::Printf(TEXT("%.2f"), InDrive.Brake)));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_steering"), TEXT("Steering"), FString::Printf(TEXT("%.2f"), InDrive.Steering)));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_handbrake"), TEXT("Handbrake"), InDrive.bHandbrake ? TEXT("On") : TEXT("Off")));

	// [v1.5.0] Drive Transition 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> DriveTransitionSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("DriveTransition"), TEXT("Transition"), ECFVehicleDebugSectionKind::Subsection, bIsDriveTransitionExpanded);
	DriveTransitionSectionViewData->BodyText = BuildDriveTransitionBodyText(InDrive);
	DriveSectionViewData->AddChildSection(DriveTransitionSectionViewData);

	return DriveSectionViewData;
}

// [v1.5.0] Input 카테고리용 Section ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::BuildInputSectionViewData(const FCFVehicleDebugInput& InInput) const
{
	// [v1.5.0] 생성할 Input 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> InputSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("Input"), TEXT("Input"), ECFVehicleDebugSectionKind::Category, bIsInputExpanded);
	InputSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Core;
	InputSectionViewData->NavigationOrder = 30;
	InputSectionViewData->bShowInNavigation = true;

	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_device_mode"), TEXT("Device Mode"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.DeviceMode))));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_input_owner"), TEXT("Input Owner"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.InputOwner))));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_move_zone"), TEXT("Move Zone"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.MoveZone))));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_move_intent"), TEXT("Move Intent"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.MoveIntent))));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_move_raw"), TEXT("Move Raw"), FString::Printf(TEXT("(%.2f, %.2f)"), InInput.MoveRaw.X, InInput.MoveRaw.Y)));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_move_magnitude"), TEXT("Move Magnitude"), FString::Printf(TEXT("%.2f"), InInput.MoveMagnitude)));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_move_angle"), TEXT("Move Angle"), FString::Printf(TEXT("%.1f"), InInput.MoveAngle)));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_black_hold"), TEXT("Black Hold"), InInput.bUsedBlackZoneHold ? TEXT("True") : TEXT("False")));

	return InputSectionViewData;
}

// [v1.5.0] Runtime 카테고리용 Section ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::BuildRuntimeSectionViewData(const FCFVehicleDebugRuntime& InRuntime) const
{
	// [v1.5.0] 생성할 Runtime 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> RuntimeSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("Runtime"), TEXT("Runtime"), ECFVehicleDebugSectionKind::Category, bIsRuntimeExpanded);
	RuntimeSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Core;
	RuntimeSectionViewData->NavigationOrder = 40;
	RuntimeSectionViewData->bShowInNavigation = true;

	RuntimeSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("runtime_ready"), TEXT("Ready"), InRuntime.bRuntimeReady ? TEXT("True") : TEXT("False"), true));
	RuntimeSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("runtime_has_drive_comp"), TEXT("Has DriveComp"), InRuntime.bHasDriveComponent ? TEXT("True") : TEXT("False")));
	RuntimeSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("runtime_has_wheelsync_comp"), TEXT("Has WheelSyncComp"), InRuntime.bHasWheelSyncComponent ? TEXT("True") : TEXT("False")));

	// [v1.5.0] Runtime Summary 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> RuntimeSummarySectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("RuntimeSummary"), TEXT("Runtime Summary"), ECFVehicleDebugSectionKind::Subsection, bIsRuntimeSummaryExpanded);
	RuntimeSummarySectionViewData->BodyText = BuildRuntimeSummaryBodyText(InRuntime);
	RuntimeSectionViewData->AddChildSection(RuntimeSummarySectionViewData);

	// [v1.5.0] Last Init 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> RuntimeLastInitSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("RuntimeLastInit"), TEXT("Last Init"), ECFVehicleDebugSectionKind::Subsection, bIsRuntimeLastInitExpanded);
	RuntimeLastInitSectionViewData->BodyText = BuildRuntimeLastInitBodyText(InRuntime);
	RuntimeSectionViewData->AddChildSection(RuntimeLastInitSectionViewData);

	// [v1.5.0] Last Validation 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> RuntimeLastValidationSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("RuntimeLastValidation"), TEXT("Last Validation"), ECFVehicleDebugSectionKind::Subsection, bIsRuntimeLastValidationExpanded);
	RuntimeLastValidationSectionViewData->BodyText = BuildRuntimeLastValidationBodyText(InRuntime);
	RuntimeSectionViewData->AddChildSection(RuntimeLastValidationSectionViewData);

	return RuntimeSectionViewData;
}
