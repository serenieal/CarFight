// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-09-07
// Description: CF-FQ-030 MG-P0-01~04 Direct 미사일 Runtime 자동화 테스트
// Scope: 정지·측면 이동·Target Snapshot·목표 파괴·오버슈트·동일 Actor Pool 재사용과 저장 DirectTest의 Accepted Fire→Pool→실제 ActorClass, production ProjectileMovement Blocking 접촉, Automation bridge 기반 Supplemental Sweep→Damage를 검증합니다.
// Changelog:
// - v1.3.0: 저장 DA_Missile_DirectTest를 실제 VehicleFireComp Accepted 실행 경계에 연결해 FireComp → ProjectilePool → 저장 ProjectileActorClass를 검증하고, CreateNewMap Automation에서 production ProjectileMovement가 실제 Blocking 접촉까지 이동함을 확인한 뒤 누락되는 swept Hit dispatch만 테스트 전용 bridge로 보완해 production Supplemental Sweep → Impact → VehicleHealth 피해를 검증.
// - v1.2.0: 저장 DA_Missile_DirectTest를 읽어 보조 연속 Sweep → 실제 Impact → VehicleHealth 피해 연결을 검증하는 Content Integration 시나리오를 추가.
// - v1.1.0: 정지 목표, 측면 이동, 오버슈트와 동일 Projectile Actor 재활성화 Pool 계약 검증을 추가.
// - v1.0.0: Direct Missile Runtime Contract 최초 추가.
// Migration:
// - 시나리오 1~6은 외부 Blueprint·DataAsset 없이 C++ 임시 ProjectileData로 런타임 계약을 검증합니다.
// - 시나리오 7은 저장 DA_Missile_DirectTest를 읽기 전용으로 로드하고 transient Vehicle/Weapon 설정만 사용하며 Content Asset을 수정·저장하지 않습니다.
// - v1.3.0 시나리오 7의 production ProjectileMovement 접촉 판정은 위치 순간이동 없이 수행합니다. 다만 CreateNewMap Automation이 swept OnComponentHit dispatch를 PIE처럼 제공하지 않아, 접촉 판정 뒤 최종 Impact/Damage 경로만 테스트 전용 위치 bridge로 production Supplemental Sweep에 인계합니다.
// - 따라서 시나리오 7을 전체 입력→명중 또는 전 구간 무텔레포트 E2E 증거로 해석하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFEquipmentPresetData.h"
#include "CFMissileFlightComp.h"
#include "CFMissileGuideComp.h"
#include "CFMissileTestTarget.h"
#include "CFProjectileActor.h"
#include "CFProjectileData.h"
#include "CFProjectileMotorComp.h"
#include "CFProjectilePoolComp.h"
#include "CFVehicleData.h"
#include "CFVehicleFireComp.h"
#include "CFVehicleHealthComp.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFMissileRuntimeContractTest,
	"CarFight.Missile.MG_P0_01_04.DirectRuntimeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace
{
	// [v1.1.0] Direct TargetActor 미사일 Automation에서 공통 사용할 임시 ProjectileData를 구성합니다.
	UCFProjectileData* BuildDirectMissileTestData(UObject* OuterObject)
	{
		// [v1.1.0] 각 시나리오가 공유할 임시 Direct 미사일 데이터입니다.
		UCFProjectileData* MissileProjectileData = NewObject<UCFProjectileData>(OuterObject);
		MissileProjectileData->InitialSpeed = 1000.0f;
		MissileProjectileData->LifeTimeSeconds = 10.0f;
		MissileProjectileData->bAffectedByGravity = false;
		MissileProjectileData->bUseSupplementalContinuousSweep = false;
		MissileProjectileData->PropulsionConfig.bUsePropulsion = true;
		MissileProjectileData->PropulsionConfig.IgnitionDelaySeconds = 0.0f;
		MissileProjectileData->PropulsionConfig.BurnDurationSeconds = 2.0f;
		MissileProjectileData->PropulsionConfig.ThrustAccelerationCmPerSecSq = 1000.0f;
		MissileProjectileData->PropulsionConfig.MaximumPropelledSpeed = 3000.0f;
		MissileProjectileData->MissileFlightConfig.bUseMissileFlight = true;
		MissileProjectileData->MissileFlightConfig.AttackProfile = ECFMissileAttackProfile::Direct;
		MissileProjectileData->MissileFlightConfig.MinimumClearanceTimeSeconds = 0.1f;
		MissileProjectileData->MissileFlightConfig.MinimumClearanceDistanceCm = 100.0f;
		MissileProjectileData->MissileGuideConfig.bUseGuidance = true;
		MissileProjectileData->MissileGuideConfig.GuideMode = ECFMissileGuideMode::TargetActor;
		MissileProjectileData->MissileGuideConfig.LostTargetPolicy = ECFMissileLostTargetPolicy::ContinueStraight;
		MissileProjectileData->MissileGuideConfig.MaximumTurnRateDegPerSec = 45.0f;
		MissileProjectileData->MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq = 4000.0f;
		MissileProjectileData->MissileGuideConfig.GuidanceResponseTimeSeconds = 0.01f;
		MissileProjectileData->MissileGuideConfig.MinimumGuidanceSpeedCmPerSec = 100.0f;
		MissileProjectileData->MissileGuideConfig.SeekerFieldOfViewDeg = 160.0f;
		MissileProjectileData->MissileGuideConfig.LockBreakAngleDeg = 100.0f;
		MissileProjectileData->MissileGuideConfig.TargetLostGraceTimeSeconds = 0.05f;
		return MissileProjectileData;
	}

	// [v1.1.0] 지정 목표와 초기 Velocity를 포함한 Direct Launch Context를 생성합니다.
	FCFProjectileLaunchContext BuildDirectMissileLaunchContext(
		AActor* GuidanceTargetActor,
		const FVector& InitialLaunchVelocity)
	{
		// [v1.1.0] 한 시나리오의 발사 순간 값만 보존할 Direct Launch Context입니다.
		FCFProjectileLaunchContext LaunchContext;
		LaunchContext.LaunchTransform = FTransform(FRotator::ZeroRotator, FVector::ZeroVector);
		LaunchContext.InitialLaunchDirection = InitialLaunchVelocity.GetSafeNormal();
		LaunchContext.InitialLaunchVelocity = InitialLaunchVelocity;
		LaunchContext.CommandTargetLocation = GuidanceTargetActor
			? GuidanceTargetActor->GetActorLocation()
			: FVector(10000.0f, 0.0f, 0.0f);
		LaunchContext.GuidanceTargetActor = GuidanceTargetActor;
		LaunchContext.ReleaseMode = ECFProjectileReleaseMode::Direct;
		return LaunchContext;
	}

	// [v1.2.0] 미사일 실제 Impact 피해를 관측할 transient VehicleHealthComp를 테스트 타겟에 부착합니다.
	UCFVehicleHealthComp* AttachMissileTestHealth(ACFMissileTestTarget* TargetActor)
	{
		if (!TargetActor)
		{
			return nullptr;
		}

		// [v1.2.0] 저장 에셋 변경 없이 실제 VehicleHealth 피해 경로를 받을 테스트 전용 Health 컴포넌트입니다.
		UCFVehicleHealthComp* HealthComponent = NewObject<UCFVehicleHealthComp>(TargetActor, TEXT("MissileDamageHealth"));
		TargetActor->AddInstanceComponent(HealthComponent);
		HealthComponent->RegisterComponent();
		HealthComponent->InitializeFromVehicleData(nullptr);
		return HealthComponent;
	}

	// [v1.3.0] 실제 Vehicle Fire 경로가 저장 ProjectileData를 활성 무기로 해석할 최소 transient VehicleData를 구성합니다.
	UCFVehicleData* CreateMissileFireIntegrationVehicleData(UObject* OuterObject, UCFProjectileData* PersistedProjectileData)
	{
		if (!OuterObject || !PersistedProjectileData)
		{
			return nullptr;
		}

		// [v1.3.0] 저장 ProjectileData를 실제 Projectile FireMode로 연결할 transient WeaponData입니다.
		UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(OuterObject, TEXT("MissileFireIntegrationWeapon"));
		WeaponData->WeaponId = TEXT("MissileFireIntegrationWeapon");
		WeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
		WeaponData->CompatibleMountTypes.Reset();
		WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
		WeaponData->FireMode = ECFWeaponFireMode::Projectile;
		WeaponData->DefaultProjectileData = PersistedProjectileData;
		WeaponData->bUseInfiniteAmmoForDebug = true;
		WeaponData->LauncherFirePatternConfig.FirePattern = ECFLauncherFirePattern::SingleCycle;

		// [v1.3.0] WeaponData를 실제 VehicleWeaponComp 활성 장비로 해석할 transient EquipmentPresetData입니다.
		UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>(OuterObject, TEXT("MissileFireIntegrationPreset"));
		EquipmentPresetData->EquipmentId = TEXT("MissileFireIntegrationPreset");
		EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
		EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Large;
		EquipmentPresetData->DefaultWeaponData = WeaponData;

		// [v1.3.0] VehicleWeaponComp가 기본 활성 MountProfile을 찾도록 제공할 transient VehicleData입니다.
		UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(OuterObject, TEXT("MissileFireIntegrationVehicleData"));

		// [v1.3.0] 실제 FireOrigin 해석용 최소 하드포인트 슬롯입니다.
		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = TEXT("Top_01");
		HardpointSlot.LocationCategory = TEXT("Top");
		HardpointSlot.LocalLocation = FVector::ZeroVector;
		VehicleData->HardpointSlots.Add(HardpointSlot);

		// [v1.3.0] VehicleWeaponComp 기본 활성 프로파일 ID와 일치하는 transient MountProfile입니다.
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

// [v1.3.0] Direct Missile의 여섯 Runtime 시나리오와 저장 DirectTest의 Accepted Fire→Pool→persisted ActorClass, production Blocking 접촉, Automation bridge 이후 Impact·Damage 통합을 검증합니다.
bool FCFMissileRuntimeContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 실제 Projectile Actor와 Target Actor를 생성할 Automation 월드입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Missile Runtime 테스트 월드가 생성돼야 함"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 모든 시나리오에서 Pool 재사용처럼 반복 활성화할 공통 Projectile Actor입니다.
	ACFProjectileActor* MissileActor = TestWorld->SpawnActor<ACFProjectileActor>();
	if (!TestNotNull(TEXT("Missile Projectile Actor가 생성돼야 함"), MissileActor))
	{
		return false;
	}
	MissileActor->SetDestroyWhenDeactivated(false);

	// [v1.1.0] 정지·측면 이동·파괴 시나리오에서 최초 Target Snapshot으로 사용할 타겟입니다.
	ACFMissileTestTarget* InitialTargetActor = TestWorld->SpawnActor<ACFMissileTestTarget>();

	// [v1.1.0] 발사 후 차량 선택 변경과 Pool 재사용 새 목표를 모사할 교체 타겟입니다.
	ACFMissileTestTarget* ReplacementTargetActor = TestWorld->SpawnActor<ACFMissileTestTarget>();

	// [v1.1.0] 오버슈트 전용 가까운 목표입니다.
	ACFMissileTestTarget* OvershootTargetActor = TestWorld->SpawnActor<ACFMissileTestTarget>();
	if (!TestNotNull(TEXT("초기 Target Actor가 생성돼야 함"), InitialTargetActor)
		|| !TestNotNull(TEXT("교체 Target Actor가 생성돼야 함"), ReplacementTargetActor)
		|| !TestNotNull(TEXT("오버슈트 Target Actor가 생성돼야 함"), OvershootTargetActor))
	{
		MissileActor->Destroy();
		return false;
	}

	InitialTargetActor->SetActorLocation(FVector(10000.0f, 0.0f, 0.0f));
	ReplacementTargetActor->SetActorLocation(FVector(10000.0f, -1500.0f, 0.0f));
	OvershootTargetActor->SetActorLocation(FVector(1000.0f, 0.0f, 0.0f));
	OvershootTargetActor->bBlockProjectileHits = false;
	OvershootTargetActor->RefreshTestTargetConfig();

	// [v1.0.0] Actor 기본 서브오브젝트로 생성돼야 하는 Missile Flight 컴포넌트입니다.
	UCFMissileFlightComp* MissileFlightComp = MissileActor->FindComponentByClass<UCFMissileFlightComp>();

	// [v1.0.0] Actor 기본 서브오브젝트로 생성돼야 하는 Missile Guide 컴포넌트입니다.
	UCFMissileGuideComp* MissileGuideComp = MissileActor->FindComponentByClass<UCFMissileGuideComp>();

	// [v1.0.0] 미사일 현재 방향 추진을 검증할 Projectile Motor 컴포넌트입니다.
	UCFProjectileMotorComp* ProjectileMotorComp = MissileActor->FindComponentByClass<UCFProjectileMotorComp>();

	// [v1.0.0] Guidance와 추진이 실제 Velocity를 공유할 ProjectileMovement입니다.
	UProjectileMovementComponent* ProjectileMovementComp = MissileActor->FindComponentByClass<UProjectileMovementComponent>();
	if (!TestNotNull(TEXT("MissileFlightComp 기본 서브오브젝트 존재"), MissileFlightComp)
		|| !TestNotNull(TEXT("MissileGuideComp 기본 서브오브젝트 존재"), MissileGuideComp)
		|| !TestNotNull(TEXT("ProjectileMotorComp 기본 서브오브젝트 존재"), ProjectileMotorComp)
		|| !TestNotNull(TEXT("ProjectileMovementComponent 기본 서브오브젝트 존재"), ProjectileMovementComp))
	{
		MissileActor->Destroy();
		return false;
	}

	// [v1.1.0] 모든 시나리오가 공유할 임시 Direct 미사일 데이터입니다.
	UCFProjectileData* MissileProjectileData = BuildDirectMissileTestData(MissileActor);

	// [v1.1.0] 시나리오 1~4에서 사용할 최초 Target Snapshot Direct 발사 Context입니다.
	FCFProjectileLaunchContext InitialLaunchContext = BuildDirectMissileLaunchContext(
		InitialTargetActor,
		FVector(1000.0f, 0.0f, 0.0f));

	MissileActor->ActivateProjectileWithContext(MissileProjectileData, InitialLaunchContext, nullptr);
	TestTrue(TEXT("미사일 Actor가 기존 Context 경로로 활성화"), MissileActor->IsProjectileActive());
	TestEqual(TEXT("미사일 Flight 최초 상태 Released"), MissileFlightComp->GetCurrentFlightState(), ECFMissileFlightState::Released);
		TestEqual(TEXT("발사 순간 Target Actor Snapshot 보존"), MissileGuideComp->GetGuidanceTargetActor(), static_cast<AActor*>(InitialTargetActor));

	InitialLaunchContext.GuidanceTargetActor = ReplacementTargetActor;
	TestEqual(TEXT("외부 Context 변경이 이미 발사된 미사일 Target을 바꾸지 않음"), MissileGuideComp->GetGuidanceTargetActor(), static_cast<AActor*>(InitialTargetActor));

	MissileActor->SetActorLocation(FVector(150.0f, 0.0f, 0.0f));
	MissileFlightComp->AdvanceFlightForAutomation(0.10f);
	TestEqual(TEXT("Direct 미사일은 Clearance 만족 뒤 GuidedFlight 진입"), MissileFlightComp->GetCurrentFlightState(), ECFMissileFlightState::GuidedFlight);
	TestTrue(TEXT("GuidedFlight에서 Guidance Window Open"), MissileFlightComp->IsGuidanceWindowOpen());

	// [v1.1.0] 시나리오 1 정지 정면 목표 Guidance 전 초기 Velocity입니다.
	const FVector StaticTargetVelocityBeforeGuidance = ProjectileMovementComp->Velocity;
	MissileGuideComp->AdvanceGuidanceForAutomation(0.05f);
	TestTrue(TEXT("정지 정면 목표는 불필요한 측면 조향을 만들지 않음"), FMath::Abs(ProjectileMovementComp->Velocity.Y) <= 0.1f);
	TestTrue(TEXT("정지 목표 Guidance는 기존 속력을 보존"), FMath::IsNearlyEqual(
		StaticTargetVelocityBeforeGuidance.Size(),
		ProjectileMovementComp->Velocity.Size(),
		0.1f));

	// [v1.1.0] 시나리오 2 Target이 측면으로 이동한 새 위치와 Velocity 추정값입니다.
	InitialTargetActor->SetActorLocation(FVector(10000.0f, 2200.0f, 0.0f));
	InitialTargetActor->SetTestVelocityForAutomation(FVector(0.0f, 1200.0f, 0.0f));
	MissileGuideComp->AdvanceGuidanceForAutomation(0.05f);
	TestTrue(TEXT("측면 이동 TargetActor Guidance가 Y Velocity 성분을 생성"), ProjectileMovementComp->Velocity.Y > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("측면 이동 Guidance 선회율이 설정 상한 이하"), MissileGuideComp->GetGuidanceCommand().AppliedTurnRateDegPerSec <= 45.0f + KINDA_SMALL_NUMBER);

	// [v1.0.0] 현재 방향 추진을 검증하기 위해 Guidance 후 Velocity를 Y축으로 고정한 값입니다.
	const float SpeedBeforeDynamicThrust = 1000.0f;
	ProjectileMovementComp->Velocity = FVector(0.0f, SpeedBeforeDynamicThrust, 0.0f);
	ProjectileMotorComp->AdvanceMotorForAutomation(0.10f);
	TestTrue(TEXT("미사일 모터는 현재 Velocity 방향으로 추진"), ProjectileMovementComp->Velocity.Y > SpeedBeforeDynamicThrust);
	TestTrue(TEXT("미사일 현재 방향 추진은 고정 X축 가속을 추가하지 않음"), FMath::Abs(ProjectileMovementComp->Velocity.X) <= KINDA_SMALL_NUMBER);

	// [v1.1.0] 시나리오 4 발사 후 목표 파괴를 모사합니다.
	InitialTargetActor->DestroyTestTargetNow();
	MissileGuideComp->AdvanceGuidanceForAutomation(0.10f);
	TestEqual(TEXT("목표 상실 유예 종료 뒤 ContinueStraight 사유 기록"), MissileGuideComp->GetGuideSnapshot().MissReason, ECFMissileMissReason::TargetLost);

	MissileActor->DeactivateProjectile();
	TestEqual(TEXT("첫 반환에서 Flight State Inactive Reset"), MissileFlightComp->GetCurrentFlightState(), ECFMissileFlightState::Inactive);
	TestNull(TEXT("첫 반환에서 Guidance Target Reset"), MissileGuideComp->GetGuidanceTargetActor());
	TestEqual(TEXT("첫 반환에서 Guidance Mode 기본값 Reset"), MissileGuideComp->GetGuideSnapshot().GuideMode, ECFMissileGuideMode::None);

	// [v1.1.0] 시나리오 5 오버슈트 판정을 위한 가까운 목표 Direct 발사 Context입니다.
	FCFProjectileLaunchContext OvershootLaunchContext = BuildDirectMissileLaunchContext(
		OvershootTargetActor,
		FVector(1000.0f, 0.0f, 0.0f));
	MissileActor->ActivateProjectileWithContext(MissileProjectileData, OvershootLaunchContext, nullptr);
	MissileActor->SetActorLocation(FVector(150.0f, 0.0f, 0.0f));
	MissileFlightComp->AdvanceFlightForAutomation(0.10f);
	MissileActor->SetActorLocation(FVector(900.0f, 0.0f, 0.0f));
	ProjectileMovementComp->Velocity = FVector(1000.0f, 0.0f, 0.0f);
	MissileGuideComp->AdvanceGuidanceForAutomation(0.02f);
	MissileActor->SetActorLocation(FVector(1500.0f, 0.0f, 0.0f));
	ProjectileMovementComp->Velocity = FVector(1000.0f, 0.0f, 0.0f);
	MissileGuideComp->AdvanceGuidanceForAutomation(0.02f);
	TestEqual(TEXT("가까운 목표를 지나 거리 증가·반대 방향이 되면 Overshoot 기록"), MissileGuideComp->GetGuideSnapshot().MissReason, ECFMissileMissReason::Overshoot);
	MissileActor->DeactivateProjectile();

	// [v1.1.0] 시나리오 6 Pool 재사용처럼 같은 Actor를 세 번째 활성화할 새 Target Context입니다.
	FCFProjectileLaunchContext ReuseLaunchContext = BuildDirectMissileLaunchContext(
		ReplacementTargetActor,
		FVector(1000.0f, 0.0f, 0.0f));
	MissileActor->ActivateProjectileWithContext(MissileProjectileData, ReuseLaunchContext, nullptr);
	TestTrue(TEXT("같은 Projectile Actor를 Pool 재사용처럼 재활성화 가능"), MissileActor->IsProjectileActive());
		TestEqual(TEXT("재활성화 Target은 이전 목표가 아닌 새 Snapshot"), MissileGuideComp->GetGuidanceTargetActor(), static_cast<AActor*>(ReplacementTargetActor));
	TestEqual(TEXT("재활성화 Flight State는 Released에서 다시 시작"), MissileFlightComp->GetCurrentFlightState(), ECFMissileFlightState::Released);
	TestTrue(TEXT("Motor 활성화 횟수는 세 시나리오 재사용을 반영"), ProjectileMotorComp->GetMotorSnapshot().MotorActivationCount >= 3);

	MissileActor->DeactivateProjectile();
	TestNull(TEXT("최종 Pool 반환에서도 Guidance Target Reset"), MissileGuideComp->GetGuidanceTargetActor());
	TestEqual(TEXT("최종 Pool 반환에서도 Flight Inactive"), MissileFlightComp->GetCurrentFlightState(), ECFMissileFlightState::Inactive);

	// [v1.3.0] 시나리오 7의 Accepted Fire→Pool→persisted ActorClass와 production Blocking 접촉, Automation bridge 이후 Impact·Damage를 받을 타겟입니다.
	ACFMissileTestTarget* DamageTargetActor = TestWorld->SpawnActor<ACFMissileTestTarget>();
	if (!TestNotNull(TEXT("Damage 통합 Target Actor가 생성돼야 함"), DamageTargetActor))
	{
		ReplacementTargetActor->Destroy();
		OvershootTargetActor->Destroy();
		MissileActor->Destroy();
		return false;
	}
	DamageTargetActor->SetActorLocation(FVector(1000.0f, 0.0f, 2000.0f));
	DamageTargetActor->bBlockProjectileHits = true;
	DamageTargetActor->RefreshTestTargetConfig();

	// [v1.3.0] 실제 Projectile Impact의 Legacy Health fallback을 받을 transient Health 컴포넌트입니다.
	UCFVehicleHealthComp* DamageTargetHealthComp = AttachMissileTestHealth(DamageTargetActor);
	if (!TestNotNull(TEXT("Damage 통합 Target에 VehicleHealthComp가 생성돼야 함"), DamageTargetHealthComp))
	{
		DamageTargetActor->Destroy();
		ReplacementTargetActor->Destroy();
		OvershootTargetActor->Destroy();
		MissileActor->Destroy();
		return false;
	}

	// [v1.3.0] 실제 Fire/Pool/ActorClass 통합에서 그대로 읽어 사용할 저장 DirectTest ProjectileData입니다.
	UCFProjectileData* PersistedMissileProjectileData = LoadObject<UCFProjectileData>(
		nullptr,
		TEXT("/Game/CarFight/Tests/Missile/DA_Missile_DirectTest.DA_Missile_DirectTest"));
	if (!TestNotNull(TEXT("저장 DA_Missile_DirectTest를 로드할 수 있어야 함"), PersistedMissileProjectileData))
	{
		DamageTargetActor->Destroy();
		ReplacementTargetActor->Destroy();
		OvershootTargetActor->Destroy();
		MissileActor->Destroy();
		return false;
	}

	TestTrue(TEXT("저장 DirectTest는 MissileFlight를 사용"), PersistedMissileProjectileData->MissileFlightConfig.bUseMissileFlight);
	TestTrue(TEXT("저장 DirectTest는 TargetActor Guidance를 사용"), PersistedMissileProjectileData->MissileGuideConfig.bUseGuidance
		&& PersistedMissileProjectileData->MissileGuideConfig.GuideMode == ECFMissileGuideMode::TargetActor);
	TestTrue(TEXT("저장 DirectTest는 보조 연속 Sweep을 사용"), PersistedMissileProjectileData->bUseSupplementalContinuousSweep);
	TestNotNull(TEXT("저장 DirectTest에 DefaultDamageData가 연결돼야 함"), PersistedMissileProjectileData->DefaultDamageData.Get());
	TestNotNull(TEXT("저장 DirectTest에 ProjectileActorClass가 연결돼야 함"), PersistedMissileProjectileData->ProjectileActorClass.Get());

	// [v1.3.0] 실제 FireComp와 ProjectilePoolComp 기본 서브오브젝트를 사용하는 transient 발사 차량의 Spawn 설정입니다.
	FActorSpawnParameters FireIntegrationVehicleSpawnParameters;
	FireIntegrationVehicleSpawnParameters.Name = TEXT("MissileFireIntegrationVehicle");

	// [v1.3.0] 저장 DirectTest를 실제 Vehicle Fire 경로에서 발사할 transient 차량 Pawn입니다.
	ACFVehiclePawn* FireIntegrationVehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector(0.0f, -5000.0f, 2000.0f),
		FRotator::ZeroRotator,
		FireIntegrationVehicleSpawnParameters);
	if (!TestNotNull(TEXT("Fire 통합 Vehicle Pawn이 생성돼야 함"), FireIntegrationVehiclePawn))
	{
		DamageTargetActor->Destroy();
		ReplacementTargetActor->Destroy();
		OvershootTargetActor->Destroy();
		MissileActor->Destroy();
		return false;
	}

	// [v1.3.0] 저장 ProjectileData를 활성 Weapon으로 해석할 실제 VehicleWeaponComp입니다.
	UCFVehicleWeaponComp* FireVehicleWeaponComp = FireIntegrationVehiclePawn->GetVehicleWeaponComp();

	// [v1.3.0] 실제 Pawn compatibility 실행이 위임할 VehicleFireComp입니다.
	UCFVehicleFireComp* FireVehicleFireComp = FireIntegrationVehiclePawn->FindComponentByClass<UCFVehicleFireComp>();

	// [v1.3.0] 저장 ProjectileActorClass를 실제 생성·재사용할 Vehicle ProjectilePoolComp입니다.
	UCFProjectilePoolComp* FireVehicleProjectilePoolComp = FireIntegrationVehiclePawn->FindComponentByClass<UCFProjectilePoolComp>();
	if (!TestNotNull(TEXT("Fire 통합 VehicleWeaponComp"), FireVehicleWeaponComp)
		|| !TestNotNull(TEXT("Fire 통합 VehicleFireComp"), FireVehicleFireComp)
		|| !TestNotNull(TEXT("Fire 통합 ProjectilePoolComp"), FireVehicleProjectilePoolComp))
	{
		FireIntegrationVehiclePawn->Destroy();
		DamageTargetActor->Destroy();
		ReplacementTargetActor->Destroy();
		OvershootTargetActor->Destroy();
		MissileActor->Destroy();
		return false;
	}

	// [v1.3.0] 저장 DirectTest를 실제 Weapon runtime에 연결할 transient VehicleData입니다.
	UCFVehicleData* FireIntegrationVehicleData = CreateMissileFireIntegrationVehicleData(
		FireIntegrationVehiclePawn,
		PersistedMissileProjectileData);
	if (!TestNotNull(TEXT("Fire 통합 VehicleData가 생성돼야 함"), FireIntegrationVehicleData))
	{
		FireIntegrationVehiclePawn->Destroy();
		DamageTargetActor->Destroy();
		ReplacementTargetActor->Destroy();
		OvershootTargetActor->Destroy();
		MissileActor->Destroy();
		return false;
	}

	TestTrue(TEXT("저장 DirectTest Weapon Runtime 초기화"), FireVehicleWeaponComp->InitializeWeaponRuntime(FireIntegrationVehiclePawn, FireIntegrationVehicleData));
	TestTrue(TEXT("저장 DirectTest Projectile Actor 실행 준비"), FireVehicleWeaponComp->IsActiveProjectileSpawnReady());
	TestEqual(TEXT("활성 ProjectileData는 저장 DirectTest"), FireVehicleWeaponComp->GetActiveProjectileData(), PersistedMissileProjectileData);

	// [v1.3.0] 실제 FireComp 실행 경계가 LaunchContext를 만들 때 사용할 Direct 발사 요청입니다.
	FCFVehicleFireRequest PersistedMissileFireRequest;
	PersistedMissileFireRequest.FireRequestId = 700;
	PersistedMissileFireRequest.AimOrigin = FVector(0.0f, 0.0f, 2000.0f);
	PersistedMissileFireRequest.AimDirection = FVector::ForwardVector;
	PersistedMissileFireRequest.PredictedAimTargetLocation = DamageTargetActor->GetActorLocation();
	PersistedMissileFireRequest.ClientFireTimeSeconds = 7.0f;
	PersistedMissileFireRequest.WeaponGroupId = TEXT("RoofTurret_MediumOrLarge");

	// [v1.3.0] Validate 단계 직후와 같은 승인 상태로 실제 ExecuteAcceptedFireCommand를 실행할 FireResult입니다.
	FCFVehicleFireResult PersistedMissileFireResult;
	PersistedMissileFireResult.FireRequestId = PersistedMissileFireRequest.FireRequestId;
	PersistedMissileFireResult.ValidationAimTargetLocation = PersistedMissileFireRequest.PredictedAimTargetLocation;
	PersistedMissileFireResult.LocalHitLocation = PersistedMissileFireRequest.PredictedAimTargetLocation;
	PersistedMissileFireResult.bAccepted = true;
	PersistedMissileFireResult.RejectReason = ECFVehicleFireRejectReason::None;

	// [v1.3.0] 실제 피해 전후 차이를 판정할 Impact 이전 타겟 내구도입니다.
	const float IntegrityBeforeMissileImpact = DamageTargetHealthComp->GetCurrentIntegrity();

	TestTrue(
		TEXT("VehicleFireComp 실행이 Pool을 통해 저장 DirectTest를 발사"),
		FireVehicleFireComp->ExecuteAcceptedFireCommand(
			PersistedMissileFireRequest,
			PersistedMissileFireResult,
			DamageTargetActor,
			false));
	TestTrue(TEXT("저장 DirectTest FireResult 승인 유지"), PersistedMissileFireResult.bAccepted);
	TestEqual(TEXT("저장 DirectTest 발사 직후 Pool 활성 수 1"), FireVehicleProjectilePoolComp->GetActivePooledProjectileCount(), 1);

	// [v1.3.0] 실제 ProjectilePool이 저장 ProjectileActorClass로 생성한 활성 미사일 Actor입니다.
	ACFProjectileActor* PersistedPoolMissileActor = nullptr;

	// [v1.3.0] 이번 FireIntegrationVehicle이 발사한 활성 Projectile을 찾는 월드 Actor 순회자입니다.
	for (TActorIterator<ACFProjectileActor> ProjectileActorIterator(TestWorld); ProjectileActorIterator; ++ProjectileActorIterator)
	{
		// [v1.3.0] 현재 순회 중인 실제 Projectile Actor입니다.
		ACFProjectileActor* CandidateProjectileActor = *ProjectileActorIterator;
		if (IsValid(CandidateProjectileActor)
			&& CandidateProjectileActor->IsProjectileActive()
			&& CandidateProjectileActor->GetActiveInstigatorActor() == FireIntegrationVehiclePawn)
		{
			PersistedPoolMissileActor = CandidateProjectileActor;
			break;
		}
	}

	if (!TestNotNull(TEXT("Pool이 실제 활성 Missile Actor를 생성해야 함"), PersistedPoolMissileActor))
	{
		FireIntegrationVehiclePawn->Destroy();
		DamageTargetActor->Destroy();
		ReplacementTargetActor->Destroy();
		OvershootTargetActor->Destroy();
		MissileActor->Destroy();
		return false;
	}

	// [v1.3.0] CreateNewMap Editor Automation World가 생략한 Actor BeginPlay를 production dispatch 경로로 보완해 OnComponentHit 바인딩을 성립시킵니다.
	if (!PersistedPoolMissileActor->HasActorBegunPlay())
	{
		PersistedPoolMissileActor->DispatchBeginPlay();
	}
	TestTrue(TEXT("Pool Missile BeginPlay가 완료돼 실제 Hit delegate가 바인딩"), PersistedPoolMissileActor->HasActorBegunPlay());

	TestTrue(
		TEXT("Pool Missile Actor Class는 저장 ProjectileActorClass와 정확히 일치"),
		PersistedPoolMissileActor->GetClass() == PersistedMissileProjectileData->ProjectileActorClass.Get());

	// [v1.3.0] 실제 Pool Missile의 저장 TargetActor Snapshot 전달을 확인할 Guidance 컴포넌트입니다.
	UCFMissileGuideComp* PersistedPoolMissileGuideComp = PersistedPoolMissileActor->FindComponentByClass<UCFMissileGuideComp>();

	// [v1.3.0] Editor Automation World에서 production 이동을 명시적으로 진행할 ProjectileMovement 컴포넌트입니다.
	UProjectileMovementComponent* PersistedPoolProjectileMovementComp = PersistedPoolMissileActor->FindComponentByClass<UProjectileMovementComponent>();
	if (!TestNotNull(TEXT("Pool Missile에 MissileGuideComp 존재"), PersistedPoolMissileGuideComp)
		|| !TestNotNull(TEXT("Pool Missile에 ProjectileMovementComponent 존재"), PersistedPoolProjectileMovementComp))
	{
		FireIntegrationVehiclePawn->Destroy();
		DamageTargetActor->Destroy();
		ReplacementTargetActor->Destroy();
		OvershootTargetActor->Destroy();
		MissileActor->Destroy();
		return false;
	}
	TestEqual(
		TEXT("FireComp LaunchContext의 Guidance Target Snapshot이 Pool Missile까지 전달"),
		PersistedPoolMissileGuideComp->GetGuidanceTargetActor(),
		static_cast<AActor*>(DamageTargetActor));

	// [v1.3.0] 순간이동 없이 실제 ProjectileMovement가 시작한 발사 위치입니다.
	const FVector PersistedPoolMissileStartLocation = PersistedPoolMissileActor->GetActorLocation();
	TestTrue(
		TEXT("Pool Missile은 FireRequest AimOrigin에서 시작"),
		PersistedPoolMissileStartLocation.Equals(FVector(PersistedMissileFireRequest.AimOrigin), 1.0f));

	// [v1.3.0] 실제 ProjectileMovement/Flight/Guide/Actor Tick을 한 프레임씩 진행할 고정 시뮬레이션 간격입니다.
	constexpr float IntegrationSimulationStepSeconds = 1.0f / 60.0f;

	// [v1.3.0] 1000cm 앞 목표 충돌을 충분히 허용하되 테스트 무한 반복을 막을 최대 프레임 수입니다.
	constexpr int32 MaximumIntegrationSimulationSteps = 120;

	// [v1.3.0] production ProjectileMovement가 실제 Blocking 접촉 또는 다른 종료 상태에 도달할 때까지 진행한 프레임 수입니다.
	int32 ExecutedIntegrationSimulationSteps = 0;
	while (PersistedPoolMissileActor->IsProjectileActive()
		&& ExecutedIntegrationSimulationSteps < MaximumIntegrationSimulationSteps)
	{
		// [v1.3.0] CreateNewMap Editor World의 자동 movement scheduling에 의존하지 않고 production ProjectileMovement 자체를 실제 Tick합니다.
		PersistedPoolProjectileMovementComp->TickComponent(
			IntegrationSimulationStepSeconds,
			LEVELTICK_All,
			nullptr);
		++ExecutedIntegrationSimulationSteps;

		if (PersistedPoolProjectileMovementComp->Velocity.IsNearlyZero())
		{
			break;
		}
	}

	// [v1.3.0] production ProjectileMovement가 순간이동 없이 도달한 실제 Blocking 접촉 위치입니다.
	const FVector PersistedPoolMissileBlockingLocation = PersistedPoolMissileActor->GetActorLocation();

	// [v1.3.0] Blocking 위치가 타겟 중심으로부터 떨어진 실제 접촉 거리입니다.
	const float BlockingDistanceToTargetCenter = FVector::Dist(
		PersistedPoolMissileBlockingLocation,
		DamageTargetActor->GetActorLocation());

	TestTrue(TEXT("실제 비행 시뮬레이션이 최소 1프레임 진행"), ExecutedIntegrationSimulationSteps > 0);
	TestTrue(
		TEXT("production ProjectileMovement가 발사 위치에서 실제 이동"),
		FVector::Dist(PersistedPoolMissileStartLocation, PersistedPoolMissileBlockingLocation) > 1.0f);
	TestTrue(TEXT("production ProjectileMovement가 Blocking 충돌로 속도 0"), PersistedPoolProjectileMovementComp->Velocity.IsNearlyZero());
	TestTrue(
		TEXT("production ProjectileMovement가 타겟 충돌 반경 경계에서 정지"),
		BlockingDistanceToTargetCenter <= 150.0f);

	AddInfo(FString::Printf(
		TEXT("Persisted Fire→Pool Movement Contact: Steps=%d, Start=(%.1f, %.1f, %.1f), Blocking=(%.1f, %.1f, %.1f), Target=(%.1f, %.1f, %.1f), CenterDistance=%.1f"),
		ExecutedIntegrationSimulationSteps,
		PersistedPoolMissileStartLocation.X,
		PersistedPoolMissileStartLocation.Y,
		PersistedPoolMissileStartLocation.Z,
		PersistedPoolMissileBlockingLocation.X,
		PersistedPoolMissileBlockingLocation.Y,
		PersistedPoolMissileBlockingLocation.Z,
		DamageTargetActor->GetActorLocation().X,
		DamageTargetActor->GetActorLocation().Y,
		DamageTargetActor->GetActorLocation().Z,
		BlockingDistanceToTargetCenter));

	if (PersistedPoolMissileActor->IsProjectileActive())
	{
		// [v1.3.0] Editor Automation이 생략하는 swept OnComponentHit dispatch만 보완해 production Supplemental Sweep이 같은 접촉을 Resolve하게 하는 테스트 전용 위치입니다.
		const FVector AutomationImpactBridgeLocation = DamageTargetActor->GetActorLocation();
		PersistedPoolMissileActor->SetActorLocation(
			AutomationImpactBridgeLocation,
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
		PersistedPoolMissileActor->SetActorTickEnabled(true);
		TestWorld->Tick(LEVELTICK_All, IntegrationSimulationStepSeconds);
	}

	// [v1.3.0] Automation impact bridge 이후 실제 Damage 적용 결과를 비교할 타겟 내구도입니다.
	const float IntegrityAfterMissileImpact = DamageTargetHealthComp->GetCurrentIntegrity();

	TestFalse(TEXT("저장 DirectTest는 production Supplemental Sweep Impact 뒤 비활성화"), PersistedPoolMissileActor->IsProjectileActive());
	TestEqual(TEXT("저장 DirectTest 비활성화 사유는 Hit"), PersistedPoolMissileActor->GetLastDeactivateReason(), ECFProjectileDeactivateReason::Hit);
	TestEqual(TEXT("저장 DirectTest 실제 충돌 대상은 Damage Target"), PersistedPoolMissileActor->GetLastHitActor(), static_cast<AActor*>(DamageTargetActor));
	TestTrue(TEXT("저장 DirectTest Impact에서 DamageHitContext 생성"), PersistedPoolMissileActor->HasLastDamageHitContext());
	TestTrue(TEXT("저장 DirectTest Impact에서 VehicleDamageResult 생성"), PersistedPoolMissileActor->HasLastVehicleDamageResult());
	TestTrue(TEXT("저장 DirectTest DamageData가 HitContext까지 전달"), PersistedPoolMissileActor->GetLastDamageHitContext().DamageData == PersistedMissileProjectileData->DefaultDamageData.Get());
	TestTrue(TEXT("저장 DirectTest 실제 Impact가 Target Integrity를 감소"), IntegrityAfterMissileImpact < IntegrityBeforeMissileImpact);
	TestEqual(TEXT("Impact 뒤 Pool 활성 수 0"), FireVehicleProjectilePoolComp->GetActivePooledProjectileCount(), 0);
	TestTrue(TEXT("Impact 뒤 실제 ProjectileActorClass Actor가 Pool 비활성 목록으로 반환"), FireVehicleProjectilePoolComp->GetInactivePooledProjectileCount() >= 1);

	FireIntegrationVehiclePawn->Destroy();
	DamageTargetActor->Destroy();
	ReplacementTargetActor->Destroy();
	OvershootTargetActor->Destroy();
	MissileActor->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
