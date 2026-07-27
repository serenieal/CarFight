// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-24
// Description: TS-P0-04 선택 대상 수명, 가림 유예와 안전 해제 자동화 테스트

#include "CFTargetSelectComp.h"
#include "CFTargetSelectContractTestTypes.h"
#include "CFDamageData.h"
#include "CFVehicleHealthComp.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
	ACFTargetSelectContractActor* SpawnLifetimeTarget(
		UWorld* TestWorld,
		const FName ActorName,
		const FVector& ActorLocation)
	{
		if (!TestWorld)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ActorName;
		ACFTargetSelectContractActor* TargetActor = TestWorld->SpawnActor<ACFTargetSelectContractActor>(
			ACFTargetSelectContractActor::StaticClass(),
			ActorLocation,
			FRotator::ZeroRotator,
			SpawnParameters);

		if (TargetActor)
		{
			TargetActor->DisplayInfo.TargetId = ActorName;
			TargetActor->DisplayInfo.DisplayName = FText::FromName(ActorName);
			TargetActor->DisplayInfo.TargetCategory = ECFTargetCategory::Vehicle;
			TargetActor->DisplayInfo.Relation = ECFTargetRelation::Unknown;
			TargetActor->DisplayInfo.InformationLevel = ECFTargetInfoLevel::Identified;
		}

		return TargetActor;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetSelectionLifetimeTest,
	"CarFight.TargetSelect.TS_P0_04.SelectionLifetime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFTargetSelectionLifetimeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	TestNotNull(TEXT("선택 수명 테스트 월드 생성"), TestWorld);
	if (!TestWorld)
	{
		return false;
	}

	ACFTargetSelectContractActor* OwnerActor = SpawnLifetimeTarget(TestWorld, TEXT("LifetimeOwner"), FVector::ZeroVector);
	ACFTargetSelectContractActor* OcclusionTarget = SpawnLifetimeTarget(TestWorld, TEXT("OcclusionTarget"), FVector(5000.0f, 0.0f, 0.0f));
	ACFTargetSelectContractActor* CandidateTarget = SpawnLifetimeTarget(TestWorld, TEXT("CandidateTarget"), FVector(4500.0f, 100.0f, 0.0f));
	ACFTargetSelectContractActor* OldTarget = SpawnLifetimeTarget(TestWorld, TEXT("OldTarget"), FVector(4000.0f, -100.0f, 0.0f));
	ACFTargetSelectContractActor* NewTarget = SpawnLifetimeTarget(TestWorld, TEXT("NewTarget"), FVector(4200.0f, 100.0f, 0.0f));
	ACFTargetSelectContractActor* HealthTarget = SpawnLifetimeTarget(TestWorld, TEXT("HealthTarget"), FVector(3500.0f, 0.0f, 0.0f));
	ACFTargetSelectContractActor* FarTarget = SpawnLifetimeTarget(TestWorld, TEXT("FarTarget"), FVector(2000.0f, 0.0f, 0.0f));
	ACFTargetSelectContractActor* DeactivateTarget = SpawnLifetimeTarget(TestWorld, TEXT("DeactivateTarget"), FVector(3000.0f, 0.0f, 0.0f));

	if (!TestNotNull(TEXT("수명 Owner 생성"), OwnerActor)
		|| !TestNotNull(TEXT("가림 대상 생성"), OcclusionTarget)
		|| !TestNotNull(TEXT("후보 대상 생성"), CandidateTarget)
		|| !TestNotNull(TEXT("이전 선택 대상 생성"), OldTarget)
		|| !TestNotNull(TEXT("신규 선택 대상 생성"), NewTarget)
		|| !TestNotNull(TEXT("Health 대상 생성"), HealthTarget)
		|| !TestNotNull(TEXT("거리 이탈 대상 생성"), FarTarget)
		|| !TestNotNull(TEXT("비활성화 대상 생성"), DeactivateTarget))
	{
		return false;
	}

	UCFTargetSelectComp* TargetSelectComp = NewObject<UCFTargetSelectComp>(OwnerActor, TEXT("TargetLifetimeComp"));
	OwnerActor->AddInstanceComponent(TargetSelectComp);
	TargetSelectComp->RegisterComponent();
	TargetSelectComp->bAutoRefreshCandidate = false;
	TargetSelectComp->FallbackTargetSelectConfig.OcclusionGracePeriodSec = 1.5f;
	TargetSelectComp->FallbackTargetSelectConfig.DirectSelectMaxDistanceCm = 10000.0f;

	TStrongObjectPtr<UCFTargetSelectContractProbe> EventProbe(NewObject<UCFTargetSelectContractProbe>());
	TestNotNull(TEXT("수명 이벤트 Probe 생성"), EventProbe.Get());
	if (!EventProbe.IsValid())
	{
		return false;
	}

	TargetSelectComp->OnSelectedTargetCleared.AddDynamic(EventProbe.Get(), &UCFTargetSelectContractProbe::HandleSelectedTargetCleared);
	TargetSelectComp->OnSelectedTargetValidityChanged.AddDynamic(EventProbe.Get(), &UCFTargetSelectContractProbe::HandleSelectedTargetValidityChanged);
	TargetSelectComp->OnSelectedTargetTrackStateChanged.AddDynamic(EventProbe.Get(), &UCFTargetSelectContractProbe::HandleSelectedTargetTrackChanged);

	FCFTargetSelectionContext SelectionContext;
	SelectionContext.bRequireLineOfSightForNewSelection = false;

	FCFTargetCandidate CandidateData;
	CandidateData.TargetActor = CandidateTarget;
	CandidateData.DisplayInfo = CandidateTarget->DisplayInfo;
	TestTrue(TEXT("자동 다음 타겟 방지 검증용 후보 설정"), TargetSelectComp->SetCurrentCandidate(CandidateData, SelectionContext));
	TestTrue(TEXT("가림 유예 검증 대상 선택"), TargetSelectComp->SetSelectedTarget(OcclusionTarget, SelectionContext));

	EventProbe->ResetCounts();
	TestTrue(TEXT("가림 유예 중 선택 유지"), TargetSelectComp->UpdateSelectedTargetVisibility(false, 0.5f));
	TestEqual(TEXT("가림 유예 중 선택 대상 유지"), TargetSelectComp->GetSelectedTargetActor(), static_cast<AActor*>(OcclusionTarget));
	TestEqual(TEXT("가림 직후 추적 상태 Occluded"), TargetSelectComp->GetSelectedTargetTrackState(), ECFTargetTrackState::Occluded);
	TestTrue(TEXT("가림 경과 시간 0.5초"), FMath::IsNearlyEqual(TargetSelectComp->GetSelectedTargetOcclusionElapsedSeconds(), 0.5f));
	TestEqual(TEXT("유예 중 선택 해제 이벤트 없음"), EventProbe->SelectedTargetClearedCount, 0);

	OcclusionTarget->SetActorLocation(FVector(-5000.0f, 0.0f, 0.0f));
	TestTrue(TEXT("화면 뒤쪽이어도 월드 가시 입력이 True이면 선택 유지"), TargetSelectComp->UpdateSelectedTargetVisibility(true, 0.1f));
	TestEqual(TEXT("가시 복구 후 추적 상태 Visible"), TargetSelectComp->GetSelectedTargetTrackState(), ECFTargetTrackState::Visible);
	TestTrue(TEXT("가시 복구 후 가림 시간 초기화"), FMath::IsNearlyZero(TargetSelectComp->GetSelectedTargetOcclusionElapsedSeconds()));

	TestTrue(TEXT("가림 1.0초는 유예 안에서 유지"), TargetSelectComp->UpdateSelectedTargetVisibility(false, 1.0f));
	TestFalse(TEXT("가림 누적 1.6초에서 선택 해제"), TargetSelectComp->UpdateSelectedTargetVisibility(false, 0.6f));
	TestFalse(TEXT("가림 유예 종료 후 선택 기록 없음"), TargetSelectComp->HasSelectedTarget());
	TestEqual(TEXT("가림 유예 종료 사유 InvalidTarget"), TargetSelectComp->GetLastSelectedTargetClearReason(), ECFTargetClearReason::InvalidTarget);
	TestEqual(TEXT("가림 유예 종료 유효성 이벤트 1회"), EventProbe->SelectedTargetValidityChangedCount, 1);
	TestEqual(TEXT("가림 유예 종료 해제 이벤트 1회"), EventProbe->SelectedTargetClearedCount, 1);
	TestEqual(TEXT("선택 해제 뒤 기존 후보는 유지"), TargetSelectComp->GetCurrentCandidateActor(), static_cast<AActor*>(CandidateTarget));
	TestNull(TEXT("선택 해제 뒤 후보가 자동 선택되지 않음"), TargetSelectComp->GetSelectedTargetActor());

	TestTrue(TEXT("이전 대상 선택"), TargetSelectComp->SetSelectedTarget(OldTarget, SelectionContext));
	TestTrue(TEXT("새 대상 선택으로 수명 구독 전환"), TargetSelectComp->SetSelectedTarget(NewTarget, SelectionContext));
	EventProbe->ResetCounts();
	TestTrue(TEXT("이전 대상 Destroy 성공"), OldTarget->Destroy());
	TestEqual(TEXT("이전 대상 제거는 현재 선택에 영향 없음"), TargetSelectComp->GetSelectedTargetActor(), static_cast<AActor*>(NewTarget));
	TestEqual(TEXT("이전 대상 제거로 해제 이벤트 없음"), EventProbe->SelectedTargetClearedCount, 0);

	TestTrue(TEXT("현재 선택 대상 Destroy 성공"), NewTarget->Destroy());
	TestFalse(TEXT("현재 선택 대상 Destroy 시 선택 기록 자동 해제"), TargetSelectComp->HasSelectedTarget());
	TestEqual(TEXT("Actor Destroy 해제 사유"), TargetSelectComp->GetLastSelectedTargetClearReason(), ECFTargetClearReason::Destroyed);
	TestEqual(TEXT("Actor Destroy 유효성 이벤트 1회"), EventProbe->SelectedTargetValidityChangedCount, 1);
	TestEqual(TEXT("Actor Destroy 해제 이벤트 1회"), EventProbe->SelectedTargetClearedCount, 1);

	UCFVehicleHealthComp* HealthComponent = NewObject<UCFVehicleHealthComp>(HealthTarget, TEXT("VehicleHealthComp_Test"));
	HealthTarget->AddInstanceComponent(HealthComponent);
	HealthComponent->RegisterComponent();
	HealthComponent->InitializeFromVehicleData(nullptr);

	UCFDamageData* FatalDamageData = NewObject<UCFDamageData>(OwnerActor, TEXT("FatalDamageData"));
	FatalDamageData->BaseDamage = 200.0f;

	EventProbe->ResetCounts();
	TestTrue(TEXT("Health 파괴 검증 대상 선택"), TargetSelectComp->SetSelectedTarget(HealthTarget, SelectionContext));
	FCFDamageHitContext DamageHitContext;
	DamageHitContext.DamageData = FatalDamageData;
	DamageHitContext.HitActor = HealthTarget;
	DamageHitContext.InstigatorActor = OwnerActor;
	DamageHitContext.bBlockingHit = true;
	FCFDamageApplyResult DamageApplyResult;
	TestTrue(TEXT("치명 피해 적용 성공"), HealthComponent->ApplyDamageFromHitContext(DamageHitContext, DamageApplyResult));
	TestTrue(TEXT("치명 피해로 Health 파괴 상태"), HealthComponent->IsDestroyed());
	TestFalse(TEXT("Health 파괴 이벤트로 선택 자동 해제"), TargetSelectComp->HasSelectedTarget());
	TestEqual(TEXT("Health 파괴 해제 사유"), TargetSelectComp->GetLastSelectedTargetClearReason(), ECFTargetClearReason::Destroyed);
	TestEqual(TEXT("Health 파괴 유효성 이벤트 1회"), EventProbe->SelectedTargetValidityChangedCount, 1);
	TestEqual(TEXT("Health 파괴 해제 이벤트 1회"), EventProbe->SelectedTargetClearedCount, 1);
	TestFalse(TEXT("파괴된 Health 대상은 재선택 불가"), TargetSelectComp->CanUseActorAsTarget(HealthTarget, SelectionContext));

	TargetSelectComp->FallbackTargetSelectConfig.DirectSelectMaxDistanceCm = 1000.0f;
	EventProbe->ResetCounts();
	TestTrue(TEXT("거리 이탈 검증 대상 선택"), TargetSelectComp->SetSelectedTarget(FarTarget, SelectionContext));
	TestFalse(TEXT("최대 추적 거리 밖 대상 수명 갱신 실패"), TargetSelectComp->RefreshSelectedTargetLifetime(0.1f));
	TestFalse(TEXT("거리 이탈 후 선택 기록 없음"), TargetSelectComp->HasSelectedTarget());
	TestEqual(TEXT("거리 이탈 해제 사유"), TargetSelectComp->GetLastSelectedTargetClearReason(), ECFTargetClearReason::OutOfTrackingRange);
	TestEqual(TEXT("거리 이탈 유효성 이벤트 1회"), EventProbe->SelectedTargetValidityChangedCount, 1);
	TestEqual(TEXT("거리 이탈 해제 이벤트 1회"), EventProbe->SelectedTargetClearedCount, 1);

	TargetSelectComp->FallbackTargetSelectConfig.DirectSelectMaxDistanceCm = 10000.0f;
	EventProbe->ResetCounts();
	TestTrue(TEXT("컴포넌트 비활성화 검증 대상 선택"), TargetSelectComp->SetSelectedTarget(DeactivateTarget, SelectionContext));
	TargetSelectComp->Deactivate();
	TestFalse(TEXT("컴포넌트 비활성화 시 선택 기록 해제"), TargetSelectComp->HasSelectedTarget());
	TestFalse(TEXT("컴포넌트 비활성화 시 후보도 해제"), TargetSelectComp->HasCurrentCandidate());
	TestEqual(TEXT("컴포넌트 비활성화 해제 사유"), TargetSelectComp->GetLastSelectedTargetClearReason(), ECFTargetClearReason::SystemDisabled);
	TestEqual(TEXT("컴포넌트 비활성화 유효성 이벤트 1회"), EventProbe->SelectedTargetValidityChangedCount, 1);
	TestEqual(TEXT("컴포넌트 비활성화 해제 이벤트 1회"), EventProbe->SelectedTargetClearedCount, 1);

	TargetSelectComp->OnSelectedTargetCleared.RemoveAll(EventProbe.Get());
	TargetSelectComp->OnSelectedTargetValidityChanged.RemoveAll(EventProbe.Get());
	TargetSelectComp->OnSelectedTargetTrackStateChanged.RemoveAll(EventProbe.Get());

	OcclusionTarget->Destroy();
	CandidateTarget->Destroy();
	HealthTarget->Destroy();
	FarTarget->Destroy();
	DeactivateTarget->Destroy();
	OwnerActor->Destroy();
	return true;
}

#endif
