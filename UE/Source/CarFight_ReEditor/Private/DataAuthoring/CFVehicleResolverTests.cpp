// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleResolverTests.cpp
// Version: v1.7.0
// Date: 2026-09-03
// Description: DAUTH-P0-08E/F Pure Resolver + CF-FQ-047 stable Mount legacy passthrough integrity Automation입니다.
// Changelog:
// - v1.7.0: CF-FQ-047 P0-06. active Standard Mount 신규 row의 hidden legacy leaf C++ default fallback과 existing same-ID current serialized passthrough source/value를 focused 검증.
// - v1.6.0: Performance Profile EngineTorqueCurve opt-in source mapping, atomic materialization, invalid curve fail-closed와 Resolver revision 5 회귀검증 추가.
// - v1.5.0: WSA-P0-05 Recipe SoftObject ChassisMesh → Target Object canonical reference가 R15 materialized readback hash와 일치하는 회귀검증 추가.
// - v1.4.0: WSA-P0-04 SocketScaleFromChassis의 FL-only shared Wheel fallback을 Resolver에서 직접 회귀검증.
// - v1.3.0: WSA-P0-02 SocketScaleFromChassis Radius/Width, narrow fingerprint, invalid scale/axle mismatch와 AssetAdoption source를 focused 검증.
// - v1.2.0: R15 Completed semantics, DefinitionValidation Blocked 분리, Stable-ID array reconstruction, materialized readback hash consistency와 FieldCodec import fail-closed 검증 추가.
// - v1.1.0: R14 FieldDiff + R16 Effective/External Drift foundation과 same-precedence fail-closed conflict 검증 추가.
// - v1.0.0: Frozen R0~R16, proposal/adoption, PreviewContext, Advanced/Legacy/Derived precedence, fingerprint mismatch, legacy serialized 검증 최초 구현.
// Migration:
// - Runtime/Content Asset mutation 없이 Snapshot value와 RF_Transient UCFVehicleData candidate만 사용합니다.
// - Synthetic request는 실제 Chassis/Wheel Class가 없으므로 R15 이후 DefinitionValidation Blocked가 정상이며 Resolver internal Error와 구분합니다.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehicleData.h"
#include "DataAuthoring/CFVehicleFieldCodec.h"
#include "DataAuthoring/CFVehicleFieldRegistry.h"
#include "DataAuthoring/CFVehicleMaterializer.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Engine/StaticMesh.h"

namespace CFVehicleResolverTestsPrivate
{
	// Canonical Registry path로 structured path를 찾습니다.
	FCFVehicleFieldPath FindRegistryPath(const TCHAR* CanonicalPattern)
	{
		// Frozen Registry descriptor입니다.
		const FCFVehicleFieldDescriptor* Descriptor = FCFVehicleFieldRegistry::GetDescriptors().FindByPredicate([CanonicalPattern](const FCFVehicleFieldDescriptor& Candidate)
		{
			return Candidate.GetCanonicalPattern() == CanonicalPattern;
		});
		return Descriptor ? Descriptor->StablePathPattern : FCFVehicleFieldPath();
	}

	// Stable collection wildcard path에 exact selector를 채웁니다.
	FCFVehicleFieldPath MakeExactPath(const TCHAR* CanonicalPattern, const FName SelectorValue)
	{
		// Exact selector를 채울 Registry path입니다.
		FCFVehicleFieldPath Result = FindRegistryPath(CanonicalPattern);
		Result.SelectorKeyValue = SelectorValue;
		return Result;
	}

	// Target float field signature로 exact override value를 만듭니다.
	FCFVehicleFieldOverride MakeFloatOverride(const TCHAR* CanonicalPath, const float Value)
	{
		// 반환할 exact field override입니다.
		FCFVehicleFieldOverride Override;
		Override.FieldPath = FindRegistryPath(CanonicalPath);

		// VehicleMovement float leaf name입니다.
		const FName LeafName = Override.FieldPath.PropertyChain.Last();
		// Target-compatible float property입니다.
		const FFloatProperty* FloatProperty = FindFProperty<FFloatProperty>(FCFVehicleMovementConfig::StaticStruct(), LeafName);
		if (FloatProperty)
		{
			// Test helper export error입니다.
			FString ExportError;
			FCFVehicleFieldCodec::ExportValue(*FloatProperty, &Value, Override.OverrideValue, ExportError);
		}
		return Override;
	}

	// Target bool field signature로 exact override value를 만듭니다.
	FCFVehicleFieldOverride MakeBoolOverride(const TCHAR* CanonicalPath, const bool Value)
	{
		// 반환할 exact field override입니다.
		FCFVehicleFieldOverride Override;
		Override.FieldPath = FindRegistryPath(CanonicalPath);

		// VehicleMovement bool leaf name입니다.
		const FName LeafName = Override.FieldPath.PropertyChain.Last();
		// Target-compatible bool property입니다.
		const FBoolProperty* BoolProperty = FindFProperty<FBoolProperty>(FCFVehicleMovementConfig::StaticStruct(), LeafName);
		if (BoolProperty)
		{
			// Test helper export error입니다.
			FString ExportError;
			FCFVehicleFieldCodec::ExportValue(*BoolProperty, &Value, Override.OverrideValue, ExportError);
		}
		return Override;
	}

	// Project Default wildcard entry를 exact selector override로 복사합니다.
	FCFVehicleFieldOverride MakeDefaultBackedArrayOverride(
		const FCFVehicleDefinitionSnapshot& ProjectDefaults,
		const TCHAR* CanonicalPattern,
		const FName SelectorValue)
	{
		// 반환할 exact legacy override입니다.
		FCFVehicleFieldOverride Override;
		// Default Snapshot의 wildcard field entry입니다.
		const FCFVehicleFieldEntry* DefaultEntry = ProjectDefaults.SortedFields.FindByPredicate([CanonicalPattern](const FCFVehicleFieldEntry& Entry)
		{
			return Entry.FieldPath.ToCanonicalString(true) == CanonicalPattern;
		});
		if (DefaultEntry)
		{
			Override.FieldPath = DefaultEntry->FieldPath;
			Override.FieldPath.SelectorKeyValue = SelectorValue;
			Override.OverrideValue = DefaultEntry->Value;
		}
		return Override;
	}

	// Asset Snapshot에 deterministic found wheel socket fact를 추가합니다.
	void AddSocket(FCFVehicleAssetSnapshot& Assets, const FName SocketName, const FVector& Location)
	{
		// Chassis socket immutable fact입니다.
		FCFVehicleSocketSnapshot& Socket = Assets.ChassisSockets.AddDefaulted_GetRef();
		Socket.SocketName = SocketName;
		Socket.bFound = true;
		Socket.RelativeLocation = Location;
		Socket.RelativeRotation = FRotator::ZeroRotator;
		Socket.RelativeScale = FVector::OneVector;
	}

