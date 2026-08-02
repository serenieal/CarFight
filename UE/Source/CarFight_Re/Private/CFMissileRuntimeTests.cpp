// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-02
// Description: CF-FQ-030 MG-P0-01~04 Direct 미사일 Runtime 자동화 테스트
// Scope: 정지·측면 이동·Target Snapshot·목표 파괴·오버슈트·동일 Actor Pool 재사용과 물리 제한형 Guidance를 검증합니다.
// Changelog:
// - v1.1.0: 정지 목표, 측면 이동, 오버슈트와 동일 Projectile Actor 재활성화 Pool 계약 검증을 추가.
// - v1.0.0: Direct Missile Runtime Contract 최초 추가.
// Migration:
// - 외부 Blueprint·DataAsset 없이 C++ 기본 Actor와 임시 ProjectileData로 런타임 계약을 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFMissileFlightComp.h"
#include "CFMissileGuideComp.h"
#include "CFMissileTestTarget.h"
#include "CFProjectileActor.h"
#include "CFProjectileData.h"
#include "CFProjectileMotorComp.h"

#include "Engine/World.h"
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
}

// [v1.1.0] Direct Missile의 여섯 검증 시나리오와 Reset 계약을 검증합니다.
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

	ReplacementTargetActor->Destroy();
	OvershootTargetActor->Destroy();
	MissileActor->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
