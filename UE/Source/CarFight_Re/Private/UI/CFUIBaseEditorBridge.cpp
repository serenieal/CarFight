// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-10
// Description: D1-10B Base Widget Blueprint Designer Tree Editor Bridge 구현
// Scope: 정확한 5종 WidgetTree 생성과 Native Parent·Button Bind·구조·Graph 안전성 검증만 담당합니다.
// Changelog:
// - v1.2.0: UE 5.8 Python용 bool-only Build/Validate 래퍼를 추가해 FString Out Parameter 마샬링과 결과 판정을 분리.
// - v1.1.3: Build 진입 선행 조건에서 WidgetBlueprint Cast 실패와 WidgetTree Null을 분리해 Editor Log에 기록.
// - v1.1.2: Python Out Parameter 반환형과 무관하게 Builder 실패 사유를 Editor Log에 직접 남기는 진단 추가.
// - v1.1.1: Button Tree 부착 단계를 개별 검증해 실패 관계를 OutFailureReason으로 정확히 노출.
// - v1.1.0: 5종 Blueprint의 정확한 Native Parent와 Button_Interaction BindWidget 메타데이터 검증을 추가.
// - v1.0.1: CodeWorkGate에 맞춰 Tree Builder·Validator의 지역 변수 역할 주석을 보강.
// - v1.0.0: Button/Panel/StatusBar/InfoRow/AlertItem Tree Builder와 검증기를 최초 구현.
// Migration:
// - 런타임 UI 코드 경로에서 사용하지 않습니다.
// - Event Graph Node를 생성하지 않으며 Gameplay Cast와 DataAsset 직접 Load를 추가하지 않습니다.

#include "UI/CFUIBaseEditorBridge.h"

#include "UI/CFButtonBaseWidget.h"
#include "UI/CFStyledWidgetBase.h"

#if WITH_EDITOR
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/NamedSlot.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "WidgetBlueprint.h"
#include "UObject/UnrealType.h"
#endif

namespace CFUIBaseEditorBridge
{
#if WITH_EDITOR
	// [v1.0.0] WidgetTree에서 지정 이름·클래스 Widget을 생성하고 실패 시 Null을 반환합니다.
	template <typename TWidget>
	TWidget* CreateWidget(UWidgetTree* WidgetTree, const TCHAR* WidgetName)
	{
		return WidgetTree
			? WidgetTree->ConstructWidget<TWidget>(TWidget::StaticClass(), FName(WidgetName))
			: nullptr;
	}

	// [v1.0.0] Panel에 Child를 추가하고 입력이 유효한지 반환합니다.
	bool AddChild(UPanelWidget* Parent, UWidget* Child)
	{
		return Parent && Child && Parent->AddChild(Child) != nullptr;
	}

	// [v1.0.0] ContentWidget에 단일 Child를 설정하고 입력이 유효한지 반환합니다.
	bool SetContent(UContentWidget* Parent, UWidget* Child)
	{
		if (!Parent || !Child)
		{
			return false;
		}
		Parent->SetContent(Child);
		return Parent->GetContent() == Child;
	}

	// [v1.0.0] 지정 Widget의 실제 이름과 클래스가 기대값과 일치하는지 검증합니다.
	bool ValidateWidgetIdentity(UWidget* Widget, const TCHAR* ExpectedName, UClass* ExpectedClass, FString& OutFailureReason)
	{
		if (!Widget)
		{
			OutFailureReason = FString::Printf(TEXT("Widget missing: %s"), ExpectedName);
			return false;
		}
		if (Widget->GetFName() != FName(ExpectedName))
		{
			OutFailureReason = FString::Printf(TEXT("Widget name mismatch: actual=%s expected=%s"), *Widget->GetName(), ExpectedName);
			return false;
		}
		if (!Widget->IsA(ExpectedClass))
		{
			OutFailureReason = FString::Printf(TEXT("Widget class mismatch: %s actual=%s expected=%s"), ExpectedName, *Widget->GetClass()->GetName(), *ExpectedClass->GetName());
			return false;
		}
		return true;
	}

	// [v1.0.0] Panel의 직접 Child 수와 순서별 이름·클래스를 검증합니다.
	bool ValidateChildren(UPanelWidget* Parent, const TArray<TPair<FName, UClass*>>& ExpectedChildren, FString& OutFailureReason)
	{
		if (!Parent)
		{
			OutFailureReason = TEXT("Parent panel is null");
			return false;
		}
		if (Parent->GetChildrenCount() != ExpectedChildren.Num())
		{
			OutFailureReason = FString::Printf(TEXT("Child count mismatch: parent=%s actual=%d expected=%d"), *Parent->GetName(), Parent->GetChildrenCount(), ExpectedChildren.Num());
			return false;
		}
				// [v1.0.1] 기대 Child 순서와 실제 Panel Child 순서를 함께 순회할 인덱스입니다.
		for (int32 ChildIndex = 0; ChildIndex < ExpectedChildren.Num(); ++ChildIndex)
		{
			// [v1.0.1] 현재 인덱스에서 실제 Panel이 소유한 Child Widget입니다.
			UWidget* Child = Parent->GetChildAt(ChildIndex);
			// [v1.0.1] 현재 인덱스에서 기대하는 Child 이름과 Class 계약입니다.
			const TPair<FName, UClass*>& ExpectedChild = ExpectedChildren[ChildIndex];
			if (!Child || Child->GetFName() != ExpectedChild.Key || !Child->IsA(ExpectedChild.Value))
			{
				OutFailureReason = FString::Printf(TEXT("Child mismatch: parent=%s index=%d actual=%s/%s expected=%s/%s"), *Parent->GetName(), ChildIndex, Child ? *Child->GetName() : TEXT("None"), Child ? *Child->GetClass()->GetName() : TEXT("None"), *ExpectedChild.Key.ToString(), *ExpectedChild.Value->GetName());
				return false;
			}
		}
		return true;
	}

