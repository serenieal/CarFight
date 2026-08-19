// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-16
// Description: CF-FQ-037 SCAN-P0-03 Pawn Sensor Gameplay command와 실제 IA_ActiveScan/V Enhanced Input 계약 테스트
// Changelog:
// - v1.1.0: IA_ActiveScan Boolean + Pressed 자산과 IMC_Vehicle_Default V 매핑을 생성/검증하는 InputAsset 테스트를 추가하고 Pawn 기본 로드 계약을 검증.
// - v1.0.0: 실제 InputAction/IMC 자산을 생성하거나 저장하지 않고 Pawn Start/Stop wrapper의 Runtime 미준비, 중복 요청, 정상 Start/Stop과 scanner-less 미지원 안전 처리를 검증.
// Migration:
// - P0 기본 조작은 V 1회 입력 → VehicleSensorComp ActiveScanDurationSec 자동 실행/종료다.
// - 별도 Stop 키는 만들지 않으며 RequestStopActiveScan은 시스템/장비 전환용 Gameplay command로 유지한다.
// - InputAsset 테스트는 `/Game/CarFight/Input/IA_ActiveScan`과 기존 `IMC_Vehicle_Default`의 V 매핑만 idempotent하게 생성/보정하고 저장한다.
// - P0-02 SensorData Apply/Contact lifecycle 계약을 재구현하지 않고 Pawn→Sensor command 경계를 보존한다.

#include "CFVehiclePawn.h"
#include "CFVehicleSensorComp.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetRegistry/AssetRegistryModule.h"
#include "EnhancedActionKeyMapping.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputTriggers.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

namespace
{
	// P0 Active Scan InputAction 패키지 경로입니다.
	const TCHAR* ActiveScanPackageName = TEXT("/Game/CarFight/Input/IA_ActiveScan");

	// P0 Active Scan InputAction 자산 이름입니다.
	const TCHAR* ActiveScanAssetName = TEXT("IA_ActiveScan");

	// 기존 차량 기본 Enhanced Input Mapping Context 경로입니다.
	const TCHAR* DefaultMappingContextPath = TEXT("/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default");

	// P0 Active Scan의 단일 키보드 기본 키입니다.
	const FKey ActiveScanDefaultKey = EKeys::V;

	// InputAction 또는 InputMappingContext package를 현재 Content 경로에 저장합니다.
	bool SaveInputAsset(UObject* AssetObject)
	{
		if (!AssetObject)
		{
			return false;
		}

		// 저장할 자산을 소유하는 Unreal package입니다.
		UPackage* AssetPackage = AssetObject->GetOutermost();
		if (!AssetPackage)
		{
			return false;
		}

		// Long package name을 실제 .uasset 저장 파일 경로로 변환한 값입니다.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			AssetPackage->GetName(),
			FPackageName::GetAssetPackageExtension());

