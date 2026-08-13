// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-10
// Description: CF-FQ-032 UI-DESIGN-GATE D1-10A Base Widget Visual Context 자동화 테스트
// Scope: 외부 Style/Density Context, 중복 Refresh 방지, Geometry/Typography 해석과 Button Bind/Focus API 계약을 검증합니다.
// Changelog:
// - v1.0.0: VisualContextContract와 ButtonBridgeContract를 최초 추가.
// Migration:
// - 테스트는 Transient Widget/Data만 사용하며 D1-10B Widget Blueprint Asset을 생성하거나 수정하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "UI/CFButtonBaseWidget.h"
#include "UI/CFHUDLayoutData.h"
#include "UI/CFStyledWidgetBase.h"
#include "UI/CFUIDensityData.h"
#include "UI/CFUIStyleData.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUIVisualContextContractTest,
	"CarFight.UI.D1_10A.VisualContextContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Style/Density Context가 외부 주입되고 동일 Context 재적용은 Refresh를 중복 발생시키지 않는지 검증합니다.
bool FCFUIVisualContextContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] UUserWidget을 안전하게 생성할 Transient WidgetTree Outer입니다.
	UWidgetTree* HostWidgetTree = NewObject<UWidgetTree>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient Host WidgetTree"), HostWidgetTree))
	{
		return false;
	}

	// [v1.0.0] Visual Context Bridge 계약을 검증할 Transient Styled Widget입니다.
	UCFStyledWidgetBase* StyledWidget = CreateWidget<UCFStyledWidgetBase>(HostWidgetTree, UCFStyledWidgetBase::StaticClass());
	if (!TestNotNull(TEXT("Transient Styled Widget"), StyledWidget))
	{
		return false;
	}

	// [v1.0.0] 경로 Load 없이 외부 주입을 검증할 Transient Style Data입니다.
	UCFUIStyleData* StyleData = NewObject<UCFUIStyleData>(GetTransientPackage());

	// [v1.0.0] Standard Density 외부 주입을 검증할 Transient Density Data입니다.
	UCFUIDensityData* DensityData = NewObject<UCFUIDensityData>(GetTransientPackage());

	// [v1.0.0] 승인된 1080p Scale과 Typography Floor를 제공할 Native Layout Fixture입니다.
	UCFHUDLayoutData* LayoutData = NewObject<UCFHUDLayoutData>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient Style Data"), StyleData)
		|| !TestNotNull(TEXT("Transient Density Data"), DensityData)
		|| !TestNotNull(TEXT("Transient Layout Data"), LayoutData))
	{
		return false;
	}

	TestEqual(TEXT("Initial Visual Context Revision"), StyledWidget->GetVisualContextRevision(), 0);
	StyledWidget->SetUIVisualContext(
		StyleData,
		DensityData,
		LayoutData->GeometryScale,
		LayoutData->TypographyScale,
		LayoutData->MinimumEffectiveFontSizes);

	TestEqual(TEXT("First Context Apply Revision"), StyledWidget->GetVisualContextRevision(), 1);
	TestEqual(TEXT("Style Data external reference"), StyledWidget->GetUIStyleData(), static_cast<const UCFUIStyleData*>(StyleData));
	TestEqual(TEXT("Density Data external reference"), StyledWidget->GetUIDensityData(), static_cast<const UCFUIDensityData*>(DensityData));
	TestTrue(TEXT("Geometry Scale = 0.75"), FMath::IsNearlyEqual(StyledWidget->GetResolvedGeometryScale(), 0.75f));
	TestTrue(TEXT("24 DU spacing -> 18px"), FMath::IsNearlyEqual(StyledWidget->ResolveScaledSpacing(24.0f), 18.0f));
	TestEqual(TEXT("HeadingM 1080p Font Size"), StyledWidget->ResolveTypographyFont(ECFUIFontFamilyRole::UI, ECFUITypographyRole::HeadingM).Size, 18.0f);
	TestEqual(TEXT("Caption 1080p Floor"), StyledWidget->ResolveTypographyFont(ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption).Size, 12.0f);

	StyledWidget->SetUIVisualContext(
		StyleData,
		DensityData,
		LayoutData->GeometryScale,
		LayoutData->TypographyScale,
		LayoutData->MinimumEffectiveFontSizes);
	TestEqual(TEXT("Identical Context does not refresh"), StyledWidget->GetVisualContextRevision(), 1);

	StyledWidget->SetUIVisualContext(
		StyleData,
		DensityData,
		1.0f,
		LayoutData->TypographyScale,
		LayoutData->MinimumEffectiveFontSizes);
	TestEqual(TEXT("Changed Geometry refreshes once"), StyledWidget->GetVisualContextRevision(), 2);
	TestTrue(TEXT("24 DU spacing -> 24px after scale change"), FMath::IsNearlyEqual(StyledWidget->ResolveScaledSpacing(24.0f), 24.0f));

	StyledWidget->RefreshUIVisualStyle();
	TestEqual(TEXT("Explicit refresh increments Revision"), StyledWidget->GetVisualContextRevision(), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUIButtonBridgeContractTest,
	"CarFight.UI.D1_10A.ButtonBridgeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Button Base가 필수 BindWidget과 Activation/Focus/Enabled API를 노출하고 Gameplay 책임을 갖지 않는 최소 Bridge인지 검증합니다.
bool FCFUIButtonBridgeContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Button_Interaction BindWidget 계약을 Reflection으로 확인할 Object Property입니다.
	const FObjectProperty* InteractionButtonProperty = FindFProperty<FObjectProperty>(UCFButtonBaseWidget::StaticClass(), TEXT("Button_Interaction"));
	if (TestNotNull(TEXT("Button_Interaction property"), InteractionButtonProperty))
	{
				TestTrue(TEXT("Button_Interaction class"), InteractionButtonProperty->PropertyClass == UButton::StaticClass());
		TestTrue(TEXT("Button_Interaction BindWidget metadata"), InteractionButtonProperty->HasMetaData(TEXT("BindWidget")));
	}

	TestNotNull(TEXT("GetInteractionButton function"), UCFButtonBaseWidget::StaticClass()->FindFunctionByName(TEXT("GetInteractionButton")));
	TestNotNull(TEXT("FocusInteractionButton function"), UCFButtonBaseWidget::StaticClass()->FindFunctionByName(TEXT("FocusInteractionButton")));
	TestNotNull(TEXT("SetButtonEnabled function"), UCFButtonBaseWidget::StaticClass()->FindFunctionByName(TEXT("SetButtonEnabled")));
	TestNotNull(TEXT("OnButtonVisualStateChanged event"), UCFButtonBaseWidget::StaticClass()->FindFunctionByName(TEXT("OnButtonVisualStateChanged")));

	// [v1.0.0] 실제 D1-10B Blueprint Bind 전 Null Button 경로가 안전한지 검증할 Transient WidgetTree Outer입니다.
	UWidgetTree* HostWidgetTree = NewObject<UWidgetTree>(GetTransientPackage());

	// [v1.0.0] InteractionButton이 아직 Bind되지 않은 C++ Native Class 인스턴스입니다.
	UCFButtonBaseWidget* ButtonWidget = HostWidgetTree
		? CreateWidget<UCFButtonBaseWidget>(HostWidgetTree, UCFButtonBaseWidget::StaticClass())
		: nullptr;
	if (!TestNotNull(TEXT("Transient Button Base Widget"), ButtonWidget))
	{
		return false;
	}

	TestNull(TEXT("Native class has no designer Button bind"), ButtonWidget->GetInteractionButton());
	TestFalse(TEXT("Focus safely fails without designer Button"), ButtonWidget->FocusInteractionButton());
	ButtonWidget->SetButtonEnabled(false);
	TestEqual(TEXT("Disabled state without designer Button"), static_cast<uint8>(ButtonWidget->GetButtonVisualState()), static_cast<uint8>(ECFUIButtonVisualState::Disabled));
	ButtonWidget->SetButtonEnabled(true);
	TestEqual(TEXT("Enabled state returns Normal"), static_cast<uint8>(ButtonWidget->GetButtonVisualState()), static_cast<uint8>(ECFUIButtonVisualState::Normal));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
