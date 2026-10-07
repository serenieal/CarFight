// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCoreTests.cpp
// Version: v1.1.0
// Date: 2026-09-29
// Description: CF-FQ-058 CCAS-P0-01 canonical schema/value/collection/hash/ExternalReadOnly/migration/provider focused Automation입니다.
// Changelog:
// - v1.1.0: WorkbookSource/RowId, Unit/Derived, Family/Profile/DesignIntent, FText mode, typed ChangeSet, collection semantic identity regression을 보강.
// - v1.0.4: locale comma/trailing junk 거부와 ASCII exponent 허용을 canonical Float/Double regression으로 고정.
// - v1.0.3: Duplicate ContentKey/ColumnId fixture의 남은 TArray self-add를 value copy로 교정.
// - v1.0.2: Duplicate ChildItemId fixture가 source TArray element를 직접 self-add하던 Automation assertion을 value copy로 교정.
// - v1.0.1: canonical value 타입 coverage와 Duplicate SheetId/ChildItemId fail-closed regression을 보강.
// - v1.0.0: in-memory exact8 focused tests를 최초 구현.
// Migration:
// - Product DataAsset load/save/mutation을 수행하지 않습니다.
// - 모든 fixture는 transient C++ model만 사용합니다.

#include "DataAuthoring/CFContentCore.h"
#include "DataAuthoring/CFContentProvider.h"
#include "DataAuthoring/CFDACommonPrimitives.h"

