// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.1
// Date: 2026-09-07
// Description: CF-FQ-030 MG-P0-12A MissileDirectTest 전용 USER 체감 비교 콘솔 명령
// Scope: 저장 DirectTest 자산을 수정하지 않고 PIE Runtime에서만 Low/Normal/High transient Guidance 설정을 실제 발사 경로에 적용합니다.
// Changelog:
// - v1.1.1: USER 시험 전용 소유권에 맞춰 CarFight_Re 런타임 모듈에서 CarFight_ReEditor 모듈로 파일을 이동. 동작과 명령 계약 변경 없음.
// - v1.1.0: MG-P0-12A Focused Automation을 추가해 네 콘솔 명령 등록과 Low/Normal/High GuideConfig exact value를 PIE 없이 검증.
// - v1.0.1: UE 5.8 실제 FAutoConsoleCommand API에 맞춰 인수형 단일 명령을 Baseline/Low/Normal/High 무인수 명령 4개로 교정.
// - v1.0.0: CarFight.MissileFeel.Set <Baseline|Low|Normal|High> 명령과 transient ProjectileData/WeaponData/EquipmentPresetData override 경로 최초 추가.
// Migration:
// - 이 파일은 CarFight_ReEditor 모듈에만 존재하고 WITH_EDITOR에서 명령을 등록하므로 패키징 Product Runtime에는 품질 enum이나 분기를 추가하지 않습니다.
// - /Game/CarFight/Tests/Missile의 저장 DirectTest DataAsset/Blueprint/Map은 읽기 전용 기준으로 사용하고 저장·수정하지 않습니다.
// - Low/Normal/High는 USER 체감 비교용 transient fixture 이름이며 Product 데이터 분류가 아닙니다.
// - Baseline 명령은 저장 EQ_Missile_DirectTest를 다시 적용해 PIE Runtime을 원래 DirectTest 구성으로 복원합니다.

#include "CFEquipmentPresetData.h"
#include "CFProjectileData.h"
#include "CFVehicleData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"

#if WITH_EDITOR

namespace CFMissileFeelCommands
{
	/** MG-P0-12A에서만 사용하는 USER 체감 비교 Variant입니다. Product Runtime 타입이 아닙니다. */
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

