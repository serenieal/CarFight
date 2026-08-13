// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-13
// Description: CF-FQ-031 AMMO-P0-08 저장 에셋 기반 통합 Automation
// Scope: P0-08 저장 VehicleFittingData를 실제 Fitting Snapshot → Weapon/Ammo Runtime → HUD Provider/Presenter의 Loaded/Capacity·Reserve 표시까지 검증합니다.
// Changelog:
// - v1.1.0: Heavy 5/10 + Reserve10, Ripple 3/4 + Reserve4와 예약 중 Loaded 표기 유지 계약을 검증하며 Immediate/CurrentUsable 내부값도 계속 확인.
// - v1.0.1: Automation TestNotNull에 TObjectPtr를 직접 넘기지 않고 .Get() 원시 UObject 포인터를 전달하도록 컴파일 경계를 교정.
// - v1.0.0: Heavy 5|15, Ripple 3|7 저장 체인, Heavy 소비 후 4|14, Ripple 4발 요청의 실제 3발 부분 예약을 검증.
// Migration:
// - 테스트는 `/Game/CarFight/Tests/AmmoIntegration` 저장 에셋을 읽기 전용으로 사용합니다.
// - Production Widget Asset을 수정하지 않으며 실제 사용자 시각 확인은 후속 PIE에서 수행합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFAmmoTypes.h"
#include "CFEquipmentPresetData.h"
#include "CFFittingTypes.h"
#include "CFVehicleAmmoComp.h"
#include "CFVehicleData.h"
#include "CFVehicleFittingData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UI/CFHUDDataProvider.h"
#include "UI/CFHUDPresenter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFAmmoIntegrationAssetTest,
	"CarFight.Ammo.AMMO_P0_08.AssetChain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 실제 저장 FittingData 두 종류를 Runtime과 HUD 표시 계약까지 연결해 검증합니다.
bool FCFAmmoIntegrationAssetTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 저장 에셋 로드와 두 테스트 Pawn이 공유할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("AMMO-P0-08 Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 한 저장 FittingData를 실제 Ammo Runtime과 HUD Presenter까지 검증하는 공통 실행 함수입니다.
	auto ValidateSavedAmmoCase = [this, TestWorld](
				const TCHAR* FittingObjectPath,
		const int32 ExpectedLoadedAmmoCount,
		const int32 ExpectedMagazineCapacity,
		const int32 ExpectedReserveAmmoCount,
		const int32 ExpectedCurrentAmmoCount,
		const float ExpectedAmmoMassKg,
		const FString& ExpectedAmmoText,
		const bool bExpectRipplePattern) -> bool
	{
		// [v1.0.0] Python Apply가 저장한 실제 VehicleFittingData입니다.
		UCFVehicleFittingData* FittingData = LoadObject<UCFVehicleFittingData>(nullptr, FittingObjectPath);
		if (!TestNotNull(TEXT("P0-08 저장 VehicleFittingData"), FittingData))
		{
			return false;
		}

		// [v1.0.0] 실제 저장 피팅에서 생성한 결정론적 Fitting Snapshot입니다.
		const FCFVehicleFittingSnapshot FittingSnapshot = FittingData->BuildFittingSnapshot();
		TestTrue(TEXT("P0-08 저장 Fitting Snapshot 유효"), FittingSnapshot.IsValid());
		TestEqual(TEXT("P0-08 저장 Fitting ResolvedMount 1개"), FittingSnapshot.ResolvedMounts.Num(), 1);
		TestEqual(TEXT("P0-08 저장 Fitting AmmoLoad 1개"), FittingSnapshot.InitialSortieAmmoLoads.Num(), 1);
		TestEqual(TEXT("P0-08 저장 Fitting AmmoMassKg"), FittingSnapshot.AmmoMassKg, ExpectedAmmoMassKg);
		if (!FittingSnapshot.IsValid() || FittingSnapshot.ResolvedMounts.Num() != 1)
		{
			return false;
		}

		// [v1.0.0] 실제 저장 Snapshot이 해석한 단일 활성 장착 결과입니다.
		const FCFResolvedFittingMount& ResolvedMount = FittingSnapshot.ResolvedMounts[0];
				if (!TestNotNull(TEXT("P0-08 Resolved EquipmentPresetData"), ResolvedMount.EquipmentPresetData.Get())
			|| !TestNotNull(TEXT("P0-08 Resolved finite WeaponData"), ResolvedMount.WeaponData.Get()))
		{
			return false;
		}
		TestTrue(TEXT("P0-08 저장 WeaponData finite Ammo 활성"), ResolvedMount.WeaponData->UsesFiniteAmmoRuntime());
		TestFalse(TEXT("P0-08 저장 WeaponData InfiniteCompatibility 비활성"), ResolvedMount.WeaponData->bUseInfiniteAmmoForDebug);
		TestEqual(
			TEXT("P0-08 저장 Weapon Pattern"),
			ResolvedMount.WeaponData->GetEffectiveLauncherFirePatternConfig().FirePattern,
			bExpectRipplePattern ? ECFLauncherFirePattern::Ripple : ECFLauncherFirePattern::SingleCycle);

		// [v1.0.0] 실제 Gameplay Component와 HUD Provider가 Rebind할 테스트 차량 Pawn입니다.
		ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>();
		if (!TestNotNull(TEXT("P0-08 Vehicle Pawn"), VehiclePawn))
		{
			return false;
		}

		// [v1.0.0] 저장 Snapshot EquipmentPresetData를 실제 활성 Weapon Runtime으로 적용할 컴포넌트입니다.
		UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();

		// [v1.0.0] 저장 Snapshot 출격 탄약을 장전·예비 상태로 소유할 컴포넌트입니다.
		UCFVehicleAmmoComp* VehicleAmmoComp = VehiclePawn->GetVehicleAmmoComp();
				if (!TestNotNull(TEXT("P0-08 VehicleWeaponComp"), VehicleWeaponComp)
			|| !TestNotNull(TEXT("P0-08 VehicleAmmoComp"), VehicleAmmoComp)
			|| !TestNotNull(TEXT("P0-08 Snapshot VehicleData"), FittingSnapshot.VehicleData.Get()))
		{
			VehiclePawn->Destroy();
			return false;
		}

		TestTrue(
			TEXT("P0-08 저장 Fitting EquipmentPreset Weapon Runtime 적용"),
			VehicleWeaponComp->InitializeWeaponRuntimeFromFitting(
				VehiclePawn,
				FittingSnapshot.VehicleData,
				ResolvedMount.MountProfileId,
				ResolvedMount.EquipmentPresetData));

		// [v1.0.0] 저장 Snapshot의 finite WeaponInstance를 VehicleAmmoComp 초기화 입력으로 변환한 배열입니다.
		TArray<FCFWeaponAmmoInitialization> WeaponAmmoInitializations;
		for (const FCFResolvedFittingMount& SnapshotMount : FittingSnapshot.ResolvedMounts)
		{
			if (!IsValid(SnapshotMount.WeaponData) || !SnapshotMount.WeaponData->UsesFiniteAmmoRuntime())
			{
				continue;
			}

			// [v1.0.0] MountProfileId별 독립 Loaded 상태를 만들 실제 저장 WeaponData 초기화 입력입니다.
			FCFWeaponAmmoInitialization WeaponAmmoInitialization;
			WeaponAmmoInitialization.WeaponInstanceId = SnapshotMount.MountProfileId;
			WeaponAmmoInitialization.WeaponData = SnapshotMount.WeaponData;
			WeaponAmmoInitialization.InitialLoadedAmmoCountOverride = INDEX_NONE;
			WeaponAmmoInitializations.Add(WeaponAmmoInitialization);
		}
		TestEqual(TEXT("P0-08 finite WeaponInstance 1개"), WeaponAmmoInitializations.Num(), 1);
		TestTrue(
			TEXT("P0-08 저장 Fitting 출격 탄약 Runtime 초기화"),
			VehicleAmmoComp->InitializeAmmoRuntime(
				VehiclePawn,
				FittingSnapshot.InitialSortieAmmoLoads,
				WeaponAmmoInitializations));

		// [v1.0.0] 실제 저장 Fitting 수량에서 만들어진 활성 무기 Ammo Snapshot입니다.
		FCFAmmoRuntimeSnapshot AmmoSnapshot;
		TestTrue(
			TEXT("P0-08 활성 Ammo Snapshot 조회"),
			VehicleAmmoComp->TryGetActiveWeaponAmmoSnapshot(VehicleWeaponComp, AmmoSnapshot));
				TestEqual(TEXT("P0-08 초기 Loaded"), AmmoSnapshot.LoadedAmmoCount, ExpectedLoadedAmmoCount);
		TestEqual(TEXT("P0-08 초기 MagazineCapacity"), AmmoSnapshot.MagazineCapacity, ExpectedMagazineCapacity);
		TestEqual(TEXT("P0-08 초기 Reserve"), AmmoSnapshot.ReserveAmmoCount, ExpectedReserveAmmoCount);
		TestEqual(TEXT("P0-08 초기 CurrentUsable 내부값"), AmmoSnapshot.CurrentUsableAmmoCount, ExpectedCurrentAmmoCount);
		TestEqual(TEXT("P0-08 초기 CurrentOnboard"), AmmoSnapshot.CurrentOnboardAmmoCount, ExpectedCurrentAmmoCount);

		// [v1.0.0] Production과 같은 Runtime→ViewData 변환을 수행할 HUD Provider입니다.
		UCFHUDDataProvider* HUDDataProvider = NewObject<UCFHUDDataProvider>(GetTransientPackage());
		if (!TestNotNull(TEXT("P0-08 HUD Data Provider"), HUDDataProvider))
		{
			VehiclePawn->Destroy();
			return false;
		}
		HUDDataProvider->RebindCurrentPawn(VehiclePawn);

		// [v1.0.0] 실제 Ammo Runtime을 Provider가 변환한 Weapon ViewData입니다.
		const FCFWeaponHUDData InitialWeaponViewData = HUDDataProvider->GetCurrentViewData().Weapon;
				TestEqual(TEXT("P0-08 HUD AmmoAvailability Known"), InitialWeaponViewData.AmmoAvailability, ECFUIViewAvailability::Known);
		TestEqual(TEXT("P0-08 HUD Loaded"), InitialWeaponViewData.LoadedAmmoCount, ExpectedLoadedAmmoCount);
		TestEqual(TEXT("P0-08 HUD MagazineCapacity"), InitialWeaponViewData.MagazineCapacity, ExpectedMagazineCapacity);
		TestEqual(TEXT("P0-08 HUD Reserve"), InitialWeaponViewData.ReserveAmmoCount, ExpectedReserveAmmoCount);
		TestEqual(TEXT("P0-08 HUD ImmediateUsable 내부값"), InitialWeaponViewData.ImmediateUsableAmmoCount, ExpectedLoadedAmmoCount);
		TestEqual(TEXT("P0-08 HUD CurrentUsable 내부값"), InitialWeaponViewData.CurrentUsableAmmoCount, ExpectedCurrentAmmoCount);

		// [v1.1.0] Production WeaponPanel Text_WeaponAmmo에 전달될 Loaded/Capacity Primary 문자열입니다.
		FText AmmoText;
		TestTrue(TEXT("P0-08 WeaponPanel Ammo Primary 문구 해석"), UCFHUDPresenter::ResolveAmmoPresentation(InitialWeaponViewData, AmmoText));
		TestEqual(TEXT("P0-08 WeaponPanel 초기 Ammo Primary 문구"), AmmoText.ToString(), ExpectedAmmoText);

		// [v1.1.0] Production WeaponPanel 우상단 Text_WeaponReserveAmmo에 전달될 label-less Reserve 숫자입니다.
		FText ReserveAmmoText;
		TestTrue(TEXT("P0-08 WeaponPanel Reserve 숫자 해석"), UCFHUDPresenter::ResolveReserveAmmoPresentation(InitialWeaponViewData, ReserveAmmoText));
		TestEqual(TEXT("P0-08 WeaponPanel 초기 Reserve 숫자"), ReserveAmmoText.ToString(), FString::FromInt(ExpectedReserveAmmoCount));

				if (bExpectRipplePattern)
		{
			// [v1.1.0] 저장 Ripple의 4발 요청에서 실제 장전 3발만 부분 Sequence로 예약됐는지 반환받을 수량입니다.
			int32 ReservedRippleShotCount = 0;
			TestEqual(
				TEXT("P0-08 Ripple 4발 요청 부분 예약 승인"),
				VehicleAmmoComp->ReserveLauncherSequenceAmmo(
					VehicleWeaponComp->GetActiveMountProfileId(),
					4,
					ResolvedMount.WeaponData->bAllowPartialSequence,
					ReservedRippleShotCount),
				ECFAmmoTransactionResult::Accepted);
			TestEqual(TEXT("P0-08 Ripple 실제 예약 3발"), ReservedRippleShotCount, 3);

			// [v1.1.0] 예약은 발사 가능량만 잠그고 실제 Loaded 소유량은 Commit 전까지 유지합니다.
			const FCFWeaponHUDData ReservedRippleViewData = HUDDataProvider->GetCurrentViewData().Weapon;
			TestEqual(TEXT("P0-08 Ripple 예약 중 Loaded 3 유지"), ReservedRippleViewData.LoadedAmmoCount, 3);
			TestEqual(TEXT("P0-08 Ripple 예약 중 Capacity 4 유지"), ReservedRippleViewData.MagazineCapacity, 4);
			TestEqual(TEXT("P0-08 Ripple 예약 중 Reserve 4 유지"), ReservedRippleViewData.ReserveAmmoCount, 4);
			TestEqual(TEXT("P0-08 Ripple 예약 중 ImmediateUsable 0 내부값"), ReservedRippleViewData.ImmediateUsableAmmoCount, 0);
			TestEqual(TEXT("P0-08 Ripple 예약 중 CurrentUsable 4 내부값"), ReservedRippleViewData.CurrentUsableAmmoCount, 4);

			// [v1.1.0] 예약만으로 Primary Ammo가 0으로 떨어지지 않고 실제 Loaded/Capacity를 유지합니다.
			FText ReservedRippleAmmoText;
			TestTrue(TEXT("P0-08 Ripple 예약 Ammo Primary 문구 해석"), UCFHUDPresenter::ResolveAmmoPresentation(ReservedRippleViewData, ReservedRippleAmmoText));
			TestEqual(TEXT("P0-08 Ripple 예약 Ammo Primary 3 / 4"), ReservedRippleAmmoText.ToString(), FString(TEXT("3 / 4")));
			VehicleAmmoComp->ReleaseLauncherSequenceReservation(VehicleWeaponComp->GetActiveMountProfileId());
		}
		else
		{
			TestEqual(
				TEXT("P0-08 Heavy 단발 예약"),
				VehicleAmmoComp->ReserveSingleFireAmmo(VehicleWeaponComp->GetActiveMountProfileId()),
				ECFAmmoTransactionResult::Accepted);
			TestEqual(
				TEXT("P0-08 Heavy 단발 소비 Commit"),
				VehicleAmmoComp->CommitSingleFireAmmo(VehicleWeaponComp->GetActiveMountProfileId()),
				ECFAmmoTransactionResult::Accepted);

			// [v1.1.0] Heavy 한 발 소비 이벤트가 Provider에 즉시 반영된 Weapon ViewData입니다.
			const FCFWeaponHUDData FiredHeavyViewData = HUDDataProvider->GetCurrentViewData().Weapon;
			TestEqual(TEXT("P0-08 Heavy 한 발 후 Loaded 4"), FiredHeavyViewData.LoadedAmmoCount, 4);
			TestEqual(TEXT("P0-08 Heavy Capacity 10 유지"), FiredHeavyViewData.MagazineCapacity, 10);
			TestEqual(TEXT("P0-08 Heavy Reserve 10 유지"), FiredHeavyViewData.ReserveAmmoCount, 10);
			TestEqual(TEXT("P0-08 Heavy 한 발 후 ImmediateUsable 4 내부값"), FiredHeavyViewData.ImmediateUsableAmmoCount, 4);
			TestEqual(TEXT("P0-08 Heavy 한 발 후 CurrentUsable 14 내부값"), FiredHeavyViewData.CurrentUsableAmmoCount, 14);

			// [v1.1.0] Heavy 한 발 소비 뒤 Primary는 Loaded만 감소하고 Capacity는 유지됩니다.
			FText FiredHeavyAmmoText;
			TestTrue(TEXT("P0-08 Heavy 소비 Ammo Primary 문구 해석"), UCFHUDPresenter::ResolveAmmoPresentation(FiredHeavyViewData, FiredHeavyAmmoText));
			TestEqual(TEXT("P0-08 Heavy 한 발 후 Ammo Primary 4 / 10"), FiredHeavyAmmoText.ToString(), FString(TEXT("4 / 10")));
		}

		HUDDataProvider->ShutdownProvider();
		VehiclePawn->Destroy();
		return true;
	};

	TestTrue(
		TEXT("AMMO-P0-08 Heavy 저장 에셋 체인"),
				ValidateSavedAmmoCase(
			TEXT("/Game/CarFight/Tests/AmmoIntegration/DA_Fit_HeavyFinite.DA_Fit_HeavyFinite"),
			5,
			10,
			10,
			15,
			30.0f,
			TEXT("5 / 10"),
			false));

	TestTrue(
		TEXT("AMMO-P0-08 Ripple 저장 에셋 체인"),
		ValidateSavedAmmoCase(
			TEXT("/Game/CarFight/Tests/AmmoIntegration/DA_Fit_RippleFinite.DA_Fit_RippleFinite"),
			3,
			4,
			4,
			7,
			35.0f,
			TEXT("3 / 4"),
			true));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
