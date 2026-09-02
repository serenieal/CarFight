// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.4
// Date: 2026-08-31
// Description: CF-FQ-032 UI-DESIGN-GATE D1-09A Layout·Density·Font·Subsystem 격리 Fallback 자동화 테스트
// Scope: 승인된 1080p Slot, Density Preset, Font Binding과 Config 미해석 시 LocalPlayer 격리 Native Fallback 계약을 검증합니다.
// Changelog:
// - v1.0.4: UISubsystem fallback이 전역 CDO가 아니라 Subsystem 소유 transient 객체이며 local mutation이 CDO에 전파되지 않음을 검증.
// - v1.0.3: UE 5.8 ClassWithin 계층을 실제 Engine → LocalPlayer → LocalPlayerSubsystem 순서로 맞추도록 Fixture를 수정.
// - v1.0.2: ULocalPlayerSubsystem의 UE 5.8 ClassWithin 계약에 맞게 Transient ULocalPlayer를 Subsystem Outer로 사용하는 Fixture로 수정.
// - v1.0.1: UE 5.8 TArray 자기참조 안전 Assert를 피하도록 중복 Slot Fixture를 지역 복사 후 추가하도록 수정.
// - v1.0.0: LayoutDataContract, DensityDataContract, FontBindingContract, SubsystemFallbackContract를 최초 추가.
// Migration:
// - v1.0.4 Fallback의 값 계약은 기존 Native defaults를 유지하지만 object identity는 전역 CDO 공유에서 LocalPlayer Subsystem 전용 instance로 변경됩니다.
// - 테스트는 Transient UObject만 사용하며 Config, Unreal Asset, 기존 HUD와 Pause Runtime을 수정하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "UI/CFHUDLayoutData.h"
#include "UI/CFUIDensityData.h"
#include "UI/CFUIStyleData.h"
#include "UI/CFUISubsystem.h"

#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUILayoutDataContractTest,
	"CarFight.UI.D1_09A.LayoutDataContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Native 1080p Layout이 D1-07 USER PASS Slot과 안전 검증 계약을 유지하는지 확인합니다.