	// [v1.0.0] Blueprint Graph 전체에서 Cast와 CallFunction Node가 하나도 없는지 검증합니다.
	bool ValidateGraphSafety(const UWidgetBlueprint* WidgetBlueprint, FString& OutFailureReason)
	{
		if (!WidgetBlueprint)
		{
			OutFailureReason = TEXT("WidgetBlueprint is null");
			return false;
		}

		// [v1.0.0] Event/Function/Macro Graph를 한 번에 검사하기 위한 Graph 목록입니다.
		TArray<UEdGraph*> Graphs;
		Graphs.Append(WidgetBlueprint->UbergraphPages);
		Graphs.Append(WidgetBlueprint->FunctionGraphs);
		Graphs.Append(WidgetBlueprint->MacroGraphs);

				// [v1.0.1] D1-10 Base Widget이 소유한 각 Blueprint Graph를 순회합니다.
		for (const UEdGraph* Graph : Graphs)
		{
			if (!Graph)
			{
				continue;
			}
			// [v1.0.1] 현재 Graph에 포함된 각 Node를 안전성 규칙으로 검사합니다.
			for (const UEdGraphNode* Node : Graph->Nodes)
			{
				if (!Node)
				{
					continue;
				}
				// [v1.0.0] Node 클래스명만으로 Gameplay Cast 또는 함수 호출이 존재하는지 판정합니다.
				const FString NodeClassName = Node->GetClass()->GetName();
				if (NodeClassName.Contains(TEXT("DynamicCast"), ESearchCase::IgnoreCase))
				{
					OutFailureReason = FString::Printf(TEXT("Gameplay Cast node is forbidden: graph=%s node=%s"), *Graph->GetName(), *NodeClassName);
					return false;
				}
				if (NodeClassName.Contains(TEXT("CallFunction"), ESearchCase::IgnoreCase))
				{
					OutFailureReason = FString::Printf(TEXT("CallFunction node is forbidden in D1-10 Base Widget: graph=%s node=%s"), *Graph->GetName(), *NodeClassName);
					return false;
				}
			}
		}
		return true;
	}

	// [v1.0.0] WidgetTree의 모든 Spacer가 Collapsed가 아닌지 검증합니다.
	bool ValidateSpacerVisibility(const UWidgetTree* WidgetTree, FString& OutFailureReason)
	{
		if (!WidgetTree)
		{
			OutFailureReason = TEXT("WidgetTree is null");
			return false;
		}
				// [v1.0.1] Spacer Visibility 검증을 위해 WidgetTree 전체 Widget을 수집한 목록입니다.
		TArray<UWidget*> AllWidgets;
		WidgetTree->GetAllWidgets(AllWidgets);
		// [v1.0.1] 전체 Widget 중 Spacer를 찾아 Collapsed 잔류 여부를 검사합니다.
		for (const UWidget* Widget : AllWidgets)
		{
			// [v1.0.1] 현재 Widget이 Spacer일 때만 유효한 읽기 전용 Spacer 포인터입니다.
			const USpacer* Spacer = Cast<USpacer>(Widget);
			if (Spacer && Spacer->GetVisibility() == ESlateVisibility::Collapsed)
			{
				OutFailureReason = FString::Printf(TEXT("Collapsed Spacer residue is forbidden: %s"), *Spacer->GetName());
				return false;
			}
		}
		return true;
	}

		// [v1.1.0] Template ID에 해당하는 정확한 Native Parent Class를 반환합니다.
	UClass* ResolveExpectedParentClass(FName TemplateId)
	{
		return TemplateId == FName(TEXT("Button"))
			? UCFButtonBaseWidget::StaticClass()
			: UCFStyledWidgetBase::StaticClass();
	}

	// [v1.1.0] Widget Blueprint의 실제 Parent Class가 Template 계약과 정확히 일치하는지 검증합니다.
	bool ValidateNativeParent(const UWidgetBlueprint* WidgetBlueprint, FName TemplateId, FString& OutFailureReason)
	{
		// [v1.1.0] 현재 Template이 요구하는 정확한 Native Parent Class입니다.
		UClass* ExpectedParentClass = ResolveExpectedParentClass(TemplateId);
		if (!WidgetBlueprint || WidgetBlueprint->ParentClass != ExpectedParentClass)
		{
			OutFailureReason = FString::Printf(
				TEXT("Native parent mismatch: actual=%s expected=%s"),
				WidgetBlueprint && WidgetBlueprint->ParentClass ? *WidgetBlueprint->ParentClass->GetName() : TEXT("None"),
				ExpectedParentClass ? *ExpectedParentClass->GetName() : TEXT("None"));
			return false;
		}
		return true;
	}

