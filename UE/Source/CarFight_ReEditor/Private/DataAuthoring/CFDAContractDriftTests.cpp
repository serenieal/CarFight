// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAContractDriftTests.cpp
// Version: v1.1.0
// Date: 2026-09-09
// Description: CF-FQ-050 DACE-P0-02 native Reflection과 Adapter/mapping/serializer/parser/fingerprint/materializer-extractor structural drift negative regression입니다.
// Changelog:
// - v1.1.0: Source descriptor↔materializer sentinel exact set binding, mapping descriptor↔serializer per-leaf distinguishable sentinel roundtrip, recursive container inner-type drift fixture, parser exact issue FieldPath 검증을 추가.
// - v1.0.1: Adapter duplicate negative fixture가 TArray 내부 element reference를 같은 배열 Add에 전달하지 않도록 값 복사해 UE self-alias assertion을 제거.
// - v1.0.0: Source field add/remove/rename/type/nested drift, Adapter/mapping drift, serializer shape, strict parser required/type/unknown matrix, fingerprint token, materializer-extractor per-leaf sentinel negative regression을 최초 추가.
// Migration:
// - 실제 Product class나 canonical Product Staging을 수정하지 않습니다. 모든 negative fixture는 Reflection 결과 복사본, memory JSON/DTO 또는 transient UObject에서만 수행합니다.
// - v1.1.0부터 sentinel 목록 개수 literal은 coverage authority가 아니며 SourceShape/SourceAdapterMapping descriptor의 current leaf set과 exact 결속되어야 합니다.

#include "CFDAContractGuard.h"

