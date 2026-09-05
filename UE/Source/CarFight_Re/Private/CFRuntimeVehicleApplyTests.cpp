// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-02
// Description: CF-FQ-041 RTA-P0-02 Vehicle Runtime Apply focused regression
// Scope: Catalog validation, successful transient apply, ApplyFailed recovery, RecoveryFailed 구분을 actual VehiclePawn runtime으로 검증합니다.
// Changelog:
// - v1.0.0: RTA-P0-02 focused Automation 4건 추가.
// Migration:
// - persisted Vehicle/Fitting Asset은 read-only fixture로 사용하며 저장 또는 mutation하지 않습니다.

#include "CFRuntimeVehicleApply.h"

#include "CFRuntimeTestCatalogData.h"
#include "CFRuntimeTestSettings.h"
#include "CFEquipmentPresetData.h"
#include "CFVehicleData.h"
#include "CFVehicleFittingComp.h"
#include "CFVehicleFittingData.h"
#include "CFVehiclePawn.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	// [v1.0.0] Vehicle Apply failure/recovery 테스트용 최소 valid transient Catalog를 생성합니다.
	UCFRuntimeTestCatalogData* BuildTransientVehicleCatalog(UCFVehicleData* CandidateVehicleData)
	{
		// Candidate authorization을 소유할 transient Catalog입니다.
		UCFRuntimeTestCatalogData* RuntimeCatalog = NewObject<UCFRuntimeTestCatalogData>(
			GetTransientPackage(),
			MakeUniqueObjectName(GetTransientPackage(), UCFRuntimeTestCatalogData::StaticClass(), TEXT("RTA_P0_02_Catalog")));

		// Catalog 전체 계약을 valid로 유지할 dummy EquipmentPresetData입니다.
		UCFEquipmentPresetData* DummyEquipmentPresetData = NewObject<UCFEquipmentPresetData>(
			RuntimeCatalog,
			MakeUniqueObjectName(RuntimeCatalog, UCFEquipmentPresetData::StaticClass(), TEXT("RTA_P0_02_Equipment")));

		if (RuntimeCatalog && CandidateVehicleData && DummyEquipmentPresetData)
		{
			RuntimeCatalog->AllowedVehicleData.Add(CandidateVehicleData);
			RuntimeCatalog->AllowedEquipmentPresetData.Add(DummyEquipmentPresetData);
		}

		return RuntimeCatalog;
	}

	// [v1.0.0] Snapshot Initial Mass가 실제 구성된 이전 Runtime을 만들어 mass-change-to-legacy failure/recovery fixture로 사용합니다.
	bool BuildSnapshotRuntimePawn(
		FAutomationTestBase& Test,
		UWorld* TestWorld,
		ACFVehiclePawn*& OutVehiclePawn,
		UCFVehicleData*& OutPreviousVehicleData,
		UCFVehicleFittingData*& OutPreviousVehicleFittingData)
	{
		OutVehiclePawn = nullptr;
		OutPreviousVehicleData = nullptr;
		OutPreviousVehicleFittingData = nullptr;

		// 실제 Blueprint 컴포넌트 구성으로 Spawn할 VehiclePawn class입니다.
		UClass* VehiclePawnClass = LoadClass<ACFVehiclePawn>(
			nullptr,
			TEXT("/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"));

		// 기존 Fitting runtime regression이 사용하는 실제 SUV VehicleData입니다.
		UCFVehicleData* PreviousVehicleData = LoadObject<UCFVehicleData>(
			nullptr,
			TEXT("/Game/CarFight/Vehicles/Data/Defense/DA_VehicleDefense_TestSUV.DA_VehicleDefense_TestSUV"));

		// 1570kg Snapshot mass가 포함된 기존 actual Fitting fixture입니다.
		UCFVehicleFittingData* PreviousVehicleFittingData = LoadObject<UCFVehicleFittingData>(
			nullptr,
			TEXT("/Game/CarFight/Tests/VehicleDefense/Data/DA_Fit_DefenseTestSUV.DA_Fit_DefenseTestSUV"));

		if (!Test.TestNotNull(TEXT("RTA-P0-02 BP_CFVehiclePawn class"), VehiclePawnClass)
			|| !Test.TestNotNull(TEXT("RTA-P0-02 previous VehicleData"), PreviousVehicleData)
			|| !Test.TestNotNull(TEXT("RTA-P0-02 previous VehicleFittingData"), PreviousVehicleFittingData))
		{
			return false;
		}

		// 실제 차량 Pawn을 결정적으로 생성할 Spawn 설정입니다.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 이전 Snapshot runtime을 소유할 actual VehiclePawn입니다.
		ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
			VehiclePawnClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParameters);
		if (!Test.TestNotNull(TEXT("RTA-P0-02 Snapshot VehiclePawn spawn"), VehiclePawn))
		{
			return false;
		}

		VehiclePawn->UnregisterAllComponents();
		VehiclePawn->VehicleData = PreviousVehicleData;
		VehiclePawn->VehicleFittingData = PreviousVehicleFittingData;
		VehiclePawn->bAutoRegisterInputMappingContext = false;
		VehiclePawn->bShowAimReticle = false;
		VehiclePawn->bShowTargetSelectHud = false;

		// 이전 출격 Snapshot state를 fresh하게 구성할 VehicleFittingComp입니다.
		UCFVehicleFittingComp* VehicleFittingComp = VehiclePawn->GetVehicleFittingComp();
		if (!Test.TestNotNull(TEXT("RTA-P0-02 VehicleFittingComp"), VehicleFittingComp))
		{
			VehiclePawn->Destroy();
			return false;
		}

		VehicleFittingComp->ResetFittingRuntimeState();
		VehiclePawn->RegisterAllComponents();

		if (!Test.TestTrue(TEXT("RTA-P0-02 Snapshot Initial Mass configured"), VehicleFittingComp->HasConfiguredInitialMass())
			|| !Test.TestTrue(TEXT("RTA-P0-02 previous Runtime initialize"), VehiclePawn->InitializeVehicleRuntime())
			|| !Test.TestTrue(TEXT("RTA-P0-02 previous Runtime ready readback"), VehiclePawn->GetVehicleDebugRuntime().bRuntimeReady))
		{
			VehiclePawn->Destroy();
			return false;
		}

		OutVehiclePawn = VehiclePawn;
		OutPreviousVehicleData = PreviousVehicleData;
		OutPreviousVehicleFittingData = PreviousVehicleFittingData;
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeVehicleValidationTest,
	"CarFight.RuntimeApply.RTA_P0_02.VehicleValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Catalog 전체 계약과 exact VehicleData membership을 mutation 없이 검증합니다.
bool FCFRuntimeVehicleValidationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// 허용 후보로 등록할 transient VehicleData입니다.
	UCFVehicleData* AllowedVehicleData = NewObject<UCFVehicleData>(GetTransientPackage(), TEXT("RTA_P0_02_AllowedVehicle"));

	// Catalog에 등록하지 않을 transient VehicleData입니다.
	UCFVehicleData* UnregisteredVehicleData = NewObject<UCFVehicleData>(GetTransientPackage(), TEXT("RTA_P0_02_UnregisteredVehicle"));

	// exact membership validation에 사용할 transient Catalog입니다.
	UCFRuntimeTestCatalogData* RuntimeCatalog = BuildTransientVehicleCatalog(AllowedVehicleData);

	if (!TestNotNull(TEXT("RTA-P0-02 Allowed VehicleData"), AllowedVehicleData)
		|| !TestNotNull(TEXT("RTA-P0-02 Unregistered VehicleData"), UnregisteredVehicleData)
		|| !TestNotNull(TEXT("RTA-P0-02 transient Catalog"), RuntimeCatalog))
	{
		return false;
	}

	// validation 실패 사유를 받을 문자열입니다.
	FString ValidationError;
	TestTrue(
		TEXT("등록 VehicleData validation PASS"),
		FCFRuntimeVehicleApplyService::ValidateCatalogVehicleCandidate(RuntimeCatalog, AllowedVehicleData, ValidationError));
	TestTrue(TEXT("등록 VehicleData PASS error empty"), ValidationError.IsEmpty());

	TestFalse(
		TEXT("미등록 VehicleData validation FAIL"),
		FCFRuntimeVehicleApplyService::ValidateCatalogVehicleCandidate(RuntimeCatalog, UnregisteredVehicleData, ValidationError));
	TestTrue(TEXT("미등록 VehicleData 오류에 허용 목록 설명 포함"), ValidationError.Contains(TEXT("허용 목록")));

	RuntimeCatalog->AllowedEquipmentPresetData.Reset();
	TestFalse(
		TEXT("전체 Catalog 계약 invalid면 Vehicle candidate도 FAIL"),
		FCFRuntimeVehicleApplyService::ValidateCatalogVehicleCandidate(RuntimeCatalog, AllowedVehicleData, ValidationError));
	TestTrue(TEXT("invalid Catalog 오류에 계약 설명 포함"), ValidationError.Contains(TEXT("Catalog 계약")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeVehicleSuccessTest,
	"CarFight.RuntimeApply.RTA_P0_02.VehicleApplySuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 실제 기본 Catalog의 두 VehicleData 사이를 same-Pawn transient reinitialize로 성공 전환하는지 검증합니다.
bool FCFRuntimeVehicleSuccessTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// 실제 BP VehiclePawn runtime을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("RTA-P0-02 Success World"), TestWorld))
	{
		return false;
	}

	// PreRegister/Chaos physics 경로를 실제 Game World 의미로 실행하고 함수 종료 시 WorldType을 복원합니다.
	TGuardValue<TEnumAsByte<EWorldType::Type>> WorldTypeGuard(
		TestWorld->WorldType,
		TEnumAsByte<EWorldType::Type>(EWorldType::Game));

	// Config locator를 통해 persisted default Catalog를 읽는 settings CDO입니다.
	const UCFRuntimeTestSettings* RuntimeTestSettings = GetDefault<UCFRuntimeTestSettings>();

	// 실제 packaged selection source와 같은 persisted Runtime Catalog입니다.
	UCFRuntimeTestCatalogData* RuntimeCatalog = RuntimeTestSettings ? RuntimeTestSettings->LoadDefaultCatalog() : nullptr;
	if (!TestNotNull(TEXT("RTA-P0-02 default Catalog"), RuntimeCatalog)
		|| !TestTrue(TEXT("RTA-P0-02 default Catalog Vehicle 2개 이상"), RuntimeCatalog && RuntimeCatalog->AllowedVehicleData.Num() >= 2))
	{
		return false;
	}

	// 이전 runtime으로 먼저 구성할 첫 번째 등록 VehicleData입니다.
	UCFVehicleData* PreviousVehicleData = RuntimeCatalog->AllowedVehicleData[0].Get();

	// service로 적용할 두 번째 등록 VehicleData입니다.
	UCFVehicleData* CandidateVehicleData = RuntimeCatalog->AllowedVehicleData[1].Get();

	// 실제 Blueprint 컴포넌트 계층을 가진 VehiclePawn class입니다.
	UClass* VehiclePawnClass = LoadClass<ACFVehiclePawn>(
		nullptr,
		TEXT("/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn_C"));

	if (!TestNotNull(TEXT("RTA-P0-02 success BP class"), VehiclePawnClass)
		|| !TestNotNull(TEXT("RTA-P0-02 previous catalog VehicleData"), PreviousVehicleData)
		|| !TestNotNull(TEXT("RTA-P0-02 candidate catalog VehicleData"), CandidateVehicleData))
	{
		return false;
	}

	// actual BP Pawn 생성 설정입니다.
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// same-Pawn 전환을 검증할 actual VehiclePawn입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		VehiclePawnClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters);
	if (!TestNotNull(TEXT("RTA-P0-02 success VehiclePawn"), VehiclePawn))
	{
		return false;
	}

	VehiclePawn->UnregisterAllComponents();
	VehiclePawn->VehicleData = PreviousVehicleData;
	VehiclePawn->VehicleFittingData = nullptr;
	VehiclePawn->bAutoRegisterInputMappingContext = false;
	VehiclePawn->bShowAimReticle = false;
	VehiclePawn->bShowTargetSelectHud = false;

	// 이전 legacy runtime state를 fresh하게 시작할 Fitting component입니다.
	UCFVehicleFittingComp* VehicleFittingComp = VehiclePawn->GetVehicleFittingComp();
	if (!TestNotNull(TEXT("RTA-P0-02 success FittingComp"), VehicleFittingComp))
	{
		VehiclePawn->Destroy();
		return false;
	}
	VehicleFittingComp->ResetFittingRuntimeState();
	VehiclePawn->RegisterAllComponents();

	if (!TestTrue(TEXT("RTA-P0-02 previous legacy Runtime initialize"), VehiclePawn->InitializeVehicleRuntime()))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// Catalog authorization + transient duplicate + same-Pawn Initialize 결과입니다.
	const FCFRuntimeVehicleApplyResult ApplyResult =
		FCFRuntimeVehicleApplyService::ApplyCatalogVehicle(VehiclePawn, RuntimeCatalog, CandidateVehicleData);

	TestEqual(TEXT("Vehicle Apply 성공 상태"), ApplyResult.Status, ECFRuntimeVehicleApplyStatus::Succeeded);
	TestTrue(TEXT("Vehicle Apply runtime ready readback"), ApplyResult.bRuntimeReady);
	TestTrue(TEXT("Vehicle Apply transient copy readback"), ApplyResult.bAppliedTransientCopyActive);
	TestFalse(TEXT("Vehicle Apply recovery 미시도"), ApplyResult.bRecoveryAttempted);
	TestTrue(TEXT("요청 path는 persistent candidate"), ApplyResult.RequestedVehicleDataPath == FSoftObjectPath(CandidateVehicleData));
	TestTrue(TEXT("current VehicleData는 원본 Asset 직접 참조가 아님"), VehiclePawn->VehicleData.Get() != CandidateVehicleData);
	TestTrue(TEXT("current VehicleData는 transient flag"), VehiclePawn->VehicleData && VehiclePawn->VehicleData->HasAnyFlags(RF_Transient));
	TestTrue(TEXT("current VehicleData outer는 TransientPackage"), VehiclePawn->VehicleData && VehiclePawn->VehicleData->GetOutermost() == GetTransientPackage());
	TestNull(TEXT("Vehicle Apply는 Step8과 동일하게 Fitting override 비움"), VehiclePawn->VehicleFittingData.Get());

	VehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeVehicleRecoveryTest,
	"CarFight.RuntimeApply.RTA_P0_02.ApplyFailedRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Snapshot Mass가 구성된 Pawn에서 null-Fitting Vehicle Apply가 실패하면 이전 VehicleData/Fitting runtime을 복구하는지 검증합니다.
bool FCFRuntimeVehicleRecoveryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// 실제 Snapshot mass/recovery 경로를 사용할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("RTA-P0-02 Recovery World"), TestWorld))
	{
		return false;
	}

	// PreRegister initial mass 경로를 활성화할 Game World guard입니다.
	TGuardValue<TEnumAsByte<EWorldType::Type>> WorldTypeGuard(
		TestWorld->WorldType,
		TEnumAsByte<EWorldType::Type>(EWorldType::Game));

	// 이전 runtime이 구성된 actual Pawn입니다.
	ACFVehiclePawn* VehiclePawn = nullptr;

	// checkpoint와 recovery가 복원해야 할 이전 VehicleData입니다.
	UCFVehicleData* PreviousVehicleData = nullptr;

	// checkpoint와 recovery가 복원해야 할 이전 VehicleFittingData입니다.
	UCFVehicleFittingData* PreviousVehicleFittingData = nullptr;

	if (!BuildSnapshotRuntimePawn(
		*this,
		TestWorld,
		VehiclePawn,
		PreviousVehicleData,
		PreviousVehicleFittingData))
	{
		return false;
	}

	// configured Snapshot mass 상태에서 Step8 null-Fitting 재적용을 거부시키는 transient candidate입니다.
	UCFVehicleData* CandidateVehicleData = NewObject<UCFVehicleData>(
		GetTransientPackage(),
		TEXT("RTA_P0_02_RecoveryCandidate"));

	// candidate를 명시 허용하는 valid transient Catalog입니다.
	UCFRuntimeTestCatalogData* RuntimeCatalog = BuildTransientVehicleCatalog(CandidateVehicleData);
	if (!TestNotNull(TEXT("RTA-P0-02 recovery Candidate"), CandidateVehicleData)
		|| !TestNotNull(TEXT("RTA-P0-02 recovery Catalog"), RuntimeCatalog))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// 후보 초기화 실패와 이전 checkpoint 복구를 한 operation 결과입니다.
	const FCFRuntimeVehicleApplyResult ApplyResult =
		FCFRuntimeVehicleApplyService::ApplyCatalogVehicle(VehiclePawn, RuntimeCatalog, CandidateVehicleData);

	TestEqual(TEXT("후보 실패 + 복구 성공은 ApplyFailed"), ApplyResult.Status, ECFRuntimeVehicleApplyStatus::ApplyFailed);
	TestTrue(TEXT("ApplyFailed recovery 시도"), ApplyResult.bRecoveryAttempted);
	TestTrue(TEXT("ApplyFailed recovery 성공"), ApplyResult.bRecoverySucceeded);
	TestTrue(TEXT("ApplyFailed 종료 Runtime ready"), ApplyResult.bRuntimeReady);
	TestFalse(TEXT("복구 상태에는 candidate transient active 아님"), ApplyResult.bAppliedTransientCopyActive);
	TestTrue(TEXT("이전 VehicleData pointer 복원"), VehiclePawn->VehicleData.Get() == PreviousVehicleData);
	TestTrue(TEXT("이전 VehicleFittingData pointer 복원"), VehiclePawn->VehicleFittingData.Get() == PreviousVehicleFittingData);
	TestEqual(TEXT("이전 VehicleData path checkpoint"), ApplyResult.PreviousVehicleDataPath, GetPathNameSafe(PreviousVehicleData));
	TestEqual(TEXT("이전 Fitting path checkpoint"), ApplyResult.PreviousVehicleFittingDataPath, GetPathNameSafe(PreviousVehicleFittingData));
	TestEqual(TEXT("복구 후 current VehicleData readback"), ApplyResult.CurrentVehicleDataPath, GetPathNameSafe(PreviousVehicleData));
	TestEqual(TEXT("복구 후 current Fitting readback"), ApplyResult.CurrentVehicleFittingDataPath, GetPathNameSafe(PreviousVehicleFittingData));

	VehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFRuntimeVehicleRecoveryFailTest,
	"CarFight.RuntimeApply.RTA_P0_02.RecoveryFailed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 후보 Apply 실패 뒤 checkpoint Fitting 자체도 재초기화 불가하면 RecoveryFailed로 구분하는지 검증합니다.
bool FCFRuntimeVehicleRecoveryFailTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// 실제 Snapshot mass/recovery failure 경로를 사용할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("RTA-P0-02 RecoveryFailed World"), TestWorld))
	{
		return false;
	}

	// PreRegister initial mass 경로를 활성화할 Game World guard입니다.
	TGuardValue<TEnumAsByte<EWorldType::Type>> WorldTypeGuard(
		TestWorld->WorldType,
		TEnumAsByte<EWorldType::Type>(EWorldType::Game));

	// 먼저 정상 Snapshot runtime을 구성할 actual Pawn입니다.
	ACFVehiclePawn* VehiclePawn = nullptr;

	// 정상 구성에 사용한 이전 VehicleData입니다.
	UCFVehicleData* PreviousVehicleData = nullptr;

	// 정상 구성에 사용한 원래 valid Fitting fixture입니다.
	UCFVehicleFittingData* InitiallyValidVehicleFittingData = nullptr;

	if (!BuildSnapshotRuntimePawn(
		*this,
		TestWorld,
		VehiclePawn,
		PreviousVehicleData,
		InitiallyValidVehicleFittingData))
	{
		return false;
	}

	// 현재 runtime은 ready지만 다음 reinitialize에서 실패하도록 checkpoint pointer에 놓을 invalid transient FittingData입니다.
	UCFVehicleFittingData* InvalidCheckpointFittingData = NewObject<UCFVehicleFittingData>(
		GetTransientPackage(),
		TEXT("RTA_P0_02_InvalidCheckpointFitting"));
	VehiclePawn->VehicleFittingData = InvalidCheckpointFittingData;

	// configured Snapshot mass 상태에서 먼저 candidate Apply를 실패시킬 transient VehicleData입니다.
	UCFVehicleData* CandidateVehicleData = NewObject<UCFVehicleData>(
		GetTransientPackage(),
		TEXT("RTA_P0_02_RecoveryFailCandidate"));

	// candidate authorization을 제공할 valid transient Catalog입니다.
	UCFRuntimeTestCatalogData* RuntimeCatalog = BuildTransientVehicleCatalog(CandidateVehicleData);
	if (!TestNotNull(TEXT("RTA-P0-02 invalid checkpoint Fitting"), InvalidCheckpointFittingData)
		|| !TestNotNull(TEXT("RTA-P0-02 recovery-fail Candidate"), CandidateVehicleData)
		|| !TestNotNull(TEXT("RTA-P0-02 recovery-fail Catalog"), RuntimeCatalog))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// candidate apply 실패 뒤 invalid checkpoint Fitting으로 recovery도 실패한 결과입니다.
	const FCFRuntimeVehicleApplyResult ApplyResult =
		FCFRuntimeVehicleApplyService::ApplyCatalogVehicle(VehiclePawn, RuntimeCatalog, CandidateVehicleData);

	TestEqual(TEXT("복구 재초기화 실패는 RecoveryFailed"), ApplyResult.Status, ECFRuntimeVehicleApplyStatus::RecoveryFailed);
	TestTrue(TEXT("RecoveryFailed recovery 시도"), ApplyResult.bRecoveryAttempted);
	TestFalse(TEXT("RecoveryFailed recovery 성공 false"), ApplyResult.bRecoverySucceeded);
	TestFalse(TEXT("RecoveryFailed final Runtime not ready"), ApplyResult.bRuntimeReady);
	TestTrue(TEXT("RecoveryFailed도 이전 VehicleData pointer는 복원"), VehiclePawn->VehicleData.Get() == PreviousVehicleData);
	TestTrue(TEXT("RecoveryFailed도 checkpoint Fitting pointer는 복원"), VehiclePawn->VehicleFittingData.Get() == InvalidCheckpointFittingData);
	TestEqual(TEXT("RecoveryFailed current VehicleData readback"), ApplyResult.CurrentVehicleDataPath, GetPathNameSafe(PreviousVehicleData));
	TestEqual(TEXT("RecoveryFailed current Fitting readback"), ApplyResult.CurrentVehicleFittingDataPath, GetPathNameSafe(InvalidCheckpointFittingData));

	VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
