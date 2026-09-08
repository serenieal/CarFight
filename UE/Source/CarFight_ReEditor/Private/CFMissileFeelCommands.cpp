// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.5.0
// Date: 2026-09-08
// Description: CF-FQ-030 MissileDirectTest 전용 USER 체감 비교 콘솔 명령
// Scope: 저장 DirectTest Projectile/Weapon/Equipment를 transient 복제하고 Low/Normal/High Guidance Preset DataAsset의 MissileGuideConfig만 적용해 재빌드 없는 반복 체감 튜닝을 지원합니다.
// Changelog:
// - v1.5.0: Low/Normal/High 신규 Preset seed를 Product DataAsset 수명주기에서 Editor authoring으로 이동. 기존 Preset은 load/Ensure에서 재seed·재save하지 않으며, 별도 임시 persisted Preset으로 사용자 튜닝값 저장→Ensure 재실행→package unload/디스크 재로드 보존을 직접 검증하는 idempotence round-trip을 추가.
// - v1.4.0: WriteProbe의 persisted save 비소유 경계를 우회하지 않고, 명시 실행형 CarFight.Authoring.MissileFeel.EnsurePresets Automation이 UE SavePackage 정식 경로로 Low/Normal/High Preset 3개를 생성·저장하도록 추가.
// - v1.3.0: MG-P0-12 Feel Preset DA 전환. Low/Normal/High 수치 하드코딩을 제거하고 CFMissileGuidePresetData 3개를 로드해 transient ProjectileData에 적용하도록 변경.
// - v1.2.1: USER Low Feel Reject 교정. PurePursuit는 유지하되 Independent 0.25s/500cm 초기 탄도 보존, 관측·응답·기동·Seeker 값을 완화해 잘 조준한 선행 탄도를 즉시 망가뜨리는 체감을 줄임.
// - v1.2.0: MG-P0-12E USER Feel Setup Rewire. Low=PurePursuit+FlightWindow, Normal=LeadPursuit+짧은 Independent, High=PN+Independent 0/0+180deg Seeker로 재배선하고 12E Focused Automation 추가.
// - v1.1.1: USER 시험 전용 소유권에 맞춰 CarFight_Re 런타임 모듈에서 CarFight_ReEditor 모듈로 파일을 이동. 동작과 명령 계약 변경 없음.
// - v1.1.0: MG-P0-12A Focused Automation을 추가해 네 콘솔 명령 등록과 Low/Normal/High GuideConfig exact value를 PIE 없이 검증.
// - v1.0.1: UE 5.8 실제 FAutoConsoleCommand API에 맞춰 인수형 단일 명령을 Baseline/Low/Normal/High 무인수 명령 4개로 교정.
// - v1.0.0: CarFight.MissileFeel.Set <Baseline|Low|Normal|High> 명령과 transient ProjectileData/WeaponData/EquipmentPresetData override 경로 최초 추가.
// Migration:
// - 이 파일은 CarFight_ReEditor 모듈에만 존재하고 WITH_EDITOR에서 명령을 등록하므로 패키징 Product Runtime에는 품질 enum이나 분기를 추가하지 않습니다.
// - /Game/CarFight/Tests/Missile의 저장 DirectTest Projectile/Weapon/Equipment/Map은 기준 자산으로 유지하고 Feel 명령이 저장값을 수정하지 않습니다.
// - Low/Normal/High는 USER 체감 비교용 Preset 이름이며 Product 품질 enum이나 런타임 분기가 아닙니다.
// - v1.3.0부터 유도 수치 수정은 /Game/Test/CarFight/Missile/FeelPresets의 DataAsset에서 수행하며 C++ 재빌드를 요구하지 않습니다.
// - Preset은 Guidance만 소유하고 Projectile 속도·피해·Collision·FX는 기존 DirectTest 복제본을 그대로 사용합니다.
// - v1.5.0부터 Low/Normal/High seed는 신규 Asset을 실제 생성하는 Editor authoring 함수에서만 적용하며 UCFMissileGuidePresetData 자체는 passive container입니다.
// - Baseline 명령은 저장 EQ_Missile_DirectTest를 다시 적용해 PIE Runtime을 원래 DirectTest 구성으로 복원합니다.

#include "CFEquipmentPresetData.h"
#include "CFMissileGuidePresetData.h"
#include "CFProjectileData.h"
#include "CFVehicleData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "PackageTools.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/UObjectGlobals.h"

#if WITH_EDITOR

namespace CFMissileFeelCommands
{
	/** MG-P0-12A/12E에서 사용하는 USER 체감 비교 Variant입니다. Product Runtime 타입이 아닙니다. */
	enum class EMissileFeelTestVariant : uint8
	{
		Baseline,
		Low,
		Normal,
		High
	};

	// 저장 DirectTest 차량 기준 자산 경로입니다.
	const TCHAR* DirectTestVehicleDataPath = TEXT("/Game/CarFight/Tests/Missile/DA_Missile_TestSUV.DA_Missile_TestSUV");

	// 저장 DirectTest 장비 프리셋 기준 자산 경로입니다.
	const TCHAR* DirectTestEquipmentPresetPath = TEXT("/Game/CarFight/Tests/Missile/EQ_Missile_DirectTest.EQ_Missile_DirectTest");

	// USER Low 체감 튜닝 값을 저장하는 Guidance Preset package 경로입니다.
	const TCHAR* LowGuidePresetPackagePath = TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Low");

	// USER Low 체감 튜닝 값을 저장하는 Guidance Preset DataAsset 경로입니다.
	const TCHAR* LowGuidePresetPath = TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Low.DA_MissileFeel_Low");

	// USER Normal 체감 튜닝 값을 저장하는 Guidance Preset package 경로입니다.
	const TCHAR* NormalGuidePresetPackagePath = TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Normal");

	// USER Normal 체감 튜닝 값을 저장하는 Guidance Preset DataAsset 경로입니다.
	const TCHAR* NormalGuidePresetPath = TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Normal.DA_MissileFeel_Normal");

	// USER High 체감 튜닝 값을 저장하는 Guidance Preset package 경로입니다.
	const TCHAR* HighGuidePresetPackagePath = TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_High");

	// USER High 체감 튜닝 값을 저장하는 Guidance Preset DataAsset 경로입니다.
	const TCHAR* HighGuidePresetPath = TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_High.DA_MissileFeel_High");

	// 이 명령이 허용되는 PIE 테스트 맵 이름 토큰입니다.
	const TCHAR* DirectTestMapNameToken = TEXT("MissileDirectTest");

