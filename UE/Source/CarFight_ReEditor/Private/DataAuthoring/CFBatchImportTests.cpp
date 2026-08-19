// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchImportTests.cpp
// Version: v1.0.1
// Date: 2026-08-17
// Description: DAUTH-P0-08K Batch Import Session / 3-way Preview Foundation Automation입니다.
// Changelog:
// - v1.0.1: physical row/column reverse helper의 Algo::Reverse explicit include를 추가.
// - v1.0.0: 3-way 분류, manifest/read-only/duplicate guard, order-independence, Recipe/Profile prospective preview와 persistent mutation0 검증 추가.
// Migration:
// - /Temp UObject와 in-memory CSV/manifest만 사용하며 B1/B2/B3 commit/apply, UI, disk file save를 수행하지 않습니다.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Algo/Reverse.h"

#include "CFVehicleData.h"
#include "DataAuthoring/CFBatchExport.h"
#include "DataAuthoring/CFBatchImport.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace CFBatchImportTestsPrivate
{
	/** Recipe/Target 한 쌍의 persistent-like /Temp test fixture입니다. */
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
	};

	// Automation object를 격리할 unique /Temp package를 만듭니다.
	UPackage* CreateTestPackage(const TCHAR* Prefix)
	{
		// 다른 Automation run과 충돌하지 않는 unique package name입니다.
		const FString PackageName = FString::Printf(TEXT("/Temp/%s_%s"), Prefix, *FGuid::NewGuid().ToString(EGuidFormats::Digits));
		return CreatePackage(*PackageName);
	}

	// P0-08J export와 P0-08K current reread에 필요한 deterministic Recipe/Target fixture를 만듭니다.
	bool BuildRecipeFixture(FRecipeFixture& OutFixture, FString& OutError)
	{
		OutFixture = FRecipeFixture();
		OutFixture.TargetPackage = CreateTestPackage(TEXT("CFBatchImportTarget"));
		OutFixture.RecipePackage = CreateTestPackage(TEXT("CFBatchImportRecipe"));
		if (!OutFixture.TargetPackage || !OutFixture.RecipePackage)
		{
			OutError = TEXT("Batch Import /Temp package 생성에 실패했습니다.");
			return false;
		}

		OutFixture.TargetVehicleData = NewObject<UCFVehicleData>(OutFixture.TargetPackage, TEXT("DA_BatchImportTarget_Test"), RF_Transactional);
		OutFixture.Recipe = NewObject<UCFVehicleRecipeData>(OutFixture.RecipePackage, TEXT("DA_BatchImportRecipe_Test"), RF_Transactional);
		if (!OutFixture.TargetVehicleData || !OutFixture.Recipe)
		{
			OutError = TEXT("Batch Import Recipe/Target UObject 생성에 실패했습니다.");
			return false;
		}

		OutFixture.Recipe->TargetVehicleData = OutFixture.TargetVehicleData;
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

		OutFixture.TargetPackage->SetDirtyFlag(false);
		OutFixture.RecipePackage->SetDirtyFlag(false);
		OutError.Reset();
		return true;
	}

	// Existing SnapshotBuilder authority로 current Recipe fingerprint를 읽습니다.
	FString BuildRecipeFingerprint(const UCFVehicleRecipeData& Recipe, FString& OutError)
	{
		// Existing immutable Recipe snapshot입니다.
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
		// Existing immutable full Definition snapshot입니다.
		FCFVehicleDefinitionSnapshot Snapshot;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(Target, Snapshot, OutError))
		{
			return FString();
		}
		return Snapshot.DefinitionHash;
	}

	// One Recipe row와 companion manifest를 P0-08J canonical exporter로 생성합니다.
	bool BuildRecipeArtifact(
		FRecipeFixture& Fixture,
		const FGuid& ExportId,
		FCFBatchExportArtifact& OutArtifact,
		FCFBatchExportRow* OutOptionalRow,
		FString& OutError)
	{
		// Current Recipe export baseline row입니다.
		FCFBatchExportRow Row;
		// P0-08J export diagnostics입니다.
		TArray<FString> Errors;
		if (!FCFBatchExportService::BuildRecipeNumericRow(*Fixture.Recipe, Fixture.TargetVehicleData, Row, Errors))
		{
			OutError = FString::Join(Errors, TEXT(" | "));
			return false;
		}
		// Canonical one-row Recipe export request입니다.
		FCFBatchExportRequest Request;
		Request.BatchExportId = ExportId;
		Request.DatasetKind = ECFBatchDatasetKind::RecipeNumericEdit;
		Request.Rows.Add(Row);
		if (!FCFBatchExportService::BuildExport(Request, OutArtifact, Errors))
		{
			OutError = FString::Join(Errors, TEXT(" | "));
			return false;
		}
		if (OutOptionalRow)
		{
			*OutOptionalRow = Row;
		}
		OutError.Reset();
		return true;
	}

	// One Profile row와 companion manifest를 P0-08J canonical exporter로 생성합니다.
	bool BuildProfileArtifact(
		UObject& ProfileObject,
		const ECFVehicleProfileDomain Domain,
		const FGuid& ExportId,
		FCFBatchExportArtifact& OutArtifact,
		FCFBatchExportRow* OutOptionalRow,
		FString& OutError)
	{
		// Current Profile export baseline row입니다.
		FCFBatchExportRow Row;
		// P0-08J export diagnostics입니다.
		TArray<FString> Errors;
		if (!FCFBatchExportService::BuildProfileNumericRow(ProfileObject, Domain, Row, Errors))
		{
			OutError = FString::Join(Errors, TEXT(" | "));
			return false;
		}
		// Canonical one-row Profile export request입니다.
		FCFBatchExportRequest Request;
		Request.BatchExportId = ExportId;
		Request.DatasetKind = ECFBatchDatasetKind::ProfileNumericEdit;
		Request.ProfileDomain = Domain;
		Request.Rows.Add(Row);
		if (!FCFBatchExportService::BuildExport(Request, OutArtifact, Errors))
		{
			OutError = FString::Join(Errors, TEXT(" | "));
			return false;
		}
		if (OutOptionalRow)
		{
			*OutOptionalRow = Row;
		}
		OutError.Reset();
		return true;
	}

	// P0 test CSV의 comma-safe simple rows에서 exact ColumnId cell 하나를 교체합니다.
	bool ReplaceSimpleCsvCell(
		const FString& CsvText,
		const FString& ColumnId,
		const FString& NewValue,
		FString& OutCsvText)
	{
		// Canonical exporter의 non-empty header/data lines입니다.
		TArray<FString> Lines;
		CsvText.ParseIntoArray(Lines, TEXT("\n"), true);
		if (Lines.Num() < 2)
		{
			return false;
		}
		// Simple test header cells입니다.
		TArray<FString> HeaderCells;
		Lines[0].ParseIntoArray(HeaderCells, TEXT(","), false);
		// Requested stable ColumnId index입니다.
		const int32 ColumnIndex = HeaderCells.Find(ColumnId);
		if (ColumnIndex == INDEX_NONE)
		{
			return false;
		}

		for (int32 LineIndex = 1; LineIndex < Lines.Num(); ++LineIndex)
		{
			// Simple test data cells입니다.
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

	// Canonical CSV의 physical data row 순서와 column 순서를 모두 뒤집어 semantic order-independence input을 만듭니다.
	bool ReverseSimpleCsvRowsAndColumns(const FString& CsvText, FString& OutCsvText)
	{
		// Canonical exporter의 non-empty physical lines입니다.
		TArray<FString> Lines;
		CsvText.ParseIntoArray(Lines, TEXT("\n"), true);
		if (Lines.Num() < 2)
		{
			return false;
		}

		for (FString& Line : Lines)
		{
			// Simple test row cells입니다.
			TArray<FString> Cells;
			Line.ParseIntoArray(Cells, TEXT(","), false);
			Algo::Reverse(Cells);
			Line = FString::Join(Cells, TEXT(","));
		}
		// Header를 제외한 physical data rows만 reverse합니다.
		TArray<FString> DataLines;
		for (int32 LineIndex = 1; LineIndex < Lines.Num(); ++LineIndex)
		{
			DataLines.Add(Lines[LineIndex]);
		}
		Algo::Reverse(DataLines);
		OutCsvText = Lines[0] + TEXT("\n");
		for (const FString& DataLine : DataLines)
		{
			OutCsvText += DataLine + TEXT("\n");
		}
		return true;
	}

	// One-row CSV의 data row를 한 번 더 복제해 duplicate identity input을 만듭니다.
	bool DuplicateSimpleCsvDataRow(const FString& CsvText, FString& OutCsvText)
	{
		// Header와 one data line입니다.
		TArray<FString> Lines;
		CsvText.ParseIntoArray(Lines, TEXT("\n"), true);
		if (Lines.Num() != 2)
		{
			return false;
		}
		OutCsvText = Lines[0] + TEXT("\n") + Lines[1] + TEXT("\n") + Lines[1] + TEXT("\n");
		return true;
	}

	// Session에서 exact RowId row preview를 찾습니다.
	const FCFBatchRowPreview* FindRow(const FCFBatchImportSession& Session, const FString& RowId)
	{
		return Session.Rows.FindByPredicate([&RowId](const FCFBatchRowPreview& Row)
		{
			return Row.RowId == RowId;
		});
	}

	// Row preview에서 exact stable ColumnId cell review를 찾습니다.
	const FCFBatchCellReview* FindCell(const FCFBatchRowPreview& Row, const FString& ColumnId)
	{
		return Row.CellReviews.FindByPredicate([&ColumnId](const FCFBatchCellReview& Cell)
		{
			return Cell.ColumnId == ColumnId;
		});
	}

	// Issue list가 requested stable code를 포함하는지 검사합니다.
	bool HasIssueCode(const TArray<FCFBatchIssue>& Issues, const ECFBatchIssueCode Code)
	{
		return Issues.ContainsByPredicate([Code](const FCFBatchIssue& Issue)
		{
			return Issue.Code == Code;
		});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchImportThreeWayTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchImport.ThreeWay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchImportGuardTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchImport.Guards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchImportOrderTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchImport.OrderDeterminism",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchRecipePreviewTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchImport.RecipePreviewMutationZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchProfilePreviewTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.BatchImport.ProfilePreviewMutationZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Export/Edited/Current의 A~D 분기와 ownership change conflict를 current Unreal reread로 검증합니다.
bool FCFBatchImportThreeWayTest::RunTest(const FString& Parameters)
{
	// Shared 3-way fixture입니다.
	CFBatchImportTestsPrivate::FRecipeFixture Fixture;
	// Fixture/export diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("3-way Recipe fixture builds"), CFBatchImportTestsPrivate::BuildRecipeFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	// Frozen export baseline artifact입니다.
	FCFBatchExportArtifact Artifact;
	// Frozen export row identity입니다.
	FCFBatchExportRow ExportRow;
	if (!TestTrue(TEXT("3-way Recipe export builds"), CFBatchImportTestsPrivate::BuildRecipeArtifact(
		Fixture,
		FGuid(0xA1000001, 0xA1000002, 0xA1000003, 0xA1000004),
		Artifact,
		&ExportRow,
		Error)))
	{
		AddError(Error);
		return false;
	}

	const FString AccelColumnId = TEXT("Recipe.DrivingFeel.AccelerationFeel");
	const FString BaseMassColumnId = TEXT("Recipe.Mass.ExplicitBaseMassKg");

	// A: Current == Baseline, Edited != Baseline인 Safe Candidate CSV입니다.
	FString SafeCsv;
	TestTrue(TEXT("Safe Candidate CSV edit succeeds"), CFBatchImportTestsPrivate::ReplaceSimpleCsvCell(Artifact.CanonicalCsvText, AccelColumnId, TEXT("0.75"), SafeCsv));
	// A branch transient preview Session입니다.
	FCFBatchImportSession SafeSession;
	FCFBatchImportRequest SafeRequest{SafeCsv, Artifact.ManifestJsonText};
	TestTrue(TEXT("Safe Candidate preview builds"), FCFBatchImportService::BuildPreview(SafeRequest, SafeSession));
	const FCFBatchRowPreview* SafeRow = CFBatchImportTestsPrivate::FindRow(SafeSession, ExportRow.RowId);
	TestNotNull(TEXT("Safe Candidate row exists"), SafeRow);
	if (SafeRow)
	{
		const FCFBatchCellReview* SafeCell = CFBatchImportTestsPrivate::FindCell(*SafeRow, AccelColumnId);
		TestNotNull(TEXT("Safe Candidate cell exists"), SafeCell);
		if (SafeCell)
		{
			TestEqual(TEXT("A branch classifies SafeCandidate"), SafeCell->MergeState, ECFBatchCellMergeState::SafeCandidate);
			TestEqual(TEXT("A branch preserves baseline"), SafeCell->ExportBaselineValue, FString(TEXT("0.25")));
			TestEqual(TEXT("A branch canonical edited value"), SafeCell->EditedCanonicalValue, FString(TEXT("0.75")));
		}
	}

	// B: Spreadsheet unchanged, Current Unreal changed after export입니다.
	Fixture.Recipe->DrivingFeelIntent.AccelerationFeel = 0.50f;
	FCFBatchImportSession NoSpreadsheetSession;
	FCFBatchImportRequest NoSpreadsheetRequest{Artifact.CanonicalCsvText, Artifact.ManifestJsonText};
	TestTrue(TEXT("No Spreadsheet Change preview builds"), FCFBatchImportService::BuildPreview(NoSpreadsheetRequest, NoSpreadsheetSession));
	const FCFBatchRowPreview* NoSpreadsheetRow = CFBatchImportTestsPrivate::FindRow(NoSpreadsheetSession, ExportRow.RowId);
	TestNotNull(TEXT("No Spreadsheet Change row exists"), NoSpreadsheetRow);
	if (NoSpreadsheetRow)
	{
		const FCFBatchCellReview* Cell = CFBatchImportTestsPrivate::FindCell(*NoSpreadsheetRow, AccelColumnId);
		TestNotNull(TEXT("No Spreadsheet Change cell exists"), Cell);
		if (Cell)
		{
			TestEqual(TEXT("B branch classifies NoSpreadsheetChange"), Cell->MergeState, ECFBatchCellMergeState::NoSpreadsheetChange);
			TestEqual(TEXT("B branch current Unreal reread"), Cell->CurrentUnrealValue, FString(TEXT("0.5")));
		}
		TestTrue(TEXT("Fingerprint mismatch is recorded as signal"), NoSpreadsheetRow->bFingerprintChangedSinceExport);
		TestFalse(TEXT("Fingerprint mismatch alone does not file-block"), NoSpreadsheetSession.bFileBlocked);
	}

	// C: Spreadsheet와 Current가 같은 new value로 수렴한 CSV입니다.
	FString ConvergedCsv;
	TestTrue(TEXT("Converged CSV edit succeeds"), CFBatchImportTestsPrivate::ReplaceSimpleCsvCell(Artifact.CanonicalCsvText, AccelColumnId, TEXT("0.5"), ConvergedCsv));
	FCFBatchImportSession ConvergedSession;
	FCFBatchImportRequest ConvergedRequest{ConvergedCsv, Artifact.ManifestJsonText};
	TestTrue(TEXT("Converged preview builds"), FCFBatchImportService::BuildPreview(ConvergedRequest, ConvergedSession));
	const FCFBatchRowPreview* ConvergedRow = CFBatchImportTestsPrivate::FindRow(ConvergedSession, ExportRow.RowId);
	TestNotNull(TEXT("Converged row exists"), ConvergedRow);
	if (ConvergedRow)
	{
		const FCFBatchCellReview* Cell = CFBatchImportTestsPrivate::FindCell(*ConvergedRow, AccelColumnId);
		TestNotNull(TEXT("Converged cell exists"), Cell);
		if (Cell)
		{
			TestEqual(TEXT("C branch classifies ConvergedNoChange"), Cell->MergeState, ECFBatchCellMergeState::ConvergedNoChange);
		}
	}

	// D: Spreadsheet와 Current가 baseline에서 서로 다른 값으로 갈라진 conflict입니다.
	FString ConflictCsv;
	TestTrue(TEXT("Concurrent conflict CSV edit succeeds"), CFBatchImportTestsPrivate::ReplaceSimpleCsvCell(Artifact.CanonicalCsvText, AccelColumnId, TEXT("0.75"), ConflictCsv));
	FCFBatchImportSession ConflictSession;
	FCFBatchImportRequest ConflictRequest{ConflictCsv, Artifact.ManifestJsonText};
	TestTrue(TEXT("Concurrent conflict preview still returns review Session"), FCFBatchImportService::BuildPreview(ConflictRequest, ConflictSession));
	const FCFBatchRowPreview* ConflictRow = CFBatchImportTestsPrivate::FindRow(ConflictSession, ExportRow.RowId);
	TestNotNull(TEXT("Concurrent conflict row exists"), ConflictRow);
	if (ConflictRow)
	{
		const FCFBatchCellReview* Cell = CFBatchImportTestsPrivate::FindCell(*ConflictRow, AccelColumnId);
		TestNotNull(TEXT("Concurrent conflict cell exists"), Cell);
		if (Cell)
		{
			TestEqual(TEXT("D branch classifies ConcurrentEditConflict"), Cell->MergeState, ECFBatchCellMergeState::ConcurrentEditConflict);
		}
		TestTrue(TEXT("Concurrent conflict emits issue"), CFBatchImportTestsPrivate::HasIssueCode(ConflictRow->Issues, ECFBatchIssueCode::ConcurrentEditConflict));
		TestEqual(TEXT("Concurrent conflict row status"), ConflictRow->Status, ECFBatchRowStatus::Conflict);
	}
	TestTrue(TEXT("Concurrent conflict blocks file plan"), ConflictSession.bFileBlocked);

	// Ownership change: export 당시 ExplicitValue였던 BaseMass가 Current에서 UseProfile로 바뀐 뒤 Spreadsheet edit됩니다.
	Fixture.Recipe->DrivingFeelIntent.AccelerationFeel = 0.25f;
	Fixture.Recipe->MassIntent.BaseMassMode = ECFAuthoringInputMode::UseProfile;
	FString OwnershipCsv;
	TestTrue(TEXT("Ownership change CSV edit succeeds"), CFBatchImportTestsPrivate::ReplaceSimpleCsvCell(Artifact.CanonicalCsvText, BaseMassColumnId, TEXT("1300"), OwnershipCsv));
	FCFBatchImportSession OwnershipSession;
	FCFBatchImportRequest OwnershipRequest{OwnershipCsv, Artifact.ManifestJsonText};
	TestTrue(TEXT("Ownership change preview returns review Session"), FCFBatchImportService::BuildPreview(OwnershipRequest, OwnershipSession));
	const FCFBatchRowPreview* OwnershipRow = CFBatchImportTestsPrivate::FindRow(OwnershipSession, ExportRow.RowId);
	TestNotNull(TEXT("Ownership change row exists"), OwnershipRow);
	if (OwnershipRow)
	{
		const FCFBatchCellReview* Cell = CFBatchImportTestsPrivate::FindCell(*OwnershipRow, BaseMassColumnId);
		TestNotNull(TEXT("Ownership change cell exists"), Cell);
		if (Cell)
		{
			TestEqual(TEXT("Ownership change classifies explicit conflict"), Cell->MergeState, ECFBatchCellMergeState::OwnershipChangedSinceExport);
			TestEqual(TEXT("Current ownership source mode is reread"), Cell->CurrentOwnershipSourceMode, FString(TEXT("UseProfile")));
			TestFalse(TEXT("Current source mode makes cell non-editable"), Cell->bEditableNow);
		}
		TestTrue(TEXT("Ownership change emits stable issue"), CFBatchImportTestsPrivate::HasIssueCode(OwnershipRow->Issues, ECFBatchIssueCode::OwnershipChangedSinceExport));
	}
	return true;
}

// Manifest tamper, reserved/read-only modification과 duplicate Recipe/Row identity를 fail-closed하는지 검증합니다.
bool FCFBatchImportGuardTest::RunTest(const FString& Parameters)
{
	// Guard fixture입니다.
	CFBatchImportTestsPrivate::FRecipeFixture Fixture;
	// Fixture/export diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Guard Recipe fixture builds"), CFBatchImportTestsPrivate::BuildRecipeFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	// Guard baseline artifact입니다.
	FCFBatchExportArtifact Artifact;
	FCFBatchExportRow ExportRow;
	if (!TestTrue(TEXT("Guard export builds"), CFBatchImportTestsPrivate::BuildRecipeArtifact(
		Fixture,
		FGuid(0xB1000001, 0xB1000002, 0xB1000003, 0xB1000004),
		Artifact,
		&ExportRow,
		Error)))
	{
		AddError(Error);
		return false;
	}

	// Companion manifest ExportSetHash를 손상시킨 untrusted JSON입니다.
	FString TamperedManifestJson = Artifact.ManifestJsonText;
	TamperedManifestJson.ReplaceInline(*Artifact.Manifest.ExportSetHash, TEXT("00000000000000000000000000000000"), ESearchCase::CaseSensitive);
	FCFBatchManifest ParsedManifest;
	TArray<FCFBatchIssue> ManifestIssues;
	TestFalse(TEXT("Tampered manifest is rejected before preview"), FCFBatchImportService::ParseAndValidateManifest(TamperedManifestJson, ParsedManifest, ManifestIssues));
	TestTrue(TEXT("Tampered manifest emits ManifestInvalid"), CFBatchImportTestsPrivate::HasIssueCode(ManifestIssues, ECFBatchIssueCode::ManifestInvalid));

	// Reserved schema revision cell만 export baseline과 다르게 바꿉니다.
	FString ReadOnlyCsv;
	TestTrue(TEXT("Reserved CSV edit succeeds"), CFBatchImportTestsPrivate::ReplaceSimpleCsvCell(Artifact.CanonicalCsvText, TEXT("__cf_schema_revision"), TEXT("999"), ReadOnlyCsv));
	FCFBatchImportSession ReadOnlySession;
	FCFBatchImportRequest ReadOnlyRequest{ReadOnlyCsv, Artifact.ManifestJsonText};
	TestTrue(TEXT("Read-only modified file returns review Session"), FCFBatchImportService::BuildPreview(ReadOnlyRequest, ReadOnlySession));
	const FCFBatchRowPreview* ReadOnlyRow = CFBatchImportTestsPrivate::FindRow(ReadOnlySession, ExportRow.RowId);
	TestNotNull(TEXT("Read-only modified row exists"), ReadOnlyRow);
	if (ReadOnlyRow)
	{
		TestTrue(TEXT("Read-only modification emits stable issue"), CFBatchImportTestsPrivate::HasIssueCode(ReadOnlyRow->Issues, ECFBatchIssueCode::ReadOnlyColumnModified));
		TestEqual(TEXT("Read-only modification blocks row"), ReadOnlyRow->Status, ECFBatchRowStatus::Blocked);
	}
	TestTrue(TEXT("Read-only modification blocks file plan"), ReadOnlySession.bFileBlocked);

	// Exact same data row를 복제해 duplicate RowId + Recipe identity를 만듭니다.
	FString DuplicateCsv;
	TestTrue(TEXT("Duplicate row CSV builds"), CFBatchImportTestsPrivate::DuplicateSimpleCsvDataRow(Artifact.CanonicalCsvText, DuplicateCsv));
	FCFBatchImportSession DuplicateSession;
	FCFBatchImportRequest DuplicateRequest{DuplicateCsv, Artifact.ManifestJsonText};
	TestTrue(TEXT("Duplicate row returns review Session"), FCFBatchImportService::BuildPreview(DuplicateRequest, DuplicateSession));
	TestTrue(TEXT("Duplicate row blocks file plan"), DuplicateSession.bFileBlocked);
	TestTrue(TEXT("Duplicate row emits stable issue"), CFBatchImportTestsPrivate::HasIssueCode(DuplicateSession.Issues, ECFBatchIssueCode::DuplicateRowIdentity));

	// Editable numeric formula는 plain numeric contract 위반입니다.
	FString FormulaCsv;
	TestTrue(TEXT("Formula CSV edit succeeds"), CFBatchImportTestsPrivate::ReplaceSimpleCsvCell(Artifact.CanonicalCsvText, TEXT("Recipe.DrivingFeel.AccelerationFeel"), TEXT("=1+1"), FormulaCsv));
	FCFBatchImportSession FormulaSession;
	FCFBatchImportRequest FormulaRequest{FormulaCsv, Artifact.ManifestJsonText};
	TestTrue(TEXT("Formula input returns review Session"), FCFBatchImportService::BuildPreview(FormulaRequest, FormulaSession));
	const FCFBatchRowPreview* FormulaRow = CFBatchImportTestsPrivate::FindRow(FormulaSession, ExportRow.RowId);
	TestNotNull(TEXT("Formula row exists"), FormulaRow);
	if (FormulaRow)
	{
		TestTrue(TEXT("Formula is InvalidNumericValue"), CFBatchImportTestsPrivate::HasIssueCode(FormulaRow->Issues, ECFBatchIssueCode::InvalidNumericValue));
	}
	return true;
}

// Physical CSV row order와 header column order를 모두 바꿔도 같은 semantic BatchPlanHash가 나오는지 검증합니다.
bool FCFBatchImportOrderTest::RunTest(const FString& Parameters)
{
	// First Recipe fixture입니다.
	CFBatchImportTestsPrivate::FRecipeFixture FirstFixture;
	// Second Recipe fixture입니다.
	CFBatchImportTestsPrivate::FRecipeFixture SecondFixture;
	// Fixture/export diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Order first fixture builds"), CFBatchImportTestsPrivate::BuildRecipeFixture(FirstFixture, Error))
		|| !TestTrue(TEXT("Order second fixture builds"), CFBatchImportTestsPrivate::BuildRecipeFixture(SecondFixture, Error)))
	{
		AddError(Error);
		return false;
	}
	SecondFixture.Recipe->DrivingFeelIntent.AccelerationFeel = 0.35f;

	// First current export row입니다.
	FCFBatchExportRow FirstRow;
	// Second current export row입니다.
	FCFBatchExportRow SecondRow;
	// P0-08J row projection diagnostics입니다.
	TArray<FString> ExportErrors;
	TestTrue(TEXT("Order first row builds"), FCFBatchExportService::BuildRecipeNumericRow(*FirstFixture.Recipe, FirstFixture.TargetVehicleData, FirstRow, ExportErrors));
	TestTrue(TEXT("Order second row builds"), FCFBatchExportService::BuildRecipeNumericRow(*SecondFixture.Recipe, SecondFixture.TargetVehicleData, SecondRow, ExportErrors));

	// Same semantic two-row canonical export입니다.
	FCFBatchExportRequest ExportRequest;
	ExportRequest.BatchExportId = FGuid(0xC1000001, 0xC1000002, 0xC1000003, 0xC1000004);
	ExportRequest.DatasetKind = ECFBatchDatasetKind::RecipeNumericEdit;
	ExportRequest.Rows = {SecondRow, FirstRow};
	// Canonical artifact baseline입니다.
	FCFBatchExportArtifact Artifact;
	if (!TestTrue(TEXT("Order canonical export builds"), FCFBatchExportService::BuildExport(ExportRequest, Artifact, ExportErrors)))
	{
		for (const FString& ExportError : ExportErrors)
		{
			AddError(ExportError);
		}
		return false;
	}

	// Canonical physical CSV preview입니다.
	FCFBatchImportSession CanonicalSession;
	FCFBatchImportRequest CanonicalRequest{Artifact.CanonicalCsvText, Artifact.ManifestJsonText};
	TestTrue(TEXT("Canonical order preview builds"), FCFBatchImportService::BuildPreview(CanonicalRequest, CanonicalSession));
	TestFalse(TEXT("Canonical order file is not blocked"), CanonicalSession.bFileBlocked);

	// Same cells지만 physical row/column 순서를 모두 뒤집은 CSV입니다.
	FString ReorderedCsv;
	TestTrue(TEXT("Reordered CSV builds"), CFBatchImportTestsPrivate::ReverseSimpleCsvRowsAndColumns(Artifact.CanonicalCsvText, ReorderedCsv));
	FCFBatchImportSession ReorderedSession;
	FCFBatchImportRequest ReorderedRequest{ReorderedCsv, Artifact.ManifestJsonText};
	TestTrue(TEXT("Reordered physical CSV preview builds"), FCFBatchImportService::BuildPreview(ReorderedRequest, ReorderedSession));
	TestFalse(TEXT("Reordered physical CSV is not blocked"), ReorderedSession.bFileBlocked);
	TestEqual(TEXT("Row/column physical order has semantic zero effect on BatchPlanHash"), ReorderedSession.BatchPlanHash, CanonicalSession.BatchPlanHash);
	TestEqual(TEXT("Canonicalized parsed row count matches"), ReorderedSession.Rows.Num(), CanonicalSession.Rows.Num());
	return true;
}

// Safe Recipe numeric candidate가 transient duplicate + existing Resolver preview를 만들고 persistent Recipe/Target을 수정하지 않는지 검증합니다.
bool FCFBatchRecipePreviewTest::RunTest(const FString& Parameters)
{
	// Recipe prospective fixture입니다.
	CFBatchImportTestsPrivate::FRecipeFixture Fixture;
	// Snapshot/export diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Recipe prospective fixture builds"), CFBatchImportTestsPrivate::BuildRecipeFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	// Frozen Recipe export baseline입니다.
	FCFBatchExportArtifact Artifact;
	FCFBatchExportRow ExportRow;
	if (!TestTrue(TEXT("Recipe prospective export builds"), CFBatchImportTestsPrivate::BuildRecipeArtifact(
		Fixture,
		FGuid(0xD1000001, 0xD1000002, 0xD1000003, 0xD1000004),
		Artifact,
		&ExportRow,
		Error)))
	{
		AddError(Error);
		return false;
	}
	// Safe candidate Spreadsheet edit입니다.
	FString EditedCsv;
	TestTrue(TEXT("Recipe prospective CSV edit succeeds"), CFBatchImportTestsPrivate::ReplaceSimpleCsvCell(Artifact.CanonicalCsvText, TEXT("Recipe.DrivingFeel.AccelerationFeel"), TEXT("0.8"), EditedCsv));

	// Preview 전 persistent Recipe authored value입니다.
	const float RecipeValueBefore = Fixture.Recipe->DrivingFeelIntent.AccelerationFeel;
	// Preview 전 Recipe diagnostic revision입니다.
	const int32 RecipeRevisionBefore = Fixture.Recipe->AuthoringRevision;
	// Preview 전 Recipe fingerprint입니다.
	const FString RecipeFingerprintBefore = CFBatchImportTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error);
	// Preview 전 Target full Definition hash입니다.
	const FString TargetHashBefore = CFBatchImportTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	// Preview 전 package dirty states입니다.
	const bool bRecipeDirtyBefore = Fixture.RecipePackage->IsDirty();
	const bool bTargetDirtyBefore = Fixture.TargetPackage->IsDirty();

	// Transient Recipe prospective preview Session입니다.
	FCFBatchImportSession Session;
	FCFBatchImportRequest Request{EditedCsv, Artifact.ManifestJsonText};
	TestTrue(TEXT("Recipe prospective preview builds"), FCFBatchImportService::BuildPreview(Request, Session));
	const FCFBatchRowPreview* Row = CFBatchImportTestsPrivate::FindRow(Session, ExportRow.RowId);
	TestNotNull(TEXT("Recipe prospective row exists"), Row);
	if (Row)
	{
		const FCFBatchCellReview* Cell = CFBatchImportTestsPrivate::FindCell(*Row, TEXT("Recipe.DrivingFeel.AccelerationFeel"));
		TestNotNull(TEXT("Recipe prospective SafeCandidate cell exists"), Cell);
		if (Cell)
		{
			TestEqual(TEXT("Recipe prospective input is SafeCandidate"), Cell->MergeState, ECFBatchCellMergeState::SafeCandidate);
		}
		TestTrue(TEXT("Recipe prospective reuses Resolver and emits Vehicle preview"), Row->VehiclePreviews.Num() > 0);
	}
	TestFalse(TEXT("BatchPlanHash is non-empty"), Session.BatchPlanHash.IsEmpty());

	TestEqual(TEXT("Persistent Recipe numeric value unchanged"), Fixture.Recipe->DrivingFeelIntent.AccelerationFeel, RecipeValueBefore);
	TestEqual(TEXT("Persistent Recipe revision unchanged"), Fixture.Recipe->AuthoringRevision, RecipeRevisionBefore);
	TestEqual(TEXT("Persistent Recipe fingerprint unchanged"), CFBatchImportTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error), RecipeFingerprintBefore);
	TestEqual(TEXT("Persistent Target hash unchanged"), CFBatchImportTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestEqual(TEXT("Recipe package dirty state unchanged"), Fixture.RecipePackage->IsDirty(), bRecipeDirtyBefore);
	TestEqual(TEXT("Target package dirty state unchanged"), Fixture.TargetPackage->IsDirty(), bTargetDirtyBefore);
	return true;
}

// Shared Profile candidate가 affected Recipe/Vehicle에 prospective Resolver preview를 fan-out하고 Profile/Recipe/Target persistent truth를 수정하지 않는지 검증합니다.
bool FCFBatchProfilePreviewTest::RunTest(const FString& Parameters)
{
	// Affected Recipe/Target fixture입니다.
	CFBatchImportTestsPrivate::FRecipeFixture Fixture;
	// Fixture/snapshot diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Profile prospective Recipe fixture builds"), CFBatchImportTestsPrivate::BuildRecipeFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	// Shared Handling Profile package입니다.
	UPackage* ProfilePackage = CFBatchImportTestsPrivate::CreateTestPackage(TEXT("CFBatchImportProfile"));
	// Current shared Handling Profile source입니다.
	UCFHandlingProfile* HandlingProfile = NewObject<UCFHandlingProfile>(ProfilePackage, TEXT("DA_BatchImportHandling_Test"), RF_Transactional);
	if (!TestNotNull(TEXT("Handling Profile fixture creates"), HandlingProfile))
	{
		return false;
	}
	HandlingProfile->Data.FrontWheelMaxBrakeTorque = 1500.0f;
	Fixture.Recipe->ProfileBindings.HandlingProfile = HandlingProfile;
	ProfilePackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);
	Fixture.TargetPackage->SetDirtyFlag(false);

	// Frozen Profile export baseline입니다.
	FCFBatchExportArtifact Artifact;
	FCFBatchExportRow ExportRow;
	if (!TestTrue(TEXT("Profile prospective export builds"), CFBatchImportTestsPrivate::BuildProfileArtifact(
		*HandlingProfile,
		ECFVehicleProfileDomain::Handling,
		FGuid(0xE1000001, 0xE1000002, 0xE1000003, 0xE1000004),
		Artifact,
		&ExportRow,
		Error)))
	{
		AddError(Error);
		return false;
	}
	// Safe shared Profile numeric Spreadsheet edit입니다.
	const FString BrakeColumnId = TEXT("Profile.Handling.FrontWheelMaxBrakeTorque");
	FString EditedCsv;
	TestTrue(TEXT("Profile prospective CSV edit succeeds"), CFBatchImportTestsPrivate::ReplaceSimpleCsvCell(Artifact.CanonicalCsvText, BrakeColumnId, TEXT("2000"), EditedCsv));

	// Preview 전 persistent Profile value입니다.
	const float ProfileValueBefore = HandlingProfile->Data.FrontWheelMaxBrakeTorque;
	// Preview 전 Profile diagnostic revision입니다.
	const int32 ProfileRevisionBefore = HandlingProfile->Meta.AuthoringRevision;
	// Preview 전 Recipe fingerprint입니다.
	const FString RecipeFingerprintBefore = CFBatchImportTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error);
	// Preview 전 Target hash입니다.
	const FString TargetHashBefore = CFBatchImportTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	// Preview 전 current Profile export fingerprint입니다.
	FCFBatchExportRow ProfileRowBefore;
	TArray<FString> ProfileErrors;
	TestTrue(TEXT("Profile fingerprint baseline row builds"), FCFBatchExportService::BuildProfileNumericRow(*HandlingProfile, ECFVehicleProfileDomain::Handling, ProfileRowBefore, ProfileErrors));
	const FString ProfileFingerprintBefore = ProfileRowBefore.ObjectFingerprint;
	// Preview 전 package dirty states입니다.
	const bool bProfileDirtyBefore = ProfilePackage->IsDirty();
	const bool bRecipeDirtyBefore = Fixture.RecipePackage->IsDirty();
	const bool bTargetDirtyBefore = Fixture.TargetPackage->IsDirty();

	// Transient shared Profile prospective preview Session입니다.
	FCFBatchImportSession Session;
	FCFBatchImportRequest Request{EditedCsv, Artifact.ManifestJsonText};
	TestTrue(TEXT("Profile prospective preview builds"), FCFBatchImportService::BuildPreview(Request, Session));
	const FCFBatchRowPreview* Row = CFBatchImportTestsPrivate::FindRow(Session, ExportRow.RowId);
	TestNotNull(TEXT("Profile prospective row exists"), Row);
	if (Row)
	{
		const FCFBatchCellReview* Cell = CFBatchImportTestsPrivate::FindCell(*Row, BrakeColumnId);
		TestNotNull(TEXT("Profile prospective SafeCandidate cell exists"), Cell);
		if (Cell)
		{
			TestEqual(TEXT("Profile prospective input is SafeCandidate"), Cell->MergeState, ECFBatchCellMergeState::SafeCandidate);
		}
		TestTrue(TEXT("Shared Profile finds affected Recipe/Vehicle preview"), Row->VehiclePreviews.Num() > 0);
		TestTrue(TEXT("Affected preview contains exact Recipe identity"), Row->VehiclePreviews.ContainsByPredicate([&Fixture](const FCFBatchVehiclePreview& Preview)
		{
			return Preview.RecipePath == FSoftObjectPath(Fixture.Recipe).ToString();
		}));
	}
	TestTrue(TEXT("Summary counts affected shared Profile"), Session.Summary.AffectedProfileCount >= 1);
	TestTrue(TEXT("Summary counts affected Recipe"), Session.Summary.AffectedRecipeCount >= 1);

	// Preview 후 current Profile row를 다시 읽습니다.
	FCFBatchExportRow ProfileRowAfter;
	TestTrue(TEXT("Profile fingerprint readback row builds"), FCFBatchExportService::BuildProfileNumericRow(*HandlingProfile, ECFVehicleProfileDomain::Handling, ProfileRowAfter, ProfileErrors));
	TestEqual(TEXT("Persistent Profile numeric unchanged"), HandlingProfile->Data.FrontWheelMaxBrakeTorque, ProfileValueBefore);
	TestEqual(TEXT("Persistent Profile revision unchanged"), HandlingProfile->Meta.AuthoringRevision, ProfileRevisionBefore);
	TestEqual(TEXT("Persistent Profile fingerprint unchanged"), ProfileRowAfter.ObjectFingerprint, ProfileFingerprintBefore);
	TestEqual(TEXT("Persistent Recipe fingerprint unchanged by Profile preview"), CFBatchImportTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error), RecipeFingerprintBefore);
	TestEqual(TEXT("Persistent Target hash unchanged by Profile preview"), CFBatchImportTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestEqual(TEXT("Profile package dirty state unchanged"), ProfilePackage->IsDirty(), bProfileDirtyBefore);
	TestEqual(TEXT("Recipe package dirty state unchanged by Profile preview"), Fixture.RecipePackage->IsDirty(), bRecipeDirtyBefore);
	TestEqual(TEXT("Target package dirty state unchanged by Profile preview"), Fixture.TargetPackage->IsDirty(), bTargetDirtyBefore);
	return true;
}

#endif
