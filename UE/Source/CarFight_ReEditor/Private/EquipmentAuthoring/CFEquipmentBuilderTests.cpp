// Copyright (c) CarFight. All Rights Reserved.
// File: CFEquipmentBuilderTests.cpp
// Version: v1.5.2
// Date: 2026-09-17
// Description: CF-FQ-054 EBA-P0-05 Guided Equipment Builder USER UX 회귀를 포함한 focused Automation입니다.
// Changelog:
// - v1.5.2: AutoIdentity 회귀에 신규 Draft 초안 되돌리기 시 asset name/EquipmentId/자동 target exact3 초기화를 추가.
// - v1.5.1: ExistingModeLocked 회귀의 초기화/로드 bool 변수에 역할 주석을 명시해 CarFight C++ 주석 규칙과 정렬.
// - v1.5.0: 신규 에셋 이름→EquipmentId/canonical target 자동 파생과 기존 EquipmentPreset 장비 종류 변경 금지를 P005 회귀에 추가.
// - v1.4.0: EBA-P0-05 USER UX 회귀로 Scanner Utility/None 강제와 Weapon 복귀 시 mount type/size/reference 복원을 focused P005에 추가.
// - v1.3.0: persisted clean VehicleData read-only mount discovery, exact3 freshness, Weapon/Scanner compatibility, incompatibility reasons, durable Review isolation과 Cross-Builder navigation registration을 추가.
// - v1.2.0: deterministic Weapon/Scanner selector+validation success, native child failure propagation, intrinsic mount contradiction과 mode-selection restore 회귀를 추가.
// - v1.1.0: mixed payload fail-closed, Scanner size contract, wrong-mode selector isolation과 persisted child validation regression을 추가.
// - v1.0.0: new draft mutation0 semantics, metadata-only browser projection, Equipment Builder Nomad Tab registration/creation을 검증.
// Migration:
// - 테스트는 Product Asset을 생성/수정/저장하지 않습니다.
// - v1.2.0+ deterministic Equipment fixtures는 GetTransientPackage()의 session-local UObject만 사용하고 SavePackage를 호출하지 않습니다.
// - v1.3.0 VehicleData는 persisted clean Product source를 read-only load만 하며 VehicleData/Fitting/RuntimeApply mutation/execution은 0입니다.
// - persisted Asset 접근은 read-only로만 유지합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "EquipmentAuthoring/CFEquipmentBuilderVM.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "CFTurretMountData.h"
#include "CFVehicleData.h"
#include "CFVehicleSensorData.h"
#include "CFWeaponData.h"
#include "Framework/Docking/TabManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SNullWidget.h"

namespace
{
	// P0-04 focused test가 찾을 persisted Mount 종류입니다.
	enum class ECFEquipmentTestMountKind : uint8
	{
		AnyConcrete,
		Weapon,
		Utility,
		WeaponBelowLarge
	};

	// persisted clean VehicleData에서 unique concrete MountProfile 하나를 read-only로 찾습니다.
	bool FindPersistedVehicleMount(
		const ECFEquipmentTestMountKind MountKind,
		FSoftObjectPath& OutVehicleDataPath,
		FCFVehicleMountProfile& OutMountProfile,
		FString& OutError)
	{
		// current editor Asset Registry authority입니다.
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		// exact native VehicleData만 찾는 read-only filter입니다.
		FARFilter Filter;
		Filter.ClassPaths.Add(UCFVehicleData::StaticClass()->GetClassPathName());
		Filter.PackagePaths.Add(FName(TEXT("/Game/CarFight")));
		Filter.bRecursivePaths = true;
		Filter.bRecursiveClasses = false;
		// persisted VehicleData metadata 후보입니다.
		TArray<FAssetData> VehicleAssets;
		AssetRegistry.GetAssets(Filter, VehicleAssets);

		for (const FAssetData& VehicleAssetData : VehicleAssets)
		{
			// read-only exact VehicleData candidate입니다.
			UCFVehicleData* VehicleData = Cast<UCFVehicleData>(VehicleAssetData.GetAsset());
			if (VehicleData == nullptr || VehicleData->GetClass() != UCFVehicleData::StaticClass())
			{
				continue;
			}
			// candidate VehicleData의 persisted package입니다.
			UPackage* VehiclePackage = VehicleData->GetOutermost();
			if (VehiclePackage == nullptr || VehiclePackage->IsDirty() || !FPackageName::DoesPackageExist(VehiclePackage->GetName()))
			{
				continue;
			}

			for (const FCFVehicleMountProfile& MountProfile : VehicleData->MountProfiles)
			{
				if (MountProfile.MountProfileId.IsNone() || MountProfile.MountType == ECFVehicleMountType::None)
				{
					continue;
				}
				// current VehicleData 안에서 selected MountProfileId의 occurrence 수입니다.
				int32 MatchingIdCount = 0;
				for (const FCFVehicleMountProfile& CandidateMount : VehicleData->MountProfiles)
				{
					MatchingIdCount += CandidateMount.MountProfileId == MountProfile.MountProfileId ? 1 : 0;
				}
				if (MatchingIdCount != 1)
				{
					continue;
				}

				// requested test category에 현재 Mount가 맞는지 나타냅니다.
				bool bMatchesKind = false;
				switch (MountKind)
				{
				case ECFEquipmentTestMountKind::AnyConcrete:
					bMatchesKind = true;
					break;
				case ECFEquipmentTestMountKind::Weapon:
					bMatchesKind = MountProfile.MountType != ECFVehicleMountType::Utility
						&& MountProfile.SizeLimit != ECFVehicleWeaponSize::None;
					break;
				case ECFEquipmentTestMountKind::Utility:
					bMatchesKind = MountProfile.MountType == ECFVehicleMountType::Utility;
					break;
				case ECFEquipmentTestMountKind::WeaponBelowLarge:
					bMatchesKind = MountProfile.MountType != ECFVehicleMountType::Utility
						&& MountProfile.SizeLimit != ECFVehicleWeaponSize::None
						&& MountProfile.SizeLimit != ECFVehicleWeaponSize::Large;
					break;
				default:
					break;
				}
				if (!bMatchesKind)
				{
					continue;
				}

				OutVehicleDataPath = VehicleAssetData.GetSoftObjectPath();
				OutMountProfile = MountProfile;
				OutError.Reset();
				return true;
			}
		}

		OutError = FString::Printf(TEXT("요구한 P0-04 persisted clean Vehicle Mount fixture를 찾지 못했습니다. kind=%d"), static_cast<int32>(MountKind));
		return false;
	}

	// persisted Vehicle Mount에 맞는 valid transient Weapon Equipment Draft를 구성합니다.
	bool ConfigureTransientWeaponDraft(
		FCFEquipmentBuilderVM& ViewModel,
		const FCFVehicleMountProfile& MountProfile,
		UCFWeaponData*& OutWeaponData,
		const ECFVehicleMountType AdditionalSupportedMount,
		FString& OutError)
	{
		ViewModel.BeginNewPreset();
		ViewModel.SetEquipmentId(TEXT("EBA_P004_Weapon"));
		ViewModel.SetDisplayName(FText::FromString(TEXT("P004 호환성 테스트 무장")));
		ViewModel.SetRequiredMountType(ECFVehicleMountType::None);
		ViewModel.SetRequiredWeaponSize(ECFVehicleWeaponSize::None);

		// test-only transient TurretMountData입니다.
		UCFTurretMountData* TurretMountData = NewObject<UCFTurretMountData>(GetTransientPackage());
		// test-only transient WeaponData입니다.
		UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
		WeaponData->CompatibleMountTypes.Reset();
		WeaponData->CompatibleMountTypes.Add(MountProfile.MountType);
		if (AdditionalSupportedMount != ECFVehicleMountType::None && AdditionalSupportedMount != MountProfile.MountType)
		{
			WeaponData->CompatibleMountTypes.Add(AdditionalSupportedMount);
		}
		WeaponData->WeaponSize = MountProfile.SizeLimit;
		OutWeaponData = WeaponData;

		if (!ViewModel.SetTurretMountDataPath(FSoftObjectPath(TurretMountData), OutError))
		{
			return false;
		}
		return ViewModel.SetWeaponDataPath(FSoftObjectPath(WeaponData), OutError);
	}

