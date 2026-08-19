// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchCommitTests.cpp
// Version: v1.0.4
// Date: 2026-08-17
// Description: DAUTH-P0-08L B1/B2 Batch Authoring Source Commit Foundation Automation입니다.
// Changelog:
// - v1.0.4: focused RCA 완료 후 일회성 CommitResult diagnostic log를 제거하고 최종 assertion-only test source로 정리.
// - v1.0.3: valid Current Definition을 Existing Definition import baseline으로 Recipe에 pin해 B2 affected-Vehicle full validation을 실제 통과하는 source fixture로 교정.
// - v1.0.2: Commit 성공/rollback fixture를 PartiallyManaged로 명시해 Managed Recipe의 4 Profile 필수 R0 계약을 의도치 않게 위반하지 않도록 교정.
// - v1.0.1: B2 affected-Vehicle validation을 실제 통과하도록 Recipe fixture Target을 existing Validator-compatible transient vehicle로 보강.
// - v1.0.0: Recipe/Profile atomic commit, stale-all-zero, rollback, External Drift non-blocking, approval invalidation, affected inventory stale 검증 추가.
// Migration:
// - /Temp Recipe/Profile/Target UObject만 사용하며 B3 Definition Apply, UI, disk file save를 수행하지 않습니다.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehicleData.h"
#include "DataAuthoring/CFBatchExport.h"
#include "DataAuthoring/CFBatchImport.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFVehicleImportService.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace CFBatchCommitTestsPrivate
{
	/** B1/B2 Automation이 사용하는 Recipe + Target test pair입니다. */
	struct FRecipeFixture
	{
		// Target Definition package입니다.
		UPackage* TargetPackage = nullptr;

		// Recipe package입니다.
		UPackage* RecipePackage = nullptr;

		// Current Runtime canonical Target Definition입니다.
		UCFVehicleData* TargetVehicleData = nullptr;

				// Current Editor-only Recipe source입니다.
		UCFVehicleRecipeData* Recipe = nullptr;

		// AssetReader/Validator가 current Target에서 읽는 transient Chassis mesh입니다.
		UStaticMesh* ChassisMesh = nullptr;
	};

	// Transient Chassis에 deterministic socket을 추가합니다.
	void AddSocket(UStaticMesh& ChassisMesh, const FName SocketName, const FVector& RelativeLocation)
	{
		// Chassis가 소유하는 transient StaticMesh socket입니다.
		UStaticMeshSocket* Socket = NewObject<UStaticMeshSocket>(&ChassisMesh);
		Socket->SocketName = SocketName;
		Socket->RelativeLocation = RelativeLocation;
		Socket->RelativeRotation = FRotator::ZeroRotator;
		Socket->RelativeScale = FVector::OneVector;
		ChassisMesh.AddSocket(Socket);
	}

	// Automation object를 격리할 unique /Temp package를 만듭니다.
	UPackage* CreateTestPackage(const TCHAR* Prefix)
	{
		// 다른 Automation object identity와 충돌하지 않는 unique package name입니다.
		const FString PackageName = FString::Printf(TEXT("/Temp/%s_%s"), Prefix, *FGuid::NewGuid().ToString(EGuidFormats::Digits));
		return CreatePackage(*PackageName);
	}

	// P0-08J/K/L 공용 contract를 통과하는 deterministic Recipe/Target fixture를 만듭니다.
	bool BuildRecipeFixture(FRecipeFixture& OutFixture, FString& OutError)
	{
		OutFixture = FRecipeFixture();
		OutFixture.TargetPackage = CreateTestPackage(TEXT("CFBatchCommitTarget"));
		OutFixture.RecipePackage = CreateTestPackage(TEXT("CFBatchCommitRecipe"));
		if (!OutFixture.TargetPackage || !OutFixture.RecipePackage)
		{
			OutError = TEXT("Batch Commit /Temp package 생성에 실패했습니다.");
			return false;
		}

		OutFixture.TargetVehicleData = NewObject<UCFVehicleData>(OutFixture.TargetPackage, TEXT("DA_BatchCommitTarget_Test"), RF_Transactional);
		OutFixture.Recipe = NewObject<UCFVehicleRecipeData>(OutFixture.RecipePackage, TEXT("DA_BatchCommitRecipe_Test"), RF_Transactional);
		if (!OutFixture.TargetVehicleData || !OutFixture.Recipe)
		{
			OutError = TEXT("Batch Commit Recipe/Target UObject 생성에 실패했습니다.");
			return false;
		}

				// Validator/AssetReader가 사용할 transient Chassis mesh입니다.
		OutFixture.ChassisMesh = NewObject<UStaticMesh>(GetTransientPackage(), NAME_None, RF_Transient);
		if (!OutFixture.ChassisMesh)
		{
			OutError = TEXT("Batch Commit transient Chassis mesh 생성에 실패했습니다.");
			return false;
		}
		AddSocket(*OutFixture.ChassisMesh, TEXT("Wheel_Anchor_FL"), FVector(110.0, -62.0, 28.0));
		AddSocket(*OutFixture.ChassisMesh, TEXT("Wheel_Anchor_FR"), FVector(110.0, 62.0, 28.0));
		AddSocket(*OutFixture.ChassisMesh, TEXT("Wheel_Anchor_RL"), FVector(-108.0, -62.0, 28.0));
		AddSocket(*OutFixture.ChassisMesh, TEXT("Wheel_Anchor_RR"), FVector(-108.0, 62.0, 28.0));

		// Non-zero bounds와 stable Engine path를 제공하는 read-only wheel mesh입니다.
		UStaticMesh* WheelMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (!WheelMesh)
		{
			OutError = TEXT("Batch Commit Engine Cube wheel mesh를 로드할 수 없습니다.");
			return false;
		}
		// Existing Validator의 required ChaosVehicleWheel class입니다.
		UClass* WheelClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Script/ChaosVehicles.ChaosVehicleWheel"));
		if (!WheelClass)
		{
			OutError = TEXT("Batch Commit ChaosVehicleWheel class를 로드할 수 없습니다.");
			return false;
		}

		OutFixture.TargetVehicleData->VehicleVisualConfig.ChassisMesh = OutFixture.ChassisMesh;
		OutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshFL = WheelMesh;
		OutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshFR = WheelMesh;
		OutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshRL = WheelMesh;
		OutFixture.TargetVehicleData->VehicleVisualConfig.WheelMeshRR = WheelMesh;
		OutFixture.TargetVehicleData->VehicleReferenceConfig.FrontWheelClass = WheelClass;
		OutFixture.TargetVehicleData->VehicleReferenceConfig.RearWheelClass = WheelClass;
		OutFixture.TargetVehicleData->VehicleLayoutConfig.bUseLayoutOverrides = true;
		OutFixture.TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketFL = TEXT("Wheel_Anchor_FL");
		OutFixture.TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketFR = TEXT("Wheel_Anchor_FR");
		OutFixture.TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketRL = TEXT("Wheel_Anchor_RL");
		OutFixture.TargetVehicleData->VehicleLayoutConfig.BodyWheelSocketRR = TEXT("Wheel_Anchor_RR");
		OutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorFL.RelativeLocation = FVector(110.0, -62.0, 28.0);
		OutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorFR.RelativeLocation = FVector(110.0, 62.0, 28.0);
		OutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorRL.RelativeLocation = FVector(-108.0, -62.0, 28.0);
		OutFixture.TargetVehicleData->VehicleLayoutConfig.WheelAnchorRR.RelativeLocation = FVector(-108.0, 62.0, 28.0);
		OutFixture.TargetVehicleData->BaseVehicleMassKg = 1540.0f;
		OutFixture.TargetVehicleData->MaximumGrossMassKg = 2280.0f;

				OutFixture.Recipe->TargetVehicleData = OutFixture.TargetVehicleData;
		// 이 fixture는 Managed Recipe의 4개 필수 Profile 계약을 검증하는 테스트가 아니므로 부분 관리 상태를 명시합니다.
		OutFixture.Recipe->ImportState.ManageState = ECFVehicleManageState::PartiallyManaged;
		OutFixture.Recipe->DrivingFeelIntent.AccelerationFeel = 0.25f;
		OutFixture.Recipe->DrivingFeelIntent.SteeringAgility = 0.50f;
		OutFixture.Recipe->DrivingFeelIntent.GripFeel = 0.75f;
		OutFixture.Recipe->DrivingFeelIntent.SuspensionFirmness = 0.60f;
		OutFixture.Recipe->MassIntent.BaseMassMode = ECFAuthoringInputMode::ExplicitValue;
		OutFixture.Recipe->MassIntent.ExplicitBaseMassKg = 1200.0f;
		OutFixture.Recipe->MassIntent.GrossMassMode = ECFAuthoringInputMode::ExplicitValue;
		OutFixture.Recipe->MassIntent.ExplicitGrossMassKg = 1500.0f;
				OutFixture.Recipe->DurabilityIntent.MaxHealthMode = ECFAuthoringInputMode::ExplicitValue;
		OutFixture.Recipe->DurabilityIntent.ExplicitMaxHealth = 500.0f;

		// B2 affected-Vehicle validation이 current valid Definition을 source로 보존하도록 exact Existing Definition baseline을 import합니다.
		FCFVehicleDefinitionSnapshot CurrentDefinitionSnapshot;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*OutFixture.TargetVehicleData, CurrentDefinitionSnapshot, OutError))
		{
			return false;
		}
		// Existing Definition import result는 Legacy Pin baseline coverage를 확인하는 데 사용됩니다.
		FCFVehicleImportResult ImportResult;
		if (!FCFVehicleImportService::ImportDefinitionSnapshot(CurrentDefinitionSnapshot, *OutFixture.Recipe, ImportResult, OutError))
		{
			return false;
		}
		if (ImportResult.LegacyPinnedFieldCount <= 0)
		{
			OutError = TEXT("Batch Commit fixture Existing Definition import가 Legacy Pin baseline을 만들지 못했습니다.");
			return false;
		}

		OutFixture.TargetPackage->SetDirtyFlag(false);
		OutFixture.RecipePackage->SetDirtyFlag(false);
		OutError.Reset();
		return true;
	}

	// Existing SnapshotBuilder authority로 current Recipe semantic fingerprint를 읽습니다.
	FString BuildRecipeFingerprint(const UCFVehicleRecipeData& Recipe, FString& OutError)
	{
		// Current immutable Recipe snapshot입니다.
		FCFVehicleRecipeSnapshot Snapshot;
		if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(Recipe, Snapshot, OutError))
		{
			return FString();
		}
		return Snapshot.RecipeFingerprint;
	}

	// Existing SnapshotBuilder authority로 current Target Definition hash를 읽습니다.
	FString BuildTargetHash(const UCFVehicleData& Target, FString& OutError)
	{
		// Current immutable full Definition snapshot입니다.
		FCFVehicleDefinitionSnapshot Snapshot;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(Target, Snapshot, OutError))
		{
			return FString();
		}
		return Snapshot.DefinitionHash;
	}

	// P0-08J Profile projection authority로 current Profile semantic fingerprint를 읽습니다.
	FString BuildHandlingProfileFingerprint(UCFHandlingProfile& Profile, FString& OutError)
	{
		// Current Profile canonical export row입니다.
		FCFBatchExportRow Row;
		// Profile row projection diagnostics입니다.
		TArray<FString> Errors;
		if (!FCFBatchExportService::BuildProfileNumericRow(Profile, ECFVehicleProfileDomain::Handling, Row, Errors))
		{
			OutError = FString::Join(Errors, TEXT(" | "));
			return FString();
		}
		OutError.Reset();
		return Row.ObjectFingerprint;
	}

	// Recipe fixtures를 one canonical RecipeNumericEdit artifact로 export합니다.
	bool BuildRecipeArtifact(
		const TArray<FRecipeFixture*>& Fixtures,
		const FGuid& ExportId,
		FCFBatchExportArtifact& OutArtifact,
		TArray<FCFBatchExportRow>& OutRows,
		FString& OutError)
	{
		OutRows.Reset();
		// P0-08J export diagnostics입니다.
		TArray<FString> Errors;
		for (FRecipeFixture* Fixture : Fixtures)
		{
			if (!Fixture || !Fixture->Recipe || !Fixture->TargetVehicleData)
			{
				OutError = TEXT("Recipe export fixture가 null입니다.");
				return false;
			}
			// One current Recipe row입니다.
			FCFBatchExportRow Row;
			if (!FCFBatchExportService::BuildRecipeNumericRow(*Fixture->Recipe, Fixture->TargetVehicleData, Row, Errors))
			{
				OutError = FString::Join(Errors, TEXT(" | "));
				return false;
			}
			OutRows.Add(MoveTemp(Row));
		}

		// Canonical RecipeNumericEdit export request입니다.
		FCFBatchExportRequest Request;
		Request.BatchExportId = ExportId;
		Request.DatasetKind = ECFBatchDatasetKind::RecipeNumericEdit;
		Request.Rows = OutRows;
		if (!FCFBatchExportService::BuildExport(Request, OutArtifact, Errors))
		{
			OutError = FString::Join(Errors, TEXT(" | "));
			return false;
		}
		OutError.Reset();
		return true;
	}

	// One Handling Profile을 canonical ProfileNumericEdit artifact로 export합니다.
	bool BuildHandlingProfileArtifact(
		UCFHandlingProfile& Profile,
		const FGuid& ExportId,
		FCFBatchExportArtifact& OutArtifact,
		FCFBatchExportRow& OutRow,
		FString& OutError)
	{
		// P0-08J export diagnostics입니다.
		TArray<FString> Errors;
		if (!FCFBatchExportService::BuildProfileNumericRow(Profile, ECFVehicleProfileDomain::Handling, OutRow, Errors))
		{
			OutError = FString::Join(Errors, TEXT(" | "));
			return false;
		}

		// Canonical one-domain Profile export request입니다.
		FCFBatchExportRequest Request;
		Request.BatchExportId = ExportId;
		Request.DatasetKind = ECFBatchDatasetKind::ProfileNumericEdit;
		Request.ProfileDomain = ECFVehicleProfileDomain::Handling;
		Request.Rows.Add(OutRow);
		if (!FCFBatchExportService::BuildExport(Request, OutArtifact, Errors))
		{
			OutError = FString::Join(Errors, TEXT(" | "));
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Comma-safe Automation CSV에서 exact ColumnId의 모든 data-row value를 교체합니다.
	bool ReplaceSimpleCsvColumnAllRows(
		const FString& CsvText,
		const FString& ColumnId,
		const FString& NewValue,
		FString& OutCsvText)
	{
		// Canonical exporter가 만든 non-empty physical lines입니다.
		TArray<FString> Lines;
		CsvText.ParseIntoArray(Lines, TEXT("\n"), true);
		if (Lines.Num() < 2)
		{
			return false;
		}

		// Simple Automation header cells입니다.
		TArray<FString> HeaderCells;
		Lines[0].ParseIntoArray(HeaderCells, TEXT(","), false);
		// Stable technical ColumnId의 physical index입니다.
		const int32 ColumnIndex = HeaderCells.Find(ColumnId);
		if (ColumnIndex == INDEX_NONE)
		{
			return false;
		}

		for (int32 LineIndex = 1; LineIndex < Lines.Num(); ++LineIndex)
		{
			// Simple Automation data row cells입니다.
			TArray<FString> DataCells;
			Lines[LineIndex].ParseIntoArray(DataCells, TEXT(","), false);
			if (!DataCells.IsValidIndex(ColumnIndex))
			{
				return false;
			}
			DataCells[ColumnIndex] = NewValue;
			Lines[LineIndex] = FString::Join(DataCells, TEXT(","));
		}
		OutCsvText = FString::Join(Lines, TEXT("\n")) + TEXT("\n");
		return true;
	}

	// Edited CSV와 frozen manifest로 K Preview + L exact approval을 연속 생성합니다.
	bool BuildPreviewAndApproval(
		const FCFBatchExportArtifact& Artifact,
		const FString& EditedCsv,
		FCFBatchImportSession& OutSession,
		FCFBatchCommitApproval& OutApproval,
		FString& OutError)
	{
		// In-memory K Preview request입니다.
		FCFBatchImportRequest PreviewRequest;
		PreviewRequest.CsvText = EditedCsv;
		PreviewRequest.ManifestJsonText = Artifact.ManifestJsonText;
		if (!FCFBatchImportService::BuildPreview(PreviewRequest, OutSession))
		{
			OutError = TEXT("Batch Preview 생성에 실패했습니다.");
			return false;
		}

		// L approval build diagnostics입니다.
		TArray<FCFBatchIssue> ApprovalIssues;
		if (!FCFBatchImportService::BuildCommitApproval(OutSession, OutApproval, ApprovalIssues))
		{
			OutError = !ApprovalIssues.IsEmpty() ? ApprovalIssues[0].Message : TEXT("Batch Commit Approval 생성에 실패했습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Exact reviewed Session/Approval로 명시적 B1/B2 commit request를 실행합니다.
	bool CommitApprovedSession(
		const FCFBatchImportSession& Session,
		const FCFBatchCommitApproval& Approval,
		FCFBatchAuthoringCommitResult& OutResult,
		const int32 OptionalAutomationFailureAfterPatchedObjectCount = INDEX_NONE)
	{
		// Explicit B1/B2 Authoring Source Commit request입니다.
		FCFBatchAuthoringCommitRequest CommitRequest;
		CommitRequest.Session = &Session;
		CommitRequest.Approval = &Approval;
		CommitRequest.bAuthoringCommitApproved = true;
		CommitRequest.AutomationFailureAfterPatchedObjectCount = OptionalAutomationFailureAfterPatchedObjectCount;
		return FCFBatchImportService::CommitAuthoringSources(CommitRequest, OutResult);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchCommitRecipeAtomicTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchCommit.RecipeAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchCommitStaleZeroTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchCommit.StaleAllZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchCommitRollbackTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchCommit.Rollback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchCommitDriftTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchCommit.ExternalDrift",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchCommitProfileTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchCommit.ProfileInvalidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchCommitProfileInventoryTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchCommit.ProfileInventoryStale",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// 두 Recipe row를 한 approval/transaction으로 commit하고 Target/save 경계를 보존하는지 검증합니다.
bool FCFBatchCommitRecipeAtomicTest::RunTest(const FString& Parameters)
{
	// First Recipe fixture입니다.
	CFBatchCommitTestsPrivate::FRecipeFixture FirstFixture;
	// Second Recipe fixture입니다.
	CFBatchCommitTestsPrivate::FRecipeFixture SecondFixture;
	// Fixture/export diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("RecipeAtomic first fixture builds"), CFBatchCommitTestsPrivate::BuildRecipeFixture(FirstFixture, Error))
		|| !TestTrue(TEXT("RecipeAtomic second fixture builds"), CFBatchCommitTestsPrivate::BuildRecipeFixture(SecondFixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Frozen two-row export artifact입니다.
	FCFBatchExportArtifact Artifact;
	// Frozen two-row export rows입니다.
	TArray<FCFBatchExportRow> ExportRows;
	TArray<CFBatchCommitTestsPrivate::FRecipeFixture*> Fixtures = {&FirstFixture, &SecondFixture};
	if (!TestTrue(TEXT("RecipeAtomic export builds"), CFBatchCommitTestsPrivate::BuildRecipeArtifact(
		Fixtures,
		FGuid(0x11000001, 0x11000002, 0x11000003, 0x11000004),
		Artifact,
		ExportRows,
		Error)))
	{
		AddError(Error);
		return false;
	}

	// Both Recipe rows가 요청할 reviewed numeric edit입니다.
	FString EditedCsv;
	TestTrue(TEXT("RecipeAtomic CSV edit builds"), CFBatchCommitTestsPrivate::ReplaceSimpleCsvColumnAllRows(
		Artifact.CanonicalCsvText,
		TEXT("Recipe.DrivingFeel.AccelerationFeel"),
		TEXT("0.8"),
		EditedCsv));
	// Reviewed K Session입니다.
	FCFBatchImportSession Session;
	// Exact L approval입니다.
	FCFBatchCommitApproval Approval;
	if (!TestTrue(TEXT("RecipeAtomic preview/approval builds"), CFBatchCommitTestsPrivate::BuildPreviewAndApproval(Artifact, EditedCsv, Session, Approval, Error)))
	{
		AddError(Error);
		return false;
	}
	TestEqual(TEXT("Approval binds both Recipe rows"), Approval.IncludedRows.Num(), 2);

	// Commit 전 first Target hash입니다.
	const FString FirstTargetHashBefore = CFBatchCommitTestsPrivate::BuildTargetHash(*FirstFixture.TargetVehicleData, Error);
	// Commit 전 second Target hash입니다.
	const FString SecondTargetHashBefore = CFBatchCommitTestsPrivate::BuildTargetHash(*SecondFixture.TargetVehicleData, Error);
	// Commit 전 first revision입니다.
	const int32 FirstRevisionBefore = FirstFixture.Recipe->AuthoringRevision;
	// Commit 전 second revision입니다.
	const int32 SecondRevisionBefore = SecondFixture.Recipe->AuthoringRevision;

	// B1 terminal result입니다.
		FCFBatchAuthoringCommitResult CommitResult;
		const bool bCommitSucceeded = CFBatchCommitTestsPrivate::CommitApprovedSession(Session, Approval, CommitResult);
	TestTrue(TEXT("RecipeAtomic B1 commit succeeds"), bCommitSucceeded);
	TestEqual(TEXT("RecipeAtomic status succeeds"), CommitResult.Status, ECFBatchCommitStatus::Succeeded);
	TestEqual(TEXT("RecipeAtomic commits two rows"), CommitResult.CommittedRowCount, 2);
	TestEqual(TEXT("RecipeAtomic commits two sources"), CommitResult.CommittedSourceObjectCount, 2);
	TestTrue(TEXT("Successful approval is consumed"), CommitResult.bApprovalConsumed);
	TestTrue(TEXT("Successful B1 requires fresh preview"), CommitResult.bRequiresFreshPreview);
	TestFalse(TEXT("B1 Target mutation remains zero"), CommitResult.bTargetMutationPerformed);
	TestFalse(TEXT("B1 auto-save remains zero"), CommitResult.bSavePerformed);
	TestEqual(TEXT("First Recipe numeric patch committed"), FirstFixture.Recipe->DrivingFeelIntent.AccelerationFeel, 0.8f);
	TestEqual(TEXT("Second Recipe numeric patch committed"), SecondFixture.Recipe->DrivingFeelIntent.AccelerationFeel, 0.8f);
	TestEqual(TEXT("First Recipe revision increments once"), FirstFixture.Recipe->AuthoringRevision, FirstRevisionBefore + 1);
	TestEqual(TEXT("Second Recipe revision increments once"), SecondFixture.Recipe->AuthoringRevision, SecondRevisionBefore + 1);
	TestTrue(TEXT("First Recipe package dirty on success"), FirstFixture.RecipePackage->IsDirty());
	TestTrue(TEXT("Second Recipe package dirty on success"), SecondFixture.RecipePackage->IsDirty());
	TestEqual(TEXT("First Target hash unchanged by B1"), CFBatchCommitTestsPrivate::BuildTargetHash(*FirstFixture.TargetVehicleData, Error), FirstTargetHashBefore);
	TestEqual(TEXT("Second Target hash unchanged by B1"), CFBatchCommitTestsPrivate::BuildTargetHash(*SecondFixture.TargetVehicleData, Error), SecondTargetHashBefore);
	return true;
}

// Included source 하나라도 approval 이후 stale이면 어느 Recipe도 mutation하지 않는 global preflight atomicity를 검증합니다.
bool FCFBatchCommitStaleZeroTest::RunTest(const FString& Parameters)
{
	// First Recipe fixture입니다.
	CFBatchCommitTestsPrivate::FRecipeFixture FirstFixture;
	// Second Recipe fixture입니다.
	CFBatchCommitTestsPrivate::FRecipeFixture SecondFixture;
	// Fixture/export diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("StaleZero first fixture builds"), CFBatchCommitTestsPrivate::BuildRecipeFixture(FirstFixture, Error))
		|| !TestTrue(TEXT("StaleZero second fixture builds"), CFBatchCommitTestsPrivate::BuildRecipeFixture(SecondFixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Frozen two-row export artifact입니다.
	FCFBatchExportArtifact Artifact;
	// Frozen export rows입니다.
	TArray<FCFBatchExportRow> ExportRows;
	TArray<CFBatchCommitTestsPrivate::FRecipeFixture*> Fixtures = {&FirstFixture, &SecondFixture};
	TestTrue(TEXT("StaleZero export builds"), CFBatchCommitTestsPrivate::BuildRecipeArtifact(
		Fixtures,
		FGuid(0x12000001, 0x12000002, 0x12000003, 0x12000004),
		Artifact,
		ExportRows,
		Error));
	// Reviewed CSV edit입니다.
	FString EditedCsv;
	TestTrue(TEXT("StaleZero CSV edit builds"), CFBatchCommitTestsPrivate::ReplaceSimpleCsvColumnAllRows(
		Artifact.CanonicalCsvText,
		TEXT("Recipe.DrivingFeel.AccelerationFeel"),
		TEXT("0.8"),
		EditedCsv));
	// Reviewed K Session입니다.
	FCFBatchImportSession Session;
	// Exact L approval입니다.
	FCFBatchCommitApproval Approval;
	if (!TestTrue(TEXT("StaleZero preview/approval builds"), CFBatchCommitTestsPrivate::BuildPreviewAndApproval(Artifact, EditedCsv, Session, Approval, Error)))
	{
		AddError(Error);
		return false;
	}

	// First Recipe preflight baseline value입니다.
	const float FirstValueBefore = FirstFixture.Recipe->DrivingFeelIntent.AccelerationFeel;
	// First Recipe preflight baseline revision입니다.
	const int32 FirstRevisionBefore = FirstFixture.Recipe->AuthoringRevision;
	// First Recipe preflight baseline dirty state입니다.
	const bool bFirstDirtyBefore = FirstFixture.RecipePackage->IsDirty();

	// Approval 이후 발생한 independent current Unreal source edit입니다.
	SecondFixture.Recipe->DrivingFeelIntent.AccelerationFeel = 0.4f;
	// Stale external source edit 자체는 package state를 바꾸지 않은 baseline입니다.
	SecondFixture.RecipePackage->SetDirtyFlag(false);

	// B1 stale terminal result입니다.
	FCFBatchAuthoringCommitResult CommitResult;
	TestFalse(TEXT("One stale source blocks whole B1 before transaction"), CFBatchCommitTestsPrivate::CommitApprovedSession(Session, Approval, CommitResult));
	TestEqual(TEXT("Stale source taxonomy"), CommitResult.ErrorCode, ECFBatchCommitErrorCode::SourceFingerprintMismatch);
	TestTrue(TEXT("Stale row id is surfaced"), CommitResult.StaleRowIds.Num() >= 1);
	TestEqual(TEXT("Fresh first Recipe remains mutation zero"), FirstFixture.Recipe->DrivingFeelIntent.AccelerationFeel, FirstValueBefore);
	TestEqual(TEXT("Fresh first Recipe revision remains zero-delta"), FirstFixture.Recipe->AuthoringRevision, FirstRevisionBefore);
	TestEqual(TEXT("Fresh first Recipe dirty state unchanged"), FirstFixture.RecipePackage->IsDirty(), bFirstDirtyBefore);
	TestEqual(TEXT("Independent stale edit is preserved"), SecondFixture.Recipe->DrivingFeelIntent.AccelerationFeel, 0.4f);
	TestEqual(TEXT("No source object reports committed"), CommitResult.CommittedSourceObjectCount, 0);
	TestFalse(TEXT("Stale failure never consumes approval as success"), CommitResult.bApprovalConsumed);
	return true;
}

// 실제 first source patch 뒤 injected failure를 발생시켜 entire Batch rollback과 dirty-state 복원을 검증합니다.
bool FCFBatchCommitRollbackTest::RunTest(const FString& Parameters)
{
	// First Recipe fixture입니다.
	CFBatchCommitTestsPrivate::FRecipeFixture FirstFixture;
	// Second Recipe fixture입니다.
	CFBatchCommitTestsPrivate::FRecipeFixture SecondFixture;
	// Fixture/export diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Rollback first fixture builds"), CFBatchCommitTestsPrivate::BuildRecipeFixture(FirstFixture, Error))
		|| !TestTrue(TEXT("Rollback second fixture builds"), CFBatchCommitTestsPrivate::BuildRecipeFixture(SecondFixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Frozen two-row export artifact입니다.
	FCFBatchExportArtifact Artifact;
	// Frozen export rows입니다.
	TArray<FCFBatchExportRow> ExportRows;
	TArray<CFBatchCommitTestsPrivate::FRecipeFixture*> Fixtures = {&FirstFixture, &SecondFixture};
	TestTrue(TEXT("Rollback export builds"), CFBatchCommitTestsPrivate::BuildRecipeArtifact(
		Fixtures,
		FGuid(0x13000001, 0x13000002, 0x13000003, 0x13000004),
		Artifact,
		ExportRows,
		Error));
	// Reviewed CSV edit입니다.
	FString EditedCsv;
	TestTrue(TEXT("Rollback CSV edit builds"), CFBatchCommitTestsPrivate::ReplaceSimpleCsvColumnAllRows(
		Artifact.CanonicalCsvText,
		TEXT("Recipe.DrivingFeel.AccelerationFeel"),
		TEXT("0.8"),
		EditedCsv));
	// Reviewed K Session입니다.
	FCFBatchImportSession Session;
	// Exact L approval입니다.
	FCFBatchCommitApproval Approval;
	if (!TestTrue(TEXT("Rollback preview/approval builds"), CFBatchCommitTestsPrivate::BuildPreviewAndApproval(Artifact, EditedCsv, Session, Approval, Error)))
	{
		AddError(Error);
		return false;
	}

	// First Recipe semantic fingerprint before transaction입니다.
	const FString FirstFingerprintBefore = CFBatchCommitTestsPrivate::BuildRecipeFingerprint(*FirstFixture.Recipe, Error);
	// Second Recipe semantic fingerprint before transaction입니다.
	const FString SecondFingerprintBefore = CFBatchCommitTestsPrivate::BuildRecipeFingerprint(*SecondFixture.Recipe, Error);
	// First revision baseline입니다.
	const int32 FirstRevisionBefore = FirstFixture.Recipe->AuthoringRevision;
	// Second revision baseline입니다.
	const int32 SecondRevisionBefore = SecondFixture.Recipe->AuthoringRevision;
	// First package dirty baseline입니다.
	const bool bFirstDirtyBefore = FirstFixture.RecipePackage->IsDirty();
	// Second package dirty baseline입니다.
	const bool bSecondDirtyBefore = SecondFixture.RecipePackage->IsDirty();

	// Injected rollback terminal result입니다.
		FCFBatchAuthoringCommitResult CommitResult;
		const bool bCommitSucceeded = CFBatchCommitTestsPrivate::CommitApprovedSession(Session, Approval, CommitResult, 1);
	TestFalse(TEXT("Injected failure returns false"), bCommitSucceeded);
	TestEqual(TEXT("Injected failure reports rollback"), CommitResult.Status, ECFBatchCommitStatus::FailedRolledBack);
	TestEqual(TEXT("Injected failure taxonomy"), CommitResult.ErrorCode, ECFBatchCommitErrorCode::AutomationInjectedFailure);
	TestEqual(TEXT("First Recipe fingerprint fully restored"), CFBatchCommitTestsPrivate::BuildRecipeFingerprint(*FirstFixture.Recipe, Error), FirstFingerprintBefore);
	TestEqual(TEXT("Second Recipe fingerprint fully restored"), CFBatchCommitTestsPrivate::BuildRecipeFingerprint(*SecondFixture.Recipe, Error), SecondFingerprintBefore);
	TestEqual(TEXT("First Recipe revision restored"), FirstFixture.Recipe->AuthoringRevision, FirstRevisionBefore);
	TestEqual(TEXT("Second Recipe revision restored"), SecondFixture.Recipe->AuthoringRevision, SecondRevisionBefore);
	TestEqual(TEXT("First package dirty restored"), FirstFixture.RecipePackage->IsDirty(), bFirstDirtyBefore);
	TestEqual(TEXT("Second package dirty restored"), SecondFixture.RecipePackage->IsDirty(), bSecondDirtyBefore);
	TestEqual(TEXT("Rollback reports no committed source"), CommitResult.CommittedSourceObjectCount, 0);
	return true;
}

// Approval 이후 Target Definition만 drift해도 B1 source fingerprint가 fresh하면 source commit을 허용하는지 검증합니다.
bool FCFBatchCommitDriftTest::RunTest(const FString& Parameters)
{
	// One Recipe/Target fixture입니다.
	CFBatchCommitTestsPrivate::FRecipeFixture Fixture;
	// Fixture/export diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("ExternalDrift fixture builds"), CFBatchCommitTestsPrivate::BuildRecipeFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Frozen one-row export artifact입니다.
	FCFBatchExportArtifact Artifact;
	// Frozen export row입니다.
	TArray<FCFBatchExportRow> ExportRows;
	TArray<CFBatchCommitTestsPrivate::FRecipeFixture*> Fixtures = {&Fixture};
	TestTrue(TEXT("ExternalDrift export builds"), CFBatchCommitTestsPrivate::BuildRecipeArtifact(
		Fixtures,
		FGuid(0x14000001, 0x14000002, 0x14000003, 0x14000004),
		Artifact,
		ExportRows,
		Error));
	// Reviewed CSV edit입니다.
	FString EditedCsv;
	TestTrue(TEXT("ExternalDrift CSV edit builds"), CFBatchCommitTestsPrivate::ReplaceSimpleCsvColumnAllRows(
		Artifact.CanonicalCsvText,
		TEXT("Recipe.DrivingFeel.AccelerationFeel"),
		TEXT("0.8"),
		EditedCsv));
	// Reviewed K Session입니다.
	FCFBatchImportSession Session;
	// Exact L approval입니다.
	FCFBatchCommitApproval Approval;
	if (!TestTrue(TEXT("ExternalDrift preview/approval builds"), CFBatchCommitTestsPrivate::BuildPreviewAndApproval(Artifact, EditedCsv, Session, Approval, Error)))
	{
		AddError(Error);
		return false;
	}

	// Approval 이전 Target hash입니다.
	const FString TargetHashBeforeDrift = CFBatchCommitTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	// Approval 이후 source와 무관하게 발생한 current Unreal Target drift입니다.
	Fixture.TargetVehicleData->DestroyedFxSocketName = TEXT("ExternalDrift_AfterApproval");
	// Drift 이후 Target hash입니다.
	const FString TargetHashAfterDrift = CFBatchCommitTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	TestNotEqual(TEXT("Manual Target drift changes Definition hash"), TargetHashAfterDrift, TargetHashBeforeDrift);

	// Drift가 있는 상태의 B1 terminal result입니다.
		FCFBatchAuthoringCommitResult CommitResult;
		const bool bCommitSucceeded = CFBatchCommitTestsPrivate::CommitApprovedSession(Session, Approval, CommitResult);
	TestTrue(TEXT("Target-only External Drift does not block B1 source commit"), bCommitSucceeded);
	TestEqual(TEXT("B1 source value commits despite unrelated Target drift"), Fixture.Recipe->DrivingFeelIntent.AccelerationFeel, 0.8f);
	TestEqual(TEXT("B1 never overwrites externally drifted Target"), CFBatchCommitTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashAfterDrift);
	TestFalse(TEXT("ExternalDrift B1 still reports Target mutation zero"), CommitResult.bTargetMutationPerformed);
	return true;
}

// Shared Handling Profile을 B2로 commit하고 old approval 재사용이 source fingerprint에서 즉시 stale 되는지 검증합니다.
bool FCFBatchCommitProfileTest::RunTest(const FString& Parameters)
{
	// Affected Recipe/Target fixture입니다.
	CFBatchCommitTestsPrivate::FRecipeFixture Fixture;
	// Fixture/export diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("ProfileCommit Recipe fixture builds"), CFBatchCommitTestsPrivate::BuildRecipeFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Shared Handling Profile package입니다.
	UPackage* ProfilePackage = CFBatchCommitTestsPrivate::CreateTestPackage(TEXT("CFBatchCommitProfile"));
	// Current shared Handling Profile입니다.
	UCFHandlingProfile* HandlingProfile = NewObject<UCFHandlingProfile>(ProfilePackage, TEXT("DA_BatchCommitHandling_Test"), RF_Transactional);
	if (!TestNotNull(TEXT("ProfileCommit Handling Profile creates"), HandlingProfile))
	{
		return false;
	}
	HandlingProfile->Data.FrontWheelMaxBrakeTorque = 1500.0f;
	Fixture.Recipe->ProfileBindings.HandlingProfile = HandlingProfile;
	ProfilePackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);
	Fixture.TargetPackage->SetDirtyFlag(false);

	// Frozen Profile export artifact입니다.
	FCFBatchExportArtifact Artifact;
	// Frozen Profile export row입니다.
	FCFBatchExportRow ExportRow;
	if (!TestTrue(TEXT("ProfileCommit export builds"), CFBatchCommitTestsPrivate::BuildHandlingProfileArtifact(
		*HandlingProfile,
		FGuid(0x15000001, 0x15000002, 0x15000003, 0x15000004),
		Artifact,
		ExportRow,
		Error)))
	{
		AddError(Error);
		return false;
	}
	// Reviewed Profile numeric edit입니다.
	FString EditedCsv;
	TestTrue(TEXT("ProfileCommit CSV edit builds"), CFBatchCommitTestsPrivate::ReplaceSimpleCsvColumnAllRows(
		Artifact.CanonicalCsvText,
		TEXT("Profile.Handling.FrontWheelMaxBrakeTorque"),
		TEXT("2000"),
		EditedCsv));
	// Reviewed K Session입니다.
	FCFBatchImportSession Session;
	// Exact L approval입니다.
	FCFBatchCommitApproval Approval;
	if (!TestTrue(TEXT("ProfileCommit preview/approval builds"), CFBatchCommitTestsPrivate::BuildPreviewAndApproval(Artifact, EditedCsv, Session, Approval, Error)))
	{
		AddError(Error);
		return false;
	}

	// Profile commit 전 semantic fingerprint입니다.
	const FString ProfileFingerprintBefore = CFBatchCommitTestsPrivate::BuildHandlingProfileFingerprint(*HandlingProfile, Error);
	// Dependent Recipe fingerprint before B2입니다.
	const FString RecipeFingerprintBefore = CFBatchCommitTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error);
	// Dependent Recipe revision before B2입니다.
	const int32 RecipeRevisionBefore = Fixture.Recipe->AuthoringRevision;
	// Profile diagnostic revision before B2입니다.
	const int32 ProfileRevisionBefore = HandlingProfile->Meta.AuthoringRevision;

	// B2 approval 뒤 발생한 unrelated dependent Target drift입니다.
	Fixture.TargetVehicleData->DestroyedFxSocketName = TEXT("ProfileExternalDrift_AfterApproval");
	// B2가 보존해야 하는 manual drifted Target hash입니다.
	const FString TargetHashAfterDrift = CFBatchCommitTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);

	// B2 terminal result입니다.
		FCFBatchAuthoringCommitResult CommitResult;
		const bool bCommitSucceeded = CFBatchCommitTestsPrivate::CommitApprovedSession(Session, Approval, CommitResult);
	TestTrue(TEXT("Profile B2 commit succeeds despite dependent Target drift"), bCommitSucceeded);
	TestEqual(TEXT("Profile numeric value committed"), HandlingProfile->Data.FrontWheelMaxBrakeTorque, 2000.0f);
	TestEqual(TEXT("Profile revision increments once"), HandlingProfile->Meta.AuthoringRevision, ProfileRevisionBefore + 1);
	TestNotEqual(TEXT("Profile semantic fingerprint changes"), CFBatchCommitTestsPrivate::BuildHandlingProfileFingerprint(*HandlingProfile, Error), ProfileFingerprintBefore);
	TestTrue(TEXT("Profile package dirty on success"), ProfilePackage->IsDirty());
	TestEqual(TEXT("Dependent Recipe semantic fingerprint unchanged"), CFBatchCommitTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error), RecipeFingerprintBefore);
	TestEqual(TEXT("Dependent Recipe revision unchanged"), Fixture.Recipe->AuthoringRevision, RecipeRevisionBefore);
	TestEqual(TEXT("Dependent Target remains manually drifted and un-applied"), CFBatchCommitTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashAfterDrift);
	TestFalse(TEXT("B2 Target mutation remains zero"), CommitResult.bTargetMutationPerformed);
	TestFalse(TEXT("B2 auto-save remains zero"), CommitResult.bSavePerformed);
	TestTrue(TEXT("B2 success requires fresh read/resolve"), CommitResult.bRequiresFreshPreview);

	// Same old Session/Approval reuse terminal result입니다.
	FCFBatchAuthoringCommitResult ReplayResult;
	TestFalse(TEXT("Successful B2 invalidates old approval"), CFBatchCommitTestsPrivate::CommitApprovedSession(Session, Approval, ReplayResult));
	TestEqual(TEXT("Old approval fails on current source fingerprint"), ReplayResult.ErrorCode, ECFBatchCommitErrorCode::SourceFingerprintMismatch);
	TestEqual(TEXT("Old approval reuse never increments Profile revision twice"), HandlingProfile->Meta.AuthoringRevision, ProfileRevisionBefore + 1);
	return true;
}

// B2 approval 이후 shared Profile을 새 Recipe가 참조하면 affected inventory stale로 Profile mutation0이 되는지 검증합니다.
bool FCFBatchCommitProfileInventoryTest::RunTest(const FString& Parameters)
{
	// Original affected Recipe fixture입니다.
	CFBatchCommitTestsPrivate::FRecipeFixture OriginalFixture;
	// Fixture/export diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("ProfileInventory original fixture builds"), CFBatchCommitTestsPrivate::BuildRecipeFixture(OriginalFixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Shared Handling Profile package입니다.
	UPackage* ProfilePackage = CFBatchCommitTestsPrivate::CreateTestPackage(TEXT("CFBatchInventoryProfile"));
	// Current shared Handling Profile입니다.
	UCFHandlingProfile* HandlingProfile = NewObject<UCFHandlingProfile>(ProfilePackage, TEXT("DA_BatchInventoryHandling_Test"), RF_Transactional);
	if (!TestNotNull(TEXT("ProfileInventory Handling Profile creates"), HandlingProfile))
	{
		return false;
	}
	HandlingProfile->Data.FrontWheelMaxBrakeTorque = 1500.0f;
	OriginalFixture.Recipe->ProfileBindings.HandlingProfile = HandlingProfile;
	ProfilePackage->SetDirtyFlag(false);

	// Frozen Profile export artifact입니다.
	FCFBatchExportArtifact Artifact;
	// Frozen Profile export row입니다.
	FCFBatchExportRow ExportRow;
	TestTrue(TEXT("ProfileInventory export builds"), CFBatchCommitTestsPrivate::BuildHandlingProfileArtifact(
		*HandlingProfile,
		FGuid(0x16000001, 0x16000002, 0x16000003, 0x16000004),
		Artifact,
		ExportRow,
		Error));
	// Reviewed Profile numeric edit입니다.
	FString EditedCsv;
	TestTrue(TEXT("ProfileInventory CSV edit builds"), CFBatchCommitTestsPrivate::ReplaceSimpleCsvColumnAllRows(
		Artifact.CanonicalCsvText,
		TEXT("Profile.Handling.FrontWheelMaxBrakeTorque"),
		TEXT("2000"),
		EditedCsv));
	// Reviewed K Session입니다.
	FCFBatchImportSession Session;
	// Exact L approval입니다.
	FCFBatchCommitApproval Approval;
	if (!TestTrue(TEXT("ProfileInventory preview/approval builds"), CFBatchCommitTestsPrivate::BuildPreviewAndApproval(Artifact, EditedCsv, Session, Approval, Error)))
	{
		AddError(Error);
		return false;
	}

	// Approval 뒤 새로 profile binding되는 Recipe fixture입니다.
	CFBatchCommitTestsPrivate::FRecipeFixture LateFixture;
	TestTrue(TEXT("ProfileInventory late fixture builds"), CFBatchCommitTestsPrivate::BuildRecipeFixture(LateFixture, Error));
	LateFixture.Recipe->ProfileBindings.HandlingProfile = HandlingProfile;

	// Mutation0 검증용 Profile value baseline입니다.
	const float ProfileValueBefore = HandlingProfile->Data.FrontWheelMaxBrakeTorque;
	// Mutation0 검증용 Profile fingerprint baseline입니다.
	const FString ProfileFingerprintBefore = CFBatchCommitTestsPrivate::BuildHandlingProfileFingerprint(*HandlingProfile, Error);
	// Mutation0 검증용 Profile revision baseline입니다.
	const int32 ProfileRevisionBefore = HandlingProfile->Meta.AuthoringRevision;
	// Mutation0 검증용 dirty state baseline입니다.
	const bool bProfileDirtyBefore = ProfilePackage->IsDirty();

	// B2 affected-inventory stale terminal result입니다.
	FCFBatchAuthoringCommitResult CommitResult;
	TestFalse(TEXT("Changed affected Recipe inventory blocks B2 before mutation"), CFBatchCommitTestsPrivate::CommitApprovedSession(Session, Approval, CommitResult));
	TestEqual(TEXT("Affected inventory stale taxonomy"), CommitResult.ErrorCode, ECFBatchCommitErrorCode::AffectedRecipeInventoryChanged);
	TestEqual(TEXT("Profile value remains mutation zero"), HandlingProfile->Data.FrontWheelMaxBrakeTorque, ProfileValueBefore);
	TestEqual(TEXT("Profile fingerprint remains mutation zero"), CFBatchCommitTestsPrivate::BuildHandlingProfileFingerprint(*HandlingProfile, Error), ProfileFingerprintBefore);
	TestEqual(TEXT("Profile revision remains mutation zero"), HandlingProfile->Meta.AuthoringRevision, ProfileRevisionBefore);
	TestEqual(TEXT("Profile package dirty state unchanged"), ProfilePackage->IsDirty(), bProfileDirtyBefore);
	TestEqual(TEXT("No Profile source reports committed"), CommitResult.CommittedSourceObjectCount, 0);
	return true;
}

#endif
