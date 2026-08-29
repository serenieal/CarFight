// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFDAUA06FixtureCmdlet.cpp
// Version: v1.0.0
// Date: 2026-08-21
// Description: DAUTH-P0-12 UA-06 Explicit Apply / Undo USER Acceptance용 Performance test fixture 생성 commandlet 구현입니다.
// Changelog:
// - v1.0.0: 실행 시점 DA_TestSedan Performance baseline을 그대로 복사하고 Acceleration Feel response를 상수화해 Profile 연결 자체는 Target 결과를 바꾸지 않는 test-only fixture를 생성합니다.
// Migration:
// - Production Recipe/Profile/VehicleData를 수정하지 않습니다.
// - /Game/Test/CarFightDataAuthoring/DA_UA06_Performance 하나만 생성·갱신하며 Recipe binding은 수행하지 않습니다.
// - RedlineStartRPM baseline을 유지한 뒤 USER Acceptance에서 Profile numeric edit로만 시험값을 만들고 원복합니다.

#include "CFDAUA06FixtureCmdlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CFVehicleData.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFDAUA06Fixture, Log, All);

namespace CFDAUA06Fixture
{
	// UA-06 Performance test Profile의 고정 package 경로입니다.
	const FString PerformancePackageName = TEXT("/Game/Test/CarFightDataAuthoring/DA_UA06_Performance");

	// 현재 Performance baseline을 복사할 대표 VehicleData의 정확한 object path입니다.
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

	// 하나의 Feel response를 어떤 AccelerationFeel에서도 동일한 raw value가 나오도록 상수화합니다.
	void SetConstantFeelResponse(FCFFeelResponse& OutResponse, const float BaselineValue)
	{
		OutResponse.LowValue = BaselineValue;
		OutResponse.NeutralValue = BaselineValue;
		OutResponse.HighValue = BaselineValue;
	}

	// 고정 package에서 기존 test Profile을 로드하거나 없으면 새 Editor-only DataAsset을 생성합니다.
	UCFPerformanceProfile* LoadOrCreatePerformanceProfile(FString& OutFailureReason)
	{
		// 고정 package name에서 계산한 UA-06 fixture asset 이름입니다.
		const FString AssetName = FPackageName::GetLongPackageAssetName(PerformancePackageName);

		// 고정 package와 asset 이름을 결합한 정확한 object path입니다.
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PerformancePackageName, *AssetName);

		// 이미 저장된 UA-06 fixture가 있으면 deterministic하게 갱신할 existing Profile입니다.
		UCFPerformanceProfile* ExistingProfile = LoadObject<UCFPerformanceProfile>(nullptr, *ObjectPath);
		if (ExistingProfile)
		{
			return ExistingProfile;
		}

		if (FPackageName::DoesPackageExist(PerformancePackageName))
		{
			OutFailureReason = FString::Printf(TEXT("Existing UA-06 fixture package has an unexpected asset class: %s"), *ObjectPath);
			return nullptr;
		}

		// 새 test-only Performance Profile을 소유할 package입니다.
		UPackage* ProfilePackage = CreatePackage(*PerformancePackageName);
		if (!ProfilePackage)
		{
			OutFailureReason = FString::Printf(TEXT("UA-06 fixture package creation failed: %s"), *PerformancePackageName);
			return nullptr;
		}

		// 새 public standalone Editor-only Performance Profile 자산입니다.
		UCFPerformanceProfile* NewProfile = NewObject<UCFPerformanceProfile>(
			ProfilePackage,
			*AssetName,
			RF_Public | RF_Standalone | RF_Transactional);
		if (!NewProfile)
		{
			OutFailureReason = FString::Printf(TEXT("UA-06 fixture asset creation failed: %s"), *ObjectPath);
			return nullptr;
		}

