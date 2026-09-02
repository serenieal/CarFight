// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-08-31
// Description: CF-FQ-026 TS-P0-08 단일 후보 범위·대칭성·Target Registry 검색 진단·LOS 사전필터·실제 설정 경로 자동화 테스트
// Scope: 현재 fallback 튜닝값과 단일 후보의 좌우 반각 경계를 Transient World에서 자산 저장 없이 검증합니다.
// Changelog:
// - v1.4.0: SearchDiagnostics가 반복 World Actor scan 0과 Registry snapshot 사용을 검증하고 RegistryLifecycle로 초기 bootstrap 이후 spawn 증분 등록·파괴 prune를 추가.
// - v1.3.0: SearchDiagnostics에 반각 밖·거리 밖 Targetable Actor를 추가해 후보 입력은 유지하면서 LOS Trace 2건을 사전 생략하는 계약을 검증.
// - v1.2.0: BP_CFVehiclePawn GeneratedClass CDO를 Load-only로 읽어 실제 TargetSelectData source와 resolved config를 기록하는 SettingsPath 회귀와 SearchDiagnostics 샘플 로그를 추가.
// - v1.1.0: Transient ACFVehiclePawn의 실제 RefreshCurrentCandidate 경로에서 월드 Actor 스캔·Visibility/전체 Trace·검색시간 진단이 채워지는 SearchDiagnostics 회귀를 추가.
// - v1.0.0: 7도 근접 반각의 좌우 6.9도 수락·7.1도 거부, 16:9·32:9 각도 수락 대칭과 현재 fallback 설정 계약을 추가.
// Migration:
// - v1.4.0 성능 교정은 후보 공급원만 Registry로 변경하며 기존 7도·1200m·15% 튜닝값과 직접 조준/LOS 의미는 변경하지 않습니다.
// - 사용자 감각 검증을 대신해 ProximityHalfAngleDeg, 거리 또는 CandidateSwitchAdvantageRatio를 변경하지 않습니다.
// - 정확히 한 후보만 평가해 후보 경쟁과 15% 히스테리시스를 이번 원인 분리에서 제외합니다.
// - WBP_TargetSelect, Input Action, Mapping Context와 다른 Content Asset을 생성·수정·저장하지 않습니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFTargetSelectComp.h"
#include "CFTargetRegistrySubsystem.h"
#include "CFTargetSelectContractTestTypes.h"
#include "CFTargetSelectData.h"
#include "CFTargetPointComp.h"
#include "CFVehiclePawn.h"
#include "Engine/Blueprint.h"

