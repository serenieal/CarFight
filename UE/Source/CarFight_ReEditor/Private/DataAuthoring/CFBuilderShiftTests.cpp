// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderShiftTests.cpp
// Version: v1.4.0
// Date: 2026-09-11
// Description: CF-FQ-040 ESH-03 WheelTorqueCrossoverShift generic + actual Wagon post-5500 DefinitionApply mutation0 Automation입니다.
// Scope: production path는 계속 default-off로 보호하고, explicit ESH-03 test opt-in에서 current 5500 Target/AppliedState authority와 ratio/radius/post-shift/fail-closed/raw fixed-common 품질 진단을 검증합니다.
// Changelog:
// - v1.4.0: fresh persisted Wagon readback에서 확인된 current AppliedDefinitionHash 8aad29...로 ESH-03 stale snapshot expectation을 동기화. Shift diagnostic/Target mutation0 계약과 Product Asset은 변경 없음.
// - v1.3.1: 당시 TargetHash 0e5b...가 Engine Curve Apply만의 결과가 아니라 이후 4500→5500 Shift DefinitionApply까지 완료된 Target identity였음을 Historical 주석으로 보존.
// - v1.3.0: actual Wagon authority를 USER-approved ChangeUpRPM 5500 Target Apply 뒤 상태로 갱신. Transmission receipt e1be..., Target hash 0e5b..., Target/Drivetrain 5500/2000을 exact 검증하면서 raw WheelTorque recommendation 약6222는 독립 diagnostic으로 보존.
// - v1.2.0: actual Wagon expectation을 pre-Apply Target=false에서 approved post-DefinitionApply Target=true + exact AppliedDefinitionHash로 전환하고 mutation0 hash invariant를 유지.
// - v1.1.0: production default-off gate, malformed ratio/radius/post-shift RPM negative regression, nested blocker parent propagation, Wagon poor-fit warning 검증을 추가.
// - v1.0.0: generic deterministic recommendation, no-curve fallback, actual Wagon persisted profile/receipt/WSA authority diagnostic, Saved WagonShiftPreview.json을 최초 구현.
// Migration:
// - v1.4.0의 hash 갱신은 현재 디스크 persisted Recipe/Target snapshot을 반영하는 test-only expectation 교정이며 Product Asset을 수정하지 않습니다.
// - ChangeUpRPM/ChangeDownRPM/Profile/Target/Recipe를 수정하거나 저장하지 않습니다.
// - actual Wagon ESH-03는 USER-approved Engine Curve가 Target DefinitionApply까지 완료된 exact post-Apply state에서만 formal diagnostic을 허용합니다.

#include "Misc/AutomationTest.h"