bool FCFUILayoutDataContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Unreal Asset 없이 Native Layout Fallback을 검증할 Transient Data입니다.
	UCFHUDLayoutData* LayoutData = NewObject<UCFHUDLayoutData>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient HUD Layout Data"), LayoutData))
	{
		return false;
	}

	// [v1.0.0] Native Layout 전체 계약의 검증 실패 사유입니다.
	FString ValidationFailureReason;
	TestTrue(TEXT("Native 1080p Layout 검증"), LayoutData->ValidateLayoutData(ValidationFailureReason));
	TestTrue(TEXT("Native Layout 검증 오류 없음"), ValidationFailureReason.IsEmpty());
	TestEqual(TEXT("Required Slot Count"), LayoutData->SlotLayouts.Num(), 7);
	TestTrue(TEXT("Reference Viewport X = 1920"), FMath::IsNearlyEqual(LayoutData->ReferenceViewportSize.X, 1920.0));
	TestTrue(TEXT("Reference Viewport Y = 1080"), FMath::IsNearlyEqual(LayoutData->ReferenceViewportSize.Y, 1080.0));
	TestTrue(TEXT("Geometry Scale = 0.75"), FMath::IsNearlyEqual(LayoutData->GeometryScale, 0.75f));
	TestTrue(TEXT("Typography Scale = 0.75"), FMath::IsNearlyEqual(LayoutData->TypographyScale, 0.75f));
	TestEqual(TEXT("HeadingM 1080p Font Floor"), LayoutData->ResolveMinimumFontSize(ECFUITypographyRole::HeadingM), 18);
	TestEqual(TEXT("Caption 1080p Font Floor"), LayoutData->ResolveMinimumFontSize(ECFUITypographyRole::Caption), 12);

	// [v1.0.0] 승인된 좌하단 Vehicle Slot 값을 검증할 Layout Entry입니다.
	const FCFHUDSlotLayout* VehicleSlot = LayoutData->FindSlotLayout(ECFHUDSlotId::VehiclePanel);
	if (TestNotNull(TEXT("VehiclePanel Slot"), VehicleSlot))
	{
		TestTrue(TEXT("Vehicle Offset X = 24"), FMath::IsNearlyEqual(VehicleSlot->PixelOffset.X, 24.0));
		TestTrue(TEXT("Vehicle Offset Y = -24"), FMath::IsNearlyEqual(VehicleSlot->PixelOffset.Y, -24.0));
		TestTrue(TEXT("Vehicle Width = 672"), FMath::IsNearlyEqual(VehicleSlot->DesiredSize.X, 672.0));
		TestTrue(TEXT("Vehicle Height = 312"), FMath::IsNearlyEqual(VehicleSlot->DesiredSize.Y, 312.0));
	}

	// [v1.0.0] 승인된 우하단 Weapon Compact Slot 값을 검증할 Layout Entry입니다.
	const FCFHUDSlotLayout* WeaponSlot = LayoutData->FindSlotLayout(ECFHUDSlotId::WeaponPanel);
	if (TestNotNull(TEXT("WeaponPanel Slot"), WeaponSlot))
	{
		TestTrue(TEXT("Weapon Width = 270"), FMath::IsNearlyEqual(WeaponSlot->DesiredSize.X, 270.0));
		TestTrue(TEXT("Weapon Height = 190.5"), FMath::IsNearlyEqual(WeaponSlot->DesiredSize.Y, 190.5));
	}

		// [v1.0.1] TArray 자기참조를 피하면서 중복 Slot ID를 만들기 위해 먼저 값으로 복사한 Fixture입니다.
	const FCFHUDSlotLayout DuplicateSlotLayout = LayoutData->SlotLayouts[0];
	LayoutData->SlotLayouts.Add(DuplicateSlotLayout);
	ValidationFailureReason.Reset();
	TestFalse(TEXT("중복 Slot ID 검출"), LayoutData->ValidateLayoutData(ValidationFailureReason));
	TestTrue(TEXT("중복 Slot 검증 사유 제공"), !ValidationFailureReason.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUIDensityDataContractTest,
	"CarFight.UI.D1_09A.DensityDataContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Compact·Standard·Expanded Density가 승인값과 의미 보존 계약을 유지하는지 확인합니다.
bool FCFUIDensityDataContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Density Preset 전환을 검증할 Transient Data입니다.
	UCFUIDensityData* DensityData = NewObject<UCFUIDensityData>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient UI Density Data"), DensityData))
	{
		return false;
	}

	// [v1.0.0] Standard Native Fallback의 안전 검증 실패 사유입니다.
	FString ValidationFailureReason;
	TestTrue(TEXT("Standard Density 검증"), DensityData->ValidateDensityData(ValidationFailureReason));
	TestEqual(TEXT("Standard Panel Padding"), DensityData->Tokens.PanelPadding, 24.0f);
	TestEqual(TEXT("Standard Header Height"), DensityData->Tokens.HeaderHeight, 48.0f);
	TestEqual(TEXT("Standard Caption Policy"), static_cast<uint8>(DensityData->Tokens.CaptionPolicy), static_cast<uint8>(ECFUICaptionPolicy::Contextual));

	DensityData->ApplyPresetDefaults(ECFUIDensityPreset::Compact);
	TestEqual(TEXT("Compact Panel Padding"), DensityData->Tokens.PanelPadding, 12.0f);
	TestEqual(TEXT("Compact Header Height"), DensityData->Tokens.HeaderHeight, 36.0f);
	TestEqual(TEXT("Compact Primary Value Scale"), DensityData->Tokens.PrimaryValueScale, 0.86f);
	TestEqual(TEXT("Compact Caption Policy"), static_cast<uint8>(DensityData->Tokens.CaptionPolicy), static_cast<uint8>(ECFUICaptionPolicy::CoreOnly));

	DensityData->ApplyPresetDefaults(ECFUIDensityPreset::Expanded);
	TestEqual(TEXT("Expanded Panel Padding"), DensityData->Tokens.PanelPadding, 32.0f);
	TestEqual(TEXT("Expanded Header Height"), DensityData->Tokens.HeaderHeight, 56.0f);
	TestEqual(TEXT("Expanded Primary Value Scale"), DensityData->Tokens.PrimaryValueScale, 1.12f);
	TestEqual(TEXT("Expanded Caption Policy"), static_cast<uint8>(DensityData->Tokens.CaptionPolicy), static_cast<uint8>(ECFUICaptionPolicy::Expanded));

	// [v1.0.0] 음수 Geometry 값을 실제로 거부하는지 확인하기 위한 임시 오류 값입니다.
	DensityData->Tokens.PanelPadding = -1.0f;
	ValidationFailureReason.Reset();
	TestFalse(TEXT("Density 음수 Geometry 검출"), DensityData->ValidateDensityData(ValidationFailureReason));
	TestTrue(TEXT("Density 검증 사유 제공"), !ValidationFailureReason.IsEmpty());

	// [v1.0.0] Density가 접근성 MinimumHitSize 자체를 변경하지 않음을 확인할 Native Style Fallback입니다.
	const UCFUIStyleData* NativeStyleData = GetDefault<UCFUIStyleData>();
	TestNotNull(TEXT("Native Style Fallback"), NativeStyleData);
	if (NativeStyleData)
	{
		TestEqual(TEXT("MinimumHitSize는 Density 비스케일"), NativeStyleData->Spacing.MinimumHitSize, 48.0f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUIFontBindingContractTest,
	"CarFight.UI.D1_09A.FontBindingContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Font Family·Weight·Typography Scale이 경로 하드코딩 없이 안정적으로 해석되는지 확인합니다.
bool FCFUIFontBindingContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 실제 Font Asset 생성 전 Native Binding 계약을 검증할 Transient Style Data입니다.
	UCFUIStyleData* StyleData = NewObject<UCFUIStyleData>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient Style Data for Font Binding"), StyleData))
	{
		return false;
	}

	TestNull(TEXT("D1-09B 전 UI Font Asset은 Null 허용"), StyleData->ResolveFontAsset(ECFUIFontFamilyRole::UI));
	TestNull(TEXT("D1-09B 전 Numeric Font Asset은 Null 허용"), StyleData->ResolveFontAsset(ECFUIFontFamilyRole::Numeric));
	TestEqual(TEXT("Regular Typeface"), StyleData->ResolveTypefaceName(ECFUIFontWeight::Regular), FName(TEXT("Regular")));
	TestEqual(TEXT("Medium Typeface"), StyleData->ResolveTypefaceName(ECFUIFontWeight::Medium), FName(TEXT("Medium")));
	TestEqual(TEXT("SemiBold Typeface"), StyleData->ResolveTypefaceName(ECFUIFontWeight::SemiBold), FName(TEXT("SemiBold")));
	TestEqual(TEXT("Bold Typeface"), StyleData->ResolveTypefaceName(ECFUIFontWeight::Bold), FName(TEXT("Bold")));

	// [v1.0.0] 1080p HeadingM 24×0.75가 정확히 18로 해석되는 Font 정보입니다.
	const FSlateFontInfo HeadingFont = StyleData->ResolveSlateFontInfo(ECFUIFontFamilyRole::UI, ECFUITypographyRole::HeadingM, 0.75f, 18);
	TestEqual(TEXT("HeadingM Effective Font Size"), HeadingFont.Size, 18.0f);

	// [v1.0.0] Caption 14×0.75가 12px 안전 하한으로 Clamp되는 Font 정보입니다.
	const FSlateFontInfo CaptionFont = StyleData->ResolveSlateFontInfo(ECFUIFontFamilyRole::UI, ECFUITypographyRole::Caption, 0.75f, 12);
	TestEqual(TEXT("Caption Effective Font Floor"), CaptionFont.Size, 12.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUISubsystemFallbackContractTest,
	"CarFight.UI.D1_09A.SubsystemFallbackContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.4] Config Asset을 아직 해석하지 않은 Subsystem Getter가 LocalPlayer 전용 Native fallback을 반환하고 전역 CDO mutation을 차단하는지 확인합니다.
bool FCFUISubsystemFallbackContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

			// [v1.0.3] Automation Editor에서 ULocalPlayer의 ClassWithin=Engine 계약을 만족할 실제 Engine Outer입니다.
	if (!TestNotNull(TEXT("Automation Engine Outer"), GEngine))
	{
		return false;
	}

	// [v1.0.3] ULocalPlayerSubsystem의 ClassWithin 계약을 만족할 Engine 소유 Transient LocalPlayer입니다.
	ULocalPlayer* TestLocalPlayer = NewObject<ULocalPlayer>(GEngine);
	if (!TestNotNull(TEXT("Transient LocalPlayer Outer"), TestLocalPlayer))
	{
		return false;
	}

	// [v1.0.3] Initialize를 호출하지 않아 Config Asset Cache가 비어 있는 LocalPlayer 소유 Transient Subsystem입니다.
	UCFUISubsystem* UISubsystem = NewObject<UCFUISubsystem>(TestLocalPlayer);
	if (!TestNotNull(TEXT("Transient UI Subsystem"), UISubsystem))
	{
		return false;
	}

	// [v1.0.4] 이 LocalPlayer Subsystem이 소유하는 Style fallback입니다.
	UCFUIStyleData* StyleFallback = UISubsystem->GetResolvedStyleData();
	// [v1.0.4] 이 LocalPlayer Subsystem이 소유하는 Density fallback입니다.
	UCFUIDensityData* DensityFallback = UISubsystem->GetResolvedDensityData();
	// [v1.0.4] 이 LocalPlayer Subsystem이 소유하는 HUD Layout fallback입니다.
	UCFHUDLayoutData* LayoutFallback = UISubsystem->GetResolvedHUDLayoutData();

	TestNotNull(TEXT("Style LocalPlayer Fallback"), StyleFallback);
	TestNotNull(TEXT("Density LocalPlayer Fallback"), DensityFallback);
	TestNotNull(TEXT("Layout LocalPlayer Fallback"), LayoutFallback);
	if (!StyleFallback || !DensityFallback || !LayoutFallback)
	{
		return false;
	}

	TestTrue(TEXT("Style Fallback은 전역 CDO가 아님"), StyleFallback != GetDefault<UCFUIStyleData>());
	TestTrue(TEXT("Density Fallback은 전역 CDO가 아님"), DensityFallback != GetDefault<UCFUIDensityData>());
	TestTrue(TEXT("Layout Fallback은 전역 CDO가 아님"), LayoutFallback != GetDefault<UCFHUDLayoutData>());
	TestEqual(TEXT("Style Fallback Outer는 UISubsystem"), StyleFallback->GetOuter(), static_cast<UObject*>(UISubsystem));
	TestEqual(TEXT("Density Fallback Outer는 UISubsystem"), DensityFallback->GetOuter(), static_cast<UObject*>(UISubsystem));
	TestEqual(TEXT("Layout Fallback Outer는 UISubsystem"), LayoutFallback->GetOuter(), static_cast<UObject*>(UISubsystem));

	// [v1.0.4] local Style mutation 전 전역 CDO MinimumHitSize 기준값입니다.
	const float GlobalStyleMinimumHitSizeBefore = GetDefault<UCFUIStyleData>()->Spacing.MinimumHitSize;
	// [v1.0.4] local Density mutation 전 전역 CDO PanelPadding 기준값입니다.
	const float GlobalDensityPanelPaddingBefore = GetDefault<UCFUIDensityData>()->Tokens.PanelPadding;
	// [v1.0.4] local Layout mutation 전 전역 CDO GeometryScale 기준값입니다.
	const float GlobalLayoutGeometryScaleBefore = GetDefault<UCFHUDLayoutData>()->GeometryScale;

	StyleFallback->Spacing.MinimumHitSize = GlobalStyleMinimumHitSizeBefore + 17.0f;
	DensityFallback->Tokens.PanelPadding = GlobalDensityPanelPaddingBefore + 11.0f;
	LayoutFallback->GeometryScale = GlobalLayoutGeometryScaleBefore + 0.25f;

	TestEqual(TEXT("Style local mutation은 전역 CDO 비변경"), GetDefault<UCFUIStyleData>()->Spacing.MinimumHitSize, GlobalStyleMinimumHitSizeBefore);
	TestEqual(TEXT("Density local mutation은 전역 CDO 비변경"), GetDefault<UCFUIDensityData>()->Tokens.PanelPadding, GlobalDensityPanelPaddingBefore);
	TestEqual(TEXT("Layout local mutation은 전역 CDO 비변경"), GetDefault<UCFHUDLayoutData>()->GeometryScale, GlobalLayoutGeometryScaleBefore);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
