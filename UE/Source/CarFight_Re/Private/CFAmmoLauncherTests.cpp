// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-13
// Description: CF-FQ-031 AMMO-P0-04 Launcher Reservation and Action Lock Automation
// Scope: Ripple·Salvo 전체 유효 발수 예약, 성공 Commit, 실패 Release, Sequence Action Lock 유지와 Terminal 전체 반환을 검증합니다.
// Changelog:
// - v1.0.0: AMMO-P0-04 Launcher 예약·Action Lock 통합 Automation 최초 추가.
// Migration:
// - Automation World와 Transient DataAsset만 사용하며 저장 Asset을 생성·수정하지 않습니다.
// - Reload 실제 수량 이동은 AMMO-P0-05 범위이므로 이 테스트에서는 Action Lock 계약만 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFAmmoData.h"
#include "CFEquipmentPresetData.h"
#include "CFLauncherComp.h"
#include "CFVehicleAmmoComp.h"
#include "CFVehicleData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	// [v1.0.0] Launcher Ammo 테스트 WeaponData를 활성 RoofTurret 프로파일로 해석할 Transient VehicleData를 생성합니다.
	UCFVehicleData* CreateLauncherAmmoTestVehicleData(UObject* Outer, UCFWeaponData* WeaponData)
	{
		if (!Outer || !WeaponData)
		{
			return nullptr;
		}

		// [v1.0.0] 테스트 MountProfile이 단일 WeaponData를 참조하도록 제공할 EquipmentPresetData입니다.
		UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>(Outer);
		EquipmentPresetData->EquipmentId = TEXT("AmmoP004_TestPreset");
		EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
		EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Large;
		EquipmentPresetData->DefaultWeaponData = WeaponData;

		// [v1.0.0] WeaponComp 활성 프로파일과 최소 하드포인트를 제공할 Transient VehicleData입니다.
		UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(Outer);

		// [v1.0.0] 테스트 활성 MountProfile이 참조할 최소 하드포인트 슬롯입니다.
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
		return VehicleData;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFAmmoLauncherLockTest,
	"CarFight.Ammo.AMMO_P0_04.LauncherLock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 유한탄 Ripple의 전체 예약, 성공·실패 처리, Action Lock과 Cancel 정리를 검증합니다.
bool FCFAmmoLauncherLockTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 실제 Pawn·AmmoComp·LauncherComp 수명을 사용할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("AMMO-P0-04 Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] VehicleAmmoComp·VehicleWeaponComp·LauncherComp 기본 서브오브젝트를 소유할 테스트 차량입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("AMMO-P0-04 Vehicle Pawn"), VehiclePawn))
	{
		return false;
	}

	// [v1.0.0] Launcher 예약과 Action Lock을 단일 소유할 차량 탄약 컴포넌트입니다.
	UCFVehicleAmmoComp* VehicleAmmoComp = VehiclePawn->GetVehicleAmmoComp();

	// [v1.0.0] 활성 WeaponData와 MountProfileId를 제공할 차량 무기 컴포넌트입니다.
	UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();

	// [v1.0.0] 예약을 인수하고 Cancel terminal에서 남은 탄약을 반환할 런처 컴포넌트입니다.
	UCFLauncherComp* LauncherComp = VehiclePawn->GetLauncherComp();
	if (!TestNotNull(TEXT("VehicleAmmoComp"), VehicleAmmoComp)
		|| !TestNotNull(TEXT("VehicleWeaponComp"), VehicleWeaponComp)
		|| !TestNotNull(TEXT("LauncherComp"), LauncherComp))
	{
		return false;
	}

	// [v1.0.0] 이번 테스트에서 공유할 실제 유한탄 Transient AmmoData입니다.
	UCFAmmoData* AmmoData = NewObject<UCFAmmoData>(GetTransientPackage());
	AmmoData->AmmoId = TEXT("AmmoP004_TestRocket");

	// [v1.0.0] 6발 Ripple 요청과 4발 탄창을 사용해 부분 Sequence 예약을 검증할 Transient WeaponData입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	WeaponData->WeaponId = TEXT("AmmoP004_TestLauncher");
	WeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
	WeaponData->CompatibleMountTypes.Reset();
	WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
	WeaponData->DefaultAmmoData = AmmoData;
	WeaponData->MagazineSize = 4;
	WeaponData->InitialLoadedAmmoCount = 4;
	WeaponData->AmmoUnitsPerShot = 1;
		WeaponData->bUseInfiniteAmmoForDebug = false;
	WeaponData->bAutoReloadWhenEmpty = false;
	WeaponData->bAllowPartialSequence = true;
	WeaponData->LauncherFirePatternConfig.FirePattern = ECFLauncherFirePattern::Ripple;
	WeaponData->LauncherFirePatternConfig.ProjectileCountPerTrigger = 6;
	WeaponData->LauncherFirePatternConfig.InterMuzzleDelaySeconds = 0.5f;
	WeaponData->LauncherFirePatternConfig.SequenceFailurePolicy = ECFLauncherSequenceFailurePolicy::ContinueRemaining;

	// [v1.0.0] 테스트 Launcher를 현재 활성 WeaponData로 해석할 Transient VehicleData입니다.
	UCFVehicleData* VehicleData = CreateLauncherAmmoTestVehicleData(GetTransientPackage(), WeaponData);
	if (!TestNotNull(TEXT("AMMO-P0-04 VehicleData"), VehicleData))
	{
		return false;
	}
	TestTrue(TEXT("Launcher Weapon Runtime 초기화"), VehicleWeaponComp->InitializeWeaponRuntime(VehiclePawn, VehicleData));
	TestTrue(TEXT("Launcher Runtime 초기화"), LauncherComp->InitializeLauncherRuntime(VehiclePawn, VehicleWeaponComp));
	TestTrue(TEXT("유한탄 Runtime 초기화"), VehicleAmmoComp->InitializeActiveWeaponAmmoRuntime(VehiclePawn, VehicleWeaponComp, 4));

	// [v1.0.0] Ammo Runtime에서 현재 Launcher를 식별할 활성 MountProfileId입니다.
	const FName WeaponInstanceId = VehicleWeaponComp->GetActiveMountProfileId();

	// [v1.0.0] 6발 요청에서 실제 장전 4발로 제한되어 예약될 발수입니다.
	int32 ReservedShotCount = 0;
	TestEqual(
		TEXT("6발 요청·4발 장전은 부분 Sequence 4발 예약"),
		VehicleAmmoComp->ReserveLauncherSequenceAmmo(WeaponInstanceId, 6, true, ReservedShotCount),
		ECFAmmoTransactionResult::Accepted);
	TestEqual(TEXT("실제 예약 발수 4"), ReservedShotCount, 4);

	// [v1.0.0] 전체 예약 직후 자유 사용량과 Action Lock을 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot ReservedSnapshot;
	TestTrue(TEXT("전체 예약 Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, ReservedSnapshot));
	TestEqual(TEXT("장전량은 예약 단계에서 아직 4 유지"), ReservedSnapshot.LoadedAmmoCount, 4);
	TestEqual(TEXT("Sequence 예약량 4"), ReservedSnapshot.ReservedSequenceAmmoCount, 4);
	TestEqual(TEXT("즉시 자유 사용량 0"), ReservedSnapshot.ImmediateUsableAmmoCount, 0);
	TestEqual(TEXT("Pending Sequence Shot 4"), ReservedSnapshot.PendingSequenceShotCount, 4);
	TestTrue(TEXT("Launcher Action Lock 활성"), ReservedSnapshot.bWeaponActionLocked);
	TestEqual(TEXT("Action Lock 사유 LauncherSequenceActive"), ReservedSnapshot.WeaponActionLockReason, ECFWeaponActionLockReason::LauncherSequenceActive);

	// [v1.0.0] 진행 중 Sequence에 새 예약 요청을 넣어 기존 Sequence가 교체되지 않는지 확인할 출력 수량입니다.
	int32 RejectedReservedShotCount = 0;
	TestEqual(
		TEXT("진행 중 추가 Sequence 예약 거부"),
		VehicleAmmoComp->ReserveLauncherSequenceAmmo(WeaponInstanceId, 2, true, RejectedReservedShotCount),
		ECFAmmoTransactionResult::SequenceAlreadyActive);
	TestEqual(TEXT("거부된 추가 예약 수량 0"), RejectedReservedShotCount, 0);

	TestEqual(TEXT("첫 성공 발사 예약 Commit"), VehicleAmmoComp->CommitReservedLauncherShot(WeaponInstanceId), ECFAmmoTransactionResult::Accepted);

	// [v1.0.0] 첫 성공 발사 뒤 장전·예약·통계를 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot AfterFirstCommitSnapshot;
	TestTrue(TEXT("첫 Commit Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, AfterFirstCommitSnapshot));
	TestEqual(TEXT("첫 성공 후 장전량 3"), AfterFirstCommitSnapshot.LoadedAmmoCount, 3);
	TestEqual(TEXT("첫 성공 후 예약량 3"), AfterFirstCommitSnapshot.ReservedSequenceAmmoCount, 3);
	TestEqual(TEXT("Sequence 실제 소비 누계 1"), AfterFirstCommitSnapshot.FiredAmmoCountThisSequence, 1);
	TestEqual(TEXT("출격 실제 소비 누계 1"), AfterFirstCommitSnapshot.FiredAmmoCountThisSortie, 1);

	TestEqual(TEXT("후속 실패 한 발 예약 반환"), VehicleAmmoComp->ReleaseReservedLauncherShot(WeaponInstanceId), ECFAmmoTransactionResult::Accepted);

	// [v1.0.0] 실패한 한 발은 소비하지 않고 자유 장전량으로 돌아왔는지 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot AfterFailedShotSnapshot;
	TestTrue(TEXT("실패 Release Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, AfterFailedShotSnapshot));
	TestEqual(TEXT("실패 후 장전량 3 유지"), AfterFailedShotSnapshot.LoadedAmmoCount, 3);
	TestEqual(TEXT("실패한 1발 예약만 해제되어 예약량 2"), AfterFailedShotSnapshot.ReservedSequenceAmmoCount, 2);
	TestEqual(TEXT("실패 반환 1발은 즉시 자유 장전량"), AfterFailedShotSnapshot.ImmediateUsableAmmoCount, 1);
	TestTrue(TEXT("실패 반환 뒤에도 Sequence Action Lock 유지"), AfterFailedShotSnapshot.bWeaponActionLocked);

	TestEqual(TEXT("API 단계 Sequence 전체 예약 반환"), VehicleAmmoComp->ReleaseLauncherSequenceReservation(WeaponInstanceId), ECFAmmoTransactionResult::Accepted);

	// [v1.0.0] API 전체 반환 뒤 Action Lock과 남은 예약이 모두 사라졌는지 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot AfterManualReleaseSnapshot;
	TestTrue(TEXT("전체 반환 Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, AfterManualReleaseSnapshot));
	TestEqual(TEXT("전체 반환 뒤 예약량 0"), AfterManualReleaseSnapshot.ReservedSequenceAmmoCount, 0);
	TestEqual(TEXT("전체 반환 뒤 장전량 3"), AfterManualReleaseSnapshot.LoadedAmmoCount, 3);
	TestFalse(TEXT("전체 반환 뒤 Action Lock 해제"), AfterManualReleaseSnapshot.bWeaponActionLocked);

	// [v1.0.0] LauncherComp terminal 정리가 실제 Ammo 예약과 Action Lock을 해제하는 통합 경로를 위해 탄약을 새 출격 상태로 복원합니다.
	TestTrue(TEXT("Launcher Cancel 통합용 Ammo 재초기화"), VehicleAmmoComp->InitializeActiveWeaponAmmoRuntime(VehiclePawn, VehicleWeaponComp, 4));

	// [v1.0.0] LauncherComp가 인수할 두 번째 4발 Sequence 전체 예약 수량입니다.
	int32 LauncherReservedShotCount = 0;
	TestEqual(
		TEXT("Launcher Cancel 통합용 4발 예약"),
		VehicleAmmoComp->ReserveLauncherSequenceAmmo(WeaponInstanceId, 4, true, LauncherReservedShotCount),
		ECFAmmoTransactionResult::Accepted);
	TestEqual(TEXT("Launcher 인수 예약 발수 4"), LauncherReservedShotCount, 4);
	TestEqual(TEXT("Launcher 첫 성공 발사 Commit"), VehicleAmmoComp->CommitReservedLauncherShot(WeaponInstanceId), ECFAmmoTransactionResult::Accepted);

	// [v1.0.0] 첫 성공 발사를 포함해 실제 예약 4발로 축소된 Ripple Scheduler 설정입니다.
	FCFLauncherFirePatternConfig ReservedRippleConfig = WeaponData->GetEffectiveLauncherFirePatternConfig();
	ReservedRippleConfig.ProjectileCountPerTrigger = LauncherReservedShotCount;
	TestTrue(
		TEXT("LauncherComp가 첫 발 뒤 남은 예약을 인수"),
		LauncherComp->StartFireSequenceAfterFirstAcceptedShot(
			ReservedRippleConfig,
			FVector(5000.0f, 0.0f, 1000.0f),
			nullptr,
			0.0f,
			WeaponInstanceId));
	TestTrue(TEXT("Launcher Sequence Active"), LauncherComp->IsFireSequenceActive());

	// [v1.0.0] Action Lock 상태에서 거부된 새 행동이 기존 Launcher Sequence를 취소하지 않는지 확인할 수량입니다.
	int32 LockedRequestReservedShotCount = 0;
	TestEqual(
		TEXT("Active Sequence 중 신규 예약은 거부"),
		VehicleAmmoComp->ReserveLauncherSequenceAmmo(WeaponInstanceId, 1, true, LockedRequestReservedShotCount),
		ECFAmmoTransactionResult::SequenceAlreadyActive);
	TestTrue(TEXT("거부 뒤 기존 Launcher Sequence 계속 Active"), LauncherComp->IsFireSequenceActive());

	LauncherComp->CancelFireSequence(ECFLauncherSequenceCancelReason::Manual);
	TestFalse(TEXT("Manual Cancel 뒤 Launcher Sequence 종료"), LauncherComp->IsFireSequenceActive());
	TestEqual(TEXT("Manual Cancel terminal 상태"), LauncherComp->GetLauncherSequenceRuntime().State, ECFLauncherSequenceState::Cancelled);

	// [v1.0.0] Cancel terminal에서 남은 미실행 예약과 Action Lock이 자동 정리됐는지 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot AfterLauncherCancelSnapshot;
	TestTrue(TEXT("Launcher Cancel 뒤 Ammo Snapshot"), VehicleAmmoComp->TryGetAmmoSnapshot(WeaponInstanceId, AfterLauncherCancelSnapshot));
	TestEqual(TEXT("Cancel 뒤 남은 Sequence 예약 0"), AfterLauncherCancelSnapshot.ReservedSequenceAmmoCount, 0);
	TestEqual(TEXT("첫 성공 발사만 소비되어 장전량 3"), AfterLauncherCancelSnapshot.LoadedAmmoCount, 3);
	TestFalse(TEXT("Cancel 뒤 Launcher Action Lock 해제"), AfterLauncherCancelSnapshot.bWeaponActionLocked);
	TestEqual(TEXT("Cancel 뒤 Action Lock 사유 None"), AfterLauncherCancelSnapshot.WeaponActionLockReason, ECFWeaponActionLockReason::None);

	WeaponData->InitialLoadedAmmoCount = 3;
	TestTrue(TEXT("부분 시퀀스 금지 검증용 Ammo 재초기화"), VehicleAmmoComp->InitializeActiveWeaponAmmoRuntime(VehiclePawn, VehicleWeaponComp, 3));

	// [v1.0.0] 4발 요청에 3발만 장전된 상태에서 부분 Sequence 금지 시 예약 결과 수량입니다.
	int32 FullSequenceOnlyReservedShotCount = 0;
	TestEqual(
		TEXT("부분 시퀀스 금지 시 4발 요청·3발 장전 전체 거부"),
		VehicleAmmoComp->ReserveLauncherSequenceAmmo(WeaponInstanceId, 4, false, FullSequenceOnlyReservedShotCount),
		ECFAmmoTransactionResult::NotEnoughLoadedAmmo);
	TestEqual(TEXT("전체 거부 예약 발수 0"), FullSequenceOnlyReservedShotCount, 0);
	TestFalse(TEXT("전체 거부 뒤 Launcher 예약 없음"), VehicleAmmoComp->HasActiveLauncherSequenceReservation(WeaponInstanceId));

	VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
