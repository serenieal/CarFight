// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAAmmoProviderTests.cpp
// Version: v1.2.0
// Date: 2026-09-10
// Description: CF-FQ-051 Ammo typed parse/fingerprint/Preview/provider readiness focused Automation입니다.
// Changelog:
// - v1.2.0: DAO-P0-03 writer 승격에 맞춰 production Ammo provider가 ReviewedMutationReady + non-null Apply callback임을 검증하도록 readiness baseline 갱신.
// - v1.1.0: empty Literal AmmoDisplayName / NAME_None AmmoFamilyId 허용 semantic과 actual Ammo common Preview rows의 deterministic BatchPlanHash integration regression을 추가.
// - v1.0.2: AmmoIcon canonical missing-asset metadata blocking regression을 PreviewProvider exact test에 추가.
// - v1.0.1: UE TNumericLimits에 없는 Infinity() 사용을 std::numeric_limits<float>::infinity()로 교정.
// - v1.0.0: exact8 parse/fingerprint, negative contract, ReadOnlyPreviewReady provider/icon metadata no-load, persisted HeavyFinite read-only current resolve exact4를 추가.
// Migration:
// - Test는 memory JSON/DTO와 persisted CFAmmoData read-only load만 사용합니다. ApplyReviewed/SavePackage/AssetRegistry mutation을 수행하지 않습니다.

#include "CFDAAmmoProvider.h"
#include "CFDACommonPrimitives.h"
#include "CFDATypeDispatch.h"
#include "DataAuthoring/CFDAStaging.h"

#include "Misc/AutomationTest.h"
#include "Math/UnrealMathUtility.h"

