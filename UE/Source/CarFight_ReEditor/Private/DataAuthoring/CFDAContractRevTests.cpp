// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAContractRevTests.cpp
// Version: v1.0.2
// Date: 2026-09-09
// Description: CF-FQ-050 DACE-P0-03 revision bump, migration declaration, accepted snapshot chain과 current baseline focused Automation입니다.
// Changelog:
// - v1.0.2: Source/Mapping-only explicit NoMigration와 보수적 Product review 경로를 분리하고, no-delta stale declaration, accepted component canonical SHA-256, Adapter revision decrease와 pure revision-only impact matrix를 보강.
// - v1.0.1: Synthetic accepted append fixture가 production history 전체를 보존한 뒤 candidate를 append하도록 교정해 future accepted history exact2+에서도 bootstrap/previous-chain 문맥을 잃지 않도록 보강.
// - v1.0.0: SchemaRevision/AdapterContractRevision bump 누락, explicit migration declaration binding, safe native refactor NoMigration, accepted chain monotonic/previous/id/signature regression과 current baseline을 최초 추가.
// Migration:
// - 모든 contract 변화는 memory-only signature/snapshot fixture로 검증합니다. Production accepted history, Product Low/Normal/High, canonical Staging JSON, UObject package를 수정하거나 저장하지 않습니다.

