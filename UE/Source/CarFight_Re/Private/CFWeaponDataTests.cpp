// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-15
// Description: CF-FQ-008 WD-P0-01 WeaponData 정적 계약과 WD-P0-02 대표 자산 Load-only 자동화 테스트
// Scope: Transient WeaponData 정적 Validation과 대표 WeaponData 2개의 현재 저장 계약을 Content mutation 없이 검증합니다.
// Changelog:
// - v1.0.0: StaticContract와 RepresentativeAssets 테스트 최초 추가.
// Migration:
// - 대표 WeaponData는 LoadObject로 읽기만 하며 Compile, Save, Fixup을 수행하지 않습니다.
// - FireRate=0, MaxRange=0, WeaponMass=0, DefaultProjectileData=None과 infinite-ammo 호환은 유효 계약으로 보존합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFWeaponData.h"

#include "CFAmmoData.h"
#include "CFProjectileData.h"
#include "Misc/AutomationTest.h"

#include <limits>


namespace
{
	// [v1.0.0] 기본적으로 정적 계약이 유효한 Transient WeaponData를 생성합니다.
	UCFWeaponData* CreateValidTransientWeaponData(const FName ObjectName)
	{
		// [v1.0.0] 각 독립 검증 시나리오에서 사용할 Transient WeaponData입니다.
		UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(GetTransientPackage(), ObjectName);
		if (WeaponData)
		{
			WeaponData->WeaponId = TEXT("TestWeapon");
			WeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
			WeaponData->CompatibleMountTypes.Reset();
			WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
			WeaponData->WeaponMassKg = 0.0f;
			WeaponData->FireRatePerMinute = 0.0f;
			WeaponData->MaxRange = 0.0f;
			WeaponData->SpreadDeg = 0.0f;
			WeaponData->MagazineSize = 0;
			WeaponData->ReloadTimeSeconds = 0.0f;
			WeaponData->DefaultAmmoData = nullptr;
			WeaponData->InitialLoadedAmmoCount = 0;
			WeaponData->AmmoUnitsPerShot = 1;
			WeaponData->bUseInfiniteAmmoForDebug = true;
			WeaponData->HeatPerShot = 0.0f;
			WeaponData->MaxHeat = 0.0f;
		}
		return WeaponData;
	}

	// [v1.0.0] WeaponData 정적 계약을 실행하고 유효 여부만 반환합니다.
	bool IsWeaponDataContractValid(UCFWeaponData* WeaponData)
	{
		if (!WeaponData)
		{
			return false;
		}

		// [v1.0.0] 계약 검증에서 생성된 오류 목록입니다.
		TArray<FText> ValidationErrors;
		return WeaponData->ValidateWeaponDataContract(ValidationErrors);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWeaponDataStaticContractTest,
	"CarFight.WeaponData.WD_P0_01.StaticContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] UCFWeaponData의 정적 Invalid/허용 계약을 Content Asset 없이 검증합니다.
bool FCFWeaponDataStaticContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 기존 fallback-safe 기본값을 사용하는 정상 WeaponData입니다.
	UCFWeaponData* BaselineWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_Baseline"));
	if (!TestNotNull(TEXT("정상 Transient WeaponData 생성"), BaselineWeaponData))
	{
		return false;
	}
	TestTrue(TEXT("0 질량/0 FireRate/0 Range/infinite ammo/Projectile None 허용"), IsWeaponDataContractValid(BaselineWeaponData));

	// [v1.0.0] WeaponId 누락 계약을 검증할 WeaponData입니다.
	UCFWeaponData* MissingIdWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_MissingId"));
	MissingIdWeaponData->WeaponId = NAME_None;
	TestFalse(TEXT("WeaponId None 거부"), IsWeaponDataContractValid(MissingIdWeaponData));

	// [v1.0.0] WeaponSize 누락 계약을 검증할 WeaponData입니다.
	UCFWeaponData* MissingSizeWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_MissingSize"));
	MissingSizeWeaponData->WeaponSize = ECFVehicleWeaponSize::None;
	TestFalse(TEXT("WeaponSize None 거부"), IsWeaponDataContractValid(MissingSizeWeaponData));

	// [v1.0.0] 호환 MountType 빈 배열 계약을 검증할 WeaponData입니다.
	UCFWeaponData* EmptyMountWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_EmptyMount"));
	EmptyMountWeaponData->CompatibleMountTypes.Reset();
	TestFalse(TEXT("CompatibleMountTypes 빈 배열 거부"), IsWeaponDataContractValid(EmptyMountWeaponData));

	// [v1.0.0] None MountType 계약을 검증할 WeaponData입니다.
	UCFWeaponData* NoneMountWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_NoneMount"));
	NoneMountWeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::None);
	TestFalse(TEXT("CompatibleMountTypes None 거부"), IsWeaponDataContractValid(NoneMountWeaponData));

