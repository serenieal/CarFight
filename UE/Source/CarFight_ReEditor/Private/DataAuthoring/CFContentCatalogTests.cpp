// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCatalogTests.cpp
// Version: v1.0.1
// Date: 2026-09-29
// Description: CF-FQ-058 CCAS-P0-05 Catalog / Compare + USER Workflow focused Automation입니다.
// Changelog:
// - v1.0.1: live Product mode가 xlsx adapter P2 미연결 상태를 Workbook/Diff unavailable로 명시하는 계약을 검증.
// - v1.0.0: persisted dual-consumer catalog, filter/family, selected compare/source, canonical workbook preview,
//   read-only authority status exact5를 최초 추가.
// Migration:
// - Product Apply/Save/Workbook persistent write를 수행하지 않습니다.
// - USER visual hierarchy/시선 흐름 평가는 Automation과 분리해 별도 USER review로 유지합니다.

#include "DataAuthoring/CFContentCatalog.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace CFContentCatalogTestsPrivate
{
	// Test용 exact catalog key를 만듭니다.
	FCFContentKey MakeKey(
		const FString& ContentType,
		const FString& ContentId)
	{
		// Test entry logical key입니다.
		FCFContentKey Key;
		Key.ContentTypeId.Value = ContentType;
		Key.ContentId = ContentId;
		return Key;
	}

	// Test용 numeric USER-facing value row를 만듭니다.
	FCFContentCatalogValue MakeNumericValue(
		const FString& FieldKey,
		const FString& DisplayName,
		const double NumericValue,
		const FString& UnitText,
		const FString& SourceText)
	{
		// Test numeric value row입니다.
		FCFContentCatalogValue Value;
		Value.FieldKey = FieldKey;
		Value.DisplayName = DisplayName;
		Value.NumericValue = NumericValue;
		Value.bNumeric = true;
		Value.UnitText = UnitText;
		Value.SourceText = SourceText;
		Value.ValueText = FString::Printf(
			TEXT("%.1f%s%s"),
			NumericValue,
			UnitText.IsEmpty() ? TEXT("") : TEXT(" "),
			*UnitText);
		return Value;
	}

	// Test용 catalog entry를 만듭니다.
	FCFContentCatalogEntry MakeEntry(
		const FString& ContentType,
		const FString& ContentId,
		const ECFContentCatalogType CatalogType,
		const FString& DisplayName,
		const FString& FamilyName,
		const FString& VariantName)
	{
		// Test catalog entry입니다.
		FCFContentCatalogEntry Entry;
		Entry.Key = MakeKey(ContentType, ContentId);
		Entry.CatalogType = CatalogType;
		Entry.DisplayName = DisplayName;
		Entry.SecondaryText =
			CatalogType == ECFContentCatalogType::Vehicle
				? TEXT("차량")
				: TEXT("무장");
		Entry.FamilyDisplayName = FamilyName;
		Entry.VariantDisplayName = VariantName;
		return Entry;
	}

	// Canonical string value를 만듭니다.
	FCFContentValue MakeStringValue(const FString& StringValue)
	{
		// Canonical string value입니다.
		FCFContentValue Value;
		Value.State = ECFContentValueState::Value;
		Value.Type = ECFContentValueType::String;
		Value.StringValue = StringValue;
		return Value;
	}

	// Canonical double value를 만듭니다.
	FCFContentValue MakeDoubleValue(const double NumericValue)
	{
		// Canonical double value입니다.
		FCFContentValue Value;
		Value.State = ECFContentValueState::Value;
		Value.Type = ECFContentValueType::Double;
		Value.FloatingPointValue = NumericValue;
		return Value;
	}

	// USER-facing text에 raw Project path가 노출됐는지 검사합니다.
	bool ContainsRawObjectPath(const FString& Text)
	{
		return Text.Contains(TEXT("/Game/"), ESearchCase::CaseSensitive)
			|| Text.Contains(TEXT("PersistentLevel"), ESearchCase::CaseSensitive);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentCatalogLiveTest,
	"CarFight.CCAS.CF_FQ_058.P0_05.LiveDualConsumerCatalog",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentCatalogFilterTest,
	"CarFight.CCAS.CF_FQ_058.P0_05.FilterFamilyVariant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentCatalogCompareTest,
	"CarFight.CCAS.CF_FQ_058.P0_05.SelectedCompareSources",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentCatalogPreviewTest,
	"CarFight.CCAS.CF_FQ_058.P0_05.CanonicalWorkbookPreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentCatalogAuthorityTest,
	"CarFight.CCAS.CF_FQ_058.P0_05.ReadOnlyAuthorityGuard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Persisted Weapon/Vehicle를 같은 P0-04 typed Resource Core를 통해 USER-facing Catalog로 읽는지 검증합니다.
bool FCFContentCatalogLiveTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Live persisted Product catalog ViewModel입니다.
	FCFContentCatalogVM ViewModel;

	// Live catalog refresh diagnostic입니다.
	FString Error;
	TestTrue(
		TEXT("Persisted Weapon + Vehicle catalog refresh succeeds"),
		ViewModel.RefreshLiveCatalog(Error));

	// Live filtered entries입니다.
	const TArray<FCFContentCatalogEntry>& Entries =
		ViewModel.GetFilteredEntries();
	TestTrue(
		TEXT("Live catalog has persisted content"),
		!Entries.IsEmpty());

	// Persisted Vehicle row 존재 여부입니다.
	bool bFoundVehicle = false;

	// Persisted Weapon row 존재 여부입니다.
	bool bFoundWeapon = false;

	// P0-04 Resource/Semantic resolution이 USER-facing row로 실제 투영됐는지 여부입니다.
	bool bFoundResolvedResource = false;

	for (const FCFContentCatalogEntry& Entry : Entries)
	{
		bFoundVehicle =
			bFoundVehicle
			|| Entry.CatalogType == ECFContentCatalogType::Vehicle;
		bFoundWeapon =
			bFoundWeapon
			|| Entry.CatalogType == ECFContentCatalogType::Weapon;
		bFoundResolvedResource =
			bFoundResolvedResource
			|| !Entry.Resources.IsEmpty();

		TestFalse(
			TEXT("DisplayName hides raw ObjectPath"),
			CFContentCatalogTestsPrivate::ContainsRawObjectPath(
				Entry.DisplayName));
		TestFalse(
			TEXT("SecondaryText hides raw ObjectPath"),
			CFContentCatalogTestsPrivate::ContainsRawObjectPath(
				Entry.SecondaryText));
		TestFalse(
			TEXT("Family label hides raw ObjectPath"),
			CFContentCatalogTestsPrivate::ContainsRawObjectPath(
				Entry.FamilyDisplayName));

		for (const FCFContentCatalogResource& Resource : Entry.Resources)
		{
			TestFalse(
				TEXT("Resource display hides raw ObjectPath"),
				CFContentCatalogTestsPrivate::ContainsRawObjectPath(
					Resource.ResourceDisplayName));
		}
	}

	TestTrue(
		TEXT("Live catalog includes Vehicle consumer"),
		bFoundVehicle);
	TestTrue(
		TEXT("Live catalog includes Weapon consumer"),
		bFoundWeapon);
	TestTrue(
		TEXT("Live catalog exposes P0-04 resolved Resource/Semantic rows"),
		bFoundResolvedResource);
	return true;
}

// Search/type filter와 Family/Variant grouping이 consumer에 독립적인 Presentation semantics로 동작하는지 검증합니다.
bool FCFContentCatalogFilterTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Synthetic read-only Presentation rows입니다.
	TArray<FCFContentCatalogEntry> Entries;
	Entries.Add(
		CFContentCatalogTestsPrivate::MakeEntry(
			TEXT("vehicle"),
			TEXT("vehicle_alpha"),
			ECFContentCatalogType::Vehicle,
			TEXT("Alpha SUV"),
			TEXT("SUV Family"),
			TEXT("기본형")));
	Entries.Add(
		CFContentCatalogTestsPrivate::MakeEntry(
			TEXT("vehicle"),
			TEXT("vehicle_beta"),
			ECFContentCatalogType::Vehicle,
			TEXT("Beta SUV"),
			TEXT("SUV Family"),
			TEXT("중장갑형")));
	Entries.Add(
		CFContentCatalogTestsPrivate::MakeEntry(
			TEXT("weapon"),
			TEXT("weapon_alpha"),
			ECFContentCatalogType::Weapon,
			TEXT("Alpha Cannon"),
			TEXT("Cannon Family"),
			TEXT("중포형")));

	// Read-only status입니다.
	FCFContentCatalogStatus Status;
	Status.ApplyStateText =
		TEXT("Product Apply 비활성");

	// Presentation ViewModel입니다.
	FCFContentCatalogVM ViewModel;
	ViewModel.SetEntriesForPresentation(Entries, Status);

	TestEqual(
		TEXT("All filter sees exact3 entries"),
		ViewModel.GetFilteredEntries().Num(),
		3);
	TestEqual(
		TEXT("Family grouping is exact2"),
		ViewModel.GetGroups().Num(),
		2);

	ViewModel.SetTypeFilter(ECFContentCatalogType::Vehicle);
	TestEqual(
		TEXT("Vehicle filter sees exact2 entries"),
		ViewModel.GetFilteredEntries().Num(),
		2);
	TestEqual(
		TEXT("Vehicle Family groups exact1"),
		ViewModel.GetGroups().Num(),
		1);

	ViewModel.SetSearchText(TEXT("중장갑"));
	TestEqual(
		TEXT("USER-facing Variant search narrows to exact1"),
		ViewModel.GetFilteredEntries().Num(),
		1);
	TestEqual(
		TEXT("Search result is Beta SUV"),
		ViewModel.GetFilteredEntries()[0].DisplayName,
		FString(TEXT("Beta SUV")));
	return true;
}

