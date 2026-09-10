// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAStagingTests.cpp
// Version: v1.8.0
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-01 exact TypeKey/provider-owned root/class-scoped StableLogicalId를 포함한 Missile compatibility parse/Preview/BatchPlanHash focused Automation입니다.
// Changelog:
// - v1.8.0: DAO-P0-05 affected regression에서 test-only second provider fixture가 v1.5+ DACE canonical target-set declaration 계약을 완성하도록 explicit empty exact0 declaration을 추가했습니다. Production registry/behavior는 변경하지 않습니다.
// - v1.7.0: DAO-P0-02 contract correction으로 read-only provider readiness/mutation fail-closed, common cached fingerprint integrity, AmmoIcon canonical SoftObjectPath + Asset Registry metadata-only/no-load seam을 기존 exact4 focused tests에 추가.
// - v1.6.0: test-only second provider의 exact StagingRoot isolation과 mixed-type payload-free common Reviewed approval seam을 기존 BatchPlanHash test에 추가.
// - v1.5.0: complete provider operation registry parity, test-only second-provider exact dispatch/mixed common batch seam, duplicate exact TypeKey fail-closed regression을 기존 exact test 4개 안에 추가.
// - v1.4.0: unknown exact TypeKey와 sibling provider root fail-closed, trusted Missile provider compatibility values, class-scoped canonical FName identity key 회귀를 기존 focused tests에 추가.
// - v1.3.0: current AdapterContractRevision 2 fixture로 갱신하고 revision 1 old Staging이 AdapterRevisionMismatch로 fail-closed되는 회귀를 고정.
// - v1.2.1: parse 뒤 mutable TargetObjectPath/StagingRelativePath/Create intent binding 변조가 Preview/BatchPlanHash에서 fail-closed되는 회귀 검증을 추가.
// - v1.2.0: canonical Staging root 밖 source path fail-closed와 Payload↔StagingSemanticFingerprint split DTO의 Preview/BatchPlanHash 차단 회귀 검증을 추가.
// - v1.1.2: MissileFeel 실사용 object의 process-local load 상태와 분리된 reserved-never-authored path/identity로 read-only absent resolver fixture를 고정.
// - v1.1.1: P0-02 Apply/Save 금지 current truth에 맞춰 persisted Pilot 0건을 valid absent resolver state로 검증하고 그 state를 Create Preview에 직접 연결.
// - v1.1.0: existing MissileGuidePreset read-only current resolver와 current duplicate StableLogicalId fail-closed 검증을 추가.
// - v1.0.0: Pilot schema fail-closed parse, semantic canonicalization, exact 3-way Preview, duplicate protection과 plan hash determinism을 검증.
// Migration:
// - 메모리 DTO/JSON과 existing persisted MissileGuidePreset의 read-only resolve만 사용합니다. UObject/package/Asset Registry mutation/Save를 수행하지 않습니다.
// - v1.8.0의 second-provider DACE target set은 test-only explicit empty exact0이며 production provider registry나 accepted history를 등록/변경하지 않습니다.

#include "DataAuthoring/CFDAStaging.h"
#include "DataAuthoring/CFDAStagingApply.h"
#include "CFDACommonPrimitives.h"
#include "CFDAMissileProvider.h"
#include "CFDATypeDispatch.h"