	// [v1.1.0] UCFButtonBaseWidget의 Button_Interaction BindWidget 메타데이터와 UButton 타입 계약을 검증합니다.
	bool ValidateButtonBindContract(FString& OutFailureReason)
	{
		// [v1.1.0] Native Button Base가 노출한 Button_Interaction Object Property입니다.
		const FObjectProperty* InteractionButtonProperty = FindFProperty<FObjectProperty>(UCFButtonBaseWidget::StaticClass(), TEXT("Button_Interaction"));
		if (!InteractionButtonProperty)
		{
			OutFailureReason = TEXT("Button_Interaction native property is missing");
			return false;
		}
		if (InteractionButtonProperty->PropertyClass != UButton::StaticClass())
		{
			OutFailureReason = TEXT("Button_Interaction native property is not UButton");
			return false;
		}
		if (!InteractionButtonProperty->HasMetaData(TEXT("BindWidget")))
		{
			OutFailureReason = TEXT("Button_Interaction native property is missing BindWidget metadata");
			return false;
		}
		return true;
	}

	// [v1.0.0] WBP_CFButtonBase 정확한 Designer Tree를 생성합니다.
	bool BuildButtonTree(UWidgetTree* Tree, FString& OutFailureReason)
	{
				// [v1.0.1] Button Base 전체 크기와 Minimum Size를 소유할 Root SizeBox입니다.
		USizeBox* Root = CreateWidget<USizeBox>(Tree, TEXT("SizeBox_Root"));
		// [v1.0.1] C++ BindWidget 계약이 연결될 실제 Interaction Button입니다.
		UButton* Interaction = CreateWidget<UButton>(Tree, TEXT("Button_Interaction"));
		// [v1.0.1] Background·Focus·Content·Marker를 겹쳐 표시할 Visual Overlay입니다.
		UOverlay* Visual = CreateWidget<UOverlay>(Tree, TEXT("Overlay_Visual"));
		// [v1.0.1] Button 기본 Surface를 표현할 Background Border입니다.
		UBorder* Background = CreateWidget<UBorder>(Tree, TEXT("Border_Background"));
		// [v1.0.1] Hover와 독립적인 Keyboard/Gamepad Focus Outline Border입니다.
		UBorder* FocusOutline = CreateWidget<UBorder>(Tree, TEXT("Border_FocusOutline"));
		// [v1.0.1] Icon·Label·Fill·InputHint를 수평 배치할 Content Box입니다.
		UHorizontalBox* Content = CreateWidget<UHorizontalBox>(Tree, TEXT("HorizontalBox_Content"));
		// [v1.0.1] Semantic Icon을 표시할 Image입니다.
		UImage* Icon = CreateWidget<UImage>(Tree, TEXT("Image_Icon"));
		// [v1.0.1] Button 본문 Label을 표시할 Text입니다.
		UTextBlock* Label = CreateWidget<UTextBlock>(Tree, TEXT("Text_Label"));
		// [v1.0.1] Label과 Input Hint 사이 남는 폭을 채울 Spacer입니다.
		USpacer* Fill = CreateWidget<USpacer>(Tree, TEXT("Spacer_Fill"));
		// [v1.0.1] Keyboard/Gamepad 입력 힌트를 표시할 Text입니다.
		UTextBlock* InputHint = CreateWidget<UTextBlock>(Tree, TEXT("Text_InputHint"));
		// [v1.0.1] Pressed·Accent 등 상태 마커를 표현할 Border입니다.
		UBorder* StateMarker = CreateWidget<UBorder>(Tree, TEXT("Border_StateMarker"));

		if (!Root || !Interaction || !Visual || !Background || !FocusOutline || !Content || !Icon || !Label || !Fill || !InputHint || !StateMarker)
		{
			OutFailureReason = TEXT("Button Tree widget construction failed");
			return false;
		}

				Tree->RootWidget = Root;
		if (!SetContent(Root, Interaction))
		{
			OutFailureReason = TEXT("Button Tree attach failed: SizeBox_Root -> Button_Interaction");
			return false;
		}
		if (!SetContent(Interaction, Visual))
		{
			OutFailureReason = TEXT("Button Tree attach failed: Button_Interaction -> Overlay_Visual");
			return false;
		}
		if (!AddChild(Visual, Background))
		{
			OutFailureReason = TEXT("Button Tree attach failed: Overlay_Visual -> Border_Background");
			return false;
		}
		if (!AddChild(Visual, FocusOutline))
		{
			OutFailureReason = TEXT("Button Tree attach failed: Overlay_Visual -> Border_FocusOutline");
			return false;
		}
		if (!AddChild(Visual, Content))
		{
			OutFailureReason = TEXT("Button Tree attach failed: Overlay_Visual -> HorizontalBox_Content");
			return false;
		}
		if (!AddChild(Content, Icon))
		{
			OutFailureReason = TEXT("Button Tree attach failed: HorizontalBox_Content -> Image_Icon");
			return false;
		}
		if (!AddChild(Content, Label))
		{
			OutFailureReason = TEXT("Button Tree attach failed: HorizontalBox_Content -> Text_Label");
			return false;
		}
		if (!AddChild(Content, Fill))
		{
			OutFailureReason = TEXT("Button Tree attach failed: HorizontalBox_Content -> Spacer_Fill");
			return false;
		}
		if (!AddChild(Content, InputHint))
		{
			OutFailureReason = TEXT("Button Tree attach failed: HorizontalBox_Content -> Text_InputHint");
			return false;
		}
		if (!AddChild(Visual, StateMarker))
		{
			OutFailureReason = TEXT("Button Tree attach failed: Overlay_Visual -> Border_StateMarker");
			return false;
		}
		return true;
	}