#include "Algo/Reverse.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace CFContentCoreTestsPrivate
{
	// CCAS-P0-01 fixture가 사용하는 logical ContentType입니다.
	const FCFContentTypeId FixtureContentType{TEXT("Weapon")};

	// CCASManaged field descriptor를 생성합니다.
	FCFContentFieldDescriptor MakeField(
		const FString& ColumnId,
		const ECFContentValueType ValueType,
		const bool bRequired = false,
		const bool bAllowNone = false)
	{
		// 생성할 field descriptor입니다.
		FCFContentFieldDescriptor Field;
		Field.ColumnId = ColumnId;
		Field.DisplayLabel = TEXT("표시 전용 Label");
		Field.ValueType = ValueType;
		Field.Ownership = ECFContentFieldOwnership::CCASManaged;
		Field.TextMode = ValueType == ECFContentValueType::Text
			? ECFContentTextMode::LiteralInvariant
			: ECFContentTextMode::NotText;
		Field.bRequired = bRequired;
		Field.bAllowNone = bAllowNone;
		return Field;
	}

	// Canonical String VALUE를 생성합니다.
	FCFContentValue MakeStringValue(const FString& Text)
	{
		// 생성할 String canonical value입니다.
		FCFContentValue Value;
		Value.Type = ECFContentValueType::String;
		Value.State = ECFContentValueState::Value;
		Value.StringValue = Text;
		return Value;
	}

	// Canonical Double VALUE를 생성합니다.
	FCFContentValue MakeDoubleValue(const double Number)
	{
		// 생성할 Double canonical value입니다.
		FCFContentValue Value;
		Value.Type = ECFContentValueType::Double;
		Value.State = ECFContentValueState::Value;
		Value.FloatingPointValue = Number;
		return Value;
	}

	// Canonical NameId VALUE를 생성합니다.
	FCFContentValue MakeNameValue(const FString& Name)
	{
		// 생성할 NameId canonical value입니다.
		FCFContentValue Value;
		Value.Type = ECFContentValueType::NameId;
		Value.State = ECFContentValueState::Value;
		Value.StringValue = Name;
		return Value;
	}

	// Collection child item을 생성합니다.
	FCFContentCollectionItem MakeItem(
		const FString& ChildItemId,
		const FString& DomainKey,
		const int32 Order,
		const FString& Text)
	{
		// 생성할 child collection item입니다.
		FCFContentCollectionItem Item;
		Item.ChildItemId = ChildItemId;
		Item.DomainKey = DomainKey;
		Item.Order = Order;
		Item.Fields.Add(TEXT("Value"), MakeStringValue(Text));
		return Item;
	}

	// P0-01 schema/value/collection/hash tests 공용 canonical workbook fixture를 생성합니다.
	FCFContentWorkbookModel MakeFixtureWorkbook()
	{
		// 생성할 canonical workbook입니다.
		FCFContentWorkbookModel Workbook;
		Workbook.WorkbookSourceId = TEXT("CarFight.Content.Primary");
		Workbook.SchemaRevision = 1;

		// Primary content sheet입니다.
		FCFContentSheetDescriptor MainSheet;
		MainSheet.SheetId = TEXT("Content.Main");
		MainSheet.SchemaRevision = 1;
		MainSheet.ContentTypeId = FixtureContentType;

		// DisplayName CCASManaged required field입니다.
		FCFContentFieldDescriptor DisplayNameField =
			MakeField(TEXT("DisplayName"), ECFContentValueType::String, true, true);
		// Mass external-managed field입니다.
		FCFContentFieldDescriptor MassField =
			MakeField(TEXT("Mass"), ECFContentValueType::Double, true, false);
		MassField.Ownership = ECFContentFieldOwnership::ExternalManaged;
		MassField.ExternalOwnerId = TEXT("Physics");
		MassField.CanonicalUnitId = TEXT("kg");
		MassField.FieldDomainId = TEXT("Physics");
		// Category read-only NameId field입니다.
		FCFContentFieldDescriptor CategoryField =
			MakeField(TEXT("Category"), ECFContentValueType::NameId, true, false);
		CategoryField.Ownership = ECFContentFieldOwnership::ReadOnly;
		// Authority input으로 중복 저장하지 않는 derived DPS descriptor입니다.
		FCFContentFieldDescriptor DerivedDpsField =
			MakeField(TEXT("Dps"), ECFContentValueType::Double, false, false);
		DerivedDpsField.Ownership = ECFContentFieldOwnership::Derived;
		DerivedDpsField.CanonicalUnitId = TEXT("dps");

		MainSheet.Fields = {DisplayNameField, MassField, CategoryField, DerivedDpsField};
		Workbook.Sheets.Add(MainSheet);

		// OrderedList child sheet입니다.
		FCFContentSheetDescriptor OrderedSheet;
		OrderedSheet.SheetId = TEXT("Content.Steps");
		OrderedSheet.SchemaRevision = 1;
		OrderedSheet.ContentTypeId = FixtureContentType;
		OrderedSheet.ParentSheetId = MainSheet.SheetId;
		OrderedSheet.CollectionId = TEXT("Steps");
		OrderedSheet.CollectionKind = ECFContentCollectionKind::OrderedList;
		OrderedSheet.Fields = {MakeField(TEXT("Value"), ECFContentValueType::String, true, false)};
		Workbook.Sheets.Add(OrderedSheet);

		// KeyedCollection child sheet입니다.
		FCFContentSheetDescriptor KeyedSheet;
		KeyedSheet.SheetId = TEXT("Content.Sockets");
		KeyedSheet.SchemaRevision = 1;
		KeyedSheet.ContentTypeId = FixtureContentType;
		KeyedSheet.ParentSheetId = MainSheet.SheetId;
		KeyedSheet.CollectionId = TEXT("Sockets");
		KeyedSheet.CollectionKind = ECFContentCollectionKind::KeyedCollection;
		KeyedSheet.Fields = {MakeField(TEXT("Value"), ECFContentValueType::String, true, false)};
		Workbook.Sheets.Add(KeyedSheet);

		// UnorderedSet child sheet입니다.
		FCFContentSheetDescriptor SetSheet;
		SetSheet.SheetId = TEXT("Content.Tags");
		SetSheet.SchemaRevision = 1;
		SetSheet.ContentTypeId = FixtureContentType;
		SetSheet.ParentSheetId = MainSheet.SheetId;
		SetSheet.CollectionId = TEXT("Tags");
		SetSheet.CollectionKind = ECFContentCollectionKind::UnorderedSet;
		SetSheet.Fields = {MakeField(TEXT("Value"), ECFContentValueType::String, true, false)};
		Workbook.Sheets.Add(SetSheet);

		// First managed record입니다.
		FCFContentRecord FirstRecord;
		FirstRecord.Key.ContentTypeId = FixtureContentType;
		FirstRecord.Key.ContentId = TEXT("HeavyCannon");
		FirstRecord.RowId = FirstRecord.Key.ToStableString();
		FirstRecord.ManagementState = ECFContentManagementState::Managed;
		FirstRecord.AuthoringMetadata.FamilyId = TEXT("Cannon120");
		FirstRecord.AuthoringMetadata.BaseContentId = TEXT("Cannon120_STD");
		FirstRecord.AuthoringMetadata.DesignIntent = TEXT("표준형보다 화력을 높이고 연사력과 질량을 희생한다.");
		FirstRecord.AuthoringMetadata.AssignedProfileIdsByDomain.Add(TEXT("Visual"), TEXT("VisualProfile.Cannon.Heavy"));
		FirstRecord.Fields.Add(TEXT("DisplayName"), MakeStringValue(TEXT("Heavy Cannon")));
		FirstRecord.Fields.Add(TEXT("Mass"), MakeDoubleValue(120.0));
		FirstRecord.Fields.Add(TEXT("Category"), MakeNameValue(TEXT("Heavy")));

		// OrderedList fixture입니다.
		FCFContentCollection Steps;
		Steps.CollectionId = TEXT("Steps");
		Steps.Kind = ECFContentCollectionKind::OrderedList;
		Steps.Items.Add(MakeItem(TEXT("step.fire"), TEXT(""), 1, TEXT("Fire")));
		Steps.Items.Add(MakeItem(TEXT("step.aim"), TEXT(""), 0, TEXT("Aim")));
		FirstRecord.Collections.Add(Steps.CollectionId, Steps);

		// KeyedCollection fixture입니다.
		FCFContentCollection Sockets;
		Sockets.CollectionId = TEXT("Sockets");
		Sockets.Kind = ECFContentCollectionKind::KeyedCollection;
		Sockets.Items.Add(MakeItem(TEXT("socket.rear"), TEXT("Rear"), INDEX_NONE, TEXT("RearSocket")));
		Sockets.Items.Add(MakeItem(TEXT("socket.front"), TEXT("Front"), INDEX_NONE, TEXT("FrontSocket")));
		FirstRecord.Collections.Add(Sockets.CollectionId, Sockets);

		// UnorderedSet fixture입니다.
		FCFContentCollection Tags;
		Tags.CollectionId = TEXT("Tags");
		Tags.Kind = ECFContentCollectionKind::UnorderedSet;
		Tags.Items.Add(MakeItem(TEXT("tag.heavy"), TEXT(""), INDEX_NONE, TEXT("Heavy")));
		Tags.Items.Add(MakeItem(TEXT("tag.cannon"), TEXT(""), INDEX_NONE, TEXT("Cannon")));
		FirstRecord.Collections.Add(Tags.CollectionId, Tags);
		Workbook.Records.Add(FirstRecord);

		// Second ExternalReadOnly record입니다.
		FCFContentRecord SecondRecord;
		SecondRecord.Key.ContentTypeId = FixtureContentType;
		SecondRecord.Key.ContentId = TEXT("LegacyRocket");
		SecondRecord.RowId = SecondRecord.Key.ToStableString();
		SecondRecord.ManagementState = ECFContentManagementState::ExternalReadOnly;
		SecondRecord.Fields.Add(TEXT("DisplayName"), MakeStringValue(TEXT("Legacy Rocket")));
		SecondRecord.Fields.Add(TEXT("Mass"), MakeDoubleValue(75.0));
		SecondRecord.Fields.Add(TEXT("Category"), MakeNameValue(TEXT("Rocket")));
		Workbook.Records.Add(SecondRecord);

		return Workbook;
	}

	// P0-01 registry test용 non-mutating fake provider입니다.
	class FFixtureProvider final : public ICFContentProvider
	{
	public:
		// Fixture provider schema를 설명합니다.
		virtual bool DescribeSchema(
			FCFContentProviderDescriptor& OutDescriptor,
			FString& OutError) const override
		{
			OutError.Reset();
			// Provider schema source fixture입니다.
			const FCFContentWorkbookModel Workbook = MakeFixtureWorkbook();
			OutDescriptor.ProviderId = TEXT("Fixture.Provider");
			OutDescriptor.ContentTypeId = FixtureContentType;
			OutDescriptor.SchemaRevision = 1;
			OutDescriptor.Sheets = Workbook.Sheets;
			return true;
		}

		// Existing Product 대신 in-memory fixture를 read-only import합니다.
		virtual bool ImportExisting(
			FCFContentImportResult& OutImport,
			FString& OutError) const override
		{
			OutError.Reset();
			OutImport = FCFContentImportResult();
			OutImport.Records = MakeFixtureWorkbook().Records;
			return true;
		}

		// Exact fixture key의 current snapshot을 생성합니다.
		virtual bool BuildCurrentSnapshot(
			const FCFContentKey& Key,
			FCFContentSnapshot& OutSnapshot,
			FString& OutError) const override
		{
			OutError.Reset();
			// Current fixture workbook입니다.
			const FCFContentWorkbookModel Workbook = MakeFixtureWorkbook();
			// Requested fixture record입니다.
			const FCFContentRecord* Record = Workbook.Records.FindByPredicate([&Key](const FCFContentRecord& Candidate)
			{
				return Candidate.Key == Key;
			});
			if (Record == nullptr)
			{
				OutError = TEXT("Fixture key not found.");
				return false;
			}
			OutSnapshot.Key = Key;
			OutSnapshot.Record = *Record;
			OutSnapshot.ReadbackFingerprint = TEXT("sha256:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
			return true;
		}

		// Fixture record basic identity를 검증합니다.
		virtual bool ValidateRecord(
			const FCFContentRecord& Record,
			TArray<FCFContentValidationIssue>& OutIssues) const override
		{
			OutIssues.Reset();
			return Record.Key.IsValid();
		}

		// Fixture는 dependency edge를 생성하지 않습니다.
		virtual bool BuildDependencyEdges(
			const FCFContentRecord& Record,
			TArray<FCFContentDependencyEdge>& OutEdges,
			FString& OutError) const override
		{
			OutError.Reset();
			OutEdges.Reset();
			return Record.Key.IsValid();
		}

		// Current/desired fixture diff seam을 구성합니다.
		virtual bool BuildDiff(
			const FCFContentSnapshot& Current,
			const FCFContentRecord& Desired,
			FCFContentDiff& OutDiff,
			FString& OutError) const override
		{
			OutError.Reset();
			OutDiff.Key = Desired.Key;
			OutDiff.CurrentFingerprint = Current.ReadbackFingerprint;
			OutDiff.DesiredFingerprint = TEXT("sha256:bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
			OutDiff.ChangedPaths = {TEXT("DisplayName")};
			return Current.Key == Desired.Key;
		}

		// Reviewable fixture mutation plan을 구성합니다.
		virtual bool BuildReviewedMutationPlan(
			const FCFContentDiff& Diff,
			const FString& BaseWorkbookSemanticHash,
			FCFContentReviewedMutationPlan& OutPlan,
			FString& OutError) const override
		{
			OutError.Reset();
			OutPlan.Key = Diff.Key;
			OutPlan.BaseWorkbookSemanticHash = BaseWorkbookSemanticHash;
			OutPlan.DesiredFingerprint = Diff.DesiredFingerprint;
			OutPlan.ChangedPaths = Diff.ChangedPaths;
			OutPlan.bApproved = false;
			return true;
		}

		// P0-01 fake provider는 Product Apply를 의도적으로 열지 않습니다.
		virtual bool ApplyReviewed(
			const FCFContentReviewedMutationPlan& ReviewedPlan,
			FCFContentApplyResult& OutResult) override
		{
			OutResult = FCFContentApplyResult();
			OutResult.Error = ReviewedPlan.bApproved
				? TEXT("P0-01 fixture has no Product writer.")
				: TEXT("Reviewed plan is not approved.");
			return false;
		}

		// Fixture readback fingerprint를 반환합니다.
		virtual bool ReadbackFingerprint(
			const FCFContentKey& Key,
			FString& OutFingerprint,
			FString& OutError) const override
		{
			OutError.Reset();
			if (!Key.IsValid())
			{
				OutError = TEXT("Invalid fixture key.");
				return false;
			}
			OutFingerprint = TEXT("sha256:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
			return true;
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentCoreSchemaTest,
	"CarFight.CCAS.CF_FQ_058.P0_01.CoreSchema",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentValueStatesTest,
	"CarFight.CCAS.CF_FQ_058.P0_01.ValueStates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentCollectionsTest,
	"CarFight.CCAS.CF_FQ_058.P0_01.Collections",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentSemanticHashTest,
	"CarFight.CCAS.CF_FQ_058.P0_01.SemanticHash",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentFailClosedTest,
	"CarFight.CCAS.CF_FQ_058.P0_01.FailClosed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentExternalReadOnlyTest,
	"CarFight.CCAS.CF_FQ_058.P0_01.ExternalReadOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentMigrationTest,
	"CarFight.CCAS.CF_FQ_058.P0_01.Migration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFContentProviderRegistryTest,
	"CarFight.CCAS.CF_FQ_058.P0_01.ProviderRegistry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Stable machine schema와 ownership classification을 검증합니다.
bool FCFContentCoreSchemaTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Canonical fixture workbook입니다.
	const FCFContentWorkbookModel Workbook = CFContentCoreTestsPrivate::MakeFixtureWorkbook();
	// Validation diagnostics입니다.
	TArray<FCFContentValidationIssue> Issues;
	TestTrue(TEXT("Canonical fixture workbook must validate"), FCFContentSchemaValidator::ValidateWorkbook(Workbook, Issues));
	TestEqual(TEXT("Canonical fixture must have no validation issues"), Issues.Num(), 0);
	TestTrue(TEXT("ContentKey type identity must be valid"), Workbook.Records[0].Key.ContentTypeId.IsValid());
	TestEqual(TEXT("Top-level RowId must equal canonical ContentKey token"), Workbook.Records[0].RowId, Workbook.Records[0].Key.ToStableString());
	TestTrue(TEXT("Stable SheetId must not depend on worksheet tab label"), CFIsStableContentId(Workbook.Sheets[0].SheetId));
	TestTrue(TEXT("Stable ColumnId must be valid"), CFIsStableContentId(Workbook.Sheets[0].Fields[0].ColumnId));
	TestEqual(TEXT("WorkbookSourceId must be stable and filename-independent"), Workbook.WorkbookSourceId, FString(TEXT("CarFight.Content.Primary")));
	TestEqual(TEXT("Schema must own canonical unit"), Workbook.Sheets[0].Fields[1].CanonicalUnitId, FString(TEXT("kg")));
	TestTrue(TEXT("Derived field must remain explicitly classified"), Workbook.Sheets[0].Fields[3].Ownership == ECFContentFieldOwnership::Derived);
	TestEqual(TEXT("Family metadata must be preserved"), Workbook.Records[0].AuthoringMetadata.FamilyId, FString(TEXT("Cannon120")));
	TestFalse(TEXT("DesignIntent must remain authored metadata"), Workbook.Records[0].AuthoringMetadata.DesignIntent.IsEmpty());

	// Change Set concurrency token으로 사용할 current WorkbookSemanticHash입니다.
	FString BaseSemanticHash;
	// Semantic hash build error입니다.
	FString HashError;
	TestTrue(TEXT("ChangeSet fixture base hash must build"), FCFContentSemanticHasher::BuildWorkbookSemanticHash(Workbook, BaseSemanticHash, HashError));

	// Frozen P0 typed Change Set fixture입니다.
	FCFContentChangeSet ChangeSet;
	ChangeSet.ChangeSetId = TEXT("change.p001.fixture");
	ChangeSet.BaseWorkbookSemanticHash = BaseSemanticHash;
	ChangeSet.WorkbookSchemaVersion = Workbook.SchemaRevision;
	ChangeSet.Reason = TEXT("P0-01 typed ChangeSet contract verification");
	ChangeSet.DesignIntent = TEXT("HeavyCannon display metadata correction");
	ChangeSet.ExpectedPostSemanticHash = BaseSemanticHash;
	// Stable machine identity로 target을 지정하는 field update입니다.
	FCFContentChangeOperation Operation;
	Operation.OperationType = ECFContentChangeOperationType::UpdateField;
	Operation.SheetId = TEXT("Content.Main");
	Operation.ContentKey = Workbook.Records[0].Key;
	Operation.RowId = Workbook.Records[0].RowId;
	Operation.ColumnId = TEXT("DisplayName");
	Operation.Value = CFContentCoreTestsPrivate::MakeStringValue(TEXT("Heavy Cannon Mk2"));
	ChangeSet.Operations.Add(Operation);
	TestTrue(TEXT("Frozen typed ChangeSet contract must validate"), FCFContentSchemaValidator::ValidateChangeSet(ChangeSet, Issues));
	return true;
}

// VALUE/NONE/INHERIT와 canonical scalar/reference parse contract를 검증합니다.
bool FCFContentValueStatesTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// String descriptor입니다.
	FCFContentFieldDescriptor StringField =
		CFContentCoreTestsPrivate::MakeField(TEXT("Text"), ECFContentValueType::String, false, true);
	// Parse output value입니다.
	FCFContentValue Value;
	// Parse error text입니다.
	FString Error;

	TestTrue(TEXT("blank must parse as INHERIT"), FCFContentCanonicalizer::ParseCell(TEXT(""), StringField, Value, Error));
	TestTrue(TEXT("blank state must be INHERIT"), Value.State == ECFContentValueState::Inherit);

	TestTrue(TEXT("@none must parse when allowed"), FCFContentCanonicalizer::ParseCell(TEXT("@none"), StringField, Value, Error));
	TestTrue(TEXT("@none state must be NONE"), Value.State == ECFContentValueState::None);

	TestTrue(TEXT("@empty must parse as explicit empty String"), FCFContentCanonicalizer::ParseCell(TEXT("@empty"), StringField, Value, Error));
	TestTrue(TEXT("@empty state must be VALUE"), Value.State == ECFContentValueState::Value);
	TestTrue(TEXT("@empty String payload must be empty"), Value.StringValue.IsEmpty());

	TestTrue(TEXT("@@ must unescape one literal @"), FCFContentCanonicalizer::ParseCell(TEXT("@@literal"), StringField, Value, Error));
	TestEqual(TEXT("@@ literal payload"), Value.StringValue, FString(TEXT("@literal")));
	TestFalse(TEXT("Unescaped @ literal must fail closed"), FCFContentCanonicalizer::ParseCell(TEXT("@literal"), StringField, Value, Error));

	// Boolean descriptor입니다.
	FCFContentFieldDescriptor BoolField =
		CFContentCoreTestsPrivate::MakeField(TEXT("Enabled"), ECFContentValueType::Boolean, false, false);
	TestTrue(TEXT("Boolean true must parse"), FCFContentCanonicalizer::ParseCell(TEXT("true"), BoolField, Value, Error));
	TestTrue(TEXT("Boolean canonical payload"), Value.bBooleanValue);

	// Signed integer descriptor입니다.
	FCFContentFieldDescriptor SignedField =
		CFContentCoreTestsPrivate::MakeField(TEXT("Signed"), ECFContentValueType::SignedInteger, false, false);
	TestTrue(TEXT("Signed integer must parse"), FCFContentCanonicalizer::ParseCell(TEXT("-42"), SignedField, Value, Error));
	TestTrue(TEXT("Signed integer canonical payload"), Value.SignedIntegerValue == -42);

	// Unsigned integer descriptor입니다.
	FCFContentFieldDescriptor UnsignedField =
		CFContentCoreTestsPrivate::MakeField(TEXT("Unsigned"), ECFContentValueType::UnsignedInteger, false, false);
	TestTrue(TEXT("Unsigned integer must parse"), FCFContentCanonicalizer::ParseCell(TEXT("42"), UnsignedField, Value, Error));
	TestTrue(TEXT("Unsigned integer canonical payload"), Value.UnsignedIntegerValue == 42u);
	TestFalse(TEXT("Unsigned integer must reject negative value"), FCFContentCanonicalizer::ParseCell(TEXT("-1"), UnsignedField, Value, Error));

	// Float descriptor입니다.
	FCFContentFieldDescriptor FloatField =
		CFContentCoreTestsPrivate::MakeField(TEXT("Float"), ECFContentValueType::Float, false, false);
	TestTrue(TEXT("Float must parse"), FCFContentCanonicalizer::ParseCell(TEXT("3.25"), FloatField, Value, Error));
	TestTrue(TEXT("Float canonical payload"), FMath::IsNearlyEqual(static_cast<float>(Value.FloatingPointValue), 3.25f));

	// Double descriptor입니다.
	FCFContentFieldDescriptor DoubleField =
		CFContentCoreTestsPrivate::MakeField(TEXT("Double"), ECFContentValueType::Double, false, false);
	TestTrue(TEXT("Double must parse"), FCFContentCanonicalizer::ParseCell(TEXT("12.5"), DoubleField, Value, Error));
	TestTrue(TEXT("ASCII exponent must parse"), FCFContentCanonicalizer::ParseCell(TEXT("1.25e3"), DoubleField, Value, Error));
	TestTrue(TEXT("ASCII exponent canonical payload"), FMath::IsNearlyEqual(Value.FloatingPointValue, 1250.0));
	TestFalse(TEXT("Locale comma numeric must fail"), FCFContentCanonicalizer::ParseCell(TEXT("12,5"), DoubleField, Value, Error));
	TestFalse(TEXT("Trailing numeric junk must fail"), FCFContentCanonicalizer::ParseCell(TEXT("12.5x"), DoubleField, Value, Error));
	TestFalse(TEXT("Leading whitespace must fail"), FCFContentCanonicalizer::ParseCell(TEXT(" 12.5"), DoubleField, Value, Error));

	// Enum descriptor입니다.
	FCFContentFieldDescriptor EnumField =
		CFContentCoreTestsPrivate::MakeField(TEXT("Mode"), ECFContentValueType::Enum, false, false);
	TestTrue(TEXT("Enum stable token must parse"), FCFContentCanonicalizer::ParseCell(TEXT("Burst"), EnumField, Value, Error));
	TestEqual(TEXT("Enum canonical payload"), Value.StringValue, FString(TEXT("Burst")));

	// FName-style identifier descriptor입니다.
	FCFContentFieldDescriptor NameField =
		CFContentCoreTestsPrivate::MakeField(TEXT("FamilyId"), ECFContentValueType::NameId, false, false);
	TestTrue(TEXT("NameId stable token must parse"), FCFContentCanonicalizer::ParseCell(TEXT("  Heavy.Cannon  "), NameField, Value, Error));
	TestEqual(TEXT("NameId outer trim canonical payload"), Value.StringValue, FString(TEXT("Heavy.Cannon")));

	// FText literal semantic descriptor입니다.
	FCFContentFieldDescriptor TextField =
		CFContentCoreTestsPrivate::MakeField(TEXT("DisplayText"), ECFContentValueType::Text, false, true);
	TestTrue(TEXT("FText literal semantic must parse"), FCFContentCanonicalizer::ParseCell(TEXT("중화기"), TextField, Value, Error));
	TestTrue(TEXT("FText parsed semantic must be literal"), Value.TextValue.Kind == ECFContentTextKind::Literal);
	TestEqual(TEXT("FText literal source"), Value.TextValue.SourceString, FString(TEXT("중화기")));
	TestTrue(TEXT("FText @empty must parse"), FCFContentCanonicalizer::ParseCell(TEXT("@empty"), TextField, Value, Error));
	TestTrue(TEXT("FText @empty must be explicit VALUE"), Value.State == ECFContentValueState::Value && Value.TextValue.SourceString.IsEmpty());

	// Content reference descriptor입니다.
	FCFContentFieldDescriptor ContentRefField =
		CFContentCoreTestsPrivate::MakeField(TEXT("Target"), ECFContentValueType::ContentReference, false, true);
	TestTrue(TEXT("Content reference must parse"), FCFContentCanonicalizer::ParseCell(TEXT("Weapon:HeavyCannon"), ContentRefField, Value, Error));
	TestEqual(TEXT("Content reference type"), Value.ContentReference.ContentTypeId.Value, FString(TEXT("Weapon")));
	TestEqual(TEXT("Content reference id"), Value.ContentReference.ContentId, FString(TEXT("HeavyCannon")));

	// Resource reference descriptor입니다.
	FCFContentFieldDescriptor ResourceRefField =
		CFContentCoreTestsPrivate::MakeField(TEXT("Mesh"), ECFContentValueType::ResourceReference, false, true);
	TestTrue(TEXT("ResourceId must parse"), FCFContentCanonicalizer::ParseCell(TEXT("Mesh.Weapon.HeavyCannon"), ResourceRefField, Value, Error));
	TestFalse(TEXT("Raw ObjectPath must not be accepted as ResourceId"), FCFContentCanonicalizer::ParseCell(TEXT("/Game/Weapons/SM_Cannon.SM_Cannon"), ResourceRefField, Value, Error));
	return true;
}

// OrderedList/KeyedCollection/UnorderedSet의 semantic ordering/hash contract를 검증합니다.
bool FCFContentCollectionsTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Base canonical workbook입니다.
	const FCFContentWorkbookModel BaseWorkbook = CFContentCoreTestsPrivate::MakeFixtureWorkbook();
	// Physical child array order만 뒤집은 workbook입니다.
	FCFContentWorkbookModel ReorderedWorkbook = BaseWorkbook;
	for (TPair<FString, FCFContentCollection>& CollectionPair : ReorderedWorkbook.Records[0].Collections)
	{
		Algo::Reverse(CollectionPair.Value.Items);
	}

	// Base semantic hash입니다.
	FString BaseHash;
	// Reordered semantic hash입니다.
	FString ReorderedHash;
	// Hash error text입니다.
	FString Error;
	TestTrue(TEXT("Base collection workbook hash must succeed"), FCFContentSemanticHasher::BuildWorkbookSemanticHash(BaseWorkbook, BaseHash, Error));
	TestTrue(TEXT("Reordered collection workbook hash must succeed"), FCFContentSemanticHasher::BuildWorkbookSemanticHash(ReorderedWorkbook, ReorderedHash, Error));
	TestEqual(TEXT("Physical child array order must not change semantic hash"), ReorderedHash, BaseHash);

	// KeyedCollection/UnorderedSet의 hidden ChildItemId만 바꾼 workbook입니다.
	FCFContentWorkbookModel HiddenIdChangedWorkbook = BaseWorkbook;
	HiddenIdChangedWorkbook.Records[0].Collections[TEXT("Sockets")].Items[0].ChildItemId = TEXT("socket.hidden.changed");
	HiddenIdChangedWorkbook.Records[0].Collections[TEXT("Tags")].Items[0].ChildItemId = TEXT("tag.hidden.changed");
	// Hidden machine identity change semantic hash입니다.
	FString HiddenIdChangedHash;
	TestTrue(TEXT("Hidden child identity changed workbook must hash"), FCFContentSemanticHasher::BuildWorkbookSemanticHash(HiddenIdChangedWorkbook, HiddenIdChangedHash, Error));
	TestEqual(TEXT("KeyedCollection/UnorderedSet hidden ChildItemId must not change semantic hash"), HiddenIdChangedHash, BaseHash);

	// OrderedList explicit semantic order를 바꾼 workbook입니다.
	FCFContentWorkbookModel SemanticChangeWorkbook = BaseWorkbook;
	SemanticChangeWorkbook.Records[0].Collections[TEXT("Steps")].Items[0].Order = 2;
	// Semantic change hash입니다.
	FString SemanticChangeHash;
	TestTrue(TEXT("Changed ordered list must still hash"), FCFContentSemanticHasher::BuildWorkbookSemanticHash(SemanticChangeWorkbook, SemanticChangeHash, Error));
	TestNotEqual(TEXT("Explicit OrderedList order must change semantic hash"), SemanticChangeHash, BaseHash);
	return true;
}

// Presentation-only metadata와 physical order가 WorkbookSemanticHash에 영향을 주지 않는지 검증합니다.
bool FCFContentSemanticHashTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Base canonical workbook입니다.
	const FCFContentWorkbookModel BaseWorkbook = CFContentCoreTestsPrivate::MakeFixtureWorkbook();
	// Presentation/physical order만 변경한 workbook입니다.
	FCFContentWorkbookModel PresentationChangedWorkbook = BaseWorkbook;
	Algo::Reverse(PresentationChangedWorkbook.Sheets);
	Algo::Reverse(PresentationChangedWorkbook.Records);
	for (FCFContentSheetDescriptor& Sheet : PresentationChangedWorkbook.Sheets)
	{
		Algo::Reverse(Sheet.Fields);
		for (FCFContentFieldDescriptor& Field : Sheet.Fields)
		{
			Field.DisplayLabel = TEXT("한국어 Label 변경 - semantic 제외");
		}
	}

	// Base semantic hash입니다.
	FString BaseHash;
	// Presentation-changed semantic hash입니다.
	FString PresentationHash;
	// Hash error text입니다.
	FString Error;
	TestTrue(TEXT("Base semantic hash must succeed"), FCFContentSemanticHasher::BuildWorkbookSemanticHash(BaseWorkbook, BaseHash, Error));
	TestTrue(TEXT("Presentation changed semantic hash must succeed"), FCFContentSemanticHasher::BuildWorkbookSemanticHash(PresentationChangedWorkbook, PresentationHash, Error));
	TestEqual(TEXT("Presentation/physical order must be hash invariant"), PresentationHash, BaseHash);
	TestTrue(TEXT("Hash must use canonical sha256 format"), CFDACommonPrimitives::IsCanonicalSha256Fingerprint(BaseHash));

	// Actual semantic value를 변경한 workbook입니다.
	FCFContentWorkbookModel ValueChangedWorkbook = BaseWorkbook;
	ValueChangedWorkbook.Records[0].Fields[TEXT("DisplayName")] = CFContentCoreTestsPrivate::MakeStringValue(TEXT("Heavy Cannon Mk2"));
	// Changed semantic hash입니다.
	FString ChangedHash;
	TestTrue(TEXT("Value-changed semantic hash must succeed"), FCFContentSemanticHasher::BuildWorkbookSemanticHash(ValueChangedWorkbook, ChangedHash, Error));
	TestNotEqual(TEXT("Semantic value change must change hash"), ChangedHash, BaseHash);

	// Canonical unit contract를 변경한 workbook입니다.
	FCFContentWorkbookModel UnitChangedWorkbook = BaseWorkbook;
	UnitChangedWorkbook.Sheets[0].Fields[1].CanonicalUnitId = TEXT("lb");
	// Unit-changed semantic hash입니다.
	FString UnitChangedHash;
	TestTrue(TEXT("Unit-changed semantic hash must succeed"), FCFContentSemanticHasher::BuildWorkbookSemanticHash(UnitChangedWorkbook, UnitChangedHash, Error));
	TestNotEqual(TEXT("Canonical unit change must change semantic hash"), UnitChangedHash, BaseHash);

	// Physical filename이 아닌 logical WorkbookSourceId를 변경한 workbook입니다.
	FCFContentWorkbookModel SourceChangedWorkbook = BaseWorkbook;
	SourceChangedWorkbook.WorkbookSourceId = TEXT("CarFight.Content.Secondary");
	// Source identity changed semantic hash입니다.
	FString SourceChangedHash;
	TestTrue(TEXT("WorkbookSourceId-changed semantic hash must succeed"), FCFContentSemanticHasher::BuildWorkbookSemanticHash(SourceChangedWorkbook, SourceChangedHash, Error));
	TestNotEqual(TEXT("WorkbookSourceId change must change semantic hash"), SourceChangedHash, BaseHash);
	return true;
}

// Duplicate identity, unclassified ownership, invalid schema와 dependency cycle을 fail-closed 검증합니다.
bool FCFContentFailClosedTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Duplicate ContentKey workbook입니다.
	FCFContentWorkbookModel DuplicateWorkbook = CFContentCoreTestsPrivate::MakeFixtureWorkbook();
	// Source array self-aliasing을 피하기 위한 duplicate record value copy입니다.
	FCFContentRecord DuplicateRecord = DuplicateWorkbook.Records[0];
	DuplicateRecord.RowId = TEXT("row.weapon.duplicate");
	DuplicateWorkbook.Records.Add(DuplicateRecord);
	// Duplicate validation issues입니다.
	TArray<FCFContentValidationIssue> Issues;
	TestFalse(TEXT("Duplicate ContentKey must fail closed"), FCFContentSchemaValidator::ValidateWorkbook(DuplicateWorkbook, Issues));

	// Unclassified ownership workbook입니다.
	FCFContentWorkbookModel UnclassifiedWorkbook = CFContentCoreTestsPrivate::MakeFixtureWorkbook();
	UnclassifiedWorkbook.Sheets[0].Fields[0].Ownership = ECFContentFieldOwnership::Unspecified;
	TestFalse(TEXT("Unclassified ownership must fail closed"), FCFContentSchemaValidator::ValidateWorkbook(UnclassifiedWorkbook, Issues));

	// Duplicate ColumnId workbook입니다.
	FCFContentWorkbookModel DuplicateColumnWorkbook = CFContentCoreTestsPrivate::MakeFixtureWorkbook();
	// Source array self-aliasing을 피하기 위한 duplicate field value copy입니다.
	const FCFContentFieldDescriptor DuplicateField = DuplicateColumnWorkbook.Sheets[0].Fields[0];
	DuplicateColumnWorkbook.Sheets[0].Fields.Add(DuplicateField);
	TestFalse(TEXT("Duplicate ColumnId must fail closed"), FCFContentSchemaValidator::ValidateWorkbook(DuplicateColumnWorkbook, Issues));

	// Duplicate SheetId workbook입니다.
	FCFContentWorkbookModel DuplicateSheetWorkbook = CFContentCoreTestsPrivate::MakeFixtureWorkbook();
	DuplicateSheetWorkbook.Sheets[1].SheetId = DuplicateSheetWorkbook.Sheets[0].SheetId;
	TestFalse(TEXT("Duplicate SheetId must fail closed"), FCFContentSchemaValidator::ValidateWorkbook(DuplicateSheetWorkbook, Issues));

	// Duplicate ChildItemId workbook입니다.
	FCFContentWorkbookModel DuplicateChildWorkbook = CFContentCoreTestsPrivate::MakeFixtureWorkbook();
	// Reallocation 중 self-aliasing을 피하기 위한 duplicate source value copy입니다.
	const FCFContentCollectionItem DuplicateChildItem =
		DuplicateChildWorkbook.Records[0].Collections[TEXT("Tags")].Items[0];
	DuplicateChildWorkbook.Records[0].Collections[TEXT("Tags")].Items.Add(DuplicateChildItem);
	TestFalse(TEXT("Duplicate ChildItemId must fail closed"), FCFContentSchemaValidator::ValidateWorkbook(DuplicateChildWorkbook, Issues));

	// UnorderedSet canonical value만 중복시키고 hidden ChildItemId는 다르게 만든 workbook입니다.
	FCFContentWorkbookModel DuplicateSetValueWorkbook = CFContentCoreTestsPrivate::MakeFixtureWorkbook();
	FCFContentCollectionItem DuplicateSetValueItem = DuplicateSetValueWorkbook.Records[0].Collections[TEXT("Tags")].Items[0];
	DuplicateSetValueItem.ChildItemId = TEXT("tag.duplicate.value");
	DuplicateSetValueWorkbook.Records[0].Collections[TEXT("Tags")].Items.Add(DuplicateSetValueItem);
	TestFalse(TEXT("Duplicate UnorderedSet canonical value must fail closed"), FCFContentSchemaValidator::ValidateWorkbook(DuplicateSetValueWorkbook, Issues));

	// Top-level RowId를 별도 identity처럼 바꾼 workbook입니다.
	FCFContentWorkbookModel InvalidRowIdWorkbook = CFContentCoreTestsPrivate::MakeFixtureWorkbook();
	InvalidRowIdWorkbook.Records[0].RowId = TEXT("row.wrong");
	TestFalse(TEXT("RowId different from canonical ContentKey token must fail closed"), FCFContentSchemaValidator::ValidateWorkbook(InvalidRowIdWorkbook, Issues));

	// Invalid BaseWorkbookSemanticHash를 가진 typed Change Set입니다.
	FCFContentChangeSet InvalidChangeSet;
	InvalidChangeSet.ChangeSetId = TEXT("change.invalid.base");
	InvalidChangeSet.BaseWorkbookSemanticHash = TEXT("sha256:not-valid");
	InvalidChangeSet.WorkbookSchemaVersion = 1;
	FCFContentChangeOperation InvalidChangeOperation;
	InvalidChangeOperation.OperationType = ECFContentChangeOperationType::UpdateField;
	InvalidChangeOperation.SheetId = TEXT("Content.Main");
	InvalidChangeOperation.ContentKey = DuplicateSetValueWorkbook.Records[0].Key;
	InvalidChangeOperation.RowId = InvalidChangeOperation.ContentKey.ToStableString();
	InvalidChangeOperation.ColumnId = TEXT("DisplayName");
	InvalidChangeSet.Operations.Add(InvalidChangeOperation);
	TestFalse(TEXT("Invalid ChangeSet concurrency hash must fail closed"), FCFContentSchemaValidator::ValidateChangeSet(InvalidChangeSet, Issues));

	// Valid graph source workbook입니다.
	const FCFContentWorkbookModel GraphWorkbook = CFContentCoreTestsPrivate::MakeFixtureWorkbook();
	// A -> B dependency edge입니다.
	FCFContentDependencyEdge ForwardEdge;
	ForwardEdge.From = GraphWorkbook.Records[0].Key;
	ForwardEdge.To = GraphWorkbook.Records[1].Key;
	ForwardEdge.FieldPath = TEXT("Target");
	// B -> A dependency edge입니다.
	FCFContentDependencyEdge BackEdge;
	BackEdge.From = GraphWorkbook.Records[1].Key;
	BackEdge.To = GraphWorkbook.Records[0].Key;
	BackEdge.FieldPath = TEXT("Target");
	// Cycle edge list입니다.
	const TArray<FCFContentDependencyEdge> CycleEdges{ForwardEdge, BackEdge};
	TestFalse(TEXT("Dependency cycle must fail closed"), FCFContentSchemaValidator::ValidateDependencyEdges(GraphWorkbook, CycleEdges, Issues));
	return true;
}

// ExternalReadOnly가 Catalog graph에는 존재하지만 CCAS mutation target이 아닌지 검증합니다.
bool FCFContentExternalReadOnlyTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Canonical fixture workbook입니다.
	const FCFContentWorkbookModel Workbook = CFContentCoreTestsPrivate::MakeFixtureWorkbook();
	TestTrue(TEXT("Managed record must be mutable by management-state gate"), FCFContentSchemaValidator::CanMutateRecord(Workbook.Records[0]));
	TestFalse(TEXT("ExternalReadOnly record must never be mutable by CCAS gate"), FCFContentSchemaValidator::CanMutateRecord(Workbook.Records[1]));

	// ExternalReadOnly까지 포함한 whole catalog validation issues입니다.
	TArray<FCFContentValidationIssue> Issues;
	TestTrue(TEXT("ExternalReadOnly record must remain a valid catalog node"), FCFContentSchemaValidator::ValidateWorkbook(Workbook, Issues));
	return true;
}