#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	// [v1.0.0] 지정 화면비로 TS-P0-08 단일 후보 경계 검증용 카메라 View를 생성합니다.
	FCFTargetSearchView BuildTuneSearchView(const float ViewportAspectRatio)
	{
		// [v1.0.0] 좌우 후보를 동일한 시선·FOV 조건에서 평가할 검색 View입니다.
		FCFTargetSearchView SearchView;
		SearchView.ViewOrigin = FVector::ZeroVector;
		SearchView.ViewDirection = FVector::ForwardVector;
		SearchView.ViewUpDirection = FVector::UpVector;
		SearchView.VerticalFOVDeg = 60.0f;
		SearchView.ViewportAspectRatio = ViewportAspectRatio;
		return SearchView;
	}

	// [v1.0.0] 전방 거리와 수평 각도를 정확한 월드 위치로 변환합니다.
	FVector BuildLocationAtAngle(const float ForwardDistanceCm, const float HorizontalAngleDeg)
	{
		// [v1.0.0] 요청한 수평 각도를 만들 월드 Y축 오프셋입니다.
		const float LateralOffsetCm = FMath::Tan(FMath::DegreesToRadians(HorizontalAngleDeg)) * ForwardDistanceCm;
		return FVector(ForwardDistanceCm, LateralOffsetCm, 0.0f);
	}

	// [v1.0.0] Content Asset 없이 Transient World에 선택 가능한 계약 Actor를 생성합니다.
	ACFTargetSelectContractActor* SpawnTuneTarget(UWorld* TestWorld, const FName ActorName, const FVector& ActorLocation)
	{
		if (!TestWorld)
		{
			return nullptr;
		}

		// [v1.0.0] 테스트 Actor의 결정적 이름을 지정할 Spawn 설정입니다.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ActorName;

		// [v1.0.0] 기존 TargetSelectable 테스트 계약을 구현한 Transient Actor입니다.
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
		}
		return TargetActor;
	}

	// [v1.0.0] 정확히 한 Actor만 후보로 평가해 히스테리시스·후보 경쟁 없는 결과를 반환합니다.
	FCFTargetSearchResult EvaluateOneTarget(
		UCFTargetSelectComp* TargetSelectComp,
		AActor* TargetActor,
		const FCFTargetSearchView& SearchView,
		const FCFTargetSelectionContext& SelectionContext)
	{
		// [v1.0.0] 후보 경쟁을 제거하기 위해 대상 Actor 하나만 담는 평가 입력입니다.
		TArray<AActor*> CandidateActors;
		CandidateActors.Add(TargetActor);
		return TargetSelectComp
			? TargetSelectComp->EvaluateCandidateActors(CandidateActors, SearchView, SelectionContext)
			: FCFTargetSearchResult();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetTuneBoundaryTest,
	"CarFight.TargetSelect.TS_P0_08.SingleTargetBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 현재 production fallback 설정과 단일 후보 좌우 반각 대칭성을 검증합니다.
bool FCFTargetTuneBoundaryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 현재 소스의 production fallback 설정을 임의 변경 없이 검증할 값입니다.
	const FCFTargetSelectConfig ProductionFallbackConfig;
	TestEqual(TEXT("Fallback 직접 선택 최대 거리 2000m"), ProductionFallbackConfig.DirectSelectMaxDistanceCm, 200000.0f);
	TestEqual(TEXT("Fallback 근접 선택 최대 거리 1200m"), ProductionFallbackConfig.ProximitySelectMaxDistanceCm, 120000.0f);
	TestEqual(TEXT("Fallback 근접 후보 반각 7도"), ProductionFallbackConfig.ProximityHalfAngleDeg, 7.0f);
	TestEqual(TEXT("Fallback 후보 갱신 간격 0.05초"), ProductionFallbackConfig.CandidateRefreshIntervalSec, 0.05f);
	TestEqual(TEXT("Fallback 가림 유예 1.5초"), ProductionFallbackConfig.OcclusionGracePeriodSec, 1.5f);
	TestEqual(TEXT("Fallback 후보 전환 우위 15퍼센트"), ProductionFallbackConfig.CandidateSwitchAdvantageRatio, 0.15f);

	// [v1.0.0] Content 저장 없이 후보 평가를 수행할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("TS-P0-08 Transient World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.0.0] 테스트 TargetSelectComp를 소유할 임시 Actor입니다.
	AActor* OwnerActor = TestWorld->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("TS-P0-08 Owner Actor 생성"), OwnerActor))
	{
		return false;
	}

	// [v1.0.0] 자동 Tick 검색을 사용하지 않고 순수 후보 평가만 수행할 컴포넌트입니다.
	UCFTargetSelectComp* TargetSelectComp = NewObject<UCFTargetSelectComp>(OwnerActor, TEXT("TargetSelectComp_TS_P0_08"));
	if (!TestNotNull(TEXT("TS-P0-08 TargetSelectComp 생성"), TargetSelectComp))
	{
		return false;
	}
	OwnerActor->AddInstanceComponent(TargetSelectComp);
	TargetSelectComp->RegisterComponent();
	TargetSelectComp->bAutoRefreshCandidate = false;
	TargetSelectComp->TargetSelectData = nullptr;
	TargetSelectComp->FallbackTargetSelectConfig = ProductionFallbackConfig;

	// [v1.0.0] 이번 테스트를 순수 각도·거리 판정으로 한정하기 위해 LOS Trace를 제외한 컨텍스트입니다.
	FCFTargetSelectionContext SelectionContext;
	SelectionContext.bRequireLineOfSightForNewSelection = false;

	// [v1.0.0] 7도 경계 안쪽을 부동소수점 오차와 분리해 검증할 각도입니다.
	constexpr float InsideHalfAngleDeg = 6.9f;
	// [v1.0.0] 7도 경계 바깥쪽을 부동소수점 오차와 분리해 검증할 각도입니다.
	constexpr float OutsideHalfAngleDeg = 7.1f;
	// [v1.0.0] 1200m 거리 제한보다 충분히 가까워 각도 판정만 분리할 전방 거리입니다.
	constexpr float TestForwardDistanceCm = 10000.0f;

	// [v1.0.0] 좌측 6.9도 경계 안 후보입니다.
	ACFTargetSelectContractActor* LeftInsideActor = SpawnTuneTarget(TestWorld, TEXT("LeftInside"), BuildLocationAtAngle(TestForwardDistanceCm, -InsideHalfAngleDeg));
	// [v1.0.0] 우측 6.9도 경계 안 후보입니다.
	ACFTargetSelectContractActor* RightInsideActor = SpawnTuneTarget(TestWorld, TEXT("RightInside"), BuildLocationAtAngle(TestForwardDistanceCm, InsideHalfAngleDeg));
	// [v1.0.0] 좌측 7.1도 경계 밖 후보입니다.
	ACFTargetSelectContractActor* LeftOutsideActor = SpawnTuneTarget(TestWorld, TEXT("LeftOutside"), BuildLocationAtAngle(TestForwardDistanceCm, -OutsideHalfAngleDeg));
	// [v1.0.0] 우측 7.1도 경계 밖 후보입니다.
	ACFTargetSelectContractActor* RightOutsideActor = SpawnTuneTarget(TestWorld, TEXT("RightOutside"), BuildLocationAtAngle(TestForwardDistanceCm, OutsideHalfAngleDeg));
	if (!TestNotNull(TEXT("좌측 inside 후보 생성"), LeftInsideActor)
		|| !TestNotNull(TEXT("우측 inside 후보 생성"), RightInsideActor)
		|| !TestNotNull(TEXT("좌측 outside 후보 생성"), LeftOutsideActor)
		|| !TestNotNull(TEXT("우측 outside 후보 생성"), RightOutsideActor))
	{
		return false;
	}

	// [v1.0.0] 표준 16:9 화면비 검색 View입니다.
	const FCFTargetSearchView Aspect16View = BuildTuneSearchView(16.0f / 9.0f);
	// [v1.0.0] 울트라와이드 32:9 화면비 검색 View입니다.
	const FCFTargetSearchView Aspect32View = BuildTuneSearchView(32.0f / 9.0f);

	// [v1.0.0] 16:9 좌측 경계 안 단일 후보 결과입니다.
	const FCFTargetSearchResult LeftInside16 = EvaluateOneTarget(TargetSelectComp, LeftInsideActor, Aspect16View, SelectionContext);
	// [v1.0.0] 16:9 우측 경계 안 단일 후보 결과입니다.
	const FCFTargetSearchResult RightInside16 = EvaluateOneTarget(TargetSelectComp, RightInsideActor, Aspect16View, SelectionContext);
	TestTrue(TEXT("16:9 좌측 6.9도 수락"), LeftInside16.bHasBestCandidate);
	TestTrue(TEXT("16:9 우측 6.9도 수락"), RightInside16.bHasBestCandidate);
	if (LeftInside16.bHasBestCandidate && RightInside16.bHasBestCandidate)
	{
		TestTrue(TEXT("16:9 좌우 CrosshairAngle 동일"), FMath::IsNearlyEqual(LeftInside16.BestCandidate.CrosshairAngleDeg, RightInside16.BestCandidate.CrosshairAngleDeg, 0.001f));
		TestTrue(TEXT("16:9 좌우 화면 거리 동일"), FMath::IsNearlyEqual(LeftInside16.BestCandidate.NormalizedScreenDistance, RightInside16.BestCandidate.NormalizedScreenDistance, 0.001f));
	}

	// [v1.0.0] 16:9 좌우 경계 밖 단일 후보 결과입니다.
	const FCFTargetSearchResult LeftOutside16 = EvaluateOneTarget(TargetSelectComp, LeftOutsideActor, Aspect16View, SelectionContext);
	// [v1.0.0] 16:9 우측 경계 밖 단일 후보 결과입니다.
	const FCFTargetSearchResult RightOutside16 = EvaluateOneTarget(TargetSelectComp, RightOutsideActor, Aspect16View, SelectionContext);
	TestFalse(TEXT("16:9 좌측 7.1도 거부"), LeftOutside16.bHasBestCandidate);
	TestFalse(TEXT("16:9 우측 7.1도 거부"), RightOutside16.bHasBestCandidate);

	// [v1.0.0] 32:9 좌측 경계 안 단일 후보 결과입니다.
	const FCFTargetSearchResult LeftInside32 = EvaluateOneTarget(TargetSelectComp, LeftInsideActor, Aspect32View, SelectionContext);
	// [v1.0.0] 32:9 우측 경계 안 단일 후보 결과입니다.
	const FCFTargetSearchResult RightInside32 = EvaluateOneTarget(TargetSelectComp, RightInsideActor, Aspect32View, SelectionContext);
	// [v1.0.0] 32:9 좌측 경계 밖 단일 후보 결과입니다.
	const FCFTargetSearchResult LeftOutside32 = EvaluateOneTarget(TargetSelectComp, LeftOutsideActor, Aspect32View, SelectionContext);
	// [v1.0.0] 32:9 우측 경계 밖 단일 후보 결과입니다.
	const FCFTargetSearchResult RightOutside32 = EvaluateOneTarget(TargetSelectComp, RightOutsideActor, Aspect32View, SelectionContext);
	TestTrue(TEXT("32:9 좌측 6.9도 수락"), LeftInside32.bHasBestCandidate);
	TestTrue(TEXT("32:9 우측 6.9도 수락"), RightInside32.bHasBestCandidate);
	TestFalse(TEXT("32:9 좌측 7.1도 거부"), LeftOutside32.bHasBestCandidate);
	TestFalse(TEXT("32:9 우측 7.1도 거부"), RightOutside32.bHasBestCandidate);
	if (LeftInside32.bHasBestCandidate && RightInside32.bHasBestCandidate)
	{
		TestTrue(TEXT("32:9 좌우 CrosshairAngle 동일"), FMath::IsNearlyEqual(LeftInside32.BestCandidate.CrosshairAngleDeg, RightInside32.BestCandidate.CrosshairAngleDeg, 0.001f));
		TestTrue(TEXT("32:9 좌우 화면 거리 동일"), FMath::IsNearlyEqual(LeftInside32.BestCandidate.NormalizedScreenDistance, RightInside32.BestCandidate.NormalizedScreenDistance, 0.001f));
	}

	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetSearchDiagnosticsTest,
	"CarFight.TargetSelect.TS_P0_08.SearchDiagnostics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.1.0] 실제 VehiclePawn RefreshCurrentCandidate 경로가 검색 성능 진단값을 채우고 순수 평가 경로와 분리되는지 검증합니다.
bool FCFTargetSearchDiagnosticsTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.1.0] Content Asset을 저장하지 않고 실제 VehiclePawn 후보 검색 경로를 실행할 Transient World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("TS-P0-08 검색 진단 Transient World 생성"), TestWorld))
	{
		return false;
	}

	AddExpectedError(
		TEXT("VehicleVisualHitCollision: SM_Body component missing on TargetPerfVehiclePawn."),
		EAutomationExpectedErrorFlags::Contains,
		1);

	// [v1.1.0] 실제 C++ TargetSelectComp와 CameraComp를 소유하는 임시 CarFight VehiclePawn 생성 설정입니다.
	FActorSpawnParameters PawnSpawnParameters;
	PawnSpawnParameters.Name = TEXT("TargetPerfVehiclePawn");
	// [v1.1.0] 실제 RefreshCurrentCandidate 런타임 검색을 수행할 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		PawnSpawnParameters);
	if (!TestNotNull(TEXT("TS-P0-08 검색 진단 VehiclePawn 생성"), VehiclePawn))
	{
		return false;
	}

		// [v1.3.0] proximity 거리·반각 안에서 실제 LOS Trace를 발생시킬 대상입니다.
	ACFTargetSelectContractActor* TargetActor = SpawnTuneTarget(
		TestWorld,
		TEXT("TargetPerfCandidate"),
		FVector(3000.0f, 0.0f, 100.0f));
	// [v1.3.0] 7도 반각 밖이지만 Targetable이므로 InputActorCount에는 남고 LOS Trace만 생략돼야 할 대상입니다.
	ACFTargetSelectContractActor* OutsideAngleActor = SpawnTuneTarget(
		TestWorld,
		TEXT("TargetPerfOutsideAngle"),
		FVector(3000.0f, 3000.0f, 100.0f));
	// [v1.3.0] 1200m 거리 밖이지만 Targetable이므로 InputActorCount에는 남고 LOS Trace만 생략돼야 할 대상입니다.
	ACFTargetSelectContractActor* OutsideDistanceActor = SpawnTuneTarget(
		TestWorld,
		TEXT("TargetPerfOutsideDistance"),
		FVector(130000.0f, 30000.0f, 100.0f));
	if (!TestNotNull(TEXT("TS-P0-08 검색 진단 inside 대상 생성"), TargetActor)
		|| !TestNotNull(TEXT("TS-P0-08 검색 진단 angle outside 대상 생성"), OutsideAngleActor)
		|| !TestNotNull(TEXT("TS-P0-08 검색 진단 distance outside 대상 생성"), OutsideDistanceActor))
	{
		if (TargetActor) TargetActor->Destroy();
		if (OutsideAngleActor) OutsideAngleActor->Destroy();
		if (OutsideDistanceActor) OutsideDistanceActor->Destroy();
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.1.0] VehiclePawn이 생성 시 소유한 실제 TargetSelect 런타임 컴포넌트입니다.
	UCFTargetSelectComp* TargetSelectComp = VehiclePawn->GetTargetSelectComp();
	if (!TestNotNull(TEXT("TS-P0-08 검색 진단 TargetSelectComp 존재"), TargetSelectComp))
	{
		TargetActor->Destroy();
		VehiclePawn->Destroy();
		return false;
	}

	TargetSelectComp->bAutoRefreshCandidate = false;
	TargetSelectComp->TargetSelectData = nullptr;
	TargetSelectComp->FallbackTargetSelectConfig = FCFTargetSelectConfig();

	// [v1.4.0] 반환값과 무관하게 실제 View 생성·Registry snapshot·Trace·후보 평가가 수행되는 런타임 갱신을 한 번 실행합니다.
	const bool bRuntimeRefreshSelectedCandidate = TargetSelectComp->RefreshCurrentCandidate();
	(void)bRuntimeRefreshSelectedCandidate;

	// [v1.1.0] 실제 런타임 검색 뒤 캐시된 성능 진단 결과입니다.
		const FCFTargetSearchResult RuntimeSearchResult = TargetSelectComp->GetLastCandidateSearchResult();
	AddInfo(FString::Printf(
		TEXT("TS-P0-08 SearchDiagnostics | RegistryTargetable=%d | WorldScanned=%d | Input=%d | Accepted=%d | VisibilityTraces=%d | PrefilterSkipped=%d | TotalTraces=%d | SearchMs=%.4f"),
		RuntimeSearchResult.RuntimeRegistryTargetableCount,
		RuntimeSearchResult.RuntimeWorldActorScanCount,
		RuntimeSearchResult.InputActorCount,
		RuntimeSearchResult.AcceptedCandidateCount,
		RuntimeSearchResult.RuntimeVisibilityTraceCount,
		RuntimeSearchResult.RuntimeVisibilityPrefilterSkipCount,
		RuntimeSearchResult.RuntimeTotalTraceCount,
		RuntimeSearchResult.RuntimeSearchDurationMs));
	TestEqual(TEXT("런타임 반복 월드 Actor 스캔 0"), RuntimeSearchResult.RuntimeWorldActorScanCount, 0);
	TestTrue(TEXT("런타임 Registry Targetable snapshot 기록"), RuntimeSearchResult.RuntimeRegistryTargetableCount >= 3);
	TestEqual(TEXT("런타임 Targetable Actor 입력 3개 유지"), RuntimeSearchResult.InputActorCount, 3);
	TestEqual(TEXT("런타임 inside 대상 Visibility Trace 1회"), RuntimeSearchResult.RuntimeVisibilityTraceCount, 1);
	TestEqual(TEXT("런타임 outside 대상 LOS 사전 생략 2회"), RuntimeSearchResult.RuntimeVisibilityPrefilterSkipCount, 2);
	TestEqual(TEXT("런타임 전체 Trace = 직접 1 + Visibility 1"), RuntimeSearchResult.RuntimeTotalTraceCount, 2);
	TestTrue(TEXT("런타임 검색 시간 음수 아님"), RuntimeSearchResult.RuntimeSearchDurationMs >= 0.0f);

	// [v1.1.0] DebugSummary가 프로파일링 값을 외부에서 바로 읽을 수 있게 포함하는지 확인합니다.
	const FString RuntimeDebugSummary = TargetSelectComp->BuildCandidateSearchDebugSummary();
	TestTrue(TEXT("DebugSummary RegistryTargetable 포함"), RuntimeDebugSummary.Contains(TEXT("RegistryTargetable=")));
	TestTrue(TEXT("DebugSummary WorldScanned 호환값 포함"), RuntimeDebugSummary.Contains(TEXT("WorldScanned=0")));
	TestTrue(TEXT("DebugSummary VisibilityTraces 포함"), RuntimeDebugSummary.Contains(TEXT("VisibilityTraces=")));
	TestTrue(TEXT("DebugSummary PrefilterSkipped 포함"), RuntimeDebugSummary.Contains(TEXT("PrefilterSkipped=")));
	TestTrue(TEXT("DebugSummary TotalTraces 포함"), RuntimeDebugSummary.Contains(TEXT("TotalTraces=")));
	TestTrue(TEXT("DebugSummary SearchMs 포함"), RuntimeDebugSummary.Contains(TEXT("SearchMs=")));

	// [v1.1.0] 같은 대상의 순수 EvaluateCandidateActors는 월드 순회·Trace를 수행하지 않는 기존 계약을 보존할 검색 View입니다.
	const FCFTargetSearchView PureSearchView = BuildTuneSearchView(16.0f / 9.0f);
	// [v1.1.0] 순수 평가에서 사용할 기본 선택 컨텍스트이며 LOS Trace는 수행하지 않습니다.
	FCFTargetSelectionContext PureSelectionContext;
	PureSelectionContext.bRequireLineOfSightForNewSelection = false;
	// [v1.1.0] 순수 평가에서 후보 경쟁을 제거할 단일 Actor 배열입니다.
	TArray<AActor*> PureCandidateActors;
	PureCandidateActors.Add(TargetActor);
	// [v1.1.0] 월드 수집을 거치지 않은 순수 후보 평가 결과입니다.
	const FCFTargetSearchResult PureEvaluationResult = TargetSelectComp->EvaluateCandidateActors(PureCandidateActors, PureSearchView, PureSelectionContext);
	TestEqual(TEXT("순수 평가 월드 스캔 0"), PureEvaluationResult.RuntimeWorldActorScanCount, 0);
	TestEqual(TEXT("순수 평가 Registry snapshot 0"), PureEvaluationResult.RuntimeRegistryTargetableCount, 0);
	TestEqual(TEXT("순수 평가 Visibility Trace 0"), PureEvaluationResult.RuntimeVisibilityTraceCount, 0);
	TestEqual(TEXT("순수 평가 LOS 사전필터 생략 0"), PureEvaluationResult.RuntimeVisibilityPrefilterSkipCount, 0);
	TestEqual(TEXT("순수 평가 전체 Trace 0"), PureEvaluationResult.RuntimeTotalTraceCount, 0);
	TestEqual(TEXT("순수 평가 검색 시간 0"), PureEvaluationResult.RuntimeSearchDurationMs, 0.0f);

		TargetActor->Destroy();
	OutsideAngleActor->Destroy();
	OutsideDistanceActor->Destroy();
	VehiclePawn->Destroy();
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetRegistryLifecycleTest,
	"CarFight.TargetSelect.TS_P0_08.RegistryLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.4.0] 초기 bootstrap 이후 spawn된 Targetable Actor의 증분 등록과 파괴 Actor prune를 검증합니다.
bool FCFTargetRegistryLifecycleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.4.0] Registry lifecycle을 Content Asset 변경 없이 검증할 transient World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("TS-P0-08 RegistryLifecycle Transient World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.4.0] 현재 World의 TargetSelect 후보 Registry Subsystem입니다.
	UCFTargetRegistrySubsystem* TargetRegistrySubsystem = TestWorld->GetSubsystem<UCFTargetRegistrySubsystem>();
	if (!TestNotNull(TEXT("Target Registry Subsystem 존재"), TargetRegistrySubsystem))
	{
		return false;
	}

	// [v1.4.0] 새 Actor spawn 전에 초기 bootstrap을 확정할 snapshot입니다.
	TArray<AActor*> InitialTargetActors;
	TargetRegistrySubsystem->CollectTargetableActors(InitialTargetActors);
	// [v1.4.0] spawn handler가 초기 scan 완료 뒤에도 새 Targetable을 잡는지 검증할 첫 대상입니다.
	ACFTargetSelectContractActor* SpawnedTargetA = SpawnTuneTarget(
		TestWorld,
		TEXT("RegistrySpawnedTargetA"),
		FVector(2000.0f, 0.0f, 100.0f));
	if (!TestNotNull(TEXT("Registry spawn 대상 A 생성"), SpawnedTargetA))
	{
		return false;
	}

	// [v1.4.0] 대상 A spawn 직후 Registry가 반환하는 snapshot입니다.
	TArray<AActor*> AfterSpawnTargetActors;
	TargetRegistrySubsystem->CollectTargetableActors(AfterSpawnTargetActors);
	TestTrue(TEXT("초기 scan 이후 spawn 대상 A 증분 등록"), AfterSpawnTargetActors.Contains(SpawnedTargetA));

	// [v1.4.0] 연속 spawn 증분 등록을 확인할 두 번째 대상입니다.
	ACFTargetSelectContractActor* SpawnedTargetB = SpawnTuneTarget(
		TestWorld,
		TEXT("RegistrySpawnedTargetB"),
		FVector(2500.0f, 100.0f, 100.0f));
	if (!TestNotNull(TEXT("Registry spawn 대상 B 생성"), SpawnedTargetB))
	{
		SpawnedTargetA->Destroy();
		return false;
	}

	// [v1.4.0] 두 번째 spawn 뒤 Registry가 반환하는 snapshot입니다.
	TArray<AActor*> AfterSecondSpawnTargetActors;
	TargetRegistrySubsystem->CollectTargetableActors(AfterSecondSpawnTargetActors);
	TestTrue(TEXT("spawn 대상 A 유지"), AfterSecondSpawnTargetActors.Contains(SpawnedTargetA));
	TestTrue(TEXT("spawn 대상 B 증분 등록"), AfterSecondSpawnTargetActors.Contains(SpawnedTargetB));

	SpawnedTargetA->Destroy();

	// [v1.4.0] 파괴된 대상 A의 weak reference가 prune된 뒤 Registry snapshot입니다.
	TArray<AActor*> AfterDestroyTargetActors;
	TargetRegistrySubsystem->CollectTargetableActors(AfterDestroyTargetActors);
	TestFalse(TEXT("파괴 대상 A Registry prune"), AfterDestroyTargetActors.Contains(SpawnedTargetA));
	TestTrue(TEXT("유효 대상 B Registry 유지"), AfterDestroyTargetActors.Contains(SpawnedTargetB));

	SpawnedTargetB->Destroy();
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetSettingsPathTest,
	"CarFight.TargetSelect.TS_P0_08.SettingsPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.2.0] 저장된 BP_CFVehiclePawn CDO를 읽기 전용으로 로드해 실제 TargetSelectData 경로와 resolved config를 검증·기록합니다.
bool FCFTargetSettingsPathTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.2.0] 실제 게임 차량 Blueprint의 정식 ObjectPath입니다.
	const TCHAR* VehicleBlueprintObjectPath = TEXT("/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn.BP_CFVehiclePawn");
	// [v1.2.0] Compile·Save 없이 기존 저장 Blueprint를 읽기 전용으로 로드합니다.
	UBlueprint* VehicleBlueprint = LoadObject<UBlueprint>(nullptr, VehicleBlueprintObjectPath);
	if (!TestNotNull(TEXT("BP_CFVehiclePawn Load-only 성공"), VehicleBlueprint))
	{
		return false;
	}
		if (!TestNotNull(TEXT("BP_CFVehiclePawn GeneratedClass 존재"), VehicleBlueprint->GeneratedClass.Get()))
	{
		return false;
	}

	// [v1.2.0] 저장된 Blueprint GeneratedClass가 사용하는 실제 Class Default Object입니다.
	ACFVehiclePawn* VehiclePawnCDO = Cast<ACFVehiclePawn>(VehicleBlueprint->GeneratedClass->GetDefaultObject());
	if (!TestNotNull(TEXT("BP_CFVehiclePawn CDO 존재"), VehiclePawnCDO))
	{
		return false;
	}

	// [v1.2.0] CDO에 상속된 실제 TargetSelect 컴포넌트 기본 서브오브젝트입니다.
	UCFTargetSelectComp* TargetSelectCompCDO = VehiclePawnCDO->GetTargetSelectComp();
	// [v1.2.0] CDO에 상속된 실제 TargetPoint 컴포넌트 기본 서브오브젝트입니다.
	UCFTargetPointComp* TargetPointCompCDO = VehiclePawnCDO->GetTargetPointComp();
	if (!TestNotNull(TEXT("BP_CFVehiclePawn TargetSelectComp CDO 존재"), TargetSelectCompCDO)
		|| !TestNotNull(TEXT("BP_CFVehiclePawn TargetPoint CDO 존재"), TargetPointCompCDO))
	{
		return false;
	}

	// [v1.2.0] 실제 CDO가 DataAsset 또는 fallback 중 어느 설정 경로를 사용하는지 판정할 DataAsset 참조입니다.
	UCFTargetSelectData* TargetSelectData = TargetSelectCompCDO->TargetSelectData;
	// [v1.2.0] 실제 런타임과 동일한 해석 함수가 반환하는 최종 TargetSelect 설정입니다.
	const FCFTargetSelectConfig ResolvedConfig = TargetSelectCompCDO->GetResolvedTargetSelectConfig();
	// [v1.2.0] 문서와 로그에서 설정 source를 명확히 구분할 문자열입니다.
	const FString ConfigSource = TargetSelectData ? TEXT("TargetSelectData") : TEXT("FallbackTargetSelectConfig");
	// [v1.2.0] DataAsset이 있으면 실제 ObjectPath, 없으면 None을 기록합니다.
	const FString TargetSelectDataPath = GetPathNameSafe(TargetSelectData);

	AddInfo(FString::Printf(
		TEXT("TS-P0-08 SettingsPath | Source=%s | TargetSelectData=%s | DirectCm=%.1f | ProximityCm=%.1f | HalfAngleDeg=%.3f | RefreshSec=%.4f | OcclusionSec=%.3f | SwitchRatio=%.4f | AutoDebug=%s | DebugRadiusCm=%.1f"),
		*ConfigSource,
		*TargetSelectDataPath,
		ResolvedConfig.DirectSelectMaxDistanceCm,
		ResolvedConfig.ProximitySelectMaxDistanceCm,
		ResolvedConfig.ProximityHalfAngleDeg,
		ResolvedConfig.CandidateRefreshIntervalSec,
		ResolvedConfig.OcclusionGracePeriodSec,
		ResolvedConfig.CandidateSwitchAdvantageRatio,
		TargetSelectCompCDO->bDrawCandidateSearchDebug ? TEXT("True") : TEXT("False"),
		TargetSelectCompCDO->CandidateSearchDebugSphereRadiusCm));
	AddInfo(FString::Printf(
		TEXT("TS-P0-08 TargetPointCDO | Use=%s | AutoAlign=%s | PreferredBounds=%s | Offset=%s"),
		TargetPointCompCDO->bUseAsTargetPoint ? TEXT("True") : TEXT("False"),
		TargetPointCompCDO->bAutoAlignToPreferredBounds ? TEXT("True") : TEXT("False"),
		*TargetPointCompCDO->PreferredBoundsComponentName.ToString(),
		*TargetPointCompCDO->PreferredBoundsLocalOffset.ToCompactString()));

	TestTrue(TEXT("Resolved 직접 선택 거리 유효"), FMath::IsFinite(ResolvedConfig.DirectSelectMaxDistanceCm) && ResolvedConfig.DirectSelectMaxDistanceCm > 0.0f);
	TestTrue(TEXT("Resolved 근접 선택 거리 유효"), FMath::IsFinite(ResolvedConfig.ProximitySelectMaxDistanceCm) && ResolvedConfig.ProximitySelectMaxDistanceCm > 0.0f);
	TestTrue(TEXT("Resolved 반각 유효"), FMath::IsFinite(ResolvedConfig.ProximityHalfAngleDeg) && ResolvedConfig.ProximityHalfAngleDeg > 0.0f && ResolvedConfig.ProximityHalfAngleDeg <= 90.0f);
	TestTrue(TEXT("Resolved 갱신 간격 유효"), FMath::IsFinite(ResolvedConfig.CandidateRefreshIntervalSec) && ResolvedConfig.CandidateRefreshIntervalSec > 0.0f);
	TestTrue(TEXT("Resolved debug 반경 유효"), FMath::IsFinite(TargetSelectCompCDO->CandidateSearchDebugSphereRadiusCm) && TargetSelectCompCDO->CandidateSearchDebugSphereRadiusCm > 0.0f);
	TestTrue(TEXT("TargetPoint 명시 위치 사용"), TargetPointCompCDO->bUseAsTargetPoint);
	TestTrue(TEXT("TargetPoint 선호 Bounds 자동 정렬"), TargetPointCompCDO->bAutoAlignToPreferredBounds);
	TestEqual(TEXT("TargetPoint 선호 Bounds는 SM_Body"), TargetPointCompCDO->PreferredBoundsComponentName, FName(TEXT("SM_Body")));

	if (!TargetSelectData)
	{
		TestEqual(TEXT("Fallback Direct 거리 일치"), ResolvedConfig.DirectSelectMaxDistanceCm, TargetSelectCompCDO->FallbackTargetSelectConfig.DirectSelectMaxDistanceCm);
		TestEqual(TEXT("Fallback Proximity 거리 일치"), ResolvedConfig.ProximitySelectMaxDistanceCm, TargetSelectCompCDO->FallbackTargetSelectConfig.ProximitySelectMaxDistanceCm);
		TestEqual(TEXT("Fallback 반각 일치"), ResolvedConfig.ProximityHalfAngleDeg, TargetSelectCompCDO->FallbackTargetSelectConfig.ProximityHalfAngleDeg);
	}
	else
	{
		TestTrue(TEXT("TargetSelectData 설정 유효"), TargetSelectData->IsTargetSelectConfigValid());
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
