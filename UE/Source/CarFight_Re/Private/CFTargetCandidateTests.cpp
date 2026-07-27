// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-24
// Description: TS-P0-03 후보 탐색, 계층형 정렬과 안정화 자동화 테스트

#include "CFTargetSelectComp.h"
#include "CFTargetSelectContractTestTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	ACFTargetSelectContractActor* SpawnCandidateActor(UWorld* TestWorld, const FName ActorName, const FVector& ActorLocation, const FName TargetId)
	{
		if (!TestWorld)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ActorName;
		ACFTargetSelectContractActor* CandidateActor = TestWorld->SpawnActor<ACFTargetSelectContractActor>(ACFTargetSelectContractActor::StaticClass(), ActorLocation, FRotator::ZeroRotator, SpawnParameters);
		if (CandidateActor)
		{
			CandidateActor->DisplayInfo.TargetId = TargetId;
			CandidateActor->DisplayInfo.DisplayName = FText::FromName(TargetId);
			CandidateActor->DisplayInfo.TargetCategory = ECFTargetCategory::Vehicle;
			CandidateActor->DisplayInfo.Relation = ECFTargetRelation::Unknown;
		}
		return CandidateActor;
	}

	FCFTargetSearchView BuildDefaultSearchView()
	{
		FCFTargetSearchView SearchView;
		SearchView.ViewOrigin = FVector::ZeroVector;
		SearchView.ViewDirection = FVector::ForwardVector;
		SearchView.ViewUpDirection = FVector::UpVector;
		SearchView.VerticalFOVDeg = 60.0f;
		SearchView.ViewportAspectRatio = 16.0f / 9.0f;
		return SearchView;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetCandidateRankingTest,
	"CarFight.TargetSelect.TS_P0_03.CandidateRanking",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFTargetCandidateRankingTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	TestNotNull(TEXT("테스트 월드 생성"), TestWorld);
	if (!TestWorld)
	{
		return false;
	}

	AActor* OwnerActor = TestWorld->SpawnActor<AActor>();
	TestNotNull(TEXT("TargetSelectComp Owner 생성"), OwnerActor);
	if (!OwnerActor)
	{
		return false;
	}

	UCFTargetSelectComp* TargetSelectComp = NewObject<UCFTargetSelectComp>(OwnerActor, TEXT("TargetSelectComp_Test"));
	OwnerActor->AddInstanceComponent(TargetSelectComp);
	TargetSelectComp->RegisterComponent();
	TargetSelectComp->bAutoRefreshCandidate = false;
	TargetSelectComp->FallbackTargetSelectConfig.DirectSelectMaxDistanceCm = 200000.0f;
	TargetSelectComp->FallbackTargetSelectConfig.ProximitySelectMaxDistanceCm = 120000.0f;
	TargetSelectComp->FallbackTargetSelectConfig.ProximityHalfAngleDeg = 20.0f;
	TargetSelectComp->FallbackTargetSelectConfig.CandidateSwitchAdvantageRatio = 0.15f;

	FCFTargetSelectionContext SelectionContext;
	SelectionContext.bRequireLineOfSightForNewSelection = false;

	ACFTargetSelectContractActor* CenterFarActor = SpawnCandidateActor(TestWorld, TEXT("CenterFarActor"), FVector(10000.0f, 0.0f, 0.0f), TEXT("CenterFar"));
	ACFTargetSelectContractActor* EdgeNearActor = SpawnCandidateActor(TestWorld, TEXT("EdgeNearActor"), FVector(3000.0f, 300.0f, 0.0f), TEXT("EdgeNear"));
	TestNotNull(TEXT("중앙 원거리 후보 생성"), CenterFarActor);
	TestNotNull(TEXT("가장자리 근거리 후보 생성"), EdgeNearActor);

	TArray<AActor*> PrimaryCandidates;
	PrimaryCandidates.Add(CenterFarActor);
	PrimaryCandidates.Add(EdgeNearActor);

	FCFTargetSearchView SearchView = BuildDefaultSearchView();
	SearchView.DirectAimHitActor = EdgeNearActor;
	FCFTargetSearchResult SearchResult = TargetSelectComp->EvaluateCandidateActors(PrimaryCandidates, SearchView, SelectionContext);
	TestTrue(TEXT("직접 조준 결과 존재"), SearchResult.bHasBestCandidate);
	TestEqual(TEXT("직접 조준 후보가 화면 중앙 후보보다 우선"), SearchResult.BestCandidate.TargetActor.Get(), static_cast<AActor*>(EdgeNearActor));
	TestEqual(TEXT("직접 조준 후보 집계"), SearchResult.DirectAimCandidateCount, 1);

	SearchView.DirectAimHitActor = nullptr;
	SearchResult = TargetSelectComp->EvaluateCandidateActors(PrimaryCandidates, SearchView, SelectionContext);
	TestEqual(TEXT("직접 대상이 없으면 화면 중앙 원거리 후보 우선"), SearchResult.BestCandidate.TargetActor.Get(), static_cast<AActor*>(CenterFarActor));

	ACFTargetSelectContractActor* CenterNearActor = SpawnCandidateActor(TestWorld, TEXT("CenterNearActor"), FVector(5000.0f, 0.0f, 0.0f), TEXT("CenterNear"));
	TArray<AActor*> CenterDistanceCandidates;
	CenterDistanceCandidates.Add(CenterFarActor);
	CenterDistanceCandidates.Add(CenterNearActor);
	SearchResult = TargetSelectComp->EvaluateCandidateActors(CenterDistanceCandidates, SearchView, SelectionContext);
	TestEqual(TEXT("화면 거리가 같으면 월드 거리 우선"), SearchResult.BestCandidate.TargetActor.Get(), static_cast<AActor*>(CenterNearActor));

	ACFTargetSelectContractActor* AlphaActor = SpawnCandidateActor(TestWorld, TEXT("StableAlphaActor"), FVector(7000.0f, 350.0f, 0.0f), TEXT("Alpha"));
	ACFTargetSelectContractActor* BetaActor = SpawnCandidateActor(TestWorld, TEXT("StableBetaActor"), FVector(7000.0f, 350.0f, 0.0f), TEXT("Beta"));
	TArray<AActor*> StableForwardOrder;
	StableForwardOrder.Add(AlphaActor);
	StableForwardOrder.Add(BetaActor);
	TArray<AActor*> StableReverseOrder;
	StableReverseOrder.Add(BetaActor);
	StableReverseOrder.Add(AlphaActor);
	const FCFTargetSearchResult StableForwardResult = TargetSelectComp->EvaluateCandidateActors(StableForwardOrder, SearchView, SelectionContext);
	const FCFTargetSearchResult StableReverseResult = TargetSelectComp->EvaluateCandidateActors(StableReverseOrder, SearchView, SelectionContext);
	TestEqual(TEXT("안정 키가 같은 평가 조건의 최종 순서를 결정"), StableForwardResult.BestCandidate.TargetActor.Get(), static_cast<AActor*>(AlphaActor));
	TestEqual(TEXT("입력 배열 순서를 바꿔도 최종 결과 동일"), StableReverseResult.BestCandidate.TargetActor.Get(), static_cast<AActor*>(AlphaActor));

	ACFTargetSelectContractActor* CurrentActor = SpawnCandidateActor(TestWorld, TEXT("CurrentStableActor"), FVector(5000.0f, 1026.0f, 0.0f), TEXT("CurrentStable"));
	ACFTargetSelectContractActor* ChallengerActor = SpawnCandidateActor(TestWorld, TEXT("ChallengerActor"), FVector(5000.0f, 923.0f, 0.0f), TEXT("Challenger"));
	TArray<AActor*> CurrentOnlyCandidates;
	CurrentOnlyCandidates.Add(CurrentActor);
	const FCFTargetSearchResult CurrentOnlyResult = TargetSelectComp->EvaluateCandidateActors(CurrentOnlyCandidates, SearchView, SelectionContext);
	TestTrue(TEXT("안정화 기준 현재 후보 설정"), TargetSelectComp->SetCurrentCandidate(CurrentOnlyResult.BestCandidate, SelectionContext));

	TArray<AActor*> StabilityCandidates;
	StabilityCandidates.Add(CurrentActor);
	StabilityCandidates.Add(ChallengerActor);
	SearchResult = TargetSelectComp->EvaluateCandidateActors(StabilityCandidates, SearchView, SelectionContext);
	TestEqual(TEXT("근소한 화면 우위는 현재 후보 유지"), SearchResult.BestCandidate.TargetActor.Get(), static_cast<AActor*>(CurrentActor));
	TestTrue(TEXT("현재 후보 유지 사유 기록"), SearchResult.bKeptCurrentCandidateForStability);

	ChallengerActor->SetActorLocation(FVector(5000.0f, 700.0f, 0.0f));
	SearchResult = TargetSelectComp->EvaluateCandidateActors(StabilityCandidates, SearchView, SelectionContext);
	TestEqual(TEXT("충분한 화면 우위가 생기면 후보 전환"), SearchResult.BestCandidate.TargetActor.Get(), static_cast<AActor*>(ChallengerActor));
	TestFalse(TEXT("충분한 우위에서는 안정화 유지 안 함"), SearchResult.bKeptCurrentCandidateForStability);

	ChallengerActor->SetActorLocation(FVector(5000.0f, 923.0f, 0.0f));
	SearchView.DirectAimHitActor = ChallengerActor;
	SearchResult = TargetSelectComp->EvaluateCandidateActors(StabilityCandidates, SearchView, SelectionContext);
	TestEqual(TEXT("직접 조준은 전환 우위 비율과 관계없이 즉시 우선"), SearchResult.BestCandidate.TargetActor.Get(), static_cast<AActor*>(ChallengerActor));
	SearchView.DirectAimHitActor = nullptr;

	ACFTargetSelectContractActor* BehindActor = SpawnCandidateActor(TestWorld, TEXT("BehindActor"), FVector(-5000.0f, 0.0f, 0.0f), TEXT("Behind"));
	ACFTargetSelectContractActor* OutOfRangeActor = SpawnCandidateActor(TestWorld, TEXT("OutOfRangeActor"), FVector(150000.0f, 0.0f, 0.0f), TEXT("OutOfRange"));
	ACFTargetSelectContractActor* OutOfAngleActor = SpawnCandidateActor(TestWorld, TEXT("OutOfAngleActor"), FVector(10000.0f, 10000.0f, 0.0f), TEXT("OutOfAngle"));
	TArray<AActor*> RejectedCandidates;
	RejectedCandidates.Add(BehindActor);
	RejectedCandidates.Add(OutOfRangeActor);
	RejectedCandidates.Add(OutOfAngleActor);
	SearchResult = TargetSelectComp->EvaluateCandidateActors(RejectedCandidates, SearchView, SelectionContext);
	TestFalse(TEXT("후방 거리 밖 반각 밖 후보는 모두 제외"), SearchResult.bHasBestCandidate);
	TestEqual(TEXT("제외 후보 집계는 0"), SearchResult.AcceptedCandidateCount, 0);

	FCFTargetSearchView Aspect16View = BuildDefaultSearchView();
	FCFTargetSearchView Aspect32View = Aspect16View;
	Aspect32View.ViewportAspectRatio = 32.0f / 9.0f;
	const FVector HorizontalTestLocation(1000.0f, 1000.0f, 0.0f);
	const float Aspect16Distance = UCFTargetSelectComp::CalculateNormalizedScreenDistance(Aspect16View, HorizontalTestLocation);
	const float Aspect32Distance = UCFTargetSelectComp::CalculateNormalizedScreenDistance(Aspect32View, HorizontalTestLocation);
	TestTrue(TEXT("울트라와이드에서는 같은 수평 각도의 정규화 화면 거리가 감소"), Aspect32Distance < Aspect16Distance);

	FCFTargetSearchView FOV90View = Aspect16View;
	FOV90View.VerticalFOVDeg = 90.0f;
	const FVector VerticalTestLocation(1000.0f, 0.0f, 500.0f);
	const float FOV60Distance = UCFTargetSelectComp::CalculateNormalizedScreenDistance(Aspect16View, VerticalTestLocation);
	const float FOV90Distance = UCFTargetSelectComp::CalculateNormalizedScreenDistance(FOV90View, VerticalTestLocation);
	TestTrue(TEXT("넓은 FOV에서는 같은 각도의 정규화 화면 거리가 감소"), FOV90Distance < FOV60Distance);

	ACFTargetSelectContractActor* OffsetActor = SpawnCandidateActor(TestWorld, TEXT("OffsetTargetActor"), FVector(5000.0f, 0.0f, 0.0f), TEXT("OffsetTarget"));
	OffsetActor->SelectionLocationOffset = FVector(0.0f, 500.0f, 0.0f);
	TArray<AActor*> OffsetCandidates;
	OffsetCandidates.Add(OffsetActor);
	SearchResult = TargetSelectComp->EvaluateCandidateActors(OffsetCandidates, SearchView, SelectionContext);
	TestTrue(TEXT("대표 선택 위치 후보 생성"), SearchResult.bHasBestCandidate);
	TestTrue(TEXT("인터페이스 대표 위치가 후보 월드 위치에 반영"), SearchResult.BestCandidate.TargetWorldLocation.Equals(FVector(5000.0f, 500.0f, 0.0f), KINDA_SMALL_NUMBER));

	TArray<AActor*> InvalidCandidates;
	InvalidCandidates.Add(nullptr);
	SearchResult = TargetSelectComp->EvaluateCandidateActors(InvalidCandidates, SearchView, SelectionContext);
	TestFalse(TEXT("Null 후보 안전 거부"), SearchResult.bHasBestCandidate);

	return true;
}

#endif
