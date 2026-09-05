// Copyright (c) CarFight. All Rights Reserved.
// File: CFVBPresentTests.cpp
// Version: v1.1.0
// Date: 2026-09-04
// Description: CF-FQ-046 사용자 표시 계층 + CF-FQ-047 Step 8 Driving Apply readiness focused Automation입니다.
// Changelog:
// - v1.1.0: VBHAI-P0-07B typed Driving Apply blocker의 USER recovery 문장, benchmark-running/ready 상태와 raw diagnostic 비노출을 Step 8 presentation fixture에 추가.
// - v1.0.0: Step state recovery, Step 7 Set/Add/Remove/Move + unknown fallback, Step 8 metric/checklist + diagnostic identity 격리 fixture를 추가.
// Migration:
// - Pure presentation formatter만 호출하며 Recipe/VehicleData/Asset/Save mutation을 수행하지 않습니다.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehicleBuilderPresent.h"

namespace CFVBPresentTestsPrivate
{
	// Scalar SetLeaf diff 한 건을 간단히 구성합니다.
	FCFVehicleFieldDiff MakeScalarDiff(
		const TArray<FName>& PropertyChain,
		const FString& BeforeText,
		const FString& AfterText)
	{
		// USER presentation에 전달할 scalar diff입니다.
		FCFVehicleFieldDiff Diff;
		Diff.Operation = ECFVehicleDiffOp::SetLeaf;
		Diff.FieldPath.PropertyChain = PropertyChain;
		Diff.bHasBeforeValue = true;
		Diff.BeforeValue.CanonicalValueText = BeforeText;
		Diff.bHasAfterValue = true;
		Diff.AfterValue.CanonicalValueText = AfterText;
		return Diff;
	}