// Selected vs Compare absolute/relative/source projection이 deterministic하게 동작하는지 검증합니다.
bool FCFContentCatalogCompareTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Selected test entry입니다.
	FCFContentCatalogEntry Selected =
		CFContentCatalogTestsPrivate::MakeEntry(
			TEXT("weapon"),
			TEXT("weapon_selected"),
			ECFContentCatalogType::Weapon,
			TEXT("Selected Cannon"),
			TEXT("Cannon Family"),
			TEXT("고속형"));
	Selected.Values.Add(
		CFContentCatalogTestsPrivate::MakeNumericValue(
			TEXT("weapon.damage"),
			TEXT("공격력"),
			120.0,
			TEXT(""),
			TEXT("직접 입력")));
	Selected.Values.Add(
		CFContentCatalogTestsPrivate::MakeNumericValue(
			TEXT("weapon.mass"),
			TEXT("질량"),
			30.0,
			TEXT("kg"),
			TEXT("프로파일")));

	// Compare baseline test entry입니다.
	FCFContentCatalogEntry Baseline =
		CFContentCatalogTestsPrivate::MakeEntry(
			TEXT("weapon"),
			TEXT("weapon_baseline"),
			ECFContentCatalogType::Weapon,
			TEXT("Baseline Cannon"),
			TEXT("Cannon Family"),
			TEXT("기본형"));
	Baseline.Values.Add(
		CFContentCatalogTestsPrivate::MakeNumericValue(
			TEXT("weapon.damage"),
			TEXT("공격력"),
			100.0,
			TEXT(""),
			TEXT("프로파일")));
	Baseline.Values.Add(
		CFContentCatalogTestsPrivate::MakeNumericValue(
			TEXT("weapon.mass"),
			TEXT("질량"),
			25.0,
			TEXT("kg"),
			TEXT("직접 입력")));

	// Compare Presentation rows입니다.
	TArray<FCFContentCatalogEntry> Entries;
	Entries.Add(Selected);
	Entries.Add(Baseline);

	// Read-only status입니다.
	FCFContentCatalogStatus Status;
	Status.ApplyStateText =
		TEXT("Product Apply 비활성");

	// Compare ViewModel입니다.
	FCFContentCatalogVM ViewModel;
	ViewModel.SetEntriesForPresentation(Entries, Status);
	TestTrue(
		TEXT("Selected row is accepted"),
		ViewModel.SelectEntry(Selected.Key));
	TestTrue(
		TEXT("Compare row is accepted"),
		ViewModel.SelectCompareEntry(Baseline.Key));

	// Deterministic compare rows입니다.
	const TArray<FCFContentCompareRow> CompareRows =
		ViewModel.BuildCompareRows();
	TestEqual(
		TEXT("Compare rows exact2"),
		CompareRows.Num(),
		2);

	// Damage compare row입니다.
	const FCFContentCompareRow* DamageRow =
		CompareRows.FindByPredicate(
			[](const FCFContentCompareRow& Row)
			{
				return Row.DisplayName.Equals(
					TEXT("공격력"),
					ESearchCase::CaseSensitive);
			});
	TestNotNull(
		TEXT("Damage compare row exists"),
		DamageRow);
	if (DamageRow != nullptr)
	{
		TestTrue(
			TEXT("Absolute delta is +20"),
			DamageRow->RelativeDelta.Contains(
				TEXT("+20.000"),
				ESearchCase::CaseSensitive));
		TestTrue(
			TEXT("Relative delta is +20 percent"),
			DamageRow->RelativeDelta.Contains(
				TEXT("+20.0%"),
				ESearchCase::CaseSensitive));
		TestEqual(
			TEXT("Selected source is preserved"),
			DamageRow->SelectedSource,
			FString(TEXT("직접 입력")));
		TestEqual(
			TEXT("Compare source is preserved"),
			DamageRow->CompareSource,
			FString(TEXT("프로파일")));
	}
	return true;
}

