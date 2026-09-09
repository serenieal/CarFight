// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAContractGuardTests.cpp
// Version: v1.1.0
// Date: 2026-09-09
// Description: CF-FQ-050 DACE-P0-01 contract descriptor, frozen bootstrap accepted snapshot과 production private probe focused Automation입니다.
// Changelog:
// - v1.1.0: DACE-P0-03 append-only history 확장을 허용하도록 BootstrapSnapshot을 exact1 history 요구에서 frozen first bootstrap + valid full chain 요구로 전환.
// - v1.0.0: descriptor exact-count/signature baseline, bootstrap snapshot chain, serializer→parser→fingerprint→materializer/extractor transient roundtrip 검증을 최초 추가.
// Migration:
// - Product path discovery, SyncProduct, ApplyReviewed, SavePackage를 호출하지 않습니다. 모든 probe는 memory JSON/DTO와 transient UObject만 사용합니다.

#include "CFDAContractGuard.h"

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDAContractGuardTestsPrivate
{
	// DACE-P0-01 probe가 사용하는 canonical main_game-relative Staging identity입니다.
	static constexpr TCHAR ProbeStagingPath[] = TEXT("Authoring/DataAssetStaging/MissileGuidePreset/__DACE_P0_01_Probe.json");

	// DACE-P0-01 probe가 사용하는 비persisted target object path token입니다.
	static constexpr TCHAR ProbeTargetObjectPath[] = TEXT("/Game/Test/CarFight/DACE/DA_DACE_P0_01_Probe.DA_DACE_P0_01_Probe");

	// Current strict schema를 만족하는 memory-only probe JSON을 생성합니다.
	FString BuildProbeJson()
	{
		return TEXT(R"JSON(
{
  "SchemaId": "CarFight.DataAsset.MissileGuidePreset",
  "SchemaRevision": 1,
  "AdapterContractRevision": 2,
  "DataAssetTypeClassPath": "/Script/CarFight_Re.CFMissileGuidePresetData",
  "StableLogicalId": "DACE_P0_01_Probe",
  "TargetObjectPath": "/Game/Test/CarFight/DACE/DA_DACE_P0_01_Probe.DA_DACE_P0_01_Probe",
  "BaseSemanticFingerprint": null,
  "Payload": {
    "PresetId": "DACE_P0_01_Probe",
    "PresetDisplayName": {"Kind": "Literal", "Text": "DACE P0-01 Probe"},
    "PresetDescription": {"Kind": "Literal", "Text": "Contract foundation transient probe"},
    "MissileGuideConfig": {
      "bUseGuidance": true,
      "GuideMode": "TargetActor",
      "LostTargetPolicy": "ContinueStraight",
      "NavigationConstant": 3.0,
      "MaximumTurnRateDegPerSec": 35.0,
      "MaximumLateralAccelerationCmPerSecSq": 2000.0,
      "GuidanceResponseTimeSeconds": 0.18,
      "MinimumGuidanceSpeedCmPerSec": 500.0,
      "SeekerFieldOfViewDeg": 60.0,
      "LockBreakAngleDeg": 85.0,
      "TargetLostGraceTimeSeconds": 0.2,
      "SeekerModel": "Stateful",
      "TargetObservationMode": "SampledPositionEstimate",
      "GuidanceLaw": "PurePursuit",
      "GuidanceActivationMode": "Independent",
      "GuidanceActivationDelaySeconds": 0.25,
      "GuidanceActivationDistanceCm": 500.0,
      "LeadTimeSeconds": 0.1,
      "MaxLeadDistanceCm": 1200.0,
      "ReacquisitionMode": "ForwardCone",
      "TargetObservationIntervalSeconds": 0.08,
      "TargetVelocityEstimateResponseTimeSeconds": 0.25,
      "AcquisitionConeHalfAngleDeg": 45.0,
      "TrackingConeHalfAngleDeg": 60.0,
      "ReacquisitionConeHalfAngleDeg": 55.0,
      "ReacquisitionTimeSeconds": 0.4
    }
  }
}
)JSON");
	}

	// Current production semantic fingerprint가 emit해야 하는 exact token label sequence를 반환합니다.
	TArray<FString> BuildExpectedFingerprintLabels()
	{
		// Production AppendMissilePresetPayloadTokens의 exact label sequence입니다.
		TArray<FString> Labels =
		{
			TEXT("SchemaId"),
			TEXT("SchemaRevision"),
			TEXT("AdapterContractRevision"),
			TEXT("DataAssetTypeClassPath"),
			TEXT("Payload.PresetId"),
			TEXT("Payload.PresetDisplayName"),
			TEXT("Payload.PresetDescription"),
			TEXT("Config.bUseGuidance"),
			TEXT("Config.GuideMode"),
			TEXT("Config.LostTargetPolicy"),
			TEXT("Config.NavigationConstant"),
			TEXT("Config.MaximumTurnRateDegPerSec"),
			TEXT("Config.MaximumLateralAccelerationCmPerSecSq"),
			TEXT("Config.GuidanceResponseTimeSeconds"),
			TEXT("Config.MinimumGuidanceSpeedCmPerSec"),
			TEXT("Config.SeekerFieldOfViewDeg"),
			TEXT("Config.LockBreakAngleDeg"),
			TEXT("Config.TargetLostGraceTimeSeconds"),
			TEXT("Config.SeekerModel"),
			TEXT("Config.TargetObservationMode"),
			TEXT("Config.GuidanceLaw"),
			TEXT("Config.GuidanceActivationMode"),
			TEXT("Config.GuidanceActivationDelaySeconds"),
			TEXT("Config.GuidanceActivationDistanceCm"),
			TEXT("Config.LeadTimeSeconds"),
			TEXT("Config.MaxLeadDistanceCm"),
			TEXT("Config.ReacquisitionMode"),
			TEXT("Config.TargetObservationIntervalSeconds"),
			TEXT("Config.TargetVelocityEstimateResponseTimeSeconds"),
			TEXT("Config.AcquisitionConeHalfAngleDeg"),
			TEXT("Config.TrackingConeHalfAngleDeg"),
			TEXT("Config.ReacquisitionConeHalfAngleDeg"),
			TEXT("Config.ReacquisitionTimeSeconds")
		};
		return Labels;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAContractDescriptorTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_01.ContractDescriptor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAContractBootstrapTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_01.BootstrapSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAContractProductionProbeTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_01.ProductionProbe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Current descriptor exact cardinality와 deterministic component signature 생성을 검증합니다.
bool FCFDAContractDescriptorTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Source descriptor must remain exact top4+nested26"), FCFDAContractGuard::GetSourceShapeDescriptor().Num(), 30);
	TestEqual(TEXT("Adapter descriptor must remain exact root/payload physical42"), FCFDAContractGuard::GetAdapterShapeDescriptor().Num(), 42);
	TestEqual(TEXT("Source↔Adapter mapping must remain exact38"), FCFDAContractGuard::GetSourceAdapterMappingDescriptor().Num(), 38);
	TestEqual(TEXT("Explicit semantic contract rules must remain exact26"), FCFDAContractGuard::GetSemanticContractDescriptor().Num(), 26);

	// 첫 번째 deterministic signature 계산 결과입니다.
	FCFDAContractSignatures FirstSignatures;
	// 첫 번째 signature 계산 실패 사유입니다.
	FString FirstError;
	TestTrue(TEXT("Current descriptor signatures must build"), FCFDAContractGuard::BuildCurrentSignatures(FirstSignatures, FirstError));
	TestTrue(TEXT("Source signature must be canonical SHA-256"), FirstSignatures.SourceShapeSignature.StartsWith(TEXT("sha256:")));
	TestTrue(TEXT("Adapter signature must be canonical SHA-256"), FirstSignatures.AdapterShapeSignature.StartsWith(TEXT("sha256:")));
	TestTrue(TEXT("Mapping signature must be canonical SHA-256"), FirstSignatures.SourceAdapterMappingSignature.StartsWith(TEXT("sha256:")));
	TestTrue(TEXT("Semantic signature must be canonical SHA-256"), FirstSignatures.SemanticContractSignature.StartsWith(TEXT("sha256:")));

	// 두 번째 deterministic signature 계산 결과입니다.
	FCFDAContractSignatures SecondSignatures;
	// 두 번째 signature 계산 실패 사유입니다.
	FString SecondError;
	TestTrue(TEXT("Repeated current descriptor signatures must build"), FCFDAContractGuard::BuildCurrentSignatures(SecondSignatures, SecondError));
	TestEqual(TEXT("Source signature must be deterministic"), FirstSignatures.SourceShapeSignature, SecondSignatures.SourceShapeSignature);
	TestEqual(TEXT("Adapter signature must be deterministic"), FirstSignatures.AdapterShapeSignature, SecondSignatures.AdapterShapeSignature);
	TestEqual(TEXT("Mapping signature must be deterministic"), FirstSignatures.SourceAdapterMappingSignature, SecondSignatures.SourceAdapterMappingSignature);
	TestEqual(TEXT("Semantic signature must be deterministic"), FirstSignatures.SemanticContractSignature, SecondSignatures.SemanticContractSignature);
	return true;
}

// Production-owned accepted history의 frozen first bootstrap과 append-only full chain을 검증합니다.
bool FCFDAContractBootstrapTest::RunTest(const FString& Parameters)
{
	// Production Editor Private authority가 반환한 accepted history입니다.
	const TArray<FCFDAAcceptedContractSnapshot>& Snapshots = FCFDAContractGuard::GetAcceptedSnapshots();
	TestTrue(TEXT("DACE accepted snapshot history must contain the frozen bootstrap"), Snapshots.Num() >= 1);
	if (Snapshots.IsEmpty())
	{
		return false;
	}

	// Historical first bootstrap accepted snapshot입니다.
	const FCFDAAcceptedContractSnapshot& Bootstrap = Snapshots[0];
	TestEqual(TEXT("Bootstrap SnapshotId"), Bootstrap.SnapshotId, FString(TEXT("DACE-MissileGuidePreset-S1-A2-Bootstrap")));
	TestEqual(TEXT("Bootstrap SchemaId"), Bootstrap.SchemaId, FString(TEXT("CarFight.DataAsset.MissileGuidePreset")));
	TestEqual(TEXT("Bootstrap SchemaRevision"), Bootstrap.SchemaRevision, 1);
	TestEqual(TEXT("Bootstrap AdapterContractRevision"), Bootstrap.AdapterContractRevision, 2);
	TestEqual(TEXT("Bootstrap class path"), Bootstrap.DataAssetTypeClassPath, FString(TEXT("/Script/CarFight_Re.CFMissileGuidePresetData")));
	TestTrue(TEXT("Bootstrap previous snapshot signature must be empty"), Bootstrap.PreviousSnapshotSignature.IsEmpty());
	TestEqual(TEXT("Bootstrap migration impact must be NoMigration"), Bootstrap.MigrationImpact, ECFDAContractMigrationImpact::NoMigration);
	TestEqual(TEXT("Bootstrap migration resolution must be NotRequired"), Bootstrap.MigrationResolution, ECFDAContractMigrationResolution::NotRequired);

	// Frozen bootstrap와 전체 append-only chain validation failure 사유입니다.
	FString ValidationError;
	// Bootstrap/history validation terminal 결과입니다.
	const bool bBootstrapValid = FCFDAContractGuard::ValidateBootstrapAcceptedSnapshot(ValidationError);
	TestTrue(TEXT("Frozen bootstrap and accepted snapshot chain must remain valid"), bBootstrapValid);
	if (!bBootstrapValid && !ValidationError.IsEmpty())
	{
		AddError(ValidationError);
	}
	return true;
}

// Production serializer/parser/fingerprint/materializer/extractor private probe를 memory/transient only로 관통 검증합니다.
bool FCFDAContractProductionProbeTest::RunTest(const FString& Parameters)
{
	using namespace CFDAContractGuardTestsPrivate;

	// Current strict parser를 통해 만든 valid memory-only source payload입니다.
	const FCFDAStagingParseResult InitialParse = CFDAContractProbeParse(BuildProbeJson(), ProbeStagingPath);
	TestTrue(TEXT("Memory-only DACE source fixture must parse through production parser"), InitialParse.bValid);
	if (!InitialParse.bValid)
	{
		return false;
	}

	// Production fingerprint 결과입니다.
	FString SourceFingerprint;
	// Production fingerprint actual emitted token label sequence입니다.
	TArray<FString> ObservedLabels;
	// Production fingerprint probe failure 사유입니다.
	FString FingerprintError;
	TestTrue(TEXT("Production fingerprint probe must succeed"), CFDAContractProbeFingerprint(InitialParse.Record.Payload, SourceFingerprint, ObservedLabels, FingerprintError));
	// Current production fingerprint가 emit해야 하는 exact expected label sequence입니다.
	const TArray<FString> ExpectedLabels = BuildExpectedFingerprintLabels();
	TestEqual(TEXT("Production fingerprint token label count must remain exact"), ObservedLabels.Num(), ExpectedLabels.Num());
	if (ObservedLabels.Num() == ExpectedLabels.Num())
	{
		for (int32 LabelIndex = 0; LabelIndex < ExpectedLabels.Num(); ++LabelIndex)
		{
			TestEqual(
				*FString::Printf(TEXT("Production fingerprint token label[%d] must remain exact"), LabelIndex),
				ObservedLabels[LabelIndex],
				ExpectedLabels[LabelIndex]);
		}
	}
	TestEqual(TEXT("Production fingerprint probe must equal parser-computed staging fingerprint"), SourceFingerprint, InitialParse.Record.StagingSemanticFingerprint);

	// Production serializer가 생성한 memory-only canonical JSON입니다.
	FString SerializedJson;
	// Production serializer probe failure 사유입니다.
	FString SerializeError;
	TestTrue(
		TEXT("Production serializer private probe must succeed without file write"),
		CFDAContractProbeSerialize(InitialParse.Record.Payload, ProbeTargetObjectPath, SourceFingerprint, SerializedJson, SerializeError));
	TestFalse(TEXT("Production serializer output must not be empty"), SerializedJson.IsEmpty());

	// Production serializer output을 production strict parser로 다시 읽은 결과입니다.
	const FCFDAStagingParseResult SerializedParse = CFDAContractProbeParse(SerializedJson, ProbeStagingPath);
	TestTrue(TEXT("Production serializer output must pass production strict parser"), SerializedParse.bValid);
	if (!SerializedParse.bValid)
	{
		return false;
	}
	TestTrue(TEXT("Serializer probe output must represent Update baseline"), SerializedParse.Record.bHasBaseSemanticFingerprint);
	TestEqual(TEXT("Serializer probe BaseSemanticFingerprint must equal supplied source fingerprint"), SerializedParse.Record.BaseSemanticFingerprint, SourceFingerprint);
	TestEqual(TEXT("Serializer→parser semantic fingerprint must remain unchanged"), SerializedParse.Record.StagingSemanticFingerprint, SourceFingerprint);

	// Production materializer→extractor transient roundtrip payload입니다.
	FCFDAMissilePresetPayload ReadbackPayload;
	// Production transient materializer/extractor probe failure 사유입니다.
	FString RoundTripError;
	TestTrue(TEXT("Production materializer→extractor transient roundtrip must succeed"), CFDAContractProbeMaterializeRoundTrip(InitialParse.Record.Payload, ReadbackPayload, RoundTripError));

	// Transient readback payload의 production semantic fingerprint입니다.
	FString ReadbackFingerprint;
	// Transient readback fingerprint failure 사유입니다.
	FString ReadbackFingerprintError;
	TestTrue(TEXT("Transient readback payload must fingerprint"), FCFDAStagingService::BuildSemanticFingerprint(ReadbackPayload, ReadbackFingerprint, ReadbackFingerprintError));
	TestEqual(TEXT("Materializer→extractor roundtrip must preserve full typed semantic fingerprint"), ReadbackFingerprint, SourceFingerprint);
	TestEqual(TEXT("Materializer→extractor roundtrip must preserve PresetId"), ReadbackPayload.PresetId, InitialParse.Record.Payload.PresetId);
	TestEqual(TEXT("Materializer→extractor roundtrip must preserve display source string"), ReadbackPayload.PresetDisplayName.Text, InitialParse.Record.Payload.PresetDisplayName.Text);
	TestEqual(TEXT("Materializer→extractor roundtrip must preserve description source string"), ReadbackPayload.PresetDescription.Text, InitialParse.Record.Payload.PresetDescription.Text);
	return true;
}

#endif