		// P0 Input asset을 public standalone 자산으로 저장할 Unreal SavePackage 인수입니다.
		FSavePackageArgs SavePackageArgs;
		SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SavePackageArgs.SaveFlags = SAVE_NoError;
		return UPackage::SavePackage(AssetPackage, AssetObject, *PackageFilename, SavePackageArgs);
	}

	// IA_ActiveScan을 Boolean + ConsumeInput + Pressed 의미로 생성하거나 현재 자산을 필요한 최소 범위만 보정합니다.
	UInputAction* EnsureActiveScanInputAction()
	{
		// IA_ActiveScan의 전체 UObject 경로입니다.
		const FString ActiveScanObjectPath = FString::Printf(TEXT("%s.%s"), ActiveScanPackageName, ActiveScanAssetName);

		// 이미 존재하면 재사용하고 없으면 새로 생성할 Active Scan InputAction입니다.
		UInputAction* ActiveScanInputAction = LoadObject<UInputAction>(nullptr, *ActiveScanObjectPath);

		// 현재 자산을 실제 저장해야 하는 변경이 발생했는지 여부입니다.
		bool bInputActionChanged = false;

		if (!ActiveScanInputAction)
		{
			// 새 IA_ActiveScan을 소유할 Content package입니다.
			UPackage* ActiveScanPackage = CreatePackage(ActiveScanPackageName);
			if (!ActiveScanPackage)
			{
				return nullptr;
			}

			ActiveScanInputAction = NewObject<UInputAction>(
				ActiveScanPackage,
				FName(ActiveScanAssetName),
				RF_Public | RF_Standalone | RF_Transactional);
			if (!ActiveScanInputAction)
			{
				return nullptr;
			}

			FAssetRegistryModule::AssetCreated(ActiveScanInputAction);
			bInputActionChanged = true;
		}

		if (ActiveScanInputAction->ValueType != EInputActionValueType::Boolean)
		{
			ActiveScanInputAction->ValueType = EInputActionValueType::Boolean;
			bInputActionChanged = true;
		}

		if (!ActiveScanInputAction->bConsumeInput)
		{
			ActiveScanInputAction->bConsumeInput = true;
			bInputActionChanged = true;
		}

		// Pressed trigger가 이미 존재하는지 여부입니다.
		bool bHasPressedTrigger = false;
		for (const UInputTrigger* InputTrigger : ActiveScanInputAction->Triggers)
		{
			if (InputTrigger && InputTrigger->IsA<UInputTriggerPressed>())
			{
				bHasPressedTrigger = true;
				break;
			}
		}

		if (!bHasPressedTrigger)
		{
			ActiveScanInputAction->Triggers.Add(NewObject<UInputTriggerPressed>(ActiveScanInputAction));
			bInputActionChanged = true;
		}

		if (bInputActionChanged)
		{
			ActiveScanInputAction->MarkPackageDirty();
			if (!SaveInputAsset(ActiveScanInputAction))
			{
				return nullptr;
			}
		}

		return ActiveScanInputAction;
	}

	// 지정 InputAction과 Key 조합이 현재 Mapping Context에 이미 존재하는지 반환합니다.
	bool HasInputMapping(const UInputMappingContext* MappingContext, const UInputAction* InputAction, const FKey& MappingKey)
	{
		if (!MappingContext || !InputAction)
		{
			return false;
		}

		for (const FEnhancedActionKeyMapping& ExistingMapping : MappingContext->GetMappings())
		{
			if (ExistingMapping.Action == InputAction && ExistingMapping.Key == MappingKey)
			{
				return true;
			}
		}
		return false;
	}

	// V 키가 다른 Action에 점유되지 않았는지 검사하고 IA_ActiveScan 단일 매핑을 추가·저장합니다.
	bool EnsureActiveScanInputMapping(UInputMappingContext* MappingContext, UInputAction* ActiveScanInputAction)
	{
		if (!MappingContext || !ActiveScanInputAction)
		{
			return false;
		}

		for (const FEnhancedActionKeyMapping& ExistingMapping : MappingContext->GetMappings())
		{
			if (ExistingMapping.Key == ActiveScanDefaultKey
				&& ExistingMapping.Action != ActiveScanInputAction)
			{
				return false;
			}
		}

		if (HasInputMapping(MappingContext, ActiveScanInputAction, ActiveScanDefaultKey))
		{
			return true;
		}

		MappingContext->Modify();
		MappingContext->MapKey(ActiveScanInputAction, ActiveScanDefaultKey);
		MappingContext->MarkPackageDirty();
		return SaveInputAsset(MappingContext);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFScannerInputAssetTest,
	"CarFight.Scanner.SCAN_P0_03.InputAsset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// IA_ActiveScan과 V 기본 매핑, Pawn 기본 로드 계약을 실제 Content 자산에 대해 검증합니다.
bool FCFScannerInputAssetTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// 생성 또는 로드된 실제 P0 Active Scan InputAction입니다.
	UInputAction* ActiveScanInputAction = EnsureActiveScanInputAction();
	TestNotNull(TEXT("IA_ActiveScan 생성 또는 로드"), ActiveScanInputAction);
	if (!ActiveScanInputAction)
	{
		return false;
	}

	// 기존 차량 기본 Enhanced Input Mapping Context입니다.
	UInputMappingContext* DefaultMappingContext = LoadObject<UInputMappingContext>(nullptr, DefaultMappingContextPath);
	TestNotNull(TEXT("IMC_Vehicle_Default 로드"), DefaultMappingContext);
	if (!DefaultMappingContext)
	{
		return false;
	}

	TestEqual(TEXT("IA_ActiveScan 값 타입 Boolean"), ActiveScanInputAction->ValueType, EInputActionValueType::Boolean);
	TestTrue(TEXT("IA_ActiveScan ConsumeInput 활성"), ActiveScanInputAction->bConsumeInput);

	// IA_ActiveScan에 Pressed trigger가 존재하는지 여부입니다.
	bool bHasPressedTrigger = false;
	for (const UInputTrigger* InputTrigger : ActiveScanInputAction->Triggers)
	{
		if (InputTrigger && InputTrigger->IsA<UInputTriggerPressed>())
		{
			bHasPressedTrigger = true;
			break;
		}
	}
	TestTrue(TEXT("IA_ActiveScan Pressed Trigger 존재"), bHasPressedTrigger);

	TestTrue(TEXT("IA_ActiveScan V 매핑 저장"), EnsureActiveScanInputMapping(DefaultMappingContext, ActiveScanInputAction));
	TestTrue(TEXT("IA_ActiveScan 기본 키 V"), HasInputMapping(DefaultMappingContext, ActiveScanInputAction, ActiveScanDefaultKey));

	// BP 조립 없이 C++ 기본 Pawn을 생성할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	TestNotNull(TEXT("Scanner InputAsset 테스트 World 생성"), TestWorld);
	if (!TestWorld)
	{
		return false;
	}

	// C++ 기본 Pawn에는 SM_Body가 없으므로 의도된 기존 경고 1회를 허용합니다.
	AddExpectedError(
		TEXT("VehicleVisualHitCollision: SM_Body component missing on ScannerInputAssetVehiclePawn."),
		EAutomationExpectedErrorFlags::Contains,
		1);

	// IA_ActiveScan 기본 로드 계약을 검증할 C++ 기본 차량 Pawn입니다.
	FActorSpawnParameters PawnSpawnParameters;
	PawnSpawnParameters.Name = TEXT("ScannerInputAssetVehiclePawn");
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		PawnSpawnParameters);
	TestNotNull(TEXT("Scanner InputAsset 차량 Pawn 생성"), VehiclePawn);
	if (!VehiclePawn)
	{
		return false;
	}

	TestEqual(TEXT("Pawn 기본 Active Scan InputAction 연결"), VehiclePawn->InputAction_StartActiveScan.Get(), ActiveScanInputAction);
	TestNull(TEXT("P0 별도 Stop InputAction 기본 미지정"), VehiclePawn->InputAction_StopActiveScan.Get());

	VehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFScannerInputCommandTest,
	"CarFight.Scanner.SCAN_P0_03.InputCommand",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// 실제 키 입력과 분리해 Pawn→Sensor Start/Stop Gameplay command의 안전 상태 전이를 asset-free로 검증합니다.