	// [v1.0.0] 중복 MountType 계약을 검증할 WeaponData입니다.
	UCFWeaponData* DuplicateMountWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_DuplicateMount"));
	DuplicateMountWeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
	TestFalse(TEXT("CompatibleMountTypes 중복 거부"), IsWeaponDataContractValid(DuplicateMountWeaponData));

	// [v1.0.0] 음수 WeaponMass 계약을 검증할 WeaponData입니다.
	UCFWeaponData* NegativeMassWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_NegativeMass"));
	NegativeMassWeaponData->WeaponMassKg = -1.0f;
	TestFalse(TEXT("음수 WeaponMassKg 거부"), IsWeaponDataContractValid(NegativeMassWeaponData));

	// [v1.0.0] 비유한 WeaponMass 계약을 검증할 WeaponData입니다.
	UCFWeaponData* InfiniteMassWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_InfiniteMass"));
		InfiniteMassWeaponData->WeaponMassKg = std::numeric_limits<float>::infinity();
	TestFalse(TEXT("비유한 WeaponMassKg 거부"), IsWeaponDataContractValid(InfiniteMassWeaponData));

	// [v1.0.0] 음수 FireRate 계약을 검증할 WeaponData입니다.
	UCFWeaponData* NegativeFireRateWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_NegativeFireRate"));
	NegativeFireRateWeaponData->FireRatePerMinute = -1.0f;
	TestFalse(TEXT("음수 FireRate 거부"), IsWeaponDataContractValid(NegativeFireRateWeaponData));

	// [v1.0.0] 음수 Range 계약을 검증할 WeaponData입니다.
	UCFWeaponData* NegativeRangeWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_NegativeRange"));
	NegativeRangeWeaponData->MaxRange = -1.0f;
	TestFalse(TEXT("음수 MaxRange 거부"), IsWeaponDataContractValid(NegativeRangeWeaponData));

	// [v1.0.0] 음수 Spread 계약을 검증할 WeaponData입니다.
	UCFWeaponData* NegativeSpreadWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_NegativeSpread"));
	NegativeSpreadWeaponData->SpreadDeg = -0.1f;
	TestFalse(TEXT("음수 SpreadDeg 거부"), IsWeaponDataContractValid(NegativeSpreadWeaponData));

	// [v1.0.0] 음수 Magazine 계약을 검증할 WeaponData입니다.
	UCFWeaponData* NegativeMagazineWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_NegativeMagazine"));
	NegativeMagazineWeaponData->MagazineSize = -1;
	TestFalse(TEXT("음수 MagazineSize 거부"), IsWeaponDataContractValid(NegativeMagazineWeaponData));

	// [v1.0.0] 음수 Reload 계약을 검증할 WeaponData입니다.
	UCFWeaponData* NegativeReloadWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_NegativeReload"));
	NegativeReloadWeaponData->ReloadTimeSeconds = -1.0f;
	TestFalse(TEXT("음수 ReloadTime 거부"), IsWeaponDataContractValid(NegativeReloadWeaponData));

	// [v1.0.0] 초기 장전량 초과 계약을 검증할 WeaponData입니다.
	UCFWeaponData* LoadedExceedsMagazineWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_LoadedExceedsMagazine"));
	LoadedExceedsMagazineWeaponData->MagazineSize = 4;
	LoadedExceedsMagazineWeaponData->InitialLoadedAmmoCount = 5;
	TestFalse(TEXT("InitialLoaded > Magazine 거부"), IsWeaponDataContractValid(LoadedExceedsMagazineWeaponData));

	// [v1.0.0] 발사당 탄약량 최소값 계약을 검증할 WeaponData입니다.
	UCFWeaponData* InvalidUnitsPerShotWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_InvalidUnitsPerShot"));
	InvalidUnitsPerShotWeaponData->AmmoUnitsPerShot = 0;
	TestFalse(TEXT("AmmoUnitsPerShot 0 거부"), IsWeaponDataContractValid(InvalidUnitsPerShotWeaponData));

	// [v1.0.0] AmmoId가 없는 명시적 AmmoData 계약을 검증할 WeaponData입니다.
	UCFWeaponData* InvalidAmmoReferenceWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_InvalidAmmoReference"));
	// [v1.0.0] AmmoId가 비어 있어 정적 계약상 유효하지 않은 Transient AmmoData입니다.
	UCFAmmoData* InvalidAmmoData = NewObject<UCFAmmoData>(GetTransientPackage(), TEXT("WD_P0_01_InvalidAmmo"));
	InvalidAmmoReferenceWeaponData->DefaultAmmoData = InvalidAmmoData;
	TestFalse(TEXT("명시적 invalid DefaultAmmoData 거부"), IsWeaponDataContractValid(InvalidAmmoReferenceWeaponData));

