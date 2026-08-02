// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-28
// Description: CF-FQ-029 Projectile Launch Handoff 자동화 테스트
// Scope: Launch Context 기본값, Actor·Pool 전달, 초기 월드 Velocity, Legacy Direct 호환과 비활성화 Reset을 검증합니다.
// Changelog:
// - v1.0.0: LM-P0-01 RuntimeContract 자동화 테스트 최초 추가.
// Migration:
// - 외부 Blueprint 또는 DataAsset 없이 C++ Launch Context와 기존 Direct Projectile 회귀를 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFProjectileActor.h"
#include "CFProjectileData.h"
#include "CFProjectileLaunchTypes.h"
#include "CFProjectileMotorComp.h"
#include "CFProjectilePoolComp.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFProjectileLaunchContractTest,
	"CarFight.ProjectileLaunch.LM_P0_01.RuntimeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Launch Context의 안전 기본값, 전달·초기 Velocity·Reset과 Legacy Direct 호환을 검증합니다.
bool FCFProjectileLaunchContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 신규 Launch Context의 기존 Projectile 호환 기본값입니다.
	const FCFProjectileLaunchContext DefaultLaunchContext;
	TestEqual(TEXT("기본 Release Mode는 Direct"), DefaultLaunchContext.ReleaseMode, ECFProjectileReleaseMode::Direct);
	TestTrue(TEXT("기본 Launch Transform은 Identity"), DefaultLaunchContext.LaunchTransform.Equals(FTransform::Identity));
	TestTrue(TEXT("기본 초기 방향은 Forward"), DefaultLaunchContext.InitialLaunchDirection.Equals(FVector::ForwardVector));
	TestTrue(TEXT("기본 초기 Velocity는 Zero"), DefaultLaunchContext.InitialLaunchVelocity.IsNearlyZero());
	TestTrue(TEXT("기본 상속 플랫폼 Velocity는 Zero"), DefaultLaunchContext.InheritedCarrierVelocity.IsNearlyZero());
	TestEqual(TEXT("기본 FireRequestId는 0"), DefaultLaunchContext.FireRequestId, 0);
	TestEqual(TEXT("기본 WeaponGroupId는 None"), DefaultLaunchContext.WeaponGroupId, NAME_None);

	// [v1.0.0] 실제 Actor와 Pool 활성화를 검증할 Automation 월드입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Projectile Launch 테스트 월드가 생성돼야 함"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] Direct 호환과 Context 초기 Velocity를 검증할 임시 ProjectileData입니다.
	UCFProjectileData* TestProjectileData = NewObject<UCFProjectileData>();
	if (!TestNotNull(TEXT("Projectile Launch 테스트 Data가 생성돼야 함"), TestProjectileData))
	{
		return false;
	}

	TestProjectileData->ProjectileActorClass = ACFProjectileActor::StaticClass();
	TestProjectileData->InitialSpeed = 321.0f;
	TestProjectileData->LifeTimeSeconds = 10.0f;
	TestProjectileData->bAffectedByGravity = false;
	TestProjectileData->bUseSupplementalContinuousSweep = false;
	TestProjectileData->PropulsionConfig.bUsePropulsion = false;

	// [v1.0.0] Context 기반 활성화와 비활성화 Reset을 직접 검증할 Projectile Actor입니다.
	ACFProjectileActor* DirectProjectileActor = TestWorld->SpawnActor<ACFProjectileActor>();
	if (!TestNotNull(TEXT("Context 직접 활성화용 Projectile Actor가 생성돼야 함"), DirectProjectileActor))
	{
		return false;
	}
	DirectProjectileActor->SetDestroyWhenDeactivated(false);

	// [v1.0.0] 초기 발사 방향과 월드 Velocity가 서로 독립적으로 전달되는 테스트 Context입니다.
	const FVector ContextLaunchDirection = FVector(1.0f, 1.0f, 0.0f).GetSafeNormal();
	const FVector ContextInitialVelocity = ContextLaunchDirection * 450.0f;

	// [v1.0.0] Actor가 값으로 복사하고 런처와 독립적으로 보존할 발사 Context입니다.
	FCFProjectileLaunchContext LaunchContext;
	LaunchContext.LaunchTransform = FTransform(ContextLaunchDirection.Rotation(), FVector(100.0f, 200.0f, 300.0f));
	LaunchContext.InitialLaunchDirection = ContextLaunchDirection;
	LaunchContext.InitialLaunchVelocity = ContextInitialVelocity;
	LaunchContext.InheritedCarrierVelocity = FVector::ZeroVector;
	LaunchContext.CommandTargetLocation = FVector(5000.0f, 4000.0f, 300.0f);
	LaunchContext.ReleaseMode = ECFProjectileReleaseMode::Direct;
	LaunchContext.FireRequestId = 77;
	LaunchContext.WeaponGroupId = TEXT("Top_01");

	DirectProjectileActor->ActivateProjectileWithContext(TestProjectileData, LaunchContext, nullptr);
	TestTrue(TEXT("Context 기반 Projectile 활성화 성공"), DirectProjectileActor->IsProjectileActive());
	TestTrue(TEXT("활성 Projectile이 Launch Context를 보유"), DirectProjectileActor->HasActiveLaunchContext());
	TestTrue(TEXT("Context Launch 위치가 Actor 위치에 적용"), DirectProjectileActor->GetActorLocation().Equals(LaunchContext.LaunchTransform.GetLocation(), KINDA_SMALL_NUMBER));

	// [v1.0.0] 실제 ProjectileMovement에 적용된 초기 월드 Velocity입니다.
	UProjectileMovementComponent* DirectMovementComp = DirectProjectileActor->FindComponentByClass<UProjectileMovementComponent>();
	if (TestNotNull(TEXT("Context Actor의 ProjectileMovementComponent 존재"), DirectMovementComp))
	{
		TestTrue(TEXT("Context InitialLaunchVelocity가 그대로 적용"), DirectMovementComp->Velocity.Equals(ContextInitialVelocity, KINDA_SMALL_NUMBER));
		TestEqual(TEXT("Context 초기 속력으로 InitialSpeed 갱신"), DirectMovementComp->InitialSpeed, 450.0f);
		TestEqual(TEXT("비추진 Context 초기 속력으로 MaxSpeed 갱신"), DirectMovementComp->MaxSpeed, 450.0f);
	}

	// [v1.0.0] Actor가 복사해 보유하는 현재 활성 Launch Context입니다.
	const FCFProjectileLaunchContext ActiveLaunchContext = DirectProjectileActor->GetActiveLaunchContext();
	TestEqual(TEXT("FireRequestId가 Actor Context에 보존"), ActiveLaunchContext.FireRequestId, 77);
	TestEqual(TEXT("WeaponGroupId가 Actor Context에 보존"), ActiveLaunchContext.WeaponGroupId, FName(TEXT("Top_01")));
	TestEqual(TEXT("Release Mode가 Actor Context에 보존"), ActiveLaunchContext.ReleaseMode, ECFProjectileReleaseMode::Direct);
	TestTrue(TEXT("명령 목표 위치가 Actor Context에 보존"), ActiveLaunchContext.CommandTargetLocation.Equals(LaunchContext.CommandTargetLocation));
	TestTrue(TEXT("Launch Summary에 요청 ID 기록"), DirectProjectileActor->BuildProjectileLaunchSummary().Contains(TEXT("Request=77")));

	// [v1.0.0] 비유도 Rocket 모터가 Context의 초기 발사 방향을 수신했는지 확인할 스냅샷입니다.
	UCFProjectileMotorComp* DirectMotorComp = DirectProjectileActor->FindComponentByClass<UCFProjectileMotorComp>();
	if (TestNotNull(TEXT("Context Actor의 ProjectileMotorComp 존재"), DirectMotorComp))
	{
		TestTrue(TEXT("모터 고정 추진 방향이 Context 초기 방향과 동일"), DirectMotorComp->GetMotorSnapshot().CurrentThrustDirection.Equals(ContextLaunchDirection, KINDA_SMALL_NUMBER));
	}

	DirectProjectileActor->DeactivateProjectile();
	TestFalse(TEXT("비활성화 뒤 Launch Context 존재 여부 Reset"), DirectProjectileActor->HasActiveLaunchContext());
	TestEqual(TEXT("비활성화 뒤 Context FireRequestId Reset"), DirectProjectileActor->GetActiveLaunchContext().FireRequestId, 0);

	// [v1.0.0] 기존 ActivateProjectile API가 InitialSpeed 기반 Direct Context를 생성하는지 검증합니다.
	DirectProjectileActor->ActivateProjectile(TestProjectileData, FVector::ForwardVector, nullptr);
	TestTrue(TEXT("Legacy ActivateProjectile도 Projectile 활성화"), DirectProjectileActor->IsProjectileActive());
	TestTrue(TEXT("Legacy ActivateProjectile도 Direct Context 생성"), DirectProjectileActor->HasActiveLaunchContext());
	if (DirectMovementComp)
	{
		TestTrue(TEXT("Legacy 초기 Velocity는 Direction * InitialSpeed"), DirectMovementComp->Velocity.Equals(FVector::ForwardVector * 321.0f, KINDA_SMALL_NUMBER));
	}
	TestEqual(TEXT("Legacy Release Mode는 Direct"), DirectProjectileActor->GetActiveLaunchContext().ReleaseMode, ECFProjectileReleaseMode::Direct);
	DirectProjectileActor->DeactivateProjectile();

	// [v1.0.0] Context가 ProjectilePool Acquire 경로를 통과하는지 검증할 소유 Actor입니다.
	AActor* PoolOwnerActor = TestWorld->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("Projectile Pool 소유 Actor가 생성돼야 함"), PoolOwnerActor))
	{
		DirectProjectileActor->Destroy();
		return false;
	}

	// [v1.0.0] 테스트 소유 Actor에 런타임 등록한 Projectile Pool 컴포넌트입니다.
	UCFProjectilePoolComp* ProjectilePoolComp = NewObject<UCFProjectilePoolComp>(PoolOwnerActor, TEXT("ProjectileLaunchTestPool"));
	PoolOwnerActor->AddInstanceComponent(ProjectilePoolComp);
	ProjectilePoolComp->RegisterComponent();

	LaunchContext.FireRequestId = 88;
	LaunchContext.WeaponGroupId = TEXT("Launcher_Test");

	// [v1.0.0] Pool에서 재사용하거나 생성해 Context로 활성화한 Projectile Actor입니다.
	ACFProjectileActor* PooledProjectileActor = ProjectilePoolComp->AcquireProjectileWithContext(TestProjectileData, LaunchContext, PoolOwnerActor);
	if (TestNotNull(TEXT("Pool Context Acquire가 Projectile Actor를 반환"), PooledProjectileActor))
	{
		TestTrue(TEXT("Pool Actor가 Context를 보유"), PooledProjectileActor->HasActiveLaunchContext());
		TestEqual(TEXT("Pool Actor에 FireRequestId 전달"), PooledProjectileActor->GetActiveLaunchContext().FireRequestId, 88);
		PooledProjectileActor->DeactivateProjectile();
		TestFalse(TEXT("Pool 반환 전 Actor Context Reset"), PooledProjectileActor->HasActiveLaunchContext());
		TestEqual(TEXT("Pool 반환 뒤 비활성 Actor 수 1"), ProjectilePoolComp->GetInactivePooledProjectileCount(), 1);
	}

	DirectProjectileActor->Destroy();
	PoolOwnerActor->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
