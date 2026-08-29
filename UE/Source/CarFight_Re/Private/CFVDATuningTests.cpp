// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-28
// Description: CF-FQ-015 VehicleData + WSA-P0-04 Wheel Size authority Validator/Runtime 계약 자동화 테스트
// Scope: Transient UCFVehicleData만 사용해 Movement flag, 피팅 질량, MountProfile↔Hardpoint와 Wheel auto-scale 검증 계약을 고정합니다.
// Changelog:
// - v1.3.0: SocketScale/Legacy AutoScale conflict, invalid X/Z scale, axle mismatch와 Socket mode Legacy clamp 비적용 검증 추가.
// - v1.2.0: 실제 ACFVehiclePawn의 ApplyVehicleDataConfig를 호출해 bUseMovementOverrides=false에서도 핵심 Movement 값이 적용되고 DriveState override on/off가 보존되는 VD-P0-03 RuntimeApplyContract를 추가.
// - v1.1.0: CarFight.VehicleData.VD_P0_02.RepresentativeCompare를 추가해 Movement/WheelVisual/Fitting mass 비교 FieldPath를 결정론적으로 검증.
// - v1.0.0: CarFight.VehicleData.VD_P0_01.ValidatorContract 최초 추가.
// Migration:
// - VD-P0-03은 Transient World/Pawn/DataAsset만 사용하며 Wheel Class CDO 전역 튜닝을 피하기 위해 bUseMovementOverrides=false로 검증합니다.
// - Content Asset을 로드·수정·저장하지 않습니다.
// - 실제 Engine/Wheel/DriveState 튜닝값의 좋고 나쁨은 판정하지 않으며 USER 주행감 검증을 대체하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVDAValidator.h"
#include "CFVehicleData.h"
#include "CFVehicleDriveComp.h"
#include "CFVehiclePawn.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	// [v1.0.0] 리포트에서 지정 FieldPath와 Severity를 가진 항목이 존재하는지 확인합니다.
	bool HasValidationItem(
		const FCFVDAValidationReport& ValidationReport,
		const FString& ExpectedFieldPath,
		const ECFVDASeverity ExpectedSeverity)
	{
		for (const FCFVDAValidationItem& ValidationItem : ValidationReport.Items)
		{
			if (ValidationItem.FieldPath == ExpectedFieldPath && ValidationItem.Severity == ExpectedSeverity)
			{
				return true;
			}
		}

		return false;
	}

	// [v1.0.0] 지정 LocationSlotId를 가진 테스트 HardpointSlot을 VehicleData에 추가합니다.
	void AddTestHardpoint(UCFVehicleData* VehicleData, const FName LocationSlotId)
	{
		if (!VehicleData)
		{
			return;
		}

		// [v1.0.0] MountProfile 참조 정합성 검사에 사용할 하드포인트 슬롯입니다.
		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = LocationSlotId;
		HardpointSlot.LocationCategory = TEXT("Top");
		HardpointSlot.LocalLocation = FVector(100.0f, 0.0f, 100.0f);
		VehicleData->HardpointSlots.Add(HardpointSlot);
	}

	// [v1.0.0] 지정 ID와 LocationSlotRef를 가진 테스트 MountProfile을 VehicleData에 추가합니다.
	void AddTestMountProfile(UCFVehicleData* VehicleData, const FName MountProfileId, const FName LocationSlotRef)
	{
		if (!VehicleData)
		{
			return;
		}

		// [v1.0.0] 안정 ID와 하드포인트 참조 검사에 사용할 장착 프로파일입니다.
		FCFVehicleMountProfile MountProfile;
		MountProfile.MountProfileId = MountProfileId;
		MountProfile.LocationSlotRef = LocationSlotRef;
		MountProfile.MountType = ECFVehicleMountType::Turret;
		MountProfile.SizeLimit = ECFVehicleWeaponSize::Large;
		VehicleData->MountProfiles.Add(MountProfile);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVDATuningValidatorContractTest,
	"CarFight.VehicleData.VD_P0_01.ValidatorContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] VD-P0-01이 정의한 현재 VehicleData 검증 계약을 Transient DataAsset으로 확인합니다.
bool FCFVDATuningValidatorContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 각 독립 검증 시나리오에서 재사용할 Transient VehicleData입니다.
	UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(GetTransientPackage(), TEXT("DA_VD_P0_01_Transient"));
	if (!TestNotNull(TEXT("Transient VehicleData 생성"), VehicleData))
	{
		return false;
	}

	// [v1.0.0] 기본 0/0 질량이 레거시 미설정 Info로 허용되는지 확인한 결과입니다.
	FCFVDAValidationReport MassReport = UCFVDAValidator::ValidateFittingMassConfig(VehicleData);
	TestEqual(TEXT("0/0 질량 Error 없음"), MassReport.ErrorCount, 0);
	TestTrue(TEXT("0/0 질량은 미설정 Info"), HasValidationItem(MassReport, TEXT("BaseVehicleMassKg / MaximumGrossMassKg"), ECFVDASeverity::Info));

	VehicleData->BaseVehicleMassKg = 1000.0f;
	VehicleData->MaximumGrossMassKg = 0.0f;
	MassReport = UCFVDAValidator::ValidateFittingMassConfig(VehicleData);
	TestTrue(TEXT("한쪽만 설정된 질량 Error"), HasValidationItem(MassReport, TEXT("BaseVehicleMassKg / MaximumGrossMassKg"), ECFVDASeverity::Error));

	VehicleData->MaximumGrossMassKg = 900.0f;
	MassReport = UCFVDAValidator::ValidateFittingMassConfig(VehicleData);
	TestTrue(TEXT("MaximumGross < Base Error"), HasValidationItem(MassReport, TEXT("MaximumGrossMassKg"), ECFVDASeverity::Error));

	VehicleData->MaximumGrossMassKg = 1500.0f;
	MassReport = UCFVDAValidator::ValidateFittingMassConfig(VehicleData);
	TestEqual(TEXT("정상 질량 Error 0"), MassReport.ErrorCount, 0);

	VehicleData->HardpointSlots.Reset();
	VehicleData->MountProfiles.Reset();
	// [v1.0.0] MountProfiles 빈 배열이 장비 없는 차량에 허용되는지 확인한 결과입니다.
	FCFVDAValidationReport MountReport = UCFVDAValidator::ValidateMountProfiles(VehicleData);
	TestEqual(TEXT("빈 MountProfiles Error 없음"), MountReport.ErrorCount, 0);
	TestTrue(TEXT("빈 MountProfiles Info"), HasValidationItem(MountReport, TEXT("MountProfiles"), ECFVDASeverity::Info));

	AddTestHardpoint(VehicleData, TEXT("Top_01"));
	AddTestMountProfile(VehicleData, TEXT("RoofTurret"), TEXT("Top_01"));
	MountReport = UCFVDAValidator::ValidateMountProfiles(VehicleData);
	TestEqual(TEXT("정상 MountProfile Error 0"), MountReport.ErrorCount, 0);

	VehicleData->MountProfiles[0].MountProfileId = NAME_None;
	MountReport = UCFVDAValidator::ValidateMountProfiles(VehicleData);
	TestTrue(TEXT("MountProfileId None Error"), HasValidationItem(MountReport, TEXT("MountProfiles[0].MountProfileId"), ECFVDASeverity::Error));
	VehicleData->MountProfiles[0].MountProfileId = TEXT("RoofTurret");

	AddTestMountProfile(VehicleData, TEXT("RoofTurret"), TEXT("Top_01"));
	MountReport = UCFVDAValidator::ValidateMountProfiles(VehicleData);
	TestTrue(TEXT("중복 MountProfileId Error"), HasValidationItem(MountReport, TEXT("MountProfiles[1].MountProfileId"), ECFVDASeverity::Error));
	VehicleData->MountProfiles.RemoveAt(1);

	VehicleData->MountProfiles[0].LocationSlotRef = TEXT("MissingSlot");
	MountReport = UCFVDAValidator::ValidateMountProfiles(VehicleData);
	TestTrue(TEXT("알 수 없는 LocationSlotRef Error"), HasValidationItem(MountReport, TEXT("MountProfiles[0].LocationSlotRef"), ECFVDASeverity::Error));
	VehicleData->MountProfiles[0].LocationSlotRef = TEXT("Top_01");

	VehicleData->VehicleMovementConfig.bUseMovementOverrides = false;
	VehicleData->VehicleMovementConfig.ThrottleInputScale = 0.0f;
	// [v1.0.0] Movement flag가 꺼졌을 때 ThrottleInputScale 미사용값을 Error로 오판하지 않는지 확인한 결과입니다.
	FCFVDAValidationReport MovementReport = UCFVDAValidator::ValidateMovementConfig(VehicleData);
	TestEqual(TEXT("Movement override false에서 Throttle 0 Error 없음"), MovementReport.ErrorCount, 0);
	TestTrue(TEXT("Movement override false는 Info 안내"), HasValidationItem(MovementReport, TEXT("VehicleMovementConfig.bUseMovementOverrides"), ECFVDASeverity::Info));

	VehicleData->VehicleMovementConfig.bUseMovementOverrides = true;
	MovementReport = UCFVDAValidator::ValidateMovementConfig(VehicleData);
	TestTrue(TEXT("Movement override true에서 Throttle 0 Error"), HasValidationItem(MovementReport, TEXT("VehicleMovementConfig.ThrottleInputScale"), ECFVDASeverity::Error));
	VehicleData->VehicleMovementConfig.ThrottleInputScale = 1.0f;

	VehicleData->WheelVisualConfig.bUseWheelVisualOverrides = true;
	VehicleData->WheelVisualConfig.bAutoScaleWheelMeshToRadius = true;
	VehicleData->WheelVisualConfig.WheelMeshScaleClampMin = 0.25f;
	VehicleData->WheelVisualConfig.WheelMeshScaleClampMax = 4.0f;
	VehicleData->VehicleMovementConfig.FrontWheelRadius = 35.0f;
	VehicleData->VehicleMovementConfig.RearWheelRadius = 35.0f;
	// [v1.0.0] 정상 Wheel auto-scale 설정 검사 결과입니다.
	FCFVDAValidationReport WheelVisualReport = UCFVDAValidator::ValidateWheelVisualConfig(VehicleData);
	TestEqual(TEXT("정상 Wheel auto-scale Error 0"), WheelVisualReport.ErrorCount, 0);

	VehicleData->WheelVisualConfig.WheelMeshScaleClampMin = 5.0f;
	VehicleData->WheelVisualConfig.WheelMeshScaleClampMax = 4.0f;
	WheelVisualReport = UCFVDAValidator::ValidateWheelVisualConfig(VehicleData);
	TestTrue(TEXT("Wheel auto-scale Min > Max Error"), HasValidationItem(WheelVisualReport, TEXT("WheelVisualConfig.WheelMeshScaleClampMin / WheelMeshScaleClampMax"), ECFVDASeverity::Error));

	VehicleData->WheelVisualConfig.WheelMeshScaleClampMin = 0.25f;
	VehicleData->VehicleMovementConfig.FrontWheelRadius = 0.0f;
	WheelVisualReport = UCFVDAValidator::ValidateWheelVisualConfig(VehicleData);
	TestTrue(TEXT("Auto-scale FrontWheelRadius 0 Error"), HasValidationItem(WheelVisualReport, TEXT("VehicleMovementConfig.FrontWheelRadius"), ECFVDASeverity::Error));

	// WSA Socket Scale과 Legacy AutoScale의 동시 활성화는 두 authority가 충돌하므로 명시 Error입니다.
	VehicleData->WheelVisualConfig.bUseWheelSocketScale = true;
	WheelVisualReport = UCFVDAValidator::ValidateWheelVisualConfig(VehicleData);
	TestTrue(TEXT("Socket Scale + Legacy AutoScale conflict Error"), HasValidationItem(WheelVisualReport, TEXT("WheelVisualConfig.bUseWheelSocketScale / bAutoScaleWheelMeshToRadius"), ECFVDASeverity::Error));

	// Socket mode에서는 Legacy clamp/radius를 사용하지 않고 authored Socket Scale만 검증합니다.
	VehicleData->WheelVisualConfig.bAutoScaleWheelMeshToRadius = false;
	VehicleData->WheelVisualConfig.WheelMeshScaleClampMin = 0.0f;
	VehicleData->WheelVisualConfig.WheelMeshScaleClampMax = 0.0f;
	VehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeScale = FVector(0.72, 1.12, 0.72);
	VehicleData->VehicleLayoutConfig.WheelAnchorFR.RelativeScale = FVector(0.72, 1.12, 0.72);
	VehicleData->VehicleLayoutConfig.WheelAnchorRL.RelativeScale = FVector(0.80, 1.00, 0.80);
	VehicleData->VehicleLayoutConfig.WheelAnchorRR.RelativeScale = FVector(0.80, 1.00, 0.80);
	WheelVisualReport = UCFVDAValidator::ValidateWheelVisualConfig(VehicleData);
	TestFalse(TEXT("Socket mode ignores unused legacy clamp min"), HasValidationItem(WheelVisualReport, TEXT("WheelVisualConfig.WheelMeshScaleClampMin"), ECFVDASeverity::Error));
	TestFalse(TEXT("Socket mode ignores legacy FrontWheelRadius validation"), HasValidationItem(WheelVisualReport, TEXT("VehicleMovementConfig.FrontWheelRadius"), ECFVDASeverity::Error));

	VehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeScale = FVector(0.72, 1.12, 0.70);
	WheelVisualReport = UCFVDAValidator::ValidateWheelVisualConfig(VehicleData);
	TestTrue(TEXT("Socket mode X/Z mismatch Error"), HasValidationItem(WheelVisualReport, TEXT("VehicleLayoutConfig.WheelAnchorFL.RelativeScale"), ECFVDASeverity::Error));

	VehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeScale = FVector(0.72, 1.12, 0.72);
	VehicleData->VehicleLayoutConfig.WheelAnchorFR.RelativeScale = FVector(0.72, 1.20, 0.72);
	WheelVisualReport = UCFVDAValidator::ValidateWheelVisualConfig(VehicleData);
	TestTrue(TEXT("Socket mode front axle mismatch Error"), HasValidationItem(WheelVisualReport, TEXT("VehicleLayoutConfig.WheelAnchorFL/FR.RelativeScale"), ECFVDASeverity::Error));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVDATuningRepresentativeCompareTest,
	"CarFight.VehicleData.VD_P0_02.RepresentativeCompare",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.1.0] Sedan 기준/SUV 비교에서 필요한 핵심 VehicleData 차이가 결정론적 FieldPath로 보고되는지 검증합니다.
bool FCFVDATuningRepresentativeCompareTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.1.0] 기준 Sedan 성격의 비교 원본으로 사용할 Transient VehicleData입니다.
	UCFVehicleData* SourceSedanData = NewObject<UCFVehicleData>(GetTransientPackage(), TEXT("DA_VD_P0_02_SourceSedan"));
	// [v1.1.0] 비교 대상 SUV 성격으로 사용할 Transient VehicleData입니다.
	UCFVehicleData* TargetSUVData = NewObject<UCFVehicleData>(GetTransientPackage(), TEXT("DA_VD_P0_02_TargetSUV"));
	if (!TestNotNull(TEXT("Representative Source Sedan 생성"), SourceSedanData)
		|| !TestNotNull(TEXT("Representative Target SUV 생성"), TargetSUVData))
	{
		return false;
	}

	SourceSedanData->VehicleMovementConfig.MovementProfileName = TEXT("SedanBaseline");
	SourceSedanData->VehicleMovementConfig.bUseMovementOverrides = true;
	SourceSedanData->VehicleMovementConfig.ThrottleInputScale = 0.60f;
	SourceSedanData->VehicleMovementConfig.FrontWheelMaxSteerAngle = 37.0f;
	SourceSedanData->VehicleMovementConfig.FrontWheelRadius = 35.0f;
	SourceSedanData->VehicleMovementConfig.RearWheelRadius = 35.0f;
	SourceSedanData->VehicleMovementConfig.FrontWheelWidth = 6.0f;
	SourceSedanData->VehicleMovementConfig.RearWheelWidth = 6.0f;
	SourceSedanData->WheelVisualConfig.bAutoScaleWheelMeshToRadius = true;
	SourceSedanData->WheelVisualConfig.WheelMeshScaleClampMin = 0.25f;
	SourceSedanData->WheelVisualConfig.WheelMeshScaleClampMax = 4.0f;
	SourceSedanData->BaseVehicleMassKg = 1000.0f;
	SourceSedanData->MaximumGrossMassKg = 1500.0f;

	TargetSUVData->VehicleMovementConfig.MovementProfileName = TEXT("SUVBaseline");
	TargetSUVData->VehicleMovementConfig.bUseMovementOverrides = false;
	TargetSUVData->VehicleMovementConfig.ThrottleInputScale = 0.50f;
	TargetSUVData->VehicleMovementConfig.FrontWheelMaxSteerAngle = 33.0f;
	TargetSUVData->VehicleMovementConfig.FrontWheelRadius = 40.0f;
	TargetSUVData->VehicleMovementConfig.RearWheelRadius = 40.0f;
	TargetSUVData->VehicleMovementConfig.FrontWheelWidth = 8.0f;
	TargetSUVData->VehicleMovementConfig.RearWheelWidth = 8.0f;
	TargetSUVData->WheelVisualConfig.bAutoScaleWheelMeshToRadius = false;
	TargetSUVData->WheelVisualConfig.WheelMeshScaleClampMin = 0.50f;
	TargetSUVData->WheelVisualConfig.WheelMeshScaleClampMax = 3.0f;
	TargetSUVData->BaseVehicleMassKg = 1300.0f;
	TargetSUVData->MaximumGrossMassKg = 1800.0f;

	// [v1.1.0] 기준 Sedan과 대상 SUV의 차이를 기존 Validator Compare 계약으로 생성한 리포트입니다.
	const FCFVDAValidationReport CompareReport = UCFVDAValidator::CompareVehicleData(TargetSUVData, SourceSedanData);

	TestTrue(TEXT("MovementProfileName 차이 보고"), HasValidationItem(CompareReport, TEXT("VehicleMovementConfig.MovementProfileName"), ECFVDASeverity::Info));
	TestTrue(TEXT("Movement override 차이 경고"), HasValidationItem(CompareReport, TEXT("VehicleMovementConfig.bUseMovementOverrides"), ECFVDASeverity::Warning));
	TestTrue(TEXT("Throttle 차이 보고"), HasValidationItem(CompareReport, TEXT("VehicleMovementConfig.ThrottleInputScale"), ECFVDASeverity::Info));
	TestTrue(TEXT("FrontWheelWidth 차이 보고"), HasValidationItem(CompareReport, TEXT("VehicleMovementConfig.FrontWheelWidth"), ECFVDASeverity::Info));
	TestTrue(TEXT("RearWheelWidth 차이 보고"), HasValidationItem(CompareReport, TEXT("VehicleMovementConfig.RearWheelWidth"), ECFVDASeverity::Info));
	TestTrue(TEXT("Wheel auto-scale 차이 보고"), HasValidationItem(CompareReport, TEXT("WheelVisualConfig.bAutoScaleWheelMeshToRadius"), ECFVDASeverity::Info));
	TestTrue(TEXT("Wheel scale clamp min 차이 보고"), HasValidationItem(CompareReport, TEXT("WheelVisualConfig.WheelMeshScaleClampMin"), ECFVDASeverity::Info));
	TestTrue(TEXT("Wheel scale clamp max 차이 경고"), HasValidationItem(CompareReport, TEXT("WheelVisualConfig.WheelMeshScaleClampMax"), ECFVDASeverity::Warning));
	TestTrue(TEXT("Base vehicle mass 차이 경고"), HasValidationItem(CompareReport, TEXT("BaseVehicleMassKg"), ECFVDASeverity::Warning));
		TestTrue(TEXT("Maximum gross mass 차이 경고"), HasValidationItem(CompareReport, TEXT("MaximumGrossMassKg"), ECFVDASeverity::Warning));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVDATuningRuntimeApplyTest,
	"CarFight.VehicleData.VD_P0_03.RuntimeApplyContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.2.0] 실제 VehiclePawn의 protected VehicleData 적용 경로가 Movement 본체와 DriveState 설정을 계약대로 전달하는지 검증합니다.
bool FCFVDATuningRuntimeApplyTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AddExpectedError(
		TEXT("VehicleVisualHitCollision: SM_Body component missing on VDP003VehiclePawn."),
		EAutomationExpectedErrorFlags::Contains,
		1);

	// [v1.2.0] Content Asset 없이 실제 CarFight VehiclePawn 구성과 Chaos Movement를 생성할 Transient Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("VD-P0-03 Transient World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.2.0] 실제 ApplyVehicleDataConfig를 실행할 테스트 차량의 결정적 Spawn 설정입니다.
	FActorSpawnParameters PawnSpawnParameters;
	PawnSpawnParameters.Name = TEXT("VDP003VehiclePawn");

	// [v1.2.0] 프로덕션 ACFVehiclePawn 기본 서브오브젝트와 Chaos Movement를 그대로 사용하는 Transient 차량입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		PawnSpawnParameters);
	if (!TestNotNull(TEXT("VD-P0-03 VehiclePawn 생성"), VehiclePawn))
	{
		return false;
	}

	// [v1.2.0] 런타임 적용 결과를 직접 확인할 실제 DriveComp 기본 서브오브젝트입니다.
	UCFVehicleDriveComp* DriveComp = VehiclePawn->GetVehicleDriveComp();
	if (!TestNotNull(TEXT("VD-P0-03 DriveComp 존재"), DriveComp))
	{
		VehiclePawn->Destroy();
		return false;
	}
	TestTrue(TEXT("VD-P0-03 Movement 캐시 성공"), DriveComp->CacheVehicleMovementComponent());

	// [v1.2.0] VehicleData 적용 결과를 직접 읽을 실제 Chaos Wheeled Vehicle Movement 컴포넌트입니다.
	UChaosWheeledVehicleMovementComponent* MovementComponent = DriveComp->GetVehicleMovementComponent();
	if (!TestNotNull(TEXT("VD-P0-03 Chaos Movement 존재"), MovementComponent))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.2.0] 프로덕션 적용 경로에 주입할 Transient VehicleData입니다.
	UCFVehicleData* RuntimeVehicleData = NewObject<UCFVehicleData>(GetTransientPackage(), TEXT("DA_VD_P0_03_Runtime"));
	if (!TestNotNull(TEXT("VD-P0-03 Runtime VehicleData 생성"), RuntimeVehicleData))
	{
		VehiclePawn->Destroy();
		return false;
	}

	RuntimeVehicleData->VehicleMovementConfig.bUseMovementOverrides = false;
	RuntimeVehicleData->VehicleMovementConfig.EngineMaxTorque = 1234.0f;
	RuntimeVehicleData->VehicleMovementConfig.EngineMaxRPM = 6789.0f;
	RuntimeVehicleData->VehicleMovementConfig.DragCoefficient = 0.42f;
	RuntimeVehicleData->VehicleMovementConfig.DownforceCoefficient = 0.73f;
	RuntimeVehicleData->VehicleMovementConfig.DifferentialType = EVehicleDifferential::RearWheelDrive;
	RuntimeVehicleData->VehicleMovementConfig.FrontRearSplit = 0.37f;
	RuntimeVehicleData->VehicleMovementConfig.SteeringType = ESteeringType::AngleRatio;
	RuntimeVehicleData->VehicleMovementConfig.SteeringAngleRatio = 0.61f;
	RuntimeVehicleData->WheelVisualConfig.bUseWheelVisualOverrides = false;

	RuntimeVehicleData->DriveStateConfig.bUseDriveStateOverrides = true;
	RuntimeVehicleData->DriveStateConfig.bEnableDriveStateHysteresis = false;
	RuntimeVehicleData->DriveStateConfig.bUsePerStateHoldTimes = false;
	RuntimeVehicleData->DriveStateConfig.DriveStateMinimumHoldTimeSeconds = 0.31f;
	RuntimeVehicleData->DriveStateConfig.IdleEnterSpeedThresholdKmh = 2.2f;
	RuntimeVehicleData->DriveStateConfig.IdleExitSpeedThresholdKmh = 3.3f;
	RuntimeVehicleData->DriveStateConfig.ReverseEnterSpeedThresholdKmh = 4.4f;
	RuntimeVehicleData->DriveStateConfig.ReverseExitSpeedThresholdKmh = 1.1f;
	RuntimeVehicleData->DriveStateConfig.AirborneMinSpeedThresholdKmh = 7.7f;
	RuntimeVehicleData->DriveStateConfig.AirborneVerticalSpeedThresholdCmPerSec = 222.0f;
	RuntimeVehicleData->DriveStateConfig.ActiveInputThreshold = 0.22f;
	RuntimeVehicleData->DriveStateConfig.bTreatOppositeThrottleAsBrake = false;

	VehiclePawn->VehicleData = RuntimeVehicleData;
	VehiclePawn->ApplyVehicleDataConfig();

	TestTrue(TEXT("Movement override false에서도 EngineMaxTorque 적용"), FMath::IsNearlyEqual(MovementComponent->EngineSetup.MaxTorque, 1234.0f));
	TestTrue(TEXT("Movement override false에서도 EngineMaxRPM 적용"), FMath::IsNearlyEqual(MovementComponent->EngineSetup.MaxRPM, 6789.0f));
	TestTrue(TEXT("Movement override false에서도 Drag 적용"), FMath::IsNearlyEqual(MovementComponent->DragCoefficient, 0.42f));
	TestTrue(TEXT("Movement override false에서도 Downforce 적용"), FMath::IsNearlyEqual(MovementComponent->DownforceCoefficient, 0.73f));
	TestEqual(TEXT("Movement override false에서도 DifferentialType 적용"), MovementComponent->DifferentialSetup.DifferentialType, EVehicleDifferential::RearWheelDrive);
	TestTrue(TEXT("Movement override false에서도 FrontRearSplit 적용"), FMath::IsNearlyEqual(MovementComponent->DifferentialSetup.FrontRearSplit, 0.37f));
	TestEqual(TEXT("Movement override false에서도 SteeringType 적용"), MovementComponent->SteeringSetup.SteeringType, ESteeringType::AngleRatio);
	TestTrue(TEXT("Movement override false에서도 SteeringAngleRatio 적용"), FMath::IsNearlyEqual(MovementComponent->SteeringSetup.AngleRatio, 0.61f));

	TestFalse(TEXT("DriveState 히스테리시스 override 적용"), DriveComp->bEnableDriveStateHysteresis);
	TestFalse(TEXT("DriveState per-state hold override 적용"), DriveComp->bUsePerStateHoldTimes);
	TestTrue(TEXT("DriveState 기본 hold 적용"), FMath::IsNearlyEqual(DriveComp->DriveStateMinimumHoldTimeSeconds, 0.31f));
	TestTrue(TEXT("DriveState IdleEnter 적용"), FMath::IsNearlyEqual(DriveComp->IdleEnterSpeedThresholdKmh, 2.2f));
	TestTrue(TEXT("DriveState IdleExit 적용"), FMath::IsNearlyEqual(DriveComp->IdleExitSpeedThresholdKmh, 3.3f));
	TestTrue(TEXT("DriveState ReverseEnter 적용"), FMath::IsNearlyEqual(DriveComp->ReverseEnterSpeedThresholdKmh, 4.4f));
	TestTrue(TEXT("DriveState ReverseExit 적용"), FMath::IsNearlyEqual(DriveComp->ReverseExitSpeedThresholdKmh, 1.1f));
	TestTrue(TEXT("DriveState AirborneMin 적용"), FMath::IsNearlyEqual(DriveComp->AirborneMinSpeedThresholdKmh, 7.7f));
	TestTrue(TEXT("DriveState AirborneVertical 적용"), FMath::IsNearlyEqual(DriveComp->AirborneVerticalSpeedThresholdCmPerSec, 222.0f));
	TestTrue(TEXT("DriveState ActiveInput 적용"), FMath::IsNearlyEqual(DriveComp->ActiveInputThreshold, 0.22f));
	TestFalse(TEXT("DriveState opposite throttle 정책 적용"), DriveComp->bTreatOppositeThrottleAsBrake);

	DriveComp->IdleEnterSpeedThresholdKmh = 9.9f;
	DriveComp->ActiveInputThreshold = 0.33f;
	DriveComp->bEnableDriveStateHysteresis = true;
	RuntimeVehicleData->DriveStateConfig.bUseDriveStateOverrides = false;
	RuntimeVehicleData->DriveStateConfig.IdleEnterSpeedThresholdKmh = 0.1f;
	RuntimeVehicleData->DriveStateConfig.ActiveInputThreshold = 0.01f;
	RuntimeVehicleData->DriveStateConfig.bEnableDriveStateHysteresis = false;
	VehiclePawn->ApplyVehicleDataConfig();

	TestTrue(TEXT("DriveState override false에서 기존 IdleEnter 유지"), FMath::IsNearlyEqual(DriveComp->IdleEnterSpeedThresholdKmh, 9.9f));
	TestTrue(TEXT("DriveState override false에서 기존 ActiveInput 유지"), FMath::IsNearlyEqual(DriveComp->ActiveInputThreshold, 0.33f));
	TestTrue(TEXT("DriveState override false에서 기존 hysteresis 유지"), DriveComp->bEnableDriveStateHysteresis);

	VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
