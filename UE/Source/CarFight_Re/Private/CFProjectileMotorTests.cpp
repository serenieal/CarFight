// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-27
// Description: CF-FQ-028 발사체 추진 모터 자동화 테스트
// Scope: 안전 기본값, 점화 지연, 추진 가속, 최대 속도, 연소 종료와 Pool 재사용 초기화를 검증합니다.
// Changelog:
// - v1.0.0: PP-P0-01 RuntimeContract 자동화 테스트 최초 추가.
// Migration:
// - 외부 Projectile 또는 Niagara 자산 없이 C++ 추진 상태와 ProjectileMovement 속도 계약을 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFProjectileActor.h"
#include "CFProjectileData.h"
#include "CFProjectileMotorComp.h"

#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFProjectileMotorContractTest,
	"CarFight.ProjectilePropulsion.PP_P0_01.RuntimeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 추진 데이터 기본값과 점화·연소·관성·Reset 계약을 검증합니다.
bool FCFProjectileMotorContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 기존 ProjectileData 호환 기본값을 확인할 CDO입니다.
	const UCFProjectileData* DefaultProjectileData = GetDefault<UCFProjectileData>();
	if (!TestNotNull(TEXT("ProjectileData CDO가 존재해야 함"), DefaultProjectileData))
	{
		return false;
	}

	TestFalse(TEXT("기존 ProjectileData 호환을 위해 자체 추진 기본값은 비활성"), DefaultProjectileData->PropulsionConfig.bUsePropulsion);
	TestEqual(TEXT("기본 점화 지연은 0.05초"), DefaultProjectileData->PropulsionConfig.IgnitionDelaySeconds, 0.05f);
	TestEqual(TEXT("기본 연소 시간은 1초"), DefaultProjectileData->PropulsionConfig.BurnDurationSeconds, 1.0f);
	TestEqual(TEXT("기본 추진 가속도는 9000cm/s²"), DefaultProjectileData->PropulsionConfig.ThrustAccelerationCmPerSecSq, 9000.0f);
	TestEqual(TEXT("기본 최대 추진 속도는 10000cm/s"), DefaultProjectileData->PropulsionConfig.MaximumPropelledSpeed, 10000.0f);

	// [v1.0.0] 실제 Actor 활성화와 모터 상태를 검증할 Automation 월드입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Projectile Motor 테스트 월드가 생성돼야 함"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] Pool 재사용과 같은 수동 비활성화를 검증할 Projectile Actor입니다.
	ACFProjectileActor* ProjectileActor = TestWorld->SpawnActor<ACFProjectileActor>();
	if (!TestNotNull(TEXT("Projectile Motor 테스트 Actor가 생성돼야 함"), ProjectileActor))
	{
		return false;
	}

	ProjectileActor->SetDestroyWhenDeactivated(false);

	UCFProjectileMotorComp* ProjectileMotorComp = ProjectileActor->FindComponentByClass<UCFProjectileMotorComp>();
	UProjectileMovementComponent* ProjectileMovementComp = ProjectileActor->FindComponentByClass<UProjectileMovementComponent>();
	if (!TestNotNull(TEXT("ProjectileMotorComp 기본 서브오브젝트 존재"), ProjectileMotorComp)
		|| !TestNotNull(TEXT("ProjectileMovementComponent 기본 서브오브젝트 존재"), ProjectileMovementComp))
	{
		ProjectileActor->Destroy();
		return false;
	}

	// [v1.0.0] 점화 지연과 최대 속도 제한을 검증할 임시 로켓 데이터입니다.
	UCFProjectileData* RocketProjectileData = NewObject<UCFProjectileData>(ProjectileActor);
	RocketProjectileData->InitialSpeed = 100.0f;
	RocketProjectileData->LifeTimeSeconds = 10.0f;
	RocketProjectileData->bAffectedByGravity = false;
	RocketProjectileData->bUseSupplementalContinuousSweep = false;
	RocketProjectileData->PropulsionConfig.bUsePropulsion = true;
	RocketProjectileData->PropulsionConfig.IgnitionDelaySeconds = 0.10f;
	RocketProjectileData->PropulsionConfig.BurnDurationSeconds = 0.50f;
	RocketProjectileData->PropulsionConfig.ThrustAccelerationCmPerSecSq = 1000.0f;
	RocketProjectileData->PropulsionConfig.MaximumPropelledSpeed = 500.0f;

	ProjectileActor->ActivateProjectile(RocketProjectileData, FVector::ForwardVector, nullptr);
	TestTrue(TEXT("추진 로켓도 기존 Projectile 활성화 경로를 사용"), ProjectileActor->IsProjectileActive());
	TestEqual(TEXT("점화 지연이 있으면 최초 상태는 IgnitionDelay"), ProjectileMotorComp->GetMotorState(), ECFProjectileMotorState::IgnitionDelay);
	TestEqual(TEXT("추진 ProjectileMovement MaxSpeed는 MaximumPropelledSpeed"), ProjectileMovementComp->MaxSpeed, 500.0f);
	TestEqual(TEXT("발사 직후 속도는 InitialSpeed"), ProjectileMovementComp->Velocity.Size(), 100.0);

		ProjectileMotorComp->AdvanceMotorForAutomation(0.05f);
	TestEqual(TEXT("점화 지연 전반부에는 가속하지 않음"), ProjectileMovementComp->Velocity.Size(), 100.0);
	TestEqual(TEXT("점화 지연 전반부 상태 유지"), ProjectileMotorComp->GetMotorState(), ECFProjectileMotorState::IgnitionDelay);

		ProjectileMotorComp->AdvanceMotorForAutomation(0.10f);
	TestEqual(TEXT("남은 점화 지연 뒤 같은 Tick의 잔여 시간부터 Burning 진입"), ProjectileMotorComp->GetMotorState(), ECFProjectileMotorState::Burning);
	TestEqual(TEXT("0.05초 연소분만큼 속도 증가"), ProjectileMovementComp->Velocity.Size(), 150.0);

		ProjectileMotorComp->AdvanceMotorForAutomation(0.50f);
	TestEqual(TEXT("연소 시간이 끝나면 BurnedOut"), ProjectileMotorComp->GetMotorState(), ECFProjectileMotorState::BurnedOut);
	TestEqual(TEXT("추진 속도는 MaximumPropelledSpeed에서 제한"), ProjectileMovementComp->Velocity.Size(), 500.0);
	TestFalse(TEXT("BurnedOut 상태에서는 추가 추진을 생산하지 않음"), ProjectileMotorComp->IsProducingThrust());
	TestTrue(TEXT("연소 종료 뒤에도 Projectile Actor는 관성 비행을 위해 활성 유지"), ProjectileActor->IsProjectileActive());

	ProjectileActor->DeactivateProjectile();
	TestEqual(TEXT("Projectile 비활성화 시 모터 상태는 Inactive로 Reset"), ProjectileMotorComp->GetMotorState(), ECFProjectileMotorState::Inactive);
	TestFalse(TEXT("Projectile 비활성화 후 추진 생산 없음"), ProjectileMotorComp->IsProducingThrust());

	// [v1.0.0] 기존 비추진 Projectile의 MaxSpeed와 Disabled 상태 호환을 검증합니다.
	UCFProjectileData* LegacyProjectileData = NewObject<UCFProjectileData>(ProjectileActor);
	LegacyProjectileData->InitialSpeed = 321.0f;
	LegacyProjectileData->LifeTimeSeconds = 10.0f;
	LegacyProjectileData->bAffectedByGravity = false;
	LegacyProjectileData->bUseSupplementalContinuousSweep = false;
	LegacyProjectileData->PropulsionConfig.bUsePropulsion = false;

	ProjectileActor->ActivateProjectile(LegacyProjectileData, FVector::ForwardVector, nullptr);
	TestEqual(TEXT("비추진 Projectile 모터 상태는 Disabled"), ProjectileMotorComp->GetMotorState(), ECFProjectileMotorState::Disabled);
	TestEqual(TEXT("비추진 Projectile MaxSpeed는 기존 InitialSpeed 유지"), ProjectileMovementComp->MaxSpeed, 321.0f);
	TestEqual(TEXT("비추진 Projectile 발사 속도는 기존 InitialSpeed 유지"), ProjectileMovementComp->Velocity.Size(), 321.0);

	ProjectileActor->DeactivateProjectile();
	ProjectileActor->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