	// PIE 월드에 체감 테스트 결과를 짧게 표시합니다.
	void ShowMissileFeelMessage(const FString& Message)
	{
		UE_LOG(LogTemp, Display, TEXT("[MissileFeel] %s"), *Message);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 6.0f, FColor::Cyan, Message);
		}
	}

	// 현재 엔진 WorldContext 중 실제 PIE 월드 하나를 반환합니다.
	UWorld* FindMissileFeelPIEWorld()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World())
			{
				return WorldContext.World();
			}
		}

		return nullptr;
	}

	// 체감 테스트 Variant의 짧은 영문 식별 이름을 반환합니다.
	const TCHAR* GetMissileFeelVariantName(const EMissileFeelTestVariant Variant)
	{
		switch (Variant)
		{
		case EMissileFeelTestVariant::Low:
			return TEXT("Low");
		case EMissileFeelTestVariant::Normal:
			return TEXT("Normal");
		case EMissileFeelTestVariant::High:
			return TEXT("High");
		case EMissileFeelTestVariant::Baseline:
		default:
			return TEXT("Baseline");
		}
	}

	// 체감 테스트 Variant의 USER-facing 한글 이름을 반환합니다.
	const TCHAR* GetMissileFeelVariantDisplayName(const EMissileFeelTestVariant Variant)
	{
		switch (Variant)
		{
		case EMissileFeelTestVariant::Low:
			return TEXT("저성능");
		case EMissileFeelTestVariant::Normal:
			return TEXT("기준형");
		case EMissileFeelTestVariant::High:
			return TEXT("고성능");
		case EMissileFeelTestVariant::Baseline:
		default:
			return TEXT("기존 DirectTest");
		}
	}

	// 체감 테스트 Guidance Law의 USER-facing 한글 이름을 반환합니다.
	const TCHAR* GetMissileFeelGuidanceLawDisplayName(const ECFMissileGuidanceLaw GuidanceLaw)
	{
		switch (GuidanceLaw)
		{
		case ECFMissileGuidanceLaw::PurePursuit:
			return TEXT("단순 추적");
		case ECFMissileGuidanceLaw::LeadPursuit:
			return TEXT("제한 선행 추적");
		case ECFMissileGuidanceLaw::ProportionalNavigation:
		default:
			return TEXT("비례항법");
		}
	}

	// 체감 테스트 Guidance Activation의 USER-facing 한글 이름을 반환합니다.
	const TCHAR* GetMissileFeelActivationDisplayName(const ECFMissileGuidanceActivationMode ActivationMode)
	{
		return ActivationMode == ECFMissileGuidanceActivationMode::Independent
			? TEXT("독립 활성화")
			: TEXT("비행 유도구간 따름");
	}

	// USER 체감 Variant가 읽어야 할 Guidance Preset DataAsset 경로를 반환합니다.
	const TCHAR* GetMissileFeelPresetPath(const EMissileFeelTestVariant Variant)
	{
		switch (Variant)
		{
		case EMissileFeelTestVariant::Low:
			return LowGuidePresetPath;
		case EMissileFeelTestVariant::Normal:
			return NormalGuidePresetPath;
		case EMissileFeelTestVariant::High:
			return HighGuidePresetPath;
		case EMissileFeelTestVariant::Baseline:
		default:
			return nullptr;
		}
	}

	// USER 체감 Variant에 대응하는 저장 Guidance Preset DataAsset을 로드합니다.
	UCFMissileGuidePresetData* LoadMissileFeelPreset(const EMissileFeelTestVariant Variant)
	{
		// 선택 Variant에 대응하는 저장 Preset object path입니다.
		const TCHAR* PresetPath = GetMissileFeelPresetPath(Variant);
		return PresetPath ? LoadObject<UCFMissileGuidePresetData>(nullptr, PresetPath) : nullptr;
	}

	// 저장 Guidance Preset에서 실제 Runtime에 적용할 유효 MissileGuideConfig를 반환합니다.
	bool ResolveMissileFeelGuideConfig(
		const EMissileFeelTestVariant Variant,
		FCFMissileGuideConfig& OutGuideConfig)
	{
		// Low/Normal/High 체감 수치를 소유하는 저장 Guidance Preset DataAsset입니다.
		UCFMissileGuidePresetData* GuidePreset = LoadMissileFeelPreset(Variant);
		if (!GuidePreset)
		{
			return false;
		}

		OutGuideConfig = GuidePreset->MissileGuideConfig.GetEffectiveConfig();
		return true;
	}

	// 신규 Guidance Preset이 공통으로 사용할 Guidance 활성/관측 기본 계약을 채웁니다.
	void InitializeCommonMissileFeelGuidance(UCFMissileGuidePresetData* PresetAsset)
	{
		if (!PresetAsset)
		{
			return;
		}

		PresetAsset->MissileGuideConfig.bUseGuidance = true;
		PresetAsset->MissileGuideConfig.GuideMode = ECFMissileGuideMode::TargetActor;
		PresetAsset->MissileGuideConfig.LostTargetPolicy = ECFMissileLostTargetPolicy::ContinueStraight;
		PresetAsset->MissileGuideConfig.NavigationConstant = 3.0f;
		PresetAsset->MissileGuideConfig.MinimumGuidanceSpeedCmPerSec = 500.0f;
		PresetAsset->MissileGuideConfig.SeekerModel = ECFMissileSeekerModel::Stateful;
		PresetAsset->MissileGuideConfig.TargetObservationMode = ECFMissileTargetObservationMode::SampledPositionEstimate;
	}

	// 새로 생성된 Guidance Preset 하나에만 선택 Variant의 최초 저작 seed를 적용합니다.
	void InitializeNewMissileFeelPreset(
		UCFMissileGuidePresetData* PresetAsset,
		const EMissileFeelTestVariant Variant)
	{
		if (!PresetAsset)
		{
			return;
		}

		InitializeCommonMissileFeelGuidance(PresetAsset);

		switch (Variant)
		{
		case EMissileFeelTestVariant::Low:
			PresetAsset->PresetId = FName(TEXT("MissileFeel_Low"));
			PresetAsset->PresetDisplayName = FText::FromString(TEXT("저성능 단순 추적"));
			PresetAsset->PresetDescription = FText::FromString(TEXT("좋은 초기 조준 탄도를 짧게 보존한 뒤 현재 관측 위치만 단순 추적하는 저가형 Pure Pursuit 체감 프리셋입니다."));
			PresetAsset->MissileGuideConfig.GuidanceLaw = ECFMissileGuidanceLaw::PurePursuit;
			PresetAsset->MissileGuideConfig.GuidanceActivationMode = ECFMissileGuidanceActivationMode::Independent;
			PresetAsset->MissileGuideConfig.GuidanceActivationDelaySeconds = 0.25f;
			PresetAsset->MissileGuideConfig.GuidanceActivationDistanceCm = 500.0f;
			PresetAsset->MissileGuideConfig.LeadTimeSeconds = 0.0f;
			PresetAsset->MissileGuideConfig.MaxLeadDistanceCm = 0.0f;
			PresetAsset->MissileGuideConfig.TargetObservationIntervalSeconds = 0.08f;
			PresetAsset->MissileGuideConfig.TargetVelocityEstimateResponseTimeSeconds = 0.25f;
			PresetAsset->MissileGuideConfig.GuidanceResponseTimeSeconds = 0.18f;
			PresetAsset->MissileGuideConfig.MaximumTurnRateDegPerSec = 35.0f;
			PresetAsset->MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq = 2000.0f;
			PresetAsset->MissileGuideConfig.AcquisitionConeHalfAngleDeg = 45.0f;
			PresetAsset->MissileGuideConfig.TrackingConeHalfAngleDeg = 60.0f;
			PresetAsset->MissileGuideConfig.TargetLostGraceTimeSeconds = 0.20f;
			PresetAsset->MissileGuideConfig.ReacquisitionMode = ECFMissileReacquisitionMode::None;
			PresetAsset->MissileGuideConfig.ReacquisitionConeHalfAngleDeg = 60.0f;
			PresetAsset->MissileGuideConfig.ReacquisitionTimeSeconds = 0.0f;
			break;

		case EMissileFeelTestVariant::Normal:
			PresetAsset->PresetId = FName(TEXT("MissileFeel_Normal"));
			PresetAsset->PresetDisplayName = FText::FromString(TEXT("기준형 제한 선행 추적"));
			PresetAsset->PresetDescription = FText::FromString(TEXT("짧은 독립 유도 시작 뒤 제한된 선행 위치를 계산해 추적하는 중간급 Lead Pursuit 체감 프리셋입니다."));
			PresetAsset->MissileGuideConfig.GuidanceLaw = ECFMissileGuidanceLaw::LeadPursuit;
			PresetAsset->MissileGuideConfig.GuidanceActivationMode = ECFMissileGuidanceActivationMode::Independent;
			PresetAsset->MissileGuideConfig.GuidanceActivationDelaySeconds = 0.06f;
			PresetAsset->MissileGuideConfig.GuidanceActivationDistanceCm = 100.0f;
			PresetAsset->MissileGuideConfig.LeadTimeSeconds = 0.18f;
			PresetAsset->MissileGuideConfig.MaxLeadDistanceCm = 900.0f;
			PresetAsset->MissileGuideConfig.TargetObservationIntervalSeconds = 0.08f;
			PresetAsset->MissileGuideConfig.TargetVelocityEstimateResponseTimeSeconds = 0.20f;
			PresetAsset->MissileGuideConfig.GuidanceResponseTimeSeconds = 0.20f;
			PresetAsset->MissileGuideConfig.MaximumTurnRateDegPerSec = 50.0f;
			PresetAsset->MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq = 6000.0f;
			PresetAsset->MissileGuideConfig.AcquisitionConeHalfAngleDeg = 75.0f;
			PresetAsset->MissileGuideConfig.TrackingConeHalfAngleDeg = 90.0f;
			PresetAsset->MissileGuideConfig.TargetLostGraceTimeSeconds = 0.40f;
			PresetAsset->MissileGuideConfig.ReacquisitionMode = ECFMissileReacquisitionMode::ForwardCone;
			PresetAsset->MissileGuideConfig.ReacquisitionConeHalfAngleDeg = 120.0f;
			PresetAsset->MissileGuideConfig.ReacquisitionTimeSeconds = 0.30f;
			break;

		case EMissileFeelTestVariant::High:
			PresetAsset->PresetId = FName(TEXT("MissileFeel_High"));
			PresetAsset->PresetDisplayName = FText::FromString(TEXT("고성능 비례항법"));
			PresetAsset->PresetDescription = FText::FromString(TEXT("발사 직후 전방/후방 각도까지 폭넓게 포착하고 PN Course Capture를 사용하는 고성능 비례항법 체감 프리셋입니다."));
			PresetAsset->MissileGuideConfig.GuidanceLaw = ECFMissileGuidanceLaw::ProportionalNavigation;
			PresetAsset->MissileGuideConfig.GuidanceActivationMode = ECFMissileGuidanceActivationMode::Independent;
			PresetAsset->MissileGuideConfig.GuidanceActivationDelaySeconds = 0.0f;
			PresetAsset->MissileGuideConfig.GuidanceActivationDistanceCm = 0.0f;
			PresetAsset->MissileGuideConfig.TargetObservationIntervalSeconds = 0.03f;
			PresetAsset->MissileGuideConfig.TargetVelocityEstimateResponseTimeSeconds = 0.10f;
			PresetAsset->MissileGuideConfig.GuidanceResponseTimeSeconds = 0.05f;
			PresetAsset->MissileGuideConfig.MaximumTurnRateDegPerSec = 140.0f;
			PresetAsset->MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq = 15000.0f;
			PresetAsset->MissileGuideConfig.AcquisitionConeHalfAngleDeg = 180.0f;
			PresetAsset->MissileGuideConfig.TrackingConeHalfAngleDeg = 180.0f;
			PresetAsset->MissileGuideConfig.TargetLostGraceTimeSeconds = 0.80f;
			PresetAsset->MissileGuideConfig.ReacquisitionMode = ECFMissileReacquisitionMode::ForwardCone;
			PresetAsset->MissileGuideConfig.ReacquisitionConeHalfAngleDeg = 180.0f;
			PresetAsset->MissileGuideConfig.ReacquisitionTimeSeconds = 0.60f;
			break;

		case EMissileFeelTestVariant::Baseline:
		default:
			break;
		}
	}

	// 한 Guidance Preset DataAsset의 현재 값을 exact .uasset으로 저장합니다.
	bool SaveMissileFeelPresetAsset(UCFMissileGuidePresetData* PresetAsset)
	{
		if (!PresetAsset)
		{
			return false;
		}

		// 현재 Preset을 소유하는 Unreal package입니다.
		UPackage* PresetPackage = PresetAsset->GetOutermost();
		if (!PresetPackage)
		{
			return false;
		}

		// Unreal long package name을 실제 Content .uasset 저장 위치로 변환한 파일 경로입니다.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			PresetPackage->GetName(),
			FPackageName::GetAssetPackageExtension());

		// 신규 하위 폴더에도 SavePackage가 안전하게 기록될 수 있도록 확보할 디렉터리입니다.
		const FString PackageDirectory = FPaths::GetPath(PackageFilename);
		IFileManager::Get().MakeDirectory(*PackageDirectory, true);

		// Preset package를 public standalone asset으로 저장하기 위한 UE SavePackage 인수입니다.
		FSavePackageArgs SavePackageArgs;
		SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SavePackageArgs.SaveFlags = SAVE_NoError;
		return UPackage::SavePackage(PresetPackage, PresetAsset, *PackageFilename, SavePackageArgs);
	}

	// 한 Guidance Preset DataAsset이 이미 있으면 그대로 사용하고 없으면 Editor authoring seed를 적용해 새 package에 저장합니다.
	bool EnsureMissileFeelPresetAsset(
		const EMissileFeelTestVariant Variant,
		const TCHAR* PackagePath,
		const TCHAR* ObjectPath,
		const TCHAR* AssetName,
		UCFMissileGuidePresetData*& OutPresetAsset)
	{
		// 기존에 저장된 같은 Guidance Preset DataAsset이 있으면 사용자 튜닝값을 보존하기 위해 그대로 재사용합니다.
		UCFMissileGuidePresetData* ExistingPresetAsset = LoadObject<UCFMissileGuidePresetData>(nullptr, ObjectPath);
		if (ExistingPresetAsset)
		{
			OutPresetAsset = ExistingPresetAsset;
			return true;
		}

		// 새 Guidance Preset을 소유할 Unreal package입니다.
		UPackage* PresetPackage = CreatePackage(PackagePath);
		if (!PresetPackage)
		{
			return false;
		}

		// Product DataAsset 수명주기와 무관하게 Editor authoring이 명시적으로 seed할 신규 Guidance Preset입니다.
		UCFMissileGuidePresetData* NewPresetAsset = NewObject<UCFMissileGuidePresetData>(
			PresetPackage,
			FName(AssetName),
			RF_Public | RF_Standalone | RF_Transactional);
		if (!NewPresetAsset)
		{
			return false;
		}

		InitializeNewMissileFeelPreset(NewPresetAsset, Variant);
		FAssetRegistryModule::AssetCreated(NewPresetAsset);
		PresetPackage->MarkPackageDirty();
		if (!SaveMissileFeelPresetAsset(NewPresetAsset))
		{
			return false;
		}

		OutPresetAsset = NewPresetAsset;
		return true;
	}

	// Low/Normal/High Guidance Preset 세 개를 idempotent 방식으로 생성·저장하고 현재 구조 계약까지 검증합니다.
	bool EnsureAllMissileFeelPresetAssets()
	{
		// Low 저장 Preset 생성/재사용 결과를 받는 자산 포인터입니다.
		UCFMissileGuidePresetData* LowPresetAsset = nullptr;

		// Normal 저장 Preset 생성/재사용 결과를 받는 자산 포인터입니다.
		UCFMissileGuidePresetData* NormalPresetAsset = nullptr;

		// High 저장 Preset 생성/재사용 결과를 받는 자산 포인터입니다.
		UCFMissileGuidePresetData* HighPresetAsset = nullptr;

		// Low Preset exact package/object 생성 또는 기존 저장 자산 재사용 결과입니다.
		const bool bLowReady = EnsureMissileFeelPresetAsset(
			EMissileFeelTestVariant::Low,
			LowGuidePresetPackagePath,
			LowGuidePresetPath,
			TEXT("DA_MissileFeel_Low"),
			LowPresetAsset);

		// Normal Preset exact package/object 생성 또는 기존 저장 자산 재사용 결과입니다.
		const bool bNormalReady = EnsureMissileFeelPresetAsset(
			EMissileFeelTestVariant::Normal,
			NormalGuidePresetPackagePath,
			NormalGuidePresetPath,
			TEXT("DA_MissileFeel_Normal"),
			NormalPresetAsset);

		// High Preset exact package/object 생성 또는 기존 저장 자산 재사용 결과입니다.
		const bool bHighReady = EnsureMissileFeelPresetAsset(
			EMissileFeelTestVariant::High,
			HighGuidePresetPackagePath,
			HighGuidePresetPath,
			TEXT("DA_MissileFeel_High"),
			HighPresetAsset);

		return bLowReady
			&& bNormalReady
			&& bHighReady
			&& LowPresetAsset
			&& NormalPresetAsset
			&& HighPresetAsset
			&& LowPresetAsset->PresetId == FName(TEXT("MissileFeel_Low"))
			&& NormalPresetAsset->PresetId == FName(TEXT("MissileFeel_Normal"))
			&& HighPresetAsset->PresetId == FName(TEXT("MissileFeel_High"))
			&& LowPresetAsset->MissileGuideConfig.GuidanceLaw == ECFMissileGuidanceLaw::PurePursuit
			&& NormalPresetAsset->MissileGuideConfig.GuidanceLaw == ECFMissileGuidanceLaw::LeadPursuit
			&& HighPresetAsset->MissileGuideConfig.GuidanceLaw == ECFMissileGuidanceLaw::ProportionalNavigation;
	}

	// 현재 MissileDirectTest PIE 차량에 선택 Variant를 transient 장비 체인으로 적용합니다.
	bool ApplyMissileFeelVariant(const EMissileFeelTestVariant Variant)
	{
		// 현재 실행 중인 실제 PIE 월드입니다.
		UWorld* PIEWorld = FindMissileFeelPIEWorld();
		if (!PIEWorld)
		{
			ShowMissileFeelMessage(TEXT("[미사일 체감 테스트] PIE 실행 중에만 사용할 수 있습니다."));
			return false;
		}

		// 다른 게임 맵에서 시험용 Runtime override가 사용되지 않도록 검사할 현재 맵 이름입니다.
		const FString CurrentMapName = PIEWorld->GetMapName();
		if (!CurrentMapName.Contains(DirectTestMapNameToken, ESearchCase::IgnoreCase))
		{
			ShowMissileFeelMessage(FString::Printf(
				TEXT("[미사일 체감 테스트] MissileDirectTest 전용 명령입니다. 현재 맵: %s"),
				*CurrentMapName));
			return false;
		}

		// 현재 PIE의 첫 로컬 플레이어 Controller입니다.
		APlayerController* PlayerController = PIEWorld->GetFirstPlayerController();

		// DirectTest에서 실제 발사 Runtime을 소유하는 플레이어 차량 Pawn입니다.
		ACFVehiclePawn* VehiclePawn = PlayerController ? Cast<ACFVehiclePawn>(PlayerController->GetPawn()) : nullptr;
		if (!VehiclePawn)
		{
			ShowMissileFeelMessage(TEXT("[미사일 체감 테스트] 플레이어 차량 Pawn을 찾지 못했습니다."));
			return false;
		}

		// 실제 발사 시 활성 Weapon/ProjectileData를 제공하는 차량 Weapon Component입니다.
		UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();
		if (!VehicleWeaponComp)
		{
			ShowMissileFeelMessage(TEXT("[미사일 체감 테스트] VehicleWeaponComp를 찾지 못했습니다."));
			return false;
		}

		// MissileDirectTest가 사용하는 저장 차량 DataAsset입니다. Runtime override 초기화 입력으로만 읽습니다.
		UCFVehicleData* BaselineVehicleData = LoadObject<UCFVehicleData>(nullptr, DirectTestVehicleDataPath);

		// MissileDirectTest가 사용하는 저장 EquipmentPresetData입니다. Baseline 복원과 transient 복제 원본으로만 읽습니다.
		UCFEquipmentPresetData* BaselineEquipmentPreset = LoadObject<UCFEquipmentPresetData>(nullptr, DirectTestEquipmentPresetPath);
		if (!BaselineVehicleData || !BaselineEquipmentPreset || !BaselineEquipmentPreset->DefaultWeaponData)
		{
			ShowMissileFeelMessage(TEXT("[미사일 체감 테스트] DirectTest 기준 Vehicle/Equipment/WeaponData를 읽지 못했습니다."));
			return false;
		}

		// 저장 EquipmentPresetData가 직접 참조하는 DirectTest WeaponData입니다.
		UCFWeaponData* BaselineWeaponData = BaselineEquipmentPreset->DefaultWeaponData;

		// 저장 WeaponData가 직접 참조하는 DirectTest ProjectileData입니다.
		UCFProjectileData* BaselineProjectileData = BaselineWeaponData->DefaultProjectileData;
		if (!BaselineProjectileData)
		{
			ShowMissileFeelMessage(TEXT("[미사일 체감 테스트] DirectTest ProjectileData를 읽지 못했습니다."));
			return false;
		}

		// 기존 DirectTest와 동일한 실제 하드포인트를 유지할 활성 MountProfile ID입니다.
		const FName ActiveMountProfileId = VehicleWeaponComp->GetActiveMountProfileId();
		if (ActiveMountProfileId.IsNone())
		{
			ShowMissileFeelMessage(TEXT("[미사일 체감 테스트] 활성 MountProfile ID가 없어 Runtime override를 적용할 수 없습니다."));
			return false;
		}

		if (Variant == EMissileFeelTestVariant::Baseline)
		{
			// 저장 DirectTest Equipment를 그대로 다시 적용해 transient 체감 설정을 제거한 결과입니다.
			const bool bBaselineApplied = VehicleWeaponComp->InitializeWeaponRuntimeFromFitting(
				VehiclePawn,
				BaselineVehicleData,
				ActiveMountProfileId,
				BaselineEquipmentPreset);
			if (!bBaselineApplied)
			{
				ShowMissileFeelMessage(TEXT("[미사일 체감 테스트] 기존 DirectTest 복원에 실패했습니다."));
				return false;
			}

			ShowMissileFeelMessage(TEXT("[미사일 체감 테스트] 기존 DirectTest 설정으로 복원했습니다."));
			return true;
		}

		// transient UObject 이름에서 현재 Variant를 즉시 식별하기 위한 짧은 이름입니다.
		const FString VariantName = GetMissileFeelVariantName(Variant);

		// 저장 ProjectileData를 건드리지 않고 PIE 수명 동안만 사용할 고유 transient Projectile 이름입니다.
		const FName TransientProjectileName = MakeUniqueObjectName(
			VehiclePawn,
			UCFProjectileData::StaticClass(),
			*FString::Printf(TEXT("MissileFeel_%s_Projectile"), *VariantName));

		// 저장 DirectTest의 Mesh/Flight/Collision/Damage/FX를 그대로 복제할 transient ProjectileData입니다.
		UCFProjectileData* TransientProjectileData = DuplicateObject<UCFProjectileData>(
			BaselineProjectileData,
			VehiclePawn,
			TransientProjectileName);

		// 저장 WeaponData를 건드리지 않고 transient ProjectileData만 연결할 고유 transient Weapon 이름입니다.
		const FName TransientWeaponName = MakeUniqueObjectName(
			VehiclePawn,
			UCFWeaponData::StaticClass(),
			*FString::Printf(TEXT("MissileFeel_%s_Weapon"), *VariantName));

		// 저장 DirectTest Weapon 설정을 그대로 복제할 transient WeaponData입니다.
		UCFWeaponData* TransientWeaponData = DuplicateObject<UCFWeaponData>(
			BaselineWeaponData,
			VehiclePawn,
			TransientWeaponName);

		// 저장 EquipmentPresetData를 건드리지 않고 transient WeaponData만 연결할 고유 transient Equipment 이름입니다.
		const FName TransientEquipmentName = MakeUniqueObjectName(
			VehiclePawn,
			UCFEquipmentPresetData::StaticClass(),
			*FString::Printf(TEXT("MissileFeel_%s_Equipment"), *VariantName));

		// 저장 DirectTest의 Turret/Mount 요구를 그대로 복제할 transient EquipmentPresetData입니다.
		UCFEquipmentPresetData* TransientEquipmentPreset = DuplicateObject<UCFEquipmentPresetData>(
			BaselineEquipmentPreset,
			VehiclePawn,
			TransientEquipmentName);

		if (!TransientProjectileData || !TransientWeaponData || !TransientEquipmentPreset)
		{
			ShowMissileFeelMessage(TEXT("[미사일 체감 테스트] transient DirectTest 복제에 실패했습니다."));
			return false;
		}

		TransientProjectileData->SetFlags(RF_Transient);
		TransientWeaponData->SetFlags(RF_Transient);
		TransientEquipmentPreset->SetFlags(RF_Transient);

		// 저장 Guidance Preset DataAsset에서 읽어온 현재 Variant 유도 설정입니다.
		FCFMissileGuideConfig VariantGuideConfig;
		if (!ResolveMissileFeelGuideConfig(Variant, VariantGuideConfig))
		{
			ShowMissileFeelMessage(FString::Printf(
				TEXT("[미사일 체감 테스트] %s Guidance Preset DataAsset을 읽지 못했습니다."),
				GetMissileFeelVariantDisplayName(Variant)));
			return false;
		}

		TransientProjectileData->MissileGuideConfig = VariantGuideConfig;
		TransientWeaponData->DefaultProjectileData = TransientProjectileData;
		TransientEquipmentPreset->DefaultWeaponData = TransientWeaponData;

		// 실제 VehicleWeaponComp가 transient 장비 체인을 활성 Runtime으로 캐시했는지 나타냅니다.
		const bool bVariantApplied = VehicleWeaponComp->InitializeWeaponRuntimeFromFitting(
			VehiclePawn,
			BaselineVehicleData,
			ActiveMountProfileId,
			TransientEquipmentPreset);
		if (!bVariantApplied
			|| VehicleWeaponComp->GetActiveWeaponData() != TransientWeaponData
			|| VehicleWeaponComp->GetActiveProjectileData() != TransientProjectileData)
		{
			VehicleWeaponComp->InitializeWeaponRuntimeFromFitting(
				VehiclePawn,
				BaselineVehicleData,
				ActiveMountProfileId,
				BaselineEquipmentPreset);
			ShowMissileFeelMessage(TEXT("[미사일 체감 테스트] Variant 적용 검증에 실패해 기존 DirectTest로 복원했습니다."));
			return false;
		}

		ShowMissileFeelMessage(FString::Printf(
			TEXT("[미사일 체감 테스트] %s | %s | %s | 활성 %.2fs/%.0fcm | 획득 %.0fdeg | 추적 %.0fdeg | 관측 %.2fs | 선회 %.0fdeg/s"),
			GetMissileFeelVariantDisplayName(Variant),
			GetMissileFeelGuidanceLawDisplayName(VariantGuideConfig.GuidanceLaw),
			GetMissileFeelActivationDisplayName(VariantGuideConfig.GuidanceActivationMode),
			VariantGuideConfig.GuidanceActivationDelaySeconds,
			VariantGuideConfig.GuidanceActivationDistanceCm,
			VariantGuideConfig.AcquisitionConeHalfAngleDeg,
			VariantGuideConfig.TrackingConeHalfAngleDeg,
			VariantGuideConfig.TargetObservationIntervalSeconds,
			VariantGuideConfig.MaximumTurnRateDegPerSec));
		return true;
	}

	// 기존 저장 DirectTest Guidance를 현재 PIE Runtime에 다시 적용합니다.
	void ApplyMissileFeelBaselineCommand()
	{
		ApplyMissileFeelVariant(EMissileFeelTestVariant::Baseline);
	}

	// 저성능 transient Guidance Variant를 현재 PIE Runtime에 적용합니다.
	void ApplyMissileFeelLowCommand()
	{
		ApplyMissileFeelVariant(EMissileFeelTestVariant::Low);
	}

	// 기준형 transient Guidance Variant를 현재 PIE Runtime에 적용합니다.
	void ApplyMissileFeelNormalCommand()
	{
		ApplyMissileFeelVariant(EMissileFeelTestVariant::Normal);
	}

	// 고성능 transient Guidance Variant를 현재 PIE Runtime에 적용합니다.
	void ApplyMissileFeelHighCommand()
	{
		ApplyMissileFeelVariant(EMissileFeelTestVariant::High);
	}

	// 기존 저장 DirectTest 설정으로 복원하는 Editor 전용 콘솔 명령입니다.
	FAutoConsoleCommand MissileFeelBaselineCommand(
		TEXT("CarFight.MissileFeel.Baseline"),
		TEXT("MissileDirectTest PIE: 저장 DirectTest Guidance 설정으로 복원합니다."),
		FConsoleCommandDelegate::CreateStatic(&ApplyMissileFeelBaselineCommand));

	// 저성능 USER 체감 Variant를 선택하는 Editor 전용 콘솔 명령입니다.
	FAutoConsoleCommand MissileFeelLowCommand(
		TEXT("CarFight.MissileFeel.Low"),
		TEXT("MissileDirectTest PIE: 저성능 transient Guidance Variant를 적용합니다."),
		FConsoleCommandDelegate::CreateStatic(&ApplyMissileFeelLowCommand));

	// 기준형 USER 체감 Variant를 선택하는 Editor 전용 콘솔 명령입니다.
	FAutoConsoleCommand MissileFeelNormalCommand(
		TEXT("CarFight.MissileFeel.Normal"),
		TEXT("MissileDirectTest PIE: 기준형 transient Guidance Variant를 적용합니다."),
		FConsoleCommandDelegate::CreateStatic(&ApplyMissileFeelNormalCommand));

	// 고성능 USER 체감 Variant를 선택하는 Editor 전용 콘솔 명령입니다.
	FAutoConsoleCommand MissileFeelHighCommand(
		TEXT("CarFight.MissileFeel.High"),
		TEXT("MissileDirectTest PIE: 고성능 transient Guidance Variant를 적용합니다."),
		FConsoleCommandDelegate::CreateStatic(&ApplyMissileFeelHighCommand));