// P0-01 Canonical Workbook + P0-02 Diff/Dependency를 raw Stable ID 노출 없이 Catalog Preview로 투영하는지 검증합니다.
bool FCFContentCatalogPreviewTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Canonical primary sheet입니다.
	FCFContentSheetDescriptor WeaponSheet;
	WeaponSheet.SheetId = TEXT("weapon_sheet");
	WeaponSheet.ContentTypeId.Value = TEXT("weapon");

	// USER-facing display name field descriptor입니다.
	FCFContentFieldDescriptor NameField;
	NameField.ColumnId = TEXT("display_name");
	NameField.DisplayLabel = TEXT("표시 이름");
	NameField.ValueType = ECFContentValueType::String;
	WeaponSheet.Fields.Add(NameField);

	// USER-facing damage field descriptor입니다.
	FCFContentFieldDescriptor DamageField;
	DamageField.ColumnId = TEXT("damage");
	DamageField.DisplayLabel = TEXT("공격력");
	DamageField.ValueType = ECFContentValueType::Double;
	DamageField.CanonicalUnitId = TEXT("damage");
	WeaponSheet.Fields.Add(DamageField);

	// Selected canonical record입니다.
	FCFContentRecord AlphaRecord;
	AlphaRecord.Key =
		CFContentCatalogTestsPrivate::MakeKey(
			TEXT("weapon"),
			TEXT("weapon_alpha"));
	AlphaRecord.ManagementState =
		ECFContentManagementState::Managed;
	AlphaRecord.AuthoringMetadata.FamilyId =
		TEXT("heavy_cannon_family");
	AlphaRecord.AuthoringMetadata.DesignIntent =
		TEXT("고정밀 중포 계열");
	AlphaRecord.Fields.Add(
		TEXT("display_name"),
		CFContentCatalogTestsPrivate::MakeStringValue(
			TEXT("알파 캐논")));
	AlphaRecord.Fields.Add(
		TEXT("damage"),
		CFContentCatalogTestsPrivate::MakeDoubleValue(120.0));

	// Compare canonical variant record입니다.
	FCFContentRecord BetaRecord;
	BetaRecord.Key =
		CFContentCatalogTestsPrivate::MakeKey(
			TEXT("weapon"),
			TEXT("weapon_beta"));
	BetaRecord.ManagementState =
		ECFContentManagementState::Managed;
	BetaRecord.AuthoringMetadata.FamilyId =
		TEXT("heavy_cannon_family");
	BetaRecord.AuthoringMetadata.BaseContentId =
		TEXT("weapon_alpha");
	BetaRecord.Fields.Add(
		TEXT("display_name"),
		CFContentCatalogTestsPrivate::MakeStringValue(
			TEXT("베타 캐논")));
	BetaRecord.Fields.Add(
		TEXT("damage"),
		CFContentCatalogTestsPrivate::MakeDoubleValue(100.0));

	// Existing P0-02 Compile Preview result shape입니다.
	FCFContentCompileResult CompileResult;
	CompileResult.bSucceeded = true;
	CompileResult.Workbook.WorkbookSourceId =
		TEXT("carfight_content");
	CompileResult.Workbook.SchemaRevision = 42;
	CompileResult.Workbook.Sheets.Add(WeaponSheet);
	CompileResult.Workbook.Records.Add(AlphaRecord);
	CompileResult.Workbook.Records.Add(BetaRecord);

	// Alpha record의 preview diff입니다.
	FCFContentCompileDiff AlphaDiff;
	AlphaDiff.Key = AlphaRecord.Key;
	AlphaDiff.Kind = ECFContentCompileDiffKind::Modified;
	AlphaDiff.ManagementState =
		ECFContentManagementState::Managed;
	AlphaDiff.ChangedPaths.Add(TEXT("damage"));
	CompileResult.Diffs.Add(AlphaDiff);

	// Alpha -> Beta canonical dependency입니다.
	FCFContentDependencyEdge Dependency;
	Dependency.From = AlphaRecord.Key;
	Dependency.To = BetaRecord.Key;
	Dependency.FieldPath = TEXT("related_variant");
	CompileResult.DependencyEdges.Add(Dependency);

	// Alpha record non-blocking validation evidence입니다.
	FCFContentValidationIssue Issue;
	Issue.Code = TEXT("PreviewWarning");
	Issue.Path = TEXT("weapon_alpha/damage");
	Issue.Message = TEXT("Preview warning");
	Issue.bBlocking = false;
	CompileResult.Issues.Add(Issue);

	// Canonical Preview Presentation ViewModel입니다.
	FCFContentCatalogVM ViewModel;
	ViewModel.ApplyCompilePreview(CompileResult);

	TestEqual(
		TEXT("Canonical preview rows exact2"),
		ViewModel.GetFilteredEntries().Num(),
		2);
	TestEqual(
		TEXT("Workbook revision is USER-visible"),
		ViewModel.GetStatus().WorkbookRevisionText,
		FString(TEXT("Workbook Schema Revision 42")));
	TestTrue(
		TEXT("Apply remains disabled"),
		ViewModel.GetStatus().ApplyStateText.Contains(
			TEXT("비활성"),
			ESearchCase::CaseSensitive));

	TestTrue(
		TEXT("Alpha selection succeeds"),
		ViewModel.SelectEntry(AlphaRecord.Key));
	TestTrue(
		TEXT("Beta compare selection succeeds"),
		ViewModel.SelectCompareEntry(BetaRecord.Key));

	// Selected USER-facing row입니다.
	const FCFContentCatalogEntry* SelectedEntry =
		ViewModel.GetSelectedEntry();
	TestNotNull(
		TEXT("Selected canonical row exists"),
		SelectedEntry);
	if (SelectedEntry != nullptr)
	{
		TestEqual(
			TEXT("Display name comes from canonical authored field"),
			SelectedEntry->DisplayName,
			FString(TEXT("알파 캐논")));
		TestEqual(
			TEXT("Raw FamilyId is hidden behind USER label"),
			SelectedEntry->FamilyDisplayName,
			FString(TEXT("Family 지정됨")));
		TestFalse(
			TEXT("Family machine ID is not shown"),
			SelectedEntry->FamilyDisplayName.Contains(
				TEXT("heavy_cannon_family"),
				ESearchCase::CaseSensitive));
	}

	TestTrue(
		TEXT("Dependency view resolves target display name"),
		ViewModel.BuildDependencyText().Contains(
			TEXT("베타 캐논"),
			ESearchCase::CaseSensitive));
	TestTrue(
		TEXT("Diff Preview shows modified state"),
		ViewModel.BuildSelectedDiffText().Contains(
			TEXT("변경 예정"),
			ESearchCase::CaseSensitive));
	TestTrue(
		TEXT("Diff Preview hides raw changed path and shows count"),
		ViewModel.BuildSelectedDiffText().Contains(
			TEXT("변경 항목 1개"),
			ESearchCase::CaseSensitive));
	return true;
}

