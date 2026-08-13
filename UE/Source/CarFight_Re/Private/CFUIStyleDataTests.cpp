// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-10
// Description: CF-FQ-032 UI Style Data 자동화 테스트
// Scope: Native Safe Fallback Token, 의미 Color·Typography·Semantic Icon 해석과 최소 안전 하한 검증 계약을 확인합니다.
// Changelog:
// - v1.1.0: D1-11 Production UI Rework용 Semantic Icon Resolver와 중복 ID 검증을 추가.
// - v1.0.0: StyleDataContract 테스트를 최초 추가.
// Migration:
// - 테스트는 Transient UCFUIStyleData만 사용하며 Unreal Asset, Widget Blueprint와 기존 HUD를 생성하거나 수정하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "UI/CFUIStyleData.h"

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFUIStyleDataContractTest,
	"CarFight.UI.D1_08.StyleDataContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] D1-05~07 승인값이 Native Safe Fallback과 의미 Token 해석 계약으로 유지되는지 검증합니다.
bool FCFUIStyleDataContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] Unreal Asset 없이 Native Fallback을 검증할 Transient Style Data입니다.
	UCFUIStyleData* StyleData = NewObject<UCFUIStyleData>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient UI Style Data"), StyleData))
	{
		return false;
	}

	// [v1.0.0] Native 기본값 전체의 안전 하한 검증 결과입니다.
	FString ValidationFailureReason;
	TestTrue(TEXT("Native Style Data 안전 하한 검증"), StyleData->ValidateStyleData(ValidationFailureReason));
	TestTrue(TEXT("Native Style Data 검증 오류 없음"), ValidationFailureReason.IsEmpty());

	TestEqual(TEXT("UI Font Preset"), StyleData->Typography.UIFontFamily, FName(TEXT("Pretendard")));
	TestEqual(TEXT("Numeric Font Preset"), StyleData->Typography.NumericFontFamily, FName(TEXT("IBM Plex Mono")));
	TestEqual(TEXT("DisplayXL FontSize"), StyleData->Typography.DisplayXL.FontSize, 52);
	TestEqual(TEXT("ValueS FontSize"), StyleData->Typography.ValueS.FontSize, 20);
	TestEqual(TEXT("Caption FontSize"), StyleData->Typography.Caption.FontSize, 14);

	TestEqual(TEXT("1440p Safe Margin"), StyleData->Spacing.SafeMargin, 64.0f);
	TestEqual(TEXT("1080p HUD Outer Margin Design Unit"), StyleData->Spacing.HUDOuterMargin1080, 32.0f);
	TestEqual(TEXT("Minimum Hit Size"), StyleData->Spacing.MinimumHitSize, 48.0f);
	TestEqual(TEXT("Hairline Outline"), StyleData->Shape.OutlineHairline, 1.0f);
	TestEqual(TEXT("Focused Outline"), StyleData->Shape.OutlineFocused, 3.0f);

	TestEqual(TEXT("Critical Pulse Duration"), StyleData->Motion.CriticalPulse.DurationSeconds, 0.70f);
	TestTrue(TEXT("Critical Pulse만 반복 허용"), StyleData->Motion.CriticalPulse.bAllowLoop);
	TestFalse(TEXT("일반 State Motion 반복 금지"), StyleData->Motion.State.bAllowLoop);

	TestEqual(TEXT("Armor Visual Segment Count"), StyleData->StatusBarStyle.Armor.VisualSegmentCount, 10);
	TestTrue(TEXT("Resource Bar는 상태색 Override 허용"), StyleData->StatusBarStyle.Resource.bUseStateColor);
	TestEqual(TEXT("Alert 최대 동시 표시 수"), StyleData->AlertStyle.MaximumVisibleAlerts, 3);

	TestEqual(TEXT("Icon Base Grid"), StyleData->IconSet.BaseGridSize, 24.0f);
	TestEqual(TEXT("Weapon Compact Icon"), StyleData->IconSet.WeaponCompact, 18.0f);
		TestEqual(TEXT("Weapon Selected Icon"), StyleData->IconSet.WeaponSelected, 20.0f);

	// [v1.1.0] Semantic Icon Resolver를 Asset Load 없이 검증할 Vehicle Icon Entry입니다.
	FCFUISemanticIconEntry VehicleIconEntry;
	VehicleIconEntry.SemanticId = FName(TEXT("Vehicle"));
	VehicleIconEntry.IconAsset = TSoftObjectPtr<UObject>(FSoftObjectPath(TEXT("/Game/CarFight/UI/Icons/Semantic/T_UII_Vehicle.T_UII_Vehicle")));
	StyleData->IconSet.SemanticIcons.Add(VehicleIconEntry);

	// [v1.1.0] 등록된 Semantic ID가 정확한 Soft Object Path로 해석되는지 확인합니다.
	const TSoftObjectPtr<UObject> ResolvedVehicleIcon = StyleData->ResolveSemanticIconAsset(FName(TEXT("Vehicle")));
	TestEqual(TEXT("Vehicle Semantic Icon Path"), ResolvedVehicleIcon.ToSoftObjectPath().ToString(), FString(TEXT("/Game/CarFight/UI/Icons/Semantic/T_UII_Vehicle.T_UII_Vehicle")));
	TestTrue(TEXT("미등록 Semantic Icon은 Null"), StyleData->ResolveSemanticIconAsset(FName(TEXT("Missing"))).IsNull());

	// [v1.1.0] 유효한 단일 Semantic Icon 매핑은 전체 Style 검증을 통과해야 합니다.
	ValidationFailureReason.Reset();
	TestTrue(TEXT("Semantic Icon 단일 매핑 검증"), StyleData->ValidateStyleData(ValidationFailureReason));

	// [v1.1.0] 중복 Semantic ID를 검출하기 위한 두 번째 Vehicle Entry입니다.
	FCFUISemanticIconEntry DuplicateVehicleIconEntry = VehicleIconEntry;
	StyleData->IconSet.SemanticIcons.Add(DuplicateVehicleIconEntry);
	ValidationFailureReason.Reset();
	TestFalse(TEXT("Semantic Icon 중복 ID 검출"), StyleData->ValidateStyleData(ValidationFailureReason));
	TestTrue(TEXT("Semantic Icon 중복 ID 사유 제공"), ValidationFailureReason.Contains(TEXT("중복")));
	StyleData->IconSet.SemanticIcons.RemoveAt(1);

	// [v1.0.0] 의미 Token이 실제 승인된 Accent 색상으로 해석되는지 확인할 sRGB 값입니다.
	const FColor TacticalAccentSRGB = StyleData->ResolveColor(ECFUIColorToken::AccentTactical).ToFColorSRGB();
	TestEqual(TEXT("Accent Tactical R"), TacticalAccentSRGB.R, static_cast<uint8>(0x55));
	TestEqual(TEXT("Accent Tactical G"), TacticalAccentSRGB.G, static_cast<uint8>(0xC7));
	TestEqual(TEXT("Accent Tactical B"), TacticalAccentSRGB.B, static_cast<uint8>(0xE8));

	// [v1.0.0] Typography Role 해석 결과가 원본 Token과 일치하는지 확인합니다.
	const FCFUITypographyStyle ResolvedHeading = StyleData->ResolveTypography(ECFUITypographyRole::HeadingM);
	TestEqual(TEXT("HeadingM Role FontSize"), ResolvedHeading.FontSize, 24);
		TestEqual(TEXT("HeadingM Role LineHeight"), ResolvedHeading.LineHeight, 30);
	TestEqual(TEXT("HeadingM Role Weight"), static_cast<uint8>(ResolvedHeading.Weight), static_cast<uint8>(ECFUIFontWeight::SemiBold));

	// [v1.0.0] 접근성 하한 위반을 검출하는지 확인하기 위해 임시로 낮춘 최소 입력 크기입니다.
	StyleData->Spacing.MinimumHitSize = 40.0f;
	ValidationFailureReason.Reset();
	TestFalse(TEXT("MinimumHitSize 하한 위반 검출"), StyleData->ValidateStyleData(ValidationFailureReason));
	TestTrue(TEXT("MinimumHitSize 하한 위반 사유 제공"), !ValidationFailureReason.IsEmpty());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