	// [v1.0.0] finite ammo 모드에서 AmmoData 누락 계약을 검증할 WeaponData입니다.
	UCFWeaponData* FiniteMissingAmmoWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_FiniteMissingAmmo"));
	FiniteMissingAmmoWeaponData->bUseInfiniteAmmoForDebug = false;
	FiniteMissingAmmoWeaponData->MagazineSize = 4;
	TestFalse(TEXT("finite ammo + DefaultAmmoData None 거부"), IsWeaponDataContractValid(FiniteMissingAmmoWeaponData));

	// [v1.0.0] finite ammo 모드에서 탄창 누락 계약을 검증할 WeaponData입니다.
	UCFWeaponData* FiniteMissingMagazineWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_FiniteMissingMagazine"));
	// [v1.0.0] finite ammo 정상 계약에 사용할 안정 AmmoId Transient AmmoData입니다.
	UCFAmmoData* ValidAmmoData = NewObject<UCFAmmoData>(GetTransientPackage(), TEXT("WD_P0_01_ValidAmmo"));
	ValidAmmoData->AmmoId = TEXT("TestAmmo");
	FiniteMissingMagazineWeaponData->DefaultAmmoData = ValidAmmoData;
	FiniteMissingMagazineWeaponData->bUseInfiniteAmmoForDebug = false;
	FiniteMissingMagazineWeaponData->MagazineSize = 0;
	TestFalse(TEXT("finite ammo + MagazineSize 0 거부"), IsWeaponDataContractValid(FiniteMissingMagazineWeaponData));

	// [v1.0.0] finite ammo 정적 완결성을 검증할 정상 WeaponData입니다.
	UCFWeaponData* ValidFiniteWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_ValidFinite"));
	ValidFiniteWeaponData->DefaultAmmoData = ValidAmmoData;
	ValidFiniteWeaponData->bUseInfiniteAmmoForDebug = false;
	ValidFiniteWeaponData->MagazineSize = 4;
	ValidFiniteWeaponData->InitialLoadedAmmoCount = 3;
	TestTrue(TEXT("유효 finite ammo 계약 통과"), IsWeaponDataContractValid(ValidFiniteWeaponData));
	TestTrue(TEXT("유효 finite ammo Runtime 판정 True"), ValidFiniteWeaponData->UsesFiniteAmmoRuntime());

	// [v1.0.0] 음수 Heat 예약 필드 계약을 검증할 WeaponData입니다.
	UCFWeaponData* NegativeHeatWeaponData = CreateValidTransientWeaponData(TEXT("WD_P0_01_NegativeHeat"));
	NegativeHeatWeaponData->HeatPerShot = -1.0f;
	TestFalse(TEXT("음수 HeatPerShot 거부"), IsWeaponDataContractValid(NegativeHeatWeaponData));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWeaponDataRepresentativeAssetsTest,
	"CarFight.WeaponData.WD_P0_02.RepresentativeAssets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 대표 WeaponData 두 개를 Load-only로 읽어 현재 정적 계약과 핵심 발사 정의를 검증합니다.
bool FCFWeaponDataRepresentativeAssetsTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 표준 포탑포 WeaponData의 정식 ObjectPath입니다.
	const TCHAR* CannonObjectPath = TEXT("/Game/CarFight/Weapons/Data/WeaponDefs/DA_ProtoTurretCannon.DA_ProtoTurretCannon");
	// [v1.0.0] 표준 로켓런처 WeaponData의 정식 ObjectPath입니다.
	const TCHAR* RocketObjectPath = TEXT("/Game/CarFight/Weapons/Data/WeaponDefs/DA_RocketLauncher.DA_RocketLauncher");

	// [v1.0.0] Compile/Save 없이 읽기 전용으로 로드한 표준 포탑포 WeaponData입니다.
	UCFWeaponData* CannonWeaponData = LoadObject<UCFWeaponData>(nullptr, CannonObjectPath);
	// [v1.0.0] Compile/Save 없이 읽기 전용으로 로드한 표준 로켓런처 WeaponData입니다.
	UCFWeaponData* RocketWeaponData = LoadObject<UCFWeaponData>(nullptr, RocketObjectPath);
	if (!TestNotNull(TEXT("DA_ProtoTurretCannon Load-only 성공"), CannonWeaponData)
		|| !TestNotNull(TEXT("DA_RocketLauncher Load-only 성공"), RocketWeaponData))
	{
		return false;
	}