#include <limits>

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDAAmmoProviderTestsPrivate
{
	// Synthetic Ammo Create candidate가 사용하는 canonical target object path입니다.
	static const FString SyntheticTargetPath = TEXT("/Game/Test/CarFight/DAO/DA_Ammo_ReadOnly.DA_Ammo_ReadOnly");

	// Existing UE Texture2D metadata validation fixture path입니다.
	static const FString TextureIconPath = TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture");

	// Existing wrong-class metadata validation fixture path입니다.
	static const FString WrongClassIconPath = TEXT("/Engine/BasicShapes/Cube.Cube");

	// DAO-P0-02 frozen exact8 Ammo whole-record JSON을 생성합니다.
	FString BuildAmmoJson(
		const FString& StableLogicalId = TEXT("Ammo_ReadOnly"),
		const FString& PayloadAmmoId = TEXT("Ammo_ReadOnly"),
		const FString& TargetObjectPath = SyntheticTargetPath,
		const FString& BaseFingerprintJson = TEXT("null"),
		const FString& UnitMassText = TEXT("2.0"),
		const FString& AmmoTagsJson = TEXT("[\"AP\",\"HE\"]"),
		const FString& AmmoIconJson = TEXT("null"),
		const FString& MaximumLoadableAmmoCountText = TEXT("30"),
		const FString& DisplayNameText = TEXT("테스트 탄약"),
		const FString& FamilyIdText = TEXT("TestFamily"))
	{
		return FString::Printf(
			TEXT("{")
			TEXT("\"SchemaId\":\"CarFight.DataAsset.AmmoData\",")
			TEXT("\"SchemaRevision\":1,")
			TEXT("\"AdapterContractRevision\":1,")
			TEXT("\"DataAssetTypeClassPath\":\"/Script/CarFight_Re.CFAmmoData\",")
			TEXT("\"StableLogicalId\":\"%s\",")
			TEXT("\"TargetObjectPath\":\"%s\",")
			TEXT("\"BaseSemanticFingerprint\":%s,")
			TEXT("\"Payload\":{")
			TEXT("\"AmmoId\":\"%s\",")
			TEXT("\"AmmoDisplayName\":{\"Kind\":\"Literal\",\"Text\":\"%s\"},")
			TEXT("\"AmmoFamilyId\":\"%s\",")
			TEXT("\"UnitMassKg\":%s,")
			TEXT("\"AmmoTags\":%s,")
			TEXT("\"AmmoIcon\":%s,")
			TEXT("\"MaximumLoadableAmmoCount\":%s,")
			TEXT("\"bCanBeResupplied\":true")
			TEXT("}")
			TEXT("}"),
			*StableLogicalId,
			*TargetObjectPath,
			*BaseFingerprintJson,
			*PayloadAmmoId,
			*DisplayNameText,
			*FamilyIdText,
			*UnitMassText,
			*AmmoTagsJson,
			*AmmoIconJson,
			*MaximumLoadableAmmoCountText);
	}

	// Valid Ammo record를 strict provider parser로 읽고 기본 assertion을 수행합니다.
	FCFDAAmmoParseResult ParseValidAmmo(
		FAutomationTestBase& Test,
		const FString& JsonText,
		const FString& StagingPath = TEXT("Authoring/DataAssetStaging/AmmoData/Ammo_ReadOnly.json"))
	{
		// Strict Ammo parser 결과입니다.
		const FCFDAAmmoParseResult ParseResult = CFDAAmmoProviderImpl::ParseJson(JsonText, StagingPath);
		Test.TestTrue(TEXT("Expected Ammo JSON must parse as valid whole-record"), ParseResult.bValid);
		Test.TestTrue(
			TEXT("Expected Ammo staging fingerprint must be canonical sha256"),
			CFDACommonPrimitives::IsCanonicalSha256Fingerprint(ParseResult.Record.StagingSemanticFingerprint));
		return ParseResult;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAAmmoParseFingerprintTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_02.ParseFingerprint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAAmmoNegativeContractTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_02.NegativeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAAmmoPreviewProviderTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_02.PreviewProvider",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAAmmoCurrentReadOnlyTest,
	"CarFight.DataManagement.CF_FQ_051.DAO_P0_02.CurrentReadOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// AmmoTags order/case와 FName casing이 semantic fingerprint를 바꾸지 않는지 검증합니다.
bool FCFDAAmmoParseFingerprintTest::RunTest(const FString& Parameters)
{
	using namespace CFDAAmmoProviderTestsPrivate;

	// Canonical first physical tag order의 Ammo record입니다.
	const FCFDAAmmoParseResult FirstResult = ParseValidAmmo(*this, BuildAmmoJson());
	// Reversed order + case variation을 가진 semantic-equivalent Ammo record입니다.
	const FCFDAAmmoParseResult ReorderedTagsResult = ParseValidAmmo(
		*this,
		BuildAmmoJson(
			TEXT("ammo_readonly"),
			TEXT("ammo_readonly"),
			SyntheticTargetPath,
			TEXT("null"),
			TEXT("2"),
			TEXT("[\"he\",\"ap\"]")));

	TestEqual(
		TEXT("AmmoTags physical order/case and FName case-only identity must preserve semantic fingerprint"),
		FirstResult.Record.StagingSemanticFingerprint,
		ReorderedTagsResult.Record.StagingSemanticFingerprint);
	TestEqual(TEXT("Canonicalized AmmoTags count"), FirstResult.Record.Payload.AmmoTags.Num(), 2);
	TestTrue(
		TEXT("Canonicalized AmmoTags order must be deterministic"),
		CFDACommonPrimitives::CanonicalNameText(FirstResult.Record.Payload.AmmoTags[0])
			< CFDACommonPrimitives::CanonicalNameText(FirstResult.Record.Payload.AmmoTags[1]));

	// Frozen optional semantic인 empty Literal DisplayName + NAME_None FamilyId를 함께 사용하는 valid Ammo record입니다.
	const FCFDAAmmoParseResult OptionalSemanticResult = ParseValidAmmo(
		*this,
		BuildAmmoJson(
			TEXT("Ammo_OptionalSemantic"),
			TEXT("Ammo_OptionalSemantic"),
			TEXT("/Game/Test/CarFight/DAO/DA_Ammo_OptionalSemantic.DA_Ammo_OptionalSemantic"),
			TEXT("null"),
			TEXT("2.0"),
			TEXT("[]"),
			TEXT("null"),
			TEXT("30"),
			TEXT(""),
			TEXT("")),
		TEXT("Authoring/DataAssetStaging/AmmoData/Ammo_OptionalSemantic.json"));
	TestTrue(
		TEXT("Empty Literal AmmoDisplayName must remain valid and empty"),
		OptionalSemanticResult.Record.Payload.AmmoDisplayName.Text.IsEmpty());
	TestTrue(
		TEXT("Empty AmmoFamilyId string must canonicalize to NAME_None and remain valid"),
		OptionalSemanticResult.Record.Payload.AmmoFamilyId.IsNone());
	TestTrue(
		TEXT("Optional semantic record must still produce canonical semantic fingerprint"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(OptionalSemanticResult.Record.StagingSemanticFingerprint));
	return true;
}

// Missing/unknown/type/identity/numeric/tag/icon/revision/cached-fingerprint 입력을 fail-closed 검증합니다.
bool FCFDAAmmoNegativeContractTest::RunTest(const FString& Parameters)
{
	using namespace CFDAAmmoProviderTestsPrivate;

	// Unknown top-level field가 포함된 JSON입니다.
	FString UnknownFieldJson = BuildAmmoJson();
	UnknownFieldJson.ReplaceInline(TEXT("\"SchemaId\":"), TEXT("\"Unexpected\":1,\"SchemaId\":"));
	// Unknown field parse 결과입니다.
	const FCFDAAmmoParseResult UnknownField = CFDAAmmoProviderImpl::ParseJson(UnknownFieldJson);
	TestFalse(TEXT("Unknown top-level field must be invalid"), UnknownField.bValid);
	TestTrue(TEXT("Unknown field diagnostic"), FCFDAStagingService::HasIssueCode(UnknownField.Issues, ECFDAStagingIssueCode::UnknownField));

	// Required AmmoFamilyId field를 제거한 JSON입니다.
	FString MissingFieldJson = BuildAmmoJson();
	MissingFieldJson.ReplaceInline(TEXT("\"AmmoFamilyId\":\"TestFamily\","), TEXT(""));
	// Missing field parse 결과입니다.
	const FCFDAAmmoParseResult MissingField = CFDAAmmoProviderImpl::ParseJson(MissingFieldJson);
	TestFalse(TEXT("Missing exact8 field must be invalid"), MissingField.bValid);
	TestTrue(TEXT("Missing field diagnostic"), FCFDAStagingService::HasIssueCode(MissingField.Issues, ECFDAStagingIssueCode::MissingRequiredField));

	// UnitMassKg physical type이 string인 JSON입니다.
	FString WrongTypeJson = BuildAmmoJson();
	WrongTypeJson.ReplaceInline(TEXT("\"UnitMassKg\":2.0"), TEXT("\"UnitMassKg\":\"2.0\""));
	// Wrong type parse 결과입니다.
	const FCFDAAmmoParseResult WrongType = CFDAAmmoProviderImpl::ParseJson(WrongTypeJson);
	TestFalse(TEXT("Wrong UnitMassKg JSON type must be invalid"), WrongType.bValid);
	TestTrue(TEXT("Wrong type diagnostic"), FCFDAStagingService::HasIssueCode(WrongType.Issues, ECFDAStagingIssueCode::TypeMismatch));

	// Top-level StableLogicalId와 Payload.AmmoId가 다른 JSON입니다.
	const FCFDAAmmoParseResult IdentityMismatch = CFDAAmmoProviderImpl::ParseJson(
		BuildAmmoJson(TEXT("Ammo_ReadOnly"), TEXT("Ammo_Other")));
	TestFalse(TEXT("AmmoId split authority must be invalid"), IdentityMismatch.bValid);
	TestTrue(TEXT("Ammo identity mismatch diagnostic"), FCFDAStagingService::HasIssueCode(IdentityMismatch.Issues, ECFDAStagingIssueCode::StableIdentityMismatch));

	// Negative UnitMassKg authored value입니다.
	const FCFDAAmmoParseResult NegativeMass = CFDAAmmoProviderImpl::ParseJson(
		BuildAmmoJson(TEXT("Ammo_ReadOnly"), TEXT("Ammo_ReadOnly"), SyntheticTargetPath, TEXT("null"), TEXT("-0.1")));
	TestFalse(TEXT("Negative authored UnitMassKg must be rejected without clamp"), NegativeMass.bValid);
	TestTrue(TEXT("Negative mass diagnostic"), FCFDAStagingService::HasIssueCode(NegativeMass.Issues, ECFDAStagingIssueCode::InvalidValue));

	// Negative MaximumLoadableAmmoCount authored value입니다.
	const FCFDAAmmoParseResult NegativeCount = CFDAAmmoProviderImpl::ParseJson(
		BuildAmmoJson(
			TEXT("Ammo_ReadOnly"),
			TEXT("Ammo_ReadOnly"),
			SyntheticTargetPath,
			TEXT("null"),
			TEXT("2.0"),
			TEXT("[]"),
			TEXT("null"),
			TEXT("-1")));
	TestFalse(TEXT("Negative authored MaximumLoadableAmmoCount must be rejected without clamp"), NegativeCount.bValid);
	TestTrue(TEXT("Negative count diagnostic"), FCFDAStagingService::HasIssueCode(NegativeCount.Issues, ECFDAStagingIssueCode::InvalidValue));

	// Case-insensitive semantic duplicate AmmoTags입니다.
	const FCFDAAmmoParseResult DuplicateTags = CFDAAmmoProviderImpl::ParseJson(
		BuildAmmoJson(
			TEXT("Ammo_ReadOnly"),
			TEXT("Ammo_ReadOnly"),
			SyntheticTargetPath,
			TEXT("null"),
			TEXT("2.0"),
			TEXT("[\"AP\",\"ap\"]")));
	TestFalse(TEXT("Semantic duplicate AmmoTags must be rejected"), DuplicateTags.bValid);
	TestTrue(TEXT("Duplicate AmmoTags diagnostic"), FCFDAStagingService::HasIssueCode(DuplicateTags.Issues, ECFDAStagingIssueCode::InvalidValue));

	// Subobject/non-canonical AmmoIcon string입니다.
	const FCFDAAmmoParseResult MalformedIcon = CFDAAmmoProviderImpl::ParseJson(
		BuildAmmoJson(
			TEXT("Ammo_ReadOnly"),
			TEXT("Ammo_ReadOnly"),
			SyntheticTargetPath,
			TEXT("null"),
			TEXT("2.0"),
			TEXT("[]"),
			TEXT("\"/Engine/EngineResources/DefaultTexture.DefaultTexture:Subobject\"")));
	TestFalse(TEXT("AmmoIcon subobject path must be rejected by strict parser"), MalformedIcon.bValid);
	TestTrue(TEXT("Malformed AmmoIcon diagnostic"), FCFDAStagingService::HasIssueCode(MalformedIcon.Issues, ECFDAStagingIssueCode::InvalidValue));

	// Wrong exact DataAssetTypeClassPath입니다.
	FString WrongClassPathJson = BuildAmmoJson();
	WrongClassPathJson.ReplaceInline(TEXT("/Script/CarFight_Re.CFAmmoData"), TEXT("/Script/CarFight_Re.CFDamageData"));
	// Wrong class path parse 결과입니다.
	const FCFDAAmmoParseResult WrongClassPath = CFDAAmmoProviderImpl::ParseJson(WrongClassPathJson);
	TestFalse(TEXT("Wrong DataAssetTypeClassPath must fail exact TypeKey lookup"), WrongClassPath.bValid);
	TestTrue(TEXT("Wrong TypeKey diagnostic"), FCFDAStagingService::HasIssueCode(WrongClassPath.Issues, ECFDAStagingIssueCode::SchemaUnsupported));

	// Wrong SchemaRevision입니다.
	FString WrongSchemaRevisionJson = BuildAmmoJson();
	WrongSchemaRevisionJson.ReplaceInline(TEXT("\"SchemaRevision\":1"), TEXT("\"SchemaRevision\":2"));
	// Wrong schema revision parse 결과입니다.
	const FCFDAAmmoParseResult WrongSchemaRevision = CFDAAmmoProviderImpl::ParseJson(WrongSchemaRevisionJson);
	TestFalse(TEXT("Wrong Ammo SchemaRevision must be rejected"), WrongSchemaRevision.bValid);
	TestTrue(TEXT("Wrong schema revision diagnostic"), FCFDAStagingService::HasIssueCode(WrongSchemaRevision.Issues, ECFDAStagingIssueCode::SchemaRevisionUnsupported));

	// Wrong AdapterContractRevision입니다.
	FString WrongAdapterRevisionJson = BuildAmmoJson();
	WrongAdapterRevisionJson.ReplaceInline(TEXT("\"AdapterContractRevision\":1"), TEXT("\"AdapterContractRevision\":2"));
	// Wrong adapter revision parse 결과입니다.
	const FCFDAAmmoParseResult WrongAdapterRevision = CFDAAmmoProviderImpl::ParseJson(WrongAdapterRevisionJson);
	TestFalse(TEXT("Wrong Ammo AdapterContractRevision must be rejected"), WrongAdapterRevision.bValid);
	TestTrue(TEXT("Wrong adapter revision diagnostic"), FCFDAStagingService::HasIssueCode(WrongAdapterRevision.Issues, ECFDAStagingIssueCode::AdapterRevisionMismatch));

	// Valid typed record를 mutable payload/cached-fingerprint split 상태로 만든 fixture입니다.
	FCFDAAmmoParseResult SplitRecordResult = ParseValidAmmo(*this, BuildAmmoJson());
	SplitRecordResult.Record.Payload.MaximumLoadableAmmoCount += 1;
	// Split-state 검증 오류입니다.
	FString SplitIntegrityError;
	TestFalse(
		TEXT("Provider-local payload/cached fingerprint split must fail-closed"),
		CFDAAmmoProviderImpl::ValidateRecordIntegrity(SplitRecordResult.Record, SplitIntegrityError));

	// Non-finite typed UnitMassKg fingerprint fixture입니다.
	FCFDAAmmoPayload NonFinitePayload = SplitRecordResult.Record.Payload;
	NonFinitePayload.UnitMassKg = std::numeric_limits<float>::infinity();
	// Non-finite fingerprint output입니다.
	FString NonFiniteFingerprint;
	// Non-finite fingerprint failure reason입니다.
	FString NonFiniteError;
	TestFalse(
		TEXT("Non-finite typed UnitMassKg must not fingerprint"),
		CFDAAmmoProviderImpl::BuildSemanticFingerprint(NonFinitePayload, NonFiniteFingerprint, NonFiniteError));
	return true;
}

// Production Ammo provider registration/readiness, reference metadata no-load와 shared Create Preview를 검증합니다.
bool FCFDAAmmoPreviewProviderTest::RunTest(const FString& Parameters)
{
	using namespace CFDAAmmoProviderTestsPrivate;

	// Production provider registry 전체 validation 오류입니다.
	FString RegistryError;
	TestTrue(TEXT("Missile + Ammo production provider registry must validate"), CFDATypeDispatch::ValidateProviderRegistry(RegistryError));

	// Exact Ammo TypeKey registry lookup 오류입니다.
	FString LookupError;
	// Exact Ammo production provider entry입니다.
	const FCFDATypeProviderEntry* ProviderEntry = CFDATypeDispatch::FindExactProviderEntry(
		TEXT("CarFight.DataAsset.AmmoData"),
		TEXT("/Script/CarFight_Re.CFAmmoData"),
		&LookupError);
	TestTrue(TEXT("Ammo exact TypeKey must resolve production provider"), ProviderEntry == &CFDAAmmoProvider::GetProvider());
	if (ProviderEntry == nullptr)
	{
		return false;
	}
	TestEqual(TEXT("Ammo provider must be ReviewedMutationReady after DAO-P0-03 writer enablement"), ProviderEntry->Readiness, ECFDAProviderReadiness::ReviewedMutationReady);

	// Mutation-ready validation 오류입니다.
	FString MutationReadyError;
	TestTrue(
		TEXT("DAO-P0-03 Ammo provider must pass Reviewed mutation readiness"),
		CFDATypeDispatch::ValidateProviderMutationReady(*ProviderEntry, MutationReadyError));
	TestTrue(TEXT("DAO-P0-03 Ammo provider mutation callback must be registered"), ProviderEntry->Operations.ApplyReviewedMutation != nullptr);

	// Reference validator 호출 전 existing icon의 process-local load state입니다.
	const FSoftObjectPath IconPath(TextureIconPath);
	// ResolveObject는 load하지 않는 observation이며 before-state pointer입니다.
	UObject* IconBefore = IconPath.ResolveObject();
	// Existing Texture2D icon을 가진 valid desired JSON입니다.
	const FString TextureIconJson = FString::Printf(TEXT("\"%s\""), *TextureIconPath);
	// Payload-free candidate output입니다.
	FCFDACommonEnvelope Envelope;
	// Provider-local validation diagnostics입니다.
	TArray<FCFDAStagingIssue> Issues;
	TestTrue(
		TEXT("Existing Texture2D metadata icon must pass Ammo ParseCommonCandidate"),
		CFDAAmmoProviderImpl::ParseCommonCandidate(
			BuildAmmoJson(
				TEXT("Ammo_ReadOnly"),
				TEXT("Ammo_ReadOnly"),
				SyntheticTargetPath,
				TEXT("null"),
				TEXT("2.0"),
				TEXT("[]"),
				TextureIconJson),
			TEXT("Authoring/DataAssetStaging/AmmoData/Ammo_ReadOnly.json"),
			Envelope,
			Issues));
	// Reference validation 뒤 process-local load state입니다.
	UObject* IconAfter = IconPath.ResolveObject();
	TestTrue(TEXT("AmmoIcon metadata validation must not change referenced asset load state"), IconBefore == IconAfter);

	// Synthetic absent target current truth입니다.
	FCFDACommonCurrentState CurrentState;
	// Current resolver diagnostics입니다.
	TArray<FCFDAStagingIssue> CurrentIssues;
	TestTrue(
		TEXT("Synthetic Ammo current-state resolve must succeed read-only"),
		CFDAAmmoProviderImpl::ResolveCommonCurrentState(Envelope, CurrentState, CurrentIssues));
	TestFalse(TEXT("Synthetic Ammo target must remain absent"), CurrentState.bRequestedTargetExists);
	TestFalse(TEXT("Synthetic Ammo identity must remain absent"), CurrentState.bStableIdentityExists);

	// Shared payload-free Preview 결과입니다.
	const FCFDACommonPreviewRow PreviewRow = CFDATypeDispatch::BuildCommonPreview(Envelope, CurrentState);
	TestEqual(TEXT("Valid absent Ammo candidate must classify Create"), PreviewRow.Kind, ECFDAStagingPreviewKind::Create);

	// BatchPlanHash integration의 두 번째 actual Ammo provider candidate입니다.
	FCFDACommonEnvelope SecondHashEnvelope;
	// 두 번째 actual Ammo candidate parse diagnostics입니다.
	TArray<FCFDAStagingIssue> SecondHashIssues;
	const bool bSecondHashCandidateParsed = CFDAAmmoProviderImpl::ParseCommonCandidate(
		BuildAmmoJson(
			TEXT("Ammo_HashSecond"),
			TEXT("Ammo_HashSecond"),
			TEXT("/Game/Test/CarFight/DAO/DA_Ammo_HashSecond.DA_Ammo_HashSecond"),
			TEXT("null"),
			TEXT("3.0"),
			TEXT("[\"Practice\"]"),
			TEXT("null"),
			TEXT("12"),
			TEXT("Hash Second"),
			TEXT("HashFamily")),
		TEXT("Authoring/DataAssetStaging/AmmoData/Ammo_HashSecond.json"),
		SecondHashEnvelope,
		SecondHashIssues);
	TestTrue(TEXT("Second actual Ammo provider candidate must parse for BatchPlanHash integration"), bSecondHashCandidateParsed);
	if (!bSecondHashCandidateParsed)
	{
		return false;
	}

	// 두 번째 candidate의 synthetic absent current truth입니다.
	FCFDACommonCurrentState SecondHashCurrentState;
	// 두 번째 actual Ammo shared Preview row입니다.
	const FCFDACommonPreviewRow SecondHashPreviewRow = CFDATypeDispatch::BuildCommonPreview(
		SecondHashEnvelope,
		SecondHashCurrentState);
	TestEqual(
		TEXT("Second actual Ammo candidate must classify Create"),
		SecondHashPreviewRow.Kind,
		ECFDAStagingPreviewKind::Create);

	// Actual Ammo provider가 만든 두 common Preview row의 forward physical order입니다.
	TArray<FCFDACommonPreviewRow> ForwardAmmoRows;
	ForwardAmmoRows.Add(PreviewRow);
	ForwardAmmoRows.Add(SecondHashPreviewRow);
	// Forward order BatchPlanHash입니다.
	FString ForwardAmmoBatchPlanHash;
	// Forward order hash 실패 이유입니다.
	FString ForwardAmmoBatchPlanHashError;
	TestTrue(
		TEXT("Actual Ammo provider Preview rows must build a BatchPlanHash"),
		CFDATypeDispatch::BuildCommonBatchPlanHash(
			ForwardAmmoRows,
			ForwardAmmoBatchPlanHash,
			ForwardAmmoBatchPlanHashError));
	TestTrue(
		TEXT("Actual Ammo BatchPlanHash must be canonical SHA-256"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(ForwardAmmoBatchPlanHash));

	// 동일한 actual Ammo rows의 reversed physical order입니다.
	TArray<FCFDACommonPreviewRow> ReverseAmmoRows;
	ReverseAmmoRows.Add(SecondHashPreviewRow);
	ReverseAmmoRows.Add(PreviewRow);
	// Reverse order BatchPlanHash입니다.
	FString ReverseAmmoBatchPlanHash;
	// Reverse order hash 실패 이유입니다.
	FString ReverseAmmoBatchPlanHashError;
	TestTrue(
		TEXT("Reversed actual Ammo provider Preview rows must build a BatchPlanHash"),
		CFDATypeDispatch::BuildCommonBatchPlanHash(
			ReverseAmmoRows,
			ReverseAmmoBatchPlanHash,
			ReverseAmmoBatchPlanHashError));
	TestEqual(
		TEXT("Actual Ammo BatchPlanHash must be deterministic across physical row order"),
		ForwardAmmoBatchPlanHash,
		ReverseAmmoBatchPlanHash);

	// Existing wrong-class icon JSON입니다.
	const FString WrongClassIconJson = FString::Printf(TEXT("\"%s\""), *WrongClassIconPath);
	// Wrong-class candidate output입니다.
	FCFDACommonEnvelope WrongClassEnvelope;
	// Wrong-class provider diagnostics입니다.
	TArray<FCFDAStagingIssue> WrongClassIssues;
	TestFalse(
		TEXT("Existing non-Texture2D icon metadata must block Ammo candidate"),
		CFDAAmmoProviderImpl::ParseCommonCandidate(
			BuildAmmoJson(
				TEXT("Ammo_WrongIcon"),
				TEXT("Ammo_WrongIcon"),
				TEXT("/Game/Test/CarFight/DAO/DA_Ammo_WrongIcon.DA_Ammo_WrongIcon"),
				TEXT("null"),
				TEXT("2.0"),
				TEXT("[]"),
				WrongClassIconJson),
			TEXT("Authoring/DataAssetStaging/AmmoData/Ammo_WrongIcon.json"),
			WrongClassEnvelope,
			WrongClassIssues));
	TestTrue(TEXT("Wrong-class icon emits InvalidValue"), FCFDAStagingService::HasIssueCode(WrongClassIssues, ECFDAStagingIssueCode::InvalidValue));

	// Canonical syntax지만 Asset Registry에 존재하지 않는 missing icon path입니다.
	const FString MissingIconPath = TEXT("/Game/CarFight/Tests/DefinitelyMissing/DAO_AmmoIcon.DAO_AmmoIcon");
	// Missing icon JSON representation입니다.
	const FString MissingIconJson = FString::Printf(TEXT("\"%s\""), *MissingIconPath);
	// Missing icon candidate output입니다.
	FCFDACommonEnvelope MissingIconEnvelope;
	// Missing icon provider diagnostics입니다.
	TArray<FCFDAStagingIssue> MissingIconIssues;
	TestFalse(
		TEXT("Canonical but missing AmmoIcon metadata must block Ammo candidate"),
		CFDAAmmoProviderImpl::ParseCommonCandidate(
			BuildAmmoJson(
				TEXT("Ammo_MissingIcon"),
				TEXT("Ammo_MissingIcon"),
				TEXT("/Game/Test/CarFight/DAO/DA_Ammo_MissingIcon.DA_Ammo_MissingIcon"),
				TEXT("null"),
				TEXT("2.0"),
				TEXT("[]"),
				MissingIconJson),
			TEXT("Authoring/DataAssetStaging/AmmoData/Ammo_MissingIcon.json"),
			MissingIconEnvelope,
			MissingIconIssues));
	TestTrue(TEXT("Missing icon emits InvalidValue"), FCFDAStagingService::HasIssueCode(MissingIconIssues, ECFDAStagingIssueCode::InvalidValue));

	// Provider root 밖 sibling Staging source입니다.
	FCFDACommonEnvelope OutsideRootEnvelope;
	// Sibling root validation diagnostics입니다.
	TArray<FCFDAStagingIssue> OutsideRootIssues;
	TestFalse(
		TEXT("Ammo provider must reject sibling Staging root"),
		CFDAAmmoProviderImpl::ParseCommonCandidate(
			BuildAmmoJson(),
			TEXT("Authoring/DataAssetStaging/MissileGuidePreset/Ammo_ReadOnly.json"),
			OutsideRootEnvelope,
			OutsideRootIssues));
	return true;
}

// Persisted test-owned HeavyFinite AmmoData를 Save 없이 exact typed current truth로 읽고 NoChange Preview를 검증합니다.
bool FCFDAAmmoCurrentReadOnlyTest::RunTest(const FString& Parameters)
{
	using namespace CFDAAmmoProviderTestsPrivate;

	// Current test-owned durable fixture exact object path입니다.
	const FString HeavyFiniteTarget = TEXT("/Game/CarFight/Tests/AmmoIntegration/DA_Ammo_HeavyFinite.DA_Ammo_HeavyFinite");
	// Persisted HeavyFinite authored exact8 값과 동일한 desired JSON입니다.
	const FString HeavyFiniteJson = BuildAmmoJson(
		TEXT("HeavyShell_Finite"),
		TEXT("HeavyShell_Finite"),
		HeavyFiniteTarget,
		TEXT("null"),
		TEXT("2.0"),
		TEXT("[]"),
		TEXT("null"),
		TEXT("30"),
		TEXT("중포탄"),
		TEXT("HeavyShell"));

	// Read-only provider candidate입니다.
	FCFDACommonEnvelope Envelope;
	// Candidate parse diagnostics입니다.
	TArray<FCFDAStagingIssue> ParseIssues;
	TestTrue(
		TEXT("Persisted HeavyFinite desired JSON must pass read-only candidate parse"),
		CFDAAmmoProviderImpl::ParseCommonCandidate(
			HeavyFiniteJson,
			TEXT("Authoring/DataAssetStaging/AmmoData/HeavyShell_Finite.json"),
			Envelope,
			ParseIssues));

	// Persisted HeavyFinite exact current state입니다.
	FCFDACommonCurrentState CurrentState;
	// Current resolver diagnostics입니다.
	TArray<FCFDAStagingIssue> CurrentIssues;
	TestTrue(
		TEXT("Persisted HeavyFinite current resolver must succeed without mutation"),
		CFDAAmmoProviderImpl::ResolveCommonCurrentState(Envelope, CurrentState, CurrentIssues));
	TestTrue(TEXT("Persisted HeavyFinite target must exist"), CurrentState.bRequestedTargetExists);
	TestFalse(TEXT("Persisted HeavyFinite target must remain package-clean"), CurrentState.bRequestedTargetDirty);
	TestEqual(TEXT("Persisted HeavyFinite StableLogicalId"), CurrentState.RequestedTargetStableLogicalId, FName(TEXT("HeavyShell_Finite")));
	TestEqual(TEXT("Persisted HeavyFinite identity match count"), CurrentState.StableIdentityMatchCount, 1);
	TestEqual(
		TEXT("Persisted HeavyFinite typed current semantic must equal desired staging semantic"),
		CurrentState.CurrentSemanticFingerprint,
		Envelope.StagingSemanticFingerprint);

	// Existing target Update/NoChange classification용 baseline-bound envelope입니다.
	FCFDACommonEnvelope BaselineEnvelope = Envelope;
	BaselineEnvelope.bHasBaseSemanticFingerprint = true;
	BaselineEnvelope.BaseSemanticFingerprint = CurrentState.CurrentSemanticFingerprint;
	// Shared exact 3-way NoChange Preview입니다.
	const FCFDACommonPreviewRow PreviewRow = CFDATypeDispatch::BuildCommonPreview(BaselineEnvelope, CurrentState);
	TestEqual(TEXT("HeavyFinite Base==Current==Staging must classify NoChange"), PreviewRow.Kind, ECFDAStagingPreviewKind::NoChange);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