#include "CFMissileGuidePresetData.h"
#include "Dom/JsonObject.h"
#include "Misc/AutomationTest.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Templates/Function.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDAContractDriftTestsPrivate
{
	// DACE-P0-02 memory probe가 사용하는 canonical Staging identity입니다.
	static constexpr TCHAR ProbeStagingPath[] = TEXT("Authoring/DataAssetStaging/MissileGuidePreset/__DACE_P0_02_Probe.json");
	// DACE-P0-02 serializer probe가 사용하는 비persisted target object path token입니다.
	static constexpr TCHAR ProbeTargetObjectPath[] = TEXT("/Game/Test/CarFight/DACE/DA_DACE_P0_02_Probe.DA_DACE_P0_02_Probe");

	// Current strict schema exact42를 만족하는 memory-only probe JSON을 생성합니다.
	FString BuildProbeJson()
	{
		return TEXT(R"JSON(
{
  "SchemaId": "CarFight.DataAsset.MissileGuidePreset",
  "SchemaRevision": 1,
  "AdapterContractRevision": 2,
  "DataAssetTypeClassPath": "/Script/CarFight_Re.CFMissileGuidePresetData",
  "StableLogicalId": "DACE_P0_02_Probe",
  "TargetObjectPath": "/Game/Test/CarFight/DACE/DA_DACE_P0_02_Probe.DA_DACE_P0_02_Probe",
  "BaseSemanticFingerprint": null,
  "Payload": {
    "PresetId": "DACE_P0_02_Probe",
    "PresetDisplayName": {"Kind": "Literal", "Text": "DACE P0-02 Probe"},
    "PresetDescription": {"Kind": "Literal", "Text": "Structural drift transient probe"},
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

	// JSON text를 mutable root object로 deserialize합니다.
	bool ParseJsonObject(const FString& JsonText, TSharedPtr<FJsonObject>& OutRoot)
	{
		// Memory-only JSON reader입니다.
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		return FJsonSerializer::Deserialize(Reader, OutRoot) && OutRoot.IsValid();
	}

	// Mutable root object를 test parser에 다시 넣을 JSON text로 serialize합니다.
	bool SerializeJsonObject(const TSharedPtr<FJsonObject>& Root, FString& OutJsonText)
	{
		OutJsonText.Reset();
		if (!Root.IsValid()) return false;
		// Memory-only JSON writer입니다.
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJsonText);
		const bool bSerialized = FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
		Writer->Close();
		return bSerialized;
	}

	// Dot-separated JSON path의 object parent와 leaf field name을 찾습니다.
	bool FindJsonParent(const TSharedPtr<FJsonObject>& Root, const FString& FieldPath, TSharedPtr<FJsonObject>& OutParent, FString& OutLeafName)
	{
		// Dot-separated path segments입니다.
		TArray<FString> Segments;
		FieldPath.ParseIntoArray(Segments, TEXT("."), true);
		if (!Root.IsValid() || Segments.IsEmpty()) return false;
		OutParent = Root;
		for (int32 SegmentIndex = 0; SegmentIndex < Segments.Num() - 1; ++SegmentIndex)
		{
			// 현재 parent에서 읽을 child object field입니다.
			const TSharedPtr<FJsonValue> ChildValue = OutParent->GetFieldUntyped(Segments[SegmentIndex]);
			if (!ChildValue.IsValid() || ChildValue->Type != EJson::Object) return false;
			OutParent = ChildValue->AsObject();
			if (!OutParent.IsValid()) return false;
		}
		OutLeafName = Segments.Last();
		return true;
	}

	// Dot-separated path가 가리키는 JSON object를 찾습니다. Empty path는 root입니다.
	bool FindJsonObject(const TSharedPtr<FJsonObject>& Root, const FString& ObjectPath, TSharedPtr<FJsonObject>& OutObject)
	{
		if (!Root.IsValid()) return false;
		if (ObjectPath.IsEmpty())
		{
			OutObject = Root;
			return true;
		}
		// Object path segments입니다.
		TArray<FString> Segments;
		ObjectPath.ParseIntoArray(Segments, TEXT("."), true);
		OutObject = Root;
		for (const FString& Segment : Segments)
		{
			// 다음 child object field입니다.
			const TSharedPtr<FJsonValue> ChildValue = OutObject->GetFieldUntyped(Segment);
			if (!ChildValue.IsValid() || ChildValue->Type != EJson::Object) return false;
			OutObject = ChildValue->AsObject();
			if (!OutObject.IsValid()) return false;
		}
		return true;
	}

	// Fresh JSON에서 requested field 하나를 제거합니다.
	bool BuildJsonWithRemovedField(const FString& SourceJson, const FString& FieldPath, FString& OutJson)
	{
		// Mutable JSON root입니다.
		TSharedPtr<FJsonObject> Root;
		if (!ParseJsonObject(SourceJson, Root)) return false;
		// Requested field parent입니다.
		TSharedPtr<FJsonObject> Parent;
		// Requested leaf field name입니다.
		FString LeafName;
		if (!FindJsonParent(Root, FieldPath, Parent, LeafName) || !Parent->HasField(LeafName)) return false;
		Parent->RemoveField(LeafName);
		return SerializeJsonObject(Root, OutJson);
	}

	// Fresh JSON에서 requested field를 descriptor와 다른 JSON type으로 교체합니다.
	bool BuildJsonWithWrongType(const FString& SourceJson, const FCFDAAdapterFieldDescriptor& Descriptor, FString& OutJson)
	{
		// Mutable JSON root입니다.
		TSharedPtr<FJsonObject> Root;
		if (!ParseJsonObject(SourceJson, Root)) return false;
		// Requested field parent입니다.
		TSharedPtr<FJsonObject> Parent;
		// Requested leaf field name입니다.
		FString LeafName;
		if (!FindJsonParent(Root, Descriptor.AdapterJsonPath, Parent, LeafName)) return false;
		if (Descriptor.JsonValueKind.Equals(TEXT("Boolean"), ESearchCase::CaseSensitive))
		{
			Parent->SetStringField(LeafName, TEXT("DACE_WrongType"));
		}
		else
		{
			Parent->SetBoolField(LeafName, true);
		}
		return SerializeJsonObject(Root, OutJson);
	}

	// Fresh JSON의 requested object에 unknown sibling field를 추가합니다.
	bool BuildJsonWithUnknownField(const FString& SourceJson, const FString& ObjectPath, FString& OutJson)
	{
		// Mutable JSON root입니다.
		TSharedPtr<FJsonObject> Root;
		if (!ParseJsonObject(SourceJson, Root)) return false;
		// Unknown sibling을 추가할 exact object입니다.
		TSharedPtr<FJsonObject> TargetObject;
		if (!FindJsonObject(Root, ObjectPath, TargetObject)) return false;
		TargetObject->SetStringField(TEXT("__DACE_UnknownField"), TEXT("must-fail"));
		return SerializeJsonObject(Root, OutJson);
	}

	// Production parser를 통과한 baseline payload와 production serializer JSON을 준비합니다.
	bool BuildProductionSerializedFixture(FCFDAMissilePresetPayload& OutPayload, FString& OutFingerprint, FString& OutSerializedJson, FString& OutError)
	{
		// Production strict parser baseline입니다.
		const FCFDAStagingParseResult Parsed = CFDAContractProbeParse(BuildProbeJson(), ProbeStagingPath);
		if (!Parsed.bValid)
		{
			OutError = TEXT("DACE-P0-02 baseline memory JSON이 production parser에서 실패했습니다.");
			return false;
		}
		OutPayload = Parsed.Record.Payload;
		// Actual production token labels는 이 helper에서는 소비하지 않습니다.
		TArray<FString> IgnoredLabels;
		if (!CFDAContractProbeFingerprint(OutPayload, OutFingerprint, IgnoredLabels, OutError)) return false;
		if (!CFDAContractProbeSerialize(OutPayload, ProbeTargetObjectPath, OutFingerprint, OutSerializedJson, OutError)) return false;
		OutError.Reset();
		return true;
	}

	// 한 payload authored leaf를 바꾸는 materializer/extractor sentinel mutation입니다.
	struct FPayloadMutation
	{
		// Source authored leaf를 식별하는 이름입니다.
		FString SourcePropertyPath;
		// 해당 leaf만 구별 가능한 값으로 바꾸는 mutation 함수입니다.
		TFunction<void(FCFDAMissilePresetPayload&)> Apply;
	};

	// Source semantic leaf exact29에 대한 distinguishable sentinel mutation을 만듭니다.
	TArray<FPayloadMutation> BuildPayloadMutations()
	{
		// Top-level exact3 + nested config exact26 mutation 목록입니다.
		TArray<FPayloadMutation> Mutations;
		Mutations.Reserve(29);
		Mutations.Add({TEXT("PresetId"), [](FCFDAMissilePresetPayload& Payload) { Payload.PresetId = FName(TEXT("DACE_P0_02_Mutated")); }});
		Mutations.Add({TEXT("PresetDisplayName"), [](FCFDAMissilePresetPayload& Payload) { Payload.PresetDisplayName.Text = TEXT("DACE mutated display"); }});
		Mutations.Add({TEXT("PresetDescription"), [](FCFDAMissilePresetPayload& Payload) { Payload.PresetDescription.Text = TEXT("DACE mutated description"); }});
		Mutations.Add({TEXT("MissileGuideConfig.bUseGuidance"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.bUseGuidance = false; }});
		Mutations.Add({TEXT("MissileGuideConfig.GuideMode"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.GuideMode = ECFMissileGuideMode::LaserPoint; }});
		Mutations.Add({TEXT("MissileGuideConfig.LostTargetPolicy"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.LostTargetPolicy = ECFMissileLostTargetPolicy::Expire; }});
		Mutations.Add({TEXT("MissileGuideConfig.NavigationConstant"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.NavigationConstant = 4.25f; }});
		Mutations.Add({TEXT("MissileGuideConfig.MaximumTurnRateDegPerSec"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.MaximumTurnRateDegPerSec = 123.0f; }});
		Mutations.Add({TEXT("MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq = 4321.0f; }});
		Mutations.Add({TEXT("MissileGuideConfig.GuidanceResponseTimeSeconds"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.GuidanceResponseTimeSeconds = 0.321f; }});
		Mutations.Add({TEXT("MissileGuideConfig.MinimumGuidanceSpeedCmPerSec"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.MinimumGuidanceSpeedCmPerSec = 777.0f; }});
		Mutations.Add({TEXT("MissileGuideConfig.SeekerFieldOfViewDeg"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.SeekerFieldOfViewDeg = 123.0f; }});
		Mutations.Add({TEXT("MissileGuideConfig.LockBreakAngleDeg"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.LockBreakAngleDeg = 99.0f; }});
		Mutations.Add({TEXT("MissileGuideConfig.TargetLostGraceTimeSeconds"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.TargetLostGraceTimeSeconds = 0.77f; }});
		Mutations.Add({TEXT("MissileGuideConfig.SeekerModel"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.SeekerModel = ECFMissileSeekerModel::LegacySingleGate; }});
		Mutations.Add({TEXT("MissileGuideConfig.TargetObservationMode"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.TargetObservationMode = ECFMissileTargetObservationMode::DirectActorKinematics; }});
		Mutations.Add({TEXT("MissileGuideConfig.GuidanceLaw"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.GuidanceLaw = ECFMissileGuidanceLaw::LeadPursuit; }});
		Mutations.Add({TEXT("MissileGuideConfig.GuidanceActivationMode"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.GuidanceActivationMode = ECFMissileGuidanceActivationMode::FollowFlightGuidanceWindow; }});
		Mutations.Add({TEXT("MissileGuideConfig.GuidanceActivationDelaySeconds"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.GuidanceActivationDelaySeconds = 0.66f; }});
		Mutations.Add({TEXT("MissileGuideConfig.GuidanceActivationDistanceCm"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.GuidanceActivationDistanceCm = 9876.0f; }});
		Mutations.Add({TEXT("MissileGuideConfig.LeadTimeSeconds"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.LeadTimeSeconds = 0.75f; }});
		Mutations.Add({TEXT("MissileGuideConfig.MaxLeadDistanceCm"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.MaxLeadDistanceCm = 3456.0f; }});
		Mutations.Add({TEXT("MissileGuideConfig.ReacquisitionMode"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.ReacquisitionMode = ECFMissileReacquisitionMode::None; }});
		Mutations.Add({TEXT("MissileGuideConfig.TargetObservationIntervalSeconds"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.TargetObservationIntervalSeconds = 0.21f; }});
		Mutations.Add({TEXT("MissileGuideConfig.TargetVelocityEstimateResponseTimeSeconds"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.TargetVelocityEstimateResponseTimeSeconds = 0.44f; }});
		Mutations.Add({TEXT("MissileGuideConfig.AcquisitionConeHalfAngleDeg"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.AcquisitionConeHalfAngleDeg = 66.0f; }});
		Mutations.Add({TEXT("MissileGuideConfig.TrackingConeHalfAngleDeg"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.TrackingConeHalfAngleDeg = 77.0f; }});
		Mutations.Add({TEXT("MissileGuideConfig.ReacquisitionConeHalfAngleDeg"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.ReacquisitionConeHalfAngleDeg = 88.0f; }});
		Mutations.Add({TEXT("MissileGuideConfig.ReacquisitionTimeSeconds"), [](FCFDAMissilePresetPayload& Payload) { Payload.MissileGuideConfig.ReacquisitionTimeSeconds = 1.2f; }});
		return Mutations;
	}

	// 두 FString set이 exact same membership인지 검사합니다.
	bool AreStringSetsEqual(const TSet<FString>& Left, const TSet<FString>& Right)
	{
		if (Left.Num() != Right.Num()) return false;
		for (const FString& Value : Left)
		{
			if (!Right.Contains(Value)) return false;
		}
		return true;
	}

	// SourceShape descriptor에서 Struct container node를 제외한 authored semantic leaf path set을 만듭니다.
	TSet<FString> BuildSourceSemanticLeafPathSet()
	{
		// Source descriptor가 소유하는 semantic leaf paths입니다.
		TSet<FString> SourceLeafPaths;
		for (const FCFDASourceFieldDescriptor& Descriptor : FCFDAContractGuard::GetSourceShapeDescriptor())
		{
			if (!Descriptor.PropertyKind.Equals(TEXT("Struct"), ESearchCase::CaseSensitive))
			{
				SourceLeafPaths.Add(Descriptor.SourcePropertyPath);
			}
		}
		return SourceLeafPaths;
	}

	// SourceAdapterMapping descriptor의 SourceToAdapter rows에서 unique Source leaf path set을 만듭니다.
	TSet<FString> BuildMappedSourceLeafPathSet()
	{
		// Mapping descriptor가 serializer value wiring 대상으로 선언한 Source leaf paths입니다.
		TSet<FString> MappedSourceLeafPaths;
		for (const FCFDASourceAdapterMappingDescriptor& Descriptor : FCFDAContractGuard::GetSourceAdapterMappingDescriptor())
		{
			if (Descriptor.MappingRole.Equals(TEXT("SourceToAdapter"), ESearchCase::CaseSensitive))
			{
				MappedSourceLeafPaths.Add(Descriptor.SourcePropertyPath);
			}
		}
		return MappedSourceLeafPaths;
	}

	// Sentinel mutation 목록의 SourcePropertyPath를 unique set으로 변환합니다.
	TSet<FString> BuildMutationSourcePathSet(const TArray<FPayloadMutation>& Mutations)
	{
		// Mutation 목록이 실제로 덮는 Source leaf paths입니다.
		TSet<FString> MutationSourcePaths;
		for (const FPayloadMutation& Mutation : Mutations)
		{
			MutationSourcePaths.Add(Mutation.SourcePropertyPath);
		}
		return MutationSourcePaths;
	}

	// Staging issue 배열에 requested code와 exact FieldPath가 함께 존재하는지 확인합니다.
	bool HasStagingIssueAtExactPath(
		const TArray<FCFDAStagingIssue>& Issues,
		const ECFDAStagingIssueCode Code,
		const FString& ExpectedFieldPath)
	{
		for (const FCFDAStagingIssue& Issue : Issues)
		{
			if (Issue.Code == Code && Issue.FieldPath.Equals(ExpectedFieldPath, ESearchCase::CaseSensitive))
			{
				return true;
			}
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDASourceReflectionDriftTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_02.SourceReflection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAAdapterMappingDriftTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_02.AdapterMapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDASerializerParserDriftTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_02.SerializerParser",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAFingerprintDriftTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_02.Fingerprint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAMaterializerExtractorDriftTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_02.MaterializerExtractor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Actual Reflection PASS와 add/remove/rename/type/nested fixture drift fail-closed를 검증합니다.
bool FCFDASourceReflectionDriftTest::RunTest(const FString& Parameters)
{
	// Production-side current structural gate 결과입니다.
	const FCFDAContractGuardResult CurrentStructuralResult = FCFDAContractGuard::ValidateCurrentStructuralCoverage();
	TestTrue(TEXT("Current native Reflection and mapping structural gate must pass"), CurrentStructuralResult.bPassed);

	// Actual native Reflection descriptor입니다.
	TArray<FCFDASourceFieldDescriptor> ReflectedDescriptors;
	// Reflection build failure 사유입니다.
	FString ReflectionError;
	TestTrue(TEXT("Native Reflection source descriptor must build"), FCFDAContractGuard::BuildReflectedSourceShapeDescriptor(ReflectedDescriptors, ReflectionError));
	TestEqual(TEXT("Native Reflection source shape must remain exact top4+nested26"), ReflectedDescriptors.Num(), 30);
	// Current descriptor와 actual Reflection positive exact comparison입니다.
	const FCFDAContractGuardResult PositiveResult = FCFDAContractGuard::ValidateSourceShapeCoverage(FCFDAContractGuard::GetSourceShapeDescriptor(), ReflectedDescriptors);
	TestTrue(TEXT("Current source descriptor must exactly match native Reflection"), PositiveResult.bPassed);

	// 가상 reflected field 추가 fixture입니다.
	TArray<FCFDASourceFieldDescriptor> AddedFieldFixture = ReflectedDescriptors;
	// Current root를 따르는 가상 authored field입니다.
	FCFDASourceFieldDescriptor AddedField = ReflectedDescriptors[0];
	AddedField.SourcePropertyPath = TEXT("DACE_VirtualAddedField");
	AddedField.PropertyKind = TEXT("Float");
	AddedField.ReflectedTypePath.Reset();
	AddedField.ContainerKind = TEXT("Scalar");
	AddedField.NestedStructPath.Reset();
	AddedFieldFixture.Add(AddedField);
	// Added field가 fail-closed되는 결과입니다.
	const FCFDAContractGuardResult AddedResult = FCFDAContractGuard::ValidateSourceShapeCoverage(FCFDAContractGuard::GetSourceShapeDescriptor(), AddedFieldFixture);
	TestFalse(TEXT("Virtual source field addition must fail closed"), AddedResult.bPassed);
	TestTrue(TEXT("Virtual source field addition must report SourceAuthoringContractDrift"), FCFDAContractGuard::HasIssueCode(AddedResult, ECFDAContractIssueCode::SourceAuthoringContractDrift));

	// 가상 reflected field 제거 fixture입니다.
	TArray<FCFDASourceFieldDescriptor> RemovedFieldFixture = ReflectedDescriptors;
	RemovedFieldFixture.RemoveAt(RemovedFieldFixture.Num() - 1);
	// Removed field가 fail-closed되는 결과입니다.
	const FCFDAContractGuardResult RemovedResult = FCFDAContractGuard::ValidateSourceShapeCoverage(FCFDAContractGuard::GetSourceShapeDescriptor(), RemovedFieldFixture);
	TestFalse(TEXT("Virtual source field removal must fail closed"), RemovedResult.bPassed);

	// 가상 reflected rename/path mismatch fixture입니다.
	TArray<FCFDASourceFieldDescriptor> RenamedFieldFixture = ReflectedDescriptors;
	RenamedFieldFixture[0].SourcePropertyPath += TEXT("_Renamed");
	// Rename가 fail-closed되는 결과입니다.
	const FCFDAContractGuardResult RenamedResult = FCFDAContractGuard::ValidateSourceShapeCoverage(FCFDAContractGuard::GetSourceShapeDescriptor(), RenamedFieldFixture);
	TestFalse(TEXT("Virtual source field rename must fail closed"), RenamedResult.bPassed);

	// 가상 reflected property type mismatch fixture입니다.
	TArray<FCFDASourceFieldDescriptor> TypeMismatchFixture = ReflectedDescriptors;
	TypeMismatchFixture[0].PropertyKind = TEXT("Float");
	// Type mismatch가 fail-closed되는 결과입니다.
	const FCFDAContractGuardResult TypeMismatchResult = FCFDAContractGuard::ValidateSourceShapeCoverage(FCFDAContractGuard::GetSourceShapeDescriptor(), TypeMismatchFixture);
	TestFalse(TEXT("Virtual source property type mismatch must fail closed"), TypeMismatchResult.bPassed);

	// Outer Map container shape는 같고 recursive key/value inner type token만 다른 future-contract fixture입니다.
	TArray<FCFDASourceFieldDescriptor> ExpectedContainerFixture = ReflectedDescriptors;
	// Future accepted descriptor를 흉내 내는 recursive container field입니다.
	FCFDASourceFieldDescriptor ExpectedContainerField = ReflectedDescriptors[0];
	ExpectedContainerField.SourcePropertyPath = TEXT("DACE_VirtualContainerField");
	ExpectedContainerField.PropertyKind = TEXT("Map");
	ExpectedContainerField.ReflectedTypePath = TEXT("Map<Name,Array<Float>>");
	ExpectedContainerField.ContainerKind = TEXT("Map");
	ExpectedContainerField.NestedStructPath.Reset();
	ExpectedContainerFixture.Add(ExpectedContainerField);
	// 같은 outer Map이지만 value inner type만 바뀐 observed fixture입니다.
	TArray<FCFDASourceFieldDescriptor> ObservedContainerFixture = ExpectedContainerFixture;
	ObservedContainerFixture.Last().ReflectedTypePath = TEXT("Map<Name,Array<Name>>");
	// Recursive container inner type drift 결과입니다.
	const FCFDAContractGuardResult ContainerInnerTypeDriftResult = FCFDAContractGuard::ValidateSourceShapeCoverage(ExpectedContainerFixture, ObservedContainerFixture);
	TestFalse(TEXT("Virtual recursive container inner type drift must fail closed"), ContainerInnerTypeDriftResult.bPassed);

	// Nested struct ownership/path drift fixture입니다.
	TArray<FCFDASourceFieldDescriptor> NestedDriftFixture = ReflectedDescriptors;
	for (FCFDASourceFieldDescriptor& Descriptor : NestedDriftFixture)
	{
		if (Descriptor.SourcePropertyPath.Equals(TEXT("MissileGuideConfig.GuidanceLaw"), ESearchCase::CaseSensitive))
		{
			Descriptor.NestedStructPath = TEXT("/Script/CarFight_Re.DACEWrongNestedStruct");
			break;
		}
	}
	// Nested struct drift가 fail-closed되는 결과입니다.
	const FCFDAContractGuardResult NestedDriftResult = FCFDAContractGuard::ValidateSourceShapeCoverage(FCFDAContractGuard::GetSourceShapeDescriptor(), NestedDriftFixture);
	TestFalse(TEXT("Virtual nested struct drift must fail closed"), NestedDriftResult.bPassed);
	return true;
}

// Adapter descriptor 자체 drift와 mapping source/adapter-side 누락·중복을 fail-closed 검증합니다.
bool FCFDAAdapterMappingDriftTest::RunTest(const FString& Parameters)
{
	// Current Adapter exact self-comparison result입니다.
	const FCFDAContractGuardResult AdapterPositiveResult = FCFDAContractGuard::ValidateAdapterShapeCoverage(FCFDAContractGuard::GetAdapterShapeDescriptor(), FCFDAContractGuard::GetAdapterShapeDescriptor());
	TestTrue(TEXT("Current Adapter descriptor exact self baseline must pass"), AdapterPositiveResult.bPassed);

	// Adapter field type drift fixture입니다.
	TArray<FCFDAAdapterFieldDescriptor> AdapterTypeDriftFixture = FCFDAContractGuard::GetAdapterShapeDescriptor();
	for (FCFDAAdapterFieldDescriptor& Descriptor : AdapterTypeDriftFixture)
	{
		if (Descriptor.AdapterJsonPath.Equals(TEXT("Payload.MissileGuideConfig.GuideMode"), ESearchCase::CaseSensitive))
		{
			Descriptor.JsonValueKind = TEXT("Number");
			break;
		}
	}
	// Adapter type drift 결과입니다.
	const FCFDAContractGuardResult AdapterTypeDriftResult = FCFDAContractGuard::ValidateAdapterShapeCoverage(FCFDAContractGuard::GetAdapterShapeDescriptor(), AdapterTypeDriftFixture);
	TestFalse(TEXT("Adapter field type drift must fail closed"), AdapterTypeDriftResult.bPassed);
	TestTrue(TEXT("Adapter field type drift must report AdapterShapeDrift"), FCFDAContractGuard::HasIssueCode(AdapterTypeDriftResult, ECFDAContractIssueCode::AdapterShapeDrift));

	// Current mapping 양방향 exact coverage입니다.
	const FCFDAContractGuardResult MappingPositiveResult = FCFDAContractGuard::ValidateSourceAdapterMappingCoverage(FCFDAContractGuard::GetSourceShapeDescriptor(), FCFDAContractGuard::GetAdapterShapeDescriptor(), FCFDAContractGuard::GetSourceAdapterMappingDescriptor());
	TestTrue(TEXT("Current Source↔Adapter mapping coverage must pass"), MappingPositiveResult.bPassed);

	// Source-side unique mapping 누락 fixture입니다.
	TArray<FCFDASourceAdapterMappingDescriptor> SourceMissingFixture = FCFDAContractGuard::GetSourceAdapterMappingDescriptor();
	SourceMissingFixture.RemoveAll([](const FCFDASourceAdapterMappingDescriptor& Descriptor)
	{
		return Descriptor.AdapterJsonPath.Equals(TEXT("Payload.MissileGuideConfig.GuideMode"), ESearchCase::CaseSensitive);
	});
	// Source-side missing mapping 결과입니다.
	const FCFDAContractGuardResult SourceMissingResult = FCFDAContractGuard::ValidateSourceAdapterMappingCoverage(FCFDAContractGuard::GetSourceShapeDescriptor(), FCFDAContractGuard::GetAdapterShapeDescriptor(), SourceMissingFixture);
	TestFalse(TEXT("Mapping source-side omission must fail closed"), SourceMissingResult.bPassed);
	TestTrue(TEXT("Mapping source-side omission must report SourceAdapterMappingDrift"), FCFDAContractGuard::HasIssueCode(SourceMissingResult, ECFDAContractIssueCode::SourceAdapterMappingDrift));

	// Adapter-only metadata mapping 누락 fixture입니다.
	TArray<FCFDASourceAdapterMappingDescriptor> AdapterMissingFixture = FCFDAContractGuard::GetSourceAdapterMappingDescriptor();
	AdapterMissingFixture.RemoveAll([](const FCFDASourceAdapterMappingDescriptor& Descriptor)
	{
		return Descriptor.AdapterJsonPath.Equals(TEXT("SchemaId"), ESearchCase::CaseSensitive);
	});
	// Adapter-side missing mapping 결과입니다.
	const FCFDAContractGuardResult AdapterMissingResult = FCFDAContractGuard::ValidateSourceAdapterMappingCoverage(FCFDAContractGuard::GetSourceShapeDescriptor(), FCFDAContractGuard::GetAdapterShapeDescriptor(), AdapterMissingFixture);
	TestFalse(TEXT("Mapping adapter-side omission must fail closed"), AdapterMissingResult.bPassed);

	// Duplicate AdapterJsonPath mapping fixture입니다.
	TArray<FCFDASourceAdapterMappingDescriptor> DuplicateAdapterFixture = FCFDAContractGuard::GetSourceAdapterMappingDescriptor();
	// TArray reallocation 중 self-alias를 피하기 위해 값 복사한 duplicate row입니다.
	const FCFDASourceAdapterMappingDescriptor DuplicateMappingRow = DuplicateAdapterFixture[0];
	DuplicateAdapterFixture.Add(DuplicateMappingRow);
	// Duplicate mapping 결과입니다.
	const FCFDAContractGuardResult DuplicateAdapterResult = FCFDAContractGuard::ValidateSourceAdapterMappingCoverage(FCFDAContractGuard::GetSourceShapeDescriptor(), FCFDAContractGuard::GetAdapterShapeDescriptor(), DuplicateAdapterFixture);
	TestFalse(TEXT("Duplicate adapter-side mapping must fail closed"), DuplicateAdapterResult.bPassed);
	return true;
}

// Actual production serializer shape와 strict parser required/type/unknown field negative matrix를 검증합니다.
bool FCFDASerializerParserDriftTest::RunTest(const FString& Parameters)
{
	using namespace CFDAContractDriftTestsPrivate;
	// Production fixture typed payload입니다.
	FCFDAMissilePresetPayload Payload;
	// Production fixture semantic fingerprint입니다.
	FString Fingerprint;
	// Production serializer actual JSON입니다.
	FString SerializedJson;
	// Fixture 준비 실패 사유입니다.
	FString FixtureError;
	TestTrue(TEXT("Production serialized fixture must build"), BuildProductionSerializedFixture(Payload, Fingerprint, SerializedJson, FixtureError));
	if (SerializedJson.IsEmpty()) return false;

	// Actual production serializer output vs Adapter physical descriptor입니다.
	const FCFDAContractGuardResult SerializerPositiveResult = FCFDAContractGuard::ValidateSerializedAdapterCoverage(SerializedJson);
	TestTrue(TEXT("Actual production serializer JSON shape must exactly match Adapter descriptor"), SerializerPositiveResult.bPassed);

	// SourceToAdapter mapping value wiring을 검증할 distinguishable per-leaf mutations입니다.
	const TArray<FPayloadMutation> SerializerMutations = BuildPayloadMutations();
	// Mapping descriptor가 선언한 unique SourceToAdapter leaf set입니다.
	const TSet<FString> MappedSourceLeafPaths = BuildMappedSourceLeafPathSet();
	// Serializer sentinel matrix가 실제로 덮는 unique Source leaf set입니다.
	const TSet<FString> SerializerMutationPaths = BuildMutationSourcePathSet(SerializerMutations);
	TestEqual(TEXT("Serializer sentinel SourcePropertyPath entries must be unique"), SerializerMutationPaths.Num(), SerializerMutations.Num());
	TestTrue(TEXT("Serializer sentinel matrix must exactly cover mapping descriptor SourceToAdapter leaf set"), AreStringSetsEqual(MappedSourceLeafPaths, SerializerMutationPaths));
	for (const FPayloadMutation& Mutation : SerializerMutations)
	{
		// Baseline에서 mapping Source leaf 하나만 distinguishable value로 바꾼 payload입니다.
		FCFDAMissilePresetPayload MutatedPayload = Payload;
		Mutation.Apply(MutatedPayload);
		// Serializer 이전 expected semantic fingerprint입니다.
		FString ExpectedMutatedFingerprint;
		// Expected fingerprint 생성 실패 사유입니다.
		FString ExpectedFingerprintError;
		const bool bExpectedFingerprintBuilt = FCFDAStagingService::BuildSemanticFingerprint(MutatedPayload, ExpectedMutatedFingerprint, ExpectedFingerprintError);
		TestTrue(*FString::Printf(TEXT("Serializer sentinel expected fingerprint must build: %s"), *Mutation.SourcePropertyPath), bExpectedFingerprintBuilt);
		if (!bExpectedFingerprintBuilt) continue;
		TestTrue(*FString::Printf(TEXT("Serializer sentinel must differ from baseline: %s"), *Mutation.SourcePropertyPath), !ExpectedMutatedFingerprint.Equals(Fingerprint, ESearchCase::CaseSensitive));
		// Actual production serializer가 작성한 memory JSON입니다.
		FString MutatedSerializedJson;
		// Serializer probe failure 사유입니다.
		FString SerializerSentinelError;
		const bool bSerialized = CFDAContractProbeSerialize(MutatedPayload, ProbeTargetObjectPath, Fingerprint, MutatedSerializedJson, SerializerSentinelError);
		TestTrue(*FString::Printf(TEXT("Production serializer sentinel must serialize: %s"), *Mutation.SourcePropertyPath), bSerialized);
		if (!bSerialized) continue;
		// Production parser가 serializer output을 다시 읽은 결과입니다.
		const FCFDAStagingParseResult SentinelParse = CFDAContractProbeParse(MutatedSerializedJson, ProbeStagingPath);
		TestTrue(*FString::Printf(TEXT("Production parser must accept serializer sentinel: %s"), *Mutation.SourcePropertyPath), SentinelParse.bValid);
		if (!SentinelParse.bValid) continue;
		TestEqual(*FString::Printf(TEXT("Serializer Source→JSON→parser semantic wiring must preserve exact leaf: %s"), *Mutation.SourcePropertyPath), SentinelParse.Record.StagingSemanticFingerprint, ExpectedMutatedFingerprint);
	}

	// Serializer field omission memory fixture입니다.
	FString SerializerMissingJson;
	TestTrue(TEXT("Serializer omission fixture must build"), BuildJsonWithRemovedField(SerializedJson, TEXT("Payload.MissileGuideConfig.GuideMode"), SerializerMissingJson));
	// Serializer omission validator 결과입니다.
	const FCFDAContractGuardResult SerializerMissingResult = FCFDAContractGuard::ValidateSerializedAdapterCoverage(SerializerMissingJson);
	TestFalse(TEXT("Serializer field omission must fail closed"), SerializerMissingResult.bPassed);
	TestTrue(TEXT("Serializer omission must report SerializerCoverageMismatch"), FCFDAContractGuard::HasIssueCode(SerializerMissingResult, ECFDAContractIssueCode::SerializerCoverageMismatch));

	// Serializer wrong-type memory fixture descriptor입니다.
	const FCFDAAdapterFieldDescriptor* GuideModeDescriptor = nullptr;
	for (const FCFDAAdapterFieldDescriptor& Descriptor : FCFDAContractGuard::GetAdapterShapeDescriptor())
	{
		if (Descriptor.AdapterJsonPath.Equals(TEXT("Payload.MissileGuideConfig.GuideMode"), ESearchCase::CaseSensitive))
		{
			GuideModeDescriptor = &Descriptor;
			break;
		}
	}
	TestNotNull(TEXT("GuideMode Adapter descriptor must exist"), GuideModeDescriptor);
	FString SerializerWrongTypeJson;
	if (GuideModeDescriptor != nullptr)
	{
		TestTrue(TEXT("Serializer wrong-type fixture must build"), BuildJsonWithWrongType(SerializedJson, *GuideModeDescriptor, SerializerWrongTypeJson));
		// Serializer wrong type validator 결과입니다.
		const FCFDAContractGuardResult SerializerWrongTypeResult = FCFDAContractGuard::ValidateSerializedAdapterCoverage(SerializerWrongTypeJson);
		TestFalse(TEXT("Serializer field type mismatch must fail closed"), SerializerWrongTypeResult.bPassed);
	}

	// Serializer unknown-field memory fixture입니다.
	FString SerializerUnknownJson;
	TestTrue(TEXT("Serializer unknown fixture must build"), BuildJsonWithUnknownField(SerializedJson, TEXT("Payload.MissileGuideConfig"), SerializerUnknownJson));
	// Serializer unknown field validator 결과입니다.
	const FCFDAContractGuardResult SerializerUnknownResult = FCFDAContractGuard::ValidateSerializedAdapterCoverage(SerializerUnknownJson);
	TestFalse(TEXT("Serializer unknown field must fail closed"), SerializerUnknownResult.bPassed);

	// Adapter descriptor의 모든 required physical field를 하나씩 제거해 production parser가 MissingRequiredField로 차단하는지 검증합니다.
	for (const FCFDAAdapterFieldDescriptor& Descriptor : FCFDAContractGuard::GetAdapterShapeDescriptor())
	{
		// 한 required field만 제거한 fresh memory JSON입니다.
		FString MissingFieldJson;
		const bool bFixtureBuilt = BuildJsonWithRemovedField(SerializedJson, Descriptor.AdapterJsonPath, MissingFieldJson);
		TestTrue(*FString::Printf(TEXT("Parser missing fixture must build: %s"), *Descriptor.AdapterJsonPath), bFixtureBuilt);
		if (!bFixtureBuilt) continue;
		// Production strict parser missing-field 결과입니다.
		const FCFDAStagingParseResult MissingParse = CFDAContractProbeParse(MissingFieldJson, ProbeStagingPath);
		TestFalse(*FString::Printf(TEXT("Parser must reject missing required field: %s"), *Descriptor.AdapterJsonPath), MissingParse.bValid);
		TestTrue(*FString::Printf(TEXT("Parser missing field must report MissingRequiredField at exact path: %s"), *Descriptor.AdapterJsonPath), HasStagingIssueAtExactPath(MissingParse.Issues, ECFDAStagingIssueCode::MissingRequiredField, Descriptor.AdapterJsonPath));
	}

	// Adapter descriptor의 모든 field를 incompatible JSON type으로 바꿔 production parser가 TypeMismatch로 차단하는지 검증합니다.
	for (const FCFDAAdapterFieldDescriptor& Descriptor : FCFDAContractGuard::GetAdapterShapeDescriptor())
	{
		// 한 field type만 바꾼 fresh memory JSON입니다.
		FString WrongTypeJson;
		const bool bFixtureBuilt = BuildJsonWithWrongType(SerializedJson, Descriptor, WrongTypeJson);
		TestTrue(*FString::Printf(TEXT("Parser wrong-type fixture must build: %s"), *Descriptor.AdapterJsonPath), bFixtureBuilt);
		if (!bFixtureBuilt) continue;
		// Production strict parser type-mismatch 결과입니다.
		const FCFDAStagingParseResult WrongTypeParse = CFDAContractProbeParse(WrongTypeJson, ProbeStagingPath);
		TestFalse(*FString::Printf(TEXT("Parser must reject wrong JSON type: %s"), *Descriptor.AdapterJsonPath), WrongTypeParse.bValid);
		TestTrue(*FString::Printf(TEXT("Parser wrong type must report TypeMismatch at exact path: %s"), *Descriptor.AdapterJsonPath), HasStagingIssueAtExactPath(WrongTypeParse.Issues, ECFDAStagingIssueCode::TypeMismatch, Descriptor.AdapterJsonPath));
	}

	// Exact strict object boundaries에서 unknown sibling을 차단할 object path 목록입니다.
	const TArray<FString> UnknownObjectPaths =
	{
		FString(),
		TEXT("Payload"),
		TEXT("Payload.PresetDisplayName"),
		TEXT("Payload.PresetDescription"),
		TEXT("Payload.MissileGuideConfig")
	};
	for (const FString& ObjectPath : UnknownObjectPaths)
	{
		// 한 strict object에 unknown sibling을 추가한 fresh memory JSON입니다.
		FString UnknownFieldJson;
		const bool bFixtureBuilt = BuildJsonWithUnknownField(SerializedJson, ObjectPath, UnknownFieldJson);
		TestTrue(*FString::Printf(TEXT("Parser unknown-field fixture must build: %s"), ObjectPath.IsEmpty() ? TEXT("<root>") : *ObjectPath), bFixtureBuilt);
		if (!bFixtureBuilt) continue;
		// Production strict parser unknown-field 결과입니다.
		const FCFDAStagingParseResult UnknownParse = CFDAContractProbeParse(UnknownFieldJson, ProbeStagingPath);
		TestFalse(*FString::Printf(TEXT("Parser must reject unknown sibling: %s"), ObjectPath.IsEmpty() ? TEXT("<root>") : *ObjectPath), UnknownParse.bValid);
		// Unknown sibling의 exact expected diagnostic path입니다.
		const FString ExpectedUnknownPath = ObjectPath.IsEmpty() ? TEXT("__DACE_UnknownField") : ObjectPath + TEXT(".__DACE_UnknownField");
		TestTrue(*FString::Printf(TEXT("Parser unknown sibling must report UnknownField at exact path: %s"), ObjectPath.IsEmpty() ? TEXT("<root>") : *ObjectPath), HasStagingIssueAtExactPath(UnknownParse.Issues, ECFDAStagingIssueCode::UnknownField, ExpectedUnknownPath));
	}
	return true;
}

// Actual production fingerprint token sequence PASS와 missing/add/order negative fixture를 fail-closed 검증합니다.
bool FCFDAFingerprintDriftTest::RunTest(const FString& Parameters)
{
	using namespace CFDAContractDriftTestsPrivate;
	// Production parser baseline payload입니다.
	const FCFDAStagingParseResult Parsed = CFDAContractProbeParse(BuildProbeJson(), ProbeStagingPath);
	TestTrue(TEXT("Fingerprint baseline must parse"), Parsed.bValid);
	if (!Parsed.bValid) return false;
	// Actual production fingerprint입니다.
	FString Fingerprint;
	// Actual production emitted token labels입니다.
	TArray<FString> ObservedLabels;
	// Fingerprint probe failure 사유입니다.
	FString FingerprintError;
	TestTrue(TEXT("Production fingerprint probe must succeed"), CFDAContractProbeFingerprint(Parsed.Record.Payload, Fingerprint, ObservedLabels, FingerprintError));
	TestEqual(TEXT("Fingerprint contract token descriptor must remain exact33"), FCFDAContractGuard::GetFingerprintTokenDescriptor().Num(), 33);
	// Actual production token coverage 결과입니다.
	const FCFDAContractGuardResult PositiveResult = FCFDAContractGuard::ValidateFingerprintCoverage(ObservedLabels);
	TestTrue(TEXT("Actual production fingerprint token labels/order must match descriptor"), PositiveResult.bPassed);

	// Token 누락 fixture입니다.
	TArray<FString> MissingTokenFixture = ObservedLabels;
	MissingTokenFixture.RemoveAt(MissingTokenFixture.Num() - 1);
	// Missing token validation 결과입니다.
	const FCFDAContractGuardResult MissingResult = FCFDAContractGuard::ValidateFingerprintCoverage(MissingTokenFixture);
	TestFalse(TEXT("Fingerprint token omission must fail closed"), MissingResult.bPassed);
	TestTrue(TEXT("Fingerprint token omission must report FingerprintCoverageMismatch"), FCFDAContractGuard::HasIssueCode(MissingResult, ECFDAContractIssueCode::FingerprintCoverageMismatch));

	// Unknown token 추가 fixture입니다.
	TArray<FString> AddedTokenFixture = ObservedLabels;
	AddedTokenFixture.Add(TEXT("Config.__DACE_UnknownToken"));
	// Added token validation 결과입니다.
	const FCFDAContractGuardResult AddedResult = FCFDAContractGuard::ValidateFingerprintCoverage(AddedTokenFixture);
	TestFalse(TEXT("Fingerprint unknown token addition must fail closed"), AddedResult.bPassed);

	// Token ordering drift fixture입니다.
	TArray<FString> ReorderedTokenFixture = ObservedLabels;
	if (ReorderedTokenFixture.Num() > 8) ReorderedTokenFixture.Swap(7, 8);
	// Reordered token validation 결과입니다.
	const FCFDAContractGuardResult ReorderedResult = FCFDAContractGuard::ValidateFingerprintCoverage(ReorderedTokenFixture);
	TestFalse(TEXT("Fingerprint token ordering drift must fail closed"), ReorderedResult.bPassed);
	return true;
}

// Source semantic leaf exact29 각각의 sentinel이 production materializer→extractor roundtrip에서 보존되는지 검증합니다.
bool FCFDAMaterializerExtractorDriftTest::RunTest(const FString& Parameters)
{
	using namespace CFDAContractDriftTestsPrivate;
	// Production parser baseline payload입니다.
	const FCFDAStagingParseResult Parsed = CFDAContractProbeParse(BuildProbeJson(), ProbeStagingPath);
	TestTrue(TEXT("Materializer baseline must parse"), Parsed.bValid);
	if (!Parsed.bValid) return false;
	// Baseline payload입니다.
	const FCFDAMissilePresetPayload BaselinePayload = Parsed.Record.Payload;
	// Baseline semantic fingerprint입니다.
	FString BaselineFingerprint;
	// Baseline fingerprint failure 사유입니다.
	FString BaselineFingerprintError;
	TestTrue(TEXT("Materializer baseline must fingerprint"), FCFDAStagingService::BuildSemanticFingerprint(BaselinePayload, BaselineFingerprint, BaselineFingerprintError));

	// Extractor를 materializer와 독립적으로 관측할 transient source asset입니다.
	UCFMissileGuidePresetData* ExtractorSourceAsset = NewObject<UCFMissileGuidePresetData>(GetTransientPackage(), NAME_None, RF_Transient);
	TestNotNull(TEXT("Extractor transient source asset must be created"), ExtractorSourceAsset);
	if (ExtractorSourceAsset != nullptr)
	{
		ExtractorSourceAsset->PresetId = BaselinePayload.PresetId;
		ExtractorSourceAsset->PresetDisplayName = FText::FromString(BaselinePayload.PresetDisplayName.Text);
		ExtractorSourceAsset->PresetDescription = FText::FromString(BaselinePayload.PresetDescription.Text);
		ExtractorSourceAsset->MissileGuideConfig = BaselinePayload.MissileGuideConfig;
		// Production extractor direct readback입니다.
		FCFDAMissilePresetPayload ExtractedPayload;
		// Production extractor diagnostics입니다.
		TArray<FCFDAStagingIssue> ExtractIssues;
		TestTrue(TEXT("Production extractor direct transient probe must succeed"), CFDAContractProbeExtract(*ExtractorSourceAsset, ExtractedPayload, ExtractIssues));
		// Direct extractor semantic coverage result입니다.
		const FCFDAContractGuardResult ExtractorCoverageResult = FCFDAContractGuard::ValidateMaterializerRoundTripCoverage(BaselinePayload, ExtractedPayload);
		TestTrue(TEXT("Production extractor direct readback must preserve full semantic payload"), ExtractorCoverageResult.bPassed);
	}

	// Source descriptor의 semantic leaf exact set과 결속할 distinguishable authored leaf mutations입니다.
	const TArray<FPayloadMutation> Mutations = BuildPayloadMutations();
	// SourceShape descriptor가 선언한 current semantic leaf set입니다.
	const TSet<FString> SourceSemanticLeafPaths = BuildSourceSemanticLeafPathSet();
	// Materializer/extractor sentinel matrix가 실제로 덮는 unique Source paths입니다.
	const TSet<FString> MutationSourcePaths = BuildMutationSourcePathSet(Mutations);
	TestEqual(TEXT("Current Source descriptor semantic leaf baseline must remain exact29"), SourceSemanticLeafPaths.Num(), 29);
	TestEqual(TEXT("Materializer/extractor sentinel SourcePropertyPath entries must be unique"), MutationSourcePaths.Num(), Mutations.Num());
	TestTrue(TEXT("Materializer/extractor sentinel matrix must exactly match Source descriptor semantic leaf set"), AreStringSetsEqual(SourceSemanticLeafPaths, MutationSourcePaths));
	for (const FPayloadMutation& Mutation : Mutations)
	{
		// Baseline에서 한 authored leaf만 바꾼 expected payload입니다.
		FCFDAMissilePresetPayload MutatedPayload = BaselinePayload;
		Mutation.Apply(MutatedPayload);
		// 한 leaf mutation의 expected semantic fingerprint입니다.
		FString MutatedFingerprint;
		// 한 leaf mutation fingerprint failure 사유입니다.
		FString MutatedFingerprintError;
		const bool bFingerprintBuilt = FCFDAStagingService::BuildSemanticFingerprint(MutatedPayload, MutatedFingerprint, MutatedFingerprintError);
		TestTrue(*FString::Printf(TEXT("Sentinel mutation must fingerprint: %s"), *Mutation.SourcePropertyPath), bFingerprintBuilt);
		if (!bFingerprintBuilt) continue;
		TestTrue(*FString::Printf(TEXT("Sentinel mutation must change semantic fingerprint: %s"), *Mutation.SourcePropertyPath), !MutatedFingerprint.Equals(BaselineFingerprint, ESearchCase::CaseSensitive));
		// Production materializer→extractor actual readback입니다.
		FCFDAMissilePresetPayload ReadbackPayload;
		// Production materializer→extractor failure 사유입니다.
		FString RoundTripError;
		const bool bRoundTripSucceeded = CFDAContractProbeMaterializeRoundTrip(MutatedPayload, ReadbackPayload, RoundTripError);
		TestTrue(*FString::Printf(TEXT("Production materializer→extractor sentinel roundtrip must succeed: %s"), *Mutation.SourcePropertyPath), bRoundTripSucceeded);
		if (!bRoundTripSucceeded) continue;
		// Expected vs actual full semantic roundtrip coverage 결과입니다.
		const FCFDAContractGuardResult CoverageResult = FCFDAContractGuard::ValidateMaterializerRoundTripCoverage(MutatedPayload, ReadbackPayload);
		TestTrue(*FString::Printf(TEXT("Materializer→extractor must preserve sentinel leaf: %s"), *Mutation.SourcePropertyPath), CoverageResult.bPassed);
	}

	// Synthetic readback corruption negative fixture입니다.
	FCFDAMissilePresetPayload CorruptedReadback = BaselinePayload;
	CorruptedReadback.PresetId = FName(TEXT("DACE_P0_02_CorruptedReadback"));
	// Synthetic mismatch fail-closed 결과입니다.
	const FCFDAContractGuardResult MismatchResult = FCFDAContractGuard::ValidateMaterializerRoundTripCoverage(BaselinePayload, CorruptedReadback);
	TestFalse(TEXT("Materializer/extractor semantic mismatch fixture must fail closed"), MismatchResult.bPassed);
	TestTrue(TEXT("Materializer/extractor mismatch must report MaterializerCoverageMismatch"), FCFDAContractGuard::HasIssueCode(MismatchResult, ECFDAContractIssueCode::MaterializerCoverageMismatch));
	return true;
}

#endif
