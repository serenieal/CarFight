// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFDAUA07FixtureCmdlet.cpp
// Version: v1.0.0
// Date: 2026-08-20
// Description: DAUTH-P0-12 UA-07/08 Driving Feel USER Acceptance용 test-only Profile fixture 생성 commandlet 구현입니다.
// Changelog:
// - v1.0.0: 기존 Resolver Automation의 monotonic Feel response와 실행 시점 DA_TestSedan direct baseline으로 Handling/Performance test Profile 2개를 생성·저장합니다.
// Migration:
// - Production Recipe/Profile/VehicleData를 수정하지 않습니다.
// - /Game/Test/CarFightDataAuthoring 아래 UA 전용 Profile만 생성하며 Recipe binding은 수행하지 않습니다.
// - Feel response 값은 production balance가 아니라 기존 Resolver technical fixture와 동일한 인과관계 검증값입니다.

#include "CFDAUA07FixtureCmdlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CFVehicleData.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFDAUA07Fixture, Log, All);

namespace CFDAUA07Fixture
{
	// UA-07 Handling test Profile의 고정 package 경로입니다.
	const FString HandlingPackageName = TEXT("/Game/Test/CarFightDataAuthoring/DA_UA07_Handling");

	// UA-07 Performance test Profile의 고정 package 경로입니다.
	const FString PerformancePackageName = TEXT("/Game/Test/CarFightDataAuthoring/DA_UA07_Performance");

	// direct baseline을 복사할 현재 대표 VehicleData의 정확한 object path입니다.
	const TCHAR* BaselineVehicleObjectPath = TEXT("/Game/CarFight/Vehicles/Data/Definitions/DA_TestSedan.DA_TestSedan");

	// Commandlet 단계별 실패를 프로세스 종료 코드로 구분합니다.
	enum class EExitCode : int32
	{
		Success = 0,
		BaselineLoadFailed = 41,
		FixtureCreateFailed = 42,
		FixtureConfigureFailed = 43,
		FixtureSaveFailed = 44
	};

	// 하나의 0.0 / 0.5 / 1.0 Driving Feel response를 명시값으로 설정합니다.
	void SetFeelResponse(FCFFeelResponse& OutResponse, const float LowValue, const float NeutralValue, const float HighValue)
	{
		OutResponse.LowValue = LowValue;
		OutResponse.NeutralValue = NeutralValue;
		OutResponse.HighValue = HighValue;
	}

	// 고정 package에서 기존 test Profile을 로드하거나 없으면 새 Editor-only DataAsset을 생성합니다.
	template <typename TProfile>
	TProfile* LoadOrCreateProfile(const FString& PackageName, FString& OutFailureReason)
	{
		// Long package name에서 계산한 자산 이름입니다.
		const FString AssetName = FPackageName::GetLongPackageAssetName(PackageName);

		// 정확한 Unreal object path입니다.
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);

		// 이미 저장된 동일 test fixture가 있으면 재사용할 Profile입니다.
		TProfile* ExistingProfile = LoadObject<TProfile>(nullptr, *ObjectPath);
		if (ExistingProfile)
		{
			return ExistingProfile;
		}

		if (FPackageName::DoesPackageExist(PackageName))
		{
			OutFailureReason = FString::Printf(TEXT("Existing UA fixture package has an unexpected asset class: %s"), *ObjectPath);
			return nullptr;
		}

		// 새 test-only DataAsset을 소유할 package입니다.
		UPackage* ProfilePackage = CreatePackage(*PackageName);
		if (!ProfilePackage)
		{
			OutFailureReason = FString::Printf(TEXT("UA fixture package creation failed: %s"), *PackageName);
			return nullptr;
		}

		// 새 public standalone Editor-only Profile 자산입니다.
		TProfile* NewProfile = NewObject<TProfile>(
			ProfilePackage,
			*AssetName,
			RF_Public | RF_Standalone | RF_Transactional);
		if (!NewProfile)
		{
			OutFailureReason = FString::Printf(TEXT("UA fixture asset creation failed: %s"), *ObjectPath);
			return nullptr;
		}