	// Resolver Foundation이 사용할 managed baseline request를 구성합니다.
	bool BuildManagedRequest(FCFVehicleResolveRequest& OutRequest, FString& OutError)
	{
		OutRequest = FCFVehicleResolveRequest();
		OutRequest.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
		OutRequest.Recipe.RecipeId = FGuid::NewGuid();
		OutRequest.Recipe.RecipeFingerprint = TEXT("recipe-foundation-v1");
		OutRequest.Recipe.ImportState.ManageState = ECFVehicleManageState::Managed;
		OutRequest.Recipe.DriveStateMode = ECFVehicleDriveStateMode::ProjectDefault;
		OutRequest.Recipe.AuthoringRevision = 7;

		// Managed Resolve에 필요한 4개 primary Profile source identity/fingerprint입니다.
		OutRequest.Profiles.BaseSource.SourceObjectPath = FSoftObjectPath(TEXT("/Game/Test/P_Base.P_Base"));
		OutRequest.Profiles.BaseSource.ProfileFingerprint = TEXT("base-profile-v1");
		OutRequest.Profiles.DrivetrainSource.SourceObjectPath = FSoftObjectPath(TEXT("/Game/Test/P_Drive.P_Drive"));
		OutRequest.Profiles.DrivetrainSource.ProfileFingerprint = TEXT("drive-profile-v1");
		OutRequest.Profiles.HandlingSource.SourceObjectPath = FSoftObjectPath(TEXT("/Game/Test/P_Handling.P_Handling"));
		OutRequest.Profiles.HandlingSource.ProfileFingerprint = TEXT("handling-profile-v1");
		OutRequest.Profiles.PerformanceSource.SourceObjectPath = FSoftObjectPath(TEXT("/Game/Test/P_Perf.P_Perf"));
		OutRequest.Profiles.PerformanceSource.ProfileFingerprint = TEXT("performance-profile-v1");

		// Base Profile resolver payload입니다.
		OutRequest.Profiles.BaseData.BaseVehicleMassKg = 1500.0f;
		OutRequest.Profiles.BaseData.MaximumGrossMassKg = 2200.0f;
		OutRequest.Profiles.BaseData.MaxHealth = 250.0f;
		OutRequest.Profiles.BaseData.ChassisHeight = 55.0f;
		OutRequest.Profiles.BaseData.ExpectedWheelCount = 4;
		OutRequest.Profiles.BaseData.FrontWheelCountForSteering = 2;
		OutRequest.Profiles.BaseData.bAutoScaleWheelMeshToRadius = true;
		OutRequest.Profiles.BaseData.WheelMeshRadiusMeasureMode = ECFWheelMeshRadiusMeasureMode::AutoMaxXZ;
		OutRequest.Profiles.BaseData.bAutoCenterWheelMeshBoundsToOrigin = true;
		OutRequest.Profiles.BaseData.WheelMeshScaleClampMin = 0.25f;
		OutRequest.Profiles.BaseData.WheelMeshScaleClampMax = 4.0f;

		// Handling Profile direct + Feel response payload입니다.
		OutRequest.Profiles.HandlingData.FrontWheelMaxSteerAngleByFeel = {25.0f, 35.0f, 45.0f};
		OutRequest.Profiles.HandlingData.SteeringAngleRatioByFeel = {0.5f, 0.7f, 0.9f};
		OutRequest.Profiles.HandlingData.FrontWheelFrictionByFeel = {1.0f, 1.5f, 2.0f};
		OutRequest.Profiles.HandlingData.RearWheelFrictionByFeel = {1.0f, 1.4f, 1.8f};
		OutRequest.Profiles.HandlingData.FrontCorneringByFeel = {700.0f, 900.0f, 1100.0f};
		OutRequest.Profiles.HandlingData.RearCorneringByFeel = {650.0f, 850.0f, 1050.0f};
		OutRequest.Profiles.HandlingData.FrontSpringRateByFeel = {100.0f, 150.0f, 200.0f};
		OutRequest.Profiles.HandlingData.RearSpringRateByFeel = {100.0f, 145.0f, 190.0f};
		OutRequest.Profiles.HandlingData.FrontSpringPreloadByFeel = {20.0f, 30.0f, 40.0f};
		OutRequest.Profiles.HandlingData.RearSpringPreloadByFeel = {20.0f, 28.0f, 36.0f};
		OutRequest.Profiles.HandlingData.FrontWheelMaxBrakeTorque = 1800.0f;
		OutRequest.Profiles.HandlingData.RearWheelMaxBrakeTorque = 1600.0f;
		OutRequest.Profiles.HandlingData.RearWheelMaxHandBrakeTorque = 2200.0f;
		OutRequest.Profiles.HandlingData.FrontWheelLoadRatio = 0.5f;
		OutRequest.Profiles.HandlingData.RearWheelLoadRatio = 0.5f;
		OutRequest.Profiles.HandlingData.FrontWheelSuspensionMaxRaise = 8.0f;
		OutRequest.Profiles.HandlingData.RearWheelSuspensionMaxRaise = 8.0f;
		OutRequest.Profiles.HandlingData.FrontWheelSuspensionMaxDrop = 12.0f;
		OutRequest.Profiles.HandlingData.RearWheelSuspensionMaxDrop = 12.0f;

		// Performance Profile direct + Acceleration response payload입니다.
		OutRequest.Profiles.PerformanceData.EngineMaxTorqueByFeel = {500.0f, 800.0f, 1100.0f};
		OutRequest.Profiles.PerformanceData.EngineMaxRPMByFeel = {4500.0f, 6000.0f, 7500.0f};
		OutRequest.Profiles.PerformanceData.ThrottleInputScaleByFeel = {0.8f, 1.0f, 1.2f};
		OutRequest.Profiles.PerformanceData.EngineIdleRPM = 850.0f;
		OutRequest.Profiles.PerformanceData.EngineBrakeEffect = 0.1f;
		OutRequest.Profiles.PerformanceData.EngineRevUpMOI = 5.0f;
		OutRequest.Profiles.PerformanceData.EngineRevDownRate = 600.0f;
		OutRequest.Profiles.PerformanceData.DragCoefficient = 0.3f;
		OutRequest.Profiles.PerformanceData.DownforceCoefficient = 0.2f;

		// Project C++ Compatibility Default 117-pattern Snapshot입니다.
		if (!FCFVehicleSnapshotBuilder::BuildProjectCompatibilityDefaultSnapshot(OutRequest.ProjectDefaults, OutError))
		{
			return false;
		}

		// Reader를 다시 호출하지 않고 직접 구성하는 immutable Asset Snapshot facts입니다.
		OutRequest.Assets.ChassisObjectPath = FSoftObjectPath(TEXT("/Game/Test/SM_Chassis.SM_Chassis"));
		OutRequest.Assets.bChassisLoaded = true;
		OutRequest.Assets.ChassisLayoutFingerprint = TEXT("chassis-layout-v1");
		AddSocket(OutRequest.Assets, TEXT("Wheel_Anchor_FL"), FVector(100.0, -60.0, 25.0));
		AddSocket(OutRequest.Assets, TEXT("Wheel_Anchor_FR"), FVector(100.0, 60.0, 25.0));
		AddSocket(OutRequest.Assets, TEXT("Wheel_Anchor_RL"), FVector(-100.0, -60.0, 25.0));
		AddSocket(OutRequest.Assets, TEXT("Wheel_Anchor_RR"), FVector(-100.0, 60.0, 25.0));
		OutRequest.Assets.ChassisSockets.Sort([](const FCFVehicleSocketSnapshot& Left, const FCFVehicleSocketSnapshot& Right)
		{
			return Left.SocketName.LexicalLess(Right.SocketName);
		});

		// 동일 규격 4개 Wheel local bounds facts입니다.
		FCFVehicleWheelAssetSnapshot* WheelSnapshots[] = {&OutRequest.Assets.WheelFL, &OutRequest.Assets.WheelFR, &OutRequest.Assets.WheelRL, &OutRequest.Assets.WheelRR};
		const TCHAR* WheelPaths[] = {TEXT("/Game/Test/SM_WheelFL.SM_WheelFL"), TEXT("/Game/Test/SM_WheelFR.SM_WheelFR"), TEXT("/Game/Test/SM_WheelRL.SM_WheelRL"), TEXT("/Game/Test/SM_WheelRR.SM_WheelRR")};
		const TCHAR* WheelFingerprints[] = {TEXT("wheel-fl-v1"), TEXT("wheel-fr-v1"), TEXT("wheel-rl-v1"), TEXT("wheel-rr-v1")};
		for (int32 WheelIndex = 0; WheelIndex < UE_ARRAY_COUNT(WheelSnapshots); ++WheelIndex)
		{
			WheelSnapshots[WheelIndex]->ObjectPath = FSoftObjectPath(WheelPaths[WheelIndex]);
			WheelSnapshots[WheelIndex]->bAssetLoaded = true;
			WheelSnapshots[WheelIndex]->BoundsOrigin = FVector::ZeroVector;
			WheelSnapshots[WheelIndex]->BoundsExtent = FVector(32.0, 11.0, 30.0);
			WheelSnapshots[WheelIndex]->MeasureFingerprint = WheelFingerprints[WheelIndex];
		}

		OutError.Reset();
		return true;
	}

	// 첫 resolve의 proposal fingerprints를 Recipe explicit adoption 상태로 복사합니다.
	void AdoptAllWheelMeasurements(FCFVehicleResolveRequest& Request, const FCFVehicleResolveResult& ProposalResult)
	{
		for (const FCFVehicleMeasurementProposal& Proposal : ProposalResult.MeasurementProposals)
		{
			// Proposal exact canonical field path입니다.
			const FString CanonicalPath = Proposal.FieldPath.ToCanonicalString(true);
			if (CanonicalPath == TEXT("VehicleMovementConfig.FrontWheelRadius"))
			{
				Request.Recipe.AssetAdoption.bUseMeasuredFrontRadius = true;
				Request.Recipe.AssetAdoption.FrontRadiusAssetFingerprint = Proposal.AssetFingerprint;
			}
			else if (CanonicalPath == TEXT("VehicleMovementConfig.RearWheelRadius"))
			{
				Request.Recipe.AssetAdoption.bUseMeasuredRearRadius = true;
				Request.Recipe.AssetAdoption.RearRadiusAssetFingerprint = Proposal.AssetFingerprint;
			}
			else if (CanonicalPath == TEXT("VehicleMovementConfig.FrontWheelWidth"))
			{
				Request.Recipe.AssetAdoption.bUseMeasuredFrontWidth = true;
				Request.Recipe.AssetAdoption.FrontWidthAssetFingerprint = Proposal.AssetFingerprint;
			}
			else if (CanonicalPath == TEXT("VehicleMovementConfig.RearWheelWidth"))
			{
				Request.Recipe.AssetAdoption.bUseMeasuredRearWidth = true;
				Request.Recipe.AssetAdoption.RearWidthAssetFingerprint = Proposal.AssetFingerprint;
			}
		}
	}

	// ResolveResult에서 exact source trace를 찾습니다.
	const FCFVehicleSourceTrace* FindTrace(const FCFVehicleResolveResult& Result, const TCHAR* CanonicalPath)
	{
		return Result.PreviewSourceTrace.FindByPredicate([CanonicalPath](const FCFVehicleSourceTrace& Trace)
		{
			return Trace.FieldPath.ToCanonicalString(true) == CanonicalPath;
		});
	}

	// ResolveResult에서 exact resolved field를 찾습니다.
	const FCFVehicleResolvedField* FindResolvedField(const FCFVehicleResolveResult& Result, const TCHAR* CanonicalPath)
	{
		return Result.SortedResolvedFields.FindByPredicate([CanonicalPath](const FCFVehicleResolvedField& Field)
		{
			return Field.FieldPath.ToCanonicalString(true) == CanonicalPath;
		});
	}

	// ResolveResult에서 exact measurement proposal을 찾습니다.
	const FCFVehicleMeasurementProposal* FindProposal(const FCFVehicleResolveResult& Result, const TCHAR* CanonicalPath)
	{
		return Result.MeasurementProposals.FindByPredicate([CanonicalPath](const FCFVehicleMeasurementProposal& Proposal)
		{
			return Proposal.FieldPath.ToCanonicalString(true) == CanonicalPath;
		});
	}

	// Asset Snapshot에서 exact Chassis socket mutable fact를 찾습니다.
	FCFVehicleSocketSnapshot* FindSocket(FCFVehicleAssetSnapshot& Assets, const FName SocketName)
	{
		return Assets.ChassisSockets.FindByPredicate([SocketName](const FCFVehicleSocketSnapshot& Socket)
		{
			return Socket.SocketName == SocketName;
		});
	}

	// Mount legacy float leaf의 exact current Definition entry를 target-compatible codec으로 만듭니다.
	FCFVehicleFieldEntry MakeMountFloatCurrentEntry(const TCHAR* CanonicalPattern, const FName SelectorValue, const float Value)
	{
		FCFVehicleFieldEntry Entry;
		Entry.FieldPath = MakeExactPath(CanonicalPattern, SelectorValue);
		const FName LeafName = Entry.FieldPath.PropertyChain.Last();
		const FFloatProperty* FloatProperty = FindFProperty<FFloatProperty>(FCFVehicleMountProfile::StaticStruct(), LeafName);
		if (FloatProperty)
		{
			FString ExportError;
			FCFVehicleFieldCodec::ExportValue(*FloatProperty, &Value, Entry.Value, ExportError);
		}
		return Entry;
	}

