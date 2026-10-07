// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFProdCutoverApplyTests.cpp
// Version: v2.0.0
// Date: 2026-10-07
// Description: CF-FQ-058 completed First Production Wave cutover의 read-only durable verification.
// Changelog:
// - v2.0.0: one-shot apply entrypoint를 제거하고 canonical Workbook / provenance / exact8 VehicleReady transactions / Publication exact8 readback만 검증하는 post-cutover test로 전환.
// - v1.1.0: first execution recovery harness.
// - v1.0.0: first Production one-shot cutover harness.
// Migration:
// - managed authority cutover가 완료됐으므로 이 파일은 더 이상 Production write를 수행하지 않습니다.
// - canonical Workbook이 sole Current authoring authority이며 CFProdBatchPrep bootstrap writer를 재호출하지 않습니다.

#include "DataAuthoring/CFContentCompiler.h"
#include "DataAuthoring/CFContentCore.h"
#include "DataAuthoring/CFContentCutover.h"
#include "DataAuthoring/CFDACommonPrimitives.h"
#include "DataAuthoring/CFNativeXlsxAdapter.h"
#include "DataAuthoring/CFProdProvisioning.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

namespace CFProdCutoverVerificationPrivate
{
	const TCHAR* ApprovedReviewFingerprint =
		TEXT("sha256:9af9a9256802244e7598ca6c92c916d1f9e11335599204cd518a094b552b5940");
	const TCHAR* ApprovedPostWorkbookHash =
		TEXT("sha256:049752116ea69850ffe245ce26365756bd15cfb1e332fc202339720c0ced42e3");
	const TCHAR* ApprovedResourceCatalogFingerprint =
		TEXT("sha256:0cd477e385e663b2914928f523b698a15efe6da516747405a20721e2418530f7");
	const TCHAR* ApprovedExecutionBindingFingerprint =
		TEXT("sha256:1e87904bfe4e65f5614c8744af50dc0cbf533f14c64b13748178e08e6cb6243a");
	const TCHAR* CutoverTransactionId =
		TEXT("CF058ProdWave1Cutover20261007");
	const TCHAR* CanonicalWorkbookPath =
		TEXT("Authoring/Content/CarFight_Content.xlsx");
	const TCHAR* StagedWorkbookPath =
		TEXT("Authoring/Content/CarFight_Content.staged.xlsx");
	const TCHAR* ProvenancePath =
		TEXT("Authoring/Content/CarFight_Content.cfsnapshot.json");

	const TArray<FString> ProductIds = {
		TEXT("Cannon_Heavy"),
		TEXT("Cannon_LongRange"),
		TEXT("Cannon_Rapid"),
		TEXT("Cannon_Standard"),
		TEXT("GuidedMissile_Standard"),
		TEXT("Rocket_Ripple"),
		TEXT("Rocket_Salvo"),
		TEXT("Rocket_Standard")
	};