#include "CFDAContractGuard.h"

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFDAContractRevTestsPrivate
{
	// Candidate signatures와 base snapshot에 결속된 explicit change declaration을 생성합니다.
	bool BuildExplicitDeclaration(
		const FCFDAAcceptedContractSnapshot& BaseSnapshot,
		const FCFDAContractSignatures& CandidateSignatures,
		const ECFDAContractMigrationImpact Impact,
		FCFDACurrentChangeDeclaration& OutDeclaration,
		FString& OutError)
	{
		OutDeclaration = FCFDACurrentChangeDeclaration();
		OutDeclaration.BaseSnapshotId = BaseSnapshot.SnapshotId;
		OutDeclaration.bImpactDeclared = true;
		OutDeclaration.Impact = Impact;
		OutDeclaration.Resolution = Impact == ECFDAContractMigrationImpact::NoMigration
			? ECFDAContractMigrationResolution::NotRequired
			: ECFDAContractMigrationResolution::Pending;
		return FCFDAContractGuard::BuildContractBundleSignature(CandidateSignatures, OutDeclaration.CandidateContractSignature, OutError);
	}

	// Synthetic accepted snapshot의 deterministic record signature를 다시 계산해 저장합니다.
	bool FinalizeSnapshotSignature(FCFDAAcceptedContractSnapshot& InOutSnapshot, FString& OutError)
	{
		// Snapshot fields에서 재계산한 deterministic signature입니다.
		FString SnapshotSignature;
		if (!FCFDAContractGuard::BuildSnapshotSignature(InOutSnapshot, SnapshotSignature, OutError))
		{
			return false;
		}
		InOutSnapshot.SnapshotSignature = SnapshotSignature;
		return true;
	}

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
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDARevisionBumpTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_03.RevisionBumps",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAMigrationDeclarationTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_03.MigrationDeclaration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDAAcceptedChainTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_03.AcceptedSnapshotChain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFDACurrentRevisionBaselineTest,
	"CarFight.DataManagement.CF_FQ_050.DACE_P0_03.CurrentBaseline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Adapter physical shape와 declared semantic 변화에 필요한 revision bump를 각각 fail-closed 검증합니다.
bool FCFDARevisionBumpTest::RunTest(const FString& Parameters)
{
	using namespace CFDAContractRevTestsPrivate;

	// Production-owned accepted history입니다.
	const TArray<FCFDAAcceptedContractSnapshot>& Snapshots = FCFDAContractGuard::GetAcceptedSnapshots();
	TestTrue(TEXT("Accepted history must contain a baseline snapshot"), !Snapshots.IsEmpty());
	if (Snapshots.IsEmpty())
	{
		return false;
	}
	// Revision fixture가 기준으로 삼는 latest accepted snapshot입니다.
	const FCFDAAcceptedContractSnapshot& BaseSnapshot = Snapshots.Last();
	// Current four component signatures입니다.
	FCFDAContractSignatures CurrentSignatures;
	// Current signatures 생성 실패 사유입니다.
	FString CurrentSignatureError;
	TestTrue(TEXT("Current signatures must build for revision fixtures"), FCFDAContractGuard::BuildCurrentSignatures(CurrentSignatures, CurrentSignatureError));
	if (!CurrentSignatureError.IsEmpty())
	{
		AddError(CurrentSignatureError);
		return false;
	}

	// Adapter physical shape만 변경한 synthetic candidate입니다.
	FCFDAContractSignatures AdapterChangedSignatures = CurrentSignatures;
	AdapterChangedSignatures.AdapterShapeSignature += TEXT(":DACE_P0_03_ADAPTER_CHANGED");
	// Adapter change에 결속된 explicit staging migration declaration입니다.
	FCFDACurrentChangeDeclaration AdapterDeclaration;
	// Adapter declaration signature 생성 실패 사유입니다.
	FString AdapterDeclarationError;
	TestTrue(TEXT("Adapter change declaration must bind to candidate signature"), BuildExplicitDeclaration(BaseSnapshot, AdapterChangedSignatures, ECFDAContractMigrationImpact::StagingMigrationRequired, AdapterDeclaration, AdapterDeclarationError));
	if (!AdapterDeclarationError.IsEmpty())
	{
		AddError(AdapterDeclarationError);
		return false;
	}
	// SchemaRevision을 올리지 않은 Adapter physical shape change 결과입니다.
	const FCFDAContractGuardResult MissingSchemaBumpResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		AdapterChangedSignatures,
		BaseSnapshot.SchemaRevision,
		BaseSnapshot.AdapterContractRevision,
		&AdapterDeclaration);
	TestFalse(TEXT("Adapter physical shape change without SchemaRevision bump must fail"), MissingSchemaBumpResult.bPassed);
	TestTrue(TEXT("Adapter physical shape change must report SchemaRevisionBumpRequired"), FCFDAContractGuard::HasIssueCode(MissingSchemaBumpResult, ECFDAContractIssueCode::SchemaRevisionBumpRequired));

	// SchemaRevision을 정확히 전진시킨 Adapter physical shape change 결과입니다.
	const FCFDAContractGuardResult FixedSchemaBumpResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		AdapterChangedSignatures,
		BaseSnapshot.SchemaRevision + 1,
		BaseSnapshot.AdapterContractRevision,
		&AdapterDeclaration);
	TestTrue(TEXT("Adapter physical shape change with SchemaRevision bump and staging migration declaration must pass"), FixedSchemaBumpResult.bPassed);

	// Explicit semantic contract만 변경한 synthetic candidate입니다.
	FCFDAContractSignatures SemanticChangedSignatures = CurrentSignatures;
	SemanticChangedSignatures.SemanticContractSignature += TEXT(":DACE_P0_03_SEMANTIC_CHANGED");
	// Semantic change에 결속된 explicit staging migration declaration입니다.
	FCFDACurrentChangeDeclaration SemanticDeclaration;
	// Semantic declaration signature 생성 실패 사유입니다.
	FString SemanticDeclarationError;
	TestTrue(TEXT("Semantic change declaration must bind to candidate signature"), BuildExplicitDeclaration(BaseSnapshot, SemanticChangedSignatures, ECFDAContractMigrationImpact::StagingMigrationRequired, SemanticDeclaration, SemanticDeclarationError));
	if (!SemanticDeclarationError.IsEmpty())
	{
		AddError(SemanticDeclarationError);
		return false;
	}
	// AdapterContractRevision을 올리지 않은 declared semantic change 결과입니다.
	const FCFDAContractGuardResult MissingAdapterBumpResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		SemanticChangedSignatures,
		BaseSnapshot.SchemaRevision,
		BaseSnapshot.AdapterContractRevision,
		&SemanticDeclaration);
	TestFalse(TEXT("Declared semantic change without AdapterContractRevision bump must fail"), MissingAdapterBumpResult.bPassed);
	TestTrue(TEXT("Declared semantic change must report AdapterRevisionBumpRequired"), FCFDAContractGuard::HasIssueCode(MissingAdapterBumpResult, ECFDAContractIssueCode::AdapterRevisionBumpRequired));

	// AdapterContractRevision을 정확히 전진시킨 semantic change 결과입니다.
	const FCFDAContractGuardResult FixedAdapterBumpResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		SemanticChangedSignatures,
		BaseSnapshot.SchemaRevision,
		BaseSnapshot.AdapterContractRevision + 1,
		&SemanticDeclaration);
	TestTrue(TEXT("Declared semantic change with AdapterContractRevision bump and staging migration declaration must pass"), FixedAdapterBumpResult.bPassed);

	// AdapterContractRevision을 latest accepted보다 감소시킨 candidate 결과입니다.
	const FCFDAContractGuardResult AdapterRevisionDecreaseResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		CurrentSignatures,
		BaseSnapshot.SchemaRevision,
		BaseSnapshot.AdapterContractRevision - 1,
		nullptr);
	TestFalse(TEXT("AdapterContractRevision decrease must fail"), AdapterRevisionDecreaseResult.bPassed);
	TestTrue(TEXT("AdapterContractRevision decrease must report AcceptedSnapshotChainInvalid"), FCFDAContractGuard::HasIssueCode(AdapterRevisionDecreaseResult, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid));

	// Pure SchemaRevision advance에 declaration이 없는 결과입니다.
	const FCFDAContractGuardResult PureSchemaNoDeclarationResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		CurrentSignatures,
		BaseSnapshot.SchemaRevision + 1,
		BaseSnapshot.AdapterContractRevision,
		nullptr);
	TestFalse(TEXT("Pure SchemaRevision advance without declaration must fail"), PureSchemaNoDeclarationResult.bPassed);
	TestTrue(TEXT("Pure SchemaRevision advance without declaration must report MigrationImpactUndeclared"), FCFDAContractGuard::HasIssueCode(PureSchemaNoDeclarationResult, ECFDAContractIssueCode::MigrationImpactUndeclared));

	// Pure revision-only candidate에 결속된 explicit NoMigration declaration입니다.
	FCFDACurrentChangeDeclaration PureRevisionNoMigrationDeclaration;
	// Pure revision NoMigration declaration 생성 실패 사유입니다.
	FString PureRevisionNoMigrationError;
	TestTrue(TEXT("Pure revision NoMigration declaration must bind to candidate"), BuildExplicitDeclaration(BaseSnapshot, CurrentSignatures, ECFDAContractMigrationImpact::NoMigration, PureRevisionNoMigrationDeclaration, PureRevisionNoMigrationError));
	// Pure SchemaRevision advance를 NoMigration으로 선언한 결과입니다.
	const FCFDAContractGuardResult PureSchemaNoMigrationResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		CurrentSignatures,
		BaseSnapshot.SchemaRevision + 1,
		BaseSnapshot.AdapterContractRevision,
		&PureRevisionNoMigrationDeclaration);
	TestFalse(TEXT("Pure SchemaRevision advance with NoMigration must fail"), PureSchemaNoMigrationResult.bPassed);
	TestTrue(TEXT("Pure SchemaRevision advance with NoMigration must report MigrationImpactUndeclared"), FCFDAContractGuard::HasIssueCode(PureSchemaNoMigrationResult, ECFDAContractIssueCode::MigrationImpactUndeclared));

	// Pure revision-only candidate에 결속된 Product-only declaration입니다.
	FCFDACurrentChangeDeclaration PureRevisionProductOnlyDeclaration;
	// Pure revision Product-only declaration 생성 실패 사유입니다.
	FString PureRevisionProductOnlyError;
	TestTrue(TEXT("Pure revision Product-only declaration must bind to candidate"), BuildExplicitDeclaration(BaseSnapshot, CurrentSignatures, ECFDAContractMigrationImpact::ProductMigrationReviewRequired, PureRevisionProductOnlyDeclaration, PureRevisionProductOnlyError));
	// Pure SchemaRevision advance를 Product-only로 선언한 결과입니다.
	const FCFDAContractGuardResult PureSchemaProductOnlyResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		CurrentSignatures,
		BaseSnapshot.SchemaRevision + 1,
		BaseSnapshot.AdapterContractRevision,
		&PureRevisionProductOnlyDeclaration);
	TestFalse(TEXT("Pure SchemaRevision advance with Product-only impact must fail"), PureSchemaProductOnlyResult.bPassed);
	TestTrue(TEXT("Pure SchemaRevision advance with Product-only impact must report MigrationImpactUndeclared"), FCFDAContractGuard::HasIssueCode(PureSchemaProductOnlyResult, ECFDAContractIssueCode::MigrationImpactUndeclared));

	// Pure revision-only candidate에 결속된 staging migration declaration입니다.
	FCFDACurrentChangeDeclaration PureRevisionStagingDeclaration;
	// Pure revision staging declaration 생성 실패 사유입니다.
	FString PureRevisionStagingError;
	TestTrue(TEXT("Pure revision staging declaration must bind to candidate"), BuildExplicitDeclaration(BaseSnapshot, CurrentSignatures, ECFDAContractMigrationImpact::StagingMigrationRequired, PureRevisionStagingDeclaration, PureRevisionStagingError));
	// Pure SchemaRevision advance에 최소 staging impact를 선언한 결과입니다.
	const FCFDAContractGuardResult PureSchemaStagingResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		CurrentSignatures,
		BaseSnapshot.SchemaRevision + 1,
		BaseSnapshot.AdapterContractRevision,
		&PureRevisionStagingDeclaration);
	TestTrue(TEXT("Pure SchemaRevision advance with staging migration impact must pass"), PureSchemaStagingResult.bPassed);
	// Pure AdapterContractRevision advance에 최소 staging impact를 선언한 결과입니다.
	const FCFDAContractGuardResult PureAdapterStagingResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		CurrentSignatures,
		BaseSnapshot.SchemaRevision,
		BaseSnapshot.AdapterContractRevision + 1,
		&PureRevisionStagingDeclaration);
	TestTrue(TEXT("Pure AdapterContractRevision advance with staging migration impact must pass"), PureAdapterStagingResult.bPassed);
	return true;
}

