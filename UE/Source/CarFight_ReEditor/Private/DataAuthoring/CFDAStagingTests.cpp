// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAStagingTests.cpp
// Version: v1.3.0
// Date: 2026-09-08
// Description: CF-FQ-049 DAS-P0-02 strict whole-record parse/canonical fingerprint/read-only current resolve/mutation0 Preview/BatchPlanHash focused Automation입니다.
// Changelog:
// - v1.3.0: current AdapterContractRevision 2 fixture로 갱신하고 revision 1 old Staging이 AdapterRevisionMismatch로 fail-closed되는 회귀를 고정.
// - v1.2.1: parse 뒤 mutable TargetObjectPath/StagingRelativePath/Create intent binding 변조가 Preview/BatchPlanHash에서 fail-closed되는 회귀 검증을 추가.
// - v1.2.0: canonical Staging root 밖 source path fail-closed와 Payload↔StagingSemanticFingerprint split DTO의 Preview/BatchPlanHash 차단 회귀 검증을 추가.
// - v1.1.2: MissileFeel 실사용 object의 process-local load 상태와 분리된 reserved-never-authored path/identity로 read-only absent resolver fixture를 고정.
// - v1.1.1: P0-02 Apply/Save 금지 current truth에 맞춰 persisted Pilot 0건을 valid absent resolver state로 검증하고 그 state를 Create Preview에 직접 연결.
// - v1.1.0: existing MissileGuidePreset read-only current resolver와 current duplicate StableLogicalId fail-closed 검증을 추가.
// - v1.0.0: Pilot schema fail-closed parse, semantic canonicalization, exact 3-way Preview, duplicate protection과 plan hash determinism을 검증.
// Migration:
// - 메모리 DTO/JSON과 existing persisted MissileGuidePreset의 read-only resolve만 사용합니다. UObject/package/Asset Registry mutation/Save를 수행하지 않습니다.

#include "DataAuthoring/CFDAStaging.h"

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
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
