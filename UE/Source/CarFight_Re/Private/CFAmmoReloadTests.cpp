// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-13
// Description: CF-FQ-031 AMMO-P0-05 FullMagazine Reload Runtime Automation
// Scope: 수동·자동 Reload, 완료 전 수량 보존, 부분 Reload, Launcher Lock, 취소와 Pause-safe Tick 계약을 검증합니다.
// Changelog:
// - v1.0.0: AMMO-P0-05 FullMagazine Reload 통합 Automation 최초 추가.
// Migration:
// - Automation World와 Transient DataAsset만 사용하며 저장 Asset과 InputAction을 수정하지 않습니다.
// - PerRound는 P0 실제 실행 대상이 아니므로 ExecutionFailed 계약만 유지합니다.

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

namespace
{
	// [v1.0.0] Reload 테스트 WeaponData를 활성 RoofTurret 프로파일로 해석할 Transient VehicleData를 생성합니다.
	UCFVehicleData* CreateReloadTestVehicleData(UObject* Outer, UCFWeaponData* WeaponData)
	{
		if (!Outer || !WeaponData)
		{
			return nullptr;
		}

		// [v1.0.0] 테스트 MountProfile에서 WeaponData를 단일 경로로 제공할 EquipmentPresetData입니다.
		UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>(Outer);
		EquipmentPresetData->EquipmentId = TEXT("AmmoP005_TestPreset");
		EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
		EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Large;
		EquipmentPresetData->DefaultWeaponData = WeaponData;

		// [v1.0.0] WeaponComp가 활성 프로파일과 하드포인트를 해석할 Transient VehicleData입니다.
		UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(Outer);

		// [v1.0.0] 테스트 MountProfile이 참조할 최소 하드포인트 슬롯입니다.
		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = TEXT("Top_01");
		HardpointSlot.LocationCategory = TEXT("Top");
		HardpointSlot.LocalLocation = FVector::ZeroVector;
		VehicleData->HardpointSlots.Add(HardpointSlot);

		// [v1.0.0] VehicleWeaponComp 기본 ActiveMountProfileId와 일치하는 테스트 프로파일입니다.
		FCFVehicleMountProfile MountProfile;
		MountProfile.MountProfileId = TEXT("RoofTurret_MediumOrLarge");
		MountProfile.LocationSlotRef = HardpointSlot.LocationSlotId;
		MountProfile.MountType = ECFVehicleMountType::Turret;
		MountProfile.SizeLimit = ECFVehicleWeaponSize::Large;
		MountProfile.DefaultEquipmentPresetData = EquipmentPresetData;
		VehicleData->MountProfiles.Add(MountProfile);
		return VehicleData;
	}