// Contract 변화의 declaration 유무/binding/impact와 safe native refactor explicit NoMigration을 검증합니다.
bool FCFDAMigrationDeclarationTest::RunTest(const FString& Parameters)
{
	using namespace CFDAContractRevTestsPrivate;

	// Production-owned accepted history입니다.
	const TArray<FCFDAAcceptedContractSnapshot>& Snapshots = FCFDAContractGuard::GetAcceptedSnapshots();
	TestTrue(TEXT("Accepted history must contain a baseline snapshot"), !Snapshots.IsEmpty());
	if (Snapshots.IsEmpty())
	{
		return false;
	}
	// Declaration fixture가 기준으로 삼는 latest accepted snapshot입니다.
	const FCFDAAcceptedContractSnapshot& BaseSnapshot = Snapshots.Last();
	// Current four component signatures입니다.
	FCFDAContractSignatures CurrentSignatures;
	// Current signatures 생성 실패 사유입니다.
	FString CurrentSignatureError;
	TestTrue(TEXT("Current signatures must build for declaration fixtures"), FCFDAContractGuard::BuildCurrentSignatures(CurrentSignatures, CurrentSignatureError));
	if (!CurrentSignatureError.IsEmpty())
	{
		AddError(CurrentSignatureError);
		return false;
	}

	// Adapter/semantic/revision은 그대로 두고 Source와 mapping만 함께 바꾼 candidate fixture입니다.
	FCFDAContractSignatures SourceMappingOnlySignatures = CurrentSignatures;
	SourceMappingOnlySignatures.SourceShapeSignature += TEXT(":DACE_P0_03_SOURCE_ONLY");
	SourceMappingOnlySignatures.SourceAdapterMappingSignature += TEXT(":DACE_P0_03_MAPPING_ONLY");
	// Declaration이 전혀 없는 Source/Mapping-only candidate 결과입니다.
	const FCFDAContractGuardResult MissingDeclarationResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		SourceMappingOnlySignatures,
		BaseSnapshot.SchemaRevision,
		BaseSnapshot.AdapterContractRevision,
		nullptr);
	TestFalse(TEXT("Any contract change without CurrentChangeDeclaration must fail"), MissingDeclarationResult.bPassed);
	TestTrue(TEXT("Missing declaration must report MigrationImpactUndeclared"), FCFDAContractGuard::HasIssueCode(MissingDeclarationResult, ECFDAContractIssueCode::MigrationImpactUndeclared));

	// Safe refactor에 결속된 explicit NoMigration declaration입니다.
	FCFDACurrentChangeDeclaration NoMigrationDeclaration;
	// Safe refactor declaration signature 생성 실패 사유입니다.
	FString NoMigrationDeclarationError;
	TestTrue(TEXT("Explicit NoMigration declaration must bind to Source/Mapping-only candidate"), BuildExplicitDeclaration(BaseSnapshot, SourceMappingOnlySignatures, ECFDAContractMigrationImpact::NoMigration, NoMigrationDeclaration, NoMigrationDeclarationError));
	if (!NoMigrationDeclarationError.IsEmpty())
	{
		AddError(NoMigrationDeclarationError);
		return false;
	}
	// Explicit NoMigration으로 승인 가능한 safe native refactor 결과입니다.
	const FCFDAContractGuardResult SafeNoMigrationResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		SourceMappingOnlySignatures,
		BaseSnapshot.SchemaRevision,
		BaseSnapshot.AdapterContractRevision,
		&NoMigrationDeclaration);
	TestTrue(TEXT("Source/Mapping-only candidate with explicit NoMigration must pass structural guard"), SafeNoMigrationResult.bPassed);

	// NoMigration enum 값은 같지만 explicit declaration bit가 없는 fixture입니다.
	FCFDACurrentChangeDeclaration UndeclaredImpact = NoMigrationDeclaration;
	UndeclaredImpact.bImpactDeclared = false;
	// Default/undeclared와 explicit NoMigration을 구분하는 결과입니다.
	const FCFDAContractGuardResult UndeclaredImpactResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		SourceMappingOnlySignatures,
		BaseSnapshot.SchemaRevision,
		BaseSnapshot.AdapterContractRevision,
		&UndeclaredImpact);
	TestFalse(TEXT("NoMigration enum without explicit impact declaration bit must fail"), UndeclaredImpactResult.bPassed);
	TestTrue(TEXT("Undeclared impact must report MigrationImpactUndeclared"), FCFDAContractGuard::HasIssueCode(UndeclaredImpactResult, ECFDAContractIssueCode::MigrationImpactUndeclared));

	// Latest accepted snapshot이 아닌 stale base를 가리키는 declaration fixture입니다.
	FCFDACurrentChangeDeclaration StaleBaseDeclaration = NoMigrationDeclaration;
	StaleBaseDeclaration.BaseSnapshotId = TEXT("DACE-Stale-Base");
	// Stale base declaration 결과입니다.
	const FCFDAContractGuardResult StaleBaseResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		SourceMappingOnlySignatures,
		BaseSnapshot.SchemaRevision,
		BaseSnapshot.AdapterContractRevision,
		&StaleBaseDeclaration);
	TestFalse(TEXT("CurrentChangeDeclaration with stale BaseSnapshotId must fail"), StaleBaseResult.bPassed);
	TestTrue(TEXT("Stale BaseSnapshotId must report AcceptedSnapshotChainInvalid"), FCFDAContractGuard::HasIssueCode(StaleBaseResult, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid));

	// Caller가 actual signature와 무관한 문자열을 승인하려는 declaration fixture입니다.
	FCFDACurrentChangeDeclaration WrongCandidateSignatureDeclaration = NoMigrationDeclaration;
	WrongCandidateSignatureDeclaration.CandidateContractSignature = TEXT("sha256:not-the-actual-candidate");
	// Wrong candidate signature binding 결과입니다.
	const FCFDAContractGuardResult WrongCandidateSignatureResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		SourceMappingOnlySignatures,
		BaseSnapshot.SchemaRevision,
		BaseSnapshot.AdapterContractRevision,
		&WrongCandidateSignatureDeclaration);
	TestFalse(TEXT("Declaration with caller-selected wrong CandidateContractSignature must fail"), WrongCandidateSignatureResult.bPassed);
	TestTrue(TEXT("Wrong CandidateContractSignature must report MigrationImpactUndeclared"), FCFDAContractGuard::HasIssueCode(WrongCandidateSignatureResult, ECFDAContractIssueCode::MigrationImpactUndeclared));

	// Source/Mapping-only candidate에 보수적으로 Product review를 선언한 fixture입니다.
	FCFDACurrentChangeDeclaration ProductReviewDeclaration;
	// Product review declaration signature 생성 실패 사유입니다.
	FString ProductReviewDeclarationError;
	TestTrue(TEXT("Source/Mapping-only Product review declaration must bind to candidate"), BuildExplicitDeclaration(BaseSnapshot, SourceMappingOnlySignatures, ECFDAContractMigrationImpact::ProductMigrationReviewRequired, ProductReviewDeclaration, ProductReviewDeclarationError));
	// Source/Mapping-only change에서 보수적 Product review를 보존하는 결과입니다.
	const FCFDAContractGuardResult ProductReviewResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		SourceMappingOnlySignatures,
		BaseSnapshot.SchemaRevision,
		BaseSnapshot.AdapterContractRevision,
		&ProductReviewDeclaration);
	TestTrue(TEXT("Source/Mapping-only candidate may conservatively declare ProductMigrationReviewRequired"), ProductReviewResult.bPassed);

	// Actual baseline과 delta가 없는데 declaration만 남은 stale lifecycle fixture입니다.
	FCFDACurrentChangeDeclaration NoDeltaDeclaration;
	// No-delta declaration signature 생성 실패 사유입니다.
	FString NoDeltaDeclarationError;
	TestTrue(TEXT("No-delta declaration fixture must bind to current candidate"), BuildExplicitDeclaration(BaseSnapshot, CurrentSignatures, ECFDAContractMigrationImpact::NoMigration, NoDeltaDeclaration, NoDeltaDeclarationError));
	// Delta가 없는 상태에서 stale declaration을 전달한 결과입니다.
	const FCFDAContractGuardResult NoDeltaDeclarationResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		CurrentSignatures,
		BaseSnapshot.SchemaRevision,
		BaseSnapshot.AdapterContractRevision,
		&NoDeltaDeclaration);
	TestFalse(TEXT("No-delta state with stale CurrentChangeDeclaration must fail"), NoDeltaDeclarationResult.bPassed);
	TestTrue(TEXT("No-delta stale declaration must report MigrationImpactUndeclared"), FCFDAContractGuard::HasIssueCode(NoDeltaDeclarationResult, ECFDAContractIssueCode::MigrationImpactUndeclared));

	// Semantic contract가 바뀐 candidate입니다.
	FCFDAContractSignatures SemanticChangedSignatures = CurrentSignatures;
	SemanticChangedSignatures.SemanticContractSignature += TEXT(":DACE_P0_03_MIN_STAGING");
	// Product review만 선언해 Staging migration이 빠진 fixture입니다.
	FCFDACurrentChangeDeclaration ProductOnlyDeclaration;
	// Product-only declaration signature 생성 실패 사유입니다.
	FString ProductOnlyDeclarationError;
	TestTrue(TEXT("Product-only declaration must bind to semantic candidate"), BuildExplicitDeclaration(BaseSnapshot, SemanticChangedSignatures, ECFDAContractMigrationImpact::ProductMigrationReviewRequired, ProductOnlyDeclaration, ProductOnlyDeclarationError));
	// Adapter revision은 올렸지만 staging migration impact가 빠진 결과입니다.
	const FCFDAContractGuardResult ProductOnlyResult = FCFDAContractGuard::ValidateRevisionGuard(
		BaseSnapshot,
		SemanticChangedSignatures,
		BaseSnapshot.SchemaRevision,
		BaseSnapshot.AdapterContractRevision + 1,
		&ProductOnlyDeclaration);
	TestFalse(TEXT("Revision advance without staging migration impact must fail"), ProductOnlyResult.bPassed);
	TestTrue(TEXT("Missing minimum staging migration impact must report MigrationImpactUndeclared"), FCFDAContractGuard::HasIssueCode(ProductOnlyResult, ECFDAContractIssueCode::MigrationImpactUndeclared));
	return true;
}