	// target Mount와 다른 concrete Weapon mount type 하나를 반환합니다.
	ECFVehicleMountType ChooseDifferentWeaponMountType(const ECFVehicleMountType MountType)
	{
		// P0 Weapon mount 후보입니다.
		const ECFVehicleMountType Candidates[] = {
			ECFVehicleMountType::Fixed,
			ECFVehicleMountType::Gimbal,
			ECFVehicleMountType::Turret,
			ECFVehicleMountType::Launcher
		};
		for (const ECFVehicleMountType Candidate : Candidates)
		{
			if (Candidate != MountType)
			{
				return Candidate;
			}
		}
		return ECFVehicleMountType::Turret;
	}

	// validation 결과에 지정 severity/title 항목이 존재하는지 확인합니다.
	bool HasValidationItem(
		const FCFEquipmentDraftValidation& Validation,
		const ECFEquipmentValidationSeverity Severity,
		const FString& Title)
	{
		// 현재 validation에서 severity/title을 비교할 항목입니다.
		for (const FCFEquipmentValidationItem& Item : Validation.Items)
		{
			if (Item.Severity == Severity && Item.Title == Title)
			{
				return true;
			}
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentDraftModelTest,
	"CarFight.EquipmentAuthoring.P001.DraftModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// 신규 draft가 Product UObject 없이 transient state와 scanner size contract만 관리하는지 검증합니다.
bool FCFEquipmentDraftModelTest::RunTest(const FString& Parameters)
{
	// focused test용 session-local ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	ViewModel.BeginNewPreset();

	TestTrue(TEXT("New draft is active"), ViewModel.HasActiveDraft());
	TestTrue(TEXT("New draft is marked new"), ViewModel.IsNewDraft());
	TestTrue(TEXT("New draft has no persisted target"), ViewModel.GetSelectedObjectPath().IsEmpty());
	TestFalse(TEXT("Blank new draft starts presentation-clean"), ViewModel.HasDraftChanges());

	ViewModel.SetEquipmentId(TEXT("Test_Scanner_Kit"));
	ViewModel.SetDisplayName(FText::FromString(TEXT("테스트 스캐너")));
	ViewModel.SetDraftMode(ECFEquipmentDraftMode::Scanner);
	ViewModel.SetRequiredMountType(ECFVehicleMountType::Utility);

	TestTrue(TEXT("Transient edits mark draft dirty"), ViewModel.HasDraftChanges());
	TestEqual(TEXT("Scanner draft forces RequiredWeaponSize None"), ViewModel.GetDraft().RequiredWeaponSize, ECFVehicleWeaponSize::None);
	TestEqual(TEXT("Draft EquipmentId changed only in transient state"), ViewModel.GetDraft().EquipmentId, FName(TEXT("Test_Scanner_Kit")));

	ViewModel.ResetDraftToSource();
	TestFalse(TEXT("Reset returns new draft to initial transient snapshot"), ViewModel.HasDraftChanges());
	TestEqual(TEXT("Reset restores Weapon UI mode"), ViewModel.GetDraft().DraftMode, ECFEquipmentDraftMode::Weapon);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentBrowserTest,
	"CarFight.EquipmentAuthoring.P001.BrowserInventory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Data Asset Manager metadata inventory에서 persisted EquipmentPreset rows를 mutation 없이 투영할 수 있는지 검증합니다.
bool FCFEquipmentBrowserTest::RunTest(const FString& Parameters)
{
	// focused test용 session-local ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	// metadata inventory 초기화 오류입니다.
	FString InitializeError;
	// metadata-only inventory 준비 결과입니다.
	const bool bInitialized = ViewModel.Initialize(InitializeError);
	TestTrue(FString::Printf(TEXT("Equipment browser initializes: %s"), *InitializeError), bInitialized);
	if (!bInitialized)
	{
		return false;
	}

	TestTrue(TEXT("Persisted EquipmentPreset browser has at least one row"), ViewModel.GetPresetEntries().Num() > 0);
	if (ViewModel.GetPresetEntries().IsEmpty())
	{
		return false;
	}

	// 첫 persisted EquipmentPreset browser row입니다.
	const FCFEquipmentPresetListEntry& FirstEntry = ViewModel.GetPresetEntries()[0];
	// existing source load 오류입니다.
	FString LoadError;
	// exact existing EquipmentPreset read-only load 결과입니다.
	const bool bLoaded = ViewModel.LoadExistingPreset(FirstEntry.ObjectPath, LoadError);
	TestTrue(FString::Printf(TEXT("Existing EquipmentPreset loads read-only: %s"), *LoadError), bLoaded);
	if (bLoaded)
	{
		TestFalse(TEXT("Existing draft is not a new draft"), ViewModel.IsNewDraft());
		TestFalse(TEXT("Existing draft starts presentation-clean"), ViewModel.HasDraftChanges());
		TestEqual(TEXT("Existing draft keeps exact source object path"), ViewModel.GetSelectedObjectPath(), FirstEntry.ObjectPath);
	}
	return bLoaded;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentMixedPayloadTest,
	"CarFight.EquipmentAuthoring.P002.MixedPayloadFailClosed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Weapon Draft에 Sensor reference가 섞이면 Product mutation 없이 fail-closed하는지 검증합니다.
bool FCFEquipmentMixedPayloadTest::RunTest(const FString& Parameters)
{
	// mixed payload 진단만 유도하는 transient Draft입니다.
	FCFEquipmentPresetDraft MixedDraft;
	MixedDraft.DraftMode = ECFEquipmentDraftMode::Weapon;
	MixedDraft.DefaultSensorData = TSoftObjectPtr<UCFVehicleSensorData>(FSoftObjectPath(TEXT("/Game/CarFight/Tests/FakeSensor.FakeSensor")));

	// Product/child Asset을 수정하지 않는 pure validation 결과를 만들 ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	// mixed payload의 blocking 결과입니다.
	const FCFEquipmentDraftValidation Validation = ViewModel.BuildValidationForDraft(MixedDraft);
	TestFalse(TEXT("Mixed Weapon+Sensor payload is never package-complete"), Validation.bPackageComplete);
	TestTrue(TEXT("Mixed Weapon+Sensor payload is blocking"), Validation.bHasBlockingErrors);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentScannerContractTest,
	"CarFight.EquipmentAuthoring.P002.ScannerSizeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Scanner Draft가 RequiredWeaponSize=None 계약을 어기면 fail-closed하는지 검증합니다.
bool FCFEquipmentScannerContractTest::RunTest(const FString& Parameters)
{
	// 잘못된 장착 크기를 가진 Scanner transient Draft입니다.
	FCFEquipmentPresetDraft ScannerDraft;
	ScannerDraft.DraftMode = ECFEquipmentDraftMode::Scanner;
	ScannerDraft.RequiredWeaponSize = ECFVehicleWeaponSize::Large;

	// Scanner size contract를 평가할 ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	// Scanner contract 위반 결과입니다.
	const FCFEquipmentDraftValidation Validation = ViewModel.BuildValidationForDraft(ScannerDraft);
	TestFalse(TEXT("Scanner draft with non-None size is incomplete"), Validation.bPackageComplete);
	TestTrue(TEXT("Scanner draft with non-None size is blocking"), Validation.bHasBlockingErrors);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentModeIsolationTest,
	"CarFight.EquipmentAuthoring.P002.ModeIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// UI-only Draft Mode가 wrong-mode selector를 차단하고 각 mode의 transient 선택을 전환 뒤 복원하는지 검증합니다.
bool FCFEquipmentModeIsolationTest::RunTest(const FString& Parameters)
{
	// mode isolation 검증용 session-local ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	ViewModel.BeginNewPreset();

	// wrong-mode setter가 반환할 진단입니다.
	FString SelectionError;
	// 실제 Asset load까지 가지 않아야 하는 가짜 Sensor path입니다.
	const FSoftObjectPath FakeSensorPath(TEXT("/Game/CarFight/Tests/FakeSensor.FakeSensor"));
	// Weapon mode에서 Sensor selector 허용 여부입니다.
	const bool bSensorAcceptedInWeaponMode = ViewModel.SetSensorDataPath(FakeSensorPath, SelectionError);
	TestFalse(TEXT("Weapon mode rejects Sensor selector before load"), bSensorAcceptedInWeaponMode);
	TestTrue(TEXT("Rejected Sensor selector leaves Sensor reference empty"), ViewModel.GetDraft().DefaultSensorData.IsNull());

	// mode restore 검증에 사용할 transient TurretMountData입니다.
	UCFTurretMountData* TurretMountData = NewObject<UCFTurretMountData>(GetTransientPackage());
	// mode restore 검증에 사용할 transient WeaponData입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	// mode restore 검증에 사용할 transient VehicleSensorData입니다.
	UCFVehicleSensorData* SensorData = NewObject<UCFVehicleSensorData>(GetTransientPackage());
	// transient TurretMountData의 soft object path입니다.
	const FSoftObjectPath TurretMountPath(TurretMountData);
	// transient WeaponData의 soft object path입니다.
	const FSoftObjectPath WeaponPath(WeaponData);
	// transient VehicleSensorData의 soft object path입니다.
	const FSoftObjectPath SensorPath(SensorData);

	TestTrue(TEXT("Weapon mode accepts transient TurretMountData"), ViewModel.SetTurretMountDataPath(TurretMountPath, SelectionError));
	TestTrue(TEXT("Weapon mode accepts transient WeaponData"), ViewModel.SetWeaponDataPath(WeaponPath, SelectionError));
	ViewModel.SetRequiredWeaponSize(ECFVehicleWeaponSize::Small);

	ViewModel.SetDraftMode(ECFEquipmentDraftMode::Scanner);
	TestEqual(TEXT("Scanner mode forces RequiredWeaponSize None"), ViewModel.GetDraft().RequiredWeaponSize, ECFVehicleWeaponSize::None);
	TestTrue(TEXT("Scanner projection clears active TurretMountData"), ViewModel.GetDraft().DefaultTurretMountData.IsNull());
	TestTrue(TEXT("Scanner projection clears active WeaponData"), ViewModel.GetDraft().DefaultWeaponData.IsNull());
	TestTrue(TEXT("Scanner mode accepts transient SensorData"), ViewModel.SetSensorDataPath(SensorPath, SelectionError));

	// 실제 Asset load까지 가지 않아야 하는 가짜 Weapon path입니다.
	const FSoftObjectPath FakeWeaponPath(TEXT("/Game/CarFight/Tests/FakeWeapon.FakeWeapon"));
	// Scanner mode에서 Weapon selector 허용 여부입니다.
	const bool bWeaponAcceptedInScannerMode = ViewModel.SetWeaponDataPath(FakeWeaponPath, SelectionError);
	TestFalse(TEXT("Scanner mode rejects Weapon selector before load"), bWeaponAcceptedInScannerMode);

	ViewModel.SetDraftMode(ECFEquipmentDraftMode::Weapon);
	TestEqual(TEXT("Weapon mode restores previous TurretMountData selection"), ViewModel.GetDraft().DefaultTurretMountData.ToSoftObjectPath(), TurretMountPath);
	TestEqual(TEXT("Weapon mode restores previous WeaponData selection"), ViewModel.GetDraft().DefaultWeaponData.ToSoftObjectPath(), WeaponPath);
	TestEqual(TEXT("Weapon mode restores previous weapon size"), ViewModel.GetDraft().RequiredWeaponSize, ECFVehicleWeaponSize::Small);
	TestTrue(TEXT("Weapon projection clears active SensorData"), ViewModel.GetDraft().DefaultSensorData.IsNull());

	ViewModel.SetDraftMode(ECFEquipmentDraftMode::Scanner);
	TestEqual(TEXT("Scanner mode restores previous SensorData selection"), ViewModel.GetDraft().DefaultSensorData.ToSoftObjectPath(), SensorPath);
	TestTrue(TEXT("Scanner projection remains free of WeaponData"), ViewModel.GetDraft().DefaultWeaponData.IsNull());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentWeaponGuidedSuccessTest,
	"CarFight.EquipmentAuthoring.P002.WeaponGuidedSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// transient exact typed selectors로 Weapon principal success path와 expected validation 결과를 결정적으로 검증합니다.
bool FCFEquipmentWeaponGuidedSuccessTest::RunTest(const FString& Parameters)
{
	// Weapon guided success 검증용 session-local ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	ViewModel.BeginNewPreset();
	ViewModel.SetRequiredMountType(ECFVehicleMountType::Turret);

	// 저장되지 않는 transient TurretMountData fixture입니다.
	UCFTurretMountData* TurretMountData = NewObject<UCFTurretMountData>(GetTransientPackage());
	// 저장되지 않는 transient WeaponData fixture입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	// transient selector 진단입니다.
	FString SelectionError;
	// transient TurretMountData path입니다.
	const FSoftObjectPath TurretMountPath(TurretMountData);
	// transient WeaponData path입니다.
	const FSoftObjectPath WeaponPath(WeaponData);

	// TurretMountData typed selector 성공 여부입니다.
	const bool bTurretSelected = ViewModel.SetTurretMountDataPath(TurretMountPath, SelectionError);
	TestTrue(FString::Printf(TEXT("Transient TurretMountData selector succeeds: %s"), *SelectionError), bTurretSelected);
	// WeaponData typed selector 성공 여부입니다.
	const bool bWeaponSelected = ViewModel.SetWeaponDataPath(WeaponPath, SelectionError);
	TestTrue(FString::Printf(TEXT("Transient WeaponData selector succeeds: %s"), *SelectionError), bWeaponSelected);

	// completed Weapon package의 bounded validation 결과입니다.
	const FCFEquipmentDraftValidation Validation = ViewModel.BuildCurrentValidation();
	TestTrue(TEXT("Weapon principal path is package-complete"), Validation.bPackageComplete);
	TestFalse(TEXT("Weapon principal path has no blocking error"), Validation.bHasBlockingErrors);
	TestTrue(TEXT("Weapon native validation reports PASS"), HasValidationItem(Validation, ECFEquipmentValidationSeverity::Pass, TEXT("무기 데이터")));
	return bTurretSelected && bWeaponSelected && Validation.bPackageComplete && !Validation.bHasBlockingErrors;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentScannerGuidedSuccessTest,
	"CarFight.EquipmentAuthoring.P002.ScannerGuidedSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// transient exact typed selector로 Scanner principal success path와 current None-size/native validation 결과를 결정적으로 검증합니다.
bool FCFEquipmentScannerGuidedSuccessTest::RunTest(const FString& Parameters)
{
	// Scanner guided success 검증용 session-local ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	ViewModel.BeginNewPreset();
	ViewModel.SetDraftMode(ECFEquipmentDraftMode::Scanner);
	ViewModel.SetRequiredMountType(ECFVehicleMountType::Utility);

	// 저장되지 않는 transient VehicleSensorData fixture입니다.
	UCFVehicleSensorData* SensorData = NewObject<UCFVehicleSensorData>(GetTransientPackage());
	// transient VehicleSensorData path입니다.
	const FSoftObjectPath SensorPath(SensorData);
	// transient selector 진단입니다.
	FString SelectionError;
	// VehicleSensorData typed selector 성공 여부입니다.
	const bool bSensorSelected = ViewModel.SetSensorDataPath(SensorPath, SelectionError);
	TestTrue(FString::Printf(TEXT("Transient VehicleSensorData selector succeeds: %s"), *SelectionError), bSensorSelected);
	TestEqual(TEXT("Scanner principal path keeps RequiredWeaponSize None"), ViewModel.GetDraft().RequiredWeaponSize, ECFVehicleWeaponSize::None);

	// completed Scanner package의 native validation 결과입니다.
	const FCFEquipmentDraftValidation Validation = ViewModel.BuildCurrentValidation();
	TestTrue(TEXT("Scanner principal path is package-complete"), Validation.bPackageComplete);
	TestFalse(TEXT("Scanner principal path has no blocking error"), Validation.bHasBlockingErrors);
	TestTrue(TEXT("Sensor native validation reports PASS"), HasValidationItem(Validation, ECFEquipmentValidationSeverity::Pass, TEXT("센서 데이터")));
	return bSensorSelected && Validation.bPackageComplete && !Validation.bHasBlockingErrors;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentNativeFailureTest,
	"CarFight.EquipmentAuthoring.P002.NativeChildFailurePropagation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// invalid native Sensor child가 Error severity와 blocking readiness로 결정적으로 전파되는지 검증합니다.
bool FCFEquipmentNativeFailureTest::RunTest(const FString& Parameters)
{
	// native child failure 검증용 session-local ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	ViewModel.BeginNewPreset();
	ViewModel.SetDraftMode(ECFEquipmentDraftMode::Scanner);
	ViewModel.SetRequiredMountType(ECFVehicleMountType::Utility);

	// UpdateIntervalSec=0으로 native contract를 의도적으로 위반한 transient SensorData fixture입니다.
	UCFVehicleSensorData* InvalidSensorData = NewObject<UCFVehicleSensorData>(GetTransientPackage());
	InvalidSensorData->SensorConfig.UpdateIntervalSec = 0.0f;
	// invalid transient SensorData path입니다.
	const FSoftObjectPath InvalidSensorPath(InvalidSensorData);
	// transient selector 진단입니다.
	FString SelectionError;
	TestTrue(TEXT("Invalid SensorData still passes exact typed selection"), ViewModel.SetSensorDataPath(InvalidSensorPath, SelectionError));

	// invalid native child가 포함된 Scanner package validation 결과입니다.
	const FCFEquipmentDraftValidation Validation = ViewModel.BuildCurrentValidation();
	TestTrue(TEXT("Invalid native child does not change payload completeness"), Validation.bPackageComplete);
	TestTrue(TEXT("Invalid native child blocks readiness"), Validation.bHasBlockingErrors);
	TestTrue(TEXT("Invalid native child is surfaced as Sensor Error"), HasValidationItem(Validation, ECFEquipmentValidationSeverity::Error, TEXT("센서 데이터")));
	return Validation.bPackageComplete && Validation.bHasBlockingErrors;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentIntrinsicMountTest,
	"CarFight.EquipmentAuthoring.P002.IntrinsicMountContradiction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// concrete Vehicle 없이도 증명 가능한 DraftMode/RequiredMountType/WeaponData 모순을 blocking하는지 검증합니다.
bool FCFEquipmentIntrinsicMountTest::RunTest(const FString& Parameters)
{
	// intrinsic mount contract를 평가할 session-local ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	// Scanner/Turret contradiction용 transient SensorData입니다.
	UCFVehicleSensorData* SensorData = NewObject<UCFVehicleSensorData>(GetTransientPackage());
	// Scanner contradiction Draft입니다.
	FCFEquipmentPresetDraft ScannerDraft;
	ScannerDraft.DraftMode = ECFEquipmentDraftMode::Scanner;
	ScannerDraft.RequiredWeaponSize = ECFVehicleWeaponSize::None;
	ScannerDraft.RequiredMountType = ECFVehicleMountType::Turret;
	ScannerDraft.DefaultSensorData = SensorData;
	// Scanner/Turret contradiction 결과입니다.
	const FCFEquipmentDraftValidation ScannerValidation = ViewModel.BuildValidationForDraft(ScannerDraft);
	TestTrue(TEXT("Scanner requiring Turret is blocking"), ScannerValidation.bHasBlockingErrors);
	TestTrue(TEXT("Scanner mount contradiction has explicit Error"), HasValidationItem(ScannerValidation, ECFEquipmentValidationSeverity::Error, TEXT("요구 장착 타입")));

	// Weapon mount contradiction용 transient TurretMountData입니다.
	UCFTurretMountData* TurretMountData = NewObject<UCFTurretMountData>(GetTransientPackage());
	// 기본 Turret만 지원하는 transient WeaponData입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	// Weapon/Utility contradiction Draft입니다.
	FCFEquipmentPresetDraft UtilityWeaponDraft;
	UtilityWeaponDraft.DraftMode = ECFEquipmentDraftMode::Weapon;
	UtilityWeaponDraft.RequiredMountType = ECFVehicleMountType::Utility;
	UtilityWeaponDraft.DefaultTurretMountData = TurretMountData;
	UtilityWeaponDraft.DefaultWeaponData = WeaponData;
	// Weapon/Utility contradiction 결과입니다.
	const FCFEquipmentDraftValidation UtilityWeaponValidation = ViewModel.BuildValidationForDraft(UtilityWeaponDraft);
	TestTrue(TEXT("Weapon requiring Utility is blocking"), UtilityWeaponValidation.bHasBlockingErrors);
	TestTrue(TEXT("Weapon Utility contradiction has explicit Error"), HasValidationItem(UtilityWeaponValidation, ECFEquipmentValidationSeverity::Error, TEXT("요구 장착 타입")));

	// WeaponData가 지원하지 않는 Fixed requirement를 가진 Draft입니다.
	FCFEquipmentPresetDraft UnsupportedWeaponDraft = UtilityWeaponDraft;
	UnsupportedWeaponDraft.RequiredMountType = ECFVehicleMountType::Fixed;
	// explicit required mount와 WeaponData support mismatch 결과입니다.
	const FCFEquipmentDraftValidation UnsupportedWeaponValidation = ViewModel.BuildValidationForDraft(UnsupportedWeaponDraft);
	TestTrue(TEXT("Unsupported explicit Weapon mount is blocking"), UnsupportedWeaponValidation.bHasBlockingErrors);
	TestTrue(TEXT("Unsupported Weapon mount has explicit Error"), HasValidationItem(UnsupportedWeaponValidation, ECFEquipmentValidationSeverity::Error, TEXT("요구 장착 타입")));
	return ScannerValidation.bHasBlockingErrors
		&& UtilityWeaponValidation.bHasBlockingErrors
		&& UnsupportedWeaponValidation.bHasBlockingErrors;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentSelectorRoundTripTest,
	"CarFight.EquipmentAuthoring.P002.PersistedSelectorRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// persisted EquipmentPreset의 실제 child reference를 transient typed setter로 다시 선택할 수 있는지 검증합니다.
bool FCFEquipmentSelectorRoundTripTest::RunTest(const FString& Parameters)
{
	// persisted EquipmentPreset을 탐색할 session-local ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	// metadata inventory 초기화 오류입니다.
	FString InitializeError;
	// persisted browser 준비 결과입니다.
	const bool bInitialized = ViewModel.Initialize(InitializeError);
	TestTrue(FString::Printf(TEXT("Equipment browser initializes for selector round-trip: %s"), *InitializeError), bInitialized);
	if (!bInitialized)
	{
		return false;
	}

	// 실제 child selector 성공 경로를 하나 이상 실행했는지 나타냅니다.
	bool bExercisedSelector = false;
	// browser row 탐색에 사용하는 index입니다.
	for (int32 EntryIndex = 0; EntryIndex < ViewModel.GetPresetEntries().Num() && !bExercisedSelector; ++EntryIndex)
	{
		// current persisted EquipmentPreset read-only load 진단입니다.
		FString LoadError;
		if (!ViewModel.LoadExistingPreset(ViewModel.GetPresetEntries()[EntryIndex].ObjectPath, LoadError))
		{
			continue;
		}

		// persisted source에서 읽은 child reference snapshot입니다.
		const FCFEquipmentPresetDraft SourceDraft = ViewModel.GetDraft();
		if (SourceDraft.DraftMode == ECFEquipmentDraftMode::Weapon
			&& !SourceDraft.DefaultTurretMountData.IsNull()
			&& !SourceDraft.DefaultWeaponData.IsNull())
		{
			ViewModel.BeginNewPreset();
			// typed selector 호출의 사용자 진단입니다.
			FString SelectionError;
			// persisted TurretMountData를 exact typed setter로 선택한 결과입니다.
			const bool bTurretSelected = ViewModel.SetTurretMountDataPath(SourceDraft.DefaultTurretMountData.ToSoftObjectPath(), SelectionError);
			TestTrue(FString::Printf(TEXT("Persisted TurretMountData selector succeeds: %s"), *SelectionError), bTurretSelected);
			// persisted WeaponData를 exact typed setter로 선택한 결과입니다.
			const bool bWeaponSelected = ViewModel.SetWeaponDataPath(SourceDraft.DefaultWeaponData.ToSoftObjectPath(), SelectionError);
			TestTrue(FString::Printf(TEXT("Persisted WeaponData selector succeeds: %s"), *SelectionError), bWeaponSelected);
			bExercisedSelector = bTurretSelected && bWeaponSelected;
		}
		else if (SourceDraft.DraftMode == ECFEquipmentDraftMode::Scanner && !SourceDraft.DefaultSensorData.IsNull())
		{
			ViewModel.BeginNewPreset();
			ViewModel.SetDraftMode(ECFEquipmentDraftMode::Scanner);
			// typed selector 호출의 사용자 진단입니다.
			FString SelectionError;
			// persisted VehicleSensorData를 exact typed setter로 선택한 결과입니다.
			const bool bSensorSelected = ViewModel.SetSensorDataPath(SourceDraft.DefaultSensorData.ToSoftObjectPath(), SelectionError);
			TestTrue(FString::Printf(TEXT("Persisted VehicleSensorData selector succeeds: %s"), *SelectionError), bSensorSelected);
			bExercisedSelector = bSensorSelected;
		}
	}

	TestTrue(TEXT("At least one persisted child reference exercises an exact typed selector"), bExercisedSelector);
	return bExercisedSelector;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentExistingValidationTest,
	"CarFight.EquipmentAuthoring.P002.ExistingChildValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// persisted EquipmentPreset 하나를 read-only load해 현재 child validation hierarchy가 실제 Asset graph에서 실행되는지 검증합니다.
bool FCFEquipmentExistingValidationTest::RunTest(const FString& Parameters)
{
	// persisted browser와 validation을 사용하는 session-local ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	// metadata inventory 초기화 오류입니다.
	FString InitializeError;
	// persisted EquipmentPreset inventory 준비 결과입니다.
	const bool bInitialized = ViewModel.Initialize(InitializeError);
	TestTrue(FString::Printf(TEXT("Equipment browser initializes for P002: %s"), *InitializeError), bInitialized);
	if (!bInitialized || ViewModel.GetPresetEntries().IsEmpty())
	{
		return false;
	}

	// 첫 persisted EquipmentPreset의 read-only load 진단입니다.
	FString LoadError;
	// 첫 persisted row를 validation 대상으로 읽은 결과입니다.
	const bool bLoaded = ViewModel.LoadExistingPreset(ViewModel.GetPresetEntries()[0].ObjectPath, LoadError);
	TestTrue(FString::Printf(TEXT("Existing EquipmentPreset loads for child validation: %s"), *LoadError), bLoaded);
	if (!bLoaded)
	{
		return false;
	}

	// 실제 persisted child graph에 대한 bounded validation 결과입니다.
	const FCFEquipmentDraftValidation Validation = ViewModel.BuildCurrentValidation();
	TestTrue(TEXT("Existing EquipmentPreset produces explicit validation items"), Validation.Items.Num() > 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentCompatibilityFreshnessTest,
	"CarFight.EquipmentAuthoring.P004.SourceFreshness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// VehicleDataObjectPath + MountProfileId transient identity와 CannotEvaluate freshness semantics를 Product mutation 없이 검증합니다.
bool FCFEquipmentCompatibilityFreshnessTest::RunTest(const FString& Parameters)
{
	// compatibility source freshness를 검증할 session-local ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	ViewModel.BeginNewPreset();

	ViewModel.RefreshCompatibilityProbe();
	TestEqual(TEXT("No Vehicle selection is CannotEvaluate"), ViewModel.GetCompatibilityResult().State, ECFEquipmentCompatibilityState::CannotEvaluate);
	TestEqual(TEXT("No Vehicle selection reason is exact"), ViewModel.GetCompatibilityResult().Reason, ECFEquipmentCompatibilityReason::NoVehicleSelected);

	ViewModel.SetCompatibilityVehicleDataPath(FSoftObjectPath(TEXT("/Game/CarFight/Tests/DoesNotExist.DoesNotExist")));
	TestEqual(TEXT("Unresolved Vehicle source is CannotEvaluate"), ViewModel.GetCompatibilityResult().State, ECFEquipmentCompatibilityState::CannotEvaluate);
	TestEqual(TEXT("Unresolved Vehicle source reason is exact"), ViewModel.GetCompatibilityResult().Reason, ECFEquipmentCompatibilityReason::VehicleSourceUnresolved);

	// disk에 persist되지 않는 transient VehicleData fixture입니다.
	UCFVehicleData* TransientVehicleData = NewObject<UCFVehicleData>(GetTransientPackage());
	ViewModel.SetCompatibilityVehicleDataPath(FSoftObjectPath(TransientVehicleData));
	TestEqual(TEXT("Unpersisted Vehicle source is CannotEvaluate"), ViewModel.GetCompatibilityResult().State, ECFEquipmentCompatibilityState::CannotEvaluate);
	TestEqual(TEXT("Unpersisted Vehicle source reason is exact"), ViewModel.GetCompatibilityResult().Reason, ECFEquipmentCompatibilityReason::VehicleSourceUnpersisted);

	// clean persisted Vehicle source path입니다.
	FSoftObjectPath VehicleDataPath;
	// clean persisted Vehicle의 unique concrete Mount입니다.
	FCFVehicleMountProfile MountProfile;
	// read-only fixture discovery 진단입니다.
	FString FindError;
	if (!FindPersistedVehicleMount(ECFEquipmentTestMountKind::AnyConcrete, VehicleDataPath, MountProfile, FindError))
	{
		AddError(FindError);
		return false;
	}
	ViewModel.SetCompatibilityVehicleDataPath(VehicleDataPath);
	TestTrue(TEXT("Persisted Vehicle selection exposes current Mount options"), ViewModel.GetCompatibilityMountOptions().Num() > 0);
	TestTrue(TEXT("Vehicle selection resets previous Mount identity"), ViewModel.GetCompatibilityMountProfileId().IsNone());
	TestEqual(TEXT("Persisted Vehicle without Mount selection stays CannotEvaluate"), ViewModel.GetCompatibilityResult().Reason, ECFEquipmentCompatibilityReason::MountProfileIdNone);

	ViewModel.SetCompatibilityMountProfileId(TEXT("P004_MissingMount"));
	TestEqual(TEXT("Missing current Mount stays CannotEvaluate"), ViewModel.GetCompatibilityResult().State, ECFEquipmentCompatibilityState::CannotEvaluate);
	TestEqual(TEXT("Missing current Mount reason is exact"), ViewModel.GetCompatibilityResult().Reason, ECFEquipmentCompatibilityReason::MountProfileMissing);

	ViewModel.SetCompatibilityVehicleDataPath(FSoftObjectPath());
	TestTrue(TEXT("Vehicle identity change clears stale Mount identity"), ViewModel.GetCompatibilityMountProfileId().IsNone());
	TestEqual(TEXT("Cleared Vehicle returns NoVehicleSelected"), ViewModel.GetCompatibilityResult().Reason, ECFEquipmentCompatibilityReason::NoVehicleSelected);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentWeaponCompatibilityTest,
	"CarFight.EquipmentAuthoring.P004.WeaponCompatible",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// persisted clean Weapon Mount와 transient valid Weapon Equipment Draft가 existing CanUseOnMount authority로 Compatible이 되는지 검증합니다.
bool FCFEquipmentWeaponCompatibilityTest::RunTest(const FString& Parameters)
{
	// read-only Vehicle source path입니다.
	FSoftObjectPath VehicleDataPath;
	// compatible test에 사용할 unique Weapon Mount입니다.
	FCFVehicleMountProfile MountProfile;
	// read-only fixture discovery 진단입니다.
	FString FindError;
	if (!FindPersistedVehicleMount(ECFEquipmentTestMountKind::Weapon, VehicleDataPath, MountProfile, FindError))
	{
		AddError(FindError);
		return false;
	}

	// compatibility test session-local VM입니다.
	FCFEquipmentBuilderVM ViewModel;
	// transient WeaponData fixture입니다.
	UCFWeaponData* WeaponData = nullptr;
	// transient Draft 구성 진단입니다.
	FString ConfigureError;
	if (!ConfigureTransientWeaponDraft(ViewModel, MountProfile, WeaponData, ECFVehicleMountType::None, ConfigureError))
	{
		AddError(ConfigureError);
		return false;
	}
	// compatibility identity 변경 전 authored Draft dirty state입니다.
	const bool bDraftDirtyBeforeProbe = ViewModel.HasDraftChanges();
	ViewModel.SetCompatibilityVehicleDataPath(VehicleDataPath);
	ViewModel.SetCompatibilityMountProfileId(MountProfile.MountProfileId);

	TestEqual(TEXT("Valid Weapon Equipment is Compatible"), ViewModel.GetCompatibilityResult().State, ECFEquipmentCompatibilityState::Compatible);
	TestEqual(TEXT("Compatible result has no failure reason"), ViewModel.GetCompatibilityResult().Reason, ECFEquipmentCompatibilityReason::None);
	TestEqual(TEXT("Compatibility probe does not alter Equipment Draft dirty state"), ViewModel.HasDraftChanges(), bDraftDirtyBeforeProbe);
	return ViewModel.GetCompatibilityResult().State == ECFEquipmentCompatibilityState::Compatible;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentMountMismatchTest,
	"CarFight.EquipmentAuthoring.P004.RequiredMountMismatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// intrinsic-valid Weapon Draft의 explicit RequiredMountType이 selected Mount와 다르면 Incompatible인지 검증합니다.
bool FCFEquipmentMountMismatchTest::RunTest(const FString& Parameters)
{
	// read-only Vehicle source path입니다.
	FSoftObjectPath VehicleDataPath;
	// mismatch target으로 사용할 unique Weapon Mount입니다.
	FCFVehicleMountProfile MountProfile;
	// read-only fixture discovery 진단입니다.
	FString FindError;
	if (!FindPersistedVehicleMount(ECFEquipmentTestMountKind::Weapon, VehicleDataPath, MountProfile, FindError))
	{
		AddError(FindError);
		return false;
	}
	// selected Mount와 다른 explicit Weapon mount requirement입니다.
	const ECFVehicleMountType DifferentMountType = ChooseDifferentWeaponMountType(MountProfile.MountType);
	// mismatch test session-local VM입니다.
	FCFEquipmentBuilderVM ViewModel;
	// selected + required mount를 모두 native support하도록 구성한 transient WeaponData입니다.
	UCFWeaponData* WeaponData = nullptr;
	// transient Draft 구성 진단입니다.
	FString ConfigureError;
	if (!ConfigureTransientWeaponDraft(ViewModel, MountProfile, WeaponData, DifferentMountType, ConfigureError))
	{
		AddError(ConfigureError);
		return false;
	}
	ViewModel.SetRequiredMountType(DifferentMountType);
	TestFalse(TEXT("Different RequiredMountType remains intrinsic-clean when WeaponData supports it"), ViewModel.BuildCurrentValidation().bHasBlockingErrors);
	ViewModel.SetCompatibilityVehicleDataPath(VehicleDataPath);
	ViewModel.SetCompatibilityMountProfileId(MountProfile.MountProfileId);

	TestEqual(TEXT("RequiredMountType mismatch is Incompatible"), ViewModel.GetCompatibilityResult().State, ECFEquipmentCompatibilityState::Incompatible);
	TestEqual(TEXT("RequiredMountType mismatch reason is exact"), ViewModel.GetCompatibilityResult().Reason, ECFEquipmentCompatibilityReason::RequiredMountTypeMismatch);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentSizeMismatchTest,
	"CarFight.EquipmentAuthoring.P004.SizeMismatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Product VehicleData mutation 없이 Large requirement > Medium SizeLimit bounded branch를 직접 검증합니다.
bool FCFEquipmentSizeMismatchTest::RunTest(const FString& Parameters)
{
	// read-only persisted Vehicle source path입니다.
	FSoftObjectPath VehicleDataPath;
	// mount type authority만 제공할 unique Weapon Mount입니다.
	FCFVehicleMountProfile MountProfile;
	// read-only fixture discovery 진단입니다.
	FString FindError;
	if (!FindPersistedVehicleMount(ECFEquipmentTestMountKind::Weapon, VehicleDataPath, MountProfile, FindError))
	{
		AddError(FindError);
		return false;
	}

	// size mismatch bounded helper를 검증할 session-local VM입니다.
	FCFEquipmentBuilderVM ViewModel;
	// target mount type을 support하는 transient WeaponData입니다.
	UCFWeaponData* WeaponData = nullptr;
	// transient Draft 구성 진단입니다.
	FString ConfigureError;
	if (!ConfigureTransientWeaponDraft(ViewModel, MountProfile, WeaponData, ECFVehicleMountType::None, ConfigureError))
	{
		AddError(ConfigureError);
		return false;
	}
	WeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
	ViewModel.SetRequiredWeaponSize(ECFVehicleWeaponSize::Large);

	// bounded helper가 반환한 exact incompatibility reason입니다.
	ECFEquipmentCompatibilityReason CompatibilityReason = ECFEquipmentCompatibilityReason::None;
	// bounded helper가 반환한 사용자 diagnostic입니다.
	FString CompatibilityDiagnostic;
	// Product Mount를 변경하지 않고 synthetic Medium limit로 existing runtime-equivalent size rule을 검증한 결과입니다.
	const bool bCompatible = ViewModel.EvaluateDraftMountContract(
		MountProfile.MountType,
		ECFVehicleWeaponSize::Medium,
		CompatibilityReason,
		CompatibilityDiagnostic);

	TestFalse(TEXT("Large RequiredWeaponSize above Medium SizeLimit is incompatible"), bCompatible);
	TestEqual(TEXT("RequiredWeaponSize mismatch reason is exact"), CompatibilityReason, ECFEquipmentCompatibilityReason::RequiredWeaponSizeTooLarge);
	TestTrue(TEXT("Size mismatch diagnostic is explicit"), !CompatibilityDiagnostic.IsEmpty());
	return !bCompatible && CompatibilityReason == ECFEquipmentCompatibilityReason::RequiredWeaponSizeTooLarge;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentWeaponNativeMountMismatchTest,
	"CarFight.EquipmentAuthoring.P004.WeaponDataMountMismatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// RequiredMountType wildcard는 유지하면서 WeaponData 자체가 selected MountType을 거부하면 Incompatible인지 검증합니다.
bool FCFEquipmentWeaponNativeMountMismatchTest::RunTest(const FString& Parameters)
{
	// read-only Vehicle source path입니다.
	FSoftObjectPath VehicleDataPath;
	// WeaponData native mount mismatch 대상으로 사용할 unique Weapon Mount입니다.
	FCFVehicleMountProfile MountProfile;
	// read-only fixture discovery 진단입니다.
	FString FindError;
	if (!FindPersistedVehicleMount(ECFEquipmentTestMountKind::Weapon, VehicleDataPath, MountProfile, FindError))
	{
		AddError(FindError);
		return false;
	}

	// target Mount와 다른 native Weapon mount type입니다.
	const ECFVehicleMountType DifferentMountType = ChooseDifferentWeaponMountType(MountProfile.MountType);
	// native WeaponData mismatch를 평가할 session-local VM입니다.
	FCFEquipmentBuilderVM ViewModel;
	// transient WeaponData fixture입니다.
	UCFWeaponData* WeaponData = nullptr;
	// transient Draft 구성 진단입니다.
	FString ConfigureError;
	if (!ConfigureTransientWeaponDraft(ViewModel, MountProfile, WeaponData, ECFVehicleMountType::None, ConfigureError))
	{
		AddError(ConfigureError);
		return false;
	}

	WeaponData->CompatibleMountTypes.Reset();
	WeaponData->CompatibleMountTypes.Add(DifferentMountType);
	ViewModel.RefreshCompatibilityProbe();
	TestFalse(TEXT("WeaponData with alternate concrete support remains intrinsic-clean under wildcard requirement"), ViewModel.BuildCurrentValidation().bHasBlockingErrors);

	ViewModel.SetCompatibilityVehicleDataPath(VehicleDataPath);
	ViewModel.SetCompatibilityMountProfileId(MountProfile.MountProfileId);
	TestEqual(TEXT("WeaponData mount mismatch is Incompatible"), ViewModel.GetCompatibilityResult().State, ECFEquipmentCompatibilityState::Incompatible);
	TestEqual(TEXT("WeaponData mount mismatch reason is exact"), ViewModel.GetCompatibilityResult().Reason, ECFEquipmentCompatibilityReason::WeaponMountUnsupported);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentIntrinsicInvalidCompatibilityTest,
	"CarFight.EquipmentAuthoring.P004.IntrinsicInvalidCannotEvaluate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// persisted clean concrete Mount를 선택해도 incomplete/blocking Equipment Draft면 compatibility가 CannotEvaluate인지 검증합니다.
bool FCFEquipmentIntrinsicInvalidCompatibilityTest::RunTest(const FString& Parameters)
{
	// read-only Vehicle source path입니다.
	FSoftObjectPath VehicleDataPath;
	// intrinsic-invalid Draft에 연결할 unique concrete Mount입니다.
	FCFVehicleMountProfile MountProfile;
	// read-only fixture discovery 진단입니다.
	FString FindError;
	if (!FindPersistedVehicleMount(ECFEquipmentTestMountKind::AnyConcrete, VehicleDataPath, MountProfile, FindError))
	{
		AddError(FindError);
		return false;
	}

	// child selection이 없는 incomplete Draft를 가진 session-local VM입니다.
	FCFEquipmentBuilderVM ViewModel;
	ViewModel.BeginNewPreset();
	ViewModel.SetCompatibilityVehicleDataPath(VehicleDataPath);
	ViewModel.SetCompatibilityMountProfileId(MountProfile.MountProfileId);

	TestEqual(TEXT("Intrinsic-invalid Equipment Draft is CannotEvaluate"), ViewModel.GetCompatibilityResult().State, ECFEquipmentCompatibilityState::CannotEvaluate);
	TestEqual(TEXT("Intrinsic-invalid Equipment Draft reason is exact"), ViewModel.GetCompatibilityResult().Reason, ECFEquipmentCompatibilityReason::EquipmentIntrinsicInvalid);
	TestTrue(TEXT("Existing intrinsic durable guard remains blocking/incomplete"), !ViewModel.BuildCurrentValidation().bPackageComplete || ViewModel.BuildCurrentValidation().bHasBlockingErrors);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentScannerCompatibilityTest,
	"CarFight.EquipmentAuthoring.P004.ScannerUtilityCompatible",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// persisted clean Utility Mount와 transient valid Scanner Equipment Draft가 Compatible인지 검증합니다.
bool FCFEquipmentScannerCompatibilityTest::RunTest(const FString& Parameters)
{
	// read-only Vehicle source path입니다.
	FSoftObjectPath VehicleDataPath;
	// Scanner test에 사용할 unique Utility Mount입니다.
	FCFVehicleMountProfile MountProfile;
	// read-only fixture discovery 진단입니다.
	FString FindError;
	if (!FindPersistedVehicleMount(ECFEquipmentTestMountKind::Utility, VehicleDataPath, MountProfile, FindError))
	{
		AddError(FindError);
		return false;
	}
	// Scanner compatibility session-local VM입니다.
	FCFEquipmentBuilderVM ViewModel;
	ViewModel.BeginNewPreset();
	ViewModel.SetEquipmentId(TEXT("EBA_P004_Scanner"));
	ViewModel.SetDisplayName(FText::FromString(TEXT("P004 호환성 테스트 스캐너")));
	ViewModel.SetDraftMode(ECFEquipmentDraftMode::Scanner);
	ViewModel.SetRequiredMountType(ECFVehicleMountType::Utility);
	// test-only transient valid SensorData입니다.
	UCFVehicleSensorData* SensorData = NewObject<UCFVehicleSensorData>(GetTransientPackage());
	// Sensor selector 진단입니다.
	FString SelectionError;
	if (!ViewModel.SetSensorDataPath(FSoftObjectPath(SensorData), SelectionError))
	{
		AddError(SelectionError);
		return false;
	}
	ViewModel.SetCompatibilityVehicleDataPath(VehicleDataPath);
	ViewModel.SetCompatibilityMountProfileId(MountProfile.MountProfileId);

	TestEqual(TEXT("Valid Scanner on Utility Mount is Compatible"), ViewModel.GetCompatibilityResult().State, ECFEquipmentCompatibilityState::Compatible);
	return ViewModel.GetCompatibilityResult().State == ECFEquipmentCompatibilityState::Compatible;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentCompatibilityReviewIsolationTest,
	"CarFight.EquipmentAuthoring.P004.DurableReviewIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// compatibility identity/result 변경이 existing Reviewed approval digest와 durable readiness를 변경하지 않는지 검증합니다.
bool FCFEquipmentCompatibilityReviewIsolationTest::RunTest(const FString& Parameters)
{
	// existing persisted EquipmentPreset 중 read-only Review 가능한 source를 찾을 VM입니다.
	FCFEquipmentBuilderVM ViewModel;
	// metadata inventory 초기화 진단입니다.
	FString InitializeError;
	if (!ViewModel.Initialize(InitializeError))
	{
		AddError(InitializeError);
		return false;
	}

	// mutation 없이 Review까지 통과한 Product source를 찾았는지 나타냅니다.
	bool bReviewedSourceFound = false;
	for (const FCFEquipmentPresetListEntry& Entry : ViewModel.GetPresetEntries())
	{
		// current candidate load 진단입니다.
		FString LoadError;
		if (!ViewModel.LoadExistingPreset(Entry.ObjectPath, LoadError))
		{
			continue;
		}
		// current candidate read-only Review 진단입니다.
		FString ReviewError;
		if (ViewModel.ReviewCurrentDraft(ReviewError))
		{
			bReviewedSourceFound = true;
			break;
		}
	}
	TestTrue(TEXT("At least one persisted EquipmentPreset can be Reviewed read-only"), bReviewedSourceFound);
	if (!bReviewedSourceFound)
	{
		return false;
	}

	// compatibility 변경 전 frozen ReviewProposalDigest입니다.
	const FString ReviewedDigest = ViewModel.GetReviewState().ReviewProposalDigest;
	// compatibility 변경 전 approval state입니다.
	const ECFEquipmentApprovalState ApprovalStateBefore = ViewModel.GetReviewState().ApprovalState;
	// compatibility 변경 전 intrinsic blocking state입니다.
	const bool bBlockingBefore = ViewModel.BuildCurrentValidation().bHasBlockingErrors;
	// compatibility 변경 전 authored Draft dirty state입니다.
	const bool bDraftDirtyBefore = ViewModel.HasDraftChanges();

	ViewModel.SetCompatibilityVehicleDataPath(FSoftObjectPath(TEXT("/Game/CarFight/Tests/DoesNotExist.DoesNotExist")));
	ViewModel.SetCompatibilityMountProfileId(TEXT("IgnoredMount"));
	ViewModel.RefreshCompatibilityProbe();

	TestEqual(TEXT("Compatibility changes do not change ReviewProposalDigest"), ViewModel.GetReviewState().ReviewProposalDigest, ReviewedDigest);
	TestEqual(TEXT("Compatibility changes do not consume/invalidate approval"), ViewModel.GetReviewState().ApprovalState, ApprovalStateBefore);
	TestEqual(TEXT("Compatibility changes do not change intrinsic blocking state"), ViewModel.BuildCurrentValidation().bHasBlockingErrors, bBlockingBefore);
	TestEqual(TEXT("Compatibility changes do not dirty authored Equipment Draft"), ViewModel.HasDraftChanges(), bDraftDirtyBefore);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentCrossBuilderNavigationTest,
	"CarFight.EquipmentAuthoring.P004.CrossBuilderNavigation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// P0 Cross-Builder contract가 existing Vehicle Builder Nomad Tab navigation만 요구할 수 있는지 검증합니다.
bool FCFEquipmentCrossBuilderNavigationTest::RunTest(const FString& Parameters)
{
	// frozen Vehicle Builder public tab identity입니다.
	const FName VehicleBuilderTabName(TEXT("CarFight.VehicleBuilder"));
	TestTrue(TEXT("Vehicle Builder tab spawner is registered for navigation-only handoff"), FGlobalTabmanager::Get()->HasTabSpawner(VehicleBuilderTabName));
	// existing registered Vehicle Builder tab입니다.
	TSharedPtr<SDockTab> VehicleBuilderTab = FGlobalTabmanager::Get()->TryInvokeTab(VehicleBuilderTabName);
	TestTrue(TEXT("Vehicle Builder can be opened without Equipment context injection"), VehicleBuilderTab.IsValid());
	if (VehicleBuilderTab.IsValid())
	{
		VehicleBuilderTab->RequestCloseTab();
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentBuilderTabTest,
	"CarFight.EquipmentAuthoring.P001.NativeSlateTab",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Editor module이 Equipment Builder Nomad Tab을 등록하고 실제 Native Slate content를 생성하는지 검증합니다.
bool FCFEquipmentBuilderTabTest::RunTest(const FString& Parameters)
{
	// Equipment Builder의 frozen public tab identity입니다.
	const FName EquipmentBuilderTabName(TEXT("CarFight.EquipmentBuilder"));
	TestTrue(TEXT("Equipment Builder tab spawner is registered"), FGlobalTabmanager::Get()->HasTabSpawner(EquipmentBuilderTabName));

	// 실제 등록된 Nomad Tab 인스턴스입니다.
	TSharedPtr<SDockTab> EquipmentBuilderTab = FGlobalTabmanager::Get()->TryInvokeTab(EquipmentBuilderTabName);
	TestTrue(TEXT("Equipment Builder tab is created"), EquipmentBuilderTab.IsValid());
	if (EquipmentBuilderTab.IsValid())
	{
		TestTrue(TEXT("Equipment Builder tab has Native Slate content"), EquipmentBuilderTab->GetContent() != SNullWidget::NullWidget);
		EquipmentBuilderTab->RequestCloseTab();
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentP005ScannerModeRestoreTest,
	"CarFight.EquipmentAuthoring.P005.ScannerUtilityWeaponRestore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Scanner 전환이 Utility/None을 강제하고 Weapon 복귀 시 이전 mount type/size/reference를 정확히 복원하는지 검증합니다.
bool FCFEquipmentP005ScannerModeRestoreTest::RunTest(const FString& Parameters)
{
	// P005 Scanner/Weapon mode 회귀를 검증할 session-local ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	ViewModel.BeginNewPreset();
	ViewModel.SetRequiredMountType(ECFVehicleMountType::Launcher);
	ViewModel.SetRequiredWeaponSize(ECFVehicleWeaponSize::Small);

	// Weapon mode에서 복원 여부를 확인할 transient TurretMountData입니다.
	UCFTurretMountData* TurretMountData = NewObject<UCFTurretMountData>(GetTransientPackage());
	// Weapon mode에서 복원 여부를 확인할 transient WeaponData입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	// Typed selector 호출의 진단 문자열입니다.
	FString SelectionError;
	// 복원 비교에 사용할 transient TurretMountData path입니다.
	const FSoftObjectPath TurretMountPath(TurretMountData);
	// 복원 비교에 사용할 transient WeaponData path입니다.
	const FSoftObjectPath WeaponPath(WeaponData);

	TestTrue(TEXT("P005 Weapon mode accepts transient TurretMountData"), ViewModel.SetTurretMountDataPath(TurretMountPath, SelectionError));
	TestTrue(TEXT("P005 Weapon mode accepts transient WeaponData"), ViewModel.SetWeaponDataPath(WeaponPath, SelectionError));

	ViewModel.SetDraftMode(ECFEquipmentDraftMode::Scanner);
	TestEqual(TEXT("P005 Scanner mode forces Utility mount type"), ViewModel.GetDraft().RequiredMountType, ECFVehicleMountType::Utility);
	TestEqual(TEXT("P005 Scanner mode forces None weapon size"), ViewModel.GetDraft().RequiredWeaponSize, ECFVehicleWeaponSize::None);
	TestTrue(TEXT("P005 Scanner projection clears TurretMountData"), ViewModel.GetDraft().DefaultTurretMountData.IsNull());
	TestTrue(TEXT("P005 Scanner projection clears WeaponData"), ViewModel.GetDraft().DefaultWeaponData.IsNull());

	// Scanner mode에서 잘못된 사용자 입력을 시도해도 고정 계약이 유지되는지 검증합니다.
	ViewModel.SetRequiredMountType(ECFVehicleMountType::Turret);
	ViewModel.SetRequiredWeaponSize(ECFVehicleWeaponSize::Large);
	TestEqual(TEXT("P005 Scanner ignores mount-type edit and stays Utility"), ViewModel.GetDraft().RequiredMountType, ECFVehicleMountType::Utility);
	TestEqual(TEXT("P005 Scanner ignores size edit and stays None"), ViewModel.GetDraft().RequiredWeaponSize, ECFVehicleWeaponSize::None);

	ViewModel.SetDraftMode(ECFEquipmentDraftMode::Weapon);
	TestEqual(TEXT("P005 Weapon return restores Launcher mount type"), ViewModel.GetDraft().RequiredMountType, ECFVehicleMountType::Launcher);
	TestEqual(TEXT("P005 Weapon return restores Small weapon size"), ViewModel.GetDraft().RequiredWeaponSize, ECFVehicleWeaponSize::Small);
	TestEqual(TEXT("P005 Weapon return restores TurretMountData"), ViewModel.GetDraft().DefaultTurretMountData.ToSoftObjectPath(), TurretMountPath);
	TestEqual(TEXT("P005 Weapon return restores WeaponData"), ViewModel.GetDraft().DefaultWeaponData.ToSoftObjectPath(), WeaponPath);
	TestTrue(TEXT("P005 Weapon return keeps SensorData empty"), ViewModel.GetDraft().DefaultSensorData.IsNull());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentP005AutoIdentityTest,
	"CarFight.EquipmentAuthoring.P005.AutoIdentity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// 신규 에셋 이름이 내부 EquipmentId와 canonical Product target을 자동으로 동일 이름으로 파생하는지 검증합니다.
bool FCFEquipmentP005AutoIdentityTest::RunTest(const FString& Parameters)
{
	// 신규 Guided identity 규칙을 검증할 session-local ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	ViewModel.BeginNewPreset();
	ViewModel.SetCreateAssetName(TEXT("  HeavyCannon_Mk2  "));

	TestEqual(TEXT("P005 auto identity trims asset name"), ViewModel.GetCreateAssetName(), FString(TEXT("HeavyCannon_Mk2")));
	TestEqual(TEXT("P005 auto identity derives EquipmentId"), ViewModel.GetDraft().EquipmentId, FName(TEXT("HeavyCannon_Mk2")));
	TestEqual(
		TEXT("P005 auto identity derives canonical EquipmentPresets target"),
		ViewModel.GetCreateTargetObjectPath(),
		FString(TEXT("/Game/CarFight/Weapons/Data/EquipmentPresets/HeavyCannon_Mk2.HeavyCannon_Mk2")));

	ViewModel.ResetDraftToSource();
	TestTrue(TEXT("P005 reset clears guided asset name"), ViewModel.GetCreateAssetName().IsEmpty());
	TestTrue(TEXT("P005 reset clears guided EquipmentId"), ViewModel.GetDraft().EquipmentId.IsNone());
	TestTrue(TEXT("P005 reset clears guided canonical target"), ViewModel.GetCreateTargetObjectPath().IsEmpty());
	TestFalse(TEXT("P005 reset returns new draft presentation-clean"), ViewModel.HasDraftChanges());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFEquipmentP005ExistingModeLockTest,
	"CarFight.EquipmentAuthoring.P005.ExistingModeLocked",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// persisted EquipmentPreset을 편집할 때 Weapon/Scanner 종류 변경 요청이 무시되는지 검증합니다.
bool FCFEquipmentP005ExistingModeLockTest::RunTest(const FString& Parameters)
{
	// persisted EquipmentPreset metadata와 편집 lock을 검증할 session-local ViewModel입니다.
	FCFEquipmentBuilderVM ViewModel;
	// persisted browser 초기화 진단입니다.
	FString InitializeError;
	// persisted browser 초기화 성공 여부입니다.
	const bool bInitialized = ViewModel.Initialize(InitializeError);
	TestTrue(FString::Printf(TEXT("P005 existing-mode browser initializes: %s"), *InitializeError), bInitialized);
	if (!bInitialized || ViewModel.GetPresetEntries().IsEmpty())
	{
		return false;
	}

	// persisted EquipmentPreset read-only load 진단입니다.
	FString LoadError;
	// 첫 persisted EquipmentPreset read-only load 성공 여부입니다.
	const bool bLoaded = ViewModel.LoadExistingPreset(ViewModel.GetPresetEntries()[0].ObjectPath, LoadError);
	TestTrue(FString::Printf(TEXT("P005 existing EquipmentPreset loads: %s"), *LoadError), bLoaded);
	if (!bLoaded)
	{
		return false;
	}

	// 장비 종류 변경 요청 전의 persisted mode입니다.
	const ECFEquipmentDraftMode OriginalMode = ViewModel.GetDraft().DraftMode;
	// 현재 persisted mode와 반대되는 변경 요청입니다.
	const ECFEquipmentDraftMode RequestedMode = OriginalMode == ECFEquipmentDraftMode::Weapon
		? ECFEquipmentDraftMode::Scanner
		: ECFEquipmentDraftMode::Weapon;

	TestFalse(TEXT("P005 persisted EquipmentPreset cannot change draft mode"), ViewModel.CanChangeDraftMode());
	ViewModel.SetDraftMode(RequestedMode);
	TestEqual(TEXT("P005 persisted EquipmentPreset keeps original mode"), ViewModel.GetDraft().DraftMode, OriginalMode);
	return true;
}

#endif
