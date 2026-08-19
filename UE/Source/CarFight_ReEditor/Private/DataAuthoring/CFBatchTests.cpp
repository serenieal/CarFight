// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchTests.cpp
// Version: v1.1.0
// Date: 2026-08-18
// Description: DAUTH-P0-08J Batch Column Registry / Canonical Export + current 118-field/79-profile-numeric compatibility Automation입니다.
// Changelog:
// - v1.1.0: UI-P0-06 explicit RedlineStartRPM으로 Current Resolved Registry 118, Performance numeric 18, five-profile numeric total 79 계약을 반영.
// - v1.0.2: TArray 전체 TestEqual 문자열화 의존을 제거하고 exact array equality를 TestTrue로 검증.
// - v1.0.1: UE 5.8 TArray에 없는 CountByPredicate test helper를 explicit loop로 교정.
// - v1.0.0: ColumnId stability, 당시 allowlist/117 projection, reserved collision, canonical export/manifest/hash, mutation0 검증 추가.
// Migration:
// - /Temp package만 사용하며 CSV/manifest disk write, Import, source commit, Definition Apply를 수행하지 않습니다.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CFVehicleData.h"
#include "DataAuthoring/CFBatchColumnRegistry.h"
#include "DataAuthoring/CFBatchExport.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFVehicleFieldRegistry.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace CFBatchTestsPrivate
{
	/** Recipe numeric export에 필요한 최소 in-memory truth fixture입니다. */
	struct FRecipeFixture
	{
		// Target UCFVehicleData를 소유하는 저장하지 않는 /Temp package입니다.
		UPackage* TargetPackage = nullptr;

		// Recipe를 소유하는 저장하지 않는 /Temp package입니다.
		UPackage* RecipePackage = nullptr;

		// Current Target Definition fixture입니다.
		UCFVehicleData* TargetVehicleData = nullptr;

		// Recipe Numeric Edit fixture입니다.
		UCFVehicleRecipeData* Recipe = nullptr;
	};

	// Automation object를 격리할 unique /Temp package를 만듭니다.
	UPackage* CreateTestPackage(const TCHAR* Prefix)
	{
		// 다른 Automation run과 충돌하지 않는 unique package name입니다.
		const FString PackageName = FString::Printf(TEXT("/Temp/%s_%s"), Prefix, *FGuid::NewGuid().ToString(EGuidFormats::Digits));
		return CreatePackage(*PackageName);
	}

	// Current Batch tests에 필요한 Recipe/Target numeric fixture를 구성합니다.
	bool BuildRecipeFixture(FRecipeFixture& OutFixture, FString& OutError)
	{
		OutFixture = FRecipeFixture();
		OutFixture.TargetPackage = CreateTestPackage(TEXT("CFBatchTarget"));
		OutFixture.RecipePackage = CreateTestPackage(TEXT("CFBatchRecipe"));
		if (!OutFixture.TargetPackage || !OutFixture.RecipePackage)
		{
			OutError = TEXT("Batch /Temp package를 생성하지 못했습니다.");
			return false;
		}

		OutFixture.TargetVehicleData = NewObject<UCFVehicleData>(OutFixture.TargetPackage, TEXT("DA_BatchTarget_Test"), RF_Transactional);
		OutFixture.Recipe = NewObject<UCFVehicleRecipeData>(OutFixture.RecipePackage, TEXT("DA_BatchRecipe_Test"), RF_Transactional);
		if (!OutFixture.TargetVehicleData || !OutFixture.Recipe)
		{
			OutError = TEXT("Batch Recipe/Target UObject fixture를 생성하지 못했습니다.");
			return false;
		}
		OutFixture.Recipe->TargetVehicleData = OutFixture.TargetVehicleData;

		OutFixture.Recipe->DrivingFeelIntent.AccelerationFeel = 0.25f;
		OutFixture.Recipe->DrivingFeelIntent.SteeringAgility = 0.5f;
		OutFixture.Recipe->DrivingFeelIntent.GripFeel = 0.75f;
		OutFixture.Recipe->DrivingFeelIntent.SuspensionFirmness = 1.0f;
		OutFixture.Recipe->MassIntent.BaseMassMode = ECFAuthoringInputMode::ExplicitValue;
		OutFixture.Recipe->MassIntent.ExplicitBaseMassKg = 1234.5f;
		OutFixture.Recipe->MassIntent.GrossMassMode = ECFAuthoringInputMode::UseProfile;
		OutFixture.Recipe->MassIntent.ExplicitGrossMassKg = 9876.5f;
		OutFixture.Recipe->DurabilityIntent.MaxHealthMode = ECFAuthoringInputMode::ExplicitValue;
		OutFixture.Recipe->DurabilityIntent.ExplicitMaxHealth = 321.25f;

		OutFixture.TargetPackage->SetDirtyFlag(false);
		OutFixture.RecipePackage->SetDirtyFlag(false);
		OutError.Reset();
		return true;
	}

	// Persistent Recipe semantic fingerprint를 existing SnapshotBuilder로 읽습니다.
	FString BuildRecipeFingerprint(const UCFVehicleRecipeData& Recipe, FString& OutError)
	{
		// Existing Recipe Snapshot/fingerprint authority입니다.
		FCFVehicleRecipeSnapshot Snapshot;
		if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(Recipe, Snapshot, OutError))
		{
			return FString();
		}
		return Snapshot.RecipeFingerprint;
	}

	// Persistent Target full Definition hash를 existing SnapshotBuilder로 읽습니다.
	FString BuildTargetHash(const UCFVehicleData& Target, FString& OutError)
	{
		// Existing full Definition Snapshot/hash authority입니다.
		FCFVehicleDefinitionSnapshot Snapshot;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(Target, Snapshot, OutError))
		{
			return FString();
		}
		return Snapshot.DefinitionHash;
	}

		// Reserved metadata를 제외한 dataset data column 수를 반환합니다.
	int32 CountDataColumns(const TArray<FCFBatchColumnDescriptor>& Columns)
	{
		// Non-reserved data column 누적 수입니다.
		int32 DataColumnCount = 0;
		for (const FCFBatchColumnDescriptor& Descriptor : Columns)
		{
			if (!Descriptor.bReservedMetadata)
			{
				++DataColumnCount;
			}
		}
		return DataColumnCount;
	}

	// Stable ColumnId 배열만 projection해 두 Registry 호출을 비교합니다.
	TArray<FString> ExtractColumnIds(const TArray<FCFBatchColumnDescriptor>& Columns)
	{
		// Descriptor order를 그대로 보존할 ColumnId 결과입니다.
		TArray<FString> ColumnIds;
		ColumnIds.Reserve(Columns.Num());
		for (const FCFBatchColumnDescriptor& Descriptor : Columns)
		{
			ColumnIds.Add(Descriptor.ColumnId);
		}
		return ColumnIds;
	}

	// UTF-8 bytes가 BOM으로 시작하는지 검사합니다.
	bool HasUtf8Bom(const TArray<uint8>& Bytes)
	{
		return Bytes.Num() >= 3 && Bytes[0] == 0xEF && Bytes[1] == 0xBB && Bytes[2] == 0xBF;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchColumnIdTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Batch.ColumnIdStability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchAllowlistTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Batch.AllowlistProjection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchReservedTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Batch.ReservedCollision",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchCanonicalExportTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Batch.CanonicalExport",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFBatchManifestMutationTest,
	"CarFight.DataAuthoring.DAUTH_P0_08.Batch.ManifestMutationZero",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Stable ColumnId가 호출 순서/locale label과 독립적이고 Resolved report가 current 118 Registry를 1:1 projection하는지 검증합니다.
bool FCFBatchColumnIdTest::RunTest(const FString& Parameters)
{
	// Registry validation diagnostics입니다.
	TArray<FString> Errors;
	// First Recipe schema projection입니다.
	TArray<FCFBatchColumnDescriptor> FirstRecipeColumns;
	// Second Recipe schema projection입니다.
	TArray<FCFBatchColumnDescriptor> SecondRecipeColumns;
	TestTrue(TEXT("Recipe Column Registry first projection succeeds"), FCFBatchColumnRegistry::GetDatasetColumns(ECFBatchDatasetKind::RecipeNumericEdit, ECFVehicleProfileDomain::None, FirstRecipeColumns, Errors));
	TestTrue(TEXT("Recipe Column Registry second projection succeeds"), FCFBatchColumnRegistry::GetDatasetColumns(ECFBatchDatasetKind::RecipeNumericEdit, ECFVehicleProfileDomain::None, SecondRecipeColumns, Errors));
		// 두 projection에서 추출한 stable technical ColumnId 배열입니다.
	const TArray<FString> FirstColumnIds = CFBatchTestsPrivate::ExtractColumnIds(FirstRecipeColumns);
	// 두 번째 projection의 stable technical ColumnId 배열입니다.
	const TArray<FString> SecondColumnIds = CFBatchTestsPrivate::ExtractColumnIds(SecondRecipeColumns);
	TestTrue(TEXT("Recipe stable ColumnId order repeats exactly"), FirstColumnIds == SecondColumnIds);
	TestEqual(TEXT("Recipe numeric non-reserved column count is frozen 7"), CFBatchTestsPrivate::CountDataColumns(FirstRecipeColumns), 7);

	// Exact Recipe numeric technical identities입니다.
	const TArray<FString> ExpectedRecipeNumericIds =
	{
		TEXT("Recipe.DrivingFeel.AccelerationFeel"),
		TEXT("Recipe.DrivingFeel.GripFeel"),
		TEXT("Recipe.DrivingFeel.SteeringAgility"),
		TEXT("Recipe.DrivingFeel.SuspensionFirmness"),
		TEXT("Recipe.Durability.ExplicitMaxHealth"),
		TEXT("Recipe.Mass.ExplicitBaseMassKg"),
		TEXT("Recipe.Mass.ExplicitGrossMassKg")
	};
	for (const FString& ExpectedId : ExpectedRecipeNumericIds)
	{
		TestTrue(*FString::Printf(TEXT("Recipe stable ColumnId exists: %s"), *ExpectedId), FirstRecipeColumns.ContainsByPredicate([&ExpectedId](const FCFBatchColumnDescriptor& Descriptor)
		{
			return Descriptor.ColumnId == ExpectedId;
		}));
	}

		// Current Field Registry projection입니다.
	TArray<FCFBatchResolvedProjection> ResolvedProjections;
	TestTrue(TEXT("Resolved current Registry projection succeeds"), FCFBatchColumnRegistry::GetResolvedFieldProjections(ResolvedProjections, Errors));
	TestEqual(TEXT("Resolved projection count equals current Registry"), ResolvedProjections.Num(), FCFVehicleFieldRegistry::ExpectedLeafPatternCount);

	// Existing Field Registry canonical pattern identity set입니다.
	TSet<FString> ExistingRegistryPatterns;
	for (const FCFVehicleFieldDescriptor& Descriptor : FCFVehicleFieldRegistry::GetDescriptors())
	{
		ExistingRegistryPatterns.Add(Descriptor.GetCanonicalPattern());
	}
	for (const FCFBatchResolvedProjection& Projection : ResolvedProjections)
	{
		TestTrue(*FString::Printf(TEXT("Resolved projection comes from existing Field Registry: %s"), *Projection.StableFieldPattern), ExistingRegistryPatterns.Contains(Projection.StableFieldPattern));
	}
	return true;
}

// Recipe 7개와 5 Profile typed numeric leaf만 editable이고 enum/bool/asset/array가 제외되는지 검증합니다.
bool FCFBatchAllowlistTest::RunTest(const FString& Parameters)
{
	// Registry diagnostics입니다.
	TArray<FString> Errors;
	// Recipe numeric dataset columns입니다.
	TArray<FCFBatchColumnDescriptor> RecipeColumns;
	if (!TestTrue(TEXT("Recipe allowlist projection succeeds"), FCFBatchColumnRegistry::GetDatasetColumns(ECFBatchDatasetKind::RecipeNumericEdit, ECFVehicleProfileDomain::None, RecipeColumns, Errors)))
	{
		return false;
	}

	for (const FCFBatchColumnDescriptor& Descriptor : RecipeColumns)
	{
		if (Descriptor.bReservedMetadata)
		{
			TestEqual(TEXT("Reserved Recipe metadata is read-only"), Descriptor.Access, ECFBatchColumnAccess::ReadOnly);
			TestEqual(TEXT("Reserved Recipe metadata has no mutation kind"), Descriptor.TypedMutationKind, ECFBatchMutationKind::None);
			continue;
		}
		TestEqual(TEXT("Recipe allowlist data is editable"), Descriptor.Access, ECFBatchColumnAccess::Editable);
		TestTrue(TEXT("Recipe allowlist data is numeric"), Descriptor.ValueType == ECFBatchValueType::Number || Descriptor.ValueType == ECFBatchValueType::Integer);
		TestEqual(TEXT("Recipe allowlist owner is Recipe"), Descriptor.AuthoringOwner, ECFBatchAuthoringOwner::Recipe);
		TestEqual(TEXT("Recipe editable blank means NoChange"), Descriptor.BlankPolicy, ECFBatchBlankPolicy::NoChange);
		TestFalse(TEXT("Recipe allowlist excludes array/stable-id path"), Descriptor.PropertyOrSemanticTarget.Contains(TEXT("Hardpoint")) || Descriptor.PropertyOrSemanticTarget.Contains(TEXT("Mount")));
		TestFalse(TEXT("Recipe allowlist excludes asset refs"), Descriptor.PropertyOrSemanticTarget.Contains(TEXT("AssetIntent")) || Descriptor.PropertyOrSemanticTarget.Contains(TEXT("DefaultData")));
	}

	// Base Mass source-mode conditional descriptor입니다.
	const FCFBatchColumnDescriptor* BaseMassDescriptor = RecipeColumns.FindByPredicate([](const FCFBatchColumnDescriptor& Descriptor)
	{
		return Descriptor.ColumnId == TEXT("Recipe.Mass.ExplicitBaseMassKg");
	});
	TestNotNull(TEXT("Base Mass explicit descriptor exists"), BaseMassDescriptor);
	if (BaseMassDescriptor)
	{
		TestEqual(TEXT("Base Mass edit condition targets source mode"), BaseMassDescriptor->EditConditionTarget, FString(TEXT("MassIntent.BaseMassMode")));
		TestEqual(TEXT("Base Mass editable only in ExplicitValue"), BaseMassDescriptor->RequiredEditConditionValue, FString(TEXT("ExplicitValue")));
	}

	// Profile Domain별 expected non-reserved numeric counts입니다.
	struct FDomainCount
	{
		// Exact Profile Domain입니다.
		ECFVehicleProfileDomain Domain;

		// Current typed schema reflection으로 기대하는 scalar numeric leaf 수입니다.
		int32 ExpectedCount;
	};
	// Section 26.15 typed payload reflection baseline입니다.
	const FDomainCount ExpectedDomainCounts[] =
	{
		{ECFVehicleProfileDomain::VehicleBase, 8},
		{ECFVehicleProfileDomain::Drivetrain, 1},
		{ECFVehicleProfileDomain::Handling, 41},
				{ECFVehicleProfileDomain::Performance, 18},
		{ECFVehicleProfileDomain::DriveState, 11}
	};
	// 5 Domain 전체 numeric editable leaf 합계입니다.
	int32 TotalProfileNumericCount = 0;
	for (const FDomainCount& DomainCount : ExpectedDomainCounts)
	{
		// Exact Domain Profile columns입니다.
		TArray<FCFBatchColumnDescriptor> ProfileColumns;
		if (!TestTrue(TEXT("Profile Domain allowlist projection succeeds"), FCFBatchColumnRegistry::GetDatasetColumns(ECFBatchDatasetKind::ProfileNumericEdit, DomainCount.Domain, ProfileColumns, Errors)))
		{
			return false;
		}
		// Current Domain non-reserved numeric count입니다.
		const int32 DataColumnCount = CFBatchTestsPrivate::CountDataColumns(ProfileColumns);
		TestEqual(TEXT("Profile Domain numeric leaf count matches typed schema"), DataColumnCount, DomainCount.ExpectedCount);
		TotalProfileNumericCount += DataColumnCount;
		for (const FCFBatchColumnDescriptor& Descriptor : ProfileColumns)
		{
			if (Descriptor.bReservedMetadata)
			{
				continue;
			}
			TestEqual(TEXT("Profile numeric leaf is editable"), Descriptor.Access, ECFBatchColumnAccess::Editable);
			TestEqual(TEXT("Profile numeric leaf owner is Profile"), Descriptor.AuthoringOwner, ECFBatchAuthoringOwner::Profile);
			TestEqual(TEXT("Profile numeric leaf uses typed mutation metadata"), Descriptor.TypedMutationKind, ECFBatchMutationKind::ProfileNumericLeaf);
			TestTrue(TEXT("Profile writable leaf is numeric only"), Descriptor.ValueType == ECFBatchValueType::Number || Descriptor.ValueType == ECFBatchValueType::Integer);
			TestTrue(TEXT("Profile mutation target stays inside typed Data payload"), Descriptor.PropertyOrSemanticTarget.StartsWith(TEXT("Data.")));
		}
	}
		TestEqual(TEXT("Five Profile Domains expose exactly 79 scalar numeric leaves"), TotalProfileNumericCount, 79);
	return true;
}

// __cf_ prefix의 owner/read-only invariant와 duplicate technical identity를 fail-closed하는지 검증합니다.
bool FCFBatchReservedTest::RunTest(const FString& Parameters)
{
	// Valid baseline Recipe descriptors입니다.
	TArray<FCFBatchColumnDescriptor> ValidColumns;
	// Registry validation errors입니다.
	TArray<FString> Errors;
	if (!TestTrue(TEXT("Valid Recipe descriptors build"), FCFBatchColumnRegistry::GetDatasetColumns(ECFBatchDatasetKind::RecipeNumericEdit, ECFVehicleProfileDomain::None, ValidColumns, Errors)))
	{
		return false;
	}
	for (const FCFBatchColumnDescriptor& Descriptor : ValidColumns)
	{
		if (FCFBatchColumnRegistry::IsReservedColumnId(Descriptor.ColumnId))
		{
			TestTrue(TEXT("__cf_ descriptor is importer-owned metadata"), Descriptor.bReservedMetadata);
			TestEqual(TEXT("__cf_ descriptor is read-only"), Descriptor.Access, ECFBatchColumnAccess::ReadOnly);
		}
	}

	// Reserved prefix를 Recipe editable field가 가로채려는 invalid descriptor입니다.
	FCFBatchColumnDescriptor ReservedCollision;
	ReservedCollision.ColumnId = TEXT("__cf_fake_recipe_write");
	ReservedCollision.DatasetKind = ECFBatchDatasetKind::RecipeNumericEdit;
	ReservedCollision.Access = ECFBatchColumnAccess::Editable;
	ReservedCollision.AuthoringOwner = ECFBatchAuthoringOwner::Recipe;
	ReservedCollision.TypedMutationKind = ECFBatchMutationKind::RecipeDrivingFeelAxis;
	ReservedCollision.BlankPolicy = ECFBatchBlankPolicy::NoChange;
	ReservedCollision.bReservedMetadata = false;
	// Reserved collision을 포함한 invalid set입니다.
	TArray<FCFBatchColumnDescriptor> CollisionColumns = ValidColumns;
	CollisionColumns.Add(ReservedCollision);
	TestFalse(TEXT("Reserved __cf_ collision is rejected"), FCFBatchColumnRegistry::ValidateDescriptorSet(CollisionColumns, Errors));
	TestTrue(TEXT("Reserved collision emits diagnostics"), Errors.Num() > 0);

	// Existing ColumnId를 복제한 invalid descriptor set입니다.
	TArray<FCFBatchColumnDescriptor> DuplicateColumns = ValidColumns;
	DuplicateColumns.Add(ValidColumns[0]);
	TestFalse(TEXT("Duplicate stable ColumnId is rejected"), FCFBatchColumnRegistry::ValidateDescriptorSet(DuplicateColumns, Errors));
	TestTrue(TEXT("Duplicate ColumnId emits diagnostics"), Errors.Num() > 0);
	return true;
}

// 같은 export identity/baseline은 row input order와 무관하게 exact CSV/JSON/UTF-8/hash를 재현하는지 검증합니다.
bool FCFBatchCanonicalExportTest::RunTest(const FString& Parameters)
{
	// Canonical export Recipe fixture입니다.
	CFBatchTestsPrivate::FRecipeFixture Fixture;
	// Fixture/export diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Canonical export Recipe fixture builds"), CFBatchTestsPrivate::BuildRecipeFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}

	// Current RecipeNumericEdit row projection입니다.
	FCFBatchExportRow FirstRow;
	// Row/export validation diagnostics입니다.
	TArray<FString> Errors;
	if (!TestTrue(TEXT("BuildRecipeNumericRow succeeds"), FCFBatchExportService::BuildRecipeNumericRow(*Fixture.Recipe, Fixture.TargetVehicleData, FirstRow, Errors)))
	{
		for (const FString& RowError : Errors)
		{
			AddError(RowError);
		}
		return false;
	}

	// Row sorting과 quote/comma escaping을 동시에 검증할 second baseline row입니다.
	FCFBatchExportRow SecondRow = FirstRow;
	SecondRow.RowId = TEXT("z,row\"B");
	// 동일 export identity를 두 Build에서 재사용하는 fixed GUID입니다.
	const FGuid FixedExportId(0x11111111, 0x22222222, 0x33333333, 0x44444444);

	// First input order export request입니다.
	FCFBatchExportRequest RequestA;
	RequestA.BatchExportId = FixedExportId;
	RequestA.DatasetKind = ECFBatchDatasetKind::RecipeNumericEdit;
	RequestA.Rows = {SecondRow, FirstRow};
	// Reversed input order export request입니다.
	FCFBatchExportRequest RequestB = RequestA;
	RequestB.Rows = {FirstRow, SecondRow};

	// First canonical artifact입니다.
	FCFBatchExportArtifact ArtifactA;
	// Reordered-input canonical artifact입니다.
	FCFBatchExportArtifact ArtifactB;
	if (!TestTrue(TEXT("Canonical export A succeeds"), FCFBatchExportService::BuildExport(RequestA, ArtifactA, Errors)))
	{
		for (const FString& ExportError : Errors)
		{
			AddError(ExportError);
		}
		return false;
	}
	if (!TestTrue(TEXT("Canonical export B succeeds"), FCFBatchExportService::BuildExport(RequestB, ArtifactB, Errors)))
	{
		for (const FString& ExportError : Errors)
		{
			AddError(ExportError);
		}
		return false;
	}

	TestEqual(TEXT("Canonical CSV ignores input row order"), ArtifactA.CanonicalCsvText, ArtifactB.CanonicalCsvText);
	TestEqual(TEXT("Canonical manifest JSON ignores input row order"), ArtifactA.ManifestJsonText, ArtifactB.ManifestJsonText);
	TestEqual(TEXT("ExportSetHash ignores input row order"), ArtifactA.Manifest.ExportSetHash, ArtifactB.Manifest.ExportSetHash);
	TestTrue(TEXT("Canonical CSV UTF-8 bytes repeat exactly"), ArtifactA.CsvUtf8Bytes == ArtifactB.CsvUtf8Bytes);
	TestTrue(TEXT("Canonical manifest UTF-8 bytes repeat exactly"), ArtifactA.ManifestUtf8Bytes == ArtifactB.ManifestUtf8Bytes);
	TestFalse(TEXT("Canonical CSV UTF-8 has no BOM"), CFBatchTestsPrivate::HasUtf8Bom(ArtifactA.CsvUtf8Bytes));
	TestFalse(TEXT("Manifest UTF-8 has no BOM"), CFBatchTestsPrivate::HasUtf8Bom(ArtifactA.ManifestUtf8Bytes));
	TestTrue(TEXT("CSV standard quote escaping is applied"), ArtifactA.CanonicalCsvText.Contains(TEXT("\"z,row\"\"B\"")));
	TestTrue(TEXT("Invariant numeric decimal uses dot"), ArtifactA.CanonicalCsvText.Contains(TEXT("1234.5")));
	TestFalse(TEXT("Invariant numeric export does not use locale comma decimal"), ArtifactA.CanonicalCsvText.Contains(TEXT("1234,5")));

	// Header/first simple data row를 column index로 검사할 line split입니다.
	TArray<FString> Lines;
	ArtifactA.CanonicalCsvText.ParseIntoArray(Lines, TEXT("\n"), true);
	TestTrue(TEXT("Canonical CSV has header plus rows"), Lines.Num() >= 3);
	if (Lines.Num() >= 2)
	{
		// Canonical stable header cells입니다.
		TArray<FString> HeaderCells;
		Lines[0].ParseIntoArray(HeaderCells, TEXT(","), false);
		// First sorted Recipe row는 comma가 없는 fixture row입니다.
		TArray<FString> FirstDataCells;
		Lines[1].ParseIntoArray(FirstDataCells, TEXT(","), false);
		// UseProfile 상태라 blank여야 하는 Gross explicit shadow value column입니다.
		const int32 GrossColumnIndex = HeaderCells.Find(TEXT("Recipe.Mass.ExplicitGrossMassKg"));
		// Explicit 상태라 visible numeric이어야 하는 Base Mass column입니다.
		const int32 BaseColumnIndex = HeaderCells.Find(TEXT("Recipe.Mass.ExplicitBaseMassKg"));
		TestTrue(TEXT("Gross column exists"), GrossColumnIndex != INDEX_NONE);
		TestTrue(TEXT("Base column exists"), BaseColumnIndex != INDEX_NONE);
		if (GrossColumnIndex != INDEX_NONE && GrossColumnIndex < FirstDataCells.Num())
		{
			TestEqual(TEXT("UseProfile Gross shadow cell exports blank/unavailable"), FirstDataCells[GrossColumnIndex], FString());
		}
		if (BaseColumnIndex != INDEX_NONE && BaseColumnIndex < FirstDataCells.Num())
		{
			TestEqual(TEXT("Explicit Base Mass exports canonical numeric"), FirstDataCells[BaseColumnIndex], FString(TEXT("1234.5")));
		}
	}

	TestTrue(TEXT("Fresh manifest validates against current Registry and hash"), FCFBatchExportService::ValidateManifestBaseline(ArtifactA.Manifest, Errors));
	return true;
}

// Manifest tamper를 hash mismatch로 차단하고 Recipe/Profile/Target persistent truth와 dirty/revision이 export 동안 변하지 않는지 검증합니다.
bool FCFBatchManifestMutationTest::RunTest(const FString& Parameters)
{
	// Mutation-zero Recipe/Target fixture입니다.
	CFBatchTestsPrivate::FRecipeFixture Fixture;
	// Snapshot/fixture diagnostic입니다.
	FString Error;
	if (!TestTrue(TEXT("Manifest mutation-zero Recipe fixture builds"), CFBatchTestsPrivate::BuildRecipeFixture(Fixture, Error)))
	{
		AddError(Error);
		return false;
	}
	// Handling Profile을 소유하는 /Temp package입니다.
	UPackage* ProfilePackage = CFBatchTestsPrivate::CreateTestPackage(TEXT("CFBatchProfile"));
	// ProfileNumericEdit persistent fixture입니다.
	UCFHandlingProfile* HandlingProfile = NewObject<UCFHandlingProfile>(ProfilePackage, TEXT("DA_BatchHandling_Test"), RF_Transactional);
	if (!TestNotNull(TEXT("Handling Profile fixture creates"), HandlingProfile))
	{
		return false;
	}
	HandlingProfile->Data.FrontWheelMaxBrakeTorque = 4321.25f;
	HandlingProfile->Data.FrontWheelMaxSteerAngleByFeel.LowValue = 12.5f;
	HandlingProfile->Data.FrontWheelMaxSteerAngleByFeel.NeutralValue = 24.0f;
	HandlingProfile->Data.FrontWheelMaxSteerAngleByFeel.HighValue = 36.5f;

	// Export 전 Recipe fingerprint입니다.
	const FString RecipeFingerprintBefore = CFBatchTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error);
	// Export 전 Target full Definition hash입니다.
	const FString TargetHashBefore = CFBatchTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error);
	// Export 전 Recipe diagnostic revision입니다.
	const int32 RecipeRevisionBefore = Fixture.Recipe->AuthoringRevision;
	// Export 전 Profile diagnostic revision입니다.
	const int32 ProfileRevisionBefore = HandlingProfile->Meta.AuthoringRevision;
	// Export 전 selected Profile numeric authored value입니다.
	const float ProfileBrakeBefore = HandlingProfile->Data.FrontWheelMaxBrakeTorque;
	Fixture.TargetPackage->SetDirtyFlag(false);
	Fixture.RecipePackage->SetDirtyFlag(false);
	ProfilePackage->SetDirtyFlag(false);

	// Recipe read-only export baseline row입니다.
	FCFBatchExportRow RecipeRow;
	// Profile read-only export baseline row입니다.
	FCFBatchExportRow ProfileRow;
	// Export/validation diagnostics입니다.
	TArray<FString> Errors;
	TestTrue(TEXT("Recipe baseline row builds without mutation"), FCFBatchExportService::BuildRecipeNumericRow(*Fixture.Recipe, Fixture.TargetVehicleData, RecipeRow, Errors));
	TestTrue(TEXT("Profile baseline row builds without mutation"), FCFBatchExportService::BuildProfileNumericRow(*HandlingProfile, ECFVehicleProfileDomain::Handling, ProfileRow, Errors));
	// First Profile fingerprint used to prove repeatability/mutation0입니다.
	const FString ProfileFingerprintBefore = ProfileRow.ObjectFingerprint;

	// Recipe canonical export request입니다.
	FCFBatchExportRequest RecipeRequest;
	RecipeRequest.BatchExportId = FGuid(0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD);
	RecipeRequest.DatasetKind = ECFBatchDatasetKind::RecipeNumericEdit;
	RecipeRequest.Rows.Add(RecipeRow);
	// Profile canonical export request입니다.
	FCFBatchExportRequest ProfileRequest;
	ProfileRequest.BatchExportId = FGuid(0x12345678, 0x90ABCDEF, 0x10203040, 0x50607080);
	ProfileRequest.DatasetKind = ECFBatchDatasetKind::ProfileNumericEdit;
	ProfileRequest.ProfileDomain = ECFVehicleProfileDomain::Handling;
	ProfileRequest.Rows.Add(ProfileRow);

	// Recipe immutable baseline artifact입니다.
	FCFBatchExportArtifact RecipeArtifact;
	// Profile immutable baseline artifact입니다.
	FCFBatchExportArtifact ProfileArtifact;
	TestTrue(TEXT("Recipe export artifact builds"), FCFBatchExportService::BuildExport(RecipeRequest, RecipeArtifact, Errors));
	TestTrue(TEXT("Profile export artifact builds"), FCFBatchExportService::BuildExport(ProfileRequest, ProfileArtifact, Errors));
	TestTrue(TEXT("Recipe manifest validates"), FCFBatchExportService::ValidateManifestBaseline(RecipeArtifact.Manifest, Errors));
	TestTrue(TEXT("Profile manifest validates"), FCFBatchExportService::ValidateManifestBaseline(ProfileArtifact.Manifest, Errors));

	// One baseline canonical value만 조작한 corrupted manifest copy입니다.
	FCFBatchManifest TamperedManifest = RecipeArtifact.Manifest;
	if (TamperedManifest.Rows.Num() > 0 && TamperedManifest.Rows[0].BaselineCells.Num() > 0)
	{
		TamperedManifest.Rows[0].BaselineCells[0].CanonicalValue = TEXT("999999");
	}
	TestFalse(TEXT("Tampered manifest baseline fails ExportSetHash validation"), FCFBatchExportService::ValidateManifestBaseline(TamperedManifest, Errors));
	TestTrue(TEXT("Tampered manifest emits diagnostics"), Errors.Num() > 0);

	// Profile readback row를 다시 만들어 fingerprint/value가 unchanged인지 확인합니다.
	FCFBatchExportRow ProfileRowAfter;
	TestTrue(TEXT("Profile row readback after exports succeeds"), FCFBatchExportService::BuildProfileNumericRow(*HandlingProfile, ECFVehicleProfileDomain::Handling, ProfileRowAfter, Errors));
	TestEqual(TEXT("Recipe fingerprint unchanged after Batch export"), CFBatchTestsPrivate::BuildRecipeFingerprint(*Fixture.Recipe, Error), RecipeFingerprintBefore);
	TestEqual(TEXT("Target Definition hash unchanged after Batch export"), CFBatchTestsPrivate::BuildTargetHash(*Fixture.TargetVehicleData, Error), TargetHashBefore);
	TestEqual(TEXT("Profile fingerprint unchanged after Batch export"), ProfileRowAfter.ObjectFingerprint, ProfileFingerprintBefore);
	TestEqual(TEXT("Recipe AuthoringRevision unchanged"), Fixture.Recipe->AuthoringRevision, RecipeRevisionBefore);
	TestEqual(TEXT("Profile AuthoringRevision unchanged"), HandlingProfile->Meta.AuthoringRevision, ProfileRevisionBefore);
	TestEqual(TEXT("Profile authored numeric unchanged"), HandlingProfile->Data.FrontWheelMaxBrakeTorque, ProfileBrakeBefore);
	TestFalse(TEXT("Recipe package remains clean"), Fixture.RecipePackage->IsDirty());
	TestFalse(TEXT("Target package remains clean"), Fixture.TargetPackage->IsDirty());
	TestFalse(TEXT("Profile package remains clean"), ProfilePackage->IsDirty());
	return true;
}

#endif