	FString ResolveRepositoryPath(const FString& RepositoryRelativePath)
	{
		const FString RepositoryRoot = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectDir(), TEXT("..")));
		return FPaths::ConvertRelativePathToFull(
			FPaths::Combine(RepositoryRoot, RepositoryRelativePath));
	}

	FString ProductTransactionId(const FString& ProductId)
	{
		return TEXT("CF058_W1_") + ProductId;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFProdCutoverVerificationTest,
	"CarFight.ContentAuthoring.P007.ProductionCutoverVerification",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFProdCutoverVerificationTest::RunTest(const FString& Parameters)
{
	using namespace CFProdCutoverVerificationPrivate;
	(void)Parameters;

	FString Error;

	const FString StagedPhysicalPath =
		ResolveRepositoryPath(StagedWorkbookPath);
	if (!TestFalse(
		TEXT("Post-cutover staged Workbook residue exact0"),
		IFileManager::Get().FileExists(*StagedPhysicalPath)))
	{
		return false;
	}

	FCFNativeXlsxAdapter WorkbookAdapter;
	FCFContentWorkbookModel CanonicalWorkbook;
	FString CanonicalSemanticHash;
	if (!TestTrue(
			TEXT("Canonical Workbook fresh readback"),
			WorkbookAdapter.ReadWorkbook(
				CanonicalWorkbookPath,
				CanonicalWorkbook,
				Error))
		|| !TestTrue(
			TEXT("Canonical Workbook semantic hash builds"),
			FCFContentSemanticHasher::BuildWorkbookSemanticHash(
				CanonicalWorkbook,
				CanonicalSemanticHash,
				Error))
		|| !TestEqual(
			TEXT("Canonical Workbook approved semantic exact"),
			CanonicalSemanticHash,
			FString(ApprovedPostWorkbookHash)))
	{
		if (!Error.IsEmpty())
		{
			AddError(Error);
		}
		return false;
	}

	FCFContentCutoverFileStore CutoverStore;
	FCFContentProvenanceSnapshot Provenance;
	if (!TestTrue(
			TEXT("Production provenance fresh readback"),
			CutoverStore.LoadProvenance(
				ProvenancePath,
				Provenance,
				Error))
		|| !TestEqual(
			TEXT("Production provenance cutover transaction identity"),
			Provenance.CutoverTransactionId,
			FString(CutoverTransactionId))
		|| !TestEqual(
			TEXT("Production provenance exact42"),
			Provenance.Records.Num(),
			42))
	{
		if (!Error.IsEmpty())
		{
			AddError(Error);
		}
		return false;
	}

	for (const FCFContentProvenanceRecord& Record : Provenance.Records)
	{
		TestEqual(
			TEXT("Provenance approved ReviewPackage identity"),
			Record.ReviewPackageFingerprint,
			FString(ApprovedReviewFingerprint));
		TestEqual(
			TEXT("Provenance approved Workbook identity"),
			Record.SourceWorkbookSemanticHash,
			FString(ApprovedPostWorkbookHash));
		TestTrue(
			TEXT("Provenance Provider schema fingerprint canonical"),
			CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				Record.ProviderSchemaFingerprint));
		TestTrue(
			TEXT("Provenance desired canonical fingerprint canonical"),
			CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				Record.DesiredCanonicalFingerprint));
		TestTrue(
			TEXT("Provenance post-readback fingerprint canonical"),
			CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				Record.PostApplyReadbackFingerprint));
	}

	FCFProdTransactionFileStore TransactionStore;
	TSet<FString> UniqueTargetKeys;
	TMap<FString, FString> ProductGraphById;
	for (const FString& ProductId : ProductIds)
	{
		bool bFound = false;
		FCFProdTransaction Transaction;
		if (!TestTrue(
			*FString::Printf(TEXT("Transaction load succeeds: %s"), *ProductId),
			TransactionStore.LoadTransaction(
				ProductTransactionId(ProductId),
				bFound,
				Transaction,
				Error))
			|| !TestTrue(
				*FString::Printf(TEXT("Transaction exists: %s"), *ProductId),
				bFound))
		{
			if (!Error.IsEmpty())
			{
				AddError(Error);
			}
			return false;
		}

		TestEqual(
			*FString::Printf(TEXT("Transaction VehicleReady: %s"), *ProductId),
			Transaction.State,
			ECFProdTxnState::VehicleReady);
		TestEqual(
			*FString::Printf(TEXT("Transaction Review identity: %s"), *ProductId),
			Transaction.ReviewPackageFingerprint,
			FString(ApprovedReviewFingerprint));
		TestEqual(
			*FString::Printf(TEXT("Transaction Workbook identity: %s"), *ProductId),
			Transaction.WorkbookSemanticHash,
			FString(ApprovedPostWorkbookHash));
		TestEqual(
			*FString::Printf(TEXT("Transaction Resource identity: %s"), *ProductId),
			Transaction.ResourceCatalogFingerprint,
			FString(ApprovedResourceCatalogFingerprint));
		TestEqual(
			*FString::Printf(TEXT("Transaction Execution identity: %s"), *ProductId),
			Transaction.ExecutionBindingFingerprint,
			FString(ApprovedExecutionBindingFingerprint));
		TestTrue(
			*FString::Printf(TEXT("Product graph fingerprint canonical: %s"), *ProductId),
			CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				Transaction.ProductGraphFingerprint));
		TestTrue(
			*FString::Printf(TEXT("Transaction diagnostic empty: %s"), *ProductId),
			Transaction.Diagnostic.IsEmpty());
		TestEqual(
			*FString::Printf(TEXT("Product target evidence exact6: %s"), *ProductId),
			Transaction.Targets.Num(),
			6);

		ProductGraphById.Add(ProductId, Transaction.ProductGraphFingerprint);
		for (const FCFProdTargetEvidence& Evidence : Transaction.Targets)
		{
			TestTrue(
				TEXT("Target persisted fingerprint canonical"),
				CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
					Evidence.PersistedFingerprint));
			TestEqual(
				TEXT("Target persisted equals approved desired"),
				Evidence.PersistedFingerprint,
				Evidence.DesiredFingerprint);
			UniqueTargetKeys.Add(Evidence.ContentKey.ToStableString());
		}
	}
	TestEqual(
		TEXT("Durable Production target union exact34"),
		UniqueTargetKeys.Num(),
		34);

	FCFProdCatalogStore PublicationStore;
	TArray<FString> PublishedIds;
	if (!TestTrue(
		TEXT("Production Publication Catalog fresh readback"),
		PublicationStore.ReadPublishedProductIds(
			PublishedIds,
			Error))
		|| !TestEqual(
			TEXT("Production Publication exact8"),
			PublishedIds.Num(),
			8))
	{
		if (!Error.IsEmpty())
		{
			AddError(Error);
		}
		return false;
	}

	TArray<FString> ExpectedProductIds = ProductIds;
	PublishedIds.Sort();
	ExpectedProductIds.Sort();
	for (int32 Index = 0; Index < ExpectedProductIds.Num(); ++Index)
	{
		TestEqual(
			TEXT("Publication ProductId exact membership"),
			PublishedIds[Index],
			ExpectedProductIds[Index]);

		FCFProdEquipCatalogEntry Entry;
		if (!TestTrue(
			TEXT("Publication entry readback"),
			PublicationStore.ReadPublishedEntry(
				ExpectedProductIds[Index],
				Entry,
				Error)))
		{
			AddError(Error);
			return false;
		}
		const FString* ExpectedGraph =
			ProductGraphById.Find(ExpectedProductIds[Index]);
		TestTrue(
			TEXT("Publication Product graph evidence exists"),
			ExpectedGraph != nullptr);
		if (ExpectedGraph != nullptr)
		{
			TestEqual(
				TEXT("Publication Product graph fingerprint exact"),
				Entry.ProductGraphFingerprint,
				*ExpectedGraph);
		}
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("CCAS_PROD_CUTOVER_VERIFY|ReviewPackageFingerprint=%s|Workbook=%s|Products=8|Targets=34|Publication=8|Provenance=42|Authority=Active|State=Verified"),
		ApprovedReviewFingerprint,
		ApprovedPostWorkbookHash);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