	TestTrue(TEXT("ProtoTurretCannon 정적 계약 PASS"), IsWeaponDataContractValid(CannonWeaponData));
	TestTrue(TEXT("RocketLauncher 정적 계약 PASS"), IsWeaponDataContractValid(RocketWeaponData));

	TestEqual(TEXT("Cannon WeaponId"), CannonWeaponData->WeaponId, FName(TEXT("Proto_TurretCannon")));
	TestEqual(TEXT("Rocket WeaponId"), RocketWeaponData->WeaponId, FName(TEXT("Proto_RocketLauncher")));
	TestEqual(TEXT("Cannon WeaponSize Large"), CannonWeaponData->WeaponSize, ECFVehicleWeaponSize::Large);
	TestEqual(TEXT("Rocket WeaponSize Large"), RocketWeaponData->WeaponSize, ECFVehicleWeaponSize::Large);
	TestTrue(TEXT("Cannon Turret 호환"), CannonWeaponData->SupportsMountType(ECFVehicleMountType::Turret));
	TestTrue(TEXT("Rocket Turret 호환"), RocketWeaponData->SupportsMountType(ECFVehicleMountType::Turret));
	TestEqual(TEXT("Cannon Projectile 모드"), CannonWeaponData->FireMode, ECFWeaponFireMode::Projectile);
	TestEqual(TEXT("Rocket Projectile 모드"), RocketWeaponData->FireMode, ECFWeaponFireMode::Projectile);
	TestTrue(TEXT("Cannon 120 RPM"), FMath::IsNearlyEqual(CannonWeaponData->FireRatePerMinute, 120.0f));
	TestTrue(TEXT("Rocket 20 RPM"), FMath::IsNearlyEqual(RocketWeaponData->FireRatePerMinute, 20.0f));
	TestNotNull(TEXT("Cannon DefaultProjectileData 존재"), CannonWeaponData->DefaultProjectileData.Get());
	TestNotNull(TEXT("Rocket DefaultProjectileData 존재"), RocketWeaponData->DefaultProjectileData.Get());

	// [v1.0.0] 저장 원본을 안전하게 보정한 Cannon 발사 패턴입니다.
	const FCFLauncherFirePatternConfig CannonLauncherConfig = CannonWeaponData->GetEffectiveLauncherFirePatternConfig();
	// [v1.0.0] 저장 원본을 안전하게 보정한 Rocket 발사 패턴입니다.
	const FCFLauncherFirePatternConfig RocketLauncherConfig = RocketWeaponData->GetEffectiveLauncherFirePatternConfig();
	TestEqual(TEXT("Cannon SingleCycle"), CannonLauncherConfig.FirePattern, ECFLauncherFirePattern::SingleCycle);
	TestEqual(TEXT("Cannon 입력당 1발"), CannonLauncherConfig.GetEffectiveProjectileCount(), 1);
	TestEqual(TEXT("Rocket Salvo"), RocketLauncherConfig.FirePattern, ECFLauncherFirePattern::Salvo);
	TestEqual(TEXT("Rocket Salvo 4발"), RocketLauncherConfig.GetEffectiveProjectileCount(), 4);
	TestEqual(TEXT("Rocket Salvo 동시 4발"), RocketLauncherConfig.GetEffectiveMaximumSimultaneousLaunchCount(), 4);

	AddInfo(FString::Printf(
		TEXT("WD-P0-02 Cannon Ammo | Finite=%s | InfiniteDebug=%s | Ammo=%s | Magazine=%d | InitialLoaded=%d | UnitsPerShot=%d"),
		CannonWeaponData->UsesFiniteAmmoRuntime() ? TEXT("True") : TEXT("False"),
		CannonWeaponData->bUseInfiniteAmmoForDebug ? TEXT("True") : TEXT("False"),
		*GetPathNameSafe(CannonWeaponData->DefaultAmmoData),
		CannonWeaponData->MagazineSize,
		CannonWeaponData->InitialLoadedAmmoCount,
		CannonWeaponData->AmmoUnitsPerShot));
	AddInfo(FString::Printf(
		TEXT("WD-P0-02 Rocket Ammo | Finite=%s | InfiniteDebug=%s | Ammo=%s | Magazine=%d | InitialLoaded=%d | UnitsPerShot=%d"),
		RocketWeaponData->UsesFiniteAmmoRuntime() ? TEXT("True") : TEXT("False"),
		RocketWeaponData->bUseInfiniteAmmoForDebug ? TEXT("True") : TEXT("False"),
		*GetPathNameSafe(RocketWeaponData->DefaultAmmoData),
		RocketWeaponData->MagazineSize,
		RocketWeaponData->InitialLoadedAmmoCount,
		RocketWeaponData->AmmoUnitsPerShot));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