	// [v1.0.0] WBP_CFPanelBase 정확한 Designer Tree를 생성합니다.
	bool BuildPanelTree(UWidgetTree* Tree, FString& OutFailureReason)
	{
				// [v1.0.1] Panel의 배경·외곽선·Accent·내용을 겹쳐 표시할 Root Overlay입니다.
		UOverlay* Root = CreateWidget<UOverlay>(Tree, TEXT("Overlay_Root"));
		// [v1.0.1] Panel 기본 Surface를 표현할 Background Border입니다.
		UBorder* Background = CreateWidget<UBorder>(Tree, TEXT("Border_Background"));
		// [v1.0.1] Panel 외곽선을 표현할 Outline Border입니다.
		UBorder* Outline = CreateWidget<UBorder>(Tree, TEXT("Border_Outline"));
		// [v1.0.1] 의미형 Accent Marker를 표현할 Border입니다.
		UBorder* AccentMarker = CreateWidget<UBorder>(Tree, TEXT("Border_AccentMarker"));
		// [v1.0.1] Header와 Content를 세로 배치할 Layout Box입니다.
		UVerticalBox* Layout = CreateWidget<UVerticalBox>(Tree, TEXT("VerticalBox_Layout"));
		// [v1.0.1] Density Header Height를 적용할 Header SizeBox입니다.
		USizeBox* Header = CreateWidget<USizeBox>(Tree, TEXT("SizeBox_Header"));
		// [v1.0.1] Header Icon·Title·추가 콘텐츠를 수평 배치할 Box입니다.
		UHorizontalBox* HeaderLayout = CreateWidget<UHorizontalBox>(Tree, TEXT("HorizontalBox_Header"));
		// [v1.0.1] Semantic Header Icon을 표시할 Image입니다.
		UImage* HeaderIcon = CreateWidget<UImage>(Tree, TEXT("Image_HeaderIcon"));
		// [v1.0.1] Panel Title을 표시할 Text입니다.
		UTextBlock* HeaderText = CreateWidget<UTextBlock>(Tree, TEXT("Text_Header"));
		// [v1.0.1] Panel별 추가 Header Visual을 외부에서 삽입할 NamedSlot입니다.
		UNamedSlot* HeaderExtra = CreateWidget<UNamedSlot>(Tree, TEXT("NamedSlot_HeaderExtra"));
		// [v1.0.1] Panel 본문을 외부 Widget이 삽입할 NamedSlot입니다.
		UNamedSlot* Content = CreateWidget<UNamedSlot>(Tree, TEXT("NamedSlot_Content"));

		if (!Root || !Background || !Outline || !AccentMarker || !Layout || !Header || !HeaderLayout || !HeaderIcon || !HeaderText || !HeaderExtra || !Content)
		{
			OutFailureReason = TEXT("Panel Tree widget construction failed");
			return false;
		}

		Tree->RootWidget = Root;
		return AddChild(Root, Background)
			&& AddChild(Root, Outline)
			&& AddChild(Root, AccentMarker)
			&& AddChild(Root, Layout)
			&& AddChild(Layout, Header)
			&& SetContent(Header, HeaderLayout)
			&& AddChild(HeaderLayout, HeaderIcon)
			&& AddChild(HeaderLayout, HeaderText)
			&& AddChild(HeaderLayout, HeaderExtra)
			&& AddChild(Layout, Content);
	}

	// [v1.0.0] WBP_CFStatusBar 정확한 Designer Tree를 생성합니다.
	bool BuildStatusBarTree(UWidgetTree* Tree, FString& OutFailureReason)
	{
				// [v1.0.1] StatusBar 전체 크기를 제어할 Root SizeBox입니다.
		USizeBox* Root = CreateWidget<USizeBox>(Tree, TEXT("SizeBox_Root"));
		// [v1.0.1] Track·Continuous·Segment·Text를 겹쳐 표시할 Bar Overlay입니다.
		UOverlay* Bar = CreateWidget<UOverlay>(Tree, TEXT("Overlay_Bar"));
		// [v1.0.1] StatusBar 빈 Track을 표현할 Border입니다.
		UBorder* Track = CreateWidget<UBorder>(Tree, TEXT("Border_Track"));
		// [v1.0.1] Shield·Integrity·Resource 등 연속형 값을 표시할 ProgressBar입니다.
		UProgressBar* Continuous = CreateWidget<UProgressBar>(Tree, TEXT("ProgressBar_Continuous"));
		// [v1.0.1] Armor 10 Segment Visual을 담을 전용 HorizontalBox입니다.
		UHorizontalBox* ArmorSegments = CreateWidget<UHorizontalBox>(Tree, TEXT("HorizontalBox_ArmorSegments"));
		// [v1.0.1] 주요 숫자 또는 상태 값을 표시할 Text입니다.
		UTextBlock* PrimaryValue = CreateWidget<UTextBlock>(Tree, TEXT("Text_PrimaryValue"));
		// [v1.0.1] 보조 숫자 또는 Label 값을 표시할 Text입니다.
		UTextBlock* SecondaryValue = CreateWidget<UTextBlock>(Tree, TEXT("Text_SecondaryValue"));

		if (!Root || !Bar || !Track || !Continuous || !ArmorSegments || !PrimaryValue || !SecondaryValue)
		{
			OutFailureReason = TEXT("StatusBar Tree widget construction failed");
			return false;
		}

		Tree->RootWidget = Root;
		return SetContent(Root, Bar)
			&& AddChild(Bar, Track)
			&& AddChild(Bar, Continuous)
			&& AddChild(Bar, ArmorSegments)
			&& AddChild(Bar, PrimaryValue)
			&& AddChild(Bar, SecondaryValue);
	}