// Disposable old-schema -> new-schema migration preview가 deterministic한지 검증합니다.
bool FCFContentMigrationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Old schema fixture입니다.
	const FCFContentWorkbookModel OldWorkbook = CFContentCoreTestsPrivate::MakeFixtureWorkbook();
	// Revision 1 -> 2 migration rule입니다.
	FCFContentMigrationRule Rule;
	Rule.FromRevision = 1;
	Rule.ToRevision = 2;
	Rule.ColumnRenames.Add(TEXT("DisplayName"), TEXT("DisplayLabel"));

	// First deterministic migration preview입니다.
	FCFContentMigrationPreview FirstPreview;
	// Second deterministic migration preview입니다.
	FCFContentMigrationPreview SecondPreview;
	// Migration error text입니다.
	FString Error;
	TestTrue(TEXT("First migration preview must succeed"), FCFContentSchemaMigration::BuildPreview(OldWorkbook, Rule, FirstPreview, Error));
	TestTrue(TEXT("Second migration preview must succeed"), FCFContentSchemaMigration::BuildPreview(OldWorkbook, Rule, SecondPreview, Error));
	TestEqual(TEXT("Migration target semantic hash must be deterministic"), SecondPreview.TargetSemanticHash, FirstPreview.TargetSemanticHash);
	TestNotEqual(TEXT("Schema migration must change semantic hash"), FirstPreview.TargetSemanticHash, FirstPreview.SourceSemanticHash);
	TestTrue(TEXT("Migrated record must use new ColumnId"), FirstPreview.MigratedModel.Records[0].Fields.Contains(TEXT("DisplayLabel")));
	TestFalse(TEXT("Migrated record must remove old ColumnId"), FirstPreview.MigratedModel.Records[0].Fields.Contains(TEXT("DisplayName")));
	TestTrue(TEXT("Source fixture must remain unchanged"), OldWorkbook.Records[0].Fields.Contains(TEXT("DisplayName")));
	return true;
}

