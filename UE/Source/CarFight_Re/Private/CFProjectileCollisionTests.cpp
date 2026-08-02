// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 2.0.0
// Date: 2026-07-30
// Description: CF-FQ-029 발사 차량 단위 Projectile 충돌 격리·요격 자동화 테스트
// Scope: 동일 차량의 모든 Projectile Ignore, 다른 차량 Projectile Block·Interception, Pool Reset과 비요격 설정을 검증합니다.
// Changelog:
// - v2.0.0: 전역 Projectile Ignore 계약을 폐기하고 ActiveInstigatorActor 기반 차량별 격리와 Intercepted 계약으로 교체.
// - v1.0.0: 모든 Projectile 상호 Ignore를 검증한 임시 계약. v2.0.0에서 폐기.
// Migration:
// - 같은 차량이 발사한 Projectile은 탄종·Volley·FireRequest와 무관하게 서로 충돌하지 않습니다.
// - 서로 다른 차량이 발사한 Projectile은 기본 Block이며 bCanBeIntercepted=true이면 유효 적중 한 번으로 Intercepted 처리됩니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFCollisionChannels.h"
#include "CFProjectileActor.h"
#include "CFProjectileData.h"
#include "CFProjectileLaunchTypes.h"
#include "CFProjectilePoolComp.h"

#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFProjectileCollisionContractTest,
	"CarFight.Projectile.LM_P0_06.SourceIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v2.0.0] 테스트 Source Actor에 ProjectilePoolComp를 생성·등록합니다.
static UCFProjectilePoolComp* CreateProjectileTestPool(AActor* InSourceActor, const FName InComponentName)
{
	if (!InSourceActor)
	{
		return nullptr;
	}

	// [v2.0.0] Source Actor가 소유할 런타임 Projectile Pool 컴포넌트입니다.
	UCFProjectilePoolComp* ProjectilePoolComp = NewObject<UCFProjectilePoolComp>(InSourceActor, InComponentName);
	InSourceActor->AddInstanceComponent(ProjectilePoolComp);
	ProjectilePoolComp->RegisterComponent();
	return ProjectilePoolComp;
}

// [v2.0.0] 지정 위치에서 Direct Projectile을 Pool로 확보할 Launch Context를 생성합니다.
static FCFProjectileLaunchContext BuildProjectileTestLaunchContext(const FVector& InLaunchLocation, const int32 InFireRequestId)
{
	// [v2.0.0] 차량별 충돌 격리 테스트용 Direct 발사 Context입니다.
	FCFProjectileLaunchContext LaunchContext;
	LaunchContext.LaunchTransform = FTransform(FRotator::ZeroRotator, InLaunchLocation);
	LaunchContext.InitialLaunchDirection = FVector::ForwardVector;
	LaunchContext.InitialLaunchVelocity = FVector(1000.0f, 0.0f, 0.0f);
	LaunchContext.ReleaseMode = ECFProjectileReleaseMode::Direct;
	LaunchContext.FireRequestId = InFireRequestId;
	LaunchContext.WeaponGroupId = TEXT("CollisionTest");
	return LaunchContext;
}