	// [v1.0.0] WBP_CFInfoRow 정확한 Designer Tree를 생성합니다.
	bool BuildInfoRowTree(UWidgetTree* Tree, FString& OutFailureReason)
	{
				// [v1.0.1] Label·Value·Fill·StateIcon을 수평 배치할 Root Box입니다.
		UHorizontalBox* Root = CreateWidget<UHorizontalBox>(Tree, TEXT("HorizontalBox_Root"));
		// [v1.0.1] Label 예약 폭을 적용할 SizeBox입니다.
		USizeBox* LabelBox = CreateWidget<USizeBox>(Tree, TEXT("SizeBox_Label"));
		// [v1.0.1] Info Row Label을 표시할 Text입니다.
		UTextBlock* Label = CreateWidget<UTextBlock>(Tree, TEXT("Text_Label"));
		// [v1.0.1] Value 예약 폭을 적용할 SizeBox입니다.
		USizeBox* ValueBox = CreateWidget<USizeBox>(Tree, TEXT("SizeBox_Value"));
		// [v1.0.1] Known/Unknown/Unavailable 등 실제 표시 문자열을 담을 Text입니다.
		UTextBlock* Value = CreateWidget<UTextBlock>(Tree, TEXT("Text_Value"));
		// [v1.0.1] 값과 상태 Icon 사이 남는 폭을 채울 Spacer입니다.
		USpacer* Fill = CreateWidget<USpacer>(Tree, TEXT("Spacer_Fill"));
		// [v1.0.1] Knowledge State의 Semantic Icon을 표시할 Image입니다.
		UImage* StateIcon = CreateWidget<UImage>(Tree, TEXT("Image_StateIcon"));

		if (!Root || !LabelBox || !Label || !ValueBox || !Value || !Fill || !StateIcon)
		{
			OutFailureReason = TEXT("InfoRow Tree widget construction failed");
			return false;
		}

		Tree->RootWidget = Root;
		return AddChild(Root, LabelBox)
			&& SetContent(LabelBox, Label)
			&& AddChild(Root, ValueBox)
			&& SetContent(ValueBox, Value)
			&& AddChild(Root, Fill)
			&& AddChild(Root, StateIcon);
	}

	// [v1.0.0] WBP_CFAlertItem 정확한 Designer Tree를 생성합니다.
	bool BuildAlertItemTree(UWidgetTree* Tree, FString& OutFailureReason)
	{
				// [v1.0.1] Alert 전체 높이와 폭을 제어할 Root SizeBox입니다.
		USizeBox* Root = CreateWidget<USizeBox>(Tree, TEXT("SizeBox_Root"));
		// [v1.0.1] Background·Accent·Content를 겹쳐 표시할 Root Overlay입니다.
		UOverlay* Overlay = CreateWidget<UOverlay>(Tree, TEXT("Overlay_Root"));
		// [v1.0.1] Alert Severity별 Surface를 표현할 Background Border입니다.
		UBorder* Background = CreateWidget<UBorder>(Tree, TEXT("Border_Background"));
		// [v1.0.1] Severity Accent Line을 표현할 Border입니다.
		UBorder* AccentLine = CreateWidget<UBorder>(Tree, TEXT("Border_AccentLine"));
		// [v1.0.1] Severity Icon과 Display Text를 수평 배치할 Content Box입니다.
		UHorizontalBox* Content = CreateWidget<UHorizontalBox>(Tree, TEXT("HorizontalBox_Content"));
		// [v1.0.1] Alert Severity Semantic Icon을 표시할 Image입니다.
		UImage* Severity = CreateWidget<UImage>(Tree, TEXT("Image_Severity"));
		// [v1.0.1] Alert 표시 문자열을 렌더링할 Text입니다.
		UTextBlock* Display = CreateWidget<UTextBlock>(Tree, TEXT("Text_Display"));

		if (!Root || !Overlay || !Background || !AccentLine || !Content || !Severity || !Display)
		{
			OutFailureReason = TEXT("AlertItem Tree widget construction failed");
			return false;
		}

		Tree->RootWidget = Root;
		return SetContent(Root, Overlay)
			&& AddChild(Overlay, Background)
			&& AddChild(Overlay, AccentLine)
			&& AddChild(Overlay, Content)
			&& AddChild(Content, Severity)
			&& AddChild(Content, Display);
	}