		FAssetRegistryModule::AssetCreated(NewProfile);
		NewProfile->MarkPackageDirty();
		return NewProfile;
	}

	// 현재 DA_TestSedan direct baseline을 유지하면서 Handling Feel response만 기술 fixture 값으로 구성합니다.
	bool ConfigureHandlingProfile(UCFHandlingProfile* Profile, const UCFVehicleData* BaselineVehicle, FString& OutFailureReason)
	{
		if (!Profile || !BaselineVehicle)
		{
			OutFailureReason = TEXT("Handling fixture configuration received null Profile or baseline VehicleData");
			return false;
		}

		Profile->Modify();
		Profile->Meta.DisplayName = FText::FromString(TEXT("UA-07 주행감 검증 Handling"));
		Profile->Meta.Description = TEXT("DAUTH-P0-12 UA-07/08 전용 test-only Profile. Feel response는 Resolver Automation 기술 fixture이며 production balance 기준이 아닙니다.");
		Profile->Meta.AuthoringRevision = 0;

		// Resolver Automation에서 이미 사용하는 Steering response fixture입니다.
		SetFeelResponse(Profile->Data.FrontWheelMaxSteerAngleByFeel, 25.0f, 35.0f, 45.0f);
		SetFeelResponse(Profile->Data.SteeringAngleRatioByFeel, 0.5f, 0.7f, 0.9f);

		// Resolver Automation에서 이미 사용하는 Grip response fixture입니다.
		SetFeelResponse(Profile->Data.FrontWheelFrictionByFeel, 1.0f, 1.5f, 2.0f);
		SetFeelResponse(Profile->Data.RearWheelFrictionByFeel, 1.0f, 1.4f, 1.8f);
		SetFeelResponse(Profile->Data.FrontCorneringByFeel, 700.0f, 900.0f, 1100.0f);
		SetFeelResponse(Profile->Data.RearCorneringByFeel, 650.0f, 850.0f, 1050.0f);

		// Resolver Automation에서 이미 사용하는 Suspension response fixture입니다.
		SetFeelResponse(Profile->Data.FrontSpringRateByFeel, 100.0f, 150.0f, 200.0f);
		SetFeelResponse(Profile->Data.RearSpringRateByFeel, 100.0f, 145.0f, 190.0f);
		SetFeelResponse(Profile->Data.FrontSpringPreloadByFeel, 20.0f, 30.0f, 40.0f);
		SetFeelResponse(Profile->Data.RearSpringPreloadByFeel, 20.0f, 28.0f, 36.0f);
		Profile->Data.SuspensionMassScale = FCFMassScaleRule();

		// Feel 축과 무관한 Handling direct 값은 현재 DA_TestSedan baseline을 그대로 복사합니다.
		const FCFVehicleMovementConfig& Movement = BaselineVehicle->VehicleMovementConfig;
		Profile->Data.FrontWheelMaxBrakeTorque = Movement.FrontWheelMaxBrakeTorque;
		Profile->Data.RearWheelMaxBrakeTorque = Movement.RearWheelMaxBrakeTorque;
		Profile->Data.RearWheelMaxHandBrakeTorque = Movement.RearWheelMaxHandBrakeTorque;
		Profile->Data.FrontWheelLoadRatio = Movement.FrontWheelLoadRatio;
		Profile->Data.RearWheelLoadRatio = Movement.RearWheelLoadRatio;
		Profile->Data.FrontWheelSuspensionMaxRaise = Movement.FrontWheelSuspensionMaxRaise;
		Profile->Data.RearWheelSuspensionMaxRaise = Movement.RearWheelSuspensionMaxRaise;
		Profile->Data.FrontWheelSuspensionMaxDrop = Movement.FrontWheelSuspensionMaxDrop;
		Profile->Data.RearWheelSuspensionMaxDrop = Movement.RearWheelSuspensionMaxDrop;
		Profile->Data.FrontWheelSweepShape = Movement.FrontWheelSweepShape;
		Profile->Data.RearWheelSweepShape = Movement.RearWheelSweepShape;
		Profile->Data.SteeringType = Movement.SteeringType;
		Profile->MarkPackageDirty();
		return true;
	}

	// 현재 DA_TestSedan direct baseline을 유지하면서 Performance Feel response만 기술 fixture 값으로 구성합니다.
	bool ConfigurePerformanceProfile(UCFPerformanceProfile* Profile, const UCFVehicleData* BaselineVehicle, FString& OutFailureReason)
	{
		if (!Profile || !BaselineVehicle)
		{
			OutFailureReason = TEXT("Performance fixture configuration received null Profile or baseline VehicleData");
			return false;
		}

		Profile->Modify();
		Profile->Meta.DisplayName = FText::FromString(TEXT("UA-07 주행감 검증 Performance"));
		Profile->Meta.Description = TEXT("DAUTH-P0-12 UA-07/08 전용 test-only Profile. Feel response는 Resolver Automation 기술 fixture이며 production balance 기준이 아닙니다.");
		Profile->Meta.AuthoringRevision = 0;

		// Resolver Automation에서 이미 사용하는 Acceleration response fixture입니다.
		SetFeelResponse(Profile->Data.EngineMaxTorqueByFeel, 500.0f, 800.0f, 1100.0f);
		SetFeelResponse(Profile->Data.EngineMaxRPMByFeel, 4500.0f, 6000.0f, 7500.0f);
		SetFeelResponse(Profile->Data.ThrottleInputScaleByFeel, 0.8f, 1.0f, 1.2f);
		Profile->Data.TorqueMassScale = FCFMassScaleRule();

		// Feel 축과 무관한 Performance direct 값은 현재 DA_TestSedan baseline을 그대로 복사합니다.
		const FCFVehicleMovementConfig& Movement = BaselineVehicle->VehicleMovementConfig;
		Profile->Data.EngineIdleRPM = Movement.EngineIdleRPM;
		Profile->Data.RedlineStartRPM = Movement.RedlineStartRPM;
		Profile->Data.EngineBrakeEffect = Movement.EngineBrakeEffect;
		Profile->Data.EngineRevUpMOI = Movement.EngineRevUpMOI;
		Profile->Data.EngineRevDownRate = Movement.EngineRevDownRate;
		Profile->Data.DragCoefficient = Movement.DragCoefficient;
		Profile->Data.DownforceCoefficient = Movement.DownforceCoefficient;
		Profile->MarkPackageDirty();
		return true;
	}

	// 지정 test Profile을 정확한 .uasset 파일에 저장합니다.
	bool SaveProfile(UObject* ProfileObject, FString& OutFailureReason)
	{
		if (!ProfileObject)
		{
			OutFailureReason = TEXT("Cannot save null UA fixture Profile");
			return false;
		}

		// Profile이 소유된 package입니다.
		UPackage* ProfilePackage = ProfileObject->GetOutermost();
		if (!ProfilePackage)
		{
			OutFailureReason = FString::Printf(TEXT("UA fixture package is missing: %s"), *ProfileObject->GetPathName());
			return false;
		}

		// Long package name을 실제 .uasset 파일 경로로 변환한 값입니다.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			ProfilePackage->GetName(),
			FPackageName::GetAssetPackageExtension());

		// 처음 생성하는 /Game/Test 하위 폴더의 실제 디스크 디렉터리입니다.
		const FString PackageDirectory = FPaths::GetPath(PackageFilename);
		if (!IFileManager::Get().MakeDirectory(*PackageDirectory, true))
		{
			OutFailureReason = FString::Printf(TEXT("UA fixture directory creation failed: %s"), *PackageDirectory);
			return false;
		}

		// Public standalone DataAsset 저장 옵션입니다.
		FSavePackageArgs SavePackageArgs;
		SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SavePackageArgs.SaveFlags = SAVE_NoError;

		ProfileObject->MarkPackageDirty();
		if (!UPackage::SavePackage(ProfilePackage, ProfileObject, *PackageFilename, SavePackageArgs))
		{
			OutFailureReason = FString::Printf(TEXT("UA fixture package save failed: %s"), *PackageFilename);
			return false;
		}
		return true;
	}
}