	// Validation bucket에 issue code가 존재하는지 검사합니다.
	bool HasIssueCode(const TArray<FCFVehicleValidationIssue>& Issues, const FName IssueCode)
	{
		return Issues.ContainsByPredicate([IssueCode](const FCFVehicleValidationIssue& Issue)
		{
			return Issue.IssueCode == IssueCode;
		});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleResolverDeterminismTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Resolver.StageDeterminism",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleResolverEngineCurveTest,
	"CarFight.DataAuthoring.CF_FQ_040.ESH_01.Resolver.EngineTorqueCurve",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleResolverPrecedenceTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Resolver.PrecedenceTrace",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleResolverFingerprintTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Resolver.MeasurementFingerprint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleResolverSocketScaleTest,
	"CarFight.DataAuthoring.CF_FQ_040.WSA_P0_02.Resolver.SocketScaleDerived",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleResolverLegacyTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Resolver.LegacySerialized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleResolverMountLegacyFallbackTest,
	"CarFight.DataAuthoring.CF_FQ_047.P0_06.Resolver.MountLegacyFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleResolverDiffStaleTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Resolver.DiffStaleFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleResolverConflictTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Resolver.ConflictFailClosed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleResolverObjectReferenceRoundTripTest,
	"CarFight.DataAuthoring.CF_FQ_040.WSA_P0_05.Resolver.ObjectReferenceRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleMaterializerArraysTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Materializer.StableArraysHash",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleMaterializerImportTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Materializer.ImportFailClosed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Frozen 17 stage 순서, proposal/adoption 분리, deterministic signature와 PreviewContext 비Source성을 검증합니다.
bool FCFVehicleResolverDeterminismTest::RunTest(const FString& Parameters)
{
	// Baseline managed Resolver request입니다.
	FCFVehicleResolveRequest Request;
	// Request builder 실패 사유입니다.
	FString BuildError;
	if (!TestTrue(TEXT("Managed Resolver request builds"), CFVehicleResolverTestsPrivate::BuildManagedRequest(Request, BuildError)))
	{
		AddError(BuildError);
		return false;
	}

	// Adoption 전 measurement proposal-only resolve입니다.
	FCFVehicleResolveResult ProposalResult;
	TestTrue(TEXT("Proposal-only Resolve executes without internal error"), FCFVehicleResolver::Resolve(Request, ProposalResult));
	TestEqual(TEXT("Managed wheel geometry without adoption is Blocked"), ProposalResult.ResolveStatus, ECFVehicleResolveStatus::Blocked);
	TestEqual(TEXT("Four radius/width measurement proposals"), ProposalResult.MeasurementProposals.Num(), 4);
	TestEqual(TEXT("Frozen R0-R16 stage count"), ProposalResult.StageRecords.Num(), 17);
	for (int32 StageIndex = 0; StageIndex < ProposalResult.StageRecords.Num(); ++StageIndex)
	{
		TestEqual(FString::Printf(TEXT("Frozen stage ordinal %d"), StageIndex), static_cast<int32>(ProposalResult.StageRecords[StageIndex].Stage), StageIndex);
	}
		TestEqual(TEXT("R15 Materializer completes for proposal preview"), ProposalResult.StageRecords[15].Status, ECFVehicleResolverStageStatus::Completed);
	TestTrue(TEXT("Synthetic proposal exposes Definition validation"), ProposalResult.DefinitionValidation.Num() > 0);

	// Proposal fingerprints를 명시적 Recipe adoption 상태로 저장합니다.
	CFVehicleResolverTestsPrivate::AdoptAllWheelMeasurements(Request, ProposalResult);
	// Explicit adoption 뒤 Resolver 계산은 유효하지만 synthetic Definition 자산 누락은 Validator가 Block합니다.
	FCFVehicleResolveResult FirstResult;
	TestTrue(TEXT("Adopted Resolve executes"), FCFVehicleResolver::Resolve(Request, FirstResult));
	TestEqual(TEXT("Synthetic adopted Definition is validator-blocked, not Resolver-error"), FirstResult.ResolveStatus, ECFVehicleResolveStatus::Blocked);
	TestEqual(TEXT("R15 Materializer completed"), FirstResult.StageRecords[15].Status, ECFVehicleResolverStageStatus::Completed);
	TestTrue(TEXT("Definition validation is connected"), FirstResult.DefinitionValidation.Num() > 0);
	TestTrue(TEXT("Source signature populated"), !FirstResult.SourceSignature.IsEmpty());
	TestTrue(TEXT("Resolved definition hash populated"), !FirstResult.ResolvedDefinitionHash.IsEmpty());
	TestEqual(TEXT("Resolved fields and source traces stay index-aligned"), FirstResult.SortedResolvedFields.Num(), FirstResult.PreviewSourceTrace.Num());

	// 완전히 같은 immutable request를 다시 실행한 결과입니다.
	FCFVehicleResolveResult RepeatedResult;
	TestTrue(TEXT("Repeated Resolve executes"), FCFVehicleResolver::Resolve(Request, RepeatedResult));
	TestEqual(TEXT("Repeated synthetic Definition remains validator-blocked"), RepeatedResult.ResolveStatus, ECFVehicleResolveStatus::Blocked);
	TestEqual(TEXT("Source signature deterministic"), RepeatedResult.SourceSignature, FirstResult.SourceSignature);
	TestEqual(TEXT("Resolved definition hash deterministic"), RepeatedResult.ResolvedDefinitionHash, FirstResult.ResolvedDefinitionHash);

	// Preview-only Fitting context만 바꾼 request입니다.
	FCFVehicleResolveRequest PreviewRequest = Request;
	PreviewRequest.bHasPreviewContext = true;
	PreviewRequest.PreviewContext.bHasFittedTotalMassKg = true;
	PreviewRequest.PreviewContext.FittedTotalMassKg = 1999.0f;
	// Preview context가 포함된 Resolve입니다.
		FCFVehicleResolveResult PreviewResult;
	TestTrue(TEXT("Preview-context Resolve executes"), FCFVehicleResolver::Resolve(PreviewRequest, PreviewResult));
	TestEqual(TEXT("Preview context does not bypass Definition validation"), PreviewResult.ResolveStatus, ECFVehicleResolveStatus::Blocked);
	TestEqual(TEXT("Preview context cannot change Source signature"), PreviewResult.SourceSignature, FirstResult.SourceSignature);
	TestEqual(TEXT("Preview context cannot change Definition hash"), PreviewResult.ResolvedDefinitionHash, FirstResult.ResolvedDefinitionHash);
	return true;
}

// ESH-01 Performance Profile의 vehicle-specific Engine Torque Curve가 atomic source로 resolve되고 invalid payload가 fail-closed되는지 검증합니다.
bool FCFVehicleResolverEngineCurveTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// ESH-01 synthetic managed request입니다.
	FCFVehicleResolveRequest Request;
	FString BuildError;
	if (!TestTrue(TEXT("ESH-01 managed Resolver request builds"), CFVehicleResolverTestsPrivate::BuildManagedRequest(Request, BuildError)))
	{
		AddError(BuildError);
		return false;
	}
	TestEqual(TEXT("ESH-01 uses Resolver revision 5"), FCFVehicleResolver::CurrentResolverContractRevision, 5);

	// vehicle-specific Engine Curve opt-in complete payload입니다.
	Request.Profiles.PerformanceData.bUseEngineTorqueCurve = true;
	Request.Profiles.PerformanceData.EngineTorqueCurve.Points.Reset();
	FCFVehicleEngineTorquePoint Point0;
	Point0.EngineRPM = 0.0f;
	Point0.TorqueMultiplier = 0.25f;
	Request.Profiles.PerformanceData.EngineTorqueCurve.Points.Add(Point0);
	FCFVehicleEngineTorquePoint Point1;
	Point1.EngineRPM = 3000.0f;
	Point1.TorqueMultiplier = 1.0f;
	Request.Profiles.PerformanceData.EngineTorqueCurve.Points.Add(Point1);
	FCFVehicleEngineTorquePoint Point2;
	Point2.EngineRPM = 6500.0f;
	Point2.TorqueMultiplier = 0.35f;
	Request.Profiles.PerformanceData.EngineTorqueCurve.Points.Add(Point2);

	// Wheel adoption 미완료는 별도 blocker지만 Engine Curve R2 mapping은 끝까지 계산되어야 합니다.
	FCFVehicleResolveResult ValidResult;
	TestTrue(TEXT("ESH-01 valid Engine Curve Resolve executes"), FCFVehicleResolver::Resolve(Request, ValidResult));
	TestFalse(TEXT("Valid Engine Curve has no curve validation blocker"), CFVehicleResolverTestsPrivate::HasIssueCode(ValidResult.ResolverValidation, TEXT("PerformanceEngineTorqueCurveInvalid")));

	const FCFVehicleSourceTrace* CurveFlagTrace = CFVehicleResolverTestsPrivate::FindTrace(ValidResult, TEXT("VehicleMovementConfig.bUseEngineTorqueCurve"));
	if (TestNotNull(TEXT("bUseEngineTorqueCurve source trace exists"), CurveFlagTrace))
	{
		TestEqual(TEXT("bUseEngineTorqueCurve effective source is Performance Profile"), CurveFlagTrace->Layers[CurveFlagTrace->EffectiveLayerIndex].SourceType, ECFVehicleSourceType::PerformanceProfile);
	}

	const FCFVehicleSourceTrace* CurveTrace = CFVehicleResolverTestsPrivate::FindTrace(ValidResult, TEXT("VehicleMovementConfig.EngineTorqueCurve"));
	if (TestNotNull(TEXT("EngineTorqueCurve atomic source trace exists"), CurveTrace))
	{
		TestEqual(TEXT("EngineTorqueCurve effective source is Performance Profile"), CurveTrace->Layers[CurveTrace->EffectiveLayerIndex].SourceType, ECFVehicleSourceType::PerformanceProfile);
	}
	TestNotNull(TEXT("EngineTorqueCurve resolved field exists"), CFVehicleResolverTestsPrivate::FindResolvedField(ValidResult, TEXT("VehicleMovementConfig.EngineTorqueCurve")));
	TestFalse(TEXT("Atomic curve does not emit Points child trace"), CFVehicleResolverTestsPrivate::FindTrace(ValidResult, TEXT("VehicleMovementConfig.EngineTorqueCurve.Points")) != nullptr);

	// 동일 RPM duplicate는 공통 Engine Curve validator가 fail-closed해야 합니다.
	FCFVehicleResolveRequest InvalidRequest = Request;
	InvalidRequest.Profiles.PerformanceData.EngineTorqueCurve.Points[2].EngineRPM = 3000.0f;
	FCFVehicleResolveResult InvalidResult;
	TestTrue(TEXT("ESH-01 invalid Engine Curve Resolve still returns diagnostic result"), FCFVehicleResolver::Resolve(InvalidRequest, InvalidResult));
	TestTrue(TEXT("Invalid Engine Curve emits fail-closed blocker"), CFVehicleResolverTestsPrivate::HasIssueCode(InvalidResult.ResolverValidation, TEXT("PerformanceEngineTorqueCurveInvalid")));

	return true;
}

// Recipe SoftObject ChassisMesh가 Target hard Object canonical reference로 정규화되어 R15 readback hash와 일치하는지 검증합니다.
bool FCFVehicleResolverObjectReferenceRoundTripTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// SoftObject→Object canonical roundtrip을 검증할 baseline managed Resolver request입니다.
	FCFVehicleResolveRequest Request;
	// Request 구성 실패 사유입니다.
	FString BuildError;
	if (!TestTrue(TEXT("Object reference roundtrip request builds"), CFVehicleResolverTestsPrivate::BuildManagedRequest(Request, BuildError)))
	{
		AddError(BuildError);
		return false;
	}

