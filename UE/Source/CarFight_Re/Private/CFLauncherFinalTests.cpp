// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-11
// Description: CF-FQ-029 LM-P0-06 최종 Launcher Release 통합 자동화 테스트
// Scope: 실제 VehicleFireComp 실행 경로에서 Angled/Vertical Ejection, Carrier Velocity 상속, 비Direct MuzzleBlocked와 Pool LaunchContext 전달을 검증합니다.
// Changelog:
// - v1.0.0: transient Vehicle/Weapon/Projectile과 TestTarget을 사용한 LM-P0-06 최종 통합 회귀를 추가.
// Migration:
// - Product DataAsset/Blueprint를 수정하거나 저장하지 않습니다.
// - 저장 DA_RocketLauncher의 Direct/EjectionSpeed 0/CarrierVelocityRatio 0 복구 상태는 AssetDump persisted evidence가 별도로 소유합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFEquipmentPresetData.h"
#include "CFMissileTestTarget.h"
#include "CFProjectileActor.h"
#include "CFProjectileData.h"
#include "CFProjectilePoolComp.h"
#include "CFVehicleData.h"
#include "CFVehicleFireComp.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFLauncherFinalIntegrationTest,
	"CarFight.Launcher.LM_P0_06.FinalIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace
{
	// [v1.0.0] 실제 VehicleFireComp 실행 경로에 연결할 transient Launcher VehicleData와 WeaponData를 구성합니다.
	UCFVehicleData* CreateLauncherIntegrationVehicleData(
		UObject* OuterObject,
		UCFProjectileData* ProjectileData,
		UCFWeaponData*& OutWeaponData)
	{
		OutWeaponData = nullptr;
		if (!OuterObject || !ProjectileData)
		{
			return nullptr;
		}

		// [v1.0.0] LM-P0-06 Release 설정을 시나리오별로 바꿀 transient WeaponData입니다.
		UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(OuterObject, TEXT("LauncherFinalWeapon"));
		WeaponData->WeaponId = TEXT("LauncherFinalWeapon");
		WeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
		WeaponData->CompatibleMountTypes.Reset();
		WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
		WeaponData->FireMode = ECFWeaponFireMode::Projectile;
		WeaponData->DefaultProjectileData = ProjectileData;
		WeaponData->bUseInfiniteAmmoForDebug = true;
		WeaponData->LauncherFirePatternConfig.FirePattern = ECFLauncherFirePattern::SingleCycle;

		// [v1.0.0] transient WeaponData를 실제 VehicleWeaponComp 활성 장비로 연결할 EquipmentPreset입니다.
		UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>(OuterObject, TEXT("LauncherFinalPreset"));
		EquipmentPresetData->EquipmentId = TEXT("LauncherFinalPreset");
		EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
		EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Large;
		EquipmentPresetData->DefaultWeaponData = WeaponData;

		// [v1.0.0] VehicleWeaponComp의 기본 활성 MountProfile을 제공할 transient VehicleData입니다.
		UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(OuterObject, TEXT("LauncherFinalVehicleData"));

		// [v1.0.0] FireRequest의 WeaponGroupId가 참조할 최소 Hardpoint 슬롯입니다.
		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = TEXT("Top_01");
		HardpointSlot.LocationCategory = TEXT("Top");
		HardpointSlot.LocalLocation = FVector::ZeroVector;
		VehicleData->HardpointSlots.Add(HardpointSlot);

		// [v1.0.0] VehicleWeaponComp 기본 활성 ID와 맞는 transient Turret MountProfile입니다.
		FCFVehicleMountProfile MountProfile;
		MountProfile.MountProfileId = TEXT("RoofTurret_MediumOrLarge");
		MountProfile.LocationSlotRef = HardpointSlot.LocationSlotId;
		MountProfile.MountType = ECFVehicleMountType::Turret;
		MountProfile.SizeLimit = ECFVehicleWeaponSize::Large;
		MountProfile.DefaultEquipmentPresetData = EquipmentPresetData;
		VehicleData->MountProfiles.Add(MountProfile);

		OutWeaponData = WeaponData;
		return VehicleData;
	}

	// [v1.0.0] 실제 Pool에서 현재 발사 차량이 활성화한 Projectile을 찾습니다.
	ACFProjectileActor* FindActiveLauncherProjectile(UWorld* TestWorld, AActor* InstigatorActor)
	{
		if (!TestWorld || !InstigatorActor)
		{
			return nullptr;
		}

		// [v1.0.0] 현재 World의 Projectile Actor를 순회하는 반복자입니다.
		for (TActorIterator<ACFProjectileActor> ProjectileActorIterator(TestWorld); ProjectileActorIterator; ++ProjectileActorIterator)
		{
			// [v1.0.0] 현재 순회 중인 Projectile Actor입니다.
			ACFProjectileActor* CandidateProjectileActor = *ProjectileActorIterator;
			if (IsValid(CandidateProjectileActor)
				&& CandidateProjectileActor->IsProjectileActive()
				&& CandidateProjectileActor->GetActiveInstigatorActor() == InstigatorActor)
			{
				return CandidateProjectileActor;
			}
		}
		return nullptr;
	}

	// [v1.0.0] Validate 단계 직후와 같은 승인 상태의 FireResult를 구성합니다.
	FCFVehicleFireResult BuildAcceptedLauncherFireResult(const FCFVehicleFireRequest& FireRequest)
	{
		// [v1.0.0] 실제 ExecuteAcceptedFireCommand에 전달할 승인 결과입니다.
		FCFVehicleFireResult FireResult;
		FireResult.FireRequestId = FireRequest.FireRequestId;
		FireResult.ValidationAimTargetLocation = FireRequest.PredictedAimTargetLocation;
		FireResult.LocalHitLocation = FireRequest.PredictedAimTargetLocation;
		FireResult.bAccepted = true;
		FireResult.RejectReason = ECFVehicleFireRejectReason::None;
		return FireResult;
	}
}

