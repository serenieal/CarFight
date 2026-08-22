// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-21
// Description: CF-FQ-032 UI-P0-08 Radar Zoom Input persisted setup + read-only contract Automation
// Scope: IA_RadarZoom Axis1D와 IMC_Vehicle_Default의 MouseScrollUp/Down 명시 부호 mapping을 exact 2-asset setup으로 생성·검증합니다.
// Changelog:
// - v1.1.0: persisted Asset을 실제 저장하는 RadarZoomInputAssetSetup을 `CarFight.Setup` namespace로 이동해 broad `CarFight.UI` regression에서 mutation setup이 자동 실행되지 않도록 격리. read-only RadarZoomInputContract 경로는 유지.
// - v1.0.0: MouseScrollUp=+1 Zoom In, MouseScrollDown=-1 Zoom Out을 IA_RadarZoom Axis1D 하나로 구성하고 MouseWheelAxis·게임패드 mapping을 금지하는 setup/contract를 추가.
// Migration:
// - RadarZoomInputAssetSetup은 `CarFight.Setup.UI_P0_08.RadarZoomInputAssetSetup`으로만 명시 실행합니다. IA_RadarZoom과 IMC_Vehicle_Default만 변경하며 기존 Wheel key 충돌은 mutation 전에 fail-closed합니다.
// - RadarZoomInputContract는 Input Asset과 ACFVehiclePawn CDO 기본 참조만 읽기 전용으로 검증합니다. Provider Range 동작은 기존 RadarRangeFoundationContract가 소유합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehiclePawn.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EnhancedActionKeyMapping.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

namespace
{
	// [v1.0.0] Radar Zoom InputAction package 경로입니다.
	const TCHAR* RadarZoomInputActionPackageName = TEXT("/Game/CarFight/Input/IA_RadarZoom");

	// [v1.0.0] Radar Zoom InputAction 자산 이름입니다.
	const TCHAR* RadarZoomInputActionAssetName = TEXT("IA_RadarZoom");

	// [v1.0.0] Radar Zoom InputAction object 경로입니다.
	const TCHAR* RadarZoomInputActionPath = TEXT("/Game/CarFight/Input/IA_RadarZoom.IA_RadarZoom");

	// [v1.0.0] 차량 Gameplay Input 소유 Mapping Context object 경로입니다.
	const TCHAR* VehicleInputMappingContextPath = TEXT("/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default");

	// [v1.0.0] Radar Zoom In을 의미하는 명시적 Mouse Scroll Up key입니다.
	const FKey RadarZoomInKey = EKeys::MouseScrollUp;

	// [v1.0.0] Radar Zoom Out을 의미하는 명시적 Mouse Scroll Down key입니다.
	const FKey RadarZoomOutKey = EKeys::MouseScrollDown;

	// [v1.0.0] InputAction 또는 MappingContext의 exact package만 저장합니다.
	bool SaveRadarInputAsset(UObject* AssetObject)
	{
		if (!AssetObject)
		{
			return false;
		}

		// [v1.0.0] 저장할 자산을 소유하는 Unreal package입니다.
		UPackage* AssetPackage = AssetObject->GetOutermost();
		if (!AssetPackage)
		{
			return false;
		}

		// [v1.0.0] Long package name을 실제 .uasset 저장 경로로 변환한 값입니다.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			AssetPackage->GetName(),
			FPackageName::GetAssetPackageExtension());