	// [v1.0.0] AmmoComp의 게임 Tick을 직접 한 번 진행해 Reload 경과 시간과 완료 경계를 검증합니다.
	void AdvanceReload(UCFVehicleAmmoComp* VehicleAmmoComp, const float DeltaSeconds)
	{
		if (VehicleAmmoComp)
		{
			VehicleAmmoComp->TickComponent(DeltaSeconds, LEVELTICK_All, nullptr);
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFAmmoReloadRuntimeTest,
	"CarFight.Ammo.AMMO_P0_05.Reload",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] FullMagazine Reload의 시간·수량 보존·Action Lock·자동 시작과 Launcher 종료 연계를 검증합니다.
bool FCFAmmoReloadRuntimeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 실제 Pawn·AmmoComp 수명과 Component Tick 설정을 사용할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("AMMO-P0-05 Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] VehicleAmmoComp와 VehicleWeaponComp 기본 서브오브젝트를 소유할 테스트 차량입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("AMMO-P0-05 Vehicle Pawn"), VehiclePawn))
	{
		return false;
	}

	// [v1.0.0] FullMagazine 상태·수량·Tick을 실제 소유할 차량 탄약 컴포넌트입니다.
	UCFVehicleAmmoComp* VehicleAmmoComp = VehiclePawn->GetVehicleAmmoComp();

	// [v1.0.0] 현재 활성 MountProfileId와 WeaponData를 제공할 차량 무기 컴포넌트입니다.
	UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();
	if (!TestNotNull(TEXT("VehicleAmmoComp"), VehicleAmmoComp)
		|| !TestNotNull(TEXT("VehicleWeaponComp"), VehicleWeaponComp))
	{
		return false;
	}
	TestFalse(TEXT("Reload Tick은 Pause 중 진행 금지"), VehicleAmmoComp->PrimaryComponentTick.bTickEvenWhenPaused);

	// [v1.0.0] 모든 Reload 시나리오에서 공유할 유한탄 Transient AmmoData입니다.
	UCFAmmoData* AmmoData = NewObject<UCFAmmoData>(GetTransientPackage());
	AmmoData->AmmoId = TEXT("AmmoP005_TestShell");

	// [v1.0.0] FullMagazine 5발 탄창과 2초 Reload를 사용하는 Transient WeaponData입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	WeaponData->WeaponId = TEXT("AmmoP005_TestWeapon");
	WeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
	WeaponData->CompatibleMountTypes.Reset();
	WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
	WeaponData->DefaultAmmoData = AmmoData;
	WeaponData->MagazineSize = 5;
	WeaponData->InitialLoadedAmmoCount = 2;
	WeaponData->AmmoUnitsPerShot = 1;
	WeaponData->ReloadTimeSeconds = 2.0f;
	WeaponData->ReloadMode = ECFWeaponReloadMode::FullMagazine;
	WeaponData->bUseInfiniteAmmoForDebug = false;
	WeaponData->bAutoReloadWhenEmpty = false;
	WeaponData->bAllowPartialReload = true;

	// [v1.0.0] Pawn의 현재 무기 Reload 명령까지 검증할 Transient VehicleData입니다.
	UCFVehicleData* VehicleData = CreateReloadTestVehicleData(GetTransientPackage(), WeaponData);
	if (!TestNotNull(TEXT("AMMO-P0-05 VehicleData"), VehicleData))
	{
		return false;
	}
	TestTrue(TEXT("Reload Weapon Runtime 초기화"), VehicleWeaponComp->InitializeWeaponRuntime(VehiclePawn, VehicleData));
	TestTrue(TEXT("수동 Reload용 Ammo Runtime 초기화"), VehicleAmmoComp->InitializeActiveWeaponAmmoRuntime(VehiclePawn, VehicleWeaponComp, 7));

	// [v1.0.0] Ammo Runtime에서 현재 WeaponInstance를 식별할 활성 MountProfileId입니다.
	const FName WeaponInstanceId = VehicleWeaponComp->GetActiveMountProfileId();
	TestEqual(TEXT("Pawn 현재 무기 수동 Reload 요청 승인"), VehiclePawn->RequestReloadCurrentWeapon(), ECFAmmoTransactionResult::Accepted);

	// [v1.0.0] Reload 시작 직후 수량 이동 없이 Pending 상태와 Action Lock만 적용됐는지 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot ReloadStartedSnapshot;
	TestTrue(TEXT("Reload 시작 Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, ReloadStartedSnapshot));
	TestEqual(TEXT("Reload 시작 전후 장전량 2 유지"), ReloadStartedSnapshot.LoadedAmmoCount, 2);
	TestEqual(TEXT("Reload 시작 전후 예비량 5 유지"), ReloadStartedSnapshot.ReserveAmmoCount, 5);
	TestEqual(TEXT("Reload 시작 전후 총 보유량 7 유지"), ReloadStartedSnapshot.CurrentOnboardAmmoCount, 7);
	TestEqual(TEXT("ReloadState Reloading"), ReloadStartedSnapshot.ReloadState, ECFWeaponReloadState::Reloading);
	TestEqual(TEXT("남은 Reload 시간 2초"), ReloadStartedSnapshot.RemainingReloadTimeSeconds, 2.0f);
	TestTrue(TEXT("Reload Action Lock 활성"), ReloadStartedSnapshot.bWeaponActionLocked);
	TestEqual(TEXT("Reload Action Lock 사유"), ReloadStartedSnapshot.WeaponActionLockReason, ECFWeaponActionLockReason::Reloading);
	TestEqual(TEXT("Reload 중 발사 검증은 Reloading"), VehicleAmmoComp->ValidateSingleFireAmmo(WeaponInstanceId), ECFAmmoTransactionResult::Reloading);

	AdvanceReload(VehicleAmmoComp, 1.0f);

	// [v1.0.0] Reload 절반 경과 시에도 수량이 아직 이동하지 않았는지 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot MidReloadSnapshot;
	TestTrue(TEXT("Reload 중간 Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, MidReloadSnapshot));
	TestEqual(TEXT("Reload 완료 전 장전량 2 유지"), MidReloadSnapshot.LoadedAmmoCount, 2);
	TestEqual(TEXT("Reload 완료 전 예비량 5 유지"), MidReloadSnapshot.ReserveAmmoCount, 5);
	TestEqual(TEXT("Reload 완료 전 총 보유량 7 유지"), MidReloadSnapshot.CurrentOnboardAmmoCount, 7);
	TestEqual(TEXT("Reload 절반 후 남은 시간 1초"), MidReloadSnapshot.RemainingReloadTimeSeconds, 1.0f);

	AdvanceReload(VehicleAmmoComp, 1.0f);

	// [v1.0.0] 완료 순간 필요한 3발만 Reserve에서 Loaded로 이동했는지 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot ReloadCompletedSnapshot;
	TestTrue(TEXT("Reload 완료 Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, ReloadCompletedSnapshot));
	TestEqual(TEXT("Reload 완료 장전량 5"), ReloadCompletedSnapshot.LoadedAmmoCount, 5);
	TestEqual(TEXT("Reload 완료 예비량 2"), ReloadCompletedSnapshot.ReserveAmmoCount, 2);
	TestEqual(TEXT("Reload 완료 총 보유량 7 보존"), ReloadCompletedSnapshot.CurrentOnboardAmmoCount, 7);
	TestEqual(TEXT("Reload 완료 상태 Ready"), ReloadCompletedSnapshot.ReloadState, ECFWeaponReloadState::Ready);
	TestFalse(TEXT("Reload 완료 Action Lock 해제"), ReloadCompletedSnapshot.bWeaponActionLocked);

	WeaponData->InitialLoadedAmmoCount = 1;
	TestTrue(TEXT("부분 Reload용 Ammo 재초기화"), VehicleAmmoComp->InitializeActiveWeaponAmmoRuntime(VehiclePawn, VehicleWeaponComp, 3));
	TestEqual(TEXT("예비 부족 부분 Reload 시작"), VehicleAmmoComp->RequestReload(WeaponInstanceId), ECFAmmoTransactionResult::Accepted);
	AdvanceReload(VehicleAmmoComp, 2.0f);

	// [v1.0.0] 예비 2발만 존재할 때 가능한 만큼만 이동한 부분 Reload 결과입니다.
	FCFAmmoRuntimeSnapshot PartialReloadSnapshot;
	TestTrue(TEXT("부분 Reload Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, PartialReloadSnapshot));
	TestEqual(TEXT("부분 Reload 장전량 3"), PartialReloadSnapshot.LoadedAmmoCount, 3);
	TestEqual(TEXT("부분 Reload 예비량 0"), PartialReloadSnapshot.ReserveAmmoCount, 0);
	TestEqual(TEXT("부분 Reload 총 보유량 3 보존"), PartialReloadSnapshot.CurrentOnboardAmmoCount, 3);

	WeaponData->InitialLoadedAmmoCount = 4;
	TestTrue(TEXT("Launcher Lock 검증용 Ammo 재초기화"), VehicleAmmoComp->InitializeActiveWeaponAmmoRuntime(VehiclePawn, VehicleWeaponComp, 5));

	// [v1.0.0] Reload 요청보다 우선할 Launcher Sequence 예약 발수입니다.
	int32 ReservedLauncherShotCount = 0;
	TestEqual(
		TEXT("Launcher Sequence 2발 예약"),
		VehicleAmmoComp->ReserveLauncherSequenceAmmo(WeaponInstanceId, 2, true, ReservedLauncherShotCount),
		ECFAmmoTransactionResult::Accepted);
	TestEqual(TEXT("Launcher Active 중 Reload 시작 거부"), VehicleAmmoComp->RequestReload(WeaponInstanceId), ECFAmmoTransactionResult::ActionLocked);
	TestTrue(TEXT("Reload 거부 뒤 Launcher 예약 유지"), VehicleAmmoComp->HasActiveLauncherSequenceReservation(WeaponInstanceId));
	VehicleAmmoComp->ReleaseLauncherSequenceReservation(WeaponInstanceId);

	WeaponData->InitialLoadedAmmoCount = 2;
	WeaponData->bAutoReloadWhenEmpty = false;
	TestTrue(TEXT("Reload 취소 검증용 Ammo 재초기화"), VehicleAmmoComp->InitializeActiveWeaponAmmoRuntime(VehiclePawn, VehicleWeaponComp, 7));
	TestEqual(TEXT("취소 검증 Reload 시작"), VehicleAmmoComp->RequestReload(WeaponInstanceId), ECFAmmoTransactionResult::Accepted);
	AdvanceReload(VehicleAmmoComp, 0.5f);
	TestEqual(TEXT("Reload 수동 취소"), VehicleAmmoComp->CancelReload(WeaponInstanceId), ECFAmmoTransactionResult::Accepted);

	// [v1.0.0] 수동 취소가 수량 이동 없이 Reload Lock만 해제했는지 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot CancelledReloadSnapshot;
	TestTrue(TEXT("Reload 취소 Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, CancelledReloadSnapshot));
	TestEqual(TEXT("취소 후 장전량 2 유지"), CancelledReloadSnapshot.LoadedAmmoCount, 2);
	TestEqual(TEXT("취소 후 예비량 5 유지"), CancelledReloadSnapshot.ReserveAmmoCount, 5);
	TestEqual(TEXT("취소 후 총 보유량 7 유지"), CancelledReloadSnapshot.CurrentOnboardAmmoCount, 7);
	TestFalse(TEXT("취소 후 Reload 비활성"), VehicleAmmoComp->IsReloadActive(WeaponInstanceId));
	TestFalse(TEXT("취소 후 Action Lock 해제"), CancelledReloadSnapshot.bWeaponActionLocked);

	WeaponData->MagazineSize = 2;
	WeaponData->InitialLoadedAmmoCount = 1;
	WeaponData->ReloadTimeSeconds = 1.0f;
	WeaponData->bAutoReloadWhenEmpty = true;
	TestTrue(TEXT("SingleCycle 자동 Reload용 Ammo 재초기화"), VehicleAmmoComp->InitializeActiveWeaponAmmoRuntime(VehiclePawn, VehicleWeaponComp, 3));
	TestEqual(TEXT("SingleCycle 탄약 예약"), VehicleAmmoComp->ReserveSingleFireAmmo(WeaponInstanceId), ECFAmmoTransactionResult::Accepted);
	TestEqual(TEXT("SingleCycle 소비 Commit"), VehicleAmmoComp->CommitSingleFireAmmo(WeaponInstanceId), ECFAmmoTransactionResult::Accepted);

	// [v1.0.0] 빈 탄창 성공 발사 직후 자동 Reload가 수량 이동 없이 시작됐는지 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot AutoReloadStartedSnapshot;
	TestTrue(TEXT("SingleCycle 자동 Reload Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, AutoReloadStartedSnapshot));
	TestEqual(TEXT("자동 Reload 시작 장전량 0"), AutoReloadStartedSnapshot.LoadedAmmoCount, 0);
	TestEqual(TEXT("자동 Reload 시작 예비량 2"), AutoReloadStartedSnapshot.ReserveAmmoCount, 2);
	TestEqual(TEXT("자동 Reload 상태 Reloading"), AutoReloadStartedSnapshot.ReloadState, ECFWeaponReloadState::Reloading);
	TestEqual(TEXT("자동 Reload 중 발사 거부"), VehicleAmmoComp->ValidateSingleFireAmmo(WeaponInstanceId), ECFAmmoTransactionResult::Reloading);
	AdvanceReload(VehicleAmmoComp, 1.0f);

	// [v1.0.0] 자동 Reload 완료 뒤 남은 차량 보유 2발이 모두 탄창으로 이동했는지 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot AutoReloadCompletedSnapshot;
	TestTrue(TEXT("SingleCycle 자동 Reload 완료 Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, AutoReloadCompletedSnapshot));
	TestEqual(TEXT("자동 Reload 완료 장전량 2"), AutoReloadCompletedSnapshot.LoadedAmmoCount, 2);
	TestEqual(TEXT("자동 Reload 완료 예비량 0"), AutoReloadCompletedSnapshot.ReserveAmmoCount, 0);
	TestEqual(TEXT("실제 발사 후 총 보유량 2"), AutoReloadCompletedSnapshot.CurrentOnboardAmmoCount, 2);

	WeaponData->InitialLoadedAmmoCount = 1;
	TestTrue(TEXT("Launcher 종료 자동 Reload용 Ammo 재초기화"), VehicleAmmoComp->InitializeActiveWeaponAmmoRuntime(VehiclePawn, VehicleWeaponComp, 3));

	// [v1.0.0] 첫 발 한 발짜리 Launcher 예약을 만들어 성공 Commit 후에도 Launcher Action Lock을 유지할 발수입니다.
	int32 SingleLauncherReservedShotCount = 0;
	TestEqual(
		TEXT("Launcher 종료 자동 Reload용 1발 예약"),
		VehicleAmmoComp->ReserveLauncherSequenceAmmo(WeaponInstanceId, 1, true, SingleLauncherReservedShotCount),
		ECFAmmoTransactionResult::Accepted);
	TestEqual(TEXT("Launcher 첫 발 Commit"), VehicleAmmoComp->CommitReservedLauncherShot(WeaponInstanceId), ECFAmmoTransactionResult::Accepted);

	// [v1.0.0] 모든 예약 탄약이 소비됐어도 Terminal 전까지 LauncherSequenceActive Lock이 유지되는 Snapshot입니다.
	FCFAmmoRuntimeSnapshot BeforeLauncherTerminalSnapshot;
	TestTrue(TEXT("Launcher Terminal 전 Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, BeforeLauncherTerminalSnapshot));
	TestEqual(TEXT("Launcher 첫 발 뒤 장전량 0"), BeforeLauncherTerminalSnapshot.LoadedAmmoCount, 0);
	TestTrue(TEXT("Launcher Terminal 전 Action Lock 유지"), BeforeLauncherTerminalSnapshot.bWeaponActionLocked);
	TestFalse(TEXT("Launcher Terminal 전 자동 Reload 미시작"), VehicleAmmoComp->IsReloadActive(WeaponInstanceId));
	TestEqual(TEXT("Launcher Terminal 예약 반환"), VehicleAmmoComp->ReleaseLauncherSequenceReservation(WeaponInstanceId), ECFAmmoTransactionResult::Accepted);

	// [v1.0.0] Terminal에서 Launcher Lock이 풀리자 같은 처리에서 자동 Reload가 시작됐는지 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot LauncherTerminalReloadSnapshot;
	TestTrue(TEXT("Launcher Terminal 자동 Reload Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, LauncherTerminalReloadSnapshot));
	TestEqual(TEXT("Launcher Terminal 뒤 Reloading"), LauncherTerminalReloadSnapshot.ReloadState, ECFWeaponReloadState::Reloading);
	TestEqual(TEXT("Launcher Lock에서 Reload Lock으로 전환"), LauncherTerminalReloadSnapshot.WeaponActionLockReason, ECFWeaponActionLockReason::Reloading);
	AdvanceReload(VehicleAmmoComp, 1.0f);

	// [v1.0.0] Launcher 종료 후 자동 Reload 완료 결과입니다.
	FCFAmmoRuntimeSnapshot LauncherAutoReloadCompletedSnapshot;
	TestTrue(TEXT("Launcher 자동 Reload 완료 Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, LauncherAutoReloadCompletedSnapshot));
	TestEqual(TEXT("Launcher 자동 Reload 완료 장전량 2"), LauncherAutoReloadCompletedSnapshot.LoadedAmmoCount, 2);
	TestEqual(TEXT("Launcher 자동 Reload 완료 예비량 0"), LauncherAutoReloadCompletedSnapshot.ReserveAmmoCount, 0);
	TestFalse(TEXT("Launcher 자동 Reload 완료 Action Lock 해제"), LauncherAutoReloadCompletedSnapshot.bWeaponActionLocked);

	WeaponData->ReloadMode = ECFWeaponReloadMode::PerRound;
	WeaponData->InitialLoadedAmmoCount = 1;
	WeaponData->bAutoReloadWhenEmpty = false;
	TestTrue(TEXT("PerRound 계약 검증용 Ammo 재초기화"), VehicleAmmoComp->InitializeActiveWeaponAmmoRuntime(VehiclePawn, VehicleWeaponComp, 3));
	TestEqual(TEXT("P0 PerRound 실제 실행은 미지원"), VehicleAmmoComp->RequestReload(WeaponInstanceId), ECFAmmoTransactionResult::ExecutionFailed);

	VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