	// Engine 기본 StaticMesh를 가리키는 immutable soft object path입니다.
	const FSoftObjectPath CubeMeshPath(TEXT("/Engine/BasicShapes/Cube.Cube"));
	// Recipe source는 SoftObject reference를 보유합니다.
	Request.Recipe.AssetIntent.ChassisMesh = TSoftObjectPtr<UStaticMesh>(CubeMeshPath);
	// Asset Snapshot도 같은 chassis identity를 가리켜 source/readback 의미를 맞춥니다.
	Request.Assets.ChassisObjectPath = CubeMeshPath;
	Request.Assets.bChassisLoaded = true;

	// SoftObject→Object adaptation이 적용된 Resolve 결과입니다.
	FCFVehicleResolveResult Result;
	if (!TestTrue(TEXT("SoftObject to Object Resolve avoids internal error"), FCFVehicleResolver::Resolve(Request, Result)))
	{
		for (const FCFVehicleValidationIssue& Issue : Result.DefinitionValidation)
		{
			AddError(FString::Printf(TEXT("Definition issue %s: %s"), *Issue.IssueCode.ToString(), *Issue.Message));
		}
		return false;
	}

	// ChassisMesh exact resolved field입니다.
	const FCFVehicleResolvedField* ChassisField = CFVehicleResolverTestsPrivate::FindResolvedField(Result, TEXT("VehicleVisualConfig.ChassisMesh"));
	if (TestNotNull(TEXT("Resolved ChassisMesh field exists"), ChassisField))
	{
		TestEqual(TEXT("ChassisMesh target type is hard StaticMesh Object"), ChassisField->Value.PropertyTypeSignature, FString(TEXT("Object:/Script/Engine.StaticMesh")));
		TestEqual(TEXT("ChassisMesh canonical text matches target Object export form"), ChassisField->Value.CanonicalValueText, FString(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cube.Cube'")));
	}

	TestFalse(TEXT("R15 readback hash mismatch is absent"), CFVehicleResolverTestsPrivate::HasIssueCode(Result.DefinitionValidation, TEXT("DefinitionHashReadbackMismatch")));
	TestEqual(TEXT("R15 materializer completes after object canonicalization"), Result.StageRecords[15].Status, ECFVehicleResolverStageStatus::Completed);
	return true;
}

// Project/Profile/Rule/Recipe/Legacy/Advanced precedence와 R11 Derived Gate가 Legacy Pin을 역전하지 않는지 검증합니다.
bool FCFVehicleResolverPrecedenceTest::RunTest(const FString& Parameters)
{
	// Baseline managed Resolver request입니다.
	FCFVehicleResolveRequest Request;
	// Request builder 실패 사유입니다.
	FString BuildError;
	if (!TestTrue(TEXT("Precedence request builds"), CFVehicleResolverTestsPrivate::BuildManagedRequest(Request, BuildError)))
	{
		AddError(BuildError);
		return false;
	}

	// 먼저 proposal fingerprint를 얻어 wheel geometry blocker를 제거합니다.
	FCFVehicleResolveResult ProposalResult;
	FCFVehicleResolver::Resolve(Request, ProposalResult);
	CFVehicleResolverTestsPrivate::AdoptAllWheelMeasurements(Request, ProposalResult);

	// EngineMaxTorque의 Legacy Pin lower-than-Advanced candidate입니다.
	Request.Recipe.ImportState.LegacyPinnedFields.Add(CFVehicleResolverTestsPrivate::MakeFloatOverride(TEXT("VehicleMovementConfig.EngineMaxTorque"), 1111.0f));
	// R11보다 높은 Legacy Pin이 movement derived gate를 false로 고정하는 candidate입니다.
	Request.Recipe.ImportState.LegacyPinnedFields.Add(CFVehicleResolverTestsPrivate::MakeBoolOverride(TEXT("VehicleMovementConfig.bUseMovementOverrides"), false));
	// EngineMaxTorque의 최상위 Advanced Override입니다.
	Request.Recipe.AdvancedOverrides.Add(CFVehicleResolverTestsPrivate::MakeFloatOverride(TEXT("VehicleMovementConfig.EngineMaxTorque"), 2222.0f));

	// Full precedence stack Resolve입니다.
	FCFVehicleResolveResult Result;
		TestTrue(TEXT("Precedence Resolve executes"), FCFVehicleResolver::Resolve(Request, Result));
	TestEqual(TEXT("Precedence calculation survives Definition validation block"), Result.ResolveStatus, ECFVehicleResolveStatus::Blocked);

	// EngineMaxTorque source trace입니다.
	const FCFVehicleSourceTrace* TorqueTrace = CFVehicleResolverTestsPrivate::FindTrace(Result, TEXT("VehicleMovementConfig.EngineMaxTorque"));
	if (TestNotNull(TEXT("EngineMaxTorque trace exists"), TorqueTrace))
	{
		TestTrue(TEXT("Torque trace has layered sources"), TorqueTrace->Layers.Num() >= 5);
		TestEqual(TEXT("Advanced Override is effective torque source"), TorqueTrace->Layers[TorqueTrace->EffectiveLayerIndex].SourceType, ECFVehicleSourceType::AdvancedLeafOverride);
	}

	// Advanced value 2222가 최종 effective value인지 확인합니다.
	const FCFVehicleResolvedField* TorqueField = CFVehicleResolverTestsPrivate::FindResolvedField(Result, TEXT("VehicleMovementConfig.EngineMaxTorque"));
	if (TestNotNull(TEXT("EngineMaxTorque resolved field exists"), TorqueField))
	{
		TestEqual(TEXT("Advanced torque value wins"), FCString::Atof(*TorqueField->Value.CanonicalValueText), 2222.0f);
	}

	// R11 Derived Gate가 뒤에서 실행돼도 Legacy Pin false를 역전하면 안 됩니다.
	const FCFVehicleSourceTrace* MovementGateTrace = CFVehicleResolverTestsPrivate::FindTrace(Result, TEXT("VehicleMovementConfig.bUseMovementOverrides"));
	if (TestNotNull(TEXT("Movement gate trace exists"), MovementGateTrace))
	{
		TestEqual(TEXT("Legacy Pin remains effective over later Derived Gate stage"), MovementGateTrace->Layers[MovementGateTrace->EffectiveLayerIndex].SourceType, ECFVehicleSourceType::LegacyImportedPinnedBaseline);
	}
	const FCFVehicleResolvedField* MovementGateField = CFVehicleResolverTestsPrivate::FindResolvedField(Result, TEXT("VehicleMovementConfig.bUseMovementOverrides"));
	if (TestNotNull(TEXT("Movement gate resolved field exists"), MovementGateField))
	{
		TestTrue(TEXT("Legacy false gate value preserved"), MovementGateField->Value.CanonicalValueText.Equals(TEXT("False"), ESearchCase::IgnoreCase) || MovementGateField->Value.CanonicalValueText == TEXT("0"));
	}
	return true;
}

// Accepted measurement fingerprint가 wheel asset fact 변경 뒤 mismatch로 Blocked 되는지 검증합니다.
bool FCFVehicleResolverFingerprintTest::RunTest(const FString& Parameters)
{
	// Baseline managed Resolver request입니다.
	FCFVehicleResolveRequest Request;
	// Request builder 실패 사유입니다.
	FString BuildError;
	if (!TestTrue(TEXT("Fingerprint request builds"), CFVehicleResolverTestsPrivate::BuildManagedRequest(Request, BuildError)))
	{
		AddError(BuildError);
		return false;
	}

	// 최초 proposal fingerprints입니다.
	FCFVehicleResolveResult ProposalResult;
	FCFVehicleResolver::Resolve(Request, ProposalResult);
	CFVehicleResolverTestsPrivate::AdoptAllWheelMeasurements(Request, ProposalResult);

	// Adoption이 현재 asset facts와 일치하는 baseline 결과입니다.
	FCFVehicleResolveResult AdoptedResult;
		TestTrue(TEXT("Baseline adopted Resolve executes"), FCFVehicleResolver::Resolve(Request, AdoptedResult));
	TestEqual(TEXT("Baseline synthetic Definition is validator-blocked"), AdoptedResult.ResolveStatus, ECFVehicleResolveStatus::Blocked);

	// Front-left wheel resolver-relevant fingerprint만 변경합니다.
	Request.Assets.WheelFL.MeasureFingerprint = TEXT("wheel-fl-v2");
	// Stale accepted fingerprint를 가진 Resolve입니다.
	FCFVehicleResolveResult ChangedAssetResult;
	TestTrue(TEXT("Changed-asset Resolve still executes"), FCFVehicleResolver::Resolve(Request, ChangedAssetResult));
	TestEqual(TEXT("Changed accepted asset fingerprint blocks Resolve"), ChangedAssetResult.ResolveStatus, ECFVehicleResolveStatus::Blocked);
	TestTrue(TEXT("Fingerprint mismatch issue emitted"), CFVehicleResolverTestsPrivate::HasIssueCode(ChangedAssetResult.ResolverValidation, TEXT("AcceptedMeasurementFingerprintMismatch")));
	TestEqual(TEXT("Measurement proposals remain preview-visible while blocked"), ChangedAssetResult.MeasurementProposals.Num(), 4);
	return true;
}

// WSA-P0-02 USER Socket Scale에서 Radius/Width와 narrow fingerprint를 derive하고 invalid input을 fail-closed하는지 검증합니다.
bool FCFVehicleResolverSocketScaleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	FCFVehicleResolveRequest Request;
	FString BuildError;
	if (!TestTrue(TEXT("WSA managed request builds"), CFVehicleResolverTestsPrivate::BuildManagedRequest(Request, BuildError)))
	{
		AddError(BuildError);
		return false;
	}

	TestTrue(TEXT("WSA-P0-02 Resolver contract revision remains represented"), FCFVehicleResolver::CurrentResolverContractRevision >= 4);
	Request.Recipe.WheelVisualIntent.Mode = ECFWheelVisualIntentMode::SocketScaleFromChassis;
	// Socket mode는 Legacy radius measure mode를 무시해야 하므로 의도적으로 AxisY를 둡니다.
	Request.Profiles.BaseData.WheelMeshRadiusMeasureMode = ECFWheelMeshRadiusMeasureMode::AxisY;
	Request.Recipe.AssetAdoption.bUseSuggestedRadiusMeasureMode = true;

	FCFVehicleWheelAssetSnapshot* Wheels[] = {&Request.Assets.WheelFL, &Request.Assets.WheelFR, &Request.Assets.WheelRL, &Request.Assets.WheelRR};
	for (int32 WheelIndex = 0; WheelIndex < UE_ARRAY_COUNT(Wheels); ++WheelIndex)
	{
		Wheels[WheelIndex]->BoundsOrigin = FVector::ZeroVector;
		Wheels[WheelIndex]->BoundsExtent = FVector(50.0, 12.5, 50.0);
		Wheels[WheelIndex]->MeasureFingerprint = FString::Printf(TEXT("wsa-wheel-%d-v1"), WheelIndex);
	}

	FCFVehicleSocketSnapshot* SocketFL = CFVehicleResolverTestsPrivate::FindSocket(Request.Assets, TEXT("Wheel_Anchor_FL"));
	FCFVehicleSocketSnapshot* SocketFR = CFVehicleResolverTestsPrivate::FindSocket(Request.Assets, TEXT("Wheel_Anchor_FR"));
	FCFVehicleSocketSnapshot* SocketRL = CFVehicleResolverTestsPrivate::FindSocket(Request.Assets, TEXT("Wheel_Anchor_RL"));
	FCFVehicleSocketSnapshot* SocketRR = CFVehicleResolverTestsPrivate::FindSocket(Request.Assets, TEXT("Wheel_Anchor_RR"));
	if (!TestNotNull(TEXT("FL socket exists"), SocketFL)
		|| !TestNotNull(TEXT("FR socket exists"), SocketFR)
		|| !TestNotNull(TEXT("RL socket exists"), SocketRL)
		|| !TestNotNull(TEXT("RR socket exists"), SocketRR))
	{
		return false;
	}

	SocketFL->RelativeScale = FVector(0.72, 1.12, 0.72);
	SocketFR->RelativeScale = FVector(0.72, 1.12, 0.72);
	SocketRL->RelativeScale = FVector(0.80, 1.00, 0.80);
	SocketRR->RelativeScale = FVector(0.80, 1.00, 0.80);
	Request.Assets.ChassisLayoutFingerprint = TEXT("wsa-layout-v1");

	FCFVehicleResolveResult ProposalResult;
	TestTrue(TEXT("Socket Scale proposal Resolve executes"), FCFVehicleResolver::Resolve(Request, ProposalResult));
	TestEqual(TEXT("Socket Scale creates four wheel geometry proposals"), ProposalResult.MeasurementProposals.Num(), 4);
	TestFalse(TEXT("Legacy radius-mode suggestion blocker is ignored in Socket mode"), CFVehicleResolverTestsPrivate::HasIssueCode(ProposalResult.ResolverValidation, TEXT("RadiusModeSuggestionNotFrozen")));

	const FCFVehicleMeasurementProposal* FrontRadius = CFVehicleResolverTestsPrivate::FindProposal(ProposalResult, TEXT("VehicleMovementConfig.FrontWheelRadius"));
	const FCFVehicleMeasurementProposal* FrontWidth = CFVehicleResolverTestsPrivate::FindProposal(ProposalResult, TEXT("VehicleMovementConfig.FrontWheelWidth"));
	const FCFVehicleMeasurementProposal* RearRadius = CFVehicleResolverTestsPrivate::FindProposal(ProposalResult, TEXT("VehicleMovementConfig.RearWheelRadius"));
	const FCFVehicleMeasurementProposal* RearWidth = CFVehicleResolverTestsPrivate::FindProposal(ProposalResult, TEXT("VehicleMovementConfig.RearWheelWidth"));
	if (!TestNotNull(TEXT("Front radius proposal exists"), FrontRadius)
		|| !TestNotNull(TEXT("Front width proposal exists"), FrontWidth)
		|| !TestNotNull(TEXT("Rear radius proposal exists"), RearRadius)
		|| !TestNotNull(TEXT("Rear width proposal exists"), RearWidth))
	{
		return false;
	}

	TestTrue(TEXT("Front radius derived 36cm"), FMath::IsNearlyEqual(FCString::Atof(*FrontRadius->MeasuredCandidateValue.CanonicalValueText), 36.0f, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("Front width derived 28cm"), FMath::IsNearlyEqual(FCString::Atof(*FrontWidth->MeasuredCandidateValue.CanonicalValueText), 28.0f, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("Rear radius derived 40cm"), FMath::IsNearlyEqual(FCString::Atof(*RearRadius->MeasuredCandidateValue.CanonicalValueText), 40.0f, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("Rear width derived 25cm"), FMath::IsNearlyEqual(FCString::Atof(*RearWidth->MeasuredCandidateValue.CanonicalValueText), 25.0f, KINDA_SMALL_NUMBER));
	TestEqual(TEXT("Socket radius rule id"), FrontRadius->MeasurementRuleId, FName(TEXT("WheelSocketScale.Radius.v1")));
	TestEqual(TEXT("Socket width rule id"), FrontWidth->MeasurementRuleId, FName(TEXT("WheelSocketScale.Width.v1")));

	// 공용 Wheel Mesh를 FL 하나만 지정한 신규 정상 경로는 FR/RL/RR 모두 FL Snapshot을 deterministic fallback으로 재사용해야 합니다.
	FCFVehicleResolveRequest SharedWheelFallbackRequest = Request;
	SharedWheelFallbackRequest.Assets.WheelFR = FCFVehicleWheelAssetSnapshot();
	SharedWheelFallbackRequest.Assets.WheelRL = FCFVehicleWheelAssetSnapshot();
	SharedWheelFallbackRequest.Assets.WheelRR = FCFVehicleWheelAssetSnapshot();
	FCFVehicleResolveResult SharedWheelFallbackResult;
	TestTrue(TEXT("Socket Scale FL-only shared Wheel Resolve executes"), FCFVehicleResolver::Resolve(SharedWheelFallbackRequest, SharedWheelFallbackResult));
	TestFalse(TEXT("FL-only shared Wheel does not emit missing Wheel blocker"), CFVehicleResolverTestsPrivate::HasIssueCode(SharedWheelFallbackResult.ResolverValidation, TEXT("WheelSocketSizeWheelMissing")));
	TestEqual(TEXT("FL-only shared Wheel still creates four geometry proposals"), SharedWheelFallbackResult.MeasurementProposals.Num(), 4);
	const FCFVehicleMeasurementProposal* SharedRearRadius = CFVehicleResolverTestsPrivate::FindProposal(SharedWheelFallbackResult, TEXT("VehicleMovementConfig.RearWheelRadius"));
	const FCFVehicleMeasurementProposal* SharedRearWidth = CFVehicleResolverTestsPrivate::FindProposal(SharedWheelFallbackResult, TEXT("VehicleMovementConfig.RearWheelWidth"));
	if (TestNotNull(TEXT("FL-only shared Wheel rear radius proposal exists"), SharedRearRadius)
		&& TestNotNull(TEXT("FL-only shared Wheel rear width proposal exists"), SharedRearWidth))
	{
		TestTrue(TEXT("FL-only shared Wheel rear radius uses rear Socket Scale"), FMath::IsNearlyEqual(FCString::Atof(*SharedRearRadius->MeasuredCandidateValue.CanonicalValueText), 40.0f, KINDA_SMALL_NUMBER));
		TestTrue(TEXT("FL-only shared Wheel rear width uses rear Socket Scale"), FMath::IsNearlyEqual(FCString::Atof(*SharedRearWidth->MeasuredCandidateValue.CanonicalValueText), 25.0f, KINDA_SMALL_NUMBER));
	}

	const FCFVehicleResolvedField* SocketModeField = CFVehicleResolverTestsPrivate::FindResolvedField(ProposalResult, TEXT("WheelVisualConfig.bUseWheelSocketScale"));
	const FCFVehicleResolvedField* LegacyAutoScaleField = CFVehicleResolverTestsPrivate::FindResolvedField(ProposalResult, TEXT("WheelVisualConfig.bAutoScaleWheelMeshToRadius"));
	if (TestNotNull(TEXT("Socket mode field exists"), SocketModeField))
	{
		TestTrue(TEXT("Socket mode resolves true"), SocketModeField->Value.CanonicalValueText.Equals(TEXT("True"), ESearchCase::IgnoreCase) || SocketModeField->Value.CanonicalValueText == TEXT("1"));
	}
	if (TestNotNull(TEXT("Legacy auto scale field exists"), LegacyAutoScaleField))
	{
		TestTrue(TEXT("Socket mode forces legacy auto scale false"), LegacyAutoScaleField->Value.CanonicalValueText.Equals(TEXT("False"), ESearchCase::IgnoreCase) || LegacyAutoScaleField->Value.CanonicalValueText == TEXT("0"));
	}

	const FCFVehicleSourceTrace* ScaleTrace = CFVehicleResolverTestsPrivate::FindTrace(ProposalResult, TEXT("VehicleLayoutConfig.WheelAnchorFL.RelativeScale"));
	if (TestNotNull(TEXT("FL scale trace exists"), ScaleTrace))
	{
		TestEqual(TEXT("FL scale effective source is AssetDerived"), ScaleTrace->Layers[ScaleTrace->EffectiveLayerIndex].SourceType, ECFVehicleSourceType::AssetDerived);
	}

	const FString BaselineFrontFingerprint = FrontRadius->AssetFingerprint;
	FCFVehicleResolveRequest LocationChangedRequest = Request;
	FCFVehicleSocketSnapshot* LocationChangedFL = CFVehicleResolverTestsPrivate::FindSocket(LocationChangedRequest.Assets, TEXT("Wheel_Anchor_FL"));
	LocationChangedFL->RelativeLocation += FVector(5.0, 0.0, 0.0);
	LocationChangedRequest.Assets.ChassisLayoutFingerprint = TEXT("wsa-layout-location-changed");
	FCFVehicleResolveResult LocationChangedResult;
	TestTrue(TEXT("Location-changed Socket Resolve executes"), FCFVehicleResolver::Resolve(LocationChangedRequest, LocationChangedResult));
	const FCFVehicleMeasurementProposal* LocationChangedFrontRadius = CFVehicleResolverTestsPrivate::FindProposal(LocationChangedResult, TEXT("VehicleMovementConfig.FrontWheelRadius"));
	if (TestNotNull(TEXT("Location-changed front radius proposal exists"), LocationChangedFrontRadius))
	{
		TestEqual(TEXT("Wheel Size fingerprint ignores location-only layout change"), LocationChangedFrontRadius->AssetFingerprint, BaselineFrontFingerprint);
	}

	FCFVehicleResolveRequest ScaleChangedRequest = Request;
	CFVehicleResolverTestsPrivate::FindSocket(ScaleChangedRequest.Assets, TEXT("Wheel_Anchor_FL"))->RelativeScale = FVector(0.74, 1.12, 0.74);
	CFVehicleResolverTestsPrivate::FindSocket(ScaleChangedRequest.Assets, TEXT("Wheel_Anchor_FR"))->RelativeScale = FVector(0.74, 1.12, 0.74);
	ScaleChangedRequest.Assets.ChassisLayoutFingerprint = TEXT("wsa-layout-scale-changed");
	FCFVehicleResolveResult ScaleChangedResult;
	TestTrue(TEXT("Scale-changed Socket Resolve executes"), FCFVehicleResolver::Resolve(ScaleChangedRequest, ScaleChangedResult));
	const FCFVehicleMeasurementProposal* ScaleChangedFrontRadius = CFVehicleResolverTestsPrivate::FindProposal(ScaleChangedResult, TEXT("VehicleMovementConfig.FrontWheelRadius"));
	if (TestNotNull(TEXT("Scale-changed front radius proposal exists"), ScaleChangedFrontRadius))
	{
		TestNotEqual(TEXT("Wheel Size fingerprint changes with authored scale"), ScaleChangedFrontRadius->AssetFingerprint, BaselineFrontFingerprint);
		TestTrue(TEXT("Scale-changed front radius is 37cm"), FMath::IsNearlyEqual(FCString::Atof(*ScaleChangedFrontRadius->MeasuredCandidateValue.CanonicalValueText), 37.0f, KINDA_SMALL_NUMBER));
	}

	FCFVehicleResolveRequest InvalidScaleRequest = Request;
	CFVehicleResolverTestsPrivate::FindSocket(InvalidScaleRequest.Assets, TEXT("Wheel_Anchor_FL"))->RelativeScale = FVector(0.72, 1.12, 0.70);
	FCFVehicleResolveResult InvalidScaleResult;
	TestTrue(TEXT("Invalid-scale Resolve executes fail-closed"), FCFVehicleResolver::Resolve(InvalidScaleRequest, InvalidScaleResult));
	TestTrue(TEXT("X/Z mismatch emits WheelSocketScaleInvalid"), CFVehicleResolverTestsPrivate::HasIssueCode(InvalidScaleResult.ResolverValidation, TEXT("WheelSocketScaleInvalid")));

	FCFVehicleResolveRequest AxleMismatchRequest = Request;
	CFVehicleResolverTestsPrivate::FindSocket(AxleMismatchRequest.Assets, TEXT("Wheel_Anchor_FR"))->RelativeScale = FVector(0.72, 1.20, 0.72);
	FCFVehicleResolveResult AxleMismatchResult;
	TestTrue(TEXT("Axle-mismatch Resolve executes fail-closed"), FCFVehicleResolver::Resolve(AxleMismatchRequest, AxleMismatchResult));
	TestTrue(TEXT("Axle mismatch emits blocker"), CFVehicleResolverTestsPrivate::HasIssueCode(AxleMismatchResult.ResolverValidation, TEXT("WheelSocketAxleSizeMismatch")));

	FCFVehicleResolveRequest AdoptedRequest = Request;
	CFVehicleResolverTestsPrivate::AdoptAllWheelMeasurements(AdoptedRequest, ProposalResult);
	FCFVehicleResolveResult AdoptedResult;
	TestTrue(TEXT("Adopted Socket Scale Resolve executes"), FCFVehicleResolver::Resolve(AdoptedRequest, AdoptedResult));
	const FCFVehicleSourceTrace* FrontRadiusTrace = CFVehicleResolverTestsPrivate::FindTrace(AdoptedResult, TEXT("VehicleMovementConfig.FrontWheelRadius"));
	if (TestNotNull(TEXT("Adopted front radius trace exists"), FrontRadiusTrace))
	{
		const FCFVehicleSourceLayer& EffectiveLayer = FrontRadiusTrace->Layers[FrontRadiusTrace->EffectiveLayerIndex];
		TestEqual(TEXT("Adopted radius source type AssetDerived"), EffectiveLayer.SourceType, ECFVehicleSourceType::AssetDerived);
		TestEqual(TEXT("Adopted radius source id"), EffectiveLayer.SourceId, FString(TEXT("Measurement.WheelSocketScale.Radius.v1")));
	}

	return true;
}

// Hidden Mount serialized field가 일반 Recipe source가 아니라 LegacySerializedPassthrough로만 유지되는지 검증합니다.
bool FCFVehicleResolverLegacyTest::RunTest(const FString& Parameters)
{
	// Baseline Resolver request입니다.
	FCFVehicleResolveRequest Request;
	// Request builder 실패 사유입니다.
	FString BuildError;
	if (!TestTrue(TEXT("Legacy request builds"), CFVehicleResolverTestsPrivate::BuildManagedRequest(Request, BuildError)))
	{
		AddError(BuildError);
		return false;
	}

	// Wheel proposal adoption으로 managed geometry blocker를 제거합니다.
	FCFVehicleResolveResult ProposalResult;
	FCFVehicleResolver::Resolve(Request, ProposalResult);
	CFVehicleResolverTestsPrivate::AdoptAllWheelMeasurements(Request, ProposalResult);

	// Active Mount semantic row입니다.
	FCFMountIntent& MountIntent = Request.Recipe.MountIntents.AddDefaulted_GetRef();
	MountIntent.MountProfileId = TEXT("M_Test");
	MountIntent.LocationSlotRef = NAME_None;

	// New authoring이 생성하지 않는 hidden legacy TurretYawMesh serialized value입니다.
	FCFVehicleFieldOverride LegacyYawMesh = CFVehicleResolverTestsPrivate::MakeDefaultBackedArrayOverride(Request.ProjectDefaults, TEXT("MountProfiles[MountProfileId=*].TurretYawMesh"), TEXT("M_Test"));
	Request.Recipe.ImportState.LegacySerializedFields.Add(LegacyYawMesh);

	// Legacy serialized passthrough Resolve입니다.
	FCFVehicleResolveResult Result;
		TestTrue(TEXT("Legacy serialized Resolve executes"), FCFVehicleResolver::Resolve(Request, Result));
	TestEqual(TEXT("Legacy serialized projection materializes before synthetic Definition block"), Result.ResolveStatus, ECFVehicleResolveStatus::Blocked);
	TestEqual(TEXT("Legacy serialized R15 completed"), Result.StageRecords[15].Status, ECFVehicleResolverStageStatus::Completed);

	// Hidden serialized exact field trace입니다.
	const FCFVehicleSourceTrace* LegacyTrace = CFVehicleResolverTestsPrivate::FindTrace(Result, TEXT("MountProfiles[MountProfileId=M_Test].TurretYawMesh"));
	if (TestNotNull(TEXT("Legacy serialized trace exists"), LegacyTrace))
	{
		TestEqual(TEXT("Hidden field has one passthrough layer"), LegacyTrace->Layers.Num(), 1);
		TestEqual(TEXT("Hidden field source is LegacySerializedPassthrough"), LegacyTrace->Layers[LegacyTrace->EffectiveLayerIndex].SourceType, ECFVehicleSourceType::LegacySerializedPassthrough);
	}
		return true;
}

// Active Standard Mount hidden legacy leaf가 신규 row에서는 struct default, existing row에서는 current serialized value를 deterministic passthrough하는지 검증합니다.
bool FCFVehicleResolverMountLegacyFallbackTest::RunTest(const FString& Parameters)
{
	FCFVehicleResolveRequest Request;
	FString BuildError;
	if (!TestTrue(TEXT("Mount legacy fallback request builds"), CFVehicleResolverTestsPrivate::BuildManagedRequest(Request, BuildError)))
	{
		AddError(BuildError);
		return false;
	}

	FCFVehicleResolveResult ProposalResult;
	FCFVehicleResolver::Resolve(Request, ProposalResult);
	CFVehicleResolverTestsPrivate::AdoptAllWheelMeasurements(Request, ProposalResult);

	CFVehicleResolverTestsPrivate::AddSocket(Request.Assets, TEXT("HP_Top_01"), FVector(-50.0, 0.0, 130.0));
	Request.Assets.ChassisSockets.Sort([](const FCFVehicleSocketSnapshot& Left, const FCFVehicleSocketSnapshot& Right)
	{
		return Left.SocketName.LexicalLess(Right.SocketName);
	});

	FCFHardpointIntent& HardpointIntent = Request.Recipe.HardpointIntents.AddDefaulted_GetRef();
	HardpointIntent.LocationSlotId = TEXT("Top_01");
	HardpointIntent.LocationCategory = TEXT("Top");
	HardpointIntent.SocketName = TEXT("HP_Top_01");

	FCFMountIntent& MountIntent = Request.Recipe.MountIntents.AddDefaulted_GetRef();
	MountIntent.MountProfileId = TEXT("Mount_Top_01");
	MountIntent.LocationSlotRef = TEXT("Top_01");
	MountIntent.MountType = ECFVehicleMountType::Turret;
	MountIntent.SizeLimit = ECFVehicleWeaponSize::Large;

	const TCHAR* LegacyPattern = TEXT("MountProfiles[MountProfileId=*].YawTurnRateDegPerSec");
	const TCHAR* ExactLegacyPath = TEXT("MountProfiles[MountProfileId=Mount_Top_01].YawTurnRateDegPerSec");
	const FCFVehicleFieldEntry* DefaultEntry = Request.ProjectDefaults.SortedFields.FindByPredicate([](const FCFVehicleFieldEntry& Entry)
	{
		return Entry.FieldPath.ToCanonicalString(true) == TEXT("MountProfiles[MountProfileId=*].YawTurnRateDegPerSec");
	});
	if (!TestNotNull(TEXT("Mount legacy wildcard project default exists"), DefaultEntry))
	{
		return false;
	}

	FCFVehicleResolveResult NewRowResult;
	TestTrue(TEXT("New Standard Mount resolve executes"), FCFVehicleResolver::Resolve(Request, NewRowResult));
	const FCFVehicleResolvedField* NewRowLegacyField = CFVehicleResolverTestsPrivate::FindResolvedField(NewRowResult, ExactLegacyPath);
	const FCFVehicleSourceTrace* NewRowLegacyTrace = CFVehicleResolverTestsPrivate::FindTrace(NewRowResult, ExactLegacyPath);
	if (TestNotNull(TEXT("New Standard Mount resolves hidden legacy leaf"), NewRowLegacyField)
		&& TestNotNull(TEXT("New Standard Mount hidden legacy trace exists"), NewRowLegacyTrace))
	{
		TestEqual(TEXT("New Standard Mount hidden legacy value uses struct default"), NewRowLegacyField->Value.CanonicalValueText, DefaultEntry->Value.CanonicalValueText);
		const FCFVehicleSourceLayer& EffectiveLayer = NewRowLegacyTrace->Layers[NewRowLegacyTrace->EffectiveLayerIndex];
		TestEqual(TEXT("New Standard Mount hidden legacy source type"), EffectiveLayer.SourceType, ECFVehicleSourceType::LegacySerializedPassthrough);
		TestEqual(TEXT("New Standard Mount hidden legacy source id"), EffectiveLayer.SourceId, FString(TEXT("Project.CppDefaultStableMountLegacy")));
	}

	Request.bHasCurrentDefinition = true;
	Request.CurrentDefinition.SortedFields.Reset();
	Request.CurrentDefinition.SortedFields.Add(CFVehicleResolverTestsPrivate::MakeMountFloatCurrentEntry(LegacyPattern, TEXT("Mount_Top_01"), 123.0f));
	FString CurrentHashError;
	TestTrue(TEXT("Current Definition hash builds for mount legacy passthrough"), FCFVehicleSnapshotBuilder::BuildDefinitionHashFromFields(Request.CurrentDefinition.SortedFields, Request.CurrentDefinition.DefinitionHash, CurrentHashError));

	FCFVehicleResolveResult ExistingRowResult;
	TestTrue(TEXT("Existing Standard Mount resolve executes"), FCFVehicleResolver::Resolve(Request, ExistingRowResult));
	const FCFVehicleResolvedField* ExistingLegacyField = CFVehicleResolverTestsPrivate::FindResolvedField(ExistingRowResult, ExactLegacyPath);
	const FCFVehicleSourceTrace* ExistingLegacyTrace = CFVehicleResolverTestsPrivate::FindTrace(ExistingRowResult, ExactLegacyPath);
	if (TestNotNull(TEXT("Existing Standard Mount resolves hidden legacy leaf"), ExistingLegacyField)
		&& TestNotNull(TEXT("Existing Standard Mount hidden legacy trace exists"), ExistingLegacyTrace))
	{
		const FCFVehicleFieldEntry& CurrentEntry = Request.CurrentDefinition.SortedFields[0];
		TestEqual(TEXT("Existing Standard Mount preserves current serialized legacy value"), ExistingLegacyField->Value.CanonicalValueText, CurrentEntry.Value.CanonicalValueText);
		const FCFVehicleSourceLayer& EffectiveLayer = ExistingLegacyTrace->Layers[ExistingLegacyTrace->EffectiveLayerIndex];
		TestEqual(TEXT("Existing Standard Mount hidden legacy source id"), EffectiveLayer.SourceId, FString(TEXT("CurrentDefinition.LegacySerialized")));
	}

	return true;
}

// R14 Current Definition diff와 R16 effective source/external drift를 UObject materialization 없이 검증합니다.
bool FCFVehicleResolverDiffStaleTest::RunTest(const FString& Parameters)
{
	// Baseline managed Resolver request입니다.
	FCFVehicleResolveRequest Request;
	// Request builder 실패 사유입니다.
	FString BuildError;
	if (!TestTrue(TEXT("Diff/Stale request builds"), CFVehicleResolverTestsPrivate::BuildManagedRequest(Request, BuildError)))
	{
		AddError(BuildError);
		return false;
	}

	// Wheel proposal adoption으로 successful baseline을 만듭니다.
	FCFVehicleResolveResult ProposalResult;
	FCFVehicleResolver::Resolve(Request, ProposalResult);
	CFVehicleResolverTestsPrivate::AdoptAllWheelMeasurements(Request, ProposalResult);
	// Last Apply를 모사할 baseline Resolve result입니다.
	FCFVehicleResolveResult BaselineResult;
		TestTrue(TEXT("Diff/Stale baseline Resolve executes"), FCFVehicleResolver::Resolve(Request, BaselineResult));
	TestEqual(TEXT("Diff/Stale synthetic baseline is validator-blocked"), BaselineResult.ResolveStatus, ECFVehicleResolveStatus::Blocked);

	// Applied baseline으로 기록할 EngineMaxTorque source trace입니다.
	const FCFVehicleSourceTrace* BaselineTorqueTrace = CFVehicleResolverTestsPrivate::FindTrace(BaselineResult, TEXT("VehicleMovementConfig.EngineMaxTorque"));
	// Applied baseline으로 기록할 EngineMaxTorque resolved field입니다.
	const FCFVehicleResolvedField* BaselineTorqueField = CFVehicleResolverTestsPrivate::FindResolvedField(BaselineResult, TEXT("VehicleMovementConfig.EngineMaxTorque"));
	if (!TestNotNull(TEXT("Baseline torque trace exists"), BaselineTorqueTrace)
		|| !TestNotNull(TEXT("Baseline torque field exists"), BaselineTorqueField))
	{
		return false;
	}

	// Baseline resolved fields를 Current Definition value-copy snapshot으로 사용합니다.
	Request.bHasCurrentDefinition = true;
	Request.CurrentDefinition.SortedFields.Reset();
	for (const FCFVehicleResolvedField& ResolvedField : BaselineResult.SortedResolvedFields)
	{
		// Current Definition exact field copy입니다.
		FCFVehicleFieldEntry& Entry = Request.CurrentDefinition.SortedFields.AddDefaulted_GetRef();
		Entry.FieldPath = ResolvedField.FieldPath;
		Entry.Value = ResolvedField.Value;
	}
	Request.CurrentDefinition.DefinitionHash = BaselineResult.ResolvedDefinitionHash;

	// Last Apply field trace baseline입니다.
	FCFVehicleAppliedTrace& AppliedTrace = Request.Recipe.AppliedState.FieldTraces.AddDefaulted_GetRef();
	AppliedTrace.FieldPath = BaselineTorqueField->FieldPath;
	AppliedTrace.EffectiveSourceSignature = BaselineTorqueTrace->EffectiveSourceSignature;
	AppliedTrace.ShadowSourceSignature = BaselineTorqueTrace->ShadowSourceSignature;
	AppliedTrace.LastAppliedValueHash = FCFVehicleFieldCodec::HashValue(BaselineTorqueField->Value);

	// Current Target torque만 외부에서 달라진 상태를 value-copy로 모사합니다.
	FCFVehicleFieldOverride DriftTorque = CFVehicleResolverTestsPrivate::MakeFloatOverride(TEXT("VehicleMovementConfig.EngineMaxTorque"), 3333.0f);
	for (FCFVehicleFieldEntry& CurrentField : Request.CurrentDefinition.SortedFields)
	{
		if (CurrentField.FieldPath.ToCanonicalString(true) == TEXT("VehicleMovementConfig.EngineMaxTorque"))
		{
			CurrentField.Value = DriftTorque.OverrideValue;
		}
	}

	// Recipe Feel semantic input과 fingerprint를 변경해 effective source signature도 stale하게 만듭니다.
	Request.Recipe.DrivingFeelIntent.AccelerationFeel = 0.8f;
	Request.Recipe.RecipeFingerprint = TEXT("recipe-foundation-v2");
	// Changed source + drift를 함께 가진 Resolve 결과입니다.
	FCFVehicleResolveResult ChangedResult;
		TestTrue(TEXT("Diff/Stale changed Resolve executes"), FCFVehicleResolver::Resolve(Request, ChangedResult));
	TestEqual(TEXT("Diff/Stale source math remains valid while Definition stays blocked"), ChangedResult.ResolveStatus, ECFVehicleResolveStatus::Blocked);
	TestTrue(TEXT("R14 produces field diff"), ChangedResult.FieldDiff.Num() > 0);
	TestTrue(TEXT("R16 reports effective stale"), ChangedResult.StaleReport.bHasEffectiveStale);
	TestTrue(TEXT("R16 reports external drift"), ChangedResult.StaleReport.bHasExternalDrift);

	// EngineMaxTorque stale row입니다.
	const FCFVehicleStaleField* TorqueStale = ChangedResult.StaleReport.Fields.FindByPredicate([](const FCFVehicleStaleField& Field)
	{
		return Field.FieldPath.ToCanonicalString(true) == TEXT("VehicleMovementConfig.EngineMaxTorque");
	});
	if (TestNotNull(TEXT("Torque stale row exists"), TorqueStale))
	{
		TestTrue(TEXT("Torque effective source changed"), TorqueStale->bEffectiveSourceChanged);
		TestTrue(TEXT("Torque external drift detected"), TorqueStale->bExternalDrift);
	}
	return true;
}

// 동일 precedence의 두 Source가 같은 field에 들어올 때 silent last-write-wins 없이 Error로 닫히는지 검증합니다.
bool FCFVehicleResolverConflictTest::RunTest(const FString& Parameters)
{
	// Baseline managed Resolver request입니다.
	FCFVehicleResolveRequest Request;
	// Request builder 실패 사유입니다.
	FString BuildError;
	if (!TestTrue(TEXT("Conflict request builds"), CFVehicleResolverTestsPrivate::BuildManagedRequest(Request, BuildError)))
	{
		AddError(BuildError);
		return false;
	}

	// Wheel proposal adoption으로 unrelated managed blocker를 제거합니다.
	FCFVehicleResolveResult ProposalResult;
	FCFVehicleResolver::Resolve(Request, ProposalResult);
	CFVehicleResolverTestsPrivate::AdoptAllWheelMeasurements(Request, ProposalResult);

	// 동일 field/동일 Advanced precedence의 첫 override입니다.
	Request.Recipe.AdvancedOverrides.Add(CFVehicleResolverTestsPrivate::MakeFloatOverride(TEXT("VehicleMovementConfig.EngineMaxTorque"), 2000.0f));
	// 동일 field/동일 Advanced precedence의 충돌 override입니다.
	Request.Recipe.AdvancedOverrides.Add(CFVehicleResolverTestsPrivate::MakeFloatOverride(TEXT("VehicleMovementConfig.EngineMaxTorque"), 2100.0f));

	// Fail-closed conflict Resolve입니다.
	FCFVehicleResolveResult ConflictResult;
	TestFalse(TEXT("Same-precedence conflict returns internal failure result"), FCFVehicleResolver::Resolve(Request, ConflictResult));
	TestEqual(TEXT("Same-precedence conflict sets Error status"), ConflictResult.ResolveStatus, ECFVehicleResolveStatus::Error);
	TestTrue(TEXT("Duplicate persisted path caught at R0"), CFVehicleResolverTestsPrivate::HasIssueCode(ConflictResult.RecipeValidation, TEXT("DuplicateOverridePath")));
		TestTrue(TEXT("Candidate stack conflict caught fail-closed"), CFVehicleResolverTestsPrivate::HasIssueCode(ConflictResult.ResolverValidation, TEXT("SamePrecedenceConflict")));
	TestEqual(TEXT("R15 is Failed when an earlier Resolver internal error exists"), ConflictResult.StageRecords[15].Status, ECFVehicleResolverStageStatus::Failed);
	return true;
}

// Stable-ID arrays를 canonical selector order로 transient 구성하고 Resolver-owned readback hash가 정확히 일치하는지 검증합니다.
bool FCFVehicleMaterializerArraysTest::RunTest(const FString& Parameters)
{
	// Stable-array materialization에 사용할 synthetic managed request입니다.
	FCFVehicleResolveRequest Request;
	// Request helper 실패 이유입니다.
	FString BuildError;
	if (!TestTrue(TEXT("Materializer array request builds"), CFVehicleResolverTestsPrivate::BuildManagedRequest(Request, BuildError)))
	{
		AddError(BuildError);
		return false;
	}

	// 의도적으로 lexical 역순에 가까운 첫 Hardpoint intent입니다.
	FCFHardpointIntent& TopHardpoint = Request.Recipe.HardpointIntents.AddDefaulted_GetRef();
	TopHardpoint.LocationSlotId = TEXT("Top_02");
	TopHardpoint.LocationCategory = TEXT("Top");
	TopHardpoint.SocketName = TEXT("HP_Top_02");
	// 두 번째 Hardpoint intent입니다.
	FCFHardpointIntent& FrontHardpoint = Request.Recipe.HardpointIntents.AddDefaulted_GetRef();
	FrontHardpoint.LocationSlotId = TEXT("Front_01");
	FrontHardpoint.LocationCategory = TEXT("Front");
	FrontHardpoint.SocketName = TEXT("HP_Front_01");
	CFVehicleResolverTestsPrivate::AddSocket(Request.Assets, TEXT("HP_Top_02"), FVector(0.0, 0.0, 90.0));
	CFVehicleResolverTestsPrivate::AddSocket(Request.Assets, TEXT("HP_Front_01"), FVector(120.0, 0.0, 40.0));

	// 의도적으로 lexical 뒤쪽 ID를 먼저 넣은 Mount intent입니다.
	FCFMountIntent& ZMount = Request.Recipe.MountIntents.AddDefaulted_GetRef();
	ZMount.MountProfileId = TEXT("M_Z");
	ZMount.LocationSlotRef = TEXT("Top_02");
	ZMount.MountType = ECFVehicleMountType::Turret;
	ZMount.SizeLimit = ECFVehicleWeaponSize::Medium;
	// 두 번째 Mount intent입니다.
	FCFMountIntent& AMount = Request.Recipe.MountIntents.AddDefaulted_GetRef();
	AMount.MountProfileId = TEXT("M_A");
	AMount.LocationSlotRef = TEXT("Front_01");
	AMount.MountType = ECFVehicleMountType::Fixed;
	AMount.SizeLimit = ECFVehicleWeaponSize::Small;

	// Wheel measurement fingerprints를 먼저 얻는 proposal resolve입니다.
	FCFVehicleResolveResult ProposalResult;
	TestTrue(TEXT("Array proposal Resolve executes"), FCFVehicleResolver::Resolve(Request, ProposalResult));
	CFVehicleResolverTestsPrivate::AdoptAllWheelMeasurements(Request, ProposalResult);

	// R15까지 수행한 stable-array Resolver result입니다.
	FCFVehicleResolveResult Result;
	TestTrue(TEXT("Array adopted Resolve executes"), FCFVehicleResolver::Resolve(Request, Result));
	TestEqual(TEXT("Synthetic Definition remains validator-blocked"), Result.ResolveStatus, ECFVehicleResolveStatus::Blocked);
	TestEqual(TEXT("R15 completed"), Result.StageRecords[15].Status, ECFVehicleResolverStageStatus::Completed);

	// Resolver output을 독립적으로 다시 materialize한 value-copy evidence입니다.
	FCFVehicleMaterializationResult MaterializedResult;
	// Direct Materializer 실패 이유입니다.
	FString MaterializeError;
	if (!TestTrue(TEXT("Direct transient materialization succeeds"), FCFVehicleMaterializer::MaterializeAndValidate(Result.SortedResolvedFields, MaterializedResult, MaterializeError)))
	{
		AddError(MaterializeError);
		return false;
	}

	TestEqual(TEXT("Materialized readback field count equals Resolver-owned field count"), MaterializedResult.ResolvedReadbackSnapshot.SortedFields.Num(), Result.SortedResolvedFields.Num());
	TestEqual(TEXT("Materialized readback hash matches Resolver hash"), MaterializedResult.ResolvedReadbackSnapshot.DefinitionHash, Result.ResolvedDefinitionHash);
	TestEqual(TEXT("Two Hardpoint elements materialized"), MaterializedResult.HardpointOrder.Num(), 2);
	TestEqual(TEXT("Hardpoint canonical order first"), MaterializedResult.HardpointOrder[0], FName(TEXT("Front_01")));
	TestEqual(TEXT("Hardpoint canonical order second"), MaterializedResult.HardpointOrder[1], FName(TEXT("Top_02")));
	TestEqual(TEXT("Two Mount elements materialized"), MaterializedResult.MountOrder.Num(), 2);
	TestEqual(TEXT("Mount canonical order first"), MaterializedResult.MountOrder[0], FName(TEXT("M_A")));
	TestEqual(TEXT("Mount canonical order second"), MaterializedResult.MountOrder[1], FName(TEXT("M_Z")));

	// 한 Hardpoint/Mount selector를 완전히 제거한 Resolver-owned projection입니다.
	TArray<FCFVehicleResolvedField> ReducedFields;
	for (const FCFVehicleResolvedField& ResolvedField : Result.SortedResolvedFields)
	{
		// Selector 제거 여부를 판정할 exact canonical path입니다.
		const FString CanonicalPath = ResolvedField.FieldPath.ToCanonicalString(true);
		if (CanonicalPath.Contains(TEXT("[LocationSlotId=Top_02]")) || CanonicalPath.Contains(TEXT("[MountProfileId=M_Z]")))
		{
			continue;
		}
		ReducedFields.Add(ResolvedField);
	}

	// 새 transient candidate에서 selector absence가 실제 array absence가 되는지 검증할 결과입니다.
	FCFVehicleMaterializationResult ReducedResult;
	if (!TestTrue(TEXT("Reduced projection materializes"), FCFVehicleMaterializer::MaterializeAndValidate(ReducedFields, ReducedResult, MaterializeError)))
	{
		AddError(MaterializeError);
		return false;
	}
	TestEqual(TEXT("Removed Hardpoint is absent from new transient array"), ReducedResult.HardpointOrder.Num(), 1);
	TestEqual(TEXT("Remaining Hardpoint identity"), ReducedResult.HardpointOrder[0], FName(TEXT("Front_01")));
	TestEqual(TEXT("Removed Mount is absent from new transient array"), ReducedResult.MountOrder.Num(), 1);
	TestEqual(TEXT("Remaining Mount identity"), ReducedResult.MountOrder[0], FName(TEXT("M_A")));
	return true;
}

// Field Codec type signature mismatch가 partial materialization으로 넘어가지 않고 즉시 fail-closed되는지 검증합니다.
bool FCFVehicleMaterializerImportTest::RunTest(const FString& Parameters)
{
	// Valid Resolver leaf set을 만들 synthetic request입니다.
	FCFVehicleResolveRequest Request;
	// Request helper 실패 이유입니다.
	FString BuildError;
	if (!TestTrue(TEXT("Materializer import request builds"), CFVehicleResolverTestsPrivate::BuildManagedRequest(Request, BuildError)))
	{
		AddError(BuildError);
		return false;
	}

	// Wheel proposal adoption을 위한 first pass입니다.
	FCFVehicleResolveResult ProposalResult;
	FCFVehicleResolver::Resolve(Request, ProposalResult);
	CFVehicleResolverTestsPrivate::AdoptAllWheelMeasurements(Request, ProposalResult);
	// 정상 Materializer 입력의 authority가 될 Resolver result입니다.
	FCFVehicleResolveResult Result;
	TestTrue(TEXT("Materializer import baseline Resolve executes"), FCFVehicleResolver::Resolve(Request, Result));
	TestTrue(TEXT("Baseline has resolved fields"), Result.SortedResolvedFields.Num() > 0);

	// 하나의 resolved field type contract만 의도적으로 손상한 ordered copy입니다.
	TArray<FCFVehicleResolvedField> CorruptedFields = Result.SortedResolvedFields;
	CorruptedFields[0].Value.PropertyTypeSignature = TEXT("Invalid.Materializer.TypeSignature");

	// Fail-closed materialization output입니다.
	FCFVehicleMaterializationResult CorruptedResult;
	// Expected checked-import error입니다.
	FString MaterializeError;
	TestFalse(TEXT("Type signature mismatch rejects materialization"), FCFVehicleMaterializer::MaterializeAndValidate(CorruptedFields, CorruptedResult, MaterializeError));
	TestTrue(TEXT("Rejected materialization reports an error"), !MaterializeError.IsEmpty());
	return true;
}

#endif