// P0-05 live USER workflow가 P0-07 이전 read-only authority boundary를 명확히 표시하는지 검증합니다.
bool FCFContentCatalogAuthorityTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Live persisted Product catalog ViewModel입니다.
	FCFContentCatalogVM ViewModel;

	// Live catalog refresh diagnostic입니다.
	FString Error;
	TestTrue(
		TEXT("Live catalog refresh succeeds"),
		ViewModel.RefreshLiveCatalog(Error));

	// USER-facing authority status입니다.
	const FCFContentCatalogStatus& Status =
		ViewModel.GetStatus();
	TestTrue(
		TEXT("Catalog says current Product is read-only"),
		Status.CatalogSourceText.Contains(
			TEXT("읽기 전용"),
			ESearchCase::CaseSensitive));
	TestTrue(
		TEXT("Workbook cutover is explicitly pending P0-07"),
		Status.WorkbookAuthorityText.Contains(
			TEXT("P0-07"),
			ESearchCase::CaseSensitive));
	TestTrue(
		TEXT("Product Apply is explicitly disabled"),
		Status.ApplyStateText.Contains(
			TEXT("비활성"),
			ESearchCase::CaseSensitive));
	TestTrue(
		TEXT("Live Workbook Preview states xlsx adapter P2 boundary"),
		Status.WorkbookRevisionText.Contains(
			TEXT("P2"),
			ESearchCase::CaseSensitive));
	TestTrue(
		TEXT("Live Diff Preview states xlsx adapter P2 boundary"),
		Status.DiffPreviewText.Contains(
			TEXT("P2"),
			ESearchCase::CaseSensitive));

	// First available entry의 specialized edit navigation contract입니다.
	const TArray<FCFContentCatalogEntry>& Entries =
		ViewModel.GetFilteredEntries();
	if (!Entries.IsEmpty())
	{
		TestTrue(
			TEXT("Live entry selection succeeds"),
			ViewModel.SelectEntry(Entries[0].Key));
		TestFalse(
			TEXT("Specialized edit TabId exists without exposing ObjectPath"),
			ViewModel.GetSelectedEditTabId().IsNone());
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
