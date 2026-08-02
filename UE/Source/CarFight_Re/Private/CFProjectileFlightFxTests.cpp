// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-07-28
// Description: CF-FQ-027 투사체 비행 FX 자동화 테스트
// Scope: 기본 데이터 계약, Niagara 컴포넌트 구성, 소켓 누락 Fallback과 비활성화 Reset을 검증합니다.
// Changelog:
// - v1.1.0: 비행 FX 독립 Scale 적용과 비활성화 Reset 검증 추가.
// - v1.0.0: PFX-P0-01 RuntimeContract 자동화 테스트 최초 추가.
// Migration:
// - 외부 Niagara 자산 없이 C++ 기본 계약, 독립 FX Scale과 Pool 재사용 전 Reset 경로를 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFProjectileActor.h"
#include "CFProjectileData.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "NiagaraComponent.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFProjectileFlightFxContractTest,
	"CarFight.ProjectileFlightFx.PFX_P0_01.RuntimeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] Projectile Flight FX의 기본 데이터, 컴포넌트와 Reset 계약을 검증합니다.
bool FCFProjectileFlightFxContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 신규 비행 FX 필드의 안전 기본값을 확인할 ProjectileData CDO입니다.
	const UCFProjectileData* DefaultProjectileData = GetDefault<UCFProjectileData>();
	if (!TestNotNull(TEXT("ProjectileData CDO가 존재해야 함"), DefaultProjectileData))
	{
		return false;
	}

	TestFalse(TEXT("기존 ProjectileData 호환을 위해 Trail FX 기본값은 비활성"), DefaultProjectileData->TrailFxSettings.bEnabled);
	TestFalse(TEXT("기존 ProjectileData 호환을 위해 추진 FX 기본값은 비활성"), DefaultProjectileData->ThrusterFxSettings.bEnabled);
	TestEqual(TEXT("Trail 기본 소켓은 FX_Trail"), DefaultProjectileData->TrailFxSettings.AttachSocketName, FName(TEXT("FX_Trail")));
	TestEqual(TEXT("추진 기본 소켓은 FX_Exhaust"), DefaultProjectileData->ThrusterFxSettings.AttachSocketName, FName(TEXT("FX_Exhaust")));
	TestEqual(TEXT("Trail 기본 부착 모드는 소켓 우선 Fallback"), DefaultProjectileData->TrailFxSettings.AttachMode, ECFProjectileFxAttachMode::MeshSocketWithFallback);
	TestEqual(TEXT("추진 기본 부착 모드는 소켓 우선 Fallback"), DefaultProjectileData->ThrusterFxSettings.AttachMode, ECFProjectileFxAttachMode::MeshSocketWithFallback);

	// [v1.0.0] 생성자에서 두 NiagaraComponent가 영구 서브오브젝트로 구성됐는지 확인할 Actor CDO입니다.
	const ACFProjectileActor* ProjectileActorCDO = GetDefault<ACFProjectileActor>();
	if (!TestNotNull(TEXT("Projectile Actor CDO가 존재해야 함"), ProjectileActorCDO))
	{
		return false;
	}

	// [v1.0.0] CDO에 등록된 모든 NiagaraComponent입니다.
	TInlineComponentArray<UNiagaraComponent*> NiagaraComponents;
	ProjectileActorCDO->GetComponents(NiagaraComponents);

	// [v1.0.0] Trail NiagaraComponent를 찾았는지 여부입니다.
	bool bFoundTrailNiagaraComponent = false;

	// [v1.0.0] 추진 NiagaraComponent를 찾았는지 여부입니다.
	bool bFoundThrusterNiagaraComponent = false;

	for (const UNiagaraComponent* NiagaraComponent : NiagaraComponents)
	{
		if (!NiagaraComponent)
		{
			continue;
		}

		if (NiagaraComponent->GetFName() == FName(TEXT("TrailNiagaraComponent")))
		{
			bFoundTrailNiagaraComponent = true;
			TestFalse(TEXT("Trail Niagara는 CDO에서 활성 상태가 아니어야 함"), NiagaraComponent->IsActive());
		}
		else if (NiagaraComponent->GetFName() == FName(TEXT("ThrusterNiagaraComponent")))
		{
			bFoundThrusterNiagaraComponent = true;
			TestFalse(TEXT("추진 Niagara는 CDO에서 활성 상태가 아니어야 함"), NiagaraComponent->IsActive());
		}
	}

	TestTrue(TEXT("Trail NiagaraComponent 기본 서브오브젝트 존재"), bFoundTrailNiagaraComponent);
	TestTrue(TEXT("추진 NiagaraComponent 기본 서브오브젝트 존재"), bFoundThrusterNiagaraComponent);

	// [v1.0.0] 소켓 누락 Fallback과 비활성화 Reset을 검증할 Automation 월드입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Projectile Flight FX 테스트 월드가 생성돼야 함"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 실제 활성화와 비활성화 흐름을 검증할 Projectile Actor입니다.
	ACFProjectileActor* ProjectileActor = TestWorld->SpawnActor<ACFProjectileActor>();
	if (!TestNotNull(TEXT("Projectile Flight FX 테스트 Actor가 생성돼야 함"), ProjectileActor))
	{
		return false;
	}

	ProjectileActor->SetDestroyWhenDeactivated(false);

	// [v1.0.0] Niagara 자산 없이 소켓 Fallback과 MissingSystem 상태를 검증할 임시 ProjectileData입니다.
	UCFProjectileData* TestProjectileData = NewObject<UCFProjectileData>(ProjectileActor);
	if (!TestNotNull(TEXT("Projectile Flight FX 테스트 Data가 생성돼야 함"), TestProjectileData))
	{
		ProjectileActor->Destroy();
		return false;
	}

	TestProjectileData->InitialSpeed = 1.0f;
	TestProjectileData->LifeTimeSeconds = 10.0f;
	TestProjectileData->bUseSupplementalContinuousSweep = false;
	TestProjectileData->TrailFxSettings.bEnabled = true;
			TestProjectileData->TrailFxSettings.AttachMode = ECFProjectileFxAttachMode::MeshSocketWithFallback;
	TestProjectileData->TrailFxSettings.AttachSocketName = TEXT("FX_Trail_MissingForTest");
	TestProjectileData->TrailFxSettings.RelativeTransform.SetScale3D(FVector(0.25f));
	TestProjectileData->ThrusterFxSettings.bEnabled = true;
	TestProjectileData->ThrusterFxSettings.AttachMode = ECFProjectileFxAttachMode::ProjectileRelative;
	TestProjectileData->ThrusterFxSettings.RelativeTransform.SetScale3D(FVector(2.0f));

	ProjectileActor->ActivateProjectile(TestProjectileData, FVector::ForwardVector, nullptr);
	TestTrue(TEXT("Niagara 자산이 없어도 Projectile은 정상 활성화"), ProjectileActor->IsProjectileActive());

	// [v1.0.0] 활성화 직후 비행 FX 상태와 부착 출처를 확인할 요약입니다.
	const FString ActiveFlightFxSummary = ProjectileActor->BuildProjectileFlightFxSummary();
	TestTrue(TEXT("Trail 소켓 누락은 MissingSocketFallback으로 기록"), ActiveFlightFxSummary.Contains(TEXT("TrailAttachment=MissingSocketFallback:FX_Trail_MissingForTest")));
	TestTrue(TEXT("Trail Niagara 미연결은 MissingSystem으로 기록"), ActiveFlightFxSummary.Contains(TEXT("TrailStatus=MissingSystem")));
	TestTrue(TEXT("추진 FX Projectile Relative 부착 출처 기록"), ActiveFlightFxSummary.Contains(TEXT("ThrusterAttachment=ProjectileRelative")));
	TestTrue(TEXT("추진 Niagara 미연결은 MissingSystem으로 기록"), ActiveFlightFxSummary.Contains(TEXT("ThrusterStatus=MissingSystem")));
	TestTrue(TEXT("Trail 독립 FX Scale이 Debug 요약에 반영"), ActiveFlightFxSummary.Contains(TEXT("TrailScale=(0.250, 0.250, 0.250)")));
	TestTrue(TEXT("추진 독립 FX Scale이 Debug 요약에 반영"), ActiveFlightFxSummary.Contains(TEXT("ThrusterScale=(2.000, 2.000, 2.000)")));

	// [v1.1.0] 실제 Origin 컴포넌트 Scale을 확인하기 위해 Actor의 모든 SceneComponent를 수집합니다.
	TInlineComponentArray<USceneComponent*> SceneComponents;
	ProjectileActor->GetComponents(SceneComponents);

	// [v1.1.0] 테스트 Actor에서 찾은 Trail 원점 컴포넌트입니다.
	USceneComponent* TrailOriginComponent = nullptr;

	// [v1.1.0] 테스트 Actor에서 찾은 추진 FX 원점 컴포넌트입니다.
	USceneComponent* ThrusterOriginComponent = nullptr;

	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (!SceneComponent)
		{
			continue;
		}

		if (SceneComponent->GetFName() == FName(TEXT("TrailOriginComponent")))
		{
			TrailOriginComponent = SceneComponent;
		}
		else if (SceneComponent->GetFName() == FName(TEXT("ThrusterOriginComponent")))
		{
			ThrusterOriginComponent = SceneComponent;
		}
	}

	TestNotNull(TEXT("Trail 원점 컴포넌트를 찾을 수 있어야 함"), TrailOriginComponent);
	TestNotNull(TEXT("추진 FX 원점 컴포넌트를 찾을 수 있어야 함"), ThrusterOriginComponent);
	if (TrailOriginComponent)
	{
		TestTrue(TEXT("Trail Origin에 0.25 독립 Scale 적용"), TrailOriginComponent->GetComponentScale().Equals(FVector(0.25f), KINDA_SMALL_NUMBER));
	}
	if (ThrusterOriginComponent)
	{
		TestTrue(TEXT("Thruster Origin에 2.0 독립 Scale 적용"), ThrusterOriginComponent->GetComponentScale().Equals(FVector(2.0f), KINDA_SMALL_NUMBER));
	}

	ProjectileActor->DeactivateProjectile();
	TestFalse(TEXT("수동 비활성화 후 Projectile 비활성"), ProjectileActor->IsProjectileActive());

	// [v1.0.0] Pool 반환 전 Reset과 동일한 비활성화 정리 결과를 확인할 요약입니다.
	const FString InactiveFlightFxSummary = ProjectileActor->BuildProjectileFlightFxSummary();
	TestTrue(TEXT("비활성화 후 Trail 상태는 Inactive"), InactiveFlightFxSummary.Contains(TEXT("TrailStatus=Inactive")));
	TestTrue(TEXT("비활성화 후 추진 상태는 Inactive"), InactiveFlightFxSummary.Contains(TEXT("ThrusterStatus=Inactive")));
	TestTrue(TEXT("비활성화 후 Trail 부착 출처 초기화"), InactiveFlightFxSummary.Contains(TEXT("TrailAttachment=None")));
			TestTrue(TEXT("비활성화 후 추진 부착 출처 초기화"), InactiveFlightFxSummary.Contains(TEXT("ThrusterAttachment=None")));
	TestTrue(TEXT("비활성화 후 Trail Scale은 1로 Reset"), InactiveFlightFxSummary.Contains(TEXT("TrailScale=(1.000, 1.000, 1.000)")));
	TestTrue(TEXT("비활성화 후 추진 Scale은 1로 Reset"), InactiveFlightFxSummary.Contains(TEXT("ThrusterScale=(1.000, 1.000, 1.000)")));

	ProjectileActor->Destroy();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