#if WITH_DEV_AUTOMATION_TESTS

	// MG-P0-12A가 도입한 Editor-only USER 체감 비교 명령 등록 기반이 계속 유지되는지 검증하는 Automation입니다.
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(
		FCFMissileFeelCommandSetupTest,
		"CarFight.Missile.MG_P0_12A.FeelCommandSetup",
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	// MG-P0-12E가 재배선한 Guidance Law·Activation·Seeker geometry Variant 행렬을 검증하는 Focused Automation입니다.
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(
		FCFMissileFeelSetupRewireTest,
		"CarFight.Missile.MG_P0_12E.FeelSetupRewire",
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	// Low/Normal/High Guidance Preset 3개를 명시 실행 시에만 persisted asset으로 생성·보존하는 Authoring Automation입니다.
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(
		FCFMissileFeelPresetAuthoringTest,
		"CarFight.Authoring.MissileFeel.EnsurePresets",
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	// 네 콘솔 명령 등록과 12A Editor-only transient 시험 기반이 유지되는지 검증합니다.
	bool FCFMissileFeelCommandSetupTest::RunTest(const FString& Parameters)
	{
		(void)Parameters;

		TestNotNull(TEXT("Baseline 체감 명령 등록"), IConsoleManager::Get().FindConsoleObject(TEXT("CarFight.MissileFeel.Baseline")));
		TestNotNull(TEXT("Low 체감 명령 등록"), IConsoleManager::Get().FindConsoleObject(TEXT("CarFight.MissileFeel.Low")));
		TestNotNull(TEXT("Normal 체감 명령 등록"), IConsoleManager::Get().FindConsoleObject(TEXT("CarFight.MissileFeel.Normal")));
		TestNotNull(TEXT("High 체감 명령 등록"), IConsoleManager::Get().FindConsoleObject(TEXT("CarFight.MissileFeel.High")));

		// Low USER 체감 수치를 소유하는 저장 Guidance Preset입니다.
		UCFMissileGuidePresetData* LowGuidePreset = LoadMissileFeelPreset(EMissileFeelTestVariant::Low);

		// Normal USER 체감 수치를 소유하는 저장 Guidance Preset입니다.
		UCFMissileGuidePresetData* NormalGuidePreset = LoadMissileFeelPreset(EMissileFeelTestVariant::Normal);

		// High USER 체감 수치를 소유하는 저장 Guidance Preset입니다.
		UCFMissileGuidePresetData* HighGuidePreset = LoadMissileFeelPreset(EMissileFeelTestVariant::High);

		TestNotNull(TEXT("Low Guidance Preset DataAsset 존재"), LowGuidePreset);
		TestNotNull(TEXT("Normal Guidance Preset DataAsset 존재"), NormalGuidePreset);
		TestNotNull(TEXT("High Guidance Preset DataAsset 존재"), HighGuidePreset);

		return true;
	}

	// Low/Normal/High Guidance Preset 세 개와 사용자 튜닝값의 Ensure 재실행·디스크 reload 보존을 검증합니다.
	bool FCFMissileFeelPresetAuthoringTest::RunTest(const FString& Parameters)
	{
		(void)Parameters;
		TestTrue(TEXT("Low/Normal/High Guidance Preset 3개 생성·재사용"), EnsureAllMissileFeelPresetAssets());

		// 실제 USER Preset 3개를 훼손하지 않고 persisted round-trip을 검증할 임시 Asset 이름 suffix입니다.
		const FString TestAssetSuffix = FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(8);

		// 32자 이하를 유지하는 임시 Guidance Preset Asset 이름입니다.
		const FString TestAssetName = FString::Printf(TEXT("DA_MFeel_Idem_%s"), *TestAssetSuffix);

		// 임시 Preset을 저장할 전용 package 경로입니다.
		const FString TestPackagePath = FString::Printf(
			TEXT("/Game/Test/CarFight/Missile/Automation/%s"),
			*TestAssetName);

		// 임시 Preset을 정확히 다시 로드할 object path입니다.
		const FString TestObjectPath = FString::Printf(
			TEXT("%s.%s"),
			*TestPackagePath,
			*TestAssetName);

		// 테스트 종료 시 제거할 실제 임시 .uasset 파일 경로입니다.
		const FString TestPackageFilename = FPackageName::LongPackageNameToFilename(
			TestPackagePath,
			FPackageName::GetAssetPackageExtension());

		// 혹시 SavePackage가 분리 payload를 만들었을 경우 함께 정리할 .uexp 경로입니다.
		const FString TestExportFilename = FPaths::ChangeExtension(TestPackageFilename, TEXT("uexp"));

		// 혹시 bulk payload가 생겼을 경우 함께 정리할 .ubulk 경로입니다.
		const FString TestBulkFilename = FPaths::ChangeExtension(TestPackageFilename, TEXT("ubulk"));

		// 최초 Low seed로 생성될 임시 persisted Preset입니다.
		UCFMissileGuidePresetData* TestPresetAsset = nullptr;

		// 임시 Asset의 최초 생성·저장 성공 여부입니다.
		const bool bInitialEnsureSucceeded = EnsureMissileFeelPresetAsset(
			EMissileFeelTestVariant::Low,
			*TestPackagePath,
			*TestObjectPath,
			*TestAssetName,
			TestPresetAsset);
		TestTrue(TEXT("Idempotence 임시 Preset 최초 생성"), bInitialEnsureSucceeded);
		TestNotNull(TEXT("Idempotence 임시 Preset 포인터"), TestPresetAsset);

		if (TestPresetAsset)
		{
			// C++ struct 기본값과 동일한 USER 튜닝값도 load-time seed에 오염되지 않는지 비교할 기본 Guidance 값입니다.
			const FCFMissileGuideConfig DefaultGuideConfig;

			// 디스크 reload 뒤에도 보존돼야 하는 USER 설명 sentinel입니다.
			const FString UserDescriptionSentinel = TEXT("MG-P0-12E Idempotence User Tune");

			// 디스크 reload 뒤에도 보존돼야 하는 명시적 USER 응답시간 sentinel입니다.
			constexpr float UserGuidanceResponseSentinel = 0.123f;

			// 최초 Low seed와 다르게 바꿀 USER Guidance Law sentinel입니다.
			constexpr ECFMissileGuidanceLaw UserGuidanceLawSentinel = ECFMissileGuidanceLaw::LeadPursuit;

			TestPresetAsset->PresetDescription = FText::FromString(UserDescriptionSentinel);
			TestPresetAsset->MissileGuideConfig.GuidanceLaw = UserGuidanceLawSentinel;
			TestPresetAsset->MissileGuideConfig.GuidanceResponseTimeSeconds = UserGuidanceResponseSentinel;
			TestPresetAsset->MissileGuideConfig.MaximumTurnRateDegPerSec = DefaultGuideConfig.MaximumTurnRateDegPerSec;
			TestPresetAsset->GetOutermost()->MarkPackageDirty();

			TestTrue(TEXT("USER 튜닝값 임시 Preset 저장"), SaveMissileFeelPresetAsset(TestPresetAsset));
			TestFalse(TEXT("USER 튜닝 저장 직후 package clean"), TestPresetAsset->GetOutermost()->IsDirty());

			// 같은 object path에 일부러 다른 High seed 요청을 보내도 existing Asset을 재seed하지 않는지 확인할 포인터입니다.
			UCFMissileGuidePresetData* ReusedPresetAsset = nullptr;

			// 기존 Asset 재사용 경로의 Ensure 성공 여부입니다.
			const bool bRepeatedEnsureSucceeded = EnsureMissileFeelPresetAsset(
				EMissileFeelTestVariant::High,
				*TestPackagePath,
				*TestObjectPath,
				*TestAssetName,
				ReusedPresetAsset);
			TestTrue(TEXT("기존 Preset Ensure 재실행 성공"), bRepeatedEnsureSucceeded);
			TestEqual(TEXT("Ensure 재실행은 existing Asset 재사용"), ReusedPresetAsset, TestPresetAsset);
			TestFalse(TEXT("Ensure 재실행은 existing package를 dirty로 만들지 않음"), TestPresetAsset->GetOutermost()->IsDirty());
			TestEqual(TEXT("Ensure 재실행 뒤 USER Guidance Law 보존"), TestPresetAsset->MissileGuideConfig.GuidanceLaw, UserGuidanceLawSentinel);
			TestTrue(TEXT("Ensure 재실행 뒤 USER 응답시간 보존"), FMath::IsNearlyEqual(TestPresetAsset->MissileGuideConfig.GuidanceResponseTimeSeconds, UserGuidanceResponseSentinel));
			TestTrue(TEXT("Ensure 재실행 뒤 C++ 기본값과 동일한 USER 선회율 보존"), FMath::IsNearlyEqual(TestPresetAsset->MissileGuideConfig.MaximumTurnRateDegPerSec, DefaultGuideConfig.MaximumTurnRateDegPerSec));

			// 저장된 사용자 값을 메모리 캐시가 아니라 실제 디스크 package에서 다시 읽기 위해 먼저 unload할 package입니다.
			UPackage* PackageBeforeReload = TestPresetAsset->GetOutermost();

			// 기존 package를 안전하게 unload하기 위한 exact package 배열입니다.
			TArray<UPackage*> PackagesToUnload;
			PackagesToUnload.Add(PackageBeforeReload);

			TestPresetAsset = nullptr;
			ReusedPresetAsset = nullptr;

			// persisted 파일만 진실로 남긴 뒤 동일 파일을 다시 읽기 위한 Editor package unload 결과입니다.
			const bool bPackageUnloaded = UPackageTools::UnloadPackages(PackagesToUnload);
			TestTrue(TEXT("Idempotence 임시 Preset memory package unload"), bPackageUnloaded);

			// unload 완료 뒤 exact .uasset 파일에서 새 package를 로드한 디스크 readback package입니다.
			UPackage* ReloadedPackage = bPackageUnloaded
				? UPackageTools::LoadPackage(TestPackageFilename, LOAD_None)
				: nullptr;
			TestNotNull(TEXT("Idempotence 임시 Preset 디스크 package reload"), ReloadedPackage);

			// 새로 로드한 package 안에서 exact object name으로 찾은 Guidance Preset입니다.
			UCFMissileGuidePresetData* ReloadedPresetAsset = ReloadedPackage
				? FindObject<UCFMissileGuidePresetData>(ReloadedPackage, *TestAssetName)
				: nullptr;
			TestNotNull(TEXT("Idempotence 임시 Preset 디스크 readback"), ReloadedPresetAsset);

			if (ReloadedPresetAsset)
			{
				TestEqual(TEXT("Reload 뒤 USER 설명 보존"), ReloadedPresetAsset->PresetDescription.ToString(), UserDescriptionSentinel);
				TestEqual(TEXT("Reload 뒤 USER Guidance Law 보존"), ReloadedPresetAsset->MissileGuideConfig.GuidanceLaw, UserGuidanceLawSentinel);
				TestTrue(TEXT("Reload 뒤 USER 응답시간 보존"), FMath::IsNearlyEqual(ReloadedPresetAsset->MissileGuideConfig.GuidanceResponseTimeSeconds, UserGuidanceResponseSentinel));
				TestTrue(TEXT("Reload 뒤 C++ 기본값과 동일한 USER 선회율 보존"), FMath::IsNearlyEqual(ReloadedPresetAsset->MissileGuideConfig.MaximumTurnRateDegPerSec, DefaultGuideConfig.MaximumTurnRateDegPerSec));
			}

			if (ReloadedPackage)
			{
				// readback package도 물리 파일 정리 전에 unload하기 위한 exact package 배열입니다.
				TArray<UPackage*> ReloadedPackagesToUnload;
				ReloadedPackagesToUnload.Add(ReloadedPackage);
				ReloadedPresetAsset = nullptr;

				// 테스트가 임시 persisted package를 남기지 않도록 readback package까지 unload한 결과입니다.
				const bool bReloadedPackageUnloaded = UPackageTools::UnloadPackages(ReloadedPackagesToUnload);
				TestTrue(TEXT("Idempotence readback package unload"), bReloadedPackageUnloaded);
			}
		}

		// unload 완료 뒤 테스트 전용 persisted main package를 물리적으로 제거한 결과입니다.
		const bool bTestPackageDeleted = IFileManager::Get().Delete(*TestPackageFilename, false, true, true);
		IFileManager::Get().Delete(*TestExportFilename, false, true, true);
		IFileManager::Get().Delete(*TestBulkFilename, false, true, true);
		TestTrue(TEXT("Idempotence 임시 .uasset 삭제 성공"), bTestPackageDeleted);
		TestFalse(TEXT("Idempotence 임시 .uasset 정리 완료"), IFileManager::Get().FileExists(*TestPackageFilename));

		return !HasAnyErrors();
	}

	// Low/Normal/High가 수치만 다른 같은 PN이 아니라 Law·Activation·Seeker geometry까지 다른 현재 12E fixture인지 검증합니다.
	bool FCFMissileFeelSetupRewireTest::RunTest(const FString& Parameters)
	{
		(void)Parameters;

		// Low USER 체감 수치를 소유하는 저장 Guidance Preset입니다.
		UCFMissileGuidePresetData* LowGuidePreset = LoadMissileFeelPreset(EMissileFeelTestVariant::Low);

		// Normal USER 체감 수치를 소유하는 저장 Guidance Preset입니다.
		UCFMissileGuidePresetData* NormalGuidePreset = LoadMissileFeelPreset(EMissileFeelTestVariant::Normal);

		// High USER 체감 수치를 소유하는 저장 Guidance Preset입니다.
		UCFMissileGuidePresetData* HighGuidePreset = LoadMissileFeelPreset(EMissileFeelTestVariant::High);
		TestNotNull(TEXT("Low Guidance Preset 존재"), LowGuidePreset);
		TestNotNull(TEXT("Normal Guidance Preset 존재"), NormalGuidePreset);
		TestNotNull(TEXT("High Guidance Preset 존재"), HighGuidePreset);
		if (!LowGuidePreset || !NormalGuidePreset || !HighGuidePreset)
		{
			return false;
		}

		// Low Preset의 저장값을 안전 보정한 실제 Runtime Guidance 설정입니다.
		const auto LowGuideConfig = LowGuidePreset->MissileGuideConfig.GetEffectiveConfig();

		// Normal Preset의 저장값을 안전 보정한 실제 Runtime Guidance 설정입니다.
		const auto NormalGuideConfig = NormalGuidePreset->MissileGuideConfig.GetEffectiveConfig();

		// High Preset의 저장값을 안전 보정한 실제 Runtime Guidance 설정입니다.
		const auto HighGuideConfig = HighGuidePreset->MissileGuideConfig.GetEffectiveConfig();

		TestTrue(TEXT("Low Preset 안정 ID"), LowGuidePreset->PresetId == FName(TEXT("MissileFeel_Low")));
		TestTrue(TEXT("Normal Preset 안정 ID"), NormalGuidePreset->PresetId == FName(TEXT("MissileFeel_Normal")));
		TestTrue(TEXT("High Preset 안정 ID"), HighGuidePreset->PresetId == FName(TEXT("MissileFeel_High")));

		TestTrue(TEXT("Low/Normal/High 모두 Guidance 활성"),
			LowGuideConfig.IsGuidanceEnabled()
			&& NormalGuideConfig.IsGuidanceEnabled()
			&& HighGuideConfig.IsGuidanceEnabled());
		TestTrue(TEXT("Low/Normal/High 모두 Stateful"),
			LowGuideConfig.SeekerModel == ECFMissileSeekerModel::Stateful
			&& NormalGuideConfig.SeekerModel == ECFMissileSeekerModel::Stateful
			&& HighGuideConfig.SeekerModel == ECFMissileSeekerModel::Stateful);
		TestTrue(TEXT("Low/Normal/High 모두 SampledPositionEstimate"),
			LowGuideConfig.TargetObservationMode == ECFMissileTargetObservationMode::SampledPositionEstimate
			&& NormalGuideConfig.TargetObservationMode == ECFMissileTargetObservationMode::SampledPositionEstimate
			&& HighGuideConfig.TargetObservationMode == ECFMissileTargetObservationMode::SampledPositionEstimate);

		TestEqual(TEXT("Low Guidance Law = PurePursuit"), LowGuideConfig.GuidanceLaw, ECFMissileGuidanceLaw::PurePursuit);
		TestEqual(TEXT("Low 재포착 없음"), LowGuideConfig.ReacquisitionMode, ECFMissileReacquisitionMode::None);
		TestTrue(TEXT("Low는 예측 선행값을 사용하지 않음"),
			FMath::IsNearlyZero(LowGuideConfig.LeadTimeSeconds)
			&& FMath::IsNearlyZero(LowGuideConfig.MaxLeadDistanceCm));

		TestEqual(TEXT("Normal Guidance Law = LeadPursuit"), NormalGuideConfig.GuidanceLaw, ECFMissileGuidanceLaw::LeadPursuit);
		TestEqual(TEXT("Normal 재포착 = ForwardCone"), NormalGuideConfig.ReacquisitionMode, ECFMissileReacquisitionMode::ForwardCone);
		TestTrue(TEXT("Normal은 bounded Lead 설정을 사용"),
			NormalGuideConfig.LeadTimeSeconds > KINDA_SMALL_NUMBER
			&& NormalGuideConfig.MaxLeadDistanceCm > KINDA_SMALL_NUMBER);

		TestEqual(TEXT("High Guidance Law = PN"), HighGuideConfig.GuidanceLaw, ECFMissileGuidanceLaw::ProportionalNavigation);
		TestEqual(TEXT("High 재포착 = ForwardCone"), HighGuideConfig.ReacquisitionMode, ECFMissileReacquisitionMode::ForwardCone);
		TestTrue(TEXT("High는 rear-aspect 시험 가능한 넓은 Seeker 구조"),
			HighGuideConfig.AcquisitionConeHalfAngleDeg >= 179.9f
			&& HighGuideConfig.TrackingConeHalfAngleDeg >= 179.9f
			&& HighGuideConfig.ReacquisitionConeHalfAngleDeg >= 179.9f);

		TestTrue(TEXT("Guidance Law 세 종류가 실제로 서로 다름"),
			LowGuideConfig.GuidanceLaw != NormalGuideConfig.GuidanceLaw
			&& NormalGuideConfig.GuidanceLaw != HighGuideConfig.GuidanceLaw
			&& LowGuideConfig.GuidanceLaw != HighGuideConfig.GuidanceLaw);

		return true;
	}

#endif // WITH_DEV_AUTOMATION_TESTS
}

#endif // WITH_EDITOR