	// Stable-ID collection structural diff 한 건을 구성합니다.
	FCFVehicleFieldDiff MakeCollectionDiff(
		const ECFVehicleDiffOp Operation,
		const FName CollectionName,
		const FName SelectorName,
		const FName SelectorValue)
	{
		// USER presentation에 전달할 collection structural diff입니다.
		FCFVehicleFieldDiff Diff;
		Diff.Operation = Operation;
		Diff.FieldPath.CollectionPropertyName = CollectionName;
		Diff.FieldPath.SelectorKeyPropertyName = SelectorName;
		Diff.FieldPath.SelectorKeyValue = SelectorValue;
		return Diff;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVBPresentationStateTest,
	"CarFight.DataAuthoring.CF_FQ_046.VBIUX_P0_05.PresentationStates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVBPresentationFinalDiffTest,
	"CarFight.DataAuthoring.CF_FQ_046.VBIUX_P0_05.FinalReviewDiff",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVBPresentationDrivingTest,
	"CarFight.DataAuthoring.CF_FQ_046.VBIUX_P0_05.DrivingSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Complete / Ready / Locked / Stale / Blocked가 backend fallback 없이 USER recovery 문장으로 변환되는지 검증합니다.
bool FCFVBPresentationStateTest::RunTest(const FString& Parameters)
{
	// Backend-only 문자열이 USER fallback으로 새지 않는지 확인할 sentinel입니다.
	const FText BackendFallback = FText::FromString(TEXT("ProposalHash=SECRET_BACKEND_VALUE"));

	// Final Review Ready fixture입니다.
	FCFVehicleBuilderStepView ReadyStep;
	ReadyStep.StepId = ECFVehicleBuilderStepId::FinalReview;
	ReadyStep.State = ECFVehicleBuilderStepState::Ready;
	// Final Review Complete fixture입니다.
	FCFVehicleBuilderStepView CompleteStep = ReadyStep;
	CompleteStep.State = ECFVehicleBuilderStepState::Complete;
	// Final Review Locked fixture입니다.
	FCFVehicleBuilderStepView LockedStep = ReadyStep;
	LockedStep.State = ECFVehicleBuilderStepState::Locked;
	// Final Review Stale fixture입니다.
	FCFVehicleBuilderStepView StaleStep = ReadyStep;
	StaleStep.State = ECFVehicleBuilderStepState::Stale;
	// Final Review Blocked fixture입니다.
	FCFVehicleBuilderStepView BlockedStep = ReadyStep;
	BlockedStep.State = ECFVehicleBuilderStepState::Blocked;

	// 각 상태의 USER next action입니다.
	const FString ReadyText = FCFVehicleBuilderPresentation::GetStepNextAction(ReadyStep, BackendFallback).ToString();
	const FString CompleteText = FCFVehicleBuilderPresentation::GetStepNextAction(CompleteStep, BackendFallback).ToString();
	const FString LockedText = FCFVehicleBuilderPresentation::GetStepNextAction(LockedStep, BackendFallback).ToString();
	const FString StaleText = FCFVehicleBuilderPresentation::GetStepNextAction(StaleStep, BackendFallback).ToString();
	const FString BlockedText = FCFVehicleBuilderPresentation::GetStepNextAction(BlockedStep, BackendFallback).ToString();

	TestTrue(TEXT("VBIUX Ready explains apply action"), ReadyText.Contains(TEXT("변경 적용")));
	TestTrue(TEXT("VBIUX Complete explains save/next action"), CompleteText.Contains(TEXT("저장")));
	TestTrue(TEXT("VBIUX Locked explains previous-step requirement"), LockedText.Contains(TEXT("앞 단계")));
	TestTrue(TEXT("VBIUX Stale explains refresh/re-review"), StaleText.Contains(TEXT("현재 상태 다시 확인")));
	TestTrue(TEXT("VBIUX Blocked explains resolution requirement"), BlockedText.Contains(TEXT("해결")));

	TestEqual(
		TEXT("VBIUX Locked state label is user-facing"),
		FCFVehicleBuilderPresentation::GetStepStateLabel(ECFVehicleBuilderStepState::Locked),
		FString(TEXT("이전 단계 필요")));
	TestEqual(
		TEXT("VBIUX Stale state label is user-facing"),
		FCFVehicleBuilderPresentation::GetStepStateLabel(ECFVehicleBuilderStepState::Stale),
		FString(TEXT("다시 확인 필요")));

	TestFalse(TEXT("VBIUX Ready hides backend fallback"), ReadyText.Contains(TEXT("SECRET_BACKEND_VALUE")));
	TestFalse(TEXT("VBIUX Blocked hides backend fallback"), BlockedText.Contains(TEXT("SECRET_BACKEND_VALUE")));
	return true;
}

// Step 7 Set/Add/Remove/Move와 unknown field가 human-readable review로 변환되는지 검증합니다.
bool FCFVBPresentationFinalDiffTest::RunTest(const FString& Parameters)
{
	using namespace CFVBPresentTestsPrivate;

	// USER-facing Final Review fixture입니다.
	FCFBuilderFinalReviewResult Review;
	Review.bApplyRequired = true;
	Review.bCanApply = true;

	// Known scalar SetLeaf fixture입니다.
	Review.FieldDiff.Add(MakeScalarDiff(
		{FName(TEXT("VehicleMovementConfig")), FName(TEXT("EngineMaxTorque"))},
		TEXT("500"),
		TEXT("650")));

	// Hardpoint Add structural fixture입니다.
	Review.FieldDiff.Add(MakeCollectionDiff(
		ECFVehicleDiffOp::AddArrayElement,
		FName(TEXT("HardpointSlots")),
		FName(TEXT("LocationSlotId")),
		FName(TEXT("HP_Front"))));

	// 같은 Hardpoint Add group에 딸린 SetLeaf가 structural summary와 중복 노출되지 않는지 확인할 child fixture입니다.
	FCFVehicleFieldDiff HardpointChildDiff = MakeScalarDiff(
		{FName(TEXT("SocketName"))},
		TEXT("OldSocket_Internal"),
		TEXT("NewSocket_Internal"));
	HardpointChildDiff.FieldPath.CollectionPropertyName = FName(TEXT("HardpointSlots"));
	HardpointChildDiff.FieldPath.SelectorKeyPropertyName = FName(TEXT("LocationSlotId"));
	HardpointChildDiff.FieldPath.SelectorKeyValue = FName(TEXT("HP_Front"));
	Review.FieldDiff.Add(HardpointChildDiff);

	// Mount Remove structural fixture입니다.
	Review.FieldDiff.Add(MakeCollectionDiff(
		ECFVehicleDiffOp::RemoveArrayElement,
		FName(TEXT("MountProfiles")),
		FName(TEXT("MountProfileId")),
		FName(TEXT("Mount_Left"))));

	// Hardpoint Move structural fixture입니다.
	Review.FieldDiff.Add(MakeCollectionDiff(
		ECFVehicleDiffOp::MoveArrayElement,
		FName(TEXT("HardpointSlots")),
		FName(TEXT("LocationSlotId")),
		FName(TEXT("HP_Rear"))));

	// Unknown future field가 raw path를 기본 USER 화면에 노출하지 않는지 확인할 fixture입니다.
	Review.FieldDiff.Add(MakeScalarDiff(
		{FName(TEXT("VehicleMovementConfig")), FName(TEXT("UnknownFutureSetting"))},
		TEXT("1"),
		TEXT("2")));

	// Pure presentation 결과입니다.
	const FString Summary = FCFVehicleBuilderPresentation::BuildFinalReviewSummary(Review).ToString();

	TestTrue(TEXT("VBIUX SetLeaf shows user field label"), Summary.Contains(TEXT("엔진 최대 토크")));
	TestTrue(TEXT("VBIUX SetLeaf shows before value with unit"), Summary.Contains(TEXT("현재: 500 Nm")));
	TestTrue(TEXT("VBIUX SetLeaf shows after value with unit"), Summary.Contains(TEXT("변경 후: 650 Nm")));
	TestTrue(TEXT("VBIUX SetLeaf explains meaning"), Summary.Contains(TEXT("의미: 엔진이 낼 수 있는 최대 회전 힘")));
	TestTrue(TEXT("VBIUX Add shows hardpoint user label"), Summary.Contains(TEXT("[추가] 장비 장착 위치 HP_Front")));
	TestTrue(TEXT("VBIUX Remove shows mount user label"), Summary.Contains(TEXT("[삭제] 장착 규칙 Mount_Left")));
	TestTrue(TEXT("VBIUX Move shows structural operation"), Summary.Contains(TEXT("[순서 변경] 장비 장착 위치 HP_Rear")));
	TestFalse(TEXT("VBIUX structural Add hides grouped child before value"), Summary.Contains(TEXT("OldSocket_Internal")));
	TestFalse(TEXT("VBIUX structural Add hides grouped child after value"), Summary.Contains(TEXT("NewSocket_Internal")));
	TestTrue(TEXT("VBIUX unknown field uses generic fallback"), Summary.Contains(TEXT("기타 차량 설정 변경")));
	TestFalse(TEXT("VBIUX unknown field hides raw canonical path"), Summary.Contains(TEXT("UnknownFutureSetting")));
	return true;
}

// Step 8 metric/checklist가 USER 판단 정보만 표시하고 technical identity를 숨기는지 검증합니다.
bool FCFVBPresentationDrivingTest::RunTest(const FString& Parameters)
{
	// Current Step 8 USER presentation fixture입니다.
	FCFVehicleDrivingPresentInfo Info;
	Info.bHasBenchmarkResult = true;
	Info.bUserDrivePrepared = true;
	Info.bUserDrivingAccepted = false;
	Info.BenchmarkResult.bHasMetric = true;
	Info.BenchmarkResult.RunId = TEXT("SECRET_RUN_ID");
	Info.BenchmarkResult.ExpectedTargetDefinitionHash = TEXT("SECRET_DEFINITION_HASH");

	// 기술 주행 측정값 fixture입니다.
	FCFVehicleBuilderDrivingMetric& Metric = Info.BenchmarkResult.Metric;
	Metric.Acceleration0To50Seconds = 3.25;
	Metric.Acceleration0To100Seconds = 8.75;
	Metric.PeakSpeedKmh = 172.4;
	Metric.PeakEngineRpm = 6250.0;
	Metric.PeakSpeedGear = 6;
	Metric.bBraking100Available = true;
	Metric.Braking100ToIdleSeconds = 3.40;
	Metric.Braking100ToIdleDistanceMeters = 45.20;
	Metric.EffectiveTurningRadiusMeters = 6.80;
	Metric.TurningAverageSpeedKmh = 28.0;

	// Pure presentation 결과입니다.
	const FString Summary = FCFVehicleBuilderPresentation::BuildDrivingSummary(Info).ToString();

	TestTrue(TEXT("VBIUX Driving shows 0-50 metric"), Summary.Contains(TEXT("0→50 km/h: 3.250초")));
	TestTrue(TEXT("VBIUX Driving shows peak speed"), Summary.Contains(TEXT("관측 최고 속도: 172.4 km/h")));
	TestTrue(TEXT("VBIUX Driving shows braking metric"), Summary.Contains(TEXT("3.400초 / 45.20 m")));
	TestTrue(TEXT("VBIUX Driving shows turning radius"), Summary.Contains(TEXT("회전반경: 6.80 m")));
	TestTrue(TEXT("VBIUX Driving says metrics are advisory"), Summary.Contains(TEXT("자동 합격/불합격 기준이 아니며")));
	TestTrue(TEXT("VBIUX Driving shows direct checklist"), Summary.Contains(TEXT("변속 시점과 변속 후 RPM 회복")));
	TestTrue(TEXT("VBIUX Driving says checklist is not persisted"), Summary.Contains(TEXT("항목별 체크 상태를 Asset에 저장하지 않습니다")));
	TestFalse(TEXT("VBIUX Driving hides RunId"), Summary.Contains(TEXT("SECRET_RUN_ID")));
	TestFalse(TEXT("VBIUX Driving hides DefinitionHash"), Summary.Contains(TEXT("SECRET_DEFINITION_HASH")));

	// 저장되지 않은 Recipe blocker의 USER presentation fixture입니다.
	FCFVehicleDrivingApplyPreflight RecipeUnsavedPreflight;
	RecipeUnsavedPreflight.Blocker = ECFVehicleDrivingApplyBlocker::RecipeUnsaved;
	RecipeUnsavedPreflight.Diagnostic = TEXT("SECRET_RECIPE_PATH_OR_HASH");
	// 저장되지 않은 Recipe blocker의 USER-visible 문장입니다.
	const FString RecipeUnsavedText = FCFVehicleBuilderPresentation::BuildDrivingApplyReadiness(
		RecipeUnsavedPreflight,
		false).ToString();
	TestTrue(TEXT("VBHAI Step 8 RecipeUnsaved tells USER what to save"), RecipeUnsavedText.Contains(TEXT("Recipe Asset")));
	TestTrue(TEXT("VBHAI Step 8 RecipeUnsaved preserves no-auto-save boundary"), RecipeUnsavedText.Contains(TEXT("자동 저장하지 않습니다")));
	TestFalse(TEXT("VBHAI Step 8 RecipeUnsaved hides raw diagnostic"), RecipeUnsavedText.Contains(TEXT("SECRET_RECIPE_PATH_OR_HASH")));

	// Stale benchmark blocker의 USER presentation fixture입니다.
	FCFVehicleDrivingApplyPreflight BenchmarkStalePreflight;
	BenchmarkStalePreflight.Blocker = ECFVehicleDrivingApplyBlocker::BenchmarkStale;
	BenchmarkStalePreflight.Diagnostic = TEXT("SECRET_STALE_HASH");
	// Stale benchmark를 USER recovery action으로 변환한 문장입니다.
	const FString BenchmarkStaleText = FCFVehicleBuilderPresentation::BuildDrivingApplyReadiness(
		BenchmarkStalePreflight,
		false).ToString();
	TestTrue(TEXT("VBHAI Step 8 stale benchmark explains rerun"), BenchmarkStaleText.Contains(TEXT("기술 주행 측정")));
	TestTrue(TEXT("VBHAI Step 8 stale benchmark explains current refresh"), BenchmarkStaleText.Contains(TEXT("현재 상태 다시 확인")));
	TestFalse(TEXT("VBHAI Step 8 stale benchmark hides raw hash"), BenchmarkStaleText.Contains(TEXT("SECRET_STALE_HASH")));

	// 모든 stable prerequisite가 충족된 USER presentation fixture입니다.
	FCFVehicleDrivingApplyPreflight ReadyPreflight;
	ReadyPreflight.Blocker = ECFVehicleDrivingApplyBlocker::None;
	// Ready 상태에서 USER에게 Play/apply 순서를 안내하는 문장입니다.
	const FString ReadyText = FCFVehicleBuilderPresentation::BuildDrivingApplyReadiness(
		ReadyPreflight,
		false).ToString();
	TestTrue(TEXT("VBHAI Step 8 ready state says apply preparation complete"), ReadyText.Contains(TEXT("준비가 완료")));
	TestTrue(TEXT("VBHAI Step 8 ready state explains Play action"), ReadyText.Contains(TEXT("Play")));

	// Benchmark child process가 실행 중일 때의 Tab-local presentation입니다.
	const FString RunningText = FCFVehicleBuilderPresentation::BuildDrivingApplyReadiness(
		ReadyPreflight,
		true).ToString();
	TestTrue(TEXT("VBHAI Step 8 running state tells USER to wait for measurement"), RunningText.Contains(TEXT("측정 완료")));
	return true;
}

#endif
