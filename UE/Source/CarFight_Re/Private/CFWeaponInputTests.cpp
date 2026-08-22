// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-21
// Description: CF-FQ-032 UI-P0-06 Player-facing Weapon Select Input persisted setup + contract Automation
// Scope: IA_SelectWeapon과 IMC_Vehicle_Default의 숫자 1~9 ordinal mapping을 idempotent하게 생성·저장하는 setup 1건과 이후 read-only exact contract를 분리합니다.
// Changelog:
// - v1.2.0: persisted Asset을 실제 저장하는 WeaponSelectInputAssetSetup을 `CarFight.Setup` namespace로 이동해 broad `CarFight.UI` regression에서 mutation setup이 자동 실행되지 않도록 격리. read-only WeaponSelectInputContract 경로는 유지.
// - v1.1.0: fresh key-conflict preflight 후 IA_SelectWeapon Axis1D와 숫자 1~9 direct ordinal mapping만 생성·보정·저장하는 WeaponSelectInputAssetSetup 추가. Mouse Wheel·게임패드·기존 다른 mapping 변경0.
// - v1.0.0: Axis1D IA_SelectWeapon + keyboard 1~9 direct ordinal mapping과 Radar Zoom용 Mouse Wheel 비점유 exact contract 추가.
// Migration:
// - WeaponSelectInputAssetSetup은 `CarFight.Setup.UI_P0_06.WeaponSelectInputAssetSetup`으로만 명시 실행합니다. 현재 P0 Production Input Asset 제작 진입으로 정확히 IA_SelectWeapon + IMC_Vehicle_Default만 변경하며 기존 숫자키 충돌이 발견되면 mutation 전 fail-closed합니다.
// - WeaponSelectInputContract는 기존 Weapon Selection Runtime/HUD Source 또는 truthful Rail lifecycle을 재검증하지 않고 Input Asset 계약만 읽기 전용으로 검증합니다.

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
	// [v1.1.0] 실제 Player-facing Weapon Select InputAction package 경로입니다.
	const TCHAR* WeaponSelectInputActionPackageName = TEXT("/Game/CarFight/Input/IA_SelectWeapon");

	// [v1.1.0] 실제 Player-facing Weapon Select InputAction 자산 이름입니다.
	const TCHAR* WeaponSelectInputActionAssetName = TEXT("IA_SelectWeapon");

	// [v1.0.0] 실제 Player-facing Weapon Select InputAction 저장 경로입니다.
	const TCHAR* WeaponSelectInputActionPath = TEXT("/Game/CarFight/Input/IA_SelectWeapon.IA_SelectWeapon");

	// [v1.0.0] 차량 Gameplay Input 소유 Mapping Context 저장 경로입니다.
	const TCHAR* VehicleInputMappingContextPath = TEXT("/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default");

	// [v1.1.0] Player-facing truthful Rail ordinal과 직접 대응하는 P0 숫자키 1~9입니다.
	const FKey WeaponSelectOrdinalKeys[] =
	{
		EKeys::One,
		EKeys::Two,
		EKeys::Three,
		EKeys::Four,
		EKeys::Five,
		EKeys::Six,
		EKeys::Seven,
		EKeys::Eight,
		EKeys::Nine
	};

	// [v1.1.0] InputAction 또는 MappingContext의 exact package만 저장합니다.
	bool SaveWeaponInputAsset(UObject* AssetObject)
	{
		if (!AssetObject)
		{
			return false;
		}

		// [v1.1.0] 저장할 자산을 소유하는 Unreal package입니다.
		UPackage* AssetPackage = AssetObject->GetOutermost();
		if (!AssetPackage)
		{
			return false;
		}

		// [v1.1.0] Long package name을 실제 .uasset 저장 경로로 변환한 값입니다.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(AssetPackage->GetName(), FPackageName::GetAssetPackageExtension());

		// [v1.1.0] Production Input Asset의 public standalone 저장 인수입니다.
		FSavePackageArgs SavePackageArgs;
		SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SavePackageArgs.SaveFlags = SAVE_NoError;
		return UPackage::SavePackage(AssetPackage, AssetObject, *PackageFilename, SavePackageArgs);
	}

	// [v1.1.0] 숫자키 1~9 중 다른 Action이 이미 점유한 키가 있는지 mutation 전에 검사합니다.
	bool HasWeaponSelectKeyConflict(const UInputMappingContext* MappingContext, const UInputAction* ExistingWeaponSelectAction)
	{
		if (!MappingContext)
		{
			return true;
		}

		for (const FEnhancedActionKeyMapping& ExistingMapping : MappingContext->GetMappings())
		{
			for (const FKey& WeaponSelectOrdinalKey : WeaponSelectOrdinalKeys)
			{
				if (ExistingMapping.Key == WeaponSelectOrdinalKey && ExistingMapping.Action != ExistingWeaponSelectAction)
				{
					return true;
				}
			}
		}
		return false;
	}

	// [v1.1.0] IA_SelectWeapon Axis1D와 숫자키 1~9 ordinal mapping을 idempotent하게 생성·보정하고 정확히 두 package만 저장합니다.
	bool EnsureWeaponSelectInputAssets()
	{
		// [v1.1.0] 기존 차량 Gameplay Mapping Context입니다.
		UInputMappingContext* VehicleInputMappingContext = LoadObject<UInputMappingContext>(nullptr, VehicleInputMappingContextPath);
		if (!VehicleInputMappingContext)
		{
			return false;
		}

		// [v1.1.0] 이미 존재하면 재사용하고 없으면 생성할 Weapon Select InputAction입니다.
		UInputAction* WeaponSelectInputAction = LoadObject<UInputAction>(nullptr, WeaponSelectInputActionPath);
		if (HasWeaponSelectKeyConflict(VehicleInputMappingContext, WeaponSelectInputAction))
		{
			return false;
		}

		// [v1.1.0] InputAction package에 실제 변경이 발생했는지 여부입니다.
		bool bInputActionChanged = false;
		if (!WeaponSelectInputAction)
		{
			// [v1.1.0] 새 IA_SelectWeapon을 소유할 Content package입니다.
			UPackage* WeaponSelectPackage = CreatePackage(WeaponSelectInputActionPackageName);
			if (!WeaponSelectPackage)
			{
				return false;
			}

			WeaponSelectInputAction = NewObject<UInputAction>(WeaponSelectPackage, FName(WeaponSelectInputActionAssetName), RF_Public | RF_Standalone | RF_Transactional);
			if (!WeaponSelectInputAction)
			{
				return false;
			}
			FAssetRegistryModule::AssetCreated(WeaponSelectInputAction);
			bInputActionChanged = true;
		}

		if (WeaponSelectInputAction->ValueType != EInputActionValueType::Axis1D)
		{
			WeaponSelectInputAction->ValueType = EInputActionValueType::Axis1D;
			bInputActionChanged = true;
		}
		if (!WeaponSelectInputAction->bConsumeInput)
		{
			WeaponSelectInputAction->bConsumeInput = true;
			bInputActionChanged = true;
		}

		VehicleInputMappingContext->Modify();
		VehicleInputMappingContext->UnmapAllKeysFromAction(WeaponSelectInputAction);
		for (int32 KeyIndex = 0; KeyIndex < UE_ARRAY_COUNT(WeaponSelectOrdinalKeys); ++KeyIndex)
		{
			// [v1.1.0] 현재 숫자키 mapping이 전달해야 하는 1-based ordinal 값입니다.
			const int32 WeaponOrdinal = KeyIndex + 1;
			// [v1.1.0] 현재 ordinal 숫자키에 새로 생성한 Enhanced Input mapping입니다.
			FEnhancedActionKeyMapping& WeaponOrdinalMapping = VehicleInputMappingContext->MapKey(WeaponSelectInputAction, WeaponSelectOrdinalKeys[KeyIndex]);
			if (WeaponOrdinal > 1)
			{
				// [v1.1.0] 기본 digital value 1을 실제 2~9 ordinal로 바꾸는 Axis1D Scalar modifier입니다.
				UInputModifierScalar* OrdinalScalarModifier = NewObject<UInputModifierScalar>(VehicleInputMappingContext);
				if (!OrdinalScalarModifier)
				{
					return false;
				}
				OrdinalScalarModifier->Scalar = FVector(static_cast<double>(WeaponOrdinal), 1.0, 1.0);
				WeaponOrdinalMapping.Modifiers.Add(OrdinalScalarModifier);
			}
		}

		if (bInputActionChanged)
		{
			WeaponSelectInputAction->MarkPackageDirty();
		}
		VehicleInputMappingContext->MarkPackageDirty();

		if ((bInputActionChanged || WeaponSelectInputAction->GetOutermost()->IsDirty()) && !SaveWeaponInputAsset(WeaponSelectInputAction))
		{
			return false;
		}
		return SaveWeaponInputAsset(VehicleInputMappingContext);
	}

	// [v1.0.0] 지정 mapping에서 Axis1D ordinal에 영향을 주는 Scalar X의 곱을 구하고 예상하지 않은 modifier 존재 여부를 반환합니다.
	float ResolveWeaponOrdinalScale(const FEnhancedActionKeyMapping& InputMapping, bool& bOutHasUnexpectedModifier)
	{
		bOutHasUnexpectedModifier = false;

		// [v1.0.0] Modifier가 없을 때 숫자 1 키가 전달하는 기본 Axis1D 값입니다.
		float EffectiveOrdinalScale = 1.0f;
		for (const UInputModifier* InputModifier : InputMapping.Modifiers)
		{
			// [v1.0.0] 숫자 2~9가 직접 ordinal 값을 만들 때 허용하는 Scalar modifier입니다.
			const UInputModifierScalar* ScalarModifier = Cast<UInputModifierScalar>(InputModifier);
			if (!ScalarModifier)
			{
				bOutHasUnexpectedModifier = true;
				continue;
			}

			EffectiveOrdinalScale *= static_cast<float>(ScalarModifier->Scalar.X);
		}
		return EffectiveOrdinalScale;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWeaponSelectInputAssetSetupTest,
	"CarFight.Setup.UI_P0_06.WeaponSelectInputAssetSetup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.1.0] fresh conflict preflight 뒤 Production IA_SelectWeapon과 IMC 숫자 1~9 mapping만 생성·보정·저장합니다.
bool FCFWeaponSelectInputAssetSetupTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	TestTrue(TEXT("UI-P0-06 Weapon Select Input Asset setup"), EnsureWeaponSelectInputAssets());
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWeaponSelectInputContractTest,
	"CarFight.UI.UI_P0_06.WeaponSelectInputContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 저장 InputAction/IMC가 truthful Weapon Rail ordinal과 같은 1~9 직접 선택 의미를 갖는지 검증합니다.
bool FCFWeaponSelectInputContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 실제 저장 IA_SelectWeapon입니다.
	UInputAction* WeaponSelectInputAction = LoadObject<UInputAction>(nullptr, WeaponSelectInputActionPath);
	TestNotNull(TEXT("UI-P0-06 IA_SelectWeapon 저장 자산"), WeaponSelectInputAction);
	if (!WeaponSelectInputAction)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 IA_SelectWeapon Axis1D"), WeaponSelectInputAction->ValueType, EInputActionValueType::Axis1D);

	// [v1.0.0] 기존 차량 Gameplay Mapping Context입니다.
	UInputMappingContext* VehicleInputMappingContext = LoadObject<UInputMappingContext>(nullptr, VehicleInputMappingContextPath);
	TestNotNull(TEXT("UI-P0-06 IMC_Vehicle_Default 저장 자산"), VehicleInputMappingContext);
	if (!VehicleInputMappingContext)
	{
		return false;
	}

	// [v1.0.0] truthful Rail 원본 1-based ordinal과 직접 대응하는 키보드 숫자키 목록입니다.
	const FKey ExpectedWeaponKeys[] =
	{
		EKeys::One,
		EKeys::Two,
		EKeys::Three,
		EKeys::Four,
		EKeys::Five,
		EKeys::Six,
		EKeys::Seven,
		EKeys::Eight,
		EKeys::Nine
	};

	// [v1.0.0] 각 숫자키가 정확히 한 번 발견됐는지 기록하는 9-slot 배열입니다.
	bool bFoundExpectedWeaponKey[UE_ARRAY_COUNT(ExpectedWeaponKeys)] = {};
	// [v1.0.0] IA_SelectWeapon에 연결된 저장 mapping 총 개수입니다.
	int32 WeaponSelectMappingCount = 0;
	for (const FEnhancedActionKeyMapping& InputMapping : VehicleInputMappingContext->GetMappings())
	{
		if (InputMapping.Action != WeaponSelectInputAction)
		{
			continue;
		}

		++WeaponSelectMappingCount;
		// [v1.0.0] 현재 mapping이 기대하는 숫자키 중 몇 번째인지 나타내는 인덱스입니다.
		int32 ExpectedKeyIndex = INDEX_NONE;
		for (int32 KeyIndex = 0; KeyIndex < UE_ARRAY_COUNT(ExpectedWeaponKeys); ++KeyIndex)
		{
			if (InputMapping.Key == ExpectedWeaponKeys[KeyIndex])
			{
				ExpectedKeyIndex = KeyIndex;
				break;
			}
		}

		TestTrue(TEXT("UI-P0-06 Weapon Select mapping은 숫자 1~9만 사용"), ExpectedKeyIndex != INDEX_NONE);
		if (ExpectedKeyIndex == INDEX_NONE)
		{
			continue;
		}

		TestFalse(TEXT("UI-P0-06 같은 Weapon Select 숫자키 중복 없음"), bFoundExpectedWeaponKey[ExpectedKeyIndex]);
		bFoundExpectedWeaponKey[ExpectedKeyIndex] = true;

		// [v1.0.0] Scalar 외 modifier가 ordinal 의미를 변형하는지 판정할 결과입니다.
		bool bHasUnexpectedModifier = false;
		// [v1.0.0] 해당 숫자키가 실제 Axis1D로 전달할 1-based ordinal 값입니다.
		const float EffectiveOrdinalScale = ResolveWeaponOrdinalScale(InputMapping, bHasUnexpectedModifier);
		TestFalse(TEXT("UI-P0-06 Weapon Select mapping unexpected modifier 없음"), bHasUnexpectedModifier);
		TestTrue(
			TEXT("UI-P0-06 숫자키 ordinal 값 일치"),
			FMath::IsNearlyEqual(EffectiveOrdinalScale, static_cast<float>(ExpectedKeyIndex + 1), KINDA_SMALL_NUMBER));
	}

	TestEqual(TEXT("UI-P0-06 Weapon Select 숫자키 mapping 정확히 9개"), WeaponSelectMappingCount, 9);
	for (int32 KeyIndex = 0; KeyIndex < UE_ARRAY_COUNT(ExpectedWeaponKeys); ++KeyIndex)
	{
		TestTrue(TEXT("UI-P0-06 Weapon Select 숫자키 1~9 모두 존재"), bFoundExpectedWeaponKey[KeyIndex]);
	}

	// [v1.0.0] Radar Range/Zoom에 예약한 Mouse Wheel이 Weapon Select Action에 잘못 연결됐는지 여부입니다.
	bool bWeaponSelectUsesMouseWheel = false;
	for (const FEnhancedActionKeyMapping& InputMapping : VehicleInputMappingContext->GetMappings())
	{
		if (InputMapping.Action == WeaponSelectInputAction
			&& (InputMapping.Key == EKeys::MouseWheelAxis
				|| InputMapping.Key == EKeys::MouseScrollUp
				|| InputMapping.Key == EKeys::MouseScrollDown))
		{
			bWeaponSelectUsesMouseWheel = true;
			break;
		}
	}
	TestFalse(TEXT("UI-P0-06 Weapon Select Mouse Wheel 비점유"), bWeaponSelectUsesMouseWheel);

	// [v1.0.0] 파생 BP가 명시 override하지 않을 때 새 Action을 기본 로드하는 C++ VehiclePawn CDO입니다.
	const ACFVehiclePawn* VehiclePawnClassDefault = GetDefault<ACFVehiclePawn>();
	TestNotNull(TEXT("UI-P0-06 VehiclePawn CDO"), VehiclePawnClassDefault);
	if (VehiclePawnClassDefault)
	{
		TestEqual(TEXT("UI-P0-06 Pawn 기본 IA_SelectWeapon 참조"), VehiclePawnClassDefault->InputAction_SelectWeapon.Get(), WeaponSelectInputAction);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