	// [v1.0.0] Button Template Tree의 전체 직접 Child 구조를 검증합니다.
	bool ValidateButtonTree(UWidgetTree* Tree, FString& OutFailureReason)
	{
		USizeBox* Root = Cast<USizeBox>(Tree ? Tree->RootWidget : nullptr);
		if (!ValidateWidgetIdentity(Root, TEXT("SizeBox_Root"), USizeBox::StaticClass(), OutFailureReason))
		{
			return false;
		}
		UButton* Interaction = Cast<UButton>(Root->GetContent());
		if (!ValidateWidgetIdentity(Interaction, TEXT("Button_Interaction"), UButton::StaticClass(), OutFailureReason))
		{
			return false;
		}
		UOverlay* Visual = Interaction ? Cast<UOverlay>(Interaction->GetContent()) : nullptr;
		if (!ValidateWidgetIdentity(Visual, TEXT("Overlay_Visual"), UOverlay::StaticClass(), OutFailureReason))
		{
			return false;
		}
		if (!ValidateChildren(Visual, {
			{FName(TEXT("Border_Background")), UBorder::StaticClass()},
			{FName(TEXT("Border_FocusOutline")), UBorder::StaticClass()},
			{FName(TEXT("HorizontalBox_Content")), UHorizontalBox::StaticClass()},
			{FName(TEXT("Border_StateMarker")), UBorder::StaticClass()}}, OutFailureReason))
		{
			return false;
		}
		UHorizontalBox* Content = Cast<UHorizontalBox>(Visual->GetChildAt(2));
		return ValidateChildren(Content, {
			{FName(TEXT("Image_Icon")), UImage::StaticClass()},
			{FName(TEXT("Text_Label")), UTextBlock::StaticClass()},
			{FName(TEXT("Spacer_Fill")), USpacer::StaticClass()},
			{FName(TEXT("Text_InputHint")), UTextBlock::StaticClass()}}, OutFailureReason);
	}

	// [v1.0.0] Panel Template Tree의 전체 직접 Child 구조를 검증합니다.
	bool ValidatePanelTree(UWidgetTree* Tree, FString& OutFailureReason)
	{
		UOverlay* Root = Cast<UOverlay>(Tree ? Tree->RootWidget : nullptr);
		if (!ValidateWidgetIdentity(Root, TEXT("Overlay_Root"), UOverlay::StaticClass(), OutFailureReason)
			|| !ValidateChildren(Root, {
				{FName(TEXT("Border_Background")), UBorder::StaticClass()},
				{FName(TEXT("Border_Outline")), UBorder::StaticClass()},
				{FName(TEXT("Border_AccentMarker")), UBorder::StaticClass()},
				{FName(TEXT("VerticalBox_Layout")), UVerticalBox::StaticClass()}}, OutFailureReason))
		{
			return false;
		}
		UVerticalBox* Layout = Cast<UVerticalBox>(Root->GetChildAt(3));
		if (!ValidateChildren(Layout, {
			{FName(TEXT("SizeBox_Header")), USizeBox::StaticClass()},
			{FName(TEXT("NamedSlot_Content")), UNamedSlot::StaticClass()}}, OutFailureReason))
		{
			return false;
		}
		USizeBox* Header = Cast<USizeBox>(Layout->GetChildAt(0));
		UHorizontalBox* HeaderLayout = Header ? Cast<UHorizontalBox>(Header->GetContent()) : nullptr;
		if (!ValidateWidgetIdentity(HeaderLayout, TEXT("HorizontalBox_Header"), UHorizontalBox::StaticClass(), OutFailureReason))
		{
			return false;
		}
		return ValidateChildren(HeaderLayout, {
			{FName(TEXT("Image_HeaderIcon")), UImage::StaticClass()},
			{FName(TEXT("Text_Header")), UTextBlock::StaticClass()},
			{FName(TEXT("NamedSlot_HeaderExtra")), UNamedSlot::StaticClass()}}, OutFailureReason);
	}

	// [v1.0.0] StatusBar Template Tree의 전체 직접 Child 구조를 검증합니다.
	bool ValidateStatusBarTree(UWidgetTree* Tree, FString& OutFailureReason)
	{
		USizeBox* Root = Cast<USizeBox>(Tree ? Tree->RootWidget : nullptr);
		if (!ValidateWidgetIdentity(Root, TEXT("SizeBox_Root"), USizeBox::StaticClass(), OutFailureReason))
		{
			return false;
		}
		UOverlay* Bar = Root ? Cast<UOverlay>(Root->GetContent()) : nullptr;
		if (!ValidateWidgetIdentity(Bar, TEXT("Overlay_Bar"), UOverlay::StaticClass(), OutFailureReason))
		{
			return false;
		}
		return ValidateChildren(Bar, {
			{FName(TEXT("Border_Track")), UBorder::StaticClass()},
			{FName(TEXT("ProgressBar_Continuous")), UProgressBar::StaticClass()},
			{FName(TEXT("HorizontalBox_ArmorSegments")), UHorizontalBox::StaticClass()},
			{FName(TEXT("Text_PrimaryValue")), UTextBlock::StaticClass()},
			{FName(TEXT("Text_SecondaryValue")), UTextBlock::StaticClass()}}, OutFailureReason);
	}

