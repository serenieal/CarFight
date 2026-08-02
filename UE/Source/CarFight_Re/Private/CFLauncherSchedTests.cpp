// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-02
// Description: CF-FQ-029 LM-P0-03B 런처 발사 시퀀스 스케줄러 자동화 테스트
// Scope: SingleCycle 완료, Ripple 시간 진행, Salvo 처리 묶음, Volley 목표 Snapshot, 실패 정책, 취소와 Reset 계약을 검증합니다.
// Changelog:
// - v1.1.0: 첫 발사 순간 위치·Actor Snapshot이 이후 선택 변경과 독립적으로 유지되고 Reset에서 함께 제거되는 계약을 추가.
// - v1.0.0: Launcher Scheduler RuntimeContract 최초 추가.
// Migration:
// - 실제 Pawn·Muzzle·Projectile 시각 실행은 Editor 검증 대상으로 남기고 이 테스트는 순수 결정적 시퀀스·Snapshot 상태를 검증합니다.

#if WITH_DEV_AUTOMATION_TESTS

#include "CFLauncherTypes.h"

#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFLauncherSchedulerContractTest,
	"CarFight.Launcher.LM_P0_03B.SchedulerContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.1.0] 런처 패턴별 Dispatch 상태와 첫 발사 순간 위치·Actor 목표 Snapshot 계약을 검증합니다.
bool FCFLauncherSchedulerContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 기존 단발 발사가 즉시 완료되는지 검증할 기본 시퀀스입니다.
	FCFLauncherSequenceRuntime SingleCycleRuntime;
	SingleCycleRuntime.Start(FCFLauncherFirePatternConfig(), 1);
	TestEqual(TEXT("SingleCycle 시작 직후 Completed"), SingleCycleRuntime.State, ECFLauncherSequenceState::Completed);
	TestEqual(TEXT("SingleCycle 전체 수량 1"), SingleCycleRuntime.TotalProjectileCount, 1);
	TestEqual(TEXT("SingleCycle 시도 수량 1"), SingleCycleRuntime.AttemptedProjectileCount, 1);
	TestEqual(TEXT("SingleCycle 승인 수량 1"), SingleCycleRuntime.AcceptedProjectileCount, 1);
	TestEqual(TEXT("SingleCycle 남은 수량 0"), SingleCycleRuntime.RemainingProjectileCount, 0);

	// [v1.1.0] 첫 발사 순간 Snapshot에 저장할 Command Target 월드 위치입니다.
	const FVector InitialCommandTargetLocation(1250.0f, -340.0f, 85.0f);

	// [v1.1.0] 첫 발사 순간 선택되어 같은 Volley가 유지해야 하는 유도 목표 Actor입니다.
	AActor* InitialGuidanceTargetActor = GetMutableDefault<AActor>();

	// [v1.1.0] 첫 발사 뒤 플레이어가 새로 선택했다고 가정할 다른 Actor입니다.
	AActor* LaterSelectedTargetActor = GetMutableDefault<APawn>();

	// [v1.1.0] 위치와 Actor가 한 번에 캡처되는 Volley 명령 목표 Snapshot입니다.
	FCFLauncherCommandTargetSnapshot CommandTargetSnapshot;
	CommandTargetSnapshot.Capture(InitialCommandTargetLocation, InitialGuidanceTargetActor);

	TestTrue(TEXT("Command Target 위치 Snapshot 유지"), CommandTargetSnapshot.CommandTargetLocation.Equals(InitialCommandTargetLocation));
	TestTrue(TEXT("Guidance Target Actor Snapshot 유지"), CommandTargetSnapshot.GetGuidanceTargetActor() == InitialGuidanceTargetActor);
	TestTrue(TEXT("후속 선택 변경과 Snapshot Actor 분리"), CommandTargetSnapshot.GetGuidanceTargetActor() != LaterSelectedTargetActor);

	CommandTargetSnapshot.Reset();
	TestTrue(TEXT("Snapshot Reset 뒤 위치 초기화"), CommandTargetSnapshot.CommandTargetLocation.IsNearlyZero());
	TestNull(TEXT("Snapshot Reset 뒤 Actor 초기화"), CommandTargetSnapshot.GetGuidanceTargetActor());

	// [v1.0.0] 4발 Ripple의 시간 간격과 한 호출당 1발 Dispatch를 검증할 설정입니다.
	FCFLauncherFirePatternConfig RippleConfig;
	RippleConfig.FirePattern = ECFLauncherFirePattern::Ripple;
	RippleConfig.ProjectileCountPerTrigger = 4;
	RippleConfig.InterMuzzleDelaySeconds = 0.10f;
	RippleConfig.SequenceFailurePolicy = ECFLauncherSequenceFailurePolicy::ContinueRemaining;

	// [v1.0.0] Ripple 시간 진행과 결과 집계를 검증할 순수 런타임 상태입니다.
	FCFLauncherSequenceRuntime RippleRuntime;
	RippleRuntime.Start(RippleConfig, 2);
	TestEqual(TEXT("Ripple 첫 발 뒤 Active"), RippleRuntime.State, ECFLauncherSequenceState::Active);
	TestEqual(TEXT("Ripple 첫 발 뒤 남은 수량 3"), RippleRuntime.RemainingProjectileCount, 3);
	TestEqual(TEXT("Ripple 0.05초 전 Dispatch 없음"), RippleRuntime.GetDispatchBudget(0.05f), 0);
	TestEqual(TEXT("Ripple 누적 0.10초에 1발 Dispatch"), RippleRuntime.GetDispatchBudget(0.05f), 1);
	TestTrue(TEXT("Ripple 첫 후속 발 Dispatch 기록"), RippleRuntime.MarkShotDispatched());
	RippleRuntime.RecordShotResult(true);
	TestEqual(TEXT("Ripple 두 번째 승인 수량 2"), RippleRuntime.AcceptedProjectileCount, 2);
	TestEqual(TEXT("Ripple 큰 Delta도 호출당 최대 1발"), RippleRuntime.GetDispatchBudget(1.0f), 1);
	TestTrue(TEXT("Ripple 세 번째 발 Dispatch 기록"), RippleRuntime.MarkShotDispatched());
	RippleRuntime.RecordShotResult(false);
	TestEqual(TEXT("ContinueRemaining 실패 수량 1"), RippleRuntime.FailedProjectileCount, 1);
	TestEqual(TEXT("ContinueRemaining 실패 후 Active 유지"), RippleRuntime.State, ECFLauncherSequenceState::Active);
	TestEqual(TEXT("Ripple 마지막 발 간격 도달"), RippleRuntime.GetDispatchBudget(0.10f), 1);
	TestTrue(TEXT("Ripple 마지막 발 Dispatch 기록"), RippleRuntime.MarkShotDispatched());
	RippleRuntime.RecordShotResult(true);
	TestEqual(TEXT("Ripple 마지막 결과 뒤 Completed"), RippleRuntime.State, ECFLauncherSequenceState::Completed);
	TestEqual(TEXT("Ripple 총 시도 수량 4"), RippleRuntime.AttemptedProjectileCount, 4);
	TestEqual(TEXT("Ripple 승인 수량 3"), RippleRuntime.AcceptedProjectileCount, 3);

	// [v1.0.0] StopSequence 정책이 첫 후속 실패에서 취소되는지 검증할 설정입니다.
	FCFLauncherFirePatternConfig StopRippleConfig = RippleConfig;
	StopRippleConfig.SequenceFailurePolicy = ECFLauncherSequenceFailurePolicy::StopSequence;

	// [v1.0.0] StopSequence 실패 정책을 검증할 순수 런타임 상태입니다.
	FCFLauncherSequenceRuntime StopRippleRuntime;
	StopRippleRuntime.Start(StopRippleConfig, 3);
	TestEqual(TEXT("Stop Ripple 첫 간격 Dispatch"), StopRippleRuntime.GetDispatchBudget(0.10f), 1);
	TestTrue(TEXT("Stop Ripple Dispatch 기록"), StopRippleRuntime.MarkShotDispatched());
	StopRippleRuntime.RecordShotResult(false);
	TestEqual(TEXT("StopSequence 실패 후 Cancelled"), StopRippleRuntime.State, ECFLauncherSequenceState::Cancelled);
	TestEqual(TEXT("StopSequence 취소 사유 ShotFailed"), StopRippleRuntime.CancelReason, ECFLauncherSequenceCancelReason::ShotFailed);

	// [v1.0.0] 6발 Salvo와 최대 동시 처리 4를 검증할 설정입니다.
	FCFLauncherFirePatternConfig SalvoConfig;
	SalvoConfig.FirePattern = ECFLauncherFirePattern::Salvo;
	SalvoConfig.ProjectileCountPerTrigger = 6;
	SalvoConfig.MaximumSimultaneousLaunchCount = 4;

	// [v1.0.0] 첫 발 포함 Salvo 처리 묶음과 완료 상태를 검증할 순수 런타임 상태입니다.
	FCFLauncherSequenceRuntime SalvoRuntime;
	SalvoRuntime.Start(SalvoConfig, 4);
	TestEqual(TEXT("Salvo 첫 처리 묶음 추가 예산 3"), SalvoRuntime.GetDispatchBudget(0.0f), 3);
	for (int32 ShotIndex = 0; ShotIndex < 3; ++ShotIndex)
	{
		TestTrue(TEXT("Salvo 첫 처리 묶음 Dispatch 기록"), SalvoRuntime.MarkShotDispatched());
		SalvoRuntime.RecordShotResult(true);
	}
	TestEqual(TEXT("Salvo 첫 묶음 뒤 남은 수량 2"), SalvoRuntime.RemainingProjectileCount, 2);
	TestEqual(TEXT("Salvo 다음 처리 묶음 예산 2"), SalvoRuntime.GetDispatchBudget(0.0f), 2);
	for (int32 ShotIndex = 0; ShotIndex < 2; ++ShotIndex)
	{
		TestTrue(TEXT("Salvo 마지막 처리 묶음 Dispatch 기록"), SalvoRuntime.MarkShotDispatched());
		SalvoRuntime.RecordShotResult(true);
	}
	TestEqual(TEXT("Salvo 전체 처리 뒤 Completed"), SalvoRuntime.State, ECFLauncherSequenceState::Completed);
	TestEqual(TEXT("Salvo 승인 수량 6"), SalvoRuntime.AcceptedProjectileCount, 6);

	// [v1.0.0] 명시적 취소와 Reset이 상태를 정리하는지 검증할 순수 런타임 상태입니다.
	FCFLauncherSequenceRuntime CancelRuntime;
	CancelRuntime.Start(RippleConfig, 5);
	CancelRuntime.Cancel(ECFLauncherSequenceCancelReason::OwnerDestroyed);
	TestEqual(TEXT("명시적 취소 상태"), CancelRuntime.State, ECFLauncherSequenceState::Cancelled);
	TestEqual(TEXT("명시적 취소 사유 보존"), CancelRuntime.CancelReason, ECFLauncherSequenceCancelReason::OwnerDestroyed);
	CancelRuntime.Reset();
	TestEqual(TEXT("Reset 뒤 Idle"), CancelRuntime.State, ECFLauncherSequenceState::Idle);
	TestEqual(TEXT("Reset 뒤 VolleyId 0"), CancelRuntime.VolleyId, 0);
	TestEqual(TEXT("Reset 뒤 수량 0"), CancelRuntime.TotalProjectileCount, 0);
	TestEqual(TEXT("Reset 뒤 취소 사유 None"), CancelRuntime.CancelReason, ECFLauncherSequenceCancelReason::None);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