	// 저장 DirectTest Guidance를 기준으로 선택 Variant의 체감 비교용 설정 복사본을 만듭니다.
	FCFMissileGuideConfig BuildMissileFeelGuideConfig(
		const EMissileFeelTestVariant Variant,
		const FCFMissileGuideConfig& BaselineGuideConfig)
	{
		// 저장 DirectTest의 비성능 계약과 미래 호환 필드를 먼저 그대로 보존할 설정 복사본입니다.
		FCFMissileGuideConfig GuideConfig = BaselineGuideConfig;
		if (Variant == EMissileFeelTestVariant::Baseline)
		{
			return GuideConfig;
		}

		GuideConfig.bUseGuidance = true;
		GuideConfig.GuideMode = ECFMissileGuideMode::TargetActor;
		GuideConfig.LostTargetPolicy = ECFMissileLostTargetPolicy::ContinueStraight;
		GuideConfig.NavigationConstant = 3.0f;
		GuideConfig.MinimumGuidanceSpeedCmPerSec = 500.0f;
		GuideConfig.SeekerModel = ECFMissileSeekerModel::Stateful;
		GuideConfig.TargetObservationMode = ECFMissileTargetObservationMode::SampledPositionEstimate;

		switch (Variant)
		{
		case EMissileFeelTestVariant::Low:
			GuideConfig.TargetObservationIntervalSeconds = 0.15f;
			GuideConfig.TargetVelocityEstimateResponseTimeSeconds = 0.35f;
			GuideConfig.GuidanceResponseTimeSeconds = 0.30f;
			GuideConfig.MaximumTurnRateDegPerSec = 25.0f;
			GuideConfig.MaximumLateralAccelerationCmPerSecSq = 1200.0f;
			GuideConfig.AcquisitionConeHalfAngleDeg = 25.0f;
			GuideConfig.TrackingConeHalfAngleDeg = 35.0f;
			GuideConfig.TargetLostGraceTimeSeconds = 0.10f;
			GuideConfig.ReacquisitionMode = ECFMissileReacquisitionMode::None;
			GuideConfig.ReacquisitionConeHalfAngleDeg = 35.0f;
			GuideConfig.ReacquisitionTimeSeconds = 0.0f;
			break;

		case EMissileFeelTestVariant::Normal:
			GuideConfig.TargetObservationIntervalSeconds = 0.08f;
			GuideConfig.TargetVelocityEstimateResponseTimeSeconds = 0.20f;
			GuideConfig.GuidanceResponseTimeSeconds = 0.20f;
			GuideConfig.MaximumTurnRateDegPerSec = 50.0f;
			GuideConfig.MaximumLateralAccelerationCmPerSecSq = 6000.0f;
			GuideConfig.AcquisitionConeHalfAngleDeg = 35.0f;
			GuideConfig.TrackingConeHalfAngleDeg = 65.0f;
			GuideConfig.TargetLostGraceTimeSeconds = 0.40f;
			GuideConfig.ReacquisitionMode = ECFMissileReacquisitionMode::ForwardCone;
			GuideConfig.ReacquisitionConeHalfAngleDeg = 45.0f;
			GuideConfig.ReacquisitionTimeSeconds = 0.30f;
			break;

		case EMissileFeelTestVariant::High:
			GuideConfig.TargetObservationIntervalSeconds = 0.03f;
			GuideConfig.TargetVelocityEstimateResponseTimeSeconds = 0.10f;
			GuideConfig.GuidanceResponseTimeSeconds = 0.10f;
			GuideConfig.MaximumTurnRateDegPerSec = 80.0f;
			GuideConfig.MaximumLateralAccelerationCmPerSecSq = 15000.0f;
			GuideConfig.AcquisitionConeHalfAngleDeg = 50.0f;
			GuideConfig.TrackingConeHalfAngleDeg = 85.0f;
			GuideConfig.TargetLostGraceTimeSeconds = 0.80f;
			GuideConfig.ReacquisitionMode = ECFMissileReacquisitionMode::ForwardCone;
			GuideConfig.ReacquisitionConeHalfAngleDeg = 65.0f;
			GuideConfig.ReacquisitionTimeSeconds = 0.60f;
			break;

		case EMissileFeelTestVariant::Baseline:
		default:
			break;
		}

		return GuideConfig.GetEffectiveConfig();
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

		// 저장 DirectTest에서 오직 Guidance 성능축만 교체한 현재 Variant 설정입니다.
		const FCFMissileGuideConfig VariantGuideConfig = BuildMissileFeelGuideConfig(
			Variant,
			BaselineProjectileData->MissileGuideConfig);
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
			TEXT("[미사일 체감 테스트] %s 적용 | 관측 %.2fs | 속도추정 %.2fs | 유도응답 %.2fs | 선회 %.0fdeg/s | 횡가속 %.0f"),
			GetMissileFeelVariantDisplayName(Variant),
			VariantGuideConfig.TargetObservationIntervalSeconds,
			VariantGuideConfig.TargetVelocityEstimateResponseTimeSeconds,
			VariantGuideConfig.GuidanceResponseTimeSeconds,
			VariantGuideConfig.MaximumTurnRateDegPerSec,
			VariantGuideConfig.MaximumLateralAccelerationCmPerSecSq));
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

	// MG-P0-12A USER 체감 비교 명령 등록과 Variant exact config를 Editor process에서 검증하는 Focused Automation입니다.
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(
		FCFMissileFeelCommandSetupTest,
		"CarFight.Missile.MG_P0_12A.FeelCommandSetup",
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	// 네 콘솔 명령과 Low/Normal/High 성능 수치가 계획된 MG-P0-11 fixture와 정확히 일치하는지 검증합니다.
	bool FCFMissileFeelCommandSetupTest::RunTest(const FString& Parameters)
	{
		TestNotNull(TEXT("Baseline 체감 명령 등록"), IConsoleManager::Get().FindConsoleObject(TEXT("CarFight.MissileFeel.Baseline")));
		TestNotNull(TEXT("Low 체감 명령 등록"), IConsoleManager::Get().FindConsoleObject(TEXT("CarFight.MissileFeel.Low")));
		TestNotNull(TEXT("Normal 체감 명령 등록"), IConsoleManager::Get().FindConsoleObject(TEXT("CarFight.MissileFeel.Normal")));
		TestNotNull(TEXT("High 체감 명령 등록"), IConsoleManager::Get().FindConsoleObject(TEXT("CarFight.MissileFeel.High")));

		// 저장 DirectTest와 무관하게 Variant builder의 비성능 기본 입력을 제공할 테스트 기준 Guidance 설정입니다.
		FCFMissileGuideConfig BaselineGuideConfig;
		BaselineGuideConfig.bUseGuidance = true;
		BaselineGuideConfig.GuideMode = ECFMissileGuideMode::TargetActor;

		// USER 저성능 명령이 실제 Runtime에 주입할 유효 Guidance 설정입니다.
		const FCFMissileGuideConfig LowGuideConfig = BuildMissileFeelGuideConfig(
			EMissileFeelTestVariant::Low,
			BaselineGuideConfig);

		// USER 기준형 명령이 실제 Runtime에 주입할 유효 Guidance 설정입니다.
		const FCFMissileGuideConfig NormalGuideConfig = BuildMissileFeelGuideConfig(
			EMissileFeelTestVariant::Normal,
			BaselineGuideConfig);

		// USER 고성능 명령이 실제 Runtime에 주입할 유효 Guidance 설정입니다.
		const FCFMissileGuideConfig HighGuideConfig = BuildMissileFeelGuideConfig(
			EMissileFeelTestVariant::High,
			BaselineGuideConfig);

		TestEqual(TEXT("Low 관측 간격"), LowGuideConfig.TargetObservationIntervalSeconds, 0.15f);
		TestEqual(TEXT("Low 속도 추정 응답"), LowGuideConfig.TargetVelocityEstimateResponseTimeSeconds, 0.35f);
		TestEqual(TEXT("Low 유도 응답"), LowGuideConfig.GuidanceResponseTimeSeconds, 0.30f);
		TestEqual(TEXT("Low 최대 선회율"), LowGuideConfig.MaximumTurnRateDegPerSec, 25.0f);
		TestEqual(TEXT("Low 최대 횡가속"), LowGuideConfig.MaximumLateralAccelerationCmPerSecSq, 1200.0f);
		TestEqual(TEXT("Low 추적 반각"), LowGuideConfig.TrackingConeHalfAngleDeg, 35.0f);
		TestEqual(TEXT("Low 재포착 없음"), LowGuideConfig.ReacquisitionMode, ECFMissileReacquisitionMode::None);

		TestEqual(TEXT("Normal 관측 간격"), NormalGuideConfig.TargetObservationIntervalSeconds, 0.08f);
		TestEqual(TEXT("Normal 속도 추정 응답"), NormalGuideConfig.TargetVelocityEstimateResponseTimeSeconds, 0.20f);
		TestEqual(TEXT("Normal 유도 응답"), NormalGuideConfig.GuidanceResponseTimeSeconds, 0.20f);
		TestEqual(TEXT("Normal 최대 선회율"), NormalGuideConfig.MaximumTurnRateDegPerSec, 50.0f);
		TestEqual(TEXT("Normal 최대 횡가속"), NormalGuideConfig.MaximumLateralAccelerationCmPerSecSq, 6000.0f);
		TestEqual(TEXT("Normal 추적 반각"), NormalGuideConfig.TrackingConeHalfAngleDeg, 65.0f);
		TestEqual(TEXT("Normal 전방 재포착"), NormalGuideConfig.ReacquisitionMode, ECFMissileReacquisitionMode::ForwardCone);

		TestEqual(TEXT("High 관측 간격"), HighGuideConfig.TargetObservationIntervalSeconds, 0.03f);
		TestEqual(TEXT("High 속도 추정 응답"), HighGuideConfig.TargetVelocityEstimateResponseTimeSeconds, 0.10f);
		TestEqual(TEXT("High 유도 응답"), HighGuideConfig.GuidanceResponseTimeSeconds, 0.10f);
		TestEqual(TEXT("High 최대 선회율"), HighGuideConfig.MaximumTurnRateDegPerSec, 80.0f);
		TestEqual(TEXT("High 최대 횡가속"), HighGuideConfig.MaximumLateralAccelerationCmPerSecSq, 15000.0f);
		TestEqual(TEXT("High 추적 반각"), HighGuideConfig.TrackingConeHalfAngleDeg, 85.0f);
		TestEqual(TEXT("High 전방 재포착"), HighGuideConfig.ReacquisitionMode, ECFMissileReacquisitionMode::ForwardCone);

		TestTrue(TEXT("Low/Normal/High 모두 Stateful"),
			LowGuideConfig.SeekerModel == ECFMissileSeekerModel::Stateful
			&& NormalGuideConfig.SeekerModel == ECFMissileSeekerModel::Stateful
			&& HighGuideConfig.SeekerModel == ECFMissileSeekerModel::Stateful);
		TestTrue(TEXT("Low/Normal/High 모두 SampledPositionEstimate"),
			LowGuideConfig.TargetObservationMode == ECFMissileTargetObservationMode::SampledPositionEstimate
			&& NormalGuideConfig.TargetObservationMode == ECFMissileTargetObservationMode::SampledPositionEstimate
			&& HighGuideConfig.TargetObservationMode == ECFMissileTargetObservationMode::SampledPositionEstimate);
		TestTrue(TEXT("성능 순서가 관측 주기에서 High < Normal < Low"),
			HighGuideConfig.TargetObservationIntervalSeconds
			< NormalGuideConfig.TargetObservationIntervalSeconds
			&& NormalGuideConfig.TargetObservationIntervalSeconds
			< LowGuideConfig.TargetObservationIntervalSeconds);
		TestTrue(TEXT("성능 순서가 최대 선회율에서 Low < Normal < High"),
			LowGuideConfig.MaximumTurnRateDegPerSec
			< NormalGuideConfig.MaximumTurnRateDegPerSec
			&& NormalGuideConfig.MaximumTurnRateDegPerSec
			< HighGuideConfig.MaximumTurnRateDegPerSec);

		return true;
	}

#endif // WITH_DEV_AUTOMATION_TESTS
}

#endif // WITH_EDITOR