		// [v1.0.0] Radar Input Asset의 public standalone 저장 인수입니다.
		FSavePackageArgs SavePackageArgs;
		SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SavePackageArgs.SaveFlags = SAVE_NoError;
		return UPackage::SavePackage(AssetPackage, AssetObject, *PackageFilename, SavePackageArgs);
	}

	// [v1.0.0] Mouse Scroll Up/Down 또는 MouseWheelAxis를 다른 Action이 이미 점유했는지 mutation 전에 검사합니다.
	bool HasRadarZoomKeyConflict(const UInputMappingContext* MappingContext, const UInputAction* ExistingRadarZoomAction)
	{
		if (!MappingContext)
		{
			return true;
		}

		for (const FEnhancedActionKeyMapping& ExistingMapping : MappingContext->GetMappings())
		{
			// [v1.0.0] 현재 mapping이 Radar Zoom 예약 Wheel 계열 key를 사용하는지 여부입니다.
			const bool bUsesReservedRadarWheelKey = ExistingMapping.Key == RadarZoomInKey
				|| ExistingMapping.Key == RadarZoomOutKey
				|| ExistingMapping.Key == EKeys::MouseWheelAxis;
			if (bUsesReservedRadarWheelKey && ExistingMapping.Action != ExistingRadarZoomAction)
			{
				return true;
			}
		}
		return false;
	}

	// [v1.0.0] mapping의 Axis1D X에 적용되는 Scalar 곱과 예상하지 않은 modifier 존재 여부를 계산합니다.
	float ResolveRadarZoomScale(const FEnhancedActionKeyMapping& InputMapping, bool& bOutHasUnexpectedModifier)
	{
		bOutHasUnexpectedModifier = false;

		// [v1.0.0] modifier가 없을 때 digital Mouse Scroll key가 전달하는 기본 Axis1D 값입니다.
		float EffectiveScale = 1.0f;
		for (const UInputModifier* InputModifier : InputMapping.Modifiers)
		{
			// [v1.0.0] Radar Zoom 방향 부호를 명시할 때 유일하게 허용하는 Scalar modifier입니다.
			const UInputModifierScalar* ScalarModifier = Cast<UInputModifierScalar>(InputModifier);
			if (!ScalarModifier)
			{
				bOutHasUnexpectedModifier = true;
				continue;
			}

			EffectiveScale *= static_cast<float>(ScalarModifier->Scalar.X);
		}
		return EffectiveScale;
	}

	// [v1.0.0] IA_RadarZoom Axis1D와 Up=+1/Down=-1 mapping을 idempotent하게 생성·보정하고 정확히 두 package만 저장합니다.
	bool EnsureRadarZoomInputAssets()
	{
		// [v1.0.0] 기존 차량 Gameplay Mapping Context입니다.
		UInputMappingContext* VehicleInputMappingContext = LoadObject<UInputMappingContext>(nullptr, VehicleInputMappingContextPath);
		if (!VehicleInputMappingContext)
		{
			return false;
		}

		// [v1.0.0] 이미 존재하면 재사용하고 없으면 생성할 Radar Zoom InputAction입니다.
		UInputAction* RadarZoomInputAction = LoadObject<UInputAction>(nullptr, RadarZoomInputActionPath);
		if (HasRadarZoomKeyConflict(VehicleInputMappingContext, RadarZoomInputAction))
		{
			return false;
		}

		// [v1.0.0] InputAction package에 실제 변경이 발생했는지 여부입니다.
		bool bInputActionChanged = false;
		if (!RadarZoomInputAction)
		{
			// [v1.0.0] 새 IA_RadarZoom을 소유할 Content package입니다.
			UPackage* RadarZoomPackage = CreatePackage(RadarZoomInputActionPackageName);
			if (!RadarZoomPackage)
			{
				return false;
			}

			RadarZoomInputAction = NewObject<UInputAction>(
				RadarZoomPackage,
				FName(RadarZoomInputActionAssetName),
				RF_Public | RF_Standalone | RF_Transactional);
			if (!RadarZoomInputAction)
			{
				return false;
			}
			FAssetRegistryModule::AssetCreated(RadarZoomInputAction);
			bInputActionChanged = true;
		}

		if (RadarZoomInputAction->ValueType != EInputActionValueType::Axis1D)
		{
			RadarZoomInputAction->ValueType = EInputActionValueType::Axis1D;
			bInputActionChanged = true;
		}
		if (!RadarZoomInputAction->bConsumeInput)
		{
			RadarZoomInputAction->bConsumeInput = true;
			bInputActionChanged = true;
		}

		VehicleInputMappingContext->Modify();
		VehicleInputMappingContext->UnmapAllKeysFromAction(RadarZoomInputAction);

		// [v1.0.0] Mouse Scroll Up은 modifier 없이 기본 +1을 사용해 작은 Display Range로 Zoom In합니다.
		VehicleInputMappingContext->MapKey(RadarZoomInputAction, RadarZoomInKey);

		// [v1.0.0] Mouse Scroll Down mapping에 명시적 -1 Scalar를 연결하기 위해 생성한 mapping입니다.
		FEnhancedActionKeyMapping& RadarZoomOutMapping = VehicleInputMappingContext->MapKey(RadarZoomInputAction, RadarZoomOutKey);

		// [v1.0.0] Mouse Scroll Down의 기본 +1을 Zoom Out 의미인 -1로 바꾸는 Axis1D Scalar modifier입니다.
		UInputModifierScalar* ZoomOutScalarModifier = NewObject<UInputModifierScalar>(VehicleInputMappingContext);
		if (!ZoomOutScalarModifier)
		{
			return false;
		}
		ZoomOutScalarModifier->Scalar = FVector(-1.0, 1.0, 1.0);
		RadarZoomOutMapping.Modifiers.Add(ZoomOutScalarModifier);

		if (bInputActionChanged)
		{
			RadarZoomInputAction->MarkPackageDirty();
		}
		VehicleInputMappingContext->MarkPackageDirty();

		if ((bInputActionChanged || RadarZoomInputAction->GetOutermost()->IsDirty())
			&& !SaveRadarInputAsset(RadarZoomInputAction))
		{
			return false;
		}
		return SaveRadarInputAsset(VehicleInputMappingContext);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRadarZoomInputAssetSetupTest,
	"CarFight.Setup.UI_P0_08.RadarZoomInputAssetSetup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] conflict preflight 뒤 Production IA_RadarZoom과 IMC Mouse Scroll Up/Down mapping만 생성·보정·저장합니다.
bool FCFRadarZoomInputAssetSetupTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	TestTrue(TEXT("UI-P0-08 Radar Zoom Input Asset setup"), EnsureRadarZoomInputAssets());
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRadarZoomInputContractTest,
	"CarFight.UI.UI_P0_08.RadarZoomInputContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 저장 IA/IMC가 Up=+1 Zoom In, Down=-1 Zoom Out의 exact 의미와 Pawn 기본 참조를 갖는지 검증합니다.
bool FCFRadarZoomInputContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 실제 저장 IA_RadarZoom입니다.
	UInputAction* RadarZoomInputAction = LoadObject<UInputAction>(nullptr, RadarZoomInputActionPath);
	TestNotNull(TEXT("UI-P0-08 IA_RadarZoom 저장 자산"), RadarZoomInputAction);
	if (!RadarZoomInputAction)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-08 IA_RadarZoom Axis1D"), RadarZoomInputAction->ValueType, EInputActionValueType::Axis1D);
	TestTrue(TEXT("UI-P0-08 IA_RadarZoom consume input"), RadarZoomInputAction->bConsumeInput);

	// [v1.0.0] 실제 저장 IMC_Vehicle_Default입니다.
	UInputMappingContext* VehicleInputMappingContext = LoadObject<UInputMappingContext>(nullptr, VehicleInputMappingContextPath);
	TestNotNull(TEXT("UI-P0-08 IMC_Vehicle_Default 저장 자산"), VehicleInputMappingContext);
	if (!VehicleInputMappingContext)
	{
		return false;
	}

	// [v1.0.0] Mouse Scroll Up mapping 발견 여부입니다.
	bool bFoundZoomInMapping = false;
	// [v1.0.0] Mouse Scroll Down mapping 발견 여부입니다.
	bool bFoundZoomOutMapping = false;
	// [v1.0.0] IA_RadarZoom에 연결된 전체 mapping 수입니다.
	int32 RadarZoomMappingCount = 0;
	for (const FEnhancedActionKeyMapping& InputMapping : VehicleInputMappingContext->GetMappings())
	{
		if (InputMapping.Action != RadarZoomInputAction)
		{
			continue;
		}

		++RadarZoomMappingCount;
		TestFalse(TEXT("UI-P0-08 Radar Zoom gamepad mapping 없음"), InputMapping.Key.IsGamepadKey());
		TestFalse(TEXT("UI-P0-08 MouseWheelAxis 암묵 부호 mapping 없음"), InputMapping.Key == EKeys::MouseWheelAxis);

		// [v1.0.0] 현재 mapping modifier가 만드는 실제 Axis1D 방향 부호입니다.
		bool bHasUnexpectedModifier = false;
		// [v1.0.0] 현재 mapping이 전달할 최종 Axis1D scale입니다.
		const float EffectiveScale = ResolveRadarZoomScale(InputMapping, bHasUnexpectedModifier);
		TestFalse(TEXT("UI-P0-08 Radar Zoom unexpected modifier 없음"), bHasUnexpectedModifier);

		if (InputMapping.Key == RadarZoomInKey)
		{
			TestFalse(TEXT("UI-P0-08 MouseScrollUp 중복 없음"), bFoundZoomInMapping);
			bFoundZoomInMapping = true;
			TestTrue(TEXT("UI-P0-08 MouseScrollUp = +1 Zoom In"), FMath::IsNearlyEqual(EffectiveScale, 1.0f, KINDA_SMALL_NUMBER));
		}
		else if (InputMapping.Key == RadarZoomOutKey)
		{
			TestFalse(TEXT("UI-P0-08 MouseScrollDown 중복 없음"), bFoundZoomOutMapping);
			bFoundZoomOutMapping = true;
			TestTrue(TEXT("UI-P0-08 MouseScrollDown = -1 Zoom Out"), FMath::IsNearlyEqual(EffectiveScale, -1.0f, KINDA_SMALL_NUMBER));
		}
		else
		{
			AddError(TEXT("UI-P0-08 IA_RadarZoom은 MouseScrollUp/Down 외 key를 사용하면 안 됩니다."));
		}
	}

	TestEqual(TEXT("UI-P0-08 Radar Zoom mapping 정확히 2개"), RadarZoomMappingCount, 2);
	TestTrue(TEXT("UI-P0-08 MouseScrollUp mapping 존재"), bFoundZoomInMapping);
	TestTrue(TEXT("UI-P0-08 MouseScrollDown mapping 존재"), bFoundZoomOutMapping);

	for (const FEnhancedActionKeyMapping& ExistingMapping : VehicleInputMappingContext->GetMappings())
	{
		// [v1.0.0] Wheel 예약 key가 RadarZoom 외 Action에 중복 연결됐는지 여부입니다.
		const bool bUsesRadarWheelKey = ExistingMapping.Key == RadarZoomInKey
			|| ExistingMapping.Key == RadarZoomOutKey
			|| ExistingMapping.Key == EKeys::MouseWheelAxis;
		if (bUsesRadarWheelKey)
		{
			TestTrue(TEXT("UI-P0-08 Wheel key owner는 IA_RadarZoom 단일"), ExistingMapping.Action == RadarZoomInputAction);
		}
	}

	// [v1.0.0] 파생 BP가 명시 override하지 않을 때 IA_RadarZoom을 기본 로드하는 C++ VehiclePawn CDO입니다.
	const ACFVehiclePawn* VehiclePawnClassDefault = GetDefault<ACFVehiclePawn>();
	TestNotNull(TEXT("UI-P0-08 VehiclePawn CDO"), VehiclePawnClassDefault);
	if (VehiclePawnClassDefault)
	{
		TestEqual(TEXT("UI-P0-08 Pawn 기본 IA_RadarZoom 참조"), VehiclePawnClassDefault->InputAction_RadarZoom.Get(), RadarZoomInputAction);
	}

	return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