// Accepted snapshot의 record signature, previous chain, unique id와 revision monotonicity를 synthetic append fixture로 검증합니다.
bool FCFDAAcceptedChainTest::RunTest(const FString& Parameters)
{
	using namespace CFDAContractRevTestsPrivate;

	// Production-owned current accepted history입니다.
	const TArray<FCFDAAcceptedContractSnapshot>& ProductionSnapshots = FCFDAContractGuard::GetAcceptedSnapshots();
	// Current production history 자체의 chain validation 결과입니다.
	const FCFDAContractGuardResult ProductionChainResult = FCFDAContractGuard::ValidateAcceptedSnapshotChain(ProductionSnapshots);
	TestTrue(TEXT("Current production accepted snapshot chain must pass"), ProductionChainResult.bPassed);
	if (ProductionSnapshots.IsEmpty())
	{
		return false;
	}
	// Synthetic append fixture의 base snapshot copy입니다.
	const FCFDAAcceptedContractSnapshot BaseSnapshot = ProductionSnapshots.Last();

	// Revision bump 없이 Source/mapping만 바뀐 explicit NoMigration safe-refactor accepted candidate입니다.
	FCFDAAcceptedContractSnapshot SafeCandidate = BaseSnapshot;
	SafeCandidate.SnapshotId = TEXT("DACE-P0-03-Synthetic-SafeRefactor");
	SafeCandidate.PreviousSnapshotSignature = BaseSnapshot.SnapshotSignature;
	SafeCandidate.SourceShapeSignature = BuildAlternateCanonicalSignature(BaseSnapshot.SourceShapeSignature);
	SafeCandidate.SourceAdapterMappingSignature = BuildAlternateCanonicalSignature(BaseSnapshot.SourceAdapterMappingSignature);
	SafeCandidate.MigrationImpact = ECFDAContractMigrationImpact::NoMigration;
	SafeCandidate.MigrationResolution = ECFDAContractMigrationResolution::NotRequired;
	SafeCandidate.MigrationEvidenceId.Reset();
	// Safe candidate record signature 계산 실패 사유입니다.
	FString SafeCandidateSignatureError;
	TestTrue(TEXT("Synthetic safe candidate snapshot signature must build"), FinalizeSnapshotSignature(SafeCandidate, SafeCandidateSignatureError));
	if (!SafeCandidateSignatureError.IsEmpty())
	{
		AddError(SafeCandidateSignatureError);
		return false;
	}
	// Production accepted history 전체를 보존한 뒤 safe candidate를 append한 valid synthetic chain입니다.
	TArray<FCFDAAcceptedContractSnapshot> ValidChain = ProductionSnapshots;
	ValidChain.Add(SafeCandidate);
	// Valid synthetic chain 결과입니다.
	const FCFDAContractGuardResult ValidChainResult = FCFDAContractGuard::ValidateAcceptedSnapshotChain(ValidChain);
	TestTrue(TEXT("Safe native refactor accepted append with same revisions and NoMigration must pass"), ValidChainResult.bPassed);

	// PreviousSnapshotSignature 연결을 고의로 끊은 fixture입니다.
	FCFDAAcceptedContractSnapshot BrokenPreviousCandidate = SafeCandidate;
	BrokenPreviousCandidate.PreviousSnapshotSignature = TEXT("sha256:broken-previous-link");
	// Broken previous fixture signature 계산 실패 사유입니다.
	FString BrokenPreviousSignatureError;
	TestTrue(TEXT("Broken previous fixture record signature must rebuild"), FinalizeSnapshotSignature(BrokenPreviousCandidate, BrokenPreviousSignatureError));
	// Production accepted history 전체를 보존한 broken previous link chain입니다.
	TArray<FCFDAAcceptedContractSnapshot> BrokenPreviousChain = ProductionSnapshots;
	BrokenPreviousChain.Add(BrokenPreviousCandidate);
	// Broken previous link validation 결과입니다.
	const FCFDAContractGuardResult BrokenPreviousResult = FCFDAContractGuard::ValidateAcceptedSnapshotChain(BrokenPreviousChain);
	TestFalse(TEXT("Accepted snapshot with broken PreviousSnapshotSignature must fail"), BrokenPreviousResult.bPassed);
	TestTrue(TEXT("Broken previous link must report AcceptedSnapshotChainInvalid"), FCFDAContractGuard::HasIssueCode(BrokenPreviousResult, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid));

	// Base와 같은 SnapshotId를 재사용한 duplicate identity fixture입니다.
	FCFDAAcceptedContractSnapshot DuplicateIdCandidate = SafeCandidate;
	DuplicateIdCandidate.SnapshotId = BaseSnapshot.SnapshotId;
	// Duplicate id fixture signature 계산 실패 사유입니다.
	FString DuplicateIdSignatureError;
	TestTrue(TEXT("Duplicate id fixture record signature must rebuild"), FinalizeSnapshotSignature(DuplicateIdCandidate, DuplicateIdSignatureError));
	// Production accepted history 전체를 보존한 duplicate id synthetic chain입니다.
	TArray<FCFDAAcceptedContractSnapshot> DuplicateIdChain = ProductionSnapshots;
	DuplicateIdChain.Add(DuplicateIdCandidate);
	// Duplicate id validation 결과입니다.
	const FCFDAContractGuardResult DuplicateIdResult = FCFDAContractGuard::ValidateAcceptedSnapshotChain(DuplicateIdChain);
	TestFalse(TEXT("Accepted snapshot chain with duplicate SnapshotId must fail"), DuplicateIdResult.bPassed);
	TestTrue(TEXT("Duplicate SnapshotId must report AcceptedSnapshotChainInvalid"), FCFDAContractGuard::HasIssueCode(DuplicateIdResult, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid));

	// Previous accepted revision보다 SchemaRevision을 낮춘 regression fixture입니다.
	FCFDAAcceptedContractSnapshot RevisionRegressionCandidate = SafeCandidate;
	RevisionRegressionCandidate.SchemaRevision = BaseSnapshot.SchemaRevision - 1;
	// Revision regression fixture signature 계산 실패 사유입니다.
	FString RevisionRegressionSignatureError;
	TestTrue(TEXT("Revision regression fixture record signature must rebuild"), FinalizeSnapshotSignature(RevisionRegressionCandidate, RevisionRegressionSignatureError));
	// Production accepted history 전체를 보존한 revision regression synthetic chain입니다.
	TArray<FCFDAAcceptedContractSnapshot> RevisionRegressionChain = ProductionSnapshots;
	RevisionRegressionChain.Add(RevisionRegressionCandidate);
	// Revision regression validation 결과입니다.
	const FCFDAContractGuardResult RevisionRegressionResult = FCFDAContractGuard::ValidateAcceptedSnapshotChain(RevisionRegressionChain);
	TestFalse(TEXT("Accepted snapshot revision regression must fail"), RevisionRegressionResult.bPassed);
	TestTrue(TEXT("Revision regression must report AcceptedSnapshotChainInvalid"), FCFDAContractGuard::HasIssueCode(RevisionRegressionResult, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid));

	// Previous accepted revision보다 AdapterContractRevision을 낮춘 regression fixture입니다.
	FCFDAAcceptedContractSnapshot AdapterRevisionRegressionCandidate = SafeCandidate;
	AdapterRevisionRegressionCandidate.AdapterContractRevision = BaseSnapshot.AdapterContractRevision - 1;
	// Adapter revision regression fixture signature 계산 실패 사유입니다.
	FString AdapterRevisionRegressionSignatureError;
	TestTrue(TEXT("Adapter revision regression fixture record signature must rebuild"), FinalizeSnapshotSignature(AdapterRevisionRegressionCandidate, AdapterRevisionRegressionSignatureError));
	// Production accepted history 전체를 보존한 Adapter revision regression chain입니다.
	TArray<FCFDAAcceptedContractSnapshot> AdapterRevisionRegressionChain = ProductionSnapshots;
	AdapterRevisionRegressionChain.Add(AdapterRevisionRegressionCandidate);
	// Adapter revision regression validation 결과입니다.
	const FCFDAContractGuardResult AdapterRevisionRegressionResult = FCFDAContractGuard::ValidateAcceptedSnapshotChain(AdapterRevisionRegressionChain);
	TestFalse(TEXT("Accepted snapshot AdapterContractRevision regression must fail"), AdapterRevisionRegressionResult.bPassed);
	TestTrue(TEXT("Adapter revision regression must report AcceptedSnapshotChainInvalid"), FCFDAContractGuard::HasIssueCode(AdapterRevisionRegressionResult, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid));

	// SourceShapeSignature를 empty로 만들고 record hash는 다시 맞춘 malformed authority fixture입니다.
	FCFDAAcceptedContractSnapshot EmptyComponentCandidate = SafeCandidate;
	EmptyComponentCandidate.SourceShapeSignature.Reset();
	// Empty component fixture signature 계산 실패 사유입니다.
	FString EmptyComponentSignatureError;
	TestTrue(TEXT("Empty component fixture record signature must rebuild"), FinalizeSnapshotSignature(EmptyComponentCandidate, EmptyComponentSignatureError));
	// Production accepted history 전체를 보존한 empty component chain입니다.
	TArray<FCFDAAcceptedContractSnapshot> EmptyComponentChain = ProductionSnapshots;
	EmptyComponentChain.Add(EmptyComponentCandidate);
	// Empty component validation 결과입니다.
	const FCFDAContractGuardResult EmptyComponentResult = FCFDAContractGuard::ValidateAcceptedSnapshotChain(EmptyComponentChain);
	TestFalse(TEXT("Accepted snapshot with empty component signature must fail"), EmptyComponentResult.bPassed);
	TestTrue(TEXT("Empty component signature must report AcceptedSnapshotChainInvalid"), FCFDAContractGuard::HasIssueCode(EmptyComponentResult, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid));

	// AdapterShapeSignature를 noncanonical uppercase hex로 만들고 record hash는 다시 맞춘 fixture입니다.
	FCFDAAcceptedContractSnapshot MalformedComponentCandidate = SafeCandidate;
	MalformedComponentCandidate.AdapterShapeSignature = TEXT("sha256:AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	// Malformed component fixture signature 계산 실패 사유입니다.
	FString MalformedComponentSignatureError;
	TestTrue(TEXT("Malformed component fixture record signature must rebuild"), FinalizeSnapshotSignature(MalformedComponentCandidate, MalformedComponentSignatureError));
	// Production accepted history 전체를 보존한 malformed component chain입니다.
	TArray<FCFDAAcceptedContractSnapshot> MalformedComponentChain = ProductionSnapshots;
	MalformedComponentChain.Add(MalformedComponentCandidate);
	// Malformed component validation 결과입니다.
	const FCFDAContractGuardResult MalformedComponentResult = FCFDAContractGuard::ValidateAcceptedSnapshotChain(MalformedComponentChain);
	TestFalse(TEXT("Accepted snapshot with noncanonical component signature must fail"), MalformedComponentResult.bPassed);
	TestTrue(TEXT("Malformed component signature must report AcceptedSnapshotChainInvalid"), FCFDAContractGuard::HasIssueCode(MalformedComponentResult, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid));

	// Stored signature를 다시 만들지 않고 canonical Source signature만 변조한 record integrity fixture입니다.
	FCFDAAcceptedContractSnapshot TamperedCandidate = SafeCandidate;
	TamperedCandidate.SourceShapeSignature = BuildAlternateCanonicalSignature(SafeCandidate.SourceShapeSignature);
	// Production accepted history 전체를 보존한 tampered record synthetic chain입니다.
	TArray<FCFDAAcceptedContractSnapshot> TamperedChain = ProductionSnapshots;
	TamperedChain.Add(TamperedCandidate);
	// Tampered record validation 결과입니다.
	const FCFDAContractGuardResult TamperedResult = FCFDAContractGuard::ValidateAcceptedSnapshotChain(TamperedChain);
	TestFalse(TEXT("Accepted snapshot with mismatched stored record signature must fail"), TamperedResult.bPassed);
	TestTrue(TEXT("Tampered record signature must report AcceptedSnapshotChainInvalid"), FCFDAContractGuard::HasIssueCode(TamperedResult, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid));
	return true;
}

// Current Schema1/Adapter2 baseline이 latest accepted snapshot과 같아 declaration 없이 PASS하는 production 상태를 검증합니다.
bool FCFDACurrentRevisionBaselineTest::RunTest(const FString& Parameters)
{
	// Current baseline에는 pending contract change declaration이 없어야 합니다.
	const FCFDACurrentChangeDeclaration* CurrentDeclaration = FCFDAContractGuard::GetCurrentChangeDeclaration();
	TestTrue(TEXT("Unchanged current baseline must not require a CurrentChangeDeclaration"), CurrentDeclaration == nullptr);
	// Production accepted history + current descriptor/service revision 종합 결과입니다.
	const FCFDAContractGuardResult CurrentRevisionResult = FCFDAContractGuard::ValidateCurrentRevisionGuard();
	TestTrue(TEXT("Current accepted baseline must pass revision guard without a declaration"), CurrentRevisionResult.bPassed);

	// Current four component signatures입니다.
	FCFDAContractSignatures CurrentSignatures;
	// Current signatures 생성 실패 사유입니다.
	FString CurrentSignatureError;
	TestTrue(TEXT("Current signatures must build for bundle determinism"), FCFDAContractGuard::BuildCurrentSignatures(CurrentSignatures, CurrentSignatureError));
	// 첫 번째 combined candidate contract signature입니다.
	FString FirstBundleSignature;
	// 첫 번째 bundle signature 생성 실패 사유입니다.
	FString FirstBundleError;
	TestTrue(TEXT("Current contract bundle signature must build"), FCFDAContractGuard::BuildContractBundleSignature(CurrentSignatures, FirstBundleSignature, FirstBundleError));
	// 두 번째 combined candidate contract signature입니다.
	FString SecondBundleSignature;
	// 두 번째 bundle signature 생성 실패 사유입니다.
	FString SecondBundleError;
	TestTrue(TEXT("Repeated current contract bundle signature must build"), FCFDAContractGuard::BuildContractBundleSignature(CurrentSignatures, SecondBundleSignature, SecondBundleError));
	TestTrue(TEXT("Combined candidate contract signature must be canonical SHA-256"), FirstBundleSignature.StartsWith(TEXT("sha256:")));
	TestEqual(TEXT("Combined candidate contract signature must be deterministic"), FirstBundleSignature, SecondBundleSignature);
	return true;
}

#endif
