// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAContractMigTests.cpp
// Version: v1.0.1
// Date: 2026-09-09
// Description: CF-FQ-050 DACE-P0-04 canonical Staging compatibility, migration Resolution/Evidence와 append/promotion gate focused Automation입니다.
// Changelog:
// - v1.0.1: NoMigration의 invalid Resolution/Evidence와 combined Staging+Product Resolved+evidence positive matrix를 직접 고정했습니다.
// - v1.0.0: canonical Product Staging exact3 read-only parse, old revision fail-closed, Staging/Product Pending, Resolved evidence와 accepted append/Current promotion negative regression을 최초 추가.
// Migration:
// - 모든 negative fixture는 memory-only JSON/declaration/snapshot copy입니다. Product Staging JSON, Product UObject/package와 production accepted history를 수정하거나 저장하지 않습니다.

#include "CFDAContractGuard.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDAContractMigTestsPrivate
{
	// Canonical Product Low Staging 상대 경로입니다.
	static constexpr TCHAR CanonicalLowStagingPath[] = TEXT("Authoring/DataAssetStaging/MissileGuidePreset/MissileFeel_Low.json");

	// 기존 canonical SHA-256과 다른 canonical synthetic component signature를 만듭니다.
	FString BuildAlternateCanonicalSignature(const FString& Signature)
	{
		// 마지막 digest nibble만 바꿀 mutable canonical copy입니다.
		FString AlternateSignature = Signature;
		if (AlternateSignature.Len() == 71)
		{
			// 기존 마지막 digest nibble입니다.
			const TCHAR LastCharacter = AlternateSignature[70];
			AlternateSignature[70] = LastCharacter == static_cast<TCHAR>('0') ? static_cast<TCHAR>('1') : static_cast<TCHAR>('0');
		}
		return AlternateSignature;
	}

	// Synthetic accepted snapshot의 deterministic record signature를 다시 계산해 저장합니다.
	bool FinalizeSnapshotSignature(FCFDAAcceptedContractSnapshot& InOutSnapshot, FString& OutError)
	{
		// Snapshot fields에서 재계산한 deterministic record signature입니다.
		FString SnapshotSignature;
		if (!FCFDAContractGuard::BuildSnapshotSignature(InOutSnapshot, SnapshotSignature, OutError))
		{
			return false;
		}
		InOutSnapshot.SnapshotSignature = SnapshotSignature;
		return true;
	}

	// Migration gate fixture용 explicit declaration을 생성합니다.
	FCFDACurrentChangeDeclaration BuildGateDeclaration(
		const ECFDAContractMigrationImpact Impact,
		const ECFDAContractMigrationResolution Resolution,
		const FString& MigrationEvidenceId)
	{
		// 반환할 explicit migration declaration입니다.
		FCFDACurrentChangeDeclaration Declaration;
		Declaration.BaseSnapshotId = TEXT("DACE-P0-04-Memory-Base");
		Declaration.CandidateContractSignature = TEXT("sha256:0000000000000000000000000000000000000000000000000000000000000000");
		Declaration.bImpactDeclared = true;
		Declaration.Impact = Impact;
		Declaration.Resolution = Resolution;
		Declaration.MigrationEvidenceId = MigrationEvidenceId;
		return Declaration;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDACanonicalStagingGateTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_04.CanonicalStaging",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAResolutionEvidenceGateTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_04.ResolutionEvidence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAPromotionAppendGateTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_04.PromotionAppend",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Current canonical exact3을 read-only strict parse하고 old Schema/Adapter revision이 silent reinterpret되지 않는지 검증합니다.
bool FCFDACanonicalStagingGateTest::RunTest(const FString& Parameters)
{
	using namespace CFDAContractMigTestsPrivate;

	// Production canonical Product Staging exact3의 current compatibility 결과입니다.
	const FCFDAContractGuardResult CurrentCanonicalResult = FCFDAContractGuard::ValidateCurrentCanonicalStagingCompatibility();
	TestTrue(TEXT("Current canonical Product Staging exact3 must pass read-only strict compatibility"), CurrentCanonicalResult.bPassed);
	if (!CurrentCanonicalResult.bPassed)
	{
		for (const FCFDAContractGuardIssue& Issue : CurrentCanonicalResult.Issues)
		{
			AddError(FString::Printf(TEXT("%s: %s"), *Issue.FieldPath, *Issue.Message));
		}
		return false;
	}

	// CarFight .uproject가 위치한 `<main_game>/UE/` directory입니다.
	const FString UnrealProjectRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	// canonical Staging authority가 위치한 `<main_game>/` directory입니다.
	const FString MainGameRoot = FPaths::ConvertRelativePathToFull(FPaths::Combine(UnrealProjectRoot, TEXT("..")));
	// Low canonical Staging physical read path입니다.
	const FString CanonicalLowPhysicalPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(MainGameRoot, CanonicalLowStagingPath));
	// Low canonical Staging 원문을 보존하는 read-only text입니다.
	FString CanonicalLowJson;
	TestTrue(TEXT("Canonical Low Staging must be readable without mutation"), FFileHelper::LoadFileToString(CanonicalLowJson, *CanonicalLowPhysicalPath));
	if (CanonicalLowJson.IsEmpty())
	{
		return false;
	}

	// SchemaRevision만 old value로 바꾼 memory-only JSON fixture입니다.
	FString OldSchemaJson = CanonicalLowJson;
	// SchemaRevision fixture 치환 횟수입니다.
	const int32 SchemaReplacementCount = OldSchemaJson.ReplaceInline(TEXT("\"SchemaRevision\": 1"), TEXT("\"SchemaRevision\": 0"), ESearchCase::CaseSensitive);
	TestEqual(TEXT("SchemaRevision memory fixture must replace exactly one field"), SchemaReplacementCount, 1);
	// Guard가 old SchemaRevision을 migration pending으로 번역한 결과입니다.
	const FCFDAContractGuardResult OldSchemaGuardResult = FCFDAContractGuard::ValidateStagingJsonCompatibility(OldSchemaJson, CanonicalLowStagingPath);
	TestFalse(TEXT("Old SchemaRevision must fail migration compatibility"), OldSchemaGuardResult.bPassed);
	TestTrue(TEXT("Old SchemaRevision must report StagingMigrationPending"), FCFDAContractGuard::HasIssueCode(OldSchemaGuardResult, ECFDAContractIssueCode::StagingMigrationPending));
	// Production strict parser 자체의 old SchemaRevision 결과입니다.
	const FCFDAStagingParseResult OldSchemaParseResult = FCFDAStagingService::ParseMissilePresetJson(OldSchemaJson, CanonicalLowStagingPath);
	TestFalse(TEXT("Production parser must reject old SchemaRevision"), OldSchemaParseResult.bValid);
	TestTrue(TEXT("Old SchemaRevision must keep SchemaRevisionUnsupported evidence"), FCFDAStagingService::HasIssueCode(OldSchemaParseResult.Issues, ECFDAStagingIssueCode::SchemaRevisionUnsupported));

	// AdapterContractRevision만 old value로 바꾼 memory-only JSON fixture입니다.
	FString OldAdapterJson = CanonicalLowJson;
	// AdapterContractRevision fixture 치환 횟수입니다.
	const int32 AdapterReplacementCount = OldAdapterJson.ReplaceInline(TEXT("\"AdapterContractRevision\": 2"), TEXT("\"AdapterContractRevision\": 1"), ESearchCase::CaseSensitive);
	TestEqual(TEXT("AdapterContractRevision memory fixture must replace exactly one field"), AdapterReplacementCount, 1);
	// Guard가 old AdapterContractRevision을 migration pending으로 번역한 결과입니다.
	const FCFDAContractGuardResult OldAdapterGuardResult = FCFDAContractGuard::ValidateStagingJsonCompatibility(OldAdapterJson, CanonicalLowStagingPath);
	TestFalse(TEXT("Old AdapterContractRevision must fail migration compatibility"), OldAdapterGuardResult.bPassed);
	TestTrue(TEXT("Old AdapterContractRevision must report StagingMigrationPending"), FCFDAContractGuard::HasIssueCode(OldAdapterGuardResult, ECFDAContractIssueCode::StagingMigrationPending));
	// Production strict parser 자체의 old AdapterContractRevision 결과입니다.
	const FCFDAStagingParseResult OldAdapterParseResult = FCFDAStagingService::ParseMissilePresetJson(OldAdapterJson, CanonicalLowStagingPath);
	TestFalse(TEXT("Production parser must reject old AdapterContractRevision"), OldAdapterParseResult.bValid);
	TestTrue(TEXT("Old AdapterContractRevision must keep AdapterRevisionMismatch evidence"), FCFDAStagingService::HasIssueCode(OldAdapterParseResult.Issues, ECFDAStagingIssueCode::AdapterRevisionMismatch));
	return true;
}

// Migration impact별 Pending/Resolved/Evidence 조합이 append/promotion gate를 fail-closed하는지 검증합니다.
bool FCFDAResolutionEvidenceGateTest::RunTest(const FString& Parameters)
{
	using namespace CFDAContractMigTestsPrivate;

	// Current-compatible canonical Staging을 나타내는 empty PASS 결과입니다.
	const FCFDAContractGuardResult CompatibleCanonicalResult;

	// NoMigration인데 Pending Resolution을 잘못 남긴 declaration입니다.
	const FCFDACurrentChangeDeclaration InvalidNoMigrationResolutionDeclaration = BuildGateDeclaration(ECFDAContractMigrationImpact::NoMigration, ECFDAContractMigrationResolution::Pending, TEXT(""));
	// Invalid NoMigration Resolution gate 결과입니다.
	const FCFDAMigrationGateResult InvalidNoMigrationResolutionGate = FCFDAContractGuard::EvaluateMigrationGate(&InvalidNoMigrationResolutionDeclaration, CompatibleCanonicalResult);
	TestFalse(TEXT("NoMigration with Pending Resolution must block accepted append"), InvalidNoMigrationResolutionGate.bAcceptedSnapshotAppendAllowed);
	TestFalse(TEXT("NoMigration with Pending Resolution must block Current System promotion"), InvalidNoMigrationResolutionGate.bCurrentSystemPromotionAllowed);
	TestTrue(TEXT("NoMigration with Pending Resolution must fail machine gate"), FCFDAContractGuard::HasIssueCode(InvalidNoMigrationResolutionGate.Validation, ECFDAContractIssueCode::MigrationImpactUndeclared));

	// NoMigration인데 불필요한 evidence를 남긴 declaration입니다.
	const FCFDACurrentChangeDeclaration InvalidNoMigrationEvidenceDeclaration = BuildGateDeclaration(ECFDAContractMigrationImpact::NoMigration, ECFDAContractMigrationResolution::NotRequired, TEXT("DACE-P0-04-Unnecessary-Evidence"));
	// Invalid NoMigration evidence gate 결과입니다.
	const FCFDAMigrationGateResult InvalidNoMigrationEvidenceGate = FCFDAContractGuard::EvaluateMigrationGate(&InvalidNoMigrationEvidenceDeclaration, CompatibleCanonicalResult);
	TestFalse(TEXT("NoMigration with evidence must block accepted append"), InvalidNoMigrationEvidenceGate.bAcceptedSnapshotAppendAllowed);
	TestFalse(TEXT("NoMigration with evidence must block Current System promotion"), InvalidNoMigrationEvidenceGate.bCurrentSystemPromotionAllowed);
	TestTrue(TEXT("NoMigration with evidence must fail machine gate"), FCFDAContractGuard::HasIssueCode(InvalidNoMigrationEvidenceGate.Validation, ECFDAContractIssueCode::MigrationImpactUndeclared));

	// Pending Staging migration declaration입니다.
	const FCFDACurrentChangeDeclaration PendingStagingDeclaration = BuildGateDeclaration(ECFDAContractMigrationImpact::StagingMigrationRequired, ECFDAContractMigrationResolution::Pending, TEXT(""));
	// Pending Staging migration gate 결과입니다.
	const FCFDAMigrationGateResult PendingStagingGate = FCFDAContractGuard::EvaluateMigrationGate(&PendingStagingDeclaration, CompatibleCanonicalResult);
	TestFalse(TEXT("Pending Staging migration must block accepted append"), PendingStagingGate.bAcceptedSnapshotAppendAllowed);
	TestFalse(TEXT("Pending Staging migration must block Current System promotion"), PendingStagingGate.bCurrentSystemPromotionAllowed);
	TestTrue(TEXT("Pending Staging migration must report StagingMigrationPending"), FCFDAContractGuard::HasIssueCode(PendingStagingGate.Validation, ECFDAContractIssueCode::StagingMigrationPending));

	// Pending Product migration review declaration입니다.
	const FCFDACurrentChangeDeclaration PendingProductDeclaration = BuildGateDeclaration(ECFDAContractMigrationImpact::ProductMigrationReviewRequired, ECFDAContractMigrationResolution::Pending, TEXT(""));
	// Pending Product migration review gate 결과입니다.
	const FCFDAMigrationGateResult PendingProductGate = FCFDAContractGuard::EvaluateMigrationGate(&PendingProductDeclaration, CompatibleCanonicalResult);
	TestFalse(TEXT("Pending Product review must block accepted append"), PendingProductGate.bAcceptedSnapshotAppendAllowed);
	TestFalse(TEXT("Pending Product review must block Current System promotion"), PendingProductGate.bCurrentSystemPromotionAllowed);
	TestTrue(TEXT("Pending Product review must report ProductMigrationReviewPending"), FCFDAContractGuard::HasIssueCode(PendingProductGate.Validation, ECFDAContractIssueCode::ProductMigrationReviewPending));

	// Pending combined migration declaration입니다.
	const FCFDACurrentChangeDeclaration PendingCombinedDeclaration = BuildGateDeclaration(ECFDAContractMigrationImpact::StagingAndProductMigrationReviewRequired, ECFDAContractMigrationResolution::Pending, TEXT(""));
	// Pending combined gate 결과입니다.
	const FCFDAMigrationGateResult PendingCombinedGate = FCFDAContractGuard::EvaluateMigrationGate(&PendingCombinedDeclaration, CompatibleCanonicalResult);
	TestTrue(TEXT("Combined Pending must report StagingMigrationPending"), FCFDAContractGuard::HasIssueCode(PendingCombinedGate.Validation, ECFDAContractIssueCode::StagingMigrationPending));
	TestTrue(TEXT("Combined Pending must report ProductMigrationReviewPending"), FCFDAContractGuard::HasIssueCode(PendingCombinedGate.Validation, ECFDAContractIssueCode::ProductMigrationReviewPending));

	// Resolved이지만 evidence가 없는 Product review declaration입니다.
	const FCFDACurrentChangeDeclaration MissingEvidenceDeclaration = BuildGateDeclaration(ECFDAContractMigrationImpact::ProductMigrationReviewRequired, ECFDAContractMigrationResolution::Resolved, TEXT(""));
	// Missing evidence gate 결과입니다.
	const FCFDAMigrationGateResult MissingEvidenceGate = FCFDAContractGuard::EvaluateMigrationGate(&MissingEvidenceDeclaration, CompatibleCanonicalResult);
	TestFalse(TEXT("Resolved migration without evidence must block accepted append"), MissingEvidenceGate.bAcceptedSnapshotAppendAllowed);
	TestFalse(TEXT("Resolved migration without evidence must block Current System promotion"), MissingEvidenceGate.bCurrentSystemPromotionAllowed);
	TestTrue(TEXT("Resolved migration without evidence must fail machine gate"), FCFDAContractGuard::HasIssueCode(MissingEvidenceGate.Validation, ECFDAContractIssueCode::MigrationImpactUndeclared));

	// Resolved+evidence Staging migration declaration입니다.
	const FCFDACurrentChangeDeclaration ResolvedStagingDeclaration = BuildGateDeclaration(ECFDAContractMigrationImpact::StagingMigrationRequired, ECFDAContractMigrationResolution::Resolved, TEXT("DACE-P0-04-Staging-Evidence"));
	// Resolved+evidence Staging migration gate 결과입니다.
	const FCFDAMigrationGateResult ResolvedStagingGate = FCFDAContractGuard::EvaluateMigrationGate(&ResolvedStagingDeclaration, CompatibleCanonicalResult);
	TestTrue(TEXT("Resolved Staging migration with evidence and compatible canonical Staging may append"), ResolvedStagingGate.bAcceptedSnapshotAppendAllowed);
	TestTrue(TEXT("Resolved Staging migration with evidence and compatible canonical Staging may promote"), ResolvedStagingGate.bCurrentSystemPromotionAllowed);

	// Resolved+evidence combined Staging+Product declaration입니다.
	const FCFDACurrentChangeDeclaration ResolvedCombinedDeclaration = BuildGateDeclaration(ECFDAContractMigrationImpact::StagingAndProductMigrationReviewRequired, ECFDAContractMigrationResolution::Resolved, TEXT("DACE-P0-04-Combined-Evidence"));
	// Resolved+evidence combined migration gate 결과입니다.
	const FCFDAMigrationGateResult ResolvedCombinedGate = FCFDAContractGuard::EvaluateMigrationGate(&ResolvedCombinedDeclaration, CompatibleCanonicalResult);
	TestTrue(TEXT("Resolved combined migration with evidence and compatible canonical Staging may append"), ResolvedCombinedGate.bAcceptedSnapshotAppendAllowed);
	TestTrue(TEXT("Resolved combined migration with evidence and compatible canonical Staging may promote"), ResolvedCombinedGate.bCurrentSystemPromotionAllowed);

	// Resolved이지만 strict-compatible canonical Staging이 없는 상태를 만드는 memory-only invalid result입니다.
	const FCFDAContractGuardResult IncompatibleCanonicalResult = FCFDAContractGuard::ValidateStagingJsonCompatibility(TEXT("{}"), CanonicalLowStagingPath);
	// Resolved declaration과 incompatible canonical Staging을 결합한 gate 결과입니다.
	const FCFDAMigrationGateResult IncompatibleResolvedGate = FCFDAContractGuard::EvaluateMigrationGate(&ResolvedStagingDeclaration, IncompatibleCanonicalResult);
	TestFalse(TEXT("Resolved declaration cannot bypass incompatible canonical Staging"), IncompatibleResolvedGate.bAcceptedSnapshotAppendAllowed);
	TestFalse(TEXT("Incompatible canonical Staging must block promotion"), IncompatibleResolvedGate.bCurrentSystemPromotionAllowed);
	TestTrue(TEXT("Incompatible canonical Staging must remain StagingMigrationPending"), FCFDAContractGuard::HasIssueCode(IncompatibleResolvedGate.Validation, ECFDAContractIssueCode::StagingMigrationPending));
	return true;
}

// Pending accepted record 차단과 no-delta current baseline의 append/promotion 분리를 검증합니다.
bool FCFDAPromotionAppendGateTest::RunTest(const FString& Parameters)
{
	using namespace CFDAContractMigTestsPrivate;

	// Production current revision/canonical exact3/migration gate 결과입니다.
	const FCFDAMigrationGateResult CurrentGate = FCFDAContractGuard::EvaluateCurrentMigrationGate();
	TestTrue(TEXT("Current accepted baseline must pass P0-04 operational gate"), CurrentGate.Validation.bPassed);
	TestFalse(TEXT("No-delta current baseline must not append a duplicate accepted snapshot"), CurrentGate.bAcceptedSnapshotAppendAllowed);
	TestTrue(TEXT("No-delta current baseline with compatible canonical Staging may pass Current System promotion prerequisite"), CurrentGate.bCurrentSystemPromotionAllowed);

	// Production-owned accepted history입니다.
	const TArray<FCFDAAcceptedContractSnapshot>& ProductionSnapshots = FCFDAContractGuard::GetAcceptedSnapshots();
	TestTrue(TEXT("Production accepted history must contain a base snapshot"), !ProductionSnapshots.IsEmpty());
	if (ProductionSnapshots.IsEmpty())
	{
		return false;
	}
	// Synthetic append의 latest accepted base copy입니다.
	const FCFDAAcceptedContractSnapshot BaseSnapshot = ProductionSnapshots.Last();

	// Source/Mapping-only change에 Product review를 요구하는 synthetic accepted candidate입니다.
	FCFDAAcceptedContractSnapshot ProductReviewCandidate = BaseSnapshot;
	ProductReviewCandidate.SnapshotId = TEXT("DACE-P0-04-Synthetic-ProductReview");
	ProductReviewCandidate.PreviousSnapshotSignature = BaseSnapshot.SnapshotSignature;
	ProductReviewCandidate.SourceShapeSignature = BuildAlternateCanonicalSignature(BaseSnapshot.SourceShapeSignature);
	ProductReviewCandidate.SourceAdapterMappingSignature = BuildAlternateCanonicalSignature(BaseSnapshot.SourceAdapterMappingSignature);
	ProductReviewCandidate.MigrationImpact = ECFDAContractMigrationImpact::ProductMigrationReviewRequired;
	ProductReviewCandidate.MigrationResolution = ECFDAContractMigrationResolution::Pending;
	ProductReviewCandidate.MigrationEvidenceId.Reset();
	// Pending candidate record hash 생성 실패 사유입니다.
	FString PendingSignatureError;
	TestTrue(TEXT("Pending synthetic accepted candidate signature must build"), FinalizeSnapshotSignature(ProductReviewCandidate, PendingSignatureError));
	if (!PendingSignatureError.IsEmpty())
	{
		AddError(PendingSignatureError);
		return false;
	}
	// Production history를 보존한 Pending synthetic chain입니다.
	TArray<FCFDAAcceptedContractSnapshot> PendingChain = ProductionSnapshots;
	PendingChain.Add(ProductReviewCandidate);
	// Pending accepted append validation 결과입니다.
	const FCFDAContractGuardResult PendingChainResult = FCFDAContractGuard::ValidateAcceptedSnapshotChain(PendingChain);
	TestFalse(TEXT("Pending Product review record must not be accepted into snapshot chain"), PendingChainResult.bPassed);
	TestTrue(TEXT("Pending accepted Product review must report ProductMigrationReviewPending"), FCFDAContractGuard::HasIssueCode(PendingChainResult, ECFDAContractIssueCode::ProductMigrationReviewPending));

	// Resolution만 Resolved로 바꾸고 evidence를 비워둔 candidate입니다.
	FCFDAAcceptedContractSnapshot MissingEvidenceCandidate = ProductReviewCandidate;
	MissingEvidenceCandidate.MigrationResolution = ECFDAContractMigrationResolution::Resolved;
	// Missing evidence candidate record hash 생성 실패 사유입니다.
	FString MissingEvidenceSignatureError;
	TestTrue(TEXT("Missing-evidence synthetic candidate signature must rebuild"), FinalizeSnapshotSignature(MissingEvidenceCandidate, MissingEvidenceSignatureError));
	// Production history를 보존한 missing-evidence synthetic chain입니다.
	TArray<FCFDAAcceptedContractSnapshot> MissingEvidenceChain = ProductionSnapshots;
	MissingEvidenceChain.Add(MissingEvidenceCandidate);
	// Missing evidence accepted append validation 결과입니다.
	const FCFDAContractGuardResult MissingEvidenceChainResult = FCFDAContractGuard::ValidateAcceptedSnapshotChain(MissingEvidenceChain);
	TestFalse(TEXT("Resolved accepted record without MigrationEvidenceId must fail"), MissingEvidenceChainResult.bPassed);
	TestTrue(TEXT("Resolved accepted record without evidence must fail machine gate"), FCFDAContractGuard::HasIssueCode(MissingEvidenceChainResult, ECFDAContractIssueCode::MigrationImpactUndeclared));

	// Resolved Product review의 non-empty evidence identity입니다.
	FCFDAAcceptedContractSnapshot ResolvedCandidate = MissingEvidenceCandidate;
	ResolvedCandidate.MigrationEvidenceId = TEXT("DACE-P0-04-Synthetic-Product-Evidence");
	// Resolved candidate record hash 생성 실패 사유입니다.
	FString ResolvedSignatureError;
	TestTrue(TEXT("Resolved synthetic accepted candidate signature must rebuild"), FinalizeSnapshotSignature(ResolvedCandidate, ResolvedSignatureError));
	// Production history를 보존한 fully resolved synthetic chain입니다.
	TArray<FCFDAAcceptedContractSnapshot> ResolvedChain = ProductionSnapshots;
	ResolvedChain.Add(ResolvedCandidate);
	// Resolved accepted append validation 결과입니다.
	const FCFDAContractGuardResult ResolvedChainResult = FCFDAContractGuard::ValidateAcceptedSnapshotChain(ResolvedChain);
	TestTrue(TEXT("Resolved Product review with evidence may pass accepted chain validation"), ResolvedChainResult.bPassed);
	return true;
}

#endif
