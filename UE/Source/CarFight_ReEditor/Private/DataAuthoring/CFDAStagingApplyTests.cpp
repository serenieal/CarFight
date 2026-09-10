// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAStagingApplyTests.cpp
// Version: v1.3.0
// Date: 2026-09-09
// Description: CF-FQ-049 DAS-P0-03 one-shot approval/global preflight와 loaded-but-unregistered StableIdentity current truth focused Automation입니다.
// Changelog:
// - v1.3.0: DAO-P0-01 provider-owned exact StagingRoot 경계에 맞춰 missing/loaded Automation fixture source를 MissileGuidePreset provider root 하위로 이동.
// - v1.2.0: P0-04 AdapterContractRevision 2 승격에 맞춰 P0-03 current valid Staging fixture revision을 2로 갱신.
// - v1.1.0: current resolver와 Apply/rollback/post-save가 공유하는 exact MissileGuidePreset UObject→typed payload extractor 회귀 검증을 LoadedIdentityPreflight에 추가.
// - v1.0.0: missing reviewed Staging source의 mutation0 ApprovalStale/Consumed lifecycle과 Asset Registry 미등록 loaded identity의 TargetMoved fail-closed를 추가.
// Migration:
// - Product Content Asset을 저장하지 않습니다. loaded identity fixture는 process memory 전용이며 test 종료 시 transient package로 이동해 폐기합니다.

#include "DataAuthoring/CFDAStagingApply.h"