// [v1.0.0] Angled/Vertical Release, 실제 Carrier Velocity 스냅샷, MuzzleBlocked와 Pool Context 인계를 통합 검증합니다.
bool FCFLauncherFinalIntegrationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 실제 VehicleFireComp, Collision Trace와 ProjectilePool을 실행할 transient Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("LM-P0-06 transient World가 생성돼야 함"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 실제 Pool에서 생성할 기본 ProjectileData입니다.
	UCFProjectileData* ProjectileData = NewObject<UCFProjectileData>(TestWorld, TEXT("LauncherFinalProjectileData"));
	if (!TestNotNull(TEXT("LM-P0-06 transient ProjectileData가 생성돼야 함"), ProjectileData))
	{
		return false;
	}
	ProjectileData->ProjectileActorClass = ACFProjectileActor::StaticClass();
	ProjectileData->InitialSpeed = 700.0f;
	ProjectileData->LifeTimeSeconds = 10.0f;
	ProjectileData->bAffectedByGravity = false;
	ProjectileData->bUseSupplementalContinuousSweep = false;
	ProjectileData->PropulsionConfig.bUsePropulsion = false;

	// [v1.0.0] 실제 Fire/Weapon/Pool 기본 서브오브젝트를 제공할 transient 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector(0.0f, 0.0f, 2000.0f),
		FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("LM-P0-06 transient Vehicle Pawn이 생성돼야 함"), VehiclePawn))
	{
		return false;
	}

	// [v1.0.0] Release 설정을 소유할 transient WeaponData입니다.
	UCFWeaponData* WeaponData = nullptr;

	// [v1.0.0] 실제 VehicleWeaponComp 초기화에 사용할 transient VehicleData입니다.
	UCFVehicleData* VehicleData = CreateLauncherIntegrationVehicleData(VehiclePawn, ProjectileData, WeaponData);

	// [v1.0.0] transient VehicleData를 활성 Weapon Runtime으로 해석할 실제 VehicleWeaponComp입니다.
	UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();

	// [v1.0.0] LaunchContext 작성과 비Direct MuzzleBlocked Trace를 실행할 실제 VehicleFireComp입니다.
	UCFVehicleFireComp* VehicleFireComp = VehiclePawn->FindComponentByClass<UCFVehicleFireComp>();

	// [v1.0.0] 실제 Projectile Actor 생성·재사용과 활성 수를 제공할 Pool 컴포넌트입니다.
	UCFProjectilePoolComp* ProjectilePoolComp = VehiclePawn->FindComponentByClass<UCFProjectilePoolComp>();

	// [v1.0.0] 실제 차량 속도 스냅샷을 제공할 Chaos Vehicle Movement 컴포넌트입니다.
	UChaosWheeledVehicleMovementComponent* VehicleMovementComp = Cast<UChaosWheeledVehicleMovementComponent>(VehiclePawn->GetVehicleMovementComponent());
	if (!TestNotNull(TEXT("LM-P0-06 transient VehicleData"), VehicleData)
		|| !TestNotNull(TEXT("LM-P0-06 transient WeaponData"), WeaponData)
		|| !TestNotNull(TEXT("LM-P0-06 VehicleWeaponComp"), VehicleWeaponComp)
		|| !TestNotNull(TEXT("LM-P0-06 VehicleFireComp"), VehicleFireComp)
		|| !TestNotNull(TEXT("LM-P0-06 ProjectilePoolComp"), ProjectilePoolComp)
		|| !TestNotNull(TEXT("LM-P0-06 Chaos Vehicle Movement"), VehicleMovementComp))
	{
		VehiclePawn->Destroy();
		return false;
	}

	TestTrue(TEXT("LM-P0-06 transient Weapon Runtime 초기화"), VehicleWeaponComp->InitializeWeaponRuntime(VehiclePawn, VehicleData));
	TestTrue(TEXT("LM-P0-06 Projectile Spawn 준비"), VehicleWeaponComp->IsActiveProjectileSpawnReady());
	TestEqual(TEXT("LM-P0-06 활성 ProjectileData 연결"), VehicleWeaponComp->GetActiveProjectileData(), ProjectileData);

	// [v1.0.0] Angled Ejection과 Carrier Velocity를 함께 검증할 실제 차량 이동 속도입니다.
	const FVector CarrierWorldVelocity(600.0f, 200.0f, 0.0f);
	VehicleMovementComp->Velocity = CarrierWorldVelocity;
	TestTrue(TEXT("VehiclePawn GetVelocity가 테스트 Carrier Velocity를 반환"), VehiclePawn->GetVelocity().Equals(CarrierWorldVelocity, 0.1f));

	// [v1.0.0] Angled Ejection에서 월드 대각 상향으로 사용할 로컬 사출 방향입니다.
	const FVector AngledLocalEjectionDirection(1.0f, 0.0f, 1.0f);

	// [v1.0.0] 실제 Release 방향 속력으로 사용할 Angled Ejection 속도입니다.
	constexpr float AngledEjectionSpeedCmPerSec = 1200.0f;

	// [v1.0.0] 실제 차량 속도의 절반을 초기 월드 Velocity에 상속할 비율입니다.
	constexpr float CarrierVelocityRatio = 0.5f;

	WeaponData->LauncherReleaseConfig.ReleaseMode = ECFProjectileReleaseMode::AngledEjection;
	WeaponData->LauncherReleaseConfig.LocalEjectionDirection = AngledLocalEjectionDirection;
	WeaponData->LauncherReleaseConfig.EjectionSpeed = AngledEjectionSpeedCmPerSec;
	WeaponData->LauncherReleaseConfig.CarrierVelocityRatio = CarrierVelocityRatio;
	WeaponData->LauncherReleaseConfig.LauncherClearanceTraceDistanceCm = 300.0f;

	// [v1.0.0] 실제 FireComp가 Angled LaunchContext를 만들 때 사용할 FireRequest입니다.
	FCFVehicleFireRequest AngledFireRequest;
	AngledFireRequest.FireRequestId = 290601;
	AngledFireRequest.AimOrigin = FVector(0.0f, 0.0f, 2000.0f);
	AngledFireRequest.AimDirection = FVector::ForwardVector;
	AngledFireRequest.PredictedAimTargetLocation = FVector(10000.0f, 0.0f, 2000.0f);
	AngledFireRequest.ClientFireTimeSeconds = 29.0601f;
	AngledFireRequest.WeaponGroupId = TEXT("RoofTurret_MediumOrLarge");

	// [v1.0.0] FireComp가 실제 ReleaseConfig와 Vehicle GetVelocity를 사용해 작성한 Angled LaunchContext입니다.
	FCFProjectileLaunchContext AngledLaunchContext;
	TestTrue(
		TEXT("Angled LaunchContext 작성 성공"),
		VehicleFireComp->BuildDirectProjectileLaunchContext(
			AngledFireRequest,
			*ProjectileData,
			nullptr,
			AngledLaunchContext));

	// [v1.0.0] Muzzle Socket이 없는 transient 경로에서 예상되는 정규화 Angled 초기 방향입니다.
	const FVector ExpectedAngledDirection = AngledLocalEjectionDirection.GetSafeNormal();

	// [v1.0.0] 실제 Vehicle GetVelocity에 CarrierVelocityRatio를 적용한 기대 상속 속도입니다.
	const FVector ExpectedInheritedCarrierVelocity = CarrierWorldVelocity * CarrierVelocityRatio;

	// [v1.0.0] Ejection 속도와 Carrier 상속을 합친 기대 초기 월드 Velocity입니다.
	const FVector ExpectedAngledInitialVelocity = ExpectedAngledDirection * AngledEjectionSpeedCmPerSec + ExpectedInheritedCarrierVelocity;

	TestEqual(TEXT("Angled ReleaseMode가 LaunchContext에 보존"), AngledLaunchContext.ReleaseMode, ECFProjectileReleaseMode::AngledEjection);
	TestTrue(TEXT("Angled 초기 방향이 실제 ReleaseConfig와 동일"), AngledLaunchContext.InitialLaunchDirection.Equals(ExpectedAngledDirection, 0.001f));
	TestTrue(TEXT("Carrier Velocity 상속값이 LaunchContext에 보존"), AngledLaunchContext.InheritedCarrierVelocity.Equals(ExpectedInheritedCarrierVelocity, 0.1f));
	TestTrue(TEXT("Ejection + Carrier 초기 Velocity가 LaunchContext에 보존"), AngledLaunchContext.InitialLaunchVelocity.Equals(ExpectedAngledInitialVelocity, 0.1f));

	// [v1.0.0] 실제 Angled 초기 사출 방향 180cm 앞에서 WeaponHit Trace를 막을 non-damageable 테스트 Actor입니다.
	ACFMissileTestTarget* MuzzleBlockerActor = TestWorld->SpawnActor<ACFMissileTestTarget>();
	if (!TestNotNull(TEXT("MuzzleBlocked 검증 Actor가 생성돼야 함"), MuzzleBlockerActor))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.0.0] MuzzleBlocker 중심을 실제 Trace 방향 안쪽에 배치할 월드 위치입니다.
	const FVector MuzzleBlockerLocation = FVector(AngledFireRequest.AimOrigin) + ExpectedAngledDirection * 180.0f;
	MuzzleBlockerActor->SetActorLocation(MuzzleBlockerLocation);
	MuzzleBlockerActor->bBlockProjectileHits = false;
	MuzzleBlockerActor->RefreshTestTargetConfig();

	// [v1.0.0] MuzzleBlocked 실행 전 Pool 활성 수 기준선입니다.
	const int32 ActiveProjectileCountBeforeBlockedShot = ProjectilePoolComp->GetActivePooledProjectileCount();

	// [v1.0.0] 실제 ExecuteAcceptedFireCommand가 MuzzleBlocked로 바꿔야 할 승인 결과입니다.
	FCFVehicleFireResult BlockedFireResult = BuildAcceptedLauncherFireResult(AngledFireRequest);

	// [v1.0.0] 비Direct 실제 사출 경로가 막힌 경우 실행이 거부됐는지 여부입니다.
	const bool bBlockedShotExecuted = VehicleFireComp->ExecuteAcceptedFireCommand(
		AngledFireRequest,
		BlockedFireResult,
		nullptr,
		false);
	TestFalse(TEXT("비Direct 실제 사출 경로가 막히면 발사 실행 거부"), bBlockedShotExecuted);
	TestFalse(TEXT("MuzzleBlocked 결과는 Accepted=false"), BlockedFireResult.bAccepted);
	TestEqual(TEXT("비Direct 실제 사출 경로 RejectReason은 MuzzleBlocked"), BlockedFireResult.RejectReason, ECFVehicleFireRejectReason::MuzzleBlocked);
	TestEqual(TEXT("MuzzleBlocked에서는 Pool 활성 Projectile이 증가하지 않음"), ProjectilePoolComp->GetActivePooledProjectileCount(), ActiveProjectileCountBeforeBlockedShot);

	MuzzleBlockerActor->SetActorEnableCollision(false);
	MuzzleBlockerActor->Destroy();

	// [v1.0.0] 장애물 제거 뒤 실제 Angled 발사가 성공해야 할 승인 결과입니다.
	FCFVehicleFireResult AngledFireResult = BuildAcceptedLauncherFireResult(AngledFireRequest);
	TestTrue(
		TEXT("장애물 제거 뒤 Angled ExecuteAcceptedFireCommand 성공"),
		VehicleFireComp->ExecuteAcceptedFireCommand(
			AngledFireRequest,
			AngledFireResult,
			nullptr,
			false));
	TestTrue(TEXT("Angled 실제 발사 뒤 FireResult 승인 유지"), AngledFireResult.bAccepted);
	TestEqual(TEXT("Angled 실제 발사 뒤 Pool 활성 수 1"), ProjectilePoolComp->GetActivePooledProjectileCount(), 1);

	// [v1.0.0] 실제 FireComp→Pool 경로가 활성화한 Angled Projectile Actor입니다.
	ACFProjectileActor* AngledProjectileActor = FindActiveLauncherProjectile(TestWorld, VehiclePawn);
	if (!TestNotNull(TEXT("Angled 실제 발사 Projectile을 찾아야 함"), AngledProjectileActor))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.0.0] Pool Actor가 값으로 복사해 보유한 실제 Angled LaunchContext입니다.
	const FCFProjectileLaunchContext ActiveAngledContext = AngledProjectileActor->GetActiveLaunchContext();
	TestEqual(TEXT("Pool Actor Angled ReleaseMode 보존"), ActiveAngledContext.ReleaseMode, ECFProjectileReleaseMode::AngledEjection);
	TestTrue(TEXT("Pool Actor Angled 초기 방향 보존"), ActiveAngledContext.InitialLaunchDirection.Equals(ExpectedAngledDirection, 0.001f));
	TestTrue(TEXT("Pool Actor Carrier 상속 보존"), ActiveAngledContext.InheritedCarrierVelocity.Equals(ExpectedInheritedCarrierVelocity, 0.1f));
	TestTrue(TEXT("Pool Actor 최종 초기 Velocity 보존"), ActiveAngledContext.InitialLaunchVelocity.Equals(ExpectedAngledInitialVelocity, 0.1f));
	AngledProjectileActor->DeactivateProjectile();
	TestEqual(TEXT("Angled Projectile 반환 뒤 Pool 활성 수 0"), ProjectilePoolComp->GetActivePooledProjectileCount(), 0);

	VehicleMovementComp->Velocity = FVector::ZeroVector;
	WeaponData->LauncherReleaseConfig.ReleaseMode = ECFProjectileReleaseMode::VerticalEjection;
	WeaponData->LauncherReleaseConfig.LocalEjectionDirection = FVector(0.0f, 1.0f, 0.0f);
	WeaponData->LauncherReleaseConfig.EjectionSpeed = 900.0f;
	WeaponData->LauncherReleaseConfig.CarrierVelocityRatio = 0.0f;
	WeaponData->LauncherReleaseConfig.LauncherClearanceTraceDistanceCm = 0.0f;

	// [v1.0.0] Vertical Release가 command aim과 같은 fallback Muzzle +X를 실제 Pool까지 전달할 FireRequest입니다.
	FCFVehicleFireRequest VerticalFireRequest;
	VerticalFireRequest.FireRequestId = 290602;
	VerticalFireRequest.AimOrigin = FVector(0.0f, 0.0f, 2000.0f);
	VerticalFireRequest.AimDirection = FVector::RightVector;
	VerticalFireRequest.PredictedAimTargetLocation = FVector(0.0f, 10000.0f, 2000.0f);
	VerticalFireRequest.ClientFireTimeSeconds = 29.0602f;
	VerticalFireRequest.WeaponGroupId = TEXT("RoofTurret_MediumOrLarge");

	// [v1.0.0] 실제 Vertical 발사 실행에 전달할 승인 FireResult입니다.
	FCFVehicleFireResult VerticalFireResult = BuildAcceptedLauncherFireResult(VerticalFireRequest);
	TestTrue(
		TEXT("Vertical ExecuteAcceptedFireCommand 성공"),
		VehicleFireComp->ExecuteAcceptedFireCommand(
			VerticalFireRequest,
			VerticalFireResult,
			nullptr,
			false));
	TestTrue(TEXT("Vertical 실제 발사 뒤 FireResult 승인 유지"), VerticalFireResult.bAccepted);
	TestEqual(TEXT("Vertical 실제 발사 뒤 Pool 활성 수 1"), ProjectilePoolComp->GetActivePooledProjectileCount(), 1);

	// [v1.0.0] 실제 FireComp→Pool 경로가 활성화한 Vertical Projectile Actor입니다.
	ACFProjectileActor* VerticalProjectileActor = FindActiveLauncherProjectile(TestWorld, VehiclePawn);
	if (!TestNotNull(TEXT("Vertical 실제 발사 Projectile을 찾아야 함"), VerticalProjectileActor))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.0.0] Pool Actor가 값으로 복사해 보유한 실제 Vertical LaunchContext입니다.
	const FCFProjectileLaunchContext ActiveVerticalContext = VerticalProjectileActor->GetActiveLaunchContext();
	TestEqual(TEXT("Pool Actor Vertical ReleaseMode 보존"), ActiveVerticalContext.ReleaseMode, ECFProjectileReleaseMode::VerticalEjection);
	TestTrue(TEXT("Vertical fallback Muzzle +X 방향 보존"), ActiveVerticalContext.InitialLaunchDirection.Equals(FVector::RightVector, 0.001f));
	TestTrue(TEXT("Vertical EjectionSpeed 900 적용"), ActiveVerticalContext.InitialLaunchVelocity.Equals(FVector::RightVector * 900.0f, 0.1f));
	TestTrue(TEXT("Vertical CarrierVelocityRatio 0이면 상속값 0"), ActiveVerticalContext.InheritedCarrierVelocity.IsNearlyZero(0.1f));
	VerticalProjectileActor->DeactivateProjectile();
	TestEqual(TEXT("Vertical Projectile 반환 뒤 Pool 활성 수 0"), ProjectilePoolComp->GetActivePooledProjectileCount(), 0);

	// [v1.0.0] transient 테스트 설정을 production 기본 Release 계약으로 되돌린 최종 상태입니다.
	WeaponData->LauncherReleaseConfig = FCFLauncherReleaseConfig();
	TestEqual(TEXT("transient 기본 복구 ReleaseMode는 Direct"), WeaponData->LauncherReleaseConfig.ReleaseMode, ECFProjectileReleaseMode::Direct);
	TestEqual(TEXT("transient 기본 복구 EjectionSpeed는 0"), WeaponData->LauncherReleaseConfig.EjectionSpeed, 0.0f);
	TestEqual(TEXT("transient 기본 복구 CarrierVelocityRatio는 0"), WeaponData->LauncherReleaseConfig.CarrierVelocityRatio, 0.0f);

	VehiclePawn->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