// [v2.0.0] 동일 발사 차량 격리, 다른 차량 요격과 Pool 재사용 계약을 검증합니다.
bool FCFProjectileCollisionContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v2.0.0] Projectile 충돌 격리와 요격을 검증할 Automation 월드입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Projectile Source Isolation 테스트 월드 생성"), TestWorld))
	{
		return false;
	}

	// [v2.0.0] 기존 DataAsset 저장 없이 기본 요격 정책을 검증할 임시 ProjectileData입니다.
	UCFProjectileData* TestProjectileData = NewObject<UCFProjectileData>();
	if (!TestNotNull(TEXT("Projectile Source Isolation 테스트 Data 생성"), TestProjectileData))
	{
		return false;
	}

	TestProjectileData->ProjectileActorClass = ACFProjectileActor::StaticClass();
	TestProjectileData->ProjectileId = TEXT("SourceIsolationTest");
	TestProjectileData->InitialSpeed = 1000.0f;
	TestProjectileData->LifeTimeSeconds = 30.0f;
	TestProjectileData->CollisionRadius = 8.0f;
	TestProjectileData->bAffectedByGravity = false;
	TestProjectileData->bUseSupplementalContinuousSweep = false;
	TestProjectileData->PropulsionConfig.bUsePropulsion = false;

	TestTrue(TEXT("기존 ProjectileData 기본 요격 가능"), TestProjectileData->bCanBeIntercepted);
	TestTrue(TEXT("기존 ProjectileData 기본 요격 폭발 FX 요청"), TestProjectileData->bDetonateWhenIntercepted);
	TestTrue(TEXT("Projectile 요약에 요격 가능 상태 포함"), TestProjectileData->BuildProjectileSummary().Contains(TEXT("Interceptable=Yes")));

	// [v2.0.0] 같은 차량 Projectile을 발사할 첫 번째 Source Actor입니다.
	AActor* SourceActorA = TestWorld->SpawnActor<AActor>();

	// [v2.0.0] 다른 차량 Projectile을 발사할 두 번째 Source Actor입니다.
	AActor* SourceActorB = TestWorld->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("Source Actor A 생성"), SourceActorA)
		|| !TestNotNull(TEXT("Source Actor B 생성"), SourceActorB))
	{
		return false;
	}

	// [v2.0.0] Source Actor A가 모든 탄종·Volley를 함께 추적할 Projectile Pool입니다.
	UCFProjectilePoolComp* ProjectilePoolA = CreateProjectileTestPool(SourceActorA, TEXT("ProjectilePoolA"));

	// [v2.0.0] Source Actor B가 별도로 소유할 Projectile Pool입니다.
	UCFProjectilePoolComp* ProjectilePoolB = CreateProjectileTestPool(SourceActorB, TEXT("ProjectilePoolB"));
	if (!TestNotNull(TEXT("Projectile Pool A 생성"), ProjectilePoolA)
		|| !TestNotNull(TEXT("Projectile Pool B 생성"), ProjectilePoolB))
	{
		return false;
	}

	// [v2.0.0] Source A의 첫 번째 Volley Projectile입니다.
	ACFProjectileActor* SourceAProjectile1 = ProjectilePoolA->AcquireProjectileWithContext(
		TestProjectileData,
		BuildProjectileTestLaunchContext(FVector(0.0f, 0.0f, 1000.0f), 1),
		SourceActorA);

	// [v2.0.0] Source A의 다른 FireRequest에서 발사된 두 번째 Projectile입니다.
	ACFProjectileActor* SourceAProjectile2 = ProjectilePoolA->AcquireProjectileWithContext(
		TestProjectileData,
		BuildProjectileTestLaunchContext(FVector(0.0f, 100.0f, 1000.0f), 2),
		SourceActorA);
	if (!TestNotNull(TEXT("Source A Projectile 1 활성화"), SourceAProjectile1)
		|| !TestNotNull(TEXT("Source A Projectile 2 활성화"), SourceAProjectile2))
	{
		return false;
	}

	// [v2.0.0] Projectile Object Channel 기본 응답을 확인할 실제 CollisionComponent입니다.
	USphereComponent* SourceAProjectileCollision = SourceAProjectile1->FindComponentByClass<USphereComponent>();
	if (!TestNotNull(TEXT("Projectile CollisionComponent 존재"), SourceAProjectileCollision))
	{
		return false;
	}

	TestEqual(
		TEXT("Projectile Object Type은 Projectile"),
		SourceAProjectileCollision->GetCollisionObjectType(),
		CFCollisionChannels::Projectile);
	TestEqual(
		TEXT("다른 차량 요격을 위해 Projectile 채널 기본 Block"),
		SourceAProjectileCollision->GetCollisionResponseToChannel(CFCollisionChannels::Projectile),
		ECR_Block);
	TestEqual(
		TEXT("WorldStatic 채널 Block 유지"),
		SourceAProjectileCollision->GetCollisionResponseToChannel(ECC_WorldStatic),
		ECR_Block);

	TestTrue(TEXT("같은 차량 Projectile 1이 Projectile 2를 Ignore"), SourceAProjectile1->IsIgnoringSameSourceProjectile(SourceAProjectile2));
	TestTrue(TEXT("같은 차량 Projectile 2가 Projectile 1을 Ignore"), SourceAProjectile2->IsIgnoringSameSourceProjectile(SourceAProjectile1));
	TestEqual(TEXT("Projectile 1 동일 차량 Ignore 수 1"), SourceAProjectile1->GetIgnoredSameSourceProjectileCount(), 1);
	TestEqual(TEXT("Projectile 2 동일 차량 Ignore 수 1"), SourceAProjectile2->GetIgnoredSameSourceProjectileCount(), 1);

	// [v2.0.0] Source B가 발사해 Source A Projectile과 실제 충돌할 수 있는 적 Projectile입니다.
	ACFProjectileActor* SourceBProjectile = ProjectilePoolB->AcquireProjectileWithContext(
		TestProjectileData,
		BuildProjectileTestLaunchContext(FVector(0.0f, 200.0f, 1000.0f), 3),
		SourceActorB);
	if (!TestNotNull(TEXT("Source B Projectile 활성화"), SourceBProjectile))
	{
		return false;
	}

	TestFalse(TEXT("다른 차량 Projectile은 Ignore 목록에 없음"), SourceAProjectile1->IsIgnoringSameSourceProjectile(SourceBProjectile));
	TestFalse(TEXT("다른 차량 Projectile도 Source A를 Ignore하지 않음"), SourceBProjectile->IsIgnoringSameSourceProjectile(SourceAProjectile1));

	// [v2.0.0] 같은 Source의 요격 요청을 거부하는지 확인할 Hit 결과입니다.
	FHitResult SameSourceHitResult;
	SameSourceHitResult.ImpactPoint = SourceAProjectile2->GetActorLocation();
	SameSourceHitResult.ImpactNormal = FVector::BackwardVector;
	SameSourceHitResult.TraceStart = SourceAProjectile1->GetActorLocation();
	SameSourceHitResult.TraceEnd = SourceAProjectile2->GetActorLocation();
	TestFalse(
		TEXT("같은 차량 Projectile은 요격 처리하지 않음"),
		SourceAProjectile2->TryResolveProjectileInterception(SourceAProjectile1, SourceActorA, SameSourceHitResult));
	TestTrue(TEXT("같은 차량 요격 거부 뒤 Projectile 유지"), SourceAProjectile2->IsProjectileActive());

	// [v2.0.0] 서로 다른 Source Projectile의 실제 요격 결과입니다.
	FHitResult DifferentSourceHitResult;
	DifferentSourceHitResult.ImpactPoint = SourceBProjectile->GetActorLocation();
	DifferentSourceHitResult.ImpactNormal = FVector::BackwardVector;
	DifferentSourceHitResult.TraceStart = SourceAProjectile1->GetActorLocation();
	DifferentSourceHitResult.TraceEnd = SourceBProjectile->GetActorLocation();
	TestTrue(
		TEXT("다른 차량 Projectile은 요격 가능"),
		SourceBProjectile->TryResolveProjectileInterception(SourceAProjectile1, SourceActorA, DifferentSourceHitResult));
	TestFalse(TEXT("요격된 Projectile 비활성"), SourceBProjectile->IsProjectileActive());
	TestEqual(TEXT("요격 비활성화 사유 Intercepted"), SourceBProjectile->GetLastDeactivateReason(), ECFProjectileDeactivateReason::Intercepted);
	TestTrue(TEXT("요격 HitContext 보존"), SourceBProjectile->HasLastDamageHitContext());
	TestEqual(TEXT("요격 HitContext DamageId"), SourceBProjectile->GetLastDamageHitContext().DamageId, FName(TEXT("ProjectileIntercept")));
	TestEqual(TEXT("Source B Pool 비활성 Projectile 1"), ProjectilePoolB->GetInactivePooledProjectileCount(), 1);

	SourceAProjectile2->DeactivateProjectile();
	TestFalse(TEXT("Pool 반환 시 상대 Projectile의 Ignore 관계 해제"), SourceAProjectile1->IsIgnoringSameSourceProjectile(SourceAProjectile2));
	TestEqual(TEXT("Ignore 관계 해제 뒤 Source A Projectile 1 수 0"), SourceAProjectile1->GetIgnoredSameSourceProjectileCount(), 0);

	// [v2.0.0] Pool에서 재사용된 같은 차량 Projectile이 다시 격리되는지 확인할 Actor입니다.
	ACFProjectileActor* ReusedSourceAProjectile = ProjectilePoolA->AcquireProjectileWithContext(
		TestProjectileData,
		BuildProjectileTestLaunchContext(FVector(0.0f, 300.0f, 1000.0f), 4),
		SourceActorA);
	if (TestNotNull(TEXT("Source A Projectile Pool 재활성화"), ReusedSourceAProjectile))
	{
		TestTrue(TEXT("재활성화 Projectile이 기존 같은 차량 Projectile을 Ignore"), ReusedSourceAProjectile->IsIgnoringSameSourceProjectile(SourceAProjectile1));
		TestTrue(TEXT("기존 Projectile도 재활성화 Projectile을 Ignore"), SourceAProjectile1->IsIgnoringSameSourceProjectile(ReusedSourceAProjectile));
	}

	TestProjectileData->bCanBeIntercepted = false;

	// [v2.0.0] 비요격 설정을 검증하기 위해 Source B Pool에서 재사용한 Projectile입니다.
	ACFProjectileActor* NonInterceptableProjectile = ProjectilePoolB->AcquireProjectileWithContext(
		TestProjectileData,
		BuildProjectileTestLaunchContext(FVector(0.0f, 400.0f, 1000.0f), 5),
		SourceActorB);
	if (TestNotNull(TEXT("비요격 Projectile 활성화"), NonInterceptableProjectile))
	{
		FHitResult NonInterceptableHitResult;
		NonInterceptableHitResult.ImpactPoint = NonInterceptableProjectile->GetActorLocation();
		NonInterceptableHitResult.ImpactNormal = FVector::BackwardVector;
		NonInterceptableHitResult.TraceStart = SourceAProjectile1->GetActorLocation();
		NonInterceptableHitResult.TraceEnd = NonInterceptableProjectile->GetActorLocation();
		TestFalse(
			TEXT("bCanBeIntercepted False는 요격 거부"),
			NonInterceptableProjectile->TryResolveProjectileInterception(SourceAProjectile1, SourceActorA, NonInterceptableHitResult));
		TestTrue(TEXT("비요격 Projectile 활성 유지"), NonInterceptableProjectile->IsProjectileActive());
		NonInterceptableProjectile->DeactivateProjectile();
	}

	if (ReusedSourceAProjectile && ReusedSourceAProjectile->IsProjectileActive())
	{
		ReusedSourceAProjectile->DeactivateProjectile();
	}
	if (SourceAProjectile1->IsProjectileActive())
	{
		SourceAProjectile1->DeactivateProjectile();
	}

	SourceActorA->Destroy();
	SourceActorB->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