#include "CFMissileGuidePresetData.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "UObject/Package.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDAStagingApplyTestsPrivate
{
	// strict Pilot whole-record JSON을 unique identity/target으로 생성합니다.
	FString BuildValidJson(
		const FString& StableLogicalId,
		const FString& TargetObjectPath)
	{
		return FString::Printf(
			TEXT("{")
			TEXT("\"SchemaId\":\"CarFight.DataAsset.MissileGuidePreset\",")
			TEXT("\"SchemaRevision\":1,")
			TEXT("\"AdapterContractRevision\":2,")
			TEXT("\"DataAssetTypeClassPath\":\"/Script/CarFight_Re.CFMissileGuidePresetData\",")
			TEXT("\"StableLogicalId\":\"%s\",")
			TEXT("\"TargetObjectPath\":\"%s\",")
			TEXT("\"BaseSemanticFingerprint\":null,")
			TEXT("\"Payload\":{")
			TEXT("\"PresetId\":\"%s\",")
			TEXT("\"PresetDisplayName\":{\"Kind\":\"Literal\",\"Text\":\"P0-03 Automation\"},")
			TEXT("\"PresetDescription\":{\"Kind\":\"Literal\",\"Text\":\"mutation0 preflight fixture\"},")
			TEXT("\"MissileGuideConfig\":{")
			TEXT("\"bUseGuidance\":true,")
			TEXT("\"GuideMode\":\"TargetActor\",")
			TEXT("\"LostTargetPolicy\":\"ContinueStraight\",")
			TEXT("\"NavigationConstant\":3.0,")
			TEXT("\"MaximumTurnRateDegPerSec\":35.0,")
			TEXT("\"MaximumLateralAccelerationCmPerSecSq\":2000.0,")
			TEXT("\"GuidanceResponseTimeSeconds\":0.18,")
			TEXT("\"MinimumGuidanceSpeedCmPerSec\":500.0,")
			TEXT("\"SeekerFieldOfViewDeg\":60.0,")
			TEXT("\"LockBreakAngleDeg\":85.0,")
			TEXT("\"TargetLostGraceTimeSeconds\":0.2,")
			TEXT("\"SeekerModel\":\"Stateful\",")
			TEXT("\"TargetObservationMode\":\"SampledPositionEstimate\",")
			TEXT("\"GuidanceLaw\":\"PurePursuit\",")
			TEXT("\"GuidanceActivationMode\":\"Independent\",")
			TEXT("\"GuidanceActivationDelaySeconds\":0.25,")
			TEXT("\"GuidanceActivationDistanceCm\":500.0,")
			TEXT("\"LeadTimeSeconds\":0.0,")
			TEXT("\"MaxLeadDistanceCm\":0.0,")
			TEXT("\"ReacquisitionMode\":\"None\",")
			TEXT("\"TargetObservationIntervalSeconds\":0.08,")
			TEXT("\"TargetVelocityEstimateResponseTimeSeconds\":0.25,")
			TEXT("\"AcquisitionConeHalfAngleDeg\":45.0,")
			TEXT("\"TrackingConeHalfAngleDeg\":60.0,")
			TEXT("\"ReacquisitionConeHalfAngleDeg\":60.0,")
			TEXT("\"ReacquisitionTimeSeconds\":0.0")
			TEXT("}")
			TEXT("}")
			TEXT("}"),
			*StableLogicalId,
			*TargetObjectPath,
			*StableLogicalId);
	}

	// unique test token으로 collision 없는 Pilot identity를 생성합니다.
	FString BuildUniqueIdentity(const TCHAR* Prefix)
	{
		return FString::Printf(TEXT("%s_%s"), Prefix, *FGuid::NewGuid().ToString(EGuidFormats::Digits));
	}

	// loaded-only fixture UObject를 Asset Registry에 등록하지 않은 채 transient ownership으로 정리합니다.
	bool CleanupLoadedFixture(UCFMissileGuidePresetData* Asset, UPackage* Package)
	{
		if (Asset == nullptr || Package == nullptr)
		{
			return false;
		}

		Asset->ClearFlags(RF_Public | RF_Standalone);
		// exact /Game test object path에서 ownership을 제거하는 rename 결과입니다.
		const bool bRenamed = Asset->Rename(
			nullptr,
			GetTransientPackage(),
			REN_DontCreateRedirectors | REN_NonTransactional);
		Asset->MarkAsGarbage();
		Package->SetDirtyFlag(false);
		return bRenamed && !Package->IsDirty();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAStagingApprovalLifecycleTest,
	"CarFight.DataManagement.CF_FQ_049.DAS_P0_03.ApprovalLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAStagingLoadedIdentityTest,
	"CarFight.DataManagement.CF_FQ_049.DAS_P0_03.LoadedIdentityPreflight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Reviewed approval이 stale disk source에서 mutation0으로 one-shot 소비되는지 검증합니다.
bool FCFDAStagingApprovalLifecycleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAStagingApplyTestsPrivate;

	// 다른 test/process object와 겹치지 않는 unique stable identity입니다.
	const FString StableLogicalId = BuildUniqueIdentity(TEXT("DASP03Approval"));
	// Product Content와 겹치지 않는 absent target object path입니다.
	const FString TargetObjectPath = FString::Printf(
		TEXT("/Game/CarFight/Tests/DAStagingP03/DA_%s.DA_%s"),
		*StableLogicalId,
		*StableLogicalId);
	// intentionally 존재하지 않는 canonical Staging source path입니다.
	const FString MissingStagingPath = FString::Printf(
		TEXT("Authoring/DataAssetStaging/MissileGuidePreset/__AutomationMissing__/%s.json"),
		*StableLogicalId);

	// valid whole-record typed parse result입니다.
	const FCFDAStagingParseResult ParseResult = FCFDAStagingService::ParseMissilePresetJson(
		BuildValidJson(StableLogicalId, TargetObjectPath),
		MissingStagingPath);
	if (!TestTrue(TEXT("P0-03 approval fixture parse succeeds"), ParseResult.bValid))
	{
		return false;
	}

	// absent current truth에서 expected Create Preview입니다.
	const FCFDAStagingPreviewRow PreviewRow = FCFDAStagingService::BuildPreview(
		ParseResult.Record,
		FCFDAStagingCurrentState());
	if (!TestEqual(TEXT("P0-03 approval fixture classifies Create"), PreviewRow.Kind, ECFDAStagingPreviewKind::Create))
	{
		return false;
	}

	// exact single-row reviewed input입니다.
	const TArray<FCFDAStagingPreviewRow> PreviewRows = {PreviewRow};
	// generated one-shot Reviewed approval입니다.
	FCFDAStagingReviewedApproval Approval;
	// approval generation error입니다.
	FString ApprovalError;
	if (!TestTrue(
		TEXT("Exact Create Preview freezes Reviewed approval"),
		FCFDAStagingApplyService::BuildReviewedApproval(PreviewRows, Approval, ApprovalError)))
	{
		AddError(ApprovalError);
		return false;
	}
	TestEqual(TEXT("Approval lifecycle enters Reviewed"), Approval.State, ECFDAStagingApprovalState::Reviewed);
	TestFalse(TEXT("Reviewed approval has BatchPlanHash"), Approval.BatchPlanHash.IsEmpty());
	TestEqual(TEXT("Reviewed approval contains one mutation target"), Approval.IncludedTargets.Num(), 1);

	// stale missing-source Apply terminal report입니다.
	FCFDAStagingApplyReport ApplyReport;
	TestFalse(TEXT("Missing reviewed Staging source blocks Apply"), FCFDAStagingApplyService::ApplyReviewedBatch(Approval, ApplyReport));
	TestEqual(TEXT("Blocked Apply consumes approval"), Approval.State, ECFDAStagingApprovalState::Consumed);
	TestTrue(TEXT("Blocked Apply reports approval consumed"), ApplyReport.bApprovalConsumed);
	TestEqual(TEXT("Missing source is BlockedBeforeMutation"), ApplyReport.Result, ECFDAStagingBatchApplyResult::BlockedBeforeMutation);
	TestEqual(TEXT("No durable mutation occurred"), ApplyReport.DurableAppliedCount, 0);
	if (!ApplyReport.Targets.IsEmpty())
	{
		TestEqual(
			TEXT("Exact stale target reports BlockedBeforeMutation"),
			ApplyReport.Targets[0].Result,
			ECFDAStagingTargetApplyResult::BlockedBeforeMutation);
		TestTrue(
			TEXT("Stale target includes ApprovalStale diagnostic"),
			FCFDAStagingService::HasIssueCode(ApplyReport.Targets[0].Issues, ECFDAStagingIssueCode::ApprovalStale));
	}

	// consumed approval을 자동 retry하려는 second call result입니다.
	FCFDAStagingApplyReport RetryReport;
	TestFalse(TEXT("Consumed approval cannot be retried"), FCFDAStagingApplyService::ApplyReviewedBatch(Approval, RetryReport));
	TestEqual(TEXT("Consumed approval remains Consumed"), Approval.State, ECFDAStagingApprovalState::Consumed);
	TestEqual(TEXT("Retry is blocked before mutation"), RetryReport.Result, ECFDAStagingBatchApplyResult::BlockedBeforeMutation);
	return true;
}

// Asset Registry에 등록하지 않은 loaded same StableIdentity도 global current truth에 잡히는지 검증합니다.
bool FCFDAStagingLoadedIdentityTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFDAStagingApplyTestsPrivate;

	// 다른 persisted/loaded asset과 겹치지 않는 unique identity입니다.
	const FString StableLogicalId = BuildUniqueIdentity(TEXT("DASP03Loaded"));
	// loaded-only fixture package long name입니다.
	const FString LoadedPackageName = FString::Printf(
		TEXT("/Game/CarFight/Tests/DAStagingP03/Loaded_%s"),
		*StableLogicalId);
	// loaded-only fixture asset object name입니다.
	const FString LoadedAssetName = FString::Printf(TEXT("DA_Loaded_%s"), *StableLogicalId);
	// loaded object와 다른 requested target path입니다.
	const FString RequestedTargetPath = FString::Printf(
		TEXT("/Game/CarFight/Tests/DAStagingP03/Requested_%s.Requested_%s"),
		*StableLogicalId,
		*StableLogicalId);
	// parse contract을 만족하는 canonical source path입니다.
	const FString StagingPath = FString::Printf(
		TEXT("Authoring/DataAssetStaging/MissileGuidePreset/__AutomationLoaded__/%s.json"),
		*StableLogicalId);

	// Asset Registry에 notify하지 않을 loaded-only fixture package입니다.
	UPackage* LoadedPackage = CreatePackage(*LoadedPackageName);
	// Asset Registry 미등록 exact Pilot loaded object입니다.
	UCFMissileGuidePresetData* LoadedAsset = LoadedPackage
		? NewObject<UCFMissileGuidePresetData>(
			LoadedPackage,
			FName(*LoadedAssetName),
			RF_Public | RF_Standalone | RF_Transactional)
		: nullptr;
	if (!TestNotNull(TEXT("Loaded-only fixture package exists"), LoadedPackage)
		|| !TestNotNull(TEXT("Loaded-only fixture asset exists"), LoadedAsset))
	{
		return false;
	}
	LoadedAsset->PresetId = FName(*StableLogicalId);
	LoadedPackage->SetDirtyFlag(false);

	// current/apply 양쪽이 공유하는 exact whole-record semantic extractor 결과입니다.
	FCFDAMissilePresetPayload ExtractedPayload;
	// shared extractor의 typed/FText representation diagnostics입니다.
	TArray<FCFDAStagingIssue> ExtractIssues;
	TestTrue(
		TEXT("Loaded-only fixture uses shared typed semantic extractor"),
		FCFDAStagingService::ExtractMissilePresetPayload(*LoadedAsset, ExtractedPayload, ExtractIssues));
	TestEqual(TEXT("Shared extractor preserves StableIdentity"), ExtractedPayload.PresetId, FName(*StableLogicalId));

	// loaded fixture exact object path입니다.
	const FString LoadedObjectPath = FSoftObjectPath(LoadedAsset).ToString();

	// same identity를 다른 requested target에 Create하려는 typed record입니다.
	const FCFDAStagingParseResult ParseResult = FCFDAStagingService::ParseMissilePresetJson(
		BuildValidJson(StableLogicalId, RequestedTargetPath),
		StagingPath);
	if (!TestTrue(TEXT("Loaded identity fixture parse succeeds"), ParseResult.bValid))
	{
		CleanupLoadedFixture(LoadedAsset, LoadedPackage);
		return false;
	}

	// Asset Registry + loaded object union으로 읽은 current truth입니다.
	FCFDAStagingCurrentState CurrentState;
	// current resolver diagnostics입니다.
	TArray<FCFDAStagingIssue> ResolveIssues;
	const bool bResolveSucceeded = FCFDAStagingService::ResolveMissilePresetCurrentState(
		ParseResult.Record,
		CurrentState,
		ResolveIssues);
	TestTrue(TEXT("Loaded-but-unregistered StableIdentity resolver succeeds"), bResolveSucceeded);
	TestTrue(TEXT("Loaded-but-unregistered identity is detected"), CurrentState.bStableIdentityExists);
	TestEqual(TEXT("Unique loaded identity has exactly one match"), CurrentState.StableIdentityMatchCount, 1);
	TestEqual(TEXT("Loaded identity object path is exact"), CurrentState.StableIdentityObjectPath, LoadedObjectPath);

	// same identity가 다른 path에 이미 있으므로 expected TargetMoved conflict입니다.
	const FCFDAStagingPreviewRow PreviewRow = FCFDAStagingService::BuildPreview(ParseResult.Record, CurrentState);
	TestEqual(TEXT("Loaded same identity blocks Create as Conflict"), PreviewRow.Kind, ECFDAStagingPreviewKind::Conflict);
	TestTrue(
		TEXT("Loaded same identity emits TargetMoved"),
		FCFDAStagingService::HasIssueCode(PreviewRow.Issues, ECFDAStagingIssueCode::TargetMoved));

	TestTrue(TEXT("Loaded-only fixture cleanup succeeds"), CleanupLoadedFixture(LoadedAsset, LoadedPackage));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