// Provider schema registry가 ContentType exact1과 duplicate fail-closed를 보장하는지 검증합니다.
bool FCFContentProviderRegistryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Registry under test입니다.
	FCFContentProviderRegistry Registry;
	// Non-mutating provider fixture입니다.
	const TSharedRef<ICFContentProvider> Provider = MakeShared<CFContentCoreTestsPrivate::FFixtureProvider>();
	// Registry error text입니다.
	FString Error;
	TestTrue(TEXT("First provider registration must succeed"), Registry.RegisterProvider(Provider, Error));
	TestEqual(TEXT("Registry provider count"), Registry.Num(), 1);
	TestTrue(TEXT("Exact ContentType lookup must find provider"), Registry.FindProvider(CFContentCoreTestsPrivate::FixtureContentType) != nullptr);
	TestFalse(TEXT("Duplicate ContentType provider must fail closed"), Registry.RegisterProvider(Provider, Error));
	TestEqual(TEXT("Duplicate registration must not change count"), Registry.Num(), 1);
	TestTrue(TEXT("Registry self-validation must succeed"), Registry.ValidateRegistry(Error));

	// P0-01 unapproved mutation plan입니다.
	FCFContentReviewedMutationPlan Plan;
	Plan.Key.ContentTypeId = CFContentCoreTestsPrivate::FixtureContentType;
	Plan.Key.ContentId = TEXT("HeavyCannon");
	Plan.bApproved = false;
	// Product apply seam result입니다.
	FCFContentApplyResult ApplyResult;
	TestFalse(TEXT("P0-01 fake provider must not open Product Apply"), Provider->ApplyReviewed(Plan, ApplyResult));
	TestFalse(TEXT("P0-01 fake provider must report mutation0"), ApplyResult.bApplied);
	return true;
}

#endif