// Headless Editor commandlet 실행 속성을 준비합니다.
UCFDAUA07FixtureCommandlet::UCFDAUA07FixtureCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// DA_TestSedan baseline을 읽고 UA-07 Handling/Performance Profile을 생성·검증·저장합니다.
int32 UCFDAUA07FixtureCommandlet::Main(const FString& Params)
{
	(void)Params;

	// 실패 시 로그와 프로세스 결과에 남길 가장 구체적인 원인입니다.
	FString FailureReason;

	// Feel과 무관한 direct Profile 값을 복사할 현재 대표 VehicleData입니다.
	UCFVehicleData* BaselineVehicle = LoadObject<UCFVehicleData>(nullptr, CFDAUA07Fixture::BaselineVehicleObjectPath);
	if (!BaselineVehicle)
	{
		UE_LOG(LogCFDAUA07Fixture, Error, TEXT("CF_DA_UA07_FIXTURE_BASELINE_FAIL path=%s"), CFDAUA07Fixture::BaselineVehicleObjectPath);
		return static_cast<int32>(CFDAUA07Fixture::EExitCode::BaselineLoadFailed);
	}

	// UA-07 Steering/Grip/Suspension response를 제공할 test-only Handling Profile입니다.
	UCFHandlingProfile* HandlingProfile = CFDAUA07Fixture::LoadOrCreateProfile<UCFHandlingProfile>(
		CFDAUA07Fixture::HandlingPackageName,
		FailureReason);

	// UA-07 Acceleration response를 제공할 test-only Performance Profile입니다.
	UCFPerformanceProfile* PerformanceProfile = CFDAUA07Fixture::LoadOrCreateProfile<UCFPerformanceProfile>(
		CFDAUA07Fixture::PerformancePackageName,
		FailureReason);

	if (!HandlingProfile || !PerformanceProfile)
	{
		UE_LOG(LogCFDAUA07Fixture, Error, TEXT("CF_DA_UA07_FIXTURE_CREATE_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFDAUA07Fixture::EExitCode::FixtureCreateFailed);
	}

	if (!CFDAUA07Fixture::ConfigureHandlingProfile(HandlingProfile, BaselineVehicle, FailureReason)
		|| !CFDAUA07Fixture::ConfigurePerformanceProfile(PerformanceProfile, BaselineVehicle, FailureReason))
	{
		UE_LOG(LogCFDAUA07Fixture, Error, TEXT("CF_DA_UA07_FIXTURE_CONFIG_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFDAUA07Fixture::EExitCode::FixtureConfigureFailed);
	}

	if (!CFDAUA07Fixture::SaveProfile(HandlingProfile, FailureReason)
		|| !CFDAUA07Fixture::SaveProfile(PerformanceProfile, FailureReason))
	{
		UE_LOG(LogCFDAUA07Fixture, Error, TEXT("CF_DA_UA07_FIXTURE_SAVE_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFDAUA07Fixture::EExitCode::FixtureSaveFailed);
	}

	UE_LOG(
		LogCFDAUA07Fixture,
		Display,
		TEXT("CF_DA_UA07_FIXTURE_PASS handling=%s performance=%s baseline=%s production_binding_changes=0"),
		*HandlingProfile->GetPathName(),
		*PerformanceProfile->GetPathName(),
		*BaselineVehicle->GetPathName());
	return static_cast<int32>(CFDAUA07Fixture::EExitCode::Success);
}