#include "CFVehicleData.h"
#include "DataAuthoring/CFBuilderEngineUtil.h"
#include "DataAuthoring/CFBuilderTransUtil.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleAuthoringService.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace CFBuilderShiftTestsPrivate
{
	// actual Wagon exact managed Recipe path입니다.
	const TCHAR* WagonRecipePath = TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_Wagon.DA_Recipe_Wagon");

	// USER-approved persisted Engine Curve proposal hash입니다.
	const TCHAR* ExpectedWagonEngineCurveHash = TEXT("773221966499c6295f2a652a21b270a5");

	// Existing vehicle-specific 8AT proposal hash입니다.
	const TCHAR* ExpectedWagonTransmissionHash = TEXT("e1be2562d77fb1f5a993d6e7f5d17962");

	// Fresh persisted readback 기준 현재 Wagon Recipe/Target의 exact AppliedDefinitionHash입니다.
	const TCHAR* ExpectedWagonAppliedTargetHash = TEXT("8aad29d04e008fcd2acb2e6470cd87f1");

	// ESH-03 generic 4-speed drivetrain fixture를 만듭니다.
	FCFDrivetrainProfileData BuildGenericDrivetrain()
	{
		FCFDrivetrainProfileData Data;
		Data.bUseTransmissionConfig = true;
		Data.bUseAutomaticGears = true;
		Data.bUseAutoReverse = true;
		Data.TransmissionRatios.ForwardGearRatios = {3.20f, 2.05f, 1.38f, 1.00f};
		Data.TransmissionRatios.ReverseGearRatios = {3.10f};
		Data.FinalRatio = 3.55f;
		Data.ChangeUpRPM = 4200.0f;
		Data.ChangeDownRPM = 1800.0f;
		Data.GearChangeTime = 0.35f;
		Data.TransmissionEfficiency = 0.90f;
		return Data;
	}

	// ESH-03 generic vehicle-specific Engine Curve fixture를 만듭니다.
	FCFPerformanceProfileData BuildGenericPerformance()
	{
		FCFPerformanceProfileData Data;
		Data.EngineMaxTorqueByFeel = {280.0f, 350.0f, 430.0f};
		Data.EngineMaxRPMByFeel = {5500.0f, 6500.0f, 7500.0f};
		Data.EngineIdleRPM = 900.0f;
		Data.bUseEngineTorqueCurve = true;

		auto AddPoint = [&Data](const float Rpm, const float Multiplier)
		{
			FCFVehicleEngineTorquePoint& Point = Data.EngineTorqueCurve.Points.AddDefaulted_GetRef();
			Point.EngineRPM = Rpm;
			Point.TorqueMultiplier = Multiplier;
		};
		AddPoint(900.0f, 0.60f);
		AddPoint(1800.0f, 1.00f);
		AddPoint(4800.0f, 1.00f);
		AddPoint(5550.0f, 0.90f);
		AddPoint(6500.0f, 0.65f);
		return Data;
	}

	// 두 ESH-03 diagnostic의 deterministic scalar/pair 결과가 동일한지 검사합니다.
	bool DiagnosticsEquivalent(
		const FCFBuilderWheelTorqueShiftDiagnostic& Left,
		const FCFBuilderWheelTorqueShiftDiagnostic& Right)
	{
		if (Left.RecommendedChangeUpRPM != Right.RecommendedChangeUpRPM
			|| Left.Pairs.Num() != Right.Pairs.Num()
			|| !FMath::IsNearlyEqual(Left.MeanSquaredRelativeTorqueGap, Right.MeanSquaredRelativeTorqueGap, 0.000001f)
			|| !FMath::IsNearlyEqual(Left.MaxRelativeTorqueGap, Right.MaxRelativeTorqueGap, 0.000001f))
		{
			return false;
		}

		for (int32 PairIndex = 0; PairIndex < Left.Pairs.Num(); ++PairIndex)
		{
			const FCFBuilderWheelTorquePairDiagnostic& LeftPair = Left.Pairs[PairIndex];
			const FCFBuilderWheelTorquePairDiagnostic& RightPair = Right.Pairs[PairIndex];
			if (LeftPair.FromGearNumber != RightPair.FromGearNumber
				|| LeftPair.ToGearNumber != RightPair.ToGearNumber
				|| LeftPair.bCrossoverFound != RightPair.bCrossoverFound
				|| LeftPair.bLimitedByEngineMaxRPM != RightPair.bLimitedByEngineMaxRPM
				|| !FMath::IsNearlyEqual(LeftPair.CrossoverRPM, RightPair.CrossoverRPM, 0.001f)
				|| !FMath::IsNearlyEqual(LeftPair.RecommendedRelativeTorqueGap, RightPair.RecommendedRelativeTorqueGap, 0.000001f))
			{
				return false;
			}
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBuilderShiftGenericTest,
	"CarFight.DataAuthoring.CF_FQ_040.ESH_03.WheelTorqueCrossoverGeneric",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBuilderShiftWagonTest,
	"CarFight.DataAuthoring.CF_FQ_040.ESH_03.WagonShiftDiagnostic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// 차종/실제 Asset과 무관하게 ESH-03 Engine Curve 기반 fixed common ChangeUpRPM 계산 계약을 검증합니다.
bool FCFBuilderShiftGenericTest::RunTest(const FString& Parameters)
{
	using namespace CFBuilderShiftTestsPrivate;
	(void)Parameters;

	const FCFDrivetrainProfileData Drivetrain = BuildGenericDrivetrain();
	const FCFPerformanceProfileData Performance = BuildGenericPerformance();

	// Production parent diagnostic의 기본 호출은 ESH-03를 explicit enable하지 않으므로 dormant 상태를 유지해야 합니다.
	FCFVehicleResolveResult DefaultGateResolveResult;
	FCFBuilderTransmissionDiagnostic DefaultGateDiagnostic;
	CFBuilderTransUtil::BuildTransmissionDiagnostic(
		ECFBuilderTransmissionPolicy::LegacyCompatible,
		FCFBuilderTransmissionReview(),
		FString(),
		Drivetrain,
		Performance,
		DefaultGateResolveResult,
		DefaultGateDiagnostic);
	TestFalse(TEXT("ESH-03 parent diagnostic remains default-off before explicit gate"), DefaultGateDiagnostic.WheelTorqueShift.bEvaluated);

	// WSA radius 없이도 explicit ESH-03 torque crossover와 common RPM 자체는 계산돼야 합니다.
	FCFBuilderWheelTorqueShiftDiagnostic FirstDiagnostic;
	CFBuilderTransUtil::BuildWheelTorqueShiftDiagnostic(
		Performance,
		Drivetrain,
		false,
		0.0f,
		0.0f,
		FirstDiagnostic);

	TestTrue(TEXT("ESH-03 generic diagnostic evaluated"), FirstDiagnostic.bEvaluated);
	TestTrue(TEXT("ESH-03 generic vehicle-specific Engine Curve available"), FirstDiagnostic.bVehicleSpecificEngineCurveAvailable);
	TestEqual(TEXT("ESH-03 generic method id"), FirstDiagnostic.MethodId, FName(TEXT("WheelTorqueCrossoverShift")));
	TestEqual(TEXT("ESH-03 generic method revision"), FirstDiagnostic.MethodRevision, 1);
	TestEqual(TEXT("ESH-03 generic pair count"), FirstDiagnostic.Pairs.Num(), Drivetrain.TransmissionRatios.ForwardGearRatios.Num() - 1);
	TestEqual(TEXT("ESH-03 generic blocker count"), FirstDiagnostic.Blockers.Num(), 0);
	TestTrue(TEXT("ESH-03 generic recommendation inside search range"),
		FirstDiagnostic.RecommendedChangeUpRPM >= FirstDiagnostic.SearchStartRPM
		&& FirstDiagnostic.RecommendedChangeUpRPM <= FirstDiagnostic.SearchEndRPM);
	TestTrue(TEXT("ESH-03 generic fixed-common approximation warning exists"),
		FirstDiagnostic.Warnings.ContainsByPredicate([](const FString& Warning)
		{
			return Warning.Contains(TEXT("Transmission.FixedCommonShiftApproximation"));
		}));

	for (const FCFBuilderWheelTorquePairDiagnostic& Pair : FirstDiagnostic.Pairs)
	{
		TestTrue(
			*FString::Printf(TEXT("ESH-03 generic %d->%d post-shift RPM positive"), Pair.FromGearNumber, Pair.ToGearNumber),
			Pair.RecommendedPostShiftRPM > 0.0f);
		TestTrue(
			*FString::Printf(TEXT("ESH-03 generic %d->%d current wheel torque positive"), Pair.FromGearNumber, Pair.ToGearNumber),
			Pair.RecommendedCurrentWheelTorqueNm > 0.0f);
		TestTrue(
			*FString::Printf(TEXT("ESH-03 generic %d->%d next wheel torque positive"), Pair.FromGearNumber, Pair.ToGearNumber),
			Pair.RecommendedNextWheelTorqueNm > 0.0f);
		TestTrue(
			*FString::Printf(TEXT("ESH-03 generic %d->%d relative gap bounded"), Pair.FromGearNumber, Pair.ToGearNumber),
			Pair.RecommendedRelativeTorqueGap >= 0.0f && Pair.RecommendedRelativeTorqueGap <= 1.0f);
		TestFalse(
			*FString::Printf(TEXT("ESH-03 generic %d->%d speed hidden without WSA authority"), Pair.FromGearNumber, Pair.ToGearNumber),
			Pair.bRecommendedSpeedAvailable);
	}

	// 같은 입력은 byte-order와 무관하게 같은 deterministic recommendation을 내야 합니다.
	FCFBuilderWheelTorqueShiftDiagnostic SecondDiagnostic;
	CFBuilderTransUtil::BuildWheelTorqueShiftDiagnostic(
		Performance,
		Drivetrain,
		false,
		0.0f,
		0.0f,
		SecondDiagnostic);
	TestTrue(TEXT("ESH-03 generic repeated calculation deterministic"), DiagnosticsEquivalent(FirstDiagnostic, SecondDiagnostic));

	// Engine Curve opt-out legacy payload는 오류가 아니라 recommendation unavailable warning이어야 합니다.
	FCFPerformanceProfileData LegacyPerformance = Performance;
	LegacyPerformance.bUseEngineTorqueCurve = false;
	FCFBuilderWheelTorqueShiftDiagnostic LegacyDiagnostic;
	CFBuilderTransUtil::BuildWheelTorqueShiftDiagnostic(
		LegacyPerformance,
		Drivetrain,
		false,
		0.0f,
		0.0f,
		LegacyDiagnostic);
	TestTrue(TEXT("ESH-03 legacy diagnostic evaluated"), LegacyDiagnostic.bEvaluated);
	TestFalse(TEXT("ESH-03 legacy has no vehicle-specific curve authority"), LegacyDiagnostic.bVehicleSpecificEngineCurveAvailable);
	TestEqual(TEXT("ESH-03 legacy no blocker"), LegacyDiagnostic.Blockers.Num(), 0);
	TestEqual(TEXT("ESH-03 legacy no pair recommendation"), LegacyDiagnostic.Pairs.Num(), 0);
	TestTrue(TEXT("ESH-03 legacy curve-unavailable warning exists"),
		LegacyDiagnostic.Warnings.ContainsByPredicate([](const FString& Warning)
		{
			return Warning.Contains(TEXT("Transmission.EngineCurveUnavailable"));
		}));

	// 0 ratio는 wheel-torque 계산에 들어가기 전에 fail-closed해야 합니다.
	FCFDrivetrainProfileData ZeroRatioDrivetrain = Drivetrain;
	ZeroRatioDrivetrain.TransmissionRatios.ForwardGearRatios[1] = 0.0f;
	FCFBuilderWheelTorqueShiftDiagnostic ZeroRatioDiagnostic;
	CFBuilderTransUtil::BuildWheelTorqueShiftDiagnostic(
		Performance,
		ZeroRatioDrivetrain,
		false,
		0.0f,
		0.0f,
		ZeroRatioDiagnostic);
	TestTrue(TEXT("ESH-03 zero ratio is blocked"), ZeroRatioDiagnostic.Blockers.ContainsByPredicate([](const FString& Blocker)
	{
		return Blocker.Contains(TEXT("Transmission.WheelTorqueCrossoverUnavailable"));
	}));

	// 앞단보다 같거나 큰 다음 gear ratio는 P0 ordered forward-ratio 계약 위반입니다.
	FCFDrivetrainProfileData NonDescendingDrivetrain = Drivetrain;
	NonDescendingDrivetrain.TransmissionRatios.ForwardGearRatios[1] = NonDescendingDrivetrain.TransmissionRatios.ForwardGearRatios[0];
	FCFBuilderWheelTorqueShiftDiagnostic NonDescendingDiagnostic;
	CFBuilderTransUtil::BuildWheelTorqueShiftDiagnostic(
		Performance,
		NonDescendingDrivetrain,
		false,
		0.0f,
		0.0f,
		NonDescendingDiagnostic);
	TestTrue(TEXT("ESH-03 non-descending ratio is blocked"), NonDescendingDiagnostic.Blockers.ContainsByPredicate([](const FString& Blocker)
	{
		return Blocker.Contains(TEXT("Transmission.WheelTorqueRatioOrderInvalid"));
	}));

	// Speed diagnostic authority=true일 때 radius 범위가 잘못되면 계산을 진행하지 않습니다.
	FCFBuilderWheelTorqueShiftDiagnostic InvalidRadiusDiagnostic;
	CFBuilderTransUtil::BuildWheelTorqueShiftDiagnostic(
		Performance,
		Drivetrain,
		true,
		-1.0f,
		40.0f,
		InvalidRadiusDiagnostic);
	TestTrue(TEXT("ESH-03 invalid wheel radius is blocked"), InvalidRadiusDiagnostic.Blockers.ContainsByPredicate([](const FString& Blocker)
	{
		return Blocker.Contains(TEXT("Transmission.WheelTorqueRadiusInvalid"));
	}));

	// 극단적인 gear drop으로 EngineMaxRPM에서도 post-shift RPM이 usable curve 시작보다 낮으면 clamp 계산 대신 fail-closed합니다.
	FCFDrivetrainProfileData OutsideCurveDrivetrain = Drivetrain;
	OutsideCurveDrivetrain.TransmissionRatios.ForwardGearRatios = {10.0f, 0.1f};
	FCFBuilderWheelTorqueShiftDiagnostic OutsideCurveDiagnostic;
	CFBuilderTransUtil::BuildWheelTorqueShiftDiagnostic(
		Performance,
		OutsideCurveDrivetrain,
		false,
		0.0f,
		0.0f,
		OutsideCurveDiagnostic);
	TestTrue(TEXT("ESH-03 unusable post-shift RPM is blocked"), OutsideCurveDiagnostic.Blockers.ContainsByPredicate([](const FString& Blocker)
	{
		return Blocker.Contains(TEXT("Transmission.PostShiftRPMOutsideCurve"));
	}));

	// ESH-03를 explicit enable한 parent Transmission diagnostic은 nested blocker를 반드시 상위 Blocker로 전파해야 합니다.
	FCFPerformanceProfileData InvalidCurvePerformance = Performance;
	InvalidCurvePerformance.EngineTorqueCurve.Points[1].EngineRPM = InvalidCurvePerformance.EngineTorqueCurve.Points[0].EngineRPM;
	FCFVehicleResolveResult NoAuthorityResolveResult;
	FCFBuilderTransmissionDiagnostic PropagatedDiagnostic;
	CFBuilderTransUtil::BuildTransmissionDiagnostic(
		ECFBuilderTransmissionPolicy::LegacyCompatible,
		FCFBuilderTransmissionReview(),
		FString(),
		Drivetrain,
		InvalidCurvePerformance,
		NoAuthorityResolveResult,
		PropagatedDiagnostic,
		true);
	TestTrue(TEXT("ESH-03 nested curve blocker propagates to parent transmission diagnostic"), PropagatedDiagnostic.Blockers.ContainsByPredicate([](const FString& Blocker)
	{
		return Blocker.Contains(TEXT("Transmission.EngineCurveInvalid"));
	}));

	return true;
}

// 실제 persisted Wagon Engine Curve/8AT/WSA authority에서 ESH-03 recommendation을 mutation0로 계산합니다.
bool FCFBuilderShiftWagonTest::RunTest(const FString& Parameters)
{
	using namespace CFBuilderShiftTestsPrivate;
	(void)Parameters;

	UCFVehicleRecipeData* Recipe = LoadObject<UCFVehicleRecipeData>(nullptr, WagonRecipePath);
	if (!TestNotNull(TEXT("ESH-03 actual Wagon Recipe loads"), Recipe))
	{
		return false;
	}

	UCFVehicleData* Target = Recipe->TargetVehicleData.LoadSynchronous();
	UCFDrivetrainProfile* Drivetrain = Recipe->ProfileBindings.DrivetrainProfile.LoadSynchronous();
	UCFPerformanceProfile* Performance = Recipe->ProfileBindings.PerformanceProfile.LoadSynchronous();
	if (!TestNotNull(TEXT("ESH-03 actual Wagon Target loads"), Target)
		|| !TestNotNull(TEXT("ESH-03 actual Wagon Drivetrain Profile loads"), Drivetrain)
		|| !TestNotNull(TEXT("ESH-03 actual Wagon Performance Profile loads"), Performance))
	{
		return false;
	}

	// ESH-02 persistent commit과 기존 8AT receipt가 current exact authority인지 선검사합니다.
	TestTrue(TEXT("ESH-03 actual Wagon Performance uses vehicle-specific curve"), Performance->Data.bUseEngineTorqueCurve);
	TestEqual(TEXT("ESH-03 actual Wagon Engine Curve receipt hash"),
		Recipe->BuilderCommitReceipt.EngineCurveProposalHash,
		FString(ExpectedWagonEngineCurveHash));
	TestEqual(TEXT("ESH-03 actual Wagon Transmission receipt hash"),
		Recipe->BuilderCommitReceipt.TransmissionProposalHash,
		FString(ExpectedWagonTransmissionHash));
	TestEqual(TEXT("ESH-03 actual Wagon Engine Curve method"),
		Recipe->BuilderCommitReceipt.EngineCurveReview.MethodId,
		FName(TEXT("SparseAnchorEngineCurveBias")));
	TestEqual(TEXT("ESH-03 actual Wagon forward gear count"), Drivetrain->Data.TransmissionRatios.ForwardGearRatios.Num(), 8);

	// ESH-03 formal diagnostic은 USER-approved Engine Curve + fixed-common 5500 DefinitionApply가 persistent Target까지 완료된 current state에서 진행합니다.
	TestTrue(TEXT("ESH-03 actual Wagon Target Engine Curve is applied"), Target->VehicleMovementConfig.bUseEngineTorqueCurve);
	TestEqual(TEXT("ESH-03 actual Wagon Target ChangeUpRPM is selected 5500"), Target->VehicleMovementConfig.ChangeUpRPM, 5500.0f);
	TestEqual(TEXT("ESH-03 actual Wagon Target ChangeDownRPM remains 2000"), Target->VehicleMovementConfig.ChangeDownRPM, 2000.0f);
	TestEqual(TEXT("ESH-03 actual Wagon Drivetrain ChangeUpRPM is selected 5500"), Drivetrain->Data.ChangeUpRPM, 5500.0f);
	TestEqual(TEXT("ESH-03 actual Wagon Drivetrain ChangeDownRPM remains 2000"), Drivetrain->Data.ChangeDownRPM, 2000.0f);
	TestEqual(TEXT("ESH-03 actual Wagon Target curve point count"), Target->VehicleMovementConfig.EngineTorqueCurve.Points.Num(), 6);
	TestEqual(
		TEXT("ESH-03 actual Wagon Recipe AppliedState hash exact"),
		Recipe->AppliedState.AppliedDefinitionHash,
		FString(ExpectedWagonAppliedTargetHash));
	TestEqual(TEXT("ESH-03 actual Wagon Recipe AppliedState resolver revision is 5"), Recipe->AppliedState.ResolverContractRevision, 5);

	FCFVehicleDefinitionSnapshot TargetBefore;
	FString SnapshotError;
	if (!TestTrue(TEXT("ESH-03 actual Wagon pre-diagnostic target snapshot builds"),
		FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetBefore, SnapshotError)))
	{
		AddError(SnapshotError);
		return false;
	}
	const int32 RecipeRevisionBefore = Recipe->AuthoringRevision;
	const int32 PerformanceRevisionBefore = Performance->Meta.AuthoringRevision;
	const int32 DrivetrainRevisionBefore = Drivetrain->Meta.AuthoringRevision;

	// Current Resolver에서 WSA-authoritative powered-wheel radius를 읽습니다.
	FCFVehicleAuthoringReadRequest ResolveRequest;
	ResolveRequest.Recipe = Recipe;
	ResolveRequest.TargetVehicleData = Target;
	ResolveRequest.CallerKind = ECFAuthoringCallerKind::Automation;

	FCFVehicleResolveReadResult ResolveRead;
	if (!TestTrue(TEXT("ESH-03 actual Wagon Resolver preview succeeds"),
		FCFVehicleAuthoringService::ResolveVehiclePreview(ResolveRequest, ResolveRead)))
	{
		AddError(ResolveRead.Operation.Message);
		return false;
	}

	FCFBuilderTransmissionDiagnostic TransmissionDiagnostic;
	CFBuilderTransUtil::BuildTransmissionDiagnostic(
		Recipe->BuilderTransmissionPolicy,
		Recipe->BuilderCommitReceipt.TransmissionReview,
		Recipe->BuilderCommitReceipt.TransmissionProposalHash,
		Drivetrain->Data,
		Performance->Data,
		ResolveRead.ResolveResult,
		TransmissionDiagnostic,
		true);

	const FCFBuilderWheelTorqueShiftDiagnostic& Shift = TransmissionDiagnostic.WheelTorqueShift;
	TestTrue(TEXT("ESH-03 actual Wagon shift diagnostic evaluated"), Shift.bEvaluated);
	TestTrue(TEXT("ESH-03 actual Wagon vehicle-specific Engine Curve authority"), Shift.bVehicleSpecificEngineCurveAvailable);
	TestEqual(TEXT("ESH-03 actual Wagon pair count"), Shift.Pairs.Num(), 7);
	TestEqual(TEXT("ESH-03 actual Wagon shift blocker count"), Shift.Blockers.Num(), 0);
	TestEqual(TEXT("ESH-03 actual Wagon method id"), Shift.MethodId, FName(TEXT("WheelTorqueCrossoverShift")));
	TestEqual(TEXT("ESH-03 actual Wagon method revision"), Shift.MethodRevision, 1);

	// Approved Engine Curve + current 8AT의 independent reference calculation은 common threshold 약 6222RPM입니다.
	TestTrue(TEXT("ESH-03 actual Wagon common ChangeUp recommendation is near 6222RPM"),
		Shift.RecommendedChangeUpRPM >= 6200 && Shift.RecommendedChangeUpRPM <= 6250);
	TestTrue(TEXT("ESH-03 actual Wagon raw common recommendation remains materially above selected 5500RPM"),
		Shift.RecommendedChangeUpRPM >= 6000);
	TestTrue(TEXT("ESH-03 actual Wagon poor fixed-common fit warning exists"),
		Shift.Warnings.ContainsByPredicate([](const FString& Warning)
		{
			return Warning.Contains(TEXT("Transmission.FixedCommonShiftFitPoor"));
		}));
	TestTrue(TEXT("ESH-03 actual Wagon nested warning propagates to parent transmission diagnostic"),
		TransmissionDiagnostic.Warnings.ContainsByPredicate([](const FString& Warning)
		{
			return Warning.Contains(TEXT("Transmission.FixedCommonShiftFitPoor"));
		}));

	if (Shift.Pairs.Num() == 7)
	{
		// 1→2는 current 6500 runtime boundary까지 crossover가 없어 limit-bound여야 합니다.
		TestFalse(TEXT("ESH-03 actual Wagon 1->2 crossover not found before EngineMaxRPM"), Shift.Pairs[0].bCrossoverFound);
		TestTrue(TEXT("ESH-03 actual Wagon 1->2 is EngineMaxRPM limited"), Shift.Pairs[0].bLimitedByEngineMaxRPM);
		TestTrue(TEXT("ESH-03 actual Wagon 1->2 limit is near 6500RPM"), FMath::IsNearlyEqual(Shift.Pairs[0].CrossoverRPM, 6500.0f, 1.0f));

		// 나머지 pair는 approved Curve 안에서 실제 crossover가 있어야 합니다.
		for (int32 PairIndex = 1; PairIndex < Shift.Pairs.Num(); ++PairIndex)
		{
			TestTrue(
				*FString::Printf(TEXT("ESH-03 actual Wagon %d->%d crossover found"), PairIndex + 1, PairIndex + 2),
				Shift.Pairs[PairIndex].bCrossoverFound);
		}

		// Current WSA resolver가 Success면 theoretical speed도 노출돼야 합니다.
		if (ResolveRead.ResolveResult.ResolveStatus == ECFVehicleResolveStatus::Success)
		{
			TestTrue(TEXT("ESH-03 actual Wagon 1->2 common speed available"), Shift.Pairs[0].bRecommendedSpeedAvailable);
			TestTrue(TEXT("ESH-03 actual Wagon 1->2 recommended speed ~58kmh"),
				Shift.Pairs[0].RecommendedSpeedMaxKmh > 55.0f && Shift.Pairs[0].RecommendedSpeedMaxKmh < 61.0f);
			TestTrue(TEXT("ESH-03 actual Wagon 5->6 recommended speed >220kmh"),
				Shift.Pairs[4].RecommendedSpeedMaxKmh > 220.0f);
		}
	}

	// mutation0 diagnostic 뒤 Product authority가 그대로인지 확인합니다.
	FCFVehicleDefinitionSnapshot TargetAfter;
	if (!TestTrue(TEXT("ESH-03 actual Wagon post-diagnostic target snapshot builds"),
		FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetAfter, SnapshotError)))
	{
		AddError(SnapshotError);
		return false;
	}
	TestEqual(TEXT("ESH-03 actual Wagon Target pre-state hash exact"), TargetBefore.DefinitionHash, FString(ExpectedWagonAppliedTargetHash));
	TestEqual(TEXT("ESH-03 actual Wagon Target hash unchanged"), TargetAfter.DefinitionHash, TargetBefore.DefinitionHash);
	TestEqual(TEXT("ESH-03 actual Wagon Recipe revision unchanged"), Recipe->AuthoringRevision, RecipeRevisionBefore);
	TestEqual(TEXT("ESH-03 actual Wagon Performance revision unchanged"), Performance->Meta.AuthoringRevision, PerformanceRevisionBefore);
	TestEqual(TEXT("ESH-03 actual Wagon Drivetrain revision unchanged"), Drivetrain->Meta.AuthoringRevision, DrivetrainRevisionBefore);
	TestTrue(TEXT("ESH-03 actual Wagon Target Curve remains applied after diagnostic"), Target->VehicleMovementConfig.bUseEngineTorqueCurve);
	TestEqual(TEXT("ESH-03 actual Wagon Target ChangeUpRPM remains 5500 after diagnostic"), Target->VehicleMovementConfig.ChangeUpRPM, 5500.0f);
	TestEqual(TEXT("ESH-03 actual Wagon Target ChangeDownRPM remains 2000 after diagnostic"), Target->VehicleMovementConfig.ChangeDownRPM, 2000.0f);

	// USER review용 machine-readable Saved diagnostic입니다. Product Asset/Recipe/Profile에는 영향이 없습니다.
	FString ShiftJson;
	if (!FJsonObjectConverter::UStructToJsonObjectString(Shift, ShiftJson))
	{
		AddError(TEXT("ESH-03 Wagon shift diagnostic JSON serialize failed."));
		return false;
	}
	const FString ResultPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
		FPaths::ProjectSavedDir(), TEXT("CarFight"), TEXT("VehicleBuilder"), TEXT("WagonShiftPreview.json")));
	if (!FFileHelper::SaveStringToFile(ShiftJson, *ResultPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		AddError(FString::Printf(TEXT("ESH-03 WagonShiftPreview.json write failed: %s"), *ResultPath));
		return false;
	}

	AddInfo(FString::Printf(
		TEXT("WAGON_SHIFT_RECOMMENDATION method=%s@%d common_rpm=%d mse=%.6f max_gap=%.6f pairs=%d target_mutation=false save=false"),
		*Shift.MethodId.ToString(),
		Shift.MethodRevision,
		Shift.RecommendedChangeUpRPM,
		Shift.MeanSquaredRelativeTorqueGap,
		Shift.MaxRelativeTorqueGap,
		Shift.Pairs.Num()));
	return true;
}