#include "Engine/Texture2D.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDAStagingTestsPrivate
{
	// 테스트에서 사용하는 canonical dummy SHA-256 fingerprint A입니다.
	static const FString FingerprintA = TEXT("sha256:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

	// 테스트에서 사용하는 canonical dummy SHA-256 fingerprint B입니다.
	static const FString FingerprintB = TEXT("sha256:bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");

	// 테스트에서 사용하는 canonical dummy SHA-256 fingerprint C입니다.
	static const FString FingerprintC = TEXT("sha256:cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc");

	// strict Pilot whole-record JSON을 현재 frozen schema에 맞춰 생성합니다.
	FString BuildValidJson(
		const FString& StableLogicalId = TEXT("MissileFeel_Low"),
		const FString& PayloadPresetId = TEXT("MissileFeel_Low"),
		const FString& BaseFingerprintJson = TEXT("null"),
		const FString& TargetObjectPath = TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Low.DA_MissileFeel_Low"),
		const FString& NavigationConstantText = TEXT("3.0"))
	{
		return FString::Printf(
			TEXT("{")
			TEXT("\"SchemaId\":\"CarFight.DataAsset.MissileGuidePreset\",")
			TEXT("\"SchemaRevision\":1,")
			TEXT("\"AdapterContractRevision\":2,")
			TEXT("\"DataAssetTypeClassPath\":\"/Script/CarFight_Re.CFMissileGuidePresetData\",")
			TEXT("\"StableLogicalId\":\"%s\",")
			TEXT("\"TargetObjectPath\":\"%s\",")
			TEXT("\"BaseSemanticFingerprint\":%s,")
			TEXT("\"Payload\":{")
			TEXT("\"PresetId\":\"%s\",")
			TEXT("\"PresetDisplayName\":{\"Kind\":\"Literal\",\"Text\":\"저성능 단순 추적\"},")
			TEXT("\"PresetDescription\":{\"Kind\":\"Literal\",\"Text\":\"테스트 프리셋\"},")
			TEXT("\"MissileGuideConfig\":{")
			TEXT("\"bUseGuidance\":true,")
			TEXT("\"GuideMode\":\"TargetActor\",")
			TEXT("\"LostTargetPolicy\":\"ContinueStraight\",")
			TEXT("\"NavigationConstant\":%s,")
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
			*BaseFingerprintJson,
			*PayloadPresetId,
			*NavigationConstantText);
	}

	// top-level key physical order와 whitespace가 다른 동일 semantic JSON을 생성합니다.
	FString BuildReorderedValidJson()
	{
		return TEXT(R"JSON(
{
  "Payload": {
    "MissileGuideConfig": {
      "ReacquisitionTimeSeconds": 0,
      "ReacquisitionConeHalfAngleDeg": 60,
      "TrackingConeHalfAngleDeg": 60,
      "AcquisitionConeHalfAngleDeg": 45,
      "TargetVelocityEstimateResponseTimeSeconds": 0.25,
      "TargetObservationIntervalSeconds": 0.08,
      "ReacquisitionMode": "None",
      "MaxLeadDistanceCm": 0,
      "LeadTimeSeconds": 0,
      "GuidanceActivationDistanceCm": 500,
      "GuidanceActivationDelaySeconds": 0.25,
      "GuidanceActivationMode": "Independent",
      "GuidanceLaw": "PurePursuit",
      "TargetObservationMode": "SampledPositionEstimate",
      "SeekerModel": "Stateful",
      "TargetLostGraceTimeSeconds": 0.2,
      "LockBreakAngleDeg": 85,
      "SeekerFieldOfViewDeg": 60,
      "MinimumGuidanceSpeedCmPerSec": 500,
      "GuidanceResponseTimeSeconds": 0.18,
      "MaximumLateralAccelerationCmPerSecSq": 2000,
      "MaximumTurnRateDegPerSec": 35,
      "NavigationConstant": 3,
      "LostTargetPolicy": "ContinueStraight",
      "GuideMode": "TargetActor",
      "bUseGuidance": true
    },
    "PresetDescription": {"Text":"테스트 프리셋","Kind":"Literal"},
    "PresetDisplayName": {"Text":"저성능 단순 추적","Kind":"Literal"},
    "PresetId": "MissileFeel_Low"
  },
  "BaseSemanticFingerprint": null,
  "TargetObjectPath": "/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Low.DA_MissileFeel_Low",
  "StableLogicalId": "MissileFeel_Low",
  "DataAssetTypeClassPath": "/Script/CarFight_Re.CFMissileGuidePresetData",
  "AdapterContractRevision": 2,
  "SchemaRevision": 1,
  "SchemaId": "CarFight.DataAsset.MissileGuidePreset"
}
)JSON");
	}

	// valid parse result를 test assertion과 함께 반환합니다.
	FCFDAStagingParseResult ParseValidRecord(FAutomationTestBase& Test, const FString& JsonText, const FString& StagingPath)
	{
		// strict parser가 반환한 result입니다.
		FCFDAStagingParseResult ParseResult = FCFDAStagingService::ParseMissilePresetJson(JsonText, StagingPath);
		Test.TestTrue(TEXT("Expected JSON must parse as valid whole-record"), ParseResult.bValid);
		Test.TestTrue(TEXT("Expected staging fingerprint must be canonical sha256"), ParseResult.Record.StagingSemanticFingerprint.StartsWith(TEXT("sha256:")));
		return ParseResult;
	}

	// valid record를 Update intent로 전환합니다.
	FCFDAStagingRecord MakeUpdateRecord(const FCFDAStagingRecord& SourceRecord, const FString& BaseFingerprint)
	{
		// Update intent로 복사할 typed record입니다.
		FCFDAStagingRecord UpdateRecord = SourceRecord;
		UpdateRecord.bHasBaseSemanticFingerprint = true;
		UpdateRecord.BaseSemanticFingerprint = BaseFingerprint;
		return UpdateRecord;
	}

	// target이 존재하는 정상 current truth DTO를 만듭니다.
	FCFDAStagingCurrentState MakeExistingCurrentState(
		const FCFDAStagingRecord& Record,
		const FString& CurrentFingerprint)
	{
		// Preview에 전달할 current target state입니다.
		FCFDAStagingCurrentState CurrentState;
		CurrentState.bRequestedTargetExists = true;
		CurrentState.RequestedTargetClassPath = Record.DataAssetTypeClassPath;
		CurrentState.RequestedTargetStableLogicalId = Record.StableLogicalId;
		CurrentState.CurrentSemanticFingerprint = CurrentFingerprint;
		CurrentState.bStableIdentityExists = true;
		CurrentState.StableIdentityMatchCount = 1;
		CurrentState.StableIdentityObjectPath = Record.TargetObjectPath;
		return CurrentState;
	}

	// test-only second provider의 common parse callback seam입니다. Production registry에는 등록하지 않습니다.
	bool ParseSecondProviderCandidate(
		const FString& JsonText,
		const FString& StagingRelativePath,
		FCFDACommonEnvelope& OutEnvelope,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		(void)JsonText;
		(void)StagingRelativePath;
		OutEnvelope = FCFDACommonEnvelope();
		OutIssues.Reset();
		return false;
	}

	// test-only second provider의 current resolver callback seam입니다. Production current truth를 조회하지 않습니다.
	bool ResolveSecondProviderCurrent(
		const FCFDACommonEnvelope& Envelope,
		FCFDACommonCurrentState& OutCurrentState,
		TArray<FCFDAStagingIssue>& OutIssues)
	{
		(void)Envelope;
		OutCurrentState = FCFDACommonCurrentState();
		OutIssues.Reset();
		return true;
	}

	// Production registry를 수정하지 않고 exact second-provider seam을 검증할 complete provider entry를 만듭니다.
	FCFDATypeProviderEntry BuildSecondProviderEntry()
	{
		// 반환할 test-only complete provider entry입니다.
		FCFDATypeProviderEntry ProviderEntry;
		ProviderEntry.Descriptor.TypeKey.SchemaId = TEXT("CarFight.DataAsset.SecondProviderTest");
		ProviderEntry.Descriptor.TypeKey.DataAssetTypeClassPath = TEXT("/Script/CarFight_Re.CFSecondProviderTestData");
		ProviderEntry.Descriptor.SchemaRevision = 1;
		ProviderEntry.Descriptor.AdapterContractRevision = 1;
		ProviderEntry.Descriptor.CanonicalStagingRoot = TEXT("Authoring/DataAssetStaging/SecondProviderTest");
		ProviderEntry.Descriptor.StableIdentityPolicy = ECFDAStableIdentityPolicy::Required;
		ProviderEntry.Descriptor.StableIdentityResolver = ECFDAStableIdentityResolver::ExplicitFName;
		ProviderEntry.Descriptor.StableIdentitySourceName = FName(TEXT("StableId"));
		ProviderEntry.Descriptor.DaceContractOwnerName = FName(TEXT("SecondProviderTestGuard"));
		ProviderEntry.Descriptor.DaceAcceptedHistoryNamespace = TEXT("DACE-SecondProviderTest");
		// Test-only provider는 accepted DACE contract가 아직 없으므로 ContractNotReady를 명시합니다.
		ProviderEntry.Descriptor.DaceReadiness = ECFDADaceReadiness::ContractNotReady;
		// Empty exact0과 미선언 상태를 구분하기 위해 test-only canonical target set을 explicit하게 선언합니다.
		ProviderEntry.Descriptor.bDaceCanonicalStagingTargetSetDeclared = true;
		// Test-only second provider의 canonical DACE target set은 의도적으로 exact0입니다.
		ProviderEntry.Descriptor.DaceCanonicalStagingRelativePaths.Reset();
		ProviderEntry.Readiness = ECFDAProviderReadiness::ReadOnlyPreviewReady;
		ProviderEntry.Operations.ParseCommonCandidate = &ParseSecondProviderCandidate;
		ProviderEntry.Operations.ResolveCommonCurrentState = &ResolveSecondProviderCurrent;
		ProviderEntry.Operations.ApplyReviewedMutation = nullptr;
		return ProviderEntry;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAStagingParseCanonicalTest,
	"CarFight.DataManagement.CF_FQ_049.DAS_P0_02.ParseCanonical",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAStagingStrictValidationTest,
	"CarFight.DataManagement.CF_FQ_049.DAS_P0_02.StrictValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAStagingPreviewMatrixTest,
	"CarFight.DataManagement.CF_FQ_049.DAS_P0_02.PreviewMatrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAStagingBatchHashTest,
	"CarFight.DataManagement.CF_FQ_049.DAS_P0_02.BatchPlanHash",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// physical JSON representation 차이가 typed semantic fingerprint를 바꾸지 않는지 검증합니다.
bool FCFDAStagingParseCanonicalTest::RunTest(const FString& Parameters)
{
	using namespace CFDAStagingTestsPrivate;

	// 기본 physical representation의 valid record입니다.
	const FCFDAStagingParseResult FirstResult = ParseValidRecord(
		*this,
		BuildValidJson(),
		TEXT("Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Low.json"));
	// key order/whitespace/numeric spelling이 다른 동일 semantic record입니다.
	const FCFDAStagingParseResult ReorderedResult = ParseValidRecord(
		*this,
		BuildReorderedValidJson(),
		TEXT("Authoring\\DataAssetStaging\\MissileGuidePreset\\MissileFeel_Low.json"));

	TestEqual(
		TEXT("Equivalent typed semantics must have identical semantic fingerprint"),
		FirstResult.Record.StagingSemanticFingerprint,
		ReorderedResult.Record.StagingSemanticFingerprint);
	TestEqual(
		TEXT("Staging source path must normalize to repository slash form"),
		ReorderedResult.Record.StagingRelativePath,
		FString(TEXT("Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Low.json")));

	// FName display casing만 다른 typed payload입니다.
	FCFDAMissilePresetPayload CaseVariantPayload = FirstResult.Record.Payload;
	CaseVariantPayload.PresetId = FName(TEXT("missilefeel_low"));
	// case-variant typed FName fingerprint입니다.
	FString CaseVariantFingerprint;
	// case-variant fingerprint 생성 오류입니다.
	FString FingerprintError;
	TestTrue(
		TEXT("FName case-variant payload must still fingerprint"),
		FCFDAStagingService::BuildSemanticFingerprint(CaseVariantPayload, CaseVariantFingerprint, FingerprintError));
	TestEqual(
		TEXT("FName case-only variation must preserve FName semantic fingerprint"),
		CaseVariantFingerprint,
		FirstResult.Record.StagingSemanticFingerprint);
	return true;
}

// malformed/unknown/missing/type/text/enum/range/identity/revision 입력이 fail-closed되는지 검증합니다.
bool FCFDAStagingStrictValidationTest::RunTest(const FString& Parameters)
{
	using namespace CFDAStagingTestsPrivate;

	// JSON syntax가 깨진 parse result입니다.
	const FCFDAStagingParseResult Malformed = FCFDAStagingService::ParseMissilePresetJson(TEXT("{not-json"));
	TestFalse(TEXT("Malformed JSON must be invalid"), Malformed.bValid);
	TestTrue(TEXT("Malformed JSON diagnostic"), FCFDAStagingService::HasIssueCode(Malformed.Issues, ECFDAStagingIssueCode::MalformedJson));

	// unknown top-level field가 추가된 JSON입니다.
	FString UnknownFieldJson = BuildValidJson();
	UnknownFieldJson.ReplaceInline(TEXT("\"SchemaId\":"), TEXT("\"Unexpected\":1,\"SchemaId\":"));
	// unknown-field parse result입니다.
	const FCFDAStagingParseResult UnknownField = FCFDAStagingService::ParseMissilePresetJson(UnknownFieldJson);
	TestFalse(TEXT("Unknown field must be invalid"), UnknownField.bValid);
	TestTrue(TEXT("Unknown field diagnostic"), FCFDAStagingService::HasIssueCode(UnknownField.Issues, ECFDAStagingIssueCode::UnknownField));

	// required nested config field를 제거한 JSON입니다.
	FString MissingFieldJson = BuildValidJson();
	MissingFieldJson.ReplaceInline(TEXT("\"ReacquisitionTimeSeconds\":0.0"), TEXT("\"UnexpectedNestedField\":0.0"));
	// missing-field parse result입니다.
	const FCFDAStagingParseResult MissingField = FCFDAStagingService::ParseMissilePresetJson(MissingFieldJson);
	TestFalse(TEXT("Missing required nested field must be invalid"), MissingField.bValid);
	TestTrue(TEXT("Missing field diagnostic"), FCFDAStagingService::HasIssueCode(MissingField.Issues, ECFDAStagingIssueCode::MissingRequiredField));
	TestTrue(TEXT("Replaced nested field must also be unknown"), FCFDAStagingService::HasIssueCode(MissingField.Issues, ECFDAStagingIssueCode::UnknownField));

	// unsupported FText representation을 넣은 JSON입니다.
	FString UnsupportedTextJson = BuildValidJson();
	UnsupportedTextJson.ReplaceInline(TEXT("\"Kind\":\"Literal\",\"Text\":\"저성능 단순 추적\""), TEXT("\"Kind\":\"StringTable\",\"Text\":\"저성능 단순 추적\""));
	// unsupported FText parse result입니다.
	const FCFDAStagingParseResult UnsupportedText = FCFDAStagingService::ParseMissilePresetJson(UnsupportedTextJson);
	TestFalse(TEXT("Unsupported FText representation must be invalid"), UnsupportedText.bValid);
	TestTrue(TEXT("Unsupported FText diagnostic"), FCFDAStagingService::HasIssueCode(UnsupportedText.Issues, ECFDAStagingIssueCode::UnsupportedTextRepresentation));

	// invalid enum token을 넣은 JSON입니다.
	FString InvalidEnumJson = BuildValidJson();
	InvalidEnumJson.ReplaceInline(TEXT("\"GuidanceLaw\":\"PurePursuit\""), TEXT("\"GuidanceLaw\":\"purepursuit\""));
	// invalid enum parse result입니다.
	const FCFDAStagingParseResult InvalidEnum = FCFDAStagingService::ParseMissilePresetJson(InvalidEnumJson);
	TestFalse(TEXT("Enum display/case variation must not be silently accepted"), InvalidEnum.bValid);
	TestTrue(TEXT("Invalid enum diagnostic"), FCFDAStagingService::HasIssueCode(InvalidEnum.Issues, ECFDAStagingIssueCode::InvalidEnumValue));

	// authored range 밖 NavigationConstant를 넣은 JSON입니다.
	const FCFDAStagingParseResult OutOfRange = FCFDAStagingService::ParseMissilePresetJson(BuildValidJson(TEXT("MissileFeel_Low"), TEXT("MissileFeel_Low"), TEXT("null"), TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_Low.DA_MissileFeel_Low"), TEXT("11.0")));
	TestFalse(TEXT("Out-of-range raw authored value must be invalid instead of clamped"), OutOfRange.bValid);
	TestTrue(TEXT("Out-of-range diagnostic"), FCFDAStagingService::HasIssueCode(OutOfRange.Issues, ECFDAStagingIssueCode::InvalidValue));

	// top-level identity와 payload identity가 다른 JSON입니다.
	const FCFDAStagingParseResult IdentityMismatch = FCFDAStagingService::ParseMissilePresetJson(BuildValidJson(TEXT("MissileFeel_Low"), TEXT("MissileFeel_High")));
	TestFalse(TEXT("Split identity authority must be invalid"), IdentityMismatch.bValid);
	TestTrue(TEXT("Identity mismatch diagnostic"), FCFDAStagingService::HasIssueCode(IdentityMismatch.Issues, ECFDAStagingIssueCode::StableIdentityMismatch));

	// unsupported SchemaRevision JSON입니다.
	FString BadSchemaRevisionJson = BuildValidJson();
	BadSchemaRevisionJson.ReplaceInline(TEXT("\"SchemaRevision\":1"), TEXT("\"SchemaRevision\":0"));
	// schema revision mismatch parse result입니다.
	const FCFDAStagingParseResult BadSchemaRevision = FCFDAStagingService::ParseMissilePresetJson(BadSchemaRevisionJson);
	TestFalse(TEXT("Schema revision mismatch must be invalid"), BadSchemaRevision.bValid);
	TestTrue(TEXT("Schema revision diagnostic"), FCFDAStagingService::HasIssueCode(BadSchemaRevision.Issues, ECFDAStagingIssueCode::SchemaRevisionUnsupported));

	// 이전 canonicalization 의미를 가진 AdapterContractRevision 1 old Staging JSON입니다.
	FString OldAdapterRevisionJson = BuildValidJson();
	OldAdapterRevisionJson.ReplaceInline(TEXT("\"AdapterContractRevision\":2"), TEXT("\"AdapterContractRevision\":1"));
	// old adapter revision mismatch parse result입니다.
	const FCFDAStagingParseResult OldAdapterRevision = FCFDAStagingService::ParseMissilePresetJson(OldAdapterRevisionJson);
	TestFalse(TEXT("Old AdapterContractRevision 1 Staging must be invalid"), OldAdapterRevision.bValid);
	TestTrue(TEXT("Old adapter revision emits AdapterRevisionMismatch"), FCFDAStagingService::HasIssueCode(OldAdapterRevision.Issues, ECFDAStagingIssueCode::AdapterRevisionMismatch));

	// direct typed current/staging payload가 authored 범위 밖 값을 가지는 경우입니다.
	FCFDAMissilePresetPayload InvalidTypedPayload = FCFDAStagingService::ParseMissilePresetJson(BuildValidJson()).Record.Payload;
	InvalidTypedPayload.MissileGuideConfig.NavigationConstant = 11.0f;
	// invalid typed payload의 hash output입니다.
	FString InvalidTypedFingerprint;
	// invalid typed payload의 hash failure reason입니다.
	FString InvalidTypedFingerprintError;
	TestFalse(TEXT("Direct typed payload outside authored range must not receive semantic fingerprint"), FCFDAStagingService::BuildSemanticFingerprint(InvalidTypedPayload, InvalidTypedFingerprint, InvalidTypedFingerprintError));

	// malformed base fingerprint Update JSON입니다.
	const FCFDAStagingParseResult BadFingerprint = FCFDAStagingService::ParseMissilePresetJson(BuildValidJson(TEXT("MissileFeel_Low"), TEXT("MissileFeel_Low"), TEXT("\"not-a-sha\"")));
	TestFalse(TEXT("Malformed Base fingerprint must be invalid"), BadFingerprint.bValid);
	TestTrue(TEXT("Malformed Base fingerprint diagnostic"), FCFDAStagingService::HasIssueCode(BadFingerprint.Issues, ECFDAStagingIssueCode::InvalidValue));

	// canonical Staging authority 밖 main_game-relative path를 넘긴 parse입니다.
	const FCFDAStagingParseResult OutsideStagingRoot = FCFDAStagingService::ParseMissilePresetJson(
		BuildValidJson(),
		TEXT("Tools/NotAStagingSource.json"));
	TestFalse(TEXT("Source path outside Authoring/DataAssetStaging must be invalid"), OutsideStagingRoot.bValid);
	TestTrue(TEXT("Outside Staging root diagnostic"), FCFDAStagingService::HasIssueCode(OutsideStagingRoot.Issues, ECFDAStagingIssueCode::InvalidValue));

	// canonical Staging root 안이지만 P0 canonical JSON extension이 아닌 path입니다.
	const FCFDAStagingParseResult WrongStagingExtension = FCFDAStagingService::ParseMissilePresetJson(
		BuildValidJson(),
		TEXT("Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Low.txt"));
	TestFalse(TEXT("Non-json Staging source path must be invalid"), WrongStagingExtension.bValid);
	TestTrue(TEXT("Non-json Staging extension diagnostic"), FCFDAStagingService::HasIssueCode(WrongStagingExtension.Issues, ECFDAStagingIssueCode::InvalidValue));

	// Missile SchemaId를 유지한 채 exact native class만 미등록 type으로 바꾼 JSON입니다.
	FString UnknownTypeKeyJson = BuildValidJson();
	UnknownTypeKeyJson.ReplaceInline(
		TEXT("/Script/CarFight_Re.CFMissileGuidePresetData"),
		TEXT("/Script/CarFight_Re.CFAmmoDefinition"));
	// trusted registry에 없는 exact TypeKey parse result입니다.
	const FCFDAStagingParseResult UnknownTypeKey = FCFDAStagingService::ParseMissilePresetJson(
		UnknownTypeKeyJson,
		TEXT("Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Low.json"));
	TestFalse(TEXT("Unknown exact TypeKey must fail before typed payload dispatch"), UnknownTypeKey.bValid);
	TestTrue(TEXT("Unknown exact TypeKey diagnostic"), FCFDAStagingService::HasIssueCode(UnknownTypeKey.Issues, ECFDAStagingIssueCode::SchemaUnsupported));

	// shared parent 아래이지만 selected Missile provider가 소유하지 않는 sibling StagingRoot path입니다.
	const FCFDAStagingParseResult SiblingProviderRoot = FCFDAStagingService::ParseMissilePresetJson(
		BuildValidJson(),
		TEXT("Authoring/DataAssetStaging/Ammo/MissileFeel_Low.json"));
	TestFalse(TEXT("Sibling provider StagingRoot must fail-closed"), SiblingProviderRoot.bValid);
	TestTrue(TEXT("Sibling provider StagingRoot diagnostic"), FCFDAStagingService::HasIssueCode(SiblingProviderRoot.Issues, ECFDAStagingIssueCode::InvalidValue));

	// Current first typed provider authority입니다.
	const FCFDAMissileTypeProvider& MissileTypedProvider = CFDAMissileProvider::GetProvider();
	TestEqual(TEXT("Missile provider SchemaId parity"), MissileTypedProvider.Descriptor.TypeKey.SchemaId, FString(FCFDAStagingService::GetMissilePresetSchemaId()));
	TestEqual(TEXT("Missile provider SchemaRevision parity"), MissileTypedProvider.Descriptor.SchemaRevision, FCFDAStagingService::GetMissilePresetSchemaRevision());
	TestEqual(TEXT("Missile provider AdapterRevision parity"), MissileTypedProvider.Descriptor.AdapterContractRevision, FCFDAStagingService::GetMissilePresetAdapterRevision());
	TestEqual(TEXT("Missile provider class parity"), MissileTypedProvider.Descriptor.TypeKey.DataAssetTypeClassPath, FString(FCFDAStagingService::GetMissilePresetClassPath()));
	TestEqual(TEXT("Missile provider StagingRoot parity"), MissileTypedProvider.Descriptor.CanonicalStagingRoot, FString(TEXT("Authoring/DataAssetStaging/MissileGuidePreset")));
	TestEqual(TEXT("Missile provider StableIdentity source parity"), MissileTypedProvider.Descriptor.StableIdentitySourceName, FName(TEXT("PresetId")));
	TestEqual(TEXT("Missile provider StableIdentity policy parity"), MissileTypedProvider.Descriptor.StableIdentityPolicy, ECFDAStableIdentityPolicy::Required);
	TestEqual(TEXT("Missile provider StableIdentity resolver parity"), MissileTypedProvider.Descriptor.StableIdentityResolver, ECFDAStableIdentityResolver::ExplicitFName);
	TestEqual(TEXT("Missile provider DACE owner parity"), MissileTypedProvider.Descriptor.DaceContractOwnerName, FName(TEXT("FCFDAContractGuard")));
	TestEqual(TEXT("Missile provider DACE history namespace parity"), MissileTypedProvider.Descriptor.DaceAcceptedHistoryNamespace, FString(TEXT("DACE-MissileGuidePreset")));
	TestTrue(TEXT("Missile shared ParseCommonCandidate operation must exist"), MissileTypedProvider.Operations.ParseCommonCandidate != nullptr);
	TestTrue(TEXT("Missile shared ResolveCommonCurrentState operation must exist"), MissileTypedProvider.Operations.ResolveCommonCurrentState != nullptr);
	TestTrue(TEXT("Missile shared ApplyReviewedMutation operation must exist"), MissileTypedProvider.Operations.ApplyReviewedMutation != nullptr);
	TestEqual(TEXT("Missile provider remains ReviewedMutationReady"), MissileTypedProvider.Readiness, ECFDAProviderReadiness::ReviewedMutationReady);
	// Missile mutation readiness validation detail입니다.
	FString MissileMutationReadinessError;
	TestTrue(
		TEXT("Missile provider must remain Reviewed mutation ready"),
		CFDATypeDispatch::ValidateProviderMutationReady(MissileTypedProvider, MissileMutationReadinessError));
	TestTrue(TEXT("Missile provider Parse callback must exist"), MissileTypedProvider.Callbacks.ParseJson != nullptr);
	TestTrue(TEXT("Missile provider Fingerprint callback must exist"), MissileTypedProvider.Callbacks.BuildSemanticFingerprint != nullptr);
	TestTrue(TEXT("Missile provider Extract callback must exist"), MissileTypedProvider.Callbacks.ExtractPayload != nullptr);
	TestTrue(TEXT("Missile provider CurrentState callback must exist"), MissileTypedProvider.Callbacks.ResolveCurrentState != nullptr);
	TestTrue(TEXT("Missile provider Serializer callback must exist"), MissileTypedProvider.Callbacks.SerializeProductStagingJson != nullptr);
	TestTrue(TEXT("Missile provider Materializer callback must exist"), MissileTypedProvider.Callbacks.MaterializePayload != nullptr);

	// Production complete provider registry structural validation failure detail입니다.
	FString ProductionRegistryError;
	TestTrue(
		TEXT("Production provider registry must validate as complete exact registry"),
		CFDATypeDispatch::ValidateProviderRegistry(ProductionRegistryError));

	// Exact production Missile TypeKey lookup failure detail입니다.
	FString ExactLookupError;
	// Exact production Missile provider entry lookup 결과입니다.
	const FCFDATypeProviderEntry* ExactProviderEntry = CFDATypeDispatch::FindExactProviderEntry(
		MissileTypedProvider.Descriptor.TypeKey.SchemaId,
		MissileTypedProvider.Descriptor.TypeKey.DataAssetTypeClassPath,
		&ExactLookupError);
	TestTrue(TEXT("Exact Missile TypeKey must resolve complete registered provider entry"), ExactProviderEntry == &MissileTypedProvider);

	// SchemaId case-variant lookup failure detail입니다.
	FString CaseVariantLookupError;
	TestTrue(
		TEXT("SchemaId case variant must not select provider"),
		CFDATypeDispatch::FindExactProviderEntry(
			TEXT("carfight.DataAsset.MissileGuidePreset"),
			MissileTypedProvider.Descriptor.TypeKey.DataAssetTypeClassPath,
			&CaseVariantLookupError) == nullptr);

	// Production registry를 변경하지 않는 complete test-only second provider entry입니다.
	const FCFDATypeProviderEntry SecondProviderEntry = BuildSecondProviderEntry();
	// Missile + second provider exact two-entry test set입니다.
	const TArray<const FCFDATypeProviderEntry*> TwoProviderSet = {&MissileTypedProvider, &SecondProviderEntry};
	// Two-provider set validation detail입니다.
	FString TwoProviderSetError;
	TestTrue(
		TEXT("Complete second-provider test set must validate without production registry mutation"),
		CFDATypeDispatch::ValidateProviderSetForTests(TwoProviderSet, TwoProviderSetError));
	// Second provider exact lookup failure detail입니다.
	FString SecondProviderLookupError;
	TestTrue(
		TEXT("Exact second-provider TypeKey must dispatch to second provider entry"),
		CFDATypeDispatch::FindExactProviderEntryInSetForTests(
			TwoProviderSet,
			SecondProviderEntry.Descriptor.TypeKey.SchemaId,
			SecondProviderEntry.Descriptor.TypeKey.DataAssetTypeClassPath,
			&SecondProviderLookupError) == &SecondProviderEntry);
	TestEqual(TEXT("Second provider is explicit read-only Preview ready"), SecondProviderEntry.Readiness, ECFDAProviderReadiness::ReadOnlyPreviewReady);
	TestTrue(TEXT("Read-only second provider must not own mutation callback"), SecondProviderEntry.Operations.ApplyReviewedMutation == nullptr);
	// Read-only provider mutation readiness failure detail입니다.
	FString ReadOnlyMutationError;
	TestFalse(
		TEXT("Read-only second provider must fail Reviewed mutation readiness"),
		CFDATypeDispatch::ValidateProviderMutationReady(SecondProviderEntry, ReadOnlyMutationError));
	TestFalse(TEXT("Read-only mutation rejection must explain failure"), ReadOnlyMutationError.IsEmpty());

	// Mutation-ready를 선언했지만 writer callback이 없는 invalid provider entry입니다.
	FCFDATypeProviderEntry IncompleteMutationProvider = BuildSecondProviderEntry();
	IncompleteMutationProvider.Readiness = ECFDAProviderReadiness::ReviewedMutationReady;
	// Incomplete mutation provider 하나를 포함하는 validation set입니다.
	const TArray<const FCFDATypeProviderEntry*> IncompleteMutationSet = {&MissileTypedProvider, &IncompleteMutationProvider};
	// Missing mutation callback validation detail입니다.
	FString IncompleteMutationError;
	TestFalse(
		TEXT("ReviewedMutationReady provider without writer callback must fail registry validation"),
		CFDATypeDispatch::ValidateProviderSetForTests(IncompleteMutationSet, IncompleteMutationError));

	// Provider-local typed fingerprint cache와 재계산값이 동일한 positive seam입니다.
	FString CachedFingerprintIntegrityError;
	TestTrue(
		TEXT("Identical canonical provider-local cached fingerprint must validate"),
		CFDACommonPrimitives::ValidateCachedSemanticFingerprint(FingerprintA, FingerprintA, CachedFingerprintIntegrityError));
	TestFalse(
		TEXT("Split provider-local cached fingerprint must fail before common projection"),
		CFDACommonPrimitives::ValidateCachedSemanticFingerprint(FingerprintA, FingerprintB, CachedFingerprintIntegrityError));

	// Nullable AmmoIcon 표현의 empty path입니다.
	FSoftObjectPath NullAmmoIconPath;
	// AmmoIcon strict parse diagnostics입니다.
	TArray<FCFDAStagingIssue> AmmoIconIssues;
	TestTrue(
		TEXT("AmmoIcon null path must pass strict syntax parse"),
		CFDACommonPrimitives::ParseCanonicalSoftObjectPath(TEXT(""), TEXT("Payload.AmmoIcon"), true, NullAmmoIconPath, AmmoIconIssues));
	TestTrue(
		TEXT("AmmoIcon null path must pass metadata validation"),
		CFDACommonPrimitives::ValidateAssetReferenceMetadata(
			NullAmmoIconPath,
			UTexture2D::StaticClass()->GetClassPathName(),
			TEXT("Payload.AmmoIcon"),
			AmmoIconIssues));

	// `/Engine` mount를 사용해 `/Game` 제한 없이 검증할 canonical existing Texture2D path입니다.
	FSoftObjectPath ExistingTexturePath;
	AmmoIconIssues.Reset();
	TestTrue(
		TEXT("Canonical /Engine Texture2D path must pass syntax without /Game restriction"),
		CFDACommonPrimitives::ParseCanonicalSoftObjectPath(
			TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"),
			TEXT("Payload.AmmoIcon"),
			false,
			ExistingTexturePath,
			AmmoIconIssues));
	// Metadata validation 전 process-local resolve 상태입니다. ResolveObject는 load를 수행하지 않습니다.
	UObject* TextureObjectBeforeMetadataValidation = ExistingTexturePath.ResolveObject();
	TestTrue(
		TEXT("Existing Texture2D metadata must satisfy AmmoIcon expected class"),
		CFDACommonPrimitives::ValidateAssetReferenceMetadata(
			ExistingTexturePath,
			UTexture2D::StaticClass()->GetClassPathName(),
			TEXT("Payload.AmmoIcon"),
			AmmoIconIssues));
	// Metadata validation 후 process-local resolve 상태입니다.
	UObject* TextureObjectAfterMetadataValidation = ExistingTexturePath.ResolveObject();
	TestTrue(
		TEXT("AmmoIcon metadata validation must not load referenced Texture2D"),
		TextureObjectBeforeMetadataValidation == TextureObjectAfterMetadataValidation);

	// 문법은 canonical이지만 Asset Registry에 존재하지 않는 AmmoIcon path입니다.
	FSoftObjectPath MissingTexturePath;
	AmmoIconIssues.Reset();
	TestTrue(
		TEXT("Missing AmmoIcon can still pass canonical SoftObjectPath syntax stage"),
		CFDACommonPrimitives::ParseCanonicalSoftObjectPath(
			TEXT("/Game/Test/CarFight/NoSuch/DA_MissingTexture.DA_MissingTexture"),
			TEXT("Payload.AmmoIcon"),
			false,
			MissingTexturePath,
			AmmoIconIssues));
	TestFalse(
		TEXT("Missing AmmoIcon metadata must block Preview validation"),
		CFDACommonPrimitives::ValidateAssetReferenceMetadata(
			MissingTexturePath,
			UTexture2D::StaticClass()->GetClassPathName(),
			TEXT("Payload.AmmoIcon"),
			AmmoIconIssues));
	TestTrue(TEXT("Missing AmmoIcon must emit InvalidValue"), FCFDAStagingService::HasIssueCode(AmmoIconIssues, ECFDAStagingIssueCode::InvalidValue));

	// Existing StaticMesh를 Texture2D expected class로 검증하는 wrong-class reference입니다.
	FSoftObjectPath WrongClassPath;
	AmmoIconIssues.Reset();
	TestTrue(
		TEXT("Existing non-texture asset path must pass syntax stage"),
		CFDACommonPrimitives::ParseCanonicalSoftObjectPath(
			TEXT("/Engine/BasicShapes/Cube.Cube"),
			TEXT("Payload.AmmoIcon"),
			false,
			WrongClassPath,
			AmmoIconIssues));
	TestFalse(
		TEXT("Existing wrong-class AmmoIcon metadata must block Preview validation"),
		CFDACommonPrimitives::ValidateAssetReferenceMetadata(
			WrongClassPath,
			UTexture2D::StaticClass()->GetClassPathName(),
			TEXT("Payload.AmmoIcon"),
			AmmoIconIssues));
	TestTrue(TEXT("Wrong-class AmmoIcon must emit InvalidValue"), FCFDAStagingService::HasIssueCode(AmmoIconIssues, ECFDAStagingIssueCode::InvalidValue));

	// Subobject syntax을 포함해 top-level asset object path가 아닌 AmmoIcon 입력입니다.
	FSoftObjectPath SubobjectTexturePath;
	AmmoIconIssues.Reset();
	TestFalse(
		TEXT("AmmoIcon SoftObjectPath with subobject must fail strict syntax stage"),
		CFDACommonPrimitives::ParseCanonicalSoftObjectPath(
			TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture:SubObject"),
			TEXT("Payload.AmmoIcon"),
			false,
			SubobjectTexturePath,
			AmmoIconIssues));

	// Duplicate exact TypeKey regression을 만들 test-only entry입니다.
	FCFDATypeProviderEntry DuplicateTypeKeyEntry = BuildSecondProviderEntry();
	DuplicateTypeKeyEntry.Descriptor.TypeKey = MissileTypedProvider.Descriptor.TypeKey;
	// Duplicate exact TypeKey를 포함하는 test-only provider set입니다.
	const TArray<const FCFDATypeProviderEntry*> DuplicateTypeKeySet = {&MissileTypedProvider, &DuplicateTypeKeyEntry};
	// Duplicate TypeKey registry validation failure detail입니다.
	FString DuplicateTypeKeyError;
	TestFalse(
		TEXT("Duplicate exact TypeKey provider set must fail-closed"),
		CFDATypeDispatch::ValidateProviderSetForTests(DuplicateTypeKeySet, DuplicateTypeKeyError));
	TestFalse(TEXT("Duplicate exact TypeKey diagnostic must not be empty"), DuplicateTypeKeyError.IsEmpty());
	// Duplicate set lookup 자체도 first-match로 진행하지 않고 fail-closed되는지 확인할 detail입니다.
	FString DuplicateLookupError;
	TestTrue(
		TEXT("Duplicate exact TypeKey lookup must not return first match"),
		CFDATypeDispatch::FindExactProviderEntryInSetForTests(
			DuplicateTypeKeySet,
			MissileTypedProvider.Descriptor.TypeKey.SchemaId,
			MissileTypedProvider.Descriptor.TypeKey.DataAssetTypeClassPath,
			&DuplicateLookupError) == nullptr);

	// 동일 class 안의 FName case variant stable identity key입니다.
	const FString SameClassIdentityKeyA = CFDATypeDispatch::BuildClassScopedStableIdentityKey(
		MissileTypedProvider.Descriptor.TypeKey.DataAssetTypeClassPath,
		FName(TEXT("MissileFeel_Low")));
	// 동일 class 안의 case variant identity key입니다.
	const FString SameClassIdentityKeyB = CFDATypeDispatch::BuildClassScopedStableIdentityKey(
		MissileTypedProvider.Descriptor.TypeKey.DataAssetTypeClassPath,
		FName(TEXT("missilefeel_low")));
	// 다른 class scope에서 같은 textual identity를 사용한 key입니다.
	const FString OtherClassIdentityKey = CFDATypeDispatch::BuildClassScopedStableIdentityKey(
		TEXT("/Script/CarFight_Re.CFAmmoDefinition"),
		FName(TEXT("MissileFeel_Low")));
	TestEqual(TEXT("Same class FName case variants must share duplicate key"), SameClassIdentityKeyA, SameClassIdentityKeyB);
	TestTrue(TEXT("Same textual StableLogicalId in another class must use another duplicate scope"), SameClassIdentityKeyA != OtherClassIdentityKey);
	return true;
}

// Create/Update/NoChange/Conflict/dirty/path/identity exact Preview matrix를 검증합니다.
bool FCFDAStagingPreviewMatrixTest::RunTest(const FString& Parameters)
{
	using namespace CFDAStagingTestsPrivate;

	// valid Create-intent parsed record입니다.
	const FCFDAStagingParseResult ParseResult = ParseValidRecord(
		*this,
		BuildValidJson(),
		TEXT("Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Low.json"));
	// MissileFeel 실사용 path와 process-local load 상태를 공유하지 않는 P0-02 reserved absent resolver record입니다.
	const FCFDAStagingParseResult ResolverAbsentParseResult = ParseValidRecord(
		*this,
		BuildValidJson(
			TEXT("DAS_P0_02_ResolverAbsent"),
			TEXT("DAS_P0_02_ResolverAbsent"),
			TEXT("null"),
			TEXT("/Game/Test/CarFight/DataAssetStaging/DA_DAS_ResolverAbsent.DA_DAS_ResolverAbsent")),
		TEXT("Authoring/DataAssetStaging/MissileGuidePreset/DAS_P0_02_ResolverAbsent.json"));

	// reserved-never-authored target/identity를 Asset Registry + typed resolver로 읽은 absent state입니다.
	FCFDAStagingCurrentState ResolvedCurrentState;
	// current resolver에서 발생한 stable diagnostic입니다.
	TArray<FCFDAStagingIssue> ResolveIssues;
	TestTrue(
		TEXT("Reserved absent preset target and identity must resolve read-only without mutation"),
		FCFDAStagingService::ResolveMissilePresetCurrentState(ResolverAbsentParseResult.Record, ResolvedCurrentState, ResolveIssues));
	TestEqual(TEXT("Reserved absent resolver must not emit blocking diagnostics"), ResolveIssues.Num(), 0);
	TestFalse(TEXT("Reserved absent target must remain absent"), ResolvedCurrentState.bRequestedTargetExists);
	TestTrue(TEXT("Reserved absent target class path must stay empty"), ResolvedCurrentState.RequestedTargetClassPath.IsEmpty());
	TestTrue(TEXT("Reserved absent target stable identity must stay None"), ResolvedCurrentState.RequestedTargetStableLogicalId.IsNone());
	TestTrue(TEXT("Reserved absent target current fingerprint must stay empty"), ResolvedCurrentState.CurrentSemanticFingerprint.IsEmpty());
	TestFalse(TEXT("Reserved absent stable identity must remain absent"), ResolvedCurrentState.bStableIdentityExists);
	TestEqual(TEXT("Reserved absent stable identity match count"), ResolvedCurrentState.StableIdentityMatchCount, 0);
	TestTrue(TEXT("Reserved absent stable identity path must stay empty"), ResolvedCurrentState.StableIdentityObjectPath.IsEmpty());
	TestFalse(TEXT("Reserved absent target cannot be dirty"), ResolvedCurrentState.bRequestedTargetDirty);

	// actual resolver의 absent current truth로 expected Create Preview를 만듭니다.
	const FCFDAStagingPreviewRow CreatePreview = FCFDAStagingService::BuildPreview(ResolverAbsentParseResult.Record, ResolvedCurrentState);
	TestEqual(TEXT("Reserved absent create target must classify Create"), CreatePreview.Kind, ECFDAStagingPreviewKind::Create);

	// Base==Current인 Update record입니다.
	const FCFDAStagingRecord UpdateRecord = MakeUpdateRecord(ParseResult.Record, FingerprintA);
	// Base==Current current state입니다.
	const FCFDAStagingCurrentState BaseCurrentState = MakeExistingCurrentState(UpdateRecord, FingerprintA);
	// Staging differs from Base/Current이므로 expected Update Preview입니다.
	const FCFDAStagingPreviewRow UpdatePreview = FCFDAStagingService::BuildPreview(UpdateRecord, BaseCurrentState);
	TestEqual(TEXT("Base==Current and Staging differs must classify Update"), UpdatePreview.Kind, ECFDAStagingPreviewKind::Update);

	// Base/Current/Staging이 모두 같은 exact unchanged Update record입니다.
	const FCFDAStagingRecord ExactNoChangeRecord = MakeUpdateRecord(ParseResult.Record, ParseResult.Record.StagingSemanticFingerprint);
	// exact unchanged current state입니다.
	const FCFDAStagingCurrentState ExactNoChangeState = MakeExistingCurrentState(ExactNoChangeRecord, ParseResult.Record.StagingSemanticFingerprint);
	// expected exact NoChange Preview입니다.
	const FCFDAStagingPreviewRow ExactNoChangePreview = FCFDAStagingService::BuildPreview(ExactNoChangeRecord, ExactNoChangeState);
	TestEqual(TEXT("Base==Current==Staging must classify NoChange"), ExactNoChangePreview.Kind, ECFDAStagingPreviewKind::NoChange);
	TestFalse(TEXT("Exact NoChange must not require baseline rebase"), FCFDAStagingService::HasIssueCode(ExactNoChangePreview.Issues, ECFDAStagingIssueCode::BaselineRebaseRequired));

	// current와 staging fingerprint를 같게 둔 existing state입니다.
	const FCFDAStagingCurrentState ConvergedState = MakeExistingCurrentState(UpdateRecord, UpdateRecord.StagingSemanticFingerprint);
	// Base differs but Current==Staging인 expected converged NoChange Preview입니다.
	const FCFDAStagingPreviewRow ConvergedPreview = FCFDAStagingService::BuildPreview(UpdateRecord, ConvergedState);
	TestEqual(TEXT("Current==Staging convergence must classify NoChange"), ConvergedPreview.Kind, ECFDAStagingPreviewKind::NoChange);
	TestTrue(TEXT("Converged NoChange must require explicit baseline rebase"), FCFDAStagingService::HasIssueCode(ConvergedPreview.Issues, ECFDAStagingIssueCode::BaselineRebaseRequired));

	// Base/Current/Staging이 모두 다른 current state입니다.
	const FCFDAStagingCurrentState DriftedState = MakeExistingCurrentState(UpdateRecord, FingerprintB);
	// expected BaselineMismatch conflict입니다.
	const FCFDAStagingPreviewRow DriftedPreview = FCFDAStagingService::BuildPreview(UpdateRecord, DriftedState);
	TestEqual(TEXT("Unexpected current drift must classify Conflict"), DriftedPreview.Kind, ECFDAStagingPreviewKind::Conflict);
	TestTrue(TEXT("Unexpected current drift diagnostic"), FCFDAStagingService::HasIssueCode(DriftedPreview.Issues, ECFDAStagingIssueCode::BaselineMismatch));

	// Staging이 old Base를 그대로 유지했지만 Current만 달라진 record입니다.
	FCFDAStagingRecord StagingEqualsBaseRecord = ParseResult.Record;
	StagingEqualsBaseRecord.bHasBaseSemanticFingerprint = true;
	StagingEqualsBaseRecord.BaseSemanticFingerprint = StagingEqualsBaseRecord.StagingSemanticFingerprint;
	// current-only drift state입니다.
	const FCFDAStagingCurrentState CurrentOnlyDriftState = MakeExistingCurrentState(StagingEqualsBaseRecord, FingerprintC);
	// old Base를 조용히 재base하지 않는 expected conflict입니다.
	const FCFDAStagingPreviewRow CurrentOnlyDriftPreview = FCFDAStagingService::BuildPreview(StagingEqualsBaseRecord, CurrentOnlyDriftState);
	TestEqual(TEXT("Staging==Base but Current differs must Conflict"), CurrentOnlyDriftPreview.Kind, ECFDAStagingPreviewKind::Conflict);
	TestTrue(TEXT("Current-only drift diagnostic"), FCFDAStagingService::HasIssueCode(CurrentOnlyDriftPreview.Issues, ECFDAStagingIssueCode::BaselineMismatch));

	// Create-intent target이 이미 존재하는 state입니다.
	const FCFDAStagingCurrentState ExistingCreateState = MakeExistingCurrentState(ParseResult.Record, ParseResult.Record.StagingSemanticFingerprint);
	// payload가 우연히 같아도 ownership을 추정하지 않는 expected conflict입니다.
	const FCFDAStagingPreviewRow ExistingCreatePreview = FCFDAStagingService::BuildPreview(ParseResult.Record, ExistingCreateState);
	TestEqual(TEXT("Create intent with existing target must Conflict"), ExistingCreatePreview.Kind, ECFDAStagingPreviewKind::Conflict);
	TestTrue(TEXT("Unexpected existing target diagnostic"), FCFDAStagingService::HasIssueCode(ExistingCreatePreview.Issues, ECFDAStagingIssueCode::UnexpectedExistingTarget));

	// Update target이 사라진 state입니다.
	const FCFDAStagingCurrentState MissingUpdateState;
	// expected TargetMissing conflict입니다.
	const FCFDAStagingPreviewRow MissingUpdatePreview = FCFDAStagingService::BuildPreview(UpdateRecord, MissingUpdateState);
	TestEqual(TEXT("Update baseline with missing target must Conflict"), MissingUpdatePreview.Kind, ECFDAStagingPreviewKind::Conflict);
	TestTrue(TEXT("Target missing diagnostic"), FCFDAStagingService::HasIssueCode(MissingUpdatePreview.Issues, ECFDAStagingIssueCode::TargetMissing));

	// same stable identity가 다른 path에 존재하는 state입니다.
	FCFDAStagingCurrentState MovedState;
	MovedState.bStableIdentityExists = true;
	MovedState.StableIdentityMatchCount = 1;
	MovedState.StableIdentityObjectPath = TEXT("/Game/Test/CarFight/Missile/FeelPresets/Moved.Moved");
	// expected TargetMoved conflict입니다.
	const FCFDAStagingPreviewRow MovedPreview = FCFDAStagingService::BuildPreview(ParseResult.Record, MovedState);
	TestEqual(TEXT("Same identity at other path must Conflict"), MovedPreview.Kind, ECFDAStagingPreviewKind::Conflict);
	TestTrue(TEXT("Target moved diagnostic"), FCFDAStagingService::HasIssueCode(MovedPreview.Issues, ECFDAStagingIssueCode::TargetMoved));

	// requested path에 다른 identity object가 존재하는 state입니다.
	FCFDAStagingCurrentState CollisionState = MakeExistingCurrentState(ParseResult.Record, FingerprintA);
	CollisionState.RequestedTargetStableLogicalId = FName(TEXT("OtherPreset"));
	// expected PathCollision conflict입니다.
	const FCFDAStagingPreviewRow CollisionPreview = FCFDAStagingService::BuildPreview(ParseResult.Record, CollisionState);
	TestEqual(TEXT("Different identity at requested path must Conflict"), CollisionPreview.Kind, ECFDAStagingPreviewKind::Conflict);
	TestTrue(TEXT("Path collision diagnostic"), FCFDAStagingService::HasIssueCode(CollisionPreview.Issues, ECFDAStagingIssueCode::PathCollision));

	// current Project에 같은 StableLogicalId가 둘 이상 존재하는 상태입니다.
	FCFDAStagingCurrentState DuplicateCurrentState = MakeExistingCurrentState(UpdateRecord, FingerprintA);
	DuplicateCurrentState.StableIdentityMatchCount = 2;
	DuplicateCurrentState.StableIdentityObjectPath.Reset();
	// expected current duplicate identity conflict입니다.
	const FCFDAStagingPreviewRow DuplicateCurrentPreview = FCFDAStagingService::BuildPreview(UpdateRecord, DuplicateCurrentState);
	TestEqual(TEXT("Duplicate current StableLogicalId must Conflict"), DuplicateCurrentPreview.Kind, ECFDAStagingPreviewKind::Conflict);
	TestTrue(TEXT("Duplicate current StableLogicalId diagnostic"), FCFDAStagingService::HasIssueCode(DuplicateCurrentPreview.Issues, ECFDAStagingIssueCode::DuplicateStableIdentity));

	// pre-existing dirty package state입니다.
	FCFDAStagingCurrentState DirtyState = MakeExistingCurrentState(UpdateRecord, FingerprintA);
	DirtyState.bRequestedTargetDirty = true;
	// expected TargetDirtyUnowned conflict입니다.
	const FCFDAStagingPreviewRow DirtyPreview = FCFDAStagingService::BuildPreview(UpdateRecord, DirtyState);
	TestEqual(TEXT("Pre-existing dirty target must Conflict"), DirtyPreview.Kind, ECFDAStagingPreviewKind::Conflict);
	TestTrue(TEXT("Dirty target diagnostic"), FCFDAStagingService::HasIssueCode(DirtyPreview.Issues, ECFDAStagingIssueCode::TargetDirtyUnowned));

	// parse 이후 mutable Payload만 바꾸고 cached StagingSemanticFingerprint는 old value로 남긴 split DTO입니다.
	FCFDAStagingRecord SplitFingerprintRecord = ParseResult.Record;
	SplitFingerprintRecord.Payload.MissileGuideConfig.NavigationConstant = 4.0f;
	// cached fingerprint와 Payload 불일치를 허용하지 않는 expected Invalid Preview입니다.
	const FCFDAStagingPreviewRow SplitFingerprintPreview = FCFDAStagingService::BuildPreview(SplitFingerprintRecord, FCFDAStagingCurrentState());
	TestEqual(TEXT("Payload/fingerprint split DTO must classify Invalid"), SplitFingerprintPreview.Kind, ECFDAStagingPreviewKind::Invalid);
	TestTrue(TEXT("Payload/fingerprint split diagnostic"), FCFDAStagingService::HasIssueCode(SplitFingerprintPreview.Issues, ECFDAStagingIssueCode::InvalidValue));

	// parse 이후 mutable TargetObjectPath를 canonical Unreal object path 밖으로 바꾼 DTO입니다.
	FCFDAStagingRecord InvalidTargetPathRecord = ParseResult.Record;
	InvalidTargetPathRecord.TargetObjectPath = TEXT("Tools/NotAnAssetPath.json");
	// mutable target path 변조를 parser-only trust로 통과시키지 않는 expected Invalid Preview입니다.
	const FCFDAStagingPreviewRow InvalidTargetPathPreview = FCFDAStagingService::BuildPreview(InvalidTargetPathRecord, FCFDAStagingCurrentState());
	TestEqual(TEXT("Mutated invalid TargetObjectPath must classify Invalid"), InvalidTargetPathPreview.Kind, ECFDAStagingPreviewKind::Invalid);
	TestTrue(TEXT("Mutated invalid TargetObjectPath diagnostic"), FCFDAStagingService::HasIssueCode(InvalidTargetPathPreview.Issues, ECFDAStagingIssueCode::InvalidValue));
	return true;
}

// duplicate protection, physical row order independence와 exact binding BatchPlanHash를 검증합니다.
bool FCFDAStagingBatchHashTest::RunTest(const FString& Parameters)
{
	using namespace CFDAStagingTestsPrivate;

	// 첫 Create-intent record입니다.
	const FCFDAStagingParseResult LowResult = ParseValidRecord(
		*this,
		BuildValidJson(),
		TEXT("Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Low.json"));
	// 두 번째 Create-intent record입니다.
	const FCFDAStagingParseResult HighResult = ParseValidRecord(
		*this,
		BuildValidJson(
			TEXT("MissileFeel_High"),
			TEXT("MissileFeel_High"),
			TEXT("null"),
			TEXT("/Game/Test/CarFight/Missile/FeelPresets/DA_MissileFeel_High.DA_MissileFeel_High"),
			TEXT("4.0")),
		TEXT("Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_High.json"));

	// Low Create Preview row입니다.
	const FCFDAStagingPreviewRow LowPreview = FCFDAStagingService::BuildPreview(LowResult.Record, FCFDAStagingCurrentState());
	// High Create Preview row입니다.
	const FCFDAStagingPreviewRow HighPreview = FCFDAStagingService::BuildPreview(HighResult.Record, FCFDAStagingCurrentState());
	// first physical row order입니다.
	const TArray<FCFDAStagingPreviewRow> FirstOrder = {LowPreview, HighPreview};
	// reversed physical row order입니다.
	const TArray<FCFDAStagingPreviewRow> ReverseOrder = {HighPreview, LowPreview};
	// first-order BatchPlanHash입니다.
	FString FirstHash;
	// reversed-order BatchPlanHash입니다.
	FString ReverseHash;
	// first hash error입니다.
	FString FirstHashError;
	// reverse hash error입니다.
	FString ReverseHashError;
	TestTrue(TEXT("First-order Create batch must build plan hash"), FCFDAStagingService::BuildBatchPlanHash(FirstOrder, FirstHash, FirstHashError));
	TestTrue(TEXT("Reverse-order Create batch must build plan hash"), FCFDAStagingService::BuildBatchPlanHash(ReverseOrder, ReverseHash, ReverseHashError));
	TestEqual(TEXT("Physical preview row order must not change BatchPlanHash"), FirstHash, ReverseHash);

	// exact same identity/path를 중복한 Preview rows입니다.
	TArray<FCFDAStagingPreviewRow> DuplicateRows = {LowPreview, LowPreview};
	FCFDAStagingService::ApplyBatchDuplicateValidation(DuplicateRows);
	TestEqual(TEXT("Duplicate identity/path first row must become Invalid"), DuplicateRows[0].Kind, ECFDAStagingPreviewKind::Invalid);
	TestEqual(TEXT("Duplicate identity/path second row must become Invalid"), DuplicateRows[1].Kind, ECFDAStagingPreviewKind::Invalid);
	TestTrue(TEXT("Duplicate identity diagnostic"), FCFDAStagingService::HasIssueCode(DuplicateRows[0].Issues, ECFDAStagingIssueCode::DuplicateStableIdentity));
	TestTrue(TEXT("Duplicate target path diagnostic"), FCFDAStagingService::HasIssueCode(DuplicateRows[0].Issues, ECFDAStagingIssueCode::DuplicateTargetPath));

	// caller가 duplicate validator를 호출하지 않은 raw duplicate rows입니다.
	const TArray<FCFDAStagingPreviewRow> RawDuplicateRows = {LowPreview, LowPreview};
	// raw duplicate rows의 hash output입니다.
	FString DuplicateHash;
	// raw duplicate rows hash failure reason입니다.
	FString DuplicateHashError;
	TestFalse(TEXT("BuildBatchPlanHash must independently fail-closed on duplicate rows"), FCFDAStagingService::BuildBatchPlanHash(RawDuplicateRows, DuplicateHash, DuplicateHashError));

	// current fingerprint binding을 검증할 Update row입니다.
	const FCFDAStagingRecord LowUpdateRecord = MakeUpdateRecord(LowResult.Record, FingerprintA);
	// Base==Current current state입니다.
	const FCFDAStagingCurrentState LowUpdateState = MakeExistingCurrentState(LowUpdateRecord, FingerprintA);
	// Update Preview입니다.
	const FCFDAStagingPreviewRow LowUpdatePreview = FCFDAStagingService::BuildPreview(LowUpdateRecord, LowUpdateState);
	// exact Update plan hash입니다.
	FString UpdateHash;
	// Update plan hash error입니다.
	FString UpdateHashError;
	// exact single Update candidate set입니다.
	const TArray<FCFDAStagingPreviewRow> SingleUpdateRows = {LowUpdatePreview};
	TestTrue(TEXT("Valid Update row must build plan hash"), FCFDAStagingService::BuildBatchPlanHash(SingleUpdateRows, UpdateHash, UpdateHashError));
	TestTrue(TEXT("Create and Update operation/current/base binding must change BatchPlanHash"), UpdateHash != FirstHash);

	// Conflict row가 포함된 batch입니다.
	const FCFDAStagingPreviewRow ConflictPreview = FCFDAStagingService::BuildPreview(LowUpdateRecord, MakeExistingCurrentState(LowUpdateRecord, FingerprintC));
	// blocked batch hash output입니다.
	FString BlockedHash;
	// blocked batch hash failure reason입니다.
	FString BlockedHashError;
	// exact single Conflict row set입니다.
	const TArray<FCFDAStagingPreviewRow> SingleConflictRows = {ConflictPreview};
	TestFalse(TEXT("Conflict-containing batch must not receive approval hash"), FCFDAStagingService::BuildBatchPlanHash(SingleConflictRows, BlockedHash, BlockedHashError));

	// Preview 이후 Payload만 바뀌고 old StagingSemanticFingerprint를 유지한 forged candidate row입니다.
	FCFDAStagingPreviewRow SplitFingerprintCandidate = LowPreview;
	SplitFingerprintCandidate.Record.Payload.MissileGuideConfig.NavigationConstant = 4.5f;
	// split candidate의 hash output입니다.
	FString SplitFingerprintHash;
	// split candidate hash failure reason입니다.
	FString SplitFingerprintHashError;
	// exact single split candidate set입니다.
	const TArray<FCFDAStagingPreviewRow> SplitFingerprintRows = {SplitFingerprintCandidate};
	TestFalse(TEXT("BatchPlanHash must reject payload/fingerprint split candidate"), FCFDAStagingService::BuildBatchPlanHash(SplitFingerprintRows, SplitFingerprintHash, SplitFingerprintHashError));

	// Preview 이후 canonical Staging source path를 authority 밖 경로로 바꾼 forged candidate입니다.
	FCFDAStagingPreviewRow OutsideStagingCandidate = LowPreview;
	OutsideStagingCandidate.Record.StagingRelativePath = TEXT("Tools/NotAStagingSource.json");
	// source-path-forged candidate의 hash output입니다.
	FString OutsideStagingHash;
	// source-path-forged candidate hash failure reason입니다.
	FString OutsideStagingHashError;
	// exact single forged source-path candidate set입니다.
	const TArray<FCFDAStagingPreviewRow> OutsideStagingRows = {OutsideStagingCandidate};
	TestFalse(TEXT("BatchPlanHash must reject candidate outside canonical Staging root"), FCFDAStagingService::BuildBatchPlanHash(OutsideStagingRows, OutsideStagingHash, OutsideStagingHashError));

	// Preview 이후 Create candidate에 존재하지 않아야 할 Base를 주입한 forged intent입니다.
	FCFDAStagingPreviewRow ForgedCreateIntent = LowPreview;
	ForgedCreateIntent.Record.bHasBaseSemanticFingerprint = true;
	ForgedCreateIntent.Record.BaseSemanticFingerprint = FingerprintA;
	// forged Create intent의 hash output입니다.
	FString ForgedCreateHash;
	// forged Create intent hash failure reason입니다.
	FString ForgedCreateHashError;
	// exact single forged Create candidate set입니다.
	const TArray<FCFDAStagingPreviewRow> ForgedCreateRows = {ForgedCreateIntent};
	TestFalse(TEXT("BatchPlanHash must reject Create candidate with injected Base"), FCFDAStagingService::BuildBatchPlanHash(ForgedCreateRows, ForgedCreateHash, ForgedCreateHashError));

	// Existing Missile Create row를 shared payload-free row로 투영한 first-provider candidate입니다.
	FCFDACommonPreviewRow MissileCommonRow;
	MissileCommonRow.Kind = LowPreview.Kind;
	MissileCommonRow.Envelope = CFDAMissileProviderImpl::BuildCommonEnvelope(
		LowPreview.Record,
		LowPreview.CurrentSemanticFingerprint,
		LowPreview.Kind);
	MissileCommonRow.Issues = LowPreview.Issues;

	// Production registry를 변경하지 않는 test-only second provider authority입니다.
	const FCFDATypeProviderEntry SecondProviderEntry = BuildSecondProviderEntry();
	// Missile row와 동일 textual StableLogicalId를 다른 exact class scope에서 사용하는 second-provider Create row입니다.
	FCFDACommonPreviewRow SecondCommonRow;
	SecondCommonRow.Kind = ECFDAStagingPreviewKind::Create;
	SecondCommonRow.Envelope.SchemaId = SecondProviderEntry.Descriptor.TypeKey.SchemaId;
	SecondCommonRow.Envelope.SchemaRevision = SecondProviderEntry.Descriptor.SchemaRevision;
	SecondCommonRow.Envelope.AdapterContractRevision = SecondProviderEntry.Descriptor.AdapterContractRevision;
	SecondCommonRow.Envelope.DataAssetTypeClassPath = SecondProviderEntry.Descriptor.TypeKey.DataAssetTypeClassPath;
	SecondCommonRow.Envelope.StableLogicalId = LowPreview.Record.StableLogicalId;
	SecondCommonRow.Envelope.TargetObjectPath = TEXT("/Game/Test/CarFight/SecondProvider/DA_SharedIdentity.DA_SharedIdentity");
	SecondCommonRow.Envelope.StagingRelativePath = TEXT("Authoring/DataAssetStaging/SecondProviderTest/SharedIdentity.json");
	SecondCommonRow.Envelope.bHasBaseSemanticFingerprint = false;
	SecondCommonRow.Envelope.StagingSemanticFingerprint = FingerprintB;
	SecondCommonRow.Envelope.PlannedOperation = ECFDAStagingPreviewKind::Create;

	// Test-only second provider exact root contract validation detail입니다.
	FString SecondProviderContractError;
	TestTrue(
		TEXT("Second-provider common row must satisfy its exact provider root contract"),
		CFDATypeDispatch::ValidateProviderContract(
			SecondCommonRow.Envelope,
			SecondProviderEntry.Descriptor,
			SecondProviderContractError));

	// 같은 TypeKey라도 second provider가 소유하지 않는 sibling root로 위조한 common envelope입니다.
	FCFDACommonEnvelope OutsideSecondProviderRoot = SecondCommonRow.Envelope;
	OutsideSecondProviderRoot.StagingRelativePath = TEXT("Authoring/DataAssetStaging/OtherProvider/SharedIdentity.json");
	// Sibling-root isolation validation detail입니다.
	FString OutsideSecondProviderRootError;
	TestFalse(
		TEXT("Second-provider common row must reject sibling Staging root"),
		CFDATypeDispatch::ValidateProviderContract(
			OutsideSecondProviderRoot,
			SecondProviderEntry.Descriptor,
			OutsideSecondProviderRootError));

	// 다른 exact class scope에서 같은 textual identity를 가진 mixed-type common batch입니다.
	TArray<FCFDACommonPreviewRow> MixedTypeRows = {MissileCommonRow, SecondCommonRow};
	CFDATypeDispatch::ApplyCommonBatchDuplicateValidation(MixedTypeRows);
	TestEqual(TEXT("Same textual StableLogicalId in Missile class remains Create"), MixedTypeRows[0].Kind, ECFDAStagingPreviewKind::Create);
	TestEqual(TEXT("Same textual StableLogicalId in second-provider class remains Create"), MixedTypeRows[1].Kind, ECFDAStagingPreviewKind::Create);

	// First physical mixed-type order의 shared BatchPlanHash입니다.
	FString MixedTypeHash;
	// First physical mixed-type hash failure detail입니다.
	FString MixedTypeHashError;
	TestTrue(
		TEXT("Mixed-type common batch must build deterministic plan hash"),
		CFDATypeDispatch::BuildCommonBatchPlanHash(MixedTypeRows, MixedTypeHash, MixedTypeHashError));
	TestFalse(TEXT("Mixed-type common batch hash must not be empty"), MixedTypeHash.IsEmpty());

	// Mixed-type common Review가 동결할 exact Reviewed approval입니다.
	FCFDAStagingReviewedApproval MixedTypeApproval;
	// Mixed-type common Review failure detail입니다.
	FString MixedTypeReviewError;
	TestTrue(
		TEXT("Mixed-type common Review must freeze Reviewed approval without Missile-only branch"),
		CFDATypeDispatch::BuildCommonReviewedApproval(
			MixedTypeRows,
			MixedTypeApproval,
			MixedTypeReviewError));
	TestEqual(TEXT("Mixed-type common Review enters Reviewed state"), MixedTypeApproval.State, ECFDAStagingApprovalState::Reviewed);
	TestEqual(TEXT("Mixed-type common Review keeps exact two targets"), MixedTypeApproval.IncludedTargets.Num(), 2);
	TestEqual(TEXT("Mixed-type common Review preserves common BatchPlanHash"), MixedTypeApproval.BatchPlanHash, MixedTypeHash);

	// Reverse physical mixed-type order입니다.
	const TArray<FCFDACommonPreviewRow> ReverseMixedTypeRows = {SecondCommonRow, MissileCommonRow};
	// Reverse physical mixed-type order hash입니다.
	FString ReverseMixedTypeHash;
	// Reverse physical mixed-type hash failure detail입니다.
	FString ReverseMixedTypeHashError;
	TestTrue(
		TEXT("Reverse mixed-type common batch must build plan hash"),
		CFDATypeDispatch::BuildCommonBatchPlanHash(ReverseMixedTypeRows, ReverseMixedTypeHash, ReverseMixedTypeHashError));
	TestEqual(TEXT("Mixed-type physical row order must not change BatchPlanHash"), MixedTypeHash, ReverseMixedTypeHash);

	// 다른 class라도 exact TargetObjectPath가 같으면 global path collision으로 차단할 second-provider row입니다.
	FCFDACommonPreviewRow CrossTypeDuplicatePathRow = SecondCommonRow;
	CrossTypeDuplicatePathRow.Envelope.TargetObjectPath = MissileCommonRow.Envelope.TargetObjectPath;
	// Cross-type duplicate target path set입니다.
	TArray<FCFDACommonPreviewRow> CrossTypeDuplicatePathRows = {MissileCommonRow, CrossTypeDuplicatePathRow};
	CFDATypeDispatch::ApplyCommonBatchDuplicateValidation(CrossTypeDuplicatePathRows);
	TestEqual(TEXT("Global duplicate TargetObjectPath invalidates Missile row"), CrossTypeDuplicatePathRows[0].Kind, ECFDAStagingPreviewKind::Invalid);
	TestEqual(TEXT("Global duplicate TargetObjectPath invalidates second-provider row"), CrossTypeDuplicatePathRows[1].Kind, ECFDAStagingPreviewKind::Invalid);
	TestTrue(
		TEXT("Cross-type duplicate target path emits DuplicateTargetPath"),
		FCFDAStagingService::HasIssueCode(CrossTypeDuplicatePathRows[0].Issues, ECFDAStagingIssueCode::DuplicateTargetPath));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
