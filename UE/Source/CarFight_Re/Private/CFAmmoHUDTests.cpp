// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-13
// Description: CF-FQ-031 AMMO-P0-06 Ammo HUD and Provider Rebind Automation
// Scope: 실제 finite Ammo Runtime을 HUD Provider·Presenter가 소비하는 Loaded/Capacity·Reserve 수치, Reload·NoAmmo 표현과 Old Pawn 이벤트 격리를 검증합니다.
// Changelog:
// - v1.1.0: MagazineCapacity 전달, `Loaded / Capacity` Primary와 label-less Reserve Presentation 회귀를 추가하고 Immediate/CurrentUsable 내부값 보존을 검증.
// - v1.0.0: AMMO-P0-06 Runtime→Provider→ViewData→Presenter와 Ammo event Pawn Rebind 통합 테스트 최초 추가.
// Migration:
// - Automation World와 Transient DataAsset만 사용하며 Production Widget Asset과 사용자 DataAsset을 수정하지 않습니다.
// - 실제 출격 탄약 수량이 없는 Production 차량에는 이 테스트 수치를 주입하지 않으며 Provider는 Unavailable을 유지합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFAmmoData.h"
#include "CFEquipmentPresetData.h"
#include "CFVehicleAmmoComp.h"
#include "CFVehicleData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UI/CFHUDDataProvider.h"
#include "UI/CFHUDPresenter.h"

namespace
{
	/**
	 * 한 테스트 차량의 finite Ammo Runtime을 구성하는 Transient 객체 묶음입니다.
	 */
	struct FCFAmmoHUDTestSetup
	{
		// [v1.0.0] 실제 HUD Provider가 Rebind할 차량 Pawn입니다.
		ACFVehiclePawn* VehiclePawn = nullptr;

		// [v1.0.0] 현재 활성 무기와 MountProfileId를 제공하는 Weapon 컴포넌트입니다.
		UCFVehicleWeaponComp* VehicleWeaponComp = nullptr;

		// [v1.0.0] 장전·예비·Reload Runtime과 변경 이벤트를 소유하는 Ammo 컴포넌트입니다.
		UCFVehicleAmmoComp* VehicleAmmoComp = nullptr;

		// [v1.0.0] 이 테스트 차량의 유한탄 정책을 제공하는 Transient WeaponData입니다.
		UCFWeaponData* WeaponData = nullptr;

		// [v1.0.0] 이 테스트 차량의 실제 탄종 ID를 제공하는 Transient AmmoData입니다.
		UCFAmmoData* AmmoData = nullptr;
	};