		FAssetRegistryModule::AssetCreated(NewProfile);
		NewProfile->MarkPackageDirty();
		return NewProfile;
	}

	// DA_TestSedan의 현재 Performance 결과와 동일한 test-only Profile payload를 구성합니다.
	bool ConfigurePerformanceProfile(UCFPerformanceProfile* Profile, const UCFVehicleData* BaselineVehicle, FString& OutFailureReason)
	{
		if (!Profile || !BaselineVehicle)
		{
			OutFailureReason = TEXT("UA-06 Performance fixture configuration received null Profile or baseline VehicleData");
			return false;
		}

		Profile->Modify();
		Profile->Meta.DisplayName = FText::FromString(TEXT("UA-06 적용/되돌리기 검증 Performance"));
		Profile->Meta.Description = TEXT("DAUTH-P0-12 UA-06 전용 test-only Profile. 연결 자체로 Target 결과를 바꾸지 않고 RedlineStartRPM Source edit 한 건만 검증하기 위한 fixture입니다.");
		Profile->Meta.AuthoringRevision = 0;

		// 실행 시점 DA_TestSedan Performance baseline입니다.
		const FCFVehicleMovementConfig& Movement = BaselineVehicle->VehicleMovementConfig;

		// 어떤 AccelerationFeel에서도 현재 Target과 같은 EngineMaxTorque가 나오도록 상수 response로 만듭니다.
		SetConstantFeelResponse(Profile->Data.EngineMaxTorqueByFeel, Movement.EngineMaxTorque);

		// 어떤 AccelerationFeel에서도 현재 Target과 같은 EngineMaxRPM이 나오도록 상수 response로 만듭니다.
		SetConstantFeelResponse(Profile->Data.EngineMaxRPMByFeel, Movement.EngineMaxRPM);

		// 어떤 AccelerationFeel에서도 현재 Target과 같은 ThrottleInputScale이 나오도록 상수 response로 만듭니다.
		SetConstantFeelResponse(Profile->Data.ThrottleInputScaleByFeel, Movement.ThrottleInputScale);

		// UA-06 fixture는 질량에 따른 Torque 재스케일을 사용하지 않습니다.
		Profile->Data.TorqueMassScale = FCFMassScaleRule();

		// Feel 축과 무관한 Performance direct 값은 현재 DA_TestSedan baseline을 그대로 복사합니다.
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

	// UA-06 test-only Performance Profile을 정확한 .uasset 파일에 저장합니다.
	bool SavePerformanceProfile(UCFPerformanceProfile* Profile, FString& OutFailureReason)
	{
		if (!Profile)
		{
			OutFailureReason = TEXT("Cannot save null UA-06 Performance fixture");
			return false;
		}

		// Profile이 소유된 package입니다.
		UPackage* ProfilePackage = Profile->GetOutermost();
		if (!ProfilePackage)
		{
			OutFailureReason = FString::Printf(TEXT("UA-06 fixture package is missing: %s"), *Profile->GetPathName());
			return false;
		}

		// Long package name을 실제 .uasset 파일 경로로 변환한 값입니다.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			ProfilePackage->GetName(),
			FPackageName::GetAssetPackageExtension());

		// /Game/Test 하위 package의 실제 디스크 디렉터리입니다.
		const FString PackageDirectory = FPaths::GetPath(PackageFilename);
		if (!IFileManager::Get().MakeDirectory(*PackageDirectory, true))
		{
			OutFailureReason = FString::Printf(TEXT("UA-06 fixture directory creation failed: %s"), *PackageDirectory);
			return false;
		}

		// Public standalone DataAsset 저장 옵션입니다.
		FSavePackageArgs SavePackageArgs;
		SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SavePackageArgs.SaveFlags = SAVE_NoError;

		Profile->MarkPackageDirty();
		if (!UPackage::SavePackage(ProfilePackage, Profile, *PackageFilename, SavePackageArgs))
		{
			OutFailureReason = FString::Printf(TEXT("UA-06 fixture package save failed: %s"), *PackageFilename);
			return false;
		}
		return true;
	}
}

// Headless Editor commandlet 실행 속성을 준비합니다.
UCFDAUA06FixtureCommandlet::UCFDAUA06FixtureCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// DA_TestSedan 현재 Performance baseline과 결과가 같은 UA-06 Performance Profile을 생성·검증·저장합니다.
int32 UCFDAUA06FixtureCommandlet::Main(const FString& Params)
{
	(void)Params;

	// 실패 시 로그와 프로세스 결과에 남길 가장 구체적인 원인입니다.
	FString FailureReason;

	// Profile payload를 구성할 현재 대표 VehicleData입니다.
	UCFVehicleData* BaselineVehicle = LoadObject<UCFVehicleData>(nullptr, CFDAUA06Fixture::BaselineVehicleObjectPath);
	if (!BaselineVehicle)
	{
		UE_LOG(LogCFDAUA06Fixture, Error, TEXT("CF_DA_UA06_FIXTURE_BASELINE_FAIL path=%s"), CFDAUA06Fixture::BaselineVehicleObjectPath);
		return static_cast<int32>(CFDAUA06Fixture::EExitCode::BaselineLoadFailed);
	}

	// UA-06 Source edit / Apply / Undo에 사용할 test-only Performance Profile입니다.
	UCFPerformanceProfile* PerformanceProfile = CFDAUA06Fixture::LoadOrCreatePerformanceProfile(FailureReason);
	if (!PerformanceProfile)
	{
		UE_LOG(LogCFDAUA06Fixture, Error, TEXT("CF_DA_UA06_FIXTURE_CREATE_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFDAUA06Fixture::EExitCode::FixtureCreateFailed);
	}

	if (!CFDAUA06Fixture::ConfigurePerformanceProfile(PerformanceProfile, BaselineVehicle, FailureReason))
	{
		UE_LOG(LogCFDAUA06Fixture, Error, TEXT("CF_DA_UA06_FIXTURE_CONFIG_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFDAUA06Fixture::EExitCode::FixtureConfigureFailed);
	}

	if (!CFDAUA06Fixture::SavePerformanceProfile(PerformanceProfile, FailureReason))
	{
		UE_LOG(LogCFDAUA06Fixture, Error, TEXT("CF_DA_UA06_FIXTURE_SAVE_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFDAUA06Fixture::EExitCode::FixtureSaveFailed);
	}

	UE_LOG(
		LogCFDAUA06Fixture,
		Display,
		TEXT("CF_DA_UA06_FIXTURE_PASS performance=%s baseline=%s redline=%.3f production_binding_changes=0"),
		*PerformanceProfile->GetPathName(),
		*BaselineVehicle->GetPathName(),
		BaselineVehicle->VehicleMovementConfig.RedlineStartRPM);
	return static_cast<int32>(CFDAUA06Fixture::EExitCode::Success);
}