bool FCFScannerInputCommandTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// asset-free Pawn command 검증에 사용할 transient Editor World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Scanner Input 테스트 World 생성"), TestWorld))
	{
		return false;
	}

	// BP 조립 없이 C++ 기본 Pawn만 생성하므로 SM_Body 누락 로그 1회는 이 테스트의 의도된 전제입니다.
	AddExpectedError(
		TEXT("VehicleVisualHitCollision: SM_Body component missing on ScannerInputVehiclePawn."),
		EAutomationExpectedErrorFlags::Contains,
		1);

	// C++ 기본 Sensor subobject와 Pawn command wrapper를 검증할 차량입니다.
	FActorSpawnParameters PawnSpawnParameters;
	PawnSpawnParameters.Name = TEXT("ScannerInputVehiclePawn");
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		PawnSpawnParameters);
	if (!TestNotNull(TEXT("Scanner Input 차량 Pawn 생성"), VehiclePawn))
	{
		return false;
	}

	// Pawn이 기본 서브오브젝트로 소유해야 하는 Sensor Runtime입니다.
	UCFVehicleSensorComp* SensorComponent = VehiclePawn->GetVehicleSensorComp();
	if (!TestNotNull(TEXT("Pawn VehicleSensorComp 존재"), SensorComponent))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// Runtime 준비 전 Start command가 암묵적으로 Sensor Runtime을 초기화하지 않는지 검증합니다.
	TestFalse(TEXT("Runtime 미준비 Start 요청 거부"), VehiclePawn->RequestStartActiveScan());
	TestFalse(TEXT("Start 요청이 Runtime을 암묵 초기화하지 않음"), SensorComponent->IsSensorRuntimeReady());
	TestNull(TEXT("Runtime 미준비 Start 요청이 SensorData를 변경하지 않음"), SensorComponent->SensorData.Get());

	// Start/Stop command 의미를 검증하기 위한 asset-free 유효 Fallback SensorConfig입니다.
	SensorComponent->FallbackSensorConfig.PassiveDetectionRangeCm = 1000.0f;
	SensorComponent->FallbackSensorConfig.ActiveScanRangeCm = 5000.0f;
	SensorComponent->FallbackSensorConfig.VisualDetectionRangeCm = 1000.0f;
	SensorComponent->FallbackSensorConfig.UpdateIntervalSec = 0.1f;
	SensorComponent->FallbackSensorConfig.MaxActorScansPerUpdate = 64;
	SensorComponent->FallbackSensorConfig.ContactMemoryTimeSec = 10.0f;
	SensorComponent->FallbackSensorConfig.DestroyedHoldTimeSec = 1.0f;
	SensorComponent->FallbackSensorConfig.ActiveScanDurationSec = 3.0f;
	SensorComponent->FallbackSensorConfig.AnalysisGainPerSec = 0.5f;
	SensorComponent->FallbackSensorConfig.AnalysisDecayPerSec = 0.2f;
	SensorComponent->FallbackSensorConfig.IdentifiedThreshold = 0.5f;
	SensorComponent->FallbackSensorConfig.DetailedScanThreshold = 1.0f;

	TestTrue(TEXT("Active Scan 검증용 Fallback Config 유효"), SensorComponent->FallbackSensorConfig.IsValid());
	TestTrue(TEXT("Fallback Sensor Runtime 명시 초기화"), SensorComponent->InitializeSensorRuntime());
	TestTrue(TEXT("명시 초기화 뒤 Sensor Runtime Ready"), SensorComponent->IsSensorRuntimeReady());

	// command 전후 SensorData Source가 바뀌지 않는지 비교할 원본 포인터입니다.
	UCFVehicleSensorData* SensorDataBeforeCommands = SensorComponent->SensorData.Get();

	// command 전후 Applied/Fallback Config가 재적용되지 않는지 비교할 원본 Config입니다.
	const FCFSensorConfig ResolvedConfigBeforeCommands = SensorComponent->GetResolvedSensorConfig();

	TestTrue(TEXT("Pawn Start Active Scan 요청 승인"), VehiclePawn->RequestStartActiveScan());
	TestTrue(TEXT("Start 요청 뒤 Active Scan 실행"), SensorComponent->IsActiveScanRunning());
	TestTrue(TEXT("Start 요청 뒤 Duration 3초 적용"), FMath::IsNearlyEqual(SensorComponent->GetActiveScanRemainingSeconds(), 3.0f));

	// 중복 Start는 Sensor Runtime owner의 기존 안전 계약대로 거부되고 남은 시간을 연장하지 않아야 합니다.
	const float RemainingBeforeDuplicateStart = SensorComponent->GetActiveScanRemainingSeconds();
	TestFalse(TEXT("실행 중 중복 Start 요청 거부"), VehiclePawn->RequestStartActiveScan());
	TestTrue(TEXT("중복 Start가 Remaining을 늘리지 않음"), FMath::IsNearlyEqual(SensorComponent->GetActiveScanRemainingSeconds(), RemainingBeforeDuplicateStart));

	TestTrue(TEXT("Pawn Stop Active Scan 요청 승인"), VehiclePawn->RequestStopActiveScan());
	TestFalse(TEXT("Stop 요청 뒤 Active Scan 종료"), SensorComponent->IsActiveScanRunning());
	TestTrue(TEXT("Stop 요청 뒤 Remaining 0"), FMath::IsNearlyZero(SensorComponent->GetActiveScanRemainingSeconds()));
	TestFalse(TEXT("실행 중 Scan 없을 때 중복 Stop 요청 거부"), VehiclePawn->RequestStopActiveScan());

	TestEqual(TEXT("Start/Stop command가 SensorData Source를 재적용하지 않음"), SensorComponent->SensorData.Get(), SensorDataBeforeCommands);
	TestTrue(TEXT("Start/Stop command가 Passive Config를 변경하지 않음"), FMath::IsNearlyEqual(SensorComponent->GetResolvedSensorConfig().PassiveDetectionRangeCm, ResolvedConfigBeforeCommands.PassiveDetectionRangeCm));
	TestTrue(TEXT("Start/Stop command가 Active Range Config를 변경하지 않음"), FMath::IsNearlyEqual(SensorComponent->GetResolvedSensorConfig().ActiveScanRangeCm, ResolvedConfigBeforeCommands.ActiveScanRangeCm));
	TestTrue(TEXT("Start/Stop command가 Active Duration Config를 변경하지 않음"), FMath::IsNearlyEqual(SensorComponent->GetResolvedSensorConfig().ActiveScanDurationSec, ResolvedConfigBeforeCommands.ActiveScanDurationSec));

	// scanner-less 0-range 상태에서 Start가 안전하게 거부되는지 검증하기 위해 Runtime을 명시적으로 초기화 해제합니다.
	SensorComponent->ResetSensorRuntime();
	SensorComponent->FallbackSensorConfig.PassiveDetectionRangeCm = 0.0f;
	SensorComponent->FallbackSensorConfig.ActiveScanRangeCm = 0.0f;
	SensorComponent->FallbackSensorConfig.VisualDetectionRangeCm = 0.0f;
	SensorComponent->FallbackSensorConfig.ActiveScanDurationSec = 0.0f;
	TestTrue(TEXT("scanner-less 0-range Fallback 계약 유효"), SensorComponent->FallbackSensorConfig.IsValid());
	TestTrue(TEXT("scanner-less Runtime 명시 초기화"), SensorComponent->InitializeSensorRuntime());
	TestFalse(TEXT("scanner-less Active Scan Start 거부"), VehiclePawn->RequestStartActiveScan());
	TestFalse(TEXT("scanner-less Start 거부 뒤 Active Scan 비실행"), SensorComponent->IsActiveScanRunning());
	TestTrue(TEXT("scanner-less Start 거부 뒤 Remaining 0"), FMath::IsNearlyZero(SensorComponent->GetActiveScanRemainingSeconds()));

	VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