	// [v1.0.0] InfoRow Template Tree의 전체 직접 Child 구조를 검증합니다.
	bool ValidateInfoRowTree(UWidgetTree* Tree, FString& OutFailureReason)
	{
		UHorizontalBox* Root = Cast<UHorizontalBox>(Tree ? Tree->RootWidget : nullptr);
		if (!ValidateWidgetIdentity(Root, TEXT("HorizontalBox_Root"), UHorizontalBox::StaticClass(), OutFailureReason)
			|| !ValidateChildren(Root, {
				{FName(TEXT("SizeBox_Label")), USizeBox::StaticClass()},
				{FName(TEXT("SizeBox_Value")), USizeBox::StaticClass()},
				{FName(TEXT("Spacer_Fill")), USpacer::StaticClass()},
				{FName(TEXT("Image_StateIcon")), UImage::StaticClass()}}, OutFailureReason))
		{
			return false;
		}
		USizeBox* LabelBox = Cast<USizeBox>(Root->GetChildAt(0));
		USizeBox* ValueBox = Cast<USizeBox>(Root->GetChildAt(1));
		return ValidateWidgetIdentity(LabelBox ? LabelBox->GetContent() : nullptr, TEXT("Text_Label"), UTextBlock::StaticClass(), OutFailureReason)
			&& ValidateWidgetIdentity(ValueBox ? ValueBox->GetContent() : nullptr, TEXT("Text_Value"), UTextBlock::StaticClass(), OutFailureReason);
	}

	// [v1.0.0] AlertItem Template Tree의 전체 직접 Child 구조를 검증합니다.
	bool ValidateAlertItemTree(UWidgetTree* Tree, FString& OutFailureReason)
	{
		USizeBox* Root = Cast<USizeBox>(Tree ? Tree->RootWidget : nullptr);
		if (!ValidateWidgetIdentity(Root, TEXT("SizeBox_Root"), USizeBox::StaticClass(), OutFailureReason))
		{
			return false;
		}
		UOverlay* Overlay = Root ? Cast<UOverlay>(Root->GetContent()) : nullptr;
		if (!ValidateWidgetIdentity(Overlay, TEXT("Overlay_Root"), UOverlay::StaticClass(), OutFailureReason)
			|| !ValidateChildren(Overlay, {
				{FName(TEXT("Border_Background")), UBorder::StaticClass()},
				{FName(TEXT("Border_AccentLine")), UBorder::StaticClass()},
				{FName(TEXT("HorizontalBox_Content")), UHorizontalBox::StaticClass()}}, OutFailureReason))
		{
			return false;
		}
		UHorizontalBox* Content = Cast<UHorizontalBox>(Overlay->GetChildAt(2));
		return ValidateChildren(Content, {
			{FName(TEXT("Image_Severity")), UImage::StaticClass()},
			{FName(TEXT("Text_Display")), UTextBlock::StaticClass()}}, OutFailureReason);
	}
#endif
}