	// [v1.0.0] 지정 World에 finite Ammo Runtime이 준비된 테스트 차량 하나를 생성합니다.
	bool CreateAmmoHUDTestSetup(
		UWorld* TestWorld,
		const FName AmmoId,
		const int32 MagazineCapacity,
		const int32 InitialLoadedAmmoCount,
		const int32 InitialSortieAmmoCount,
		const float ReloadTimeSeconds,
		FCFAmmoHUDTestSetup& OutSetup)
	{
		OutSetup = FCFAmmoHUDTestSetup();
		if (!TestWorld || AmmoId.IsNone())
		{
			return false;
		}

		// [v1.0.0] HUD Provider의 실제 Gameplay Source가 될 차량 Pawn입니다.
		OutSetup.VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>();
		if (!OutSetup.VehiclePawn)
		{
			return false;
		}

		// [v1.0.0] Pawn 기본 서브오브젝트에서 얻은 실제 Weapon Runtime 컴포넌트입니다.
		OutSetup.VehicleWeaponComp = OutSetup.VehiclePawn->GetVehicleWeaponComp();

		// [v1.0.0] Pawn 기본 서브오브젝트에서 얻은 실제 Ammo Runtime 컴포넌트입니다.
		OutSetup.VehicleAmmoComp = OutSetup.VehiclePawn->GetVehicleAmmoComp();
		if (!OutSetup.VehicleWeaponComp || !OutSetup.VehicleAmmoComp)
		{
			return false;
		}

		// [v1.0.0] 테스트 차량의 실제 finite 탄종으로 사용할 Transient AmmoData입니다.
		OutSetup.AmmoData = NewObject<UCFAmmoData>(GetTransientPackage());
		OutSetup.AmmoData->AmmoId = AmmoId;

		// [v1.0.0] Ammo HUD와 Reload 표시를 검증할 Transient WeaponData입니다.
		OutSetup.WeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
		OutSetup.WeaponData->WeaponId = FName(*(AmmoId.ToString() + TEXT("_Weapon")));
		OutSetup.WeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
		OutSetup.WeaponData->CompatibleMountTypes.Reset();
		OutSetup.WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
		OutSetup.WeaponData->DefaultAmmoData = OutSetup.AmmoData;
		OutSetup.WeaponData->MagazineSize = MagazineCapacity;
		OutSetup.WeaponData->InitialLoadedAmmoCount = InitialLoadedAmmoCount;
		OutSetup.WeaponData->AmmoUnitsPerShot = 1;
		OutSetup.WeaponData->ReloadTimeSeconds = ReloadTimeSeconds;
		OutSetup.WeaponData->ReloadMode = ECFWeaponReloadMode::FullMagazine;
		OutSetup.WeaponData->bUseInfiniteAmmoForDebug = false;
		OutSetup.WeaponData->bAutoReloadWhenEmpty = false;
		OutSetup.WeaponData->bAllowPartialReload = true;
		OutSetup.WeaponData->LauncherFirePatternConfig.FirePattern = ECFLauncherFirePattern::SingleCycle;

		// [v1.0.0] 현재 MountProfile이 Transient WeaponData를 단일 장비 원본으로 사용하게 할 EquipmentPresetData입니다.
		UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>(GetTransientPackage());
		EquipmentPresetData->EquipmentId = FName(*(AmmoId.ToString() + TEXT("_Preset")));
		EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
		EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Large;
		EquipmentPresetData->DefaultWeaponData = OutSetup.WeaponData;

		// [v1.0.0] WeaponComp가 RoofTurret 활성 프로파일과 하드포인트를 해석할 Transient VehicleData입니다.
		UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(GetTransientPackage());

		// [v1.0.0] 활성 MountProfile이 참조할 최소 Top_01 하드포인트 슬롯입니다.
		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = TEXT("Top_01");
		HardpointSlot.LocationCategory = TEXT("Top");
		HardpointSlot.LocalLocation = FVector::ZeroVector;
		VehicleData->HardpointSlots.Add(HardpointSlot);

		// [v1.0.0] VehicleWeaponComp 기본 ActiveMountProfileId와 일치하는 테스트 MountProfile입니다.
		FCFVehicleMountProfile MountProfile;
		MountProfile.MountProfileId = TEXT("RoofTurret_MediumOrLarge");
		MountProfile.LocationSlotRef = HardpointSlot.LocationSlotId;
		MountProfile.MountType = ECFVehicleMountType::Turret;
		MountProfile.SizeLimit = ECFVehicleWeaponSize::Large;
		MountProfile.DefaultEquipmentPresetData = EquipmentPresetData;
		VehicleData->MountProfiles.Add(MountProfile);

		if (!OutSetup.VehicleWeaponComp->InitializeWeaponRuntime(OutSetup.VehiclePawn, VehicleData))
		{
			return false;
		}

		return OutSetup.VehicleAmmoComp->InitializeActiveWeaponAmmoRuntime(
			OutSetup.VehiclePawn,
			OutSetup.VehicleWeaponComp,
			InitialSortieAmmoCount);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFAmmoHUDRuntimeTest,
	"CarFight.Ammo.AMMO_P0_06.HUD",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 실제 finite Ammo Runtime이 Provider·Presenter와 Pawn Rebind 경계에서 정확히 표시·격리되는지 검증합니다.
bool FCFAmmoHUDRuntimeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 두 차량과 Provider Timer를 같은 World에서 검증할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("AMMO-P0-06 Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 첫 Pawn의 2발 장전 + 5발 예비 Runtime 설정입니다.
	FCFAmmoHUDTestSetup FirstSetup;
	if (!TestTrue(
		TEXT("첫 finite Ammo HUD 차량 초기화"),
		CreateAmmoHUDTestSetup(TestWorld, TEXT("AmmoP006_First"), 5, 2, 7, 2.0f, FirstSetup)))
	{
		return false;
	}

	// [v1.0.0] 두 번째 Pawn의 1발 장전 + 2발 예비 Runtime 설정입니다.
	FCFAmmoHUDTestSetup SecondSetup;
	if (!TestTrue(
		TEXT("두 번째 finite Ammo HUD 차량 초기화"),
		CreateAmmoHUDTestSetup(TestWorld, TEXT("AmmoP006_Second"), 4, 1, 3, 3.0f, SecondSetup)))
	{
		return false;
	}

	// [v1.0.0] Production과 같은 Gameplay Runtime→ViewData 변환을 수행할 HUD Provider입니다.
	UCFHUDDataProvider* HUDDataProvider = NewObject<UCFHUDDataProvider>(GetTransientPackage());
	if (!TestNotNull(TEXT("AMMO-P0-06 HUD Provider"), HUDDataProvider))
	{
		return false;
	}

	HUDDataProvider->RebindCurrentPawn(FirstSetup.VehiclePawn);

	// [v1.0.0] 첫 Pawn Rebind 직후 Provider가 계산한 실제 Weapon Ammo ViewData입니다.
	FCFWeaponHUDData FirstWeaponViewData = HUDDataProvider->GetCurrentViewData().Weapon;
		TestEqual(TEXT("첫 Pawn AmmoAvailability Known"), FirstWeaponViewData.AmmoAvailability, ECFUIViewAvailability::Known);
	TestEqual(TEXT("첫 Pawn Loaded 2"), FirstWeaponViewData.LoadedAmmoCount, 2);
	TestEqual(TEXT("첫 Pawn MagazineCapacity 5"), FirstWeaponViewData.MagazineCapacity, 5);
	TestEqual(TEXT("첫 Pawn ImmediateUsable 2 내부값 유지"), FirstWeaponViewData.ImmediateUsableAmmoCount, 2);
	TestEqual(TEXT("첫 Pawn Reserve 5"), FirstWeaponViewData.ReserveAmmoCount, 5);
	TestEqual(TEXT("첫 Pawn CurrentUsable 7 내부값 유지"), FirstWeaponViewData.CurrentUsableAmmoCount, 7);
	TestEqual(TEXT("첫 Pawn CurrentOnboard 7"), FirstWeaponViewData.CurrentOnboardAmmoCount, 7);

	// [v1.1.0] Presenter가 실제 Loaded/Capacity ViewData를 Production Primary Text로 변환한 값입니다.
	FText AmmoText;
	TestTrue(TEXT("실제 Ammo Primary 문구 표시"), UCFHUDPresenter::ResolveAmmoPresentation(FirstWeaponViewData, AmmoText));
	TestEqual(TEXT("Ammo Primary 문구 2 / 5"), AmmoText.ToString(), FString(TEXT("2 / 5")));

	// [v1.1.0] Presenter가 실제 ReserveAmmoCount를 우상단 label-less 숫자로 변환한 값입니다.
	FText ReserveAmmoText;
	TestTrue(TEXT("실제 Reserve 숫자 표시"), UCFHUDPresenter::ResolveReserveAmmoPresentation(FirstWeaponViewData, ReserveAmmoText));
	TestEqual(TEXT("Reserve 숫자 5"), ReserveAmmoText.ToString(), FString(TEXT("5")));

	TestEqual(
		TEXT("첫 Pawn 수동 Reload 시작"),
		FirstSetup.VehicleAmmoComp->RequestActiveWeaponReload(FirstSetup.VehicleWeaponComp),
		ECFAmmoTransactionResult::Accepted);

	// [v1.0.0] Ammo Runtime 변경 이벤트가 Provider를 즉시 갱신한 Reload 시작 ViewData입니다.
	FCFWeaponHUDData ReloadStartedViewData = HUDDataProvider->GetCurrentViewData().Weapon;
	TestEqual(TEXT("ReloadState Reloading"), ReloadStartedViewData.ReloadState, ECFWeaponReloadState::Reloading);
	TestEqual(TEXT("Reload 전체 시간 2초"), ReloadStartedViewData.ReloadDurationSeconds, 2.0f);
	TestEqual(TEXT("Reload 남은 시간 2초"), ReloadStartedViewData.RemainingReloadTimeSeconds, 2.0f);

	// [v1.0.0] Reload 시작 상태를 Production 상태 행 문구와 진행률로 변환한 값입니다.
	FText ReloadStatusText;

	// [v1.0.0] Reload 시작 시 Production 상태 Progress에 적용할 값입니다.
	float ReloadStatusProgress = 0.0f;
	TestTrue(
		TEXT("Reload 상태 문구 표시"),
		UCFHUDPresenter::ResolveWeaponStatusPresentation(
			ReloadStartedViewData,
			false,
			ReloadStatusText,
			ReloadStatusProgress));
	TestEqual(TEXT("Reload 문구"), ReloadStatusText.ToString(), FString(TEXT("RELOAD 2.0 / 2.0 s")));
	TestEqual(TEXT("Reload 시작 진행률 0"), ReloadStatusProgress, 0.0f);

	FirstSetup.VehicleAmmoComp->TickComponent(1.0f, LEVELTICK_All, nullptr);

	// [v1.0.0] Reload 1초 경과 이벤트를 Provider가 반영한 Weapon ViewData입니다.
	FCFWeaponHUDData ReloadMidViewData = HUDDataProvider->GetCurrentViewData().Weapon;
	TestEqual(TEXT("Reload 1초 후 남은 시간"), ReloadMidViewData.RemainingReloadTimeSeconds, 1.0f);

	// [v1.0.0] Reload 절반 진행 상태 문구입니다.
	FText ReloadMidStatusText;

	// [v1.0.0] Reload 절반 진행 상태 Progress입니다.
	float ReloadMidStatusProgress = 0.0f;
	TestTrue(
		TEXT("Reload 중간 상태 문구 표시"),
		UCFHUDPresenter::ResolveWeaponStatusPresentation(
			ReloadMidViewData,
			false,
			ReloadMidStatusText,
			ReloadMidStatusProgress));
	TestEqual(TEXT("Reload 중간 문구"), ReloadMidStatusText.ToString(), FString(TEXT("RELOAD 1.0 / 2.0 s")));
	TestEqual(TEXT("Reload 중간 진행률 0.5"), ReloadMidStatusProgress, 0.5f);
	TestEqual(
		TEXT("첫 Pawn Reload 취소"),
		FirstSetup.VehicleAmmoComp->CancelReload(FirstSetup.VehicleWeaponComp->GetActiveMountProfileId()),
		ECFAmmoTransactionResult::Accepted);

	HUDDataProvider->RebindCurrentPawn(SecondSetup.VehiclePawn);

	// [v1.0.0] 두 번째 Pawn으로 교체한 직후 Provider의 현재 Weapon Ammo ViewData입니다.
	FCFWeaponHUDData SecondWeaponViewData = HUDDataProvider->GetCurrentViewData().Weapon;
	TestEqual(TEXT("두 번째 Pawn CurrentUsable 3"), SecondWeaponViewData.CurrentUsableAmmoCount, 3);
	TestEqual(TEXT("두 번째 Pawn ImmediateUsable 1"), SecondWeaponViewData.ImmediateUsableAmmoCount, 1);

	TestEqual(
		TEXT("Old Pawn 단발 예약"),
		FirstSetup.VehicleAmmoComp->ReserveSingleFireAmmo(FirstSetup.VehicleWeaponComp->GetActiveMountProfileId()),
		ECFAmmoTransactionResult::Accepted);
	TestEqual(
		TEXT("Old Pawn 단발 소비"),
		FirstSetup.VehicleAmmoComp->CommitSingleFireAmmo(FirstSetup.VehicleWeaponComp->GetActiveMountProfileId()),
		ECFAmmoTransactionResult::Accepted);

	// [v1.0.0] Old Pawn Ammo 이벤트 이후에도 Provider가 두 번째 Pawn 수치를 유지하는지 확인할 ViewData입니다.
	FCFWeaponHUDData AfterOldPawnEventViewData = HUDDataProvider->GetCurrentViewData().Weapon;
	TestEqual(TEXT("Old Pawn 이벤트가 새 HUD CurrentUsable을 덮어쓰지 않음"), AfterOldPawnEventViewData.CurrentUsableAmmoCount, 3);
	TestEqual(TEXT("Old Pawn 이벤트가 새 HUD ImmediateUsable을 덮어쓰지 않음"), AfterOldPawnEventViewData.ImmediateUsableAmmoCount, 1);

	TestEqual(
		TEXT("현재 Pawn 단발 예약"),
		SecondSetup.VehicleAmmoComp->ReserveSingleFireAmmo(SecondSetup.VehicleWeaponComp->GetActiveMountProfileId()),
		ECFAmmoTransactionResult::Accepted);
	TestEqual(
		TEXT("현재 Pawn 단발 소비"),
		SecondSetup.VehicleAmmoComp->CommitSingleFireAmmo(SecondSetup.VehicleWeaponComp->GetActiveMountProfileId()),
		ECFAmmoTransactionResult::Accepted);

	// [v1.0.0] 현재 Bound Pawn Ammo 이벤트가 Provider에 즉시 반영된 ViewData입니다.
	FCFWeaponHUDData AfterCurrentPawnEventViewData = HUDDataProvider->GetCurrentViewData().Weapon;
	TestEqual(TEXT("현재 Pawn 소비 후 ImmediateUsable 0"), AfterCurrentPawnEventViewData.ImmediateUsableAmmoCount, 0);
	TestEqual(TEXT("현재 Pawn 소비 후 CurrentUsable 2"), AfterCurrentPawnEventViewData.CurrentUsableAmmoCount, 2);
	TestEqual(TEXT("현재 Pawn 소비 후 CurrentOnboard 2"), AfterCurrentPawnEventViewData.CurrentOnboardAmmoCount, 2);

	SecondSetup.WeaponData->InitialLoadedAmmoCount = 0;
	TestTrue(
		TEXT("KnownZero 검증용 0발 출격 재초기화"),
		SecondSetup.VehicleAmmoComp->InitializeActiveWeaponAmmoRuntime(
			SecondSetup.VehiclePawn,
			SecondSetup.VehicleWeaponComp,
			0));

		// [v1.1.0] 실제 finite Ammo Runtime이 0발인 상태를 Provider가 KnownZero로 보존한 ViewData입니다.
	FCFWeaponHUDData ZeroAmmoViewData = HUDDataProvider->GetCurrentViewData().Weapon;
	TestEqual(TEXT("0발 finite Ammo는 KnownZero"), ZeroAmmoViewData.AmmoAvailability, ECFUIViewAvailability::KnownZero);
	TestEqual(TEXT("0발 Loaded 0"), ZeroAmmoViewData.LoadedAmmoCount, 0);
	TestEqual(TEXT("0발 MagazineCapacity 4 유지"), ZeroAmmoViewData.MagazineCapacity, 4);
	TestEqual(TEXT("0발 Reserve 0"), ZeroAmmoViewData.ReserveAmmoCount, 0);
	TestEqual(TEXT("0발 CurrentOnboard 0"), ZeroAmmoViewData.CurrentOnboardAmmoCount, 0);

	// [v1.1.0] 탄약이 0이어도 실제 탄창 용량은 유지해 `0 / 4`로 표시합니다.
	FText ZeroAmmoText;
	TestTrue(TEXT("0발 Ammo Primary 문구 표시"), UCFHUDPresenter::ResolveAmmoPresentation(ZeroAmmoViewData, ZeroAmmoText));
	TestEqual(TEXT("0발 Ammo Primary 문구 0 / 4"), ZeroAmmoText.ToString(), FString(TEXT("0 / 4")));

	// [v1.1.0] Reserve KnownZero도 숨기지 않고 라벨 없는 숫자 0으로 표시합니다.
	FText ZeroReserveAmmoText;
	TestTrue(TEXT("0발 Reserve 숫자 표시"), UCFHUDPresenter::ResolveReserveAmmoPresentation(ZeroAmmoViewData, ZeroReserveAmmoText));
	TestEqual(TEXT("0발 Reserve 숫자 0"), ZeroReserveAmmoText.ToString(), FString(TEXT("0")));

	// [v1.0.0] 탄약 완전 소진 상태의 Production Weapon 상태 문구입니다.
	FText NoAmmoStatusText;

	// [v1.0.0] NO AMMO 상태 Progress 값입니다.
	float NoAmmoStatusProgress = 1.0f;
	TestTrue(
		TEXT("NO AMMO 상태 표시"),
		UCFHUDPresenter::ResolveWeaponStatusPresentation(
			ZeroAmmoViewData,
			false,
			NoAmmoStatusText,
			NoAmmoStatusProgress));
	TestEqual(TEXT("NO AMMO 문구"), NoAmmoStatusText.ToString(), FString(TEXT("NO AMMO")));
	TestEqual(TEXT("NO AMMO 진행률 0"), NoAmmoStatusProgress, 0.0f);

	// [v1.0.0] Launcher Sequence 전용 Row가 보일 때 상태 행 중복 표시가 없어야 하는 합성 ViewData입니다.
	FCFWeaponHUDData SequencePriorityViewData = ZeroAmmoViewData;
	SequencePriorityViewData.bLauncherSequenceActive = true;
	SequencePriorityViewData.LauncherAvailability = ECFUIViewAvailability::Known;
	SequencePriorityViewData.LauncherPattern = ECFLauncherFirePattern::Ripple;
	SequencePriorityViewData.LauncherTotalProjectileCount = 4;
	SequencePriorityViewData.LauncherAcceptedProjectileCount = 1;

	// [v1.0.0] Sequence 우선순위 검증에서 상태 행이 숨겨질 때 비워질 Text입니다.
	FText HiddenStatusText;

	// [v1.0.0] Sequence 우선순위 검증에서 상태 행이 숨겨질 때 0으로 유지될 Progress입니다.
	float HiddenStatusProgress = 1.0f;
	TestFalse(
		TEXT("Launcher Sequence가 보이면 Reload/NoAmmo/Cooldown 상태 행 숨김"),
		UCFHUDPresenter::ResolveWeaponStatusPresentation(
			SequencePriorityViewData,
			true,
			HiddenStatusText,
			HiddenStatusProgress));

	HUDDataProvider->ShutdownProvider();
	FirstSetup.VehiclePawn->Destroy();
	SecondSetup.VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
