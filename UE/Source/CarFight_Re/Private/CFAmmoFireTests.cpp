// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-13
// Description: CF-FQ-031 AMMO-P0-03 Single Fire Ammo Transaction Automation
// Scope: 실제 ACFVehiclePawn ExecuteAcceptedFireCommand 경로에서 HitScan Miss 소비, Projectile 실패 Rollback, Direct fallback 소비와 NoAmmo를 검증합니다.
// Changelog:
// - v1.0.0: AMMO-P0-03 SingleCycle Fire Transaction 통합 Automation 최초 추가.
// Migration:
// - Automation World와 Transient DataAsset만 사용하며 저장 Asset과 사용자 맵을 수정하지 않습니다.
// - Ripple·Salvo 예약은 이 테스트 범위가 아니며 AMMO-P0-04에서 별도로 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFAmmoData.h"
#include "CFEquipmentPresetData.h"
#include "CFProjectileActor.h"
#include "CFProjectileData.h"
#include "CFVehicleAmmoComp.h"
#include "CFVehicleData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	// [v1.0.0] 단일 RoofTurret 프로파일이 지정 WeaponData를 활성 장비로 해석하도록 Transient VehicleData를 구성합니다.
	UCFVehicleData* CreateAmmoFireTestVehicleData(UObject* Outer, UCFWeaponData* WeaponData)
	{
		if (!Outer || !WeaponData)
		{
			return nullptr;
		}

		// [v1.0.0] 활성 MountProfile에서 WeaponData를 단일 경로로 제공할 Transient EquipmentPresetData입니다.
		UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>(Outer);
		EquipmentPresetData->EquipmentId = TEXT("AmmoP003_TestPreset");
		EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
		EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Large;
		EquipmentPresetData->DefaultWeaponData = WeaponData;

		// [v1.0.0] WeaponComp의 활성 프로파일과 하드포인트 참조를 제공할 Transient VehicleData입니다.
		UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(Outer);

		// [v1.0.0] 테스트 활성 프로파일이 참조할 최소 하드포인트 슬롯입니다.
		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = TEXT("Top_01");
		HardpointSlot.LocationCategory = TEXT("Top");
		HardpointSlot.LocalLocation = FVector::ZeroVector;
		VehicleData->HardpointSlots.Add(HardpointSlot);

		// [v1.0.0] VehicleWeaponComp 기본 ActiveMountProfileId와 정확히 일치하는 테스트 MountProfile입니다.
		FCFVehicleMountProfile MountProfile;
		MountProfile.MountProfileId = TEXT("RoofTurret_MediumOrLarge");
		MountProfile.LocationSlotRef = HardpointSlot.LocationSlotId;
		MountProfile.MountType = ECFVehicleMountType::Turret;
		MountProfile.SizeLimit = ECFVehicleWeaponSize::Large;
		MountProfile.DefaultEquipmentPresetData = EquipmentPresetData;
		VehicleData->MountProfiles.Add(MountProfile);
		return VehicleData;
	}

	// [v1.0.0] 실제 ExecuteAcceptedFireCommand에 전달할 유효 SingleCycle 발사 요청을 생성합니다.
	FCFVehicleFireRequest BuildAmmoFireTestRequest(const int32 FireRequestId)
	{
		// [v1.0.0] 빈 Automation World에서 수평 Trace Miss가 발생하도록 충분히 높은 위치를 사용하는 발사 요청입니다.
		FCFVehicleFireRequest FireRequest;
		FireRequest.FireRequestId = FireRequestId;
		FireRequest.AimOrigin = FVector(0.0f, 0.0f, 2000.0f);
		FireRequest.AimDirection = FVector::ForwardVector;
		FireRequest.PredictedAimTargetLocation = FVector(5000.0f, 0.0f, 2000.0f);
		FireRequest.ClientFireTimeSeconds = static_cast<float>(FireRequestId);
		FireRequest.WeaponGroupId = TEXT("RoofTurret_MediumOrLarge");
		return FireRequest;
	}

	// [v1.0.0] 검증 승인 직후 Execute 단계만 직접 시험할 초기 Accepted FireResult를 생성합니다.
	FCFVehicleFireResult BuildAcceptedAmmoFireResult(const FCFVehicleFireRequest& FireRequest)
	{
		// [v1.0.0] 실제 Validate 성공 직후와 같은 최소 승인 상태를 표현하는 FireResult입니다.
		FCFVehicleFireResult FireResult;
		FireResult.FireRequestId = FireRequest.FireRequestId;
		FireResult.ValidationAimTargetLocation = FireRequest.PredictedAimTargetLocation;
		FireResult.LocalHitLocation = FireRequest.PredictedAimTargetLocation;
		FireResult.bAccepted = true;
		FireResult.RejectReason = ECFVehicleFireRejectReason::None;
		return FireResult;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFAmmoFireTransactionTest,
	"CarFight.Ammo.AMMO_P0_03.FireTransaction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 실제 Pawn 실행 경로에서 성공 발사만 소비되고 실행 실패는 예약을 반환하는지 검증합니다.
bool FCFAmmoFireTransactionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 실제 ACFVehiclePawn과 Fire 실행 경로를 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("AMMO-P0-03 Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] VehicleAmmoComp·VehicleWeaponComp·AimComp·ProjectilePoolComp 기본 서브오브젝트를 소유할 테스트 차량입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("AMMO-P0-03 Vehicle Pawn"), VehiclePawn))
	{
		return false;
	}

	// [v1.0.0] 실제 발사 Transaction을 소유할 차량 탄약 컴포넌트입니다.
	UCFVehicleAmmoComp* VehicleAmmoComp = VehiclePawn->GetVehicleAmmoComp();

	// [v1.0.0] 활성 WeaponData와 발사 모드를 제공할 차량 무기 컴포넌트입니다.
	UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();
	if (!TestNotNull(TEXT("VehicleAmmoComp"), VehicleAmmoComp)
		|| !TestNotNull(TEXT("VehicleWeaponComp"), VehicleWeaponComp))
	{
		return false;
	}

	// [v1.0.0] 이번 테스트에서 실제 유한탄으로 사용할 Transient AmmoData입니다.
	UCFAmmoData* AmmoData = NewObject<UCFAmmoData>(GetTransientPackage());
	AmmoData->AmmoId = TEXT("AmmoP003_TestShell");

	// [v1.0.0] 처음에는 HitScan SingleCycle로 구성하고 이후 같은 Runtime을 Projectile Direct로 전환할 WeaponData입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	WeaponData->WeaponId = TEXT("AmmoP003_TestWeapon");
	WeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
	WeaponData->CompatibleMountTypes.Reset();
	WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
	WeaponData->FireMode = ECFWeaponFireMode::HitScan;
	WeaponData->DefaultAmmoData = AmmoData;
	WeaponData->MagazineSize = 2;
	WeaponData->InitialLoadedAmmoCount = 2;
		WeaponData->AmmoUnitsPerShot = 1;
	WeaponData->bUseInfiniteAmmoForDebug = false;
	WeaponData->bAutoReloadWhenEmpty = false;
	WeaponData->LauncherFirePatternConfig.FirePattern = ECFLauncherFirePattern::SingleCycle;

	// [v1.0.0] WeaponComp가 테스트 WeaponData를 현재 활성 무기로 해석할 Transient VehicleData입니다.
	UCFVehicleData* VehicleData = CreateAmmoFireTestVehicleData(GetTransientPackage(), WeaponData);
	if (!TestNotNull(TEXT("AMMO-P0-03 VehicleData"), VehicleData))
	{
		return false;
	}
	TestTrue(TEXT("HitScan Weapon Runtime 초기화"), VehicleWeaponComp->InitializeWeaponRuntime(VehiclePawn, VehicleData));
	TestTrue(TEXT("유한탄 Runtime 초기화"), VehicleAmmoComp->InitializeActiveWeaponAmmoRuntime(VehiclePawn, VehicleWeaponComp, 3));

	// [v1.0.0] 첫 HitScan 발사 전 장전 2 + 예비 1 상태입니다.
	FCFAmmoRuntimeSnapshot InitialSnapshot;
	TestTrue(TEXT("초기 Ammo Snapshot"), VehicleAmmoComp->TryGetActiveWeaponAmmoSnapshot(VehicleWeaponComp, InitialSnapshot));
	TestEqual(TEXT("초기 장전량 2"), InitialSnapshot.LoadedAmmoCount, 2);
	TestEqual(TEXT("초기 예비량 1"), InitialSnapshot.ReserveAmmoCount, 1);
	TestEqual(TEXT("초기 차량 보유량 3"), InitialSnapshot.CurrentOnboardAmmoCount, 3);

	// [v1.0.0] 빈 월드 수평 방향으로 실제 Trace Miss를 발생시킬 첫 HitScan 요청입니다.
	const FCFVehicleFireRequest HitScanFireRequest = BuildAmmoFireTestRequest(1);

	// [v1.0.0] Validate 성공 직후 상태를 재현해 실제 Execute 경계만 검증할 HitScan 결과입니다.
	FCFVehicleFireResult HitScanFireResult = BuildAcceptedAmmoFireResult(HitScanFireRequest);
	TestTrue(TEXT("HitScan Trace Miss 실행 자체는 승인 발사"), VehiclePawn->ExecuteAcceptedFireCommand(HitScanFireRequest, HitScanFireResult, nullptr, true));
	TestTrue(TEXT("HitScan Trace Miss 후 FireResult 승인 유지"), HitScanFireResult.bAccepted);

	// [v1.0.0] Trace Miss도 실제 발사이므로 정확히 1발 소비됐는지 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot AfterHitScanSnapshot;
	TestTrue(TEXT("HitScan 후 Ammo Snapshot"), VehicleAmmoComp->TryGetActiveWeaponAmmoSnapshot(VehicleWeaponComp, AfterHitScanSnapshot));
	TestEqual(TEXT("Trace Miss도 장전량 2에서 1로 소비"), AfterHitScanSnapshot.LoadedAmmoCount, 1);
	TestEqual(TEXT("Trace Miss 후 차량 전체 보유량 2"), AfterHitScanSnapshot.CurrentOnboardAmmoCount, 2);
	TestEqual(TEXT("출격 실제 소비 누계 1"), AfterHitScanSnapshot.FiredAmmoCountThisSortie, 1);

	// [v1.0.0] Pool 실행 실패와 Direct fallback을 같은 WeaponData에서 재현할 Transient ProjectileData입니다.
	UCFProjectileData* ProjectileData = NewObject<UCFProjectileData>(GetTransientPackage());
	ProjectileData->ProjectileId = TEXT("AmmoP003_TestProjectile");
	ProjectileData->ProjectileActorClass = ACFProjectileActor::StaticClass();
	ProjectileData->InitialSpeed = 2000.0f;
	WeaponData->FireMode = ECFWeaponFireMode::Projectile;
	WeaponData->DefaultProjectileData = ProjectileData;
	TestTrue(TEXT("Projectile Weapon Runtime 재초기화"), VehicleWeaponComp->InitializeWeaponRuntime(VehiclePawn, VehicleData));
	TestTrue(TEXT("Projectile Actor 실행 준비"), VehicleWeaponComp->IsActiveProjectileSpawnReady());

	// [v1.0.0] Projectile 실제 실행 실패를 결정적으로 만들기 위해 Pawn의 Pool 참조만 테스트 범위에서 비웁니다.
	VehiclePawn->ProjectilePoolComp = nullptr;

	// [v1.0.0] fallback을 금지해 Pool 없음이 실제 Projectile 실행 실패로 끝나는 요청입니다.
	const FCFVehicleFireRequest FailedProjectileRequest = BuildAmmoFireTestRequest(2);

	// [v1.0.0] Projectile 실행 직전 Validate 승인 상태를 재현하는 결과입니다.
	FCFVehicleFireResult FailedProjectileResult = BuildAcceptedAmmoFireResult(FailedProjectileRequest);
	TestFalse(TEXT("Pool 없는 Projectile 실행 실패"), VehiclePawn->ExecuteAcceptedFireCommand(FailedProjectileRequest, FailedProjectileResult, nullptr, false));
	TestFalse(TEXT("Projectile 실행 실패는 FireResult 거부"), FailedProjectileResult.bAccepted);

	// [v1.0.0] 실패한 Projectile 예약이 Rollback되어 장전량과 차량 보유량이 보존됐는지 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot AfterRollbackSnapshot;
	TestTrue(TEXT("Rollback 후 Ammo Snapshot"), VehicleAmmoComp->TryGetActiveWeaponAmmoSnapshot(VehicleWeaponComp, AfterRollbackSnapshot));
	TestEqual(TEXT("Projectile 실패 후 장전량 1 유지"), AfterRollbackSnapshot.LoadedAmmoCount, 1);
	TestEqual(TEXT("Projectile 실패 후 즉시 사용 가능 1 복구"), AfterRollbackSnapshot.ImmediateUsableAmmoCount, 1);
	TestEqual(TEXT("Projectile 실패 후 차량 전체 보유량 2 유지"), AfterRollbackSnapshot.CurrentOnboardAmmoCount, 2);
	TestEqual(TEXT("Projectile 실패는 소비 누계 증가 없음"), AfterRollbackSnapshot.FiredAmmoCountThisSortie, 1);

	// [v1.0.0] 같은 Direct Projectile 실패를 기존 Dummy HitScan fallback 성공으로 처리할 세 번째 요청입니다.
	const FCFVehicleFireRequest DirectFallbackRequest = BuildAmmoFireTestRequest(3);

	// [v1.0.0] Direct fallback 실행 직전 Validate 승인 상태를 재현하는 결과입니다.
	FCFVehicleFireResult DirectFallbackResult = BuildAcceptedAmmoFireResult(DirectFallbackRequest);
	TestTrue(TEXT("Direct Projectile Pool 실패 fallback 성공"), VehiclePawn->ExecuteAcceptedFireCommand(DirectFallbackRequest, DirectFallbackResult, nullptr, true));
	TestTrue(TEXT("Direct fallback은 승인 발사 유지"), DirectFallbackResult.bAccepted);

	// [v1.0.0] Direct fallback HitScan이 실제 한 발을 소비해 탄창이 비었는지 확인할 Snapshot입니다.
	FCFAmmoRuntimeSnapshot AfterFallbackSnapshot;
	TestTrue(TEXT("Direct fallback 후 Ammo Snapshot"), VehicleAmmoComp->TryGetActiveWeaponAmmoSnapshot(VehicleWeaponComp, AfterFallbackSnapshot));
	TestEqual(TEXT("Direct fallback 성공 후 장전량 0"), AfterFallbackSnapshot.LoadedAmmoCount, 0);
	TestEqual(TEXT("두 번의 실제 발사로 소비 누계 2"), AfterFallbackSnapshot.FiredAmmoCountThisSortie, 2);
	TestEqual(TEXT("예비 1발은 아직 차량에 보존"), AfterFallbackSnapshot.ReserveAmmoCount, 1);
	TestEqual(TEXT("탄창 발사 불가·예비 존재 상태는 Empty"), AfterFallbackSnapshot.ReloadState, ECFWeaponReloadState::Empty);

	// [v1.0.0] Reload가 아직 실행되지 않은 AMMO-P0-03에서 빈 탄창 발사를 거부하는 최종 검증 결과입니다.
	const ECFAmmoTransactionResult NoAmmoResult = VehicleAmmoComp->ValidateSingleFireAmmo(VehicleWeaponComp->GetActiveMountProfileId());
	TestEqual(TEXT("빈 탄창은 NotEnoughLoadedAmmo"), NoAmmoResult, ECFAmmoTransactionResult::NotEnoughLoadedAmmo);

	VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