// [v1.0.0] 지정 Widget Blueprint의 Designer Tree를 D1-10B Template ID에 맞는 정확한 구조로 생성합니다.
bool UCFUIBaseEditorBridge::BuildBaseWidgetTree(UObject* WidgetBlueprintObject, FName TemplateId, FString& OutFailureReason)
{
	OutFailureReason.Reset();
#if WITH_EDITOR
	// [v1.1.3] Python에서 전달된 객체를 실제 Widget Blueprint로 변환한 결과입니다.
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(WidgetBlueprintObject);
	if (!WidgetBlueprint)
	{
		OutFailureReason = TEXT("WidgetBlueprint cast failed");
		UE_LOG(LogTemp, Error, TEXT("[CarFight][UIBaseEditorBridge] Build precondition failed: WidgetBlueprint cast failed ObjectClass=%s"), WidgetBlueprintObject ? *WidgetBlueprintObject->GetClass()->GetName() : TEXT("None"));
		return false;
	}
	if (!WidgetBlueprint->WidgetTree)
	{
		OutFailureReason = TEXT("WidgetTree is null");
		UE_LOG(LogTemp, Error, TEXT("[CarFight][UIBaseEditorBridge] Build precondition failed: WidgetTree is null Blueprint=%s"), *WidgetBlueprint->GetPathName());
		return false;
	}

	WidgetBlueprint->Modify();
	WidgetBlueprint->WidgetTree->Modify();
	WidgetBlueprint->WidgetTree->RootWidget = nullptr;

	bool bBuilt = false;
	if (TemplateId == FName(TEXT("Button")))
	{
		bBuilt = CFUIBaseEditorBridge::BuildButtonTree(WidgetBlueprint->WidgetTree, OutFailureReason);
	}
	else if (TemplateId == FName(TEXT("Panel")))
	{
		bBuilt = CFUIBaseEditorBridge::BuildPanelTree(WidgetBlueprint->WidgetTree, OutFailureReason);
	}
	else if (TemplateId == FName(TEXT("StatusBar")))
	{
		bBuilt = CFUIBaseEditorBridge::BuildStatusBarTree(WidgetBlueprint->WidgetTree, OutFailureReason);
	}
	else if (TemplateId == FName(TEXT("InfoRow")))
	{
		bBuilt = CFUIBaseEditorBridge::BuildInfoRowTree(WidgetBlueprint->WidgetTree, OutFailureReason);
	}
	else if (TemplateId == FName(TEXT("AlertItem")))
	{
		bBuilt = CFUIBaseEditorBridge::BuildAlertItemTree(WidgetBlueprint->WidgetTree, OutFailureReason);
	}
	else
	{
		OutFailureReason = FString::Printf(TEXT("Unknown D1-10B TemplateId: %s"), *TemplateId.ToString());
		return false;
	}

		if (!bBuilt)
	{
		if (OutFailureReason.IsEmpty())
		{
			OutFailureReason = TEXT("WidgetTree builder returned false");
		}
		UE_LOG(LogTemp, Error, TEXT("[CarFight][UIBaseEditorBridge] Build failed: TemplateId=%s Reason=%s"), *TemplateId.ToString(), *OutFailureReason);
		return false;
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
	return true;
#else
	OutFailureReason = TEXT("D1-10B Base Widget Tree creation is Editor-only");
	return false;
#endif
}

// [v1.2.0] UE Python에서 Build 결과 Bool만 안정적으로 받기 위해 기존 Build 함수의 Out FString을 내부 지역 변수로 흡수합니다.
bool UCFUIBaseEditorBridge::BuildBaseWidgetTreeResult(UObject* WidgetBlueprintObject, FName TemplateId)
{
	// [v1.2.0] 기존 Build 함수가 제공하는 실패 사유를 Python 반환 계약과 분리해 보존하는 지역 문자열입니다.
	FString FailureReason;
	// [v1.2.0] 기존 Build 구현의 정확한 성공 여부입니다.
	const bool bBuildSucceeded = BuildBaseWidgetTree(WidgetBlueprintObject, TemplateId, FailureReason);
	if (!bBuildSucceeded)
	{
		UE_LOG(LogTemp, Error, TEXT("[CarFight][UIBaseEditorBridge] Build result wrapper failed: TemplateId=%s Reason=%s"), *TemplateId.ToString(), *FailureReason);
	}
	return bBuildSucceeded;
}

// [v1.0.0] 지정 Widget Blueprint가 D1-10B Tree 구조, Spacer Visibility, Cast 0, CallFunction 0 계약을 만족하는지 읽기 전용으로 검증합니다.
bool UCFUIBaseEditorBridge::ValidateBaseWidgetTree(UObject* WidgetBlueprintObject, FName TemplateId, FString& OutFailureReason)
{
	OutFailureReason.Reset();
#if WITH_EDITOR
	const UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(WidgetBlueprintObject);
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
	{
		OutFailureReason = TEXT("WidgetBlueprint or WidgetTree is invalid");
		return false;
	}

	bool bTreeValid = false;
	if (TemplateId == FName(TEXT("Button")))
	{
		bTreeValid = CFUIBaseEditorBridge::ValidateButtonTree(WidgetBlueprint->WidgetTree, OutFailureReason);
	}
	else if (TemplateId == FName(TEXT("Panel")))
	{
		bTreeValid = CFUIBaseEditorBridge::ValidatePanelTree(WidgetBlueprint->WidgetTree, OutFailureReason);
	}
	else if (TemplateId == FName(TEXT("StatusBar")))
	{
		bTreeValid = CFUIBaseEditorBridge::ValidateStatusBarTree(WidgetBlueprint->WidgetTree, OutFailureReason);
	}
	else if (TemplateId == FName(TEXT("InfoRow")))
	{
		bTreeValid = CFUIBaseEditorBridge::ValidateInfoRowTree(WidgetBlueprint->WidgetTree, OutFailureReason);
	}
	else if (TemplateId == FName(TEXT("AlertItem")))
	{
		bTreeValid = CFUIBaseEditorBridge::ValidateAlertItemTree(WidgetBlueprint->WidgetTree, OutFailureReason);
	}
	else
	{
		OutFailureReason = FString::Printf(TEXT("Unknown D1-10B TemplateId: %s"), *TemplateId.ToString());
		return false;
	}

		if (!CFUIBaseEditorBridge::ValidateNativeParent(WidgetBlueprint, TemplateId, OutFailureReason))
	{
		return false;
	}
	if (TemplateId == FName(TEXT("Button")) && !CFUIBaseEditorBridge::ValidateButtonBindContract(OutFailureReason))
	{
		return false;
	}

	return bTreeValid
		&& CFUIBaseEditorBridge::ValidateSpacerVisibility(WidgetBlueprint->WidgetTree, OutFailureReason)
		&& CFUIBaseEditorBridge::ValidateGraphSafety(WidgetBlueprint, OutFailureReason);
#else
	OutFailureReason = TEXT("D1-10B Base Widget validation is Editor-only");
	return false;
#endif
}

// [v1.2.0] UE Python에서 Validate 결과 Bool만 안정적으로 받기 위해 기존 Validate 함수의 Out FString을 내부 지역 변수로 흡수합니다.
bool UCFUIBaseEditorBridge::ValidateBaseWidgetTreeResult(UObject* WidgetBlueprintObject, FName TemplateId)
{
	// [v1.2.0] 기존 Validate 함수가 제공하는 실패 사유를 Python 반환 계약과 분리해 보존하는 지역 문자열입니다.
	FString FailureReason;
	// [v1.2.0] 기존 Validate 구현의 정확한 성공 여부입니다.
	const bool bValidationSucceeded = ValidateBaseWidgetTree(WidgetBlueprintObject, TemplateId, FailureReason);
	if (!bValidationSucceeded)
	{
		UE_LOG(LogTemp, Error, TEXT("[CarFight][UIBaseEditorBridge] Validate result wrapper failed: TemplateId=%s Reason=%s"), *TemplateId.ToString(), *FailureReason);
	}
	return bValidationSucceeded;
}
