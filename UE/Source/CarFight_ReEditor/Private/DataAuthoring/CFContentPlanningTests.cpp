// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentPlanningTests.cpp
// Version: v1.2.3
// Date: 2026-10-03
// Description: CF-FQ-058 CCAS-P0-06 focused Automation exact7 + USER Acceptance disposable Weapon Roster review fixture입니다.
// Changelog:
// - v1.2.3: ApprovalBinding에 approved fingerprint 문자열을 유지한 ReviewPackage payload tamper fail-closed 회귀를 추가.
// - v1.2.2: existing anchor의 current Product identity를 HeavyCannon / RocketLauncher 그대로 유지해 rename 오해를 제거.
// - v1.2.1: USER review fixture를 실제 Roster 기준선에 맞춰 Heavy/Salvo existing anchor update + 신규 exact6 Add로 교정.
// - v1.2.0: USER Accepted Weapon Roster exact8을 AI Change Proposal -> Review Package로 실제 통과시키고 사람이 읽을 review marker를 출력하는 acceptance fixture를 추가.
// - v1.1.0: BulkSemanticPreflight에 unknown SheetId planning operation fail-closed 회귀를 추가.
// - v1.0.0: Snapshot, Planning/Lifecycle, semantic preflight, retire reference guard, stale/approval, no-direct-DA-write를 검증.
// Migration:
// - disposable in-memory fixture만 사용하며 Product DataAsset/Workbook 파일을 생성·수정·저장하지 않습니다.

#include "DataAuthoring/CFContentPlanning.h"

#include "DataAuthoring/CFContentCore.h"
#include "DataAuthoring/CFDACommonPrimitives.h"
#include "Misc/AutomationTest.h"

namespace CFContentPlanningTestsPrivate
{
	/** Test field descriptor를 생성합니다. */
	FCFContentFieldDescriptor MakeField(
		const FString& ColumnId,
		const ECFContentValueType ValueType,
		const ECFContentFieldOwnership Ownership,
		const bool bRequired = false,
		const bool bAllowNone = false,
		const FString& FieldDomainId = FString())
	{
		// 생성할 field descriptor입니다.
		FCFContentFieldDescriptor Field;
		Field.ColumnId = ColumnId;
		Field.DisplayLabel = ColumnId;
		Field.ValueType = ValueType;
		Field.Ownership = Ownership;
		Field.bRequired = bRequired;
		Field.bAllowNone = bAllowNone;
		Field.FieldDomainId = FieldDomainId;
		if (Ownership == ECFContentFieldOwnership::ExternalManaged)
		{
			Field.ExternalOwnerId = TEXT("Test.External");
		}
		return Field;
	}

	/** Canonical String VALUE를 생성합니다. */
	FCFContentValue MakeStringValue(const FString& Value)
	{
		// 생성할 value입니다.
		FCFContentValue Result;
		Result.Type = ECFContentValueType::String;
		Result.State = ECFContentValueState::Value;
		Result.StringValue = Value;
		return Result;
	}

	/** Canonical Double VALUE를 생성합니다. */
	FCFContentValue MakeDoubleValue(const double Value)
	{
		// 생성할 value입니다.
		FCFContentValue Result;
		Result.Type = ECFContentValueType::Double;
		Result.State = ECFContentValueState::Value;
		Result.FloatingPointValue = Value;
		return Result;
	}

	/** Optional ContentReference VALUE/NONE을 생성합니다. */
	FCFContentValue MakeReferenceValue(const FString& TargetContentId)
	{
		// 생성할 value입니다.
		FCFContentValue Result;
		Result.Type = ECFContentValueType::ContentReference;
		if (TargetContentId.IsEmpty())
		{
			Result.State = ECFContentValueState::None;
			return Result;
		}

		Result.State = ECFContentValueState::Value;
		Result.ContentReference.ContentTypeId.Value = TEXT("Weapon");
		Result.ContentReference.ContentId = TargetContentId;
		return Result;
	}

	/** Optional ResourceReference NONE을 생성합니다. */
	FCFContentValue MakeEmptyResourceValue()
	{
		// 생성할 value입니다.
		FCFContentValue Result;
		Result.Type = ECFContentValueType::ResourceReference;
		Result.State = ECFContentValueState::None;
		return Result;
	}

	/** P0-06 disposable schema를 생성합니다. */
	TArray<FCFContentSheetDescriptor> MakeSchema()
	{
		// Primary sheet입니다.
		FCFContentSheetDescriptor MainSheet;
		MainSheet.SheetId = TEXT("Content.Main");
		MainSheet.SchemaRevision = 1;
		MainSheet.ContentTypeId.Value = TEXT("Weapon");
		MainSheet.Fields.Add(MakeField(TEXT("DisplayName"), ECFContentValueType::String, ECFContentFieldOwnership::CCASManaged, true));
		MainSheet.Fields.Add(MakeField(TEXT("Power"), ECFContentValueType::Double, ECFContentFieldOwnership::CCASManaged, true, false, TEXT("Weapon.Balance")));
		MainSheet.Fields.Add(MakeField(TEXT("Target"), ECFContentValueType::ContentReference, ECFContentFieldOwnership::CCASManaged, false, true));
		MainSheet.Fields.Add(MakeField(TEXT("Resource"), ECFContentValueType::ResourceReference, ECFContentFieldOwnership::CCASManaged, false, true));
		MainSheet.Fields.Add(MakeField(TEXT("ExternalNote"), ECFContentValueType::String, ECFContentFieldOwnership::ExternalManaged));

		// Child sheet입니다.
		FCFContentSheetDescriptor ChildSheet;
		ChildSheet.SheetId = TEXT("Content.Children");
		ChildSheet.SchemaRevision = 1;
		ChildSheet.ContentTypeId.Value = TEXT("Weapon");
		ChildSheet.ParentSheetId = TEXT("Content.Main");
		ChildSheet.CollectionId = TEXT("Children");
		ChildSheet.CollectionKind = ECFContentCollectionKind::OrderedList;
		ChildSheet.Fields.Add(MakeField(TEXT("Value"), ECFContentValueType::SignedInteger, ECFContentFieldOwnership::CCASManaged, true));

		// 전체 schema입니다.
		TArray<FCFContentSheetDescriptor> Sheets;
		Sheets.Add(MoveTemp(MainSheet));
		Sheets.Add(MoveTemp(ChildSheet));
		return Sheets;
	}

	/** Disposable canonical record를 생성합니다. */
	FCFContentRecord MakeRecord(
		const FString& ContentId,
		const double Power,
		const FString& TargetContentId,
		const ECFContentManagementState ManagementState = ECFContentManagementState::Managed)
	{
		// 생성할 record입니다.
		FCFContentRecord Record;
		Record.Key.ContentTypeId.Value = TEXT("Weapon");
		Record.Key.ContentId = ContentId;
		Record.RowId = Record.Key.ToStableString();
		Record.ManagementState = ManagementState;
		Record.LifecycleState = ECFContentLifecycleState::Active;
		Record.Fields.Add(TEXT("DisplayName"), MakeStringValue(ContentId));
		Record.Fields.Add(TEXT("Power"), MakeDoubleValue(Power));
		Record.Fields.Add(TEXT("Target"), MakeReferenceValue(TargetContentId));
		Record.Fields.Add(TEXT("Resource"), MakeEmptyResourceValue());
		Record.Fields.Add(TEXT("ExternalNote"), MakeStringValue(TEXT("external")));
		return Record;
	}

	/** Baseline Workbook을 생성합니다. */
	FCFContentWorkbookModel MakeWorkbook()
	{
		// 생성할 workbook입니다.
		FCFContentWorkbookModel Workbook;
		Workbook.WorkbookSourceId = TEXT("Test.P006.Workbook");
		Workbook.SchemaRevision = 1;
		Workbook.Sheets = MakeSchema();
		Workbook.Records.Add(MakeRecord(TEXT("Base"), 10.0, TEXT("")));
		Workbook.Records.Add(MakeRecord(TEXT("Consumer"), 3.0, TEXT("Base")));
		Workbook.Records.Add(MakeRecord(TEXT("Spare"), 2.0, TEXT("")));
		Workbook.Records.Add(MakeRecord(TEXT("External"), 1.0, TEXT(""), ECFContentManagementState::ExternalReadOnly));
		return Workbook;
	}

	/** Fixture 문자열을 canonical SHA-256으로 변환합니다. */
	FString HashText(const FString& Text)
	{
		// UTF-8 source입니다.
		FTCHARToUTF8 Utf8(*Text);
		// Hash bytes입니다.
		TArray<uint8> Bytes;
		Bytes.Append(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());

		// Hash result입니다.
		FString Fingerprint;
		// Hash error입니다.
		FString Error;
		return CFDACommonPrimitives::HashCanonicalBytes(Bytes, Fingerprint, Error)
			? Fingerprint
			: FString();
	}

	/** Provider-owned gameplay payload signature를 생성합니다. */
	FString BuildManagedSignature(const FCFContentRecord& Record)
	{
		// Signature입니다.
		FString Signature = Record.Key.ToStableString();

		// DisplayName입니다.
		const FCFContentValue* DisplayName = Record.Fields.Find(TEXT("DisplayName"));
		// Power입니다.
		const FCFContentValue* Power = Record.Fields.Find(TEXT("Power"));
		// Target입니다.
		const FCFContentValue* Target = Record.Fields.Find(TEXT("Target"));
		// Resource입니다.
		const FCFContentValue* Resource = Record.Fields.Find(TEXT("Resource"));

		Signature += TEXT("|Name=") + (DisplayName != nullptr ? DisplayName->StringValue : TEXT("<missing>"));
		Signature += TEXT("|Power=") + (Power != nullptr ? FString::Printf(TEXT("%.17g"), Power->FloatingPointValue) : TEXT("<missing>"));
		Signature += TEXT("|Target=") + (Target != nullptr && Target->State == ECFContentValueState::Value ? Target->ContentReference.ToStableString() : TEXT("<none>"));
		Signature += TEXT("|Resource=") + (Resource != nullptr && Resource->State == ECFContentValueState::Value ? Resource->StringValue : TEXT("<none>"));
		return Signature;
	}

	/** Provider-owned gameplay payload fingerprint를 생성합니다. */
	FString BuildManagedFingerprint(const FCFContentRecord& Record)
	{
		return HashText(BuildManagedSignature(Record));
	}

	/** Disposable Workbook adapter입니다. */
	class FMemoryWorkbookAdapter final : public ICFContentWorkbookAdapter
	{
	public:
		/** Workbook을 설정합니다. */
		explicit FMemoryWorkbookAdapter(const FCFContentWorkbookModel& InWorkbook)
			: Workbook(InWorkbook)
		{
		}

		/** Test adapter metadata를 반환합니다. */
		virtual FCFWorkbookAdapterInfo DescribeAdapter() const override
		{
			// Adapter metadata입니다.
			FCFWorkbookAdapterInfo Info;
			Info.AdapterId = TEXT("Test.P006.Memory");
			Info.AdapterVersion = TEXT("1.0");
			return Info;
		}

		/** In-memory Workbook을 반환합니다. */
		virtual bool ReadWorkbook(const FString& WorkbookPath, FCFContentWorkbookModel& OutWorkbook, FString& OutError) override
		{
			if (WorkbookPath.IsEmpty())
			{
				OutError = TEXT("Empty path.");
				return false;
			}
			OutError.Reset();
			OutWorkbook = Workbook;
			return true;
		}

		/** Persistent write를 감시하고 차단합니다. */
		virtual bool WriteStagedWorkbook(
			const FString& BaseWorkbookPath,
			const FString& StagedWorkbookPath,
			const FCFContentWorkbookModel& ReviewedWorkbook,
			FString& OutError) override
		{
			(void)BaseWorkbookPath;
			(void)StagedWorkbookPath;
			(void)ReviewedWorkbook;
			++WriteCount;
			OutError = TEXT("No write.");
			return false;
		}

		/** Disk reopen을 감시하고 차단합니다. */
		virtual bool ReopenWorkbook(const FString& StagedWorkbookPath, FCFContentWorkbookModel& OutWorkbook, FString& OutError) override
		{
			(void)StagedWorkbookPath;
			(void)OutWorkbook;
			++ReopenCount;
			OutError = TEXT("No reopen.");
			return false;
		}

		// Source Workbook입니다.
		FCFContentWorkbookModel Workbook;

		// Write 호출 수입니다.
		int32 WriteCount = 0;

		// Reopen 호출 수입니다.
		int32 ReopenCount = 0;
	};

	/** Disposable provider입니다. */
	class FMemoryContentProvider final : public ICFContentProvider
	{
	public:
		/** Schema/current fixture를 설정합니다. */
		FMemoryContentProvider(
			const TArray<FCFContentSheetDescriptor>& InSchema,
			const TArray<FCFContentRecord>& InCurrentRecords)
			: Schema(InSchema)
		{
			for (const FCFContentRecord& Record : InCurrentRecords)
			{
				CurrentRecordsByKey.Add(Record.Key.ToStableString(), Record);
			}
		}

		/** Stable schema를 반환합니다. */
		virtual bool DescribeSchema(FCFContentProviderDescriptor& OutDescriptor, FString& OutError) const override
		{
			OutError.Reset();
			OutDescriptor = FCFContentProviderDescriptor();
			OutDescriptor.ProviderId = TEXT("Test.P006.Provider");
			OutDescriptor.ContentTypeId.Value = TEXT("Weapon");
			OutDescriptor.SchemaRevision = 1;
			OutDescriptor.Sheets = Schema;
			return true;
		}

		/** Current Product fixture를 import합니다. */
		virtual bool ImportExisting(FCFContentImportResult& OutImport, FString& OutError) const override
		{
			OutError.Reset();
			OutImport = FCFContentImportResult();
			for (const TPair<FString, FCFContentRecord>& Pair : CurrentRecordsByKey)
			{
				OutImport.Records.Add(Pair.Value);
			}
			return true;
		}

		/** Exact current snapshot을 반환합니다. */
		virtual bool BuildCurrentSnapshot(const FCFContentKey& Key, FCFContentSnapshot& OutSnapshot, FString& OutError) const override
		{
			OutError.Reset();

			// Current record입니다.
			const FCFContentRecord* Record = CurrentRecordsByKey.Find(Key.ToStableString());
			if (Record == nullptr)
			{
				OutError = TEXT("Current record not found.");
				return false;
			}

			OutSnapshot = FCFContentSnapshot();
			OutSnapshot.Key = Key;
			OutSnapshot.Record = *Record;
			OutSnapshot.ReadbackFingerprint = BuildManagedFingerprint(*Record);
			return true;
		}

		/** Required Power invariant를 검증합니다. */
		virtual bool ValidateRecord(const FCFContentRecord& Record, TArray<FCFContentValidationIssue>& OutIssues) const override
		{
			OutIssues.Reset();

			// Power value입니다.
			const FCFContentValue* Power = Record.Fields.Find(TEXT("Power"));
			if (Power == nullptr || Power->State != ECFContentValueState::Value || Power->FloatingPointValue < 0.0)
			{
				// Blocking issue입니다.
				FCFContentValidationIssue Issue;
				Issue.Code = TEXT("InvalidPower");
				Issue.Path = Record.Key.ToStableString() + TEXT(".Power");
				Issue.Message = TEXT("Power VALUE is required.");
				OutIssues.Add(MoveTemp(Issue));
				return false;
			}
			return true;
		}

		/** Target dependency edge를 생성합니다. */
		virtual bool BuildDependencyEdges(const FCFContentRecord& Record, TArray<FCFContentDependencyEdge>& OutEdges, FString& OutError) const override
		{
			OutEdges.Reset();
			OutError.Reset();

			// Target reference입니다.
			const FCFContentValue* Target = Record.Fields.Find(TEXT("Target"));
			if (Target != nullptr && Target->State == ECFContentValueState::Value)
			{
				// Dependency edge입니다.
				FCFContentDependencyEdge Edge;
				Edge.From = Record.Key;
				Edge.To = Target->ContentReference;
				Edge.FieldPath = TEXT("Target");
				OutEdges.Add(MoveTemp(Edge));
			}
			return true;
		}

		/** Provider gameplay payload diff를 생성합니다. */
		virtual bool BuildDiff(
			const FCFContentSnapshot& Current,
			const FCFContentRecord& Desired,
			FCFContentDiff& OutDiff,
			FString& OutError) const override
		{
			OutError.Reset();
			OutDiff = FCFContentDiff();
			OutDiff.Key = Desired.Key;
			OutDiff.CurrentFingerprint = BuildManagedFingerprint(Current.Record);
			OutDiff.DesiredFingerprint = BuildManagedFingerprint(Desired);
			if (!OutDiff.CurrentFingerprint.Equals(OutDiff.DesiredFingerprint, ESearchCase::CaseSensitive))
			{
				OutDiff.ChangedPaths.Add(TEXT("$managedPayload"));
			}
			return Current.Key == Desired.Key;
		}

		/** Interface completeness용 reviewed plan을 생성합니다. */
		virtual bool BuildReviewedMutationPlan(
			const FCFContentDiff& Diff,
			const FString& BaseWorkbookSemanticHash,
			FCFContentReviewedMutationPlan& OutPlan,
			FString& OutError) const override
		{
			OutError.Reset();
			OutPlan = FCFContentReviewedMutationPlan();
			OutPlan.Key = Diff.Key;
			OutPlan.BaseWorkbookSemanticHash = BaseWorkbookSemanticHash;
			OutPlan.DesiredFingerprint = Diff.DesiredFingerprint;
			return true;
		}

		/** Product Apply 호출을 감시하고 차단합니다. */
		virtual bool ApplyReviewed(const FCFContentReviewedMutationPlan& ReviewedPlan, FCFContentApplyResult& OutResult) override
		{
			(void)ReviewedPlan;
			++ApplyCallCount;
			OutResult = FCFContentApplyResult();
			OutResult.Error = TEXT("No Product writer.");
			return false;
		}

		/** Current provider fingerprint를 반환합니다. */
		virtual bool ReadbackFingerprint(const FCFContentKey& Key, FString& OutFingerprint, FString& OutError) const override
		{
			OutError.Reset();

			// Current record입니다.
			const FCFContentRecord* Record = CurrentRecordsByKey.Find(Key.ToStableString());
			if (Record == nullptr)
			{
				OutError = TEXT("Current record not found.");
				return false;
			}
			OutFingerprint = BuildManagedFingerprint(*Record);
			return true;
		}

		// Provider schema입니다.
		TArray<FCFContentSheetDescriptor> Schema;

		// Current records입니다.
		TMap<FString, FCFContentRecord> CurrentRecordsByKey;

		// Product Apply call count입니다.
		int32 ApplyCallCount = 0;
	};

	/** Fixture 전체 context입니다. */
	struct FFixtureContext
	{
		/** 기본 focused fixture의 Baseline Compiler + Snapshot을 구성합니다. */
		bool Initialize(FString& OutError)
		{
			return InitializeWithWorkbook(MakeWorkbook(), OutError);
		}

		/** 지정한 disposable Workbook으로 Baseline Compiler + Snapshot을 구성합니다. */
		bool InitializeWithWorkbook(
			const FCFContentWorkbookModel& InWorkbook,
			FString& OutError)
		{
			OutError.Reset();
			Workbook = InWorkbook;

			// Disposable provider입니다.
			TSharedRef<FMemoryContentProvider> ProviderRef = MakeShared<FMemoryContentProvider>(Workbook.Sheets, Workbook.Records);
			if (!Registry.RegisterProvider(ProviderRef, OutError))
			{
				return false;
			}
			Provider = ProviderRef;

			// Baseline adapter입니다.
			FMemoryWorkbookAdapter Adapter(Workbook);
			if (!FCFContentCompiler::CompilePreview(TEXT("Memory://P006/Base"), Adapter, Registry, CompileResult))
			{
				OutError = CompileResult.Error;
				return false;
			}
			if (Adapter.WriteCount != 0 || Adapter.ReopenCount != 0)
			{
				OutError = TEXT("Compiler unexpectedly wrote Workbook.");
				return false;
			}

			return FCFContentPlanningService::BuildCatalogSnapshot(
				CompileResult,
				Registry,
				ResourceCatalog,
				PickerRegistry,
				Snapshot,
				OutError);
		}

		// Canonical Workbook입니다.
		FCFContentWorkbookModel Workbook;

		// Provider registry입니다.
		FCFContentProviderRegistry Registry;

		// Provider instance입니다.
		TSharedPtr<FMemoryContentProvider> Provider;

		// Empty Resource Catalog입니다.
		FCFResourceCatalog ResourceCatalog;

		// Empty Picker Registry입니다.
		FCFResourcePickerRegistry PickerRegistry;

		// Baseline Compiler result입니다.
		FCFContentCompileResult CompileResult;

		// Baseline immutable snapshot입니다.
		FCFContentCatalogSnapshot Snapshot;
	};

	/** Common operation target을 생성합니다. */
	FCFContentChangeOperation MakeOperation(
		const ECFContentChangeOperationType OperationType,
		const FString& ContentId,
		const FString& SheetId = TEXT("Content.Main"))
	{
		// 생성할 operation입니다.
		FCFContentChangeOperation Operation;
		Operation.OperationType = OperationType;
		Operation.SheetId = SheetId;
		Operation.ContentKey.ContentTypeId.Value = TEXT("Weapon");
		Operation.ContentKey.ContentId = ContentId;
		Operation.RowId = Operation.ContentKey.ToStableString();
		return Operation;
	}

	/** Snapshot-bound proposal을 생성합니다. */
	FCFContentChangeProposal MakeProposal(
		const FCFContentCatalogSnapshot& Snapshot,
		const FString& ChangeSetId,
		const TArray<FCFContentChangeOperation>& Operations)
	{
		// 생성할 proposal입니다.
		FCFContentChangeProposal Proposal;
		Proposal.ChangeSet.ChangeSetId = ChangeSetId;
		Proposal.ChangeSet.BaseWorkbookSemanticHash = Snapshot.WorkbookSemanticHash;
		Proposal.ChangeSet.BaseCatalogSnapshotFingerprint = Snapshot.SnapshotFingerprint;
		Proposal.ChangeSet.WorkbookSchemaVersion = Snapshot.WorkbookSchemaVersion;
		Proposal.ChangeSet.Reason = TEXT("Focused test.");
		Proposal.ChangeSet.DesignIntent = TEXT("Focused test intent.");
		Proposal.ChangeSet.Operations = Operations;
		Proposal.BaseCatalogSnapshotFingerprint = Snapshot.SnapshotFingerprint;

		// Record별 operation indexes입니다.
		TMap<FString, TArray<int32>> OperationIndexesByRecord;
		for (int32 OperationIndex = 0; OperationIndex < Operations.Num(); ++OperationIndex)
		{
			OperationIndexesByRecord.FindOrAdd(Operations[OperationIndex].ContentKey.ToStableString()).Add(OperationIndex);
		}

		// Stable rationale keys입니다.
		TArray<FString> RationaleKeys;
		OperationIndexesByRecord.GetKeys(RationaleKeys);
		RationaleKeys.Sort();
		for (const FString& RationaleKey : RationaleKeys)
		{
			// Related indexes입니다.
			const TArray<int32>* RelatedIndexes = OperationIndexesByRecord.Find(RationaleKey);
			if (RelatedIndexes == nullptr || RelatedIndexes->IsEmpty())
			{
				continue;
			}

			// Record rationale입니다.
			FCFContentRecordRationale Rationale;
			Rationale.ContentKey = Operations[(*RelatedIndexes)[0]].ContentKey;
			Rationale.Reason = TEXT("Focused rationale.");
			Rationale.DesignIntentDelta = TEXT("Focused delta.");
			Rationale.RelatedOperationIndexes = *RelatedIndexes;
			Proposal.RecordRationales.Add(MoveTemp(Rationale));
		}
		return Proposal;
	}

	/** Issue code 존재 여부를 확인합니다. */
	bool HasIssueCode(const TArray<FCFContentValidationIssue>& Issues, const FString& Code)
	{
		return Issues.ContainsByPredicate(
			[&Code](const FCFContentValidationIssue& Issue)
			{
				return Issue.Code.Equals(Code, ESearchCase::CaseSensitive);
			});
	}

	/** ReviewPackage를 helper로 생성합니다. */
	bool BuildReview(
		FFixtureContext& Context,
		const FCFContentChangeProposal& Proposal,
		FCFContentReviewPackage& OutPackage,
		TArray<FCFContentValidationIssue>& OutIssues,
		FString& OutError)
	{
		return FCFContentPlanningService::BuildReviewPackage(
			Context.Snapshot,
			Proposal,
			Context.Registry,
			Context.ResourceCatalog,
			Context.PickerRegistry,
			OutPackage,
			OutIssues,
			OutError);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP006SnapshotFingerprintTest,
	"CarFight.CCAS.CF_FQ_058.P0_06.SnapshotFingerprint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFP006SnapshotFingerprintTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Fixture context입니다.
	CFContentPlanningTestsPrivate::FFixtureContext Context;
	// Initialization error입니다.
	FString Error;
	TestTrue(TEXT("Fixture initializes"), Context.Initialize(Error));
	TestTrue(TEXT("Snapshot fingerprint canonical"), CFDACommonPrimitives::IsCanonicalSha256Fingerprint(Context.Snapshot.SnapshotFingerprint));
	TestTrue(TEXT("Provider schema fingerprint canonical"), CFDACommonPrimitives::IsCanonicalSha256Fingerprint(Context.Snapshot.ProviderSchemaFingerprint));
	TestTrue(TEXT("Product state fingerprint canonical"), CFDACommonPrimitives::IsCanonicalSha256Fingerprint(Context.Snapshot.ProductStateFingerprint));
	TestTrue(TEXT("Resource catalog fingerprint canonical"), CFDACommonPrimitives::IsCanonicalSha256Fingerprint(Context.Snapshot.ResourceCatalogFingerprint));

	// Rebuilt snapshot입니다.
	FCFContentCatalogSnapshot Rebuilt;
	TestTrue(TEXT("Snapshot rebuild succeeds"), FCFContentPlanningService::BuildCatalogSnapshot(
		Context.CompileResult,
		Context.Registry,
		Context.ResourceCatalog,
		Context.PickerRegistry,
		Rebuilt,
		Error));
	TestEqual(TEXT("Snapshot identity is deterministic"), Rebuilt.SnapshotFingerprint, Context.Snapshot.SnapshotFingerprint);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP006PlanningLifecycleTest,
	"CarFight.CCAS.CF_FQ_058.P0_06.PlanningLifecycleReview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFP006PlanningLifecycleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Fixture context입니다.
	CFContentPlanningTestsPrivate::FFixtureContext Context;
	// Error text입니다.
	FString Error;
	TestTrue(TEXT("Fixture initializes"), Context.Initialize(Error));

	// Planning metadata operation입니다.
	FCFContentChangeOperation PlanningOperation =
		CFContentPlanningTestsPrivate::MakeOperation(ECFContentChangeOperationType::SetPlanningMetadata, TEXT("Base"));
	PlanningOperation.PlanningFamilyId = TEXT("Cannon");
	PlanningOperation.PlanningBaseContentId = TEXT("Base");
	PlanningOperation.PlanningDesignIntent = TEXT("Standard cannon family base.");
	PlanningOperation.PlanningMetadata.VariantId = TEXT("Standard");
	PlanningOperation.PlanningMetadata.RoleId = TEXT("FamilyBase");
	PlanningOperation.PlanningMetadata.ProductionWaveId = TEXT("Wave1A");
	PlanningOperation.PlanningMetadata.Readiness = ECFContentPlanningReadiness::AuthoringReady;

	// Relative intent입니다.
	FCFContentRelativeIntent RelativeIntent;
	RelativeIntent.DimensionId = TEXT("Damage");
	RelativeIntent.Direction = ECFContentRelativeDirection::Equal;
	PlanningOperation.PlanningMetadata.RelativeIntents.Add(RelativeIntent);

	// Lifecycle operation입니다.
	FCFContentChangeOperation RetireOperation =
		CFContentPlanningTestsPrivate::MakeOperation(ECFContentChangeOperationType::RetireRecord, TEXT("Spare"));

	// Proposal operations입니다.
	TArray<FCFContentChangeOperation> Operations{PlanningOperation, RetireOperation};
	// Proposal입니다.
	FCFContentChangeProposal Proposal =
		CFContentPlanningTestsPrivate::MakeProposal(Context.Snapshot, TEXT("P006.PlanningLifecycle"), Operations);

	// Review package입니다.
	FCFContentReviewPackage Package;
	// Review issues입니다.
	TArray<FCFContentValidationIssue> Issues;
	TestTrue(TEXT("Planning/lifecycle review succeeds"), CFContentPlanningTestsPrivate::BuildReview(Context, Proposal, Package, Issues, Error));
	TestEqual(TEXT("Planning changes preserve exact2 operations"), Package.PlanningChanges.Num(), 2);
	TestNotEqual(TEXT("Planning/lifecycle changes alter post semantic hash"), Package.ExpectedPostSemanticHash, Context.Snapshot.WorkbookSemanticHash);
	TestTrue(TEXT("Review package fingerprint canonical"), CFDACommonPrimitives::IsCanonicalSha256Fingerprint(Package.ReviewPackageFingerprint));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP006BulkSemanticPreflightTest,
	"CarFight.CCAS.CF_FQ_058.P0_06.BulkSemanticPreflight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFP006BulkSemanticPreflightTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Fixture context입니다.
	CFContentPlanningTestsPrivate::FFixtureContext Context;
	// Error text입니다.
	FString Error;
	TestTrue(TEXT("Fixture initializes"), Context.Initialize(Error));

	// New record Power update를 AddRecord보다 먼저 authored해도 semantic result가 동일해야 합니다.
	FCFContentChangeOperation PowerOperation =
		CFContentPlanningTestsPrivate::MakeOperation(ECFContentChangeOperationType::UpdateField, TEXT("NewWeapon"));
	PowerOperation.ColumnId = TEXT("Power");
	PowerOperation.Value = CFContentPlanningTestsPrivate::MakeDoubleValue(7.0);

	// New record add operation입니다.
	FCFContentChangeOperation AddOperation =
		CFContentPlanningTestsPrivate::MakeOperation(ECFContentChangeOperationType::AddRecord, TEXT("NewWeapon"));

	// New record display name operation입니다.
	FCFContentChangeOperation NameOperation =
		CFContentPlanningTestsPrivate::MakeOperation(ECFContentChangeOperationType::UpdateField, TEXT("NewWeapon"));
	NameOperation.ColumnId = TEXT("DisplayName");
	NameOperation.Value = CFContentPlanningTestsPrivate::MakeStringValue(TEXT("NewWeapon"));

	// Target NONE operation입니다.
	FCFContentChangeOperation TargetOperation =
		CFContentPlanningTestsPrivate::MakeOperation(ECFContentChangeOperationType::SetNone, TEXT("NewWeapon"));
	TargetOperation.ColumnId = TEXT("Target");

	// Resource NONE operation입니다.
	FCFContentChangeOperation ResourceOperation =
		CFContentPlanningTestsPrivate::MakeOperation(ECFContentChangeOperationType::SetNone, TEXT("NewWeapon"));
	ResourceOperation.ColumnId = TEXT("Resource");

	// Proposal입니다.
	TArray<FCFContentChangeOperation> Operations{PowerOperation, AddOperation, NameOperation, TargetOperation, ResourceOperation};
	FCFContentChangeProposal Proposal =
		CFContentPlanningTestsPrivate::MakeProposal(Context.Snapshot, TEXT("P006.BulkAdd"), Operations);

	// Review package입니다.
	FCFContentReviewPackage Package;
	// Review issues입니다.
	TArray<FCFContentValidationIssue> Issues;
	TestTrue(TEXT("Add + same-record mutations succeed independent of authored order"), CFContentPlanningTestsPrivate::BuildReview(Context, Proposal, Package, Issues, Error));

	// Duplicate semantic target operation입니다.
	FCFContentChangeOperation DuplicatePower = PowerOperation;
	TArray<FCFContentChangeOperation> ConflictOperations{PowerOperation, DuplicatePower};
	FCFContentChangeProposal ConflictProposal =
		CFContentPlanningTestsPrivate::MakeProposal(Context.Snapshot, TEXT("P006.DuplicateTarget"), ConflictOperations);

	// Conflict package입니다.
	FCFContentReviewPackage ConflictPackage;
	Issues.Reset();
	Error.Reset();
	TestFalse(TEXT("Duplicate semantic target blocks"), CFContentPlanningTestsPrivate::BuildReview(Context, ConflictProposal, ConflictPackage, Issues, Error));
	TestTrue(TEXT("Duplicate target diagnostic exists"), CFContentPlanningTestsPrivate::HasIssueCode(Issues, TEXT("DuplicateSemanticTarget")));

	// Unknown SheetId를 사용한 top-level planning operation입니다.
	FCFContentChangeOperation UnknownSheetPlanning =
		CFContentPlanningTestsPrivate::MakeOperation(
			ECFContentChangeOperationType::SetPlanningMetadata,
			TEXT("Spare"),
			TEXT("Content.Unknown"));
	UnknownSheetPlanning.PlanningFamilyId = TEXT("Cannon");
	UnknownSheetPlanning.PlanningMetadata.VariantId = TEXT("Standard");

	// Unknown Sheet proposal입니다.
	TArray<FCFContentChangeOperation> UnknownSheetOperations{UnknownSheetPlanning};
	FCFContentChangeProposal UnknownSheetProposal =
		CFContentPlanningTestsPrivate::MakeProposal(
			Context.Snapshot,
			TEXT("P006.UnknownSheet"),
			UnknownSheetOperations);

	// Unknown Sheet package입니다.
	FCFContentReviewPackage UnknownSheetPackage;
	Issues.Reset();
	Error.Reset();
	TestFalse(TEXT("Unknown SheetId blocks before transient mutation"), CFContentPlanningTestsPrivate::BuildReview(Context, UnknownSheetProposal, UnknownSheetPackage, Issues, Error));
	TestTrue(TEXT("Unknown SheetId diagnostic exists"), CFContentPlanningTestsPrivate::HasIssueCode(Issues, TEXT("UnknownChangeSheetId")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP006RetireReferenceGuardTest,
	"CarFight.CCAS.CF_FQ_058.P0_06.RetireReferenceGuard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFP006RetireReferenceGuardTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Fixture context입니다.
	CFContentPlanningTestsPrivate::FFixtureContext Context;
	// Error text입니다.
	FString Error;
	TestTrue(TEXT("Fixture initializes"), Context.Initialize(Error));

	// Consumer가 참조하는 Base retire operation입니다.
	FCFContentChangeOperation RetireBase =
		CFContentPlanningTestsPrivate::MakeOperation(ECFContentChangeOperationType::RetireRecord, TEXT("Base"));

	// Proposal입니다.
	TArray<FCFContentChangeOperation> Operations{RetireBase};
	FCFContentChangeProposal Proposal =
		CFContentPlanningTestsPrivate::MakeProposal(Context.Snapshot, TEXT("P006.RetireReferenced"), Operations);

	// Review package입니다.
	FCFContentReviewPackage Package;
	// Review issues입니다.
	TArray<FCFContentValidationIssue> Issues;
	TestFalse(TEXT("Retiring required referenced target blocks"), CFContentPlanningTestsPrivate::BuildReview(Context, Proposal, Package, Issues, Error));
	TestTrue(TEXT("Retired required reference diagnostic exists"), CFContentPlanningTestsPrivate::HasIssueCode(Issues, TEXT("RetiredRequiredReference")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP006StaleGuardTest,
	"CarFight.CCAS.CF_FQ_058.P0_06.TwoLayerStaleGuard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFP006StaleGuardTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Fixture context입니다.
	CFContentPlanningTestsPrivate::FFixtureContext Context;
	// Error text입니다.
	FString Error;
	TestTrue(TEXT("Fixture initializes"), Context.Initialize(Error));

	// Planning operation입니다.
	FCFContentChangeOperation PlanningOperation =
		CFContentPlanningTestsPrivate::MakeOperation(ECFContentChangeOperationType::SetPlanningMetadata, TEXT("Spare"));
	PlanningOperation.PlanningFamilyId = TEXT("Rocket");
	PlanningOperation.PlanningMetadata.VariantId = TEXT("Standard");

	// Proposal입니다.
	TArray<FCFContentChangeOperation> Operations{PlanningOperation};
	FCFContentChangeProposal Proposal =
		CFContentPlanningTestsPrivate::MakeProposal(Context.Snapshot, TEXT("P006.Stale"), Operations);

	// Review package입니다.
	FCFContentReviewPackage Package;
	// Review issues입니다.
	TArray<FCFContentValidationIssue> Issues;
	TestTrue(TEXT("Review succeeds"), CFContentPlanningTestsPrivate::BuildReview(Context, Proposal, Package, Issues, Error));
	Issues.Reset();
	TestTrue(TEXT("Exact same fresh snapshot passes"), FCFContentPlanningService::ValidateReviewFreshness(Package, Context.Snapshot, Issues));

	// Workbook stale snapshot입니다.
	FCFContentCatalogSnapshot WorkbookStale = Context.Snapshot;
	WorkbookStale.WorkbookSemanticHash = CFContentPlanningTestsPrivate::HashText(TEXT("different-workbook"));
	Issues.Reset();
	TestFalse(TEXT("Workbook stale blocks"), FCFContentPlanningService::ValidateReviewFreshness(Package, WorkbookStale, Issues));
	TestTrue(TEXT("StaleWorkbook diagnostic exists"), CFContentPlanningTestsPrivate::HasIssueCode(Issues, TEXT("StaleWorkbook")));

	// Planning truth stale snapshot입니다.
	FCFContentCatalogSnapshot PlanningStale = Context.Snapshot;
	PlanningStale.SnapshotFingerprint = CFContentPlanningTestsPrivate::HashText(TEXT("different-snapshot"));
	Issues.Reset();
	TestFalse(TEXT("Catalog snapshot stale blocks"), FCFContentPlanningService::ValidateReviewFreshness(Package, PlanningStale, Issues));
	TestTrue(TEXT("StalePlanningSnapshot diagnostic exists"), CFContentPlanningTestsPrivate::HasIssueCode(Issues, TEXT("StalePlanningSnapshot")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP006ApprovalBindingTest,
	"CarFight.CCAS.CF_FQ_058.P0_06.ApprovalBinding",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFP006ApprovalBindingTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Fixture context입니다.
	CFContentPlanningTestsPrivate::FFixtureContext Context;
	// Error text입니다.
	FString Error;
	TestTrue(TEXT("Fixture initializes"), Context.Initialize(Error));

	// Planning operation입니다.
	FCFContentChangeOperation PlanningOperation =
		CFContentPlanningTestsPrivate::MakeOperation(ECFContentChangeOperationType::SetPlanningMetadata, TEXT("Spare"));
	PlanningOperation.PlanningFamilyId = TEXT("Cannon");
	PlanningOperation.PlanningMetadata.VariantId = TEXT("Rapid");

	// Proposal입니다.
	TArray<FCFContentChangeOperation> Operations{PlanningOperation};
	FCFContentChangeProposal Proposal =
		CFContentPlanningTestsPrivate::MakeProposal(Context.Snapshot, TEXT("P006.Approval"), Operations);

	// Review package입니다.
	FCFContentReviewPackage Package;
	// Review issues입니다.
	TArray<FCFContentValidationIssue> Issues;
	TestTrue(TEXT("Review succeeds"), CFContentPlanningTestsPrivate::BuildReview(Context, Proposal, Package, Issues, Error));

	// Exact approval입니다.
	FCFContentReviewApproval Approval;
	Approval.bApproved = true;
	Approval.ReviewPackageFingerprint = Package.ReviewPackageFingerprint;
	Issues.Reset();
	TestTrue(TEXT("Exact package approval passes"), FCFContentPlanningService::ValidateApproval(Package, Approval, Issues));

	// Mismatched approval입니다.
	Approval.ReviewPackageFingerprint = CFContentPlanningTestsPrivate::HashText(TEXT("different-review-package"));
	Issues.Reset();
	TestFalse(TEXT("Mismatched package approval blocks"), FCFContentPlanningService::ValidateApproval(Package, Approval, Issues));
	TestTrue(TEXT("Approval fingerprint mismatch diagnostic exists"), CFContentPlanningTestsPrivate::HasIssueCode(Issues, TEXT("ReviewApprovalFingerprintMismatch")));

	// 승인된 fingerprint 문자열은 유지하고 package payload만 변조한 fixture입니다.
	FCFContentReviewPackage TamperedPackage = Package;
	TamperedPackage.ExpectedPostSemanticHash =
		CFContentPlanningTestsPrivate::HashText(TEXT("tampered-post-workbook"));
	Approval.ReviewPackageFingerprint = Package.ReviewPackageFingerprint;
	Issues.Reset();
	TestFalse(
		TEXT("Approved ReviewPackage payload tamper blocks"),
		FCFContentPlanningService::ValidateApproval(
			TamperedPackage,
			Approval,
			Issues));
	TestTrue(
		TEXT("ReviewPackage payload fingerprint mismatch diagnostic exists"),
		CFContentPlanningTestsPrivate::HasIssueCode(
			Issues,
			TEXT("ReviewPackagePayloadFingerprintMismatch")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP006NoDirectDAWriteTest,
	"CarFight.CCAS.CF_FQ_058.P0_06.NoDirectDAWrite",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFP006NoDirectDAWriteTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Fixture context입니다.
	CFContentPlanningTestsPrivate::FFixtureContext Context;
	// Error text입니다.
	FString Error;
	TestTrue(TEXT("Fixture initializes"), Context.Initialize(Error));

	// Managed field update입니다.
	FCFContentChangeOperation PowerOperation =
		CFContentPlanningTestsPrivate::MakeOperation(ECFContentChangeOperationType::UpdateField, TEXT("Spare"));
	PowerOperation.ColumnId = TEXT("Power");
	PowerOperation.Value = CFContentPlanningTestsPrivate::MakeDoubleValue(4.0);

	// Proposal입니다.
	TArray<FCFContentChangeOperation> Operations{PowerOperation};
	FCFContentChangeProposal Proposal =
		CFContentPlanningTestsPrivate::MakeProposal(Context.Snapshot, TEXT("P006.NoDAWrite"), Operations);

	// Review package입니다.
	FCFContentReviewPackage Package;
	// Review issues입니다.
	TArray<FCFContentValidationIssue> Issues;
	TestTrue(TEXT("Review succeeds"), CFContentPlanningTestsPrivate::BuildReview(Context, Proposal, Package, Issues, Error));
	TestTrue(TEXT("No persistent Workbook write guard passes"), Package.bNoPersistentWorkbookWriteGuardPassed);
	TestTrue(TEXT("No direct DA write guard passes"), Package.bNoDirectDataAssetWriteGuardPassed);
	TestEqual(TEXT("Provider.ApplyReviewed exact0"), Context.Provider.IsValid() ? Context.Provider->ApplyCallCount : -1, 0);
	return true;
}


namespace CFContentPlanningUserReviewPrivate
{
	/** USER review fixture용 relative intent를 생성합니다. */
	FCFContentRelativeIntent MakeRelativeIntent(
		const FString& DimensionId,
		const ECFContentRelativeDirection Direction)
	{
		// 생성할 relative intent입니다.
		FCFContentRelativeIntent Intent;
		Intent.DimensionId = DimensionId;
		Intent.Direction = Direction;
		return Intent;
	}

	/** USER review fixture용 technology dependency를 생성합니다. */
	FCFContentTechnologyDependency MakeTechnologyDependency(
		const FString& DependencyId,
		const ECFContentDependencyState State,
		const FString& Reason)
	{
		// 생성할 technology dependency입니다.
		FCFContentTechnologyDependency Dependency;
		Dependency.DependencyId = DependencyId;
		Dependency.State = State;
		Dependency.Reason = Reason;
		return Dependency;
	}

	/** Review batch state를 사람이 읽을 수 있는 stable text로 변환합니다. */
	const TCHAR* BatchStateToText(const ECFContentBatchItemState State)
	{
		switch (State)
		{
		case ECFContentBatchItemState::ReadyToReview:
			return TEXT("ReadyToReview");
		case ECFContentBatchItemState::RetireCandidate:
			return TEXT("RetireCandidate");
		case ECFContentBatchItemState::AlreadyCurrent:
			return TEXT("AlreadyCurrent");
		case ECFContentBatchItemState::ExternalReadOnly:
			return TEXT("ExternalReadOnly");
		case ECFContentBatchItemState::Blocked:
		default:
			return TEXT("Blocked");
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP006WeaponRosterUserReviewTest,
	"CarFight.CCAS.CF_FQ_058.P0_06_USER.WeaponRosterProposalReview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

/** USER Accepted Weapon Roster exact8을 disposable AI Change Proposal -> Review Package 경로로 검수합니다. */
bool FCFP006WeaponRosterUserReviewTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// USER Accepted Roster의 현재 Product anchor exact2 + ExternalReadOnly exact1을 모사하는 baseline Workbook입니다.
	FCFContentWorkbookModel WeaponRosterBaseline;
	WeaponRosterBaseline.WorkbookSourceId = TEXT("Test.P006.UserReview.WeaponRoster");
	WeaponRosterBaseline.SchemaRevision = 1;
	WeaponRosterBaseline.Sheets = CFContentPlanningTestsPrivate::MakeSchema();
	WeaponRosterBaseline.Records.Add(
		CFContentPlanningTestsPrivate::MakeRecord(TEXT("HeavyCannon"), 14.0, TEXT("")));
	WeaponRosterBaseline.Records.Add(
		CFContentPlanningTestsPrivate::MakeRecord(TEXT("RocketLauncher"), 12.0, TEXT("")));
	WeaponRosterBaseline.Records.Add(
		CFContentPlanningTestsPrivate::MakeRecord(
			TEXT("External"),
			1.0,
			TEXT(""),
			ECFContentManagementState::ExternalReadOnly));

	// Disposable baseline fixture입니다.
	CFContentPlanningTestsPrivate::FFixtureContext Context;
	// Fixture initialization error입니다.
	FString Error;
	TestTrue(TEXT("Weapon roster fixture initializes"), Context.InitializeWithWorkbook(WeaponRosterBaseline, Error));

	// AI Change Proposal에 들어갈 typed operations입니다.
	TArray<FCFContentChangeOperation> Operations;
	// Record별 USER-facing 변경 이유입니다.
	TMap<FString, FString> ReasonByContentId;
	// Record별 USER-facing design intent delta입니다.
	TMap<FString, FString> DesignDeltaByContentId;

	// Weapon Roster record 하나의 Add + required fields + PlanningMetadata operations를 구성하는 helper입니다.
	auto AppendWeaponRosterItem =
		[&Operations, &ReasonByContentId, &DesignDeltaByContentId](
			const FString& ContentId,
			const bool bExistingAnchor,
			const FString& DisplayName,
			const FString& FamilyId,
			const FString& VariantId,
			const FString& RoleId,
			const FString& BaseContentId,
			const FString& DesignIntent,
			const FString& ProductionWaveId,
			const ECFContentPlanningReadiness Readiness,
			const TArray<FCFContentRelativeIntent>& RelativeIntents,
			const TArray<FCFContentTechnologyDependency>& TechnologyDependencies,
			const TArray<FString>& SharedDataRoleIds,
			const TArray<FString>& VariantOwnedDataRoleIds,
			const double SyntheticPower,
			const FString& Reason,
			const FString& DesignDelta)
		{
			if (!bExistingAnchor)
			{
				// 신규 Product 후보 record 생성 operation입니다.
				FCFContentChangeOperation AddOperation =
					CFContentPlanningTestsPrivate::MakeOperation(
						ECFContentChangeOperationType::AddRecord,
						ContentId);
				Operations.Add(MoveTemp(AddOperation));

				// USER-facing 표시 이름 operation입니다.
				FCFContentChangeOperation DisplayNameOperation =
					CFContentPlanningTestsPrivate::MakeOperation(
						ECFContentChangeOperationType::UpdateField,
						ContentId);
				DisplayNameOperation.ColumnId = TEXT("DisplayName");
				DisplayNameOperation.Value =
					CFContentPlanningTestsPrivate::MakeStringValue(DisplayName);
				Operations.Add(MoveTemp(DisplayNameOperation));

				// Fixture compiler의 required gameplay field를 채우는 synthetic 값입니다.
				FCFContentChangeOperation PowerOperation =
					CFContentPlanningTestsPrivate::MakeOperation(
						ECFContentChangeOperationType::UpdateField,
						ContentId);
				PowerOperation.ColumnId = TEXT("Power");
				PowerOperation.Value =
					CFContentPlanningTestsPrivate::MakeDoubleValue(SyntheticPower);
				Operations.Add(MoveTemp(PowerOperation));

				// Optional ContentReference를 명시적으로 NONE으로 두는 operation입니다.
				FCFContentChangeOperation TargetOperation =
					CFContentPlanningTestsPrivate::MakeOperation(
						ECFContentChangeOperationType::SetNone,
						ContentId);
				TargetOperation.ColumnId = TEXT("Target");
				Operations.Add(MoveTemp(TargetOperation));

				// Optional ResourceReference를 명시적으로 NONE으로 두는 operation입니다.
				FCFContentChangeOperation ResourceOperation =
					CFContentPlanningTestsPrivate::MakeOperation(
						ECFContentChangeOperationType::SetNone,
						ContentId);
				ResourceOperation.ColumnId = TEXT("Resource");
				Operations.Add(MoveTemp(ResourceOperation));
			}

			// Family/Variant/Role/Wave/Readiness를 보존하는 planning operation입니다.
			FCFContentChangeOperation PlanningOperation =
				CFContentPlanningTestsPrivate::MakeOperation(
					ECFContentChangeOperationType::SetPlanningMetadata,
					ContentId);
			PlanningOperation.PlanningFamilyId = FamilyId;
			PlanningOperation.PlanningBaseContentId = BaseContentId;
			PlanningOperation.PlanningDesignIntent = DesignIntent;
			PlanningOperation.PlanningMetadata.VariantId = VariantId;
			PlanningOperation.PlanningMetadata.RoleId = RoleId;
			PlanningOperation.PlanningMetadata.ProductionWaveId = ProductionWaveId;
			PlanningOperation.PlanningMetadata.Readiness = Readiness;
			PlanningOperation.PlanningMetadata.RelativeIntents = RelativeIntents;
			PlanningOperation.PlanningMetadata.TechnologyDependencies = TechnologyDependencies;
			PlanningOperation.PlanningMetadata.SharedDataRoleIds = SharedDataRoleIds;
			PlanningOperation.PlanningMetadata.VariantOwnedDataRoleIds = VariantOwnedDataRoleIds;
			Operations.Add(MoveTemp(PlanningOperation));

			ReasonByContentId.Add(ContentId, Reason);
			DesignDeltaByContentId.Add(ContentId, DesignDelta);
		};

	// Cannon Standard relative intent입니다.
	TArray<FCFContentRelativeIntent> CannonStandardIntents{
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("Damage"), ECFContentRelativeDirection::Equal),
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("FireRate"), ECFContentRelativeDirection::Equal),
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("Range"), ECFContentRelativeDirection::Equal)};
	AppendWeaponRosterItem(
		TEXT("Cannon_Standard"),
		false,
		TEXT("Cannon Standard"),
		TEXT("Cannon"),
		TEXT("Standard"),
		TEXT("FamilyBase"),
		TEXT("Cannon_Standard"),
		TEXT("Cannon Family의 신규 표준 기준점."),
		TEXT("Wave1A"),
		ECFContentPlanningReadiness::AuthoringReady,
		CannonStandardIntents,
		{},
		{TEXT("TurretMount.Cannon")},
		{TEXT("WeaponData"), TEXT("ProjectileData")},
		10.0,
		TEXT("USER Accepted Roster의 Cannon Family Base를 신규 계획한다."),
		TEXT("Prototype Heavy 수치가 아니라 Standard 기준점을 별도로 만든다."));

	// Cannon Heavy relative intent입니다.
	TArray<FCFContentRelativeIntent> CannonHeavyIntents{
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("Damage"), ECFContentRelativeDirection::Increase),
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("FireRate"), ECFContentRelativeDirection::Decrease)};
	AppendWeaponRosterItem(
		TEXT("HeavyCannon"),
		true,
		TEXT("Heavy Cannon"),
		TEXT("Cannon"),
		TEXT("Heavy"),
		TEXT("HeavyVariant"),
		TEXT("Cannon_Standard"),
		TEXT("기존 Heavy Cannon Product identity를 Heavy Variant anchor로 유지."),
		TEXT("Wave1A"),
		ECFContentPlanningReadiness::AuthoringReady,
		CannonHeavyIntents,
		{},
		{TEXT("TurretMount.Cannon")},
		{TEXT("WeaponData"), TEXT("ProjectileData"), TEXT("DamageData")},
		14.0,
		TEXT("기존 Heavy Cannon을 폐기하지 않고 Heavy Variant 역할로 정리한다."),
		TEXT("Standard 대비 한 발 위력은 높이고 발사 빈도는 낮춘다."));

	// Cannon LongRange relative intent입니다.
	TArray<FCFContentRelativeIntent> CannonLongRangeIntents{
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("Range"), ECFContentRelativeDirection::Increase),
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("FireRate"), ECFContentRelativeDirection::Decrease)};
	AppendWeaponRosterItem(
		TEXT("Cannon_LongRange"),
		false,
		TEXT("Cannon Long Range"),
		TEXT("Cannon"),
		TEXT("LongRange"),
		TEXT("LongRangeVariant"),
		TEXT("Cannon_Standard"),
		TEXT("장거리 교전 특화 Cannon Variant."),
		TEXT("Wave1A"),
		ECFContentPlanningReadiness::AuthoringReady,
		CannonLongRangeIntents,
		{},
		{TEXT("TurretMount.Cannon")},
		{TEXT("WeaponData"), TEXT("ProjectileData")},
		11.0,
		TEXT("Cannon Family에 장거리 역할을 추가한다."),
		TEXT("Standard 대비 사거리 우선, 연사력은 낮추는 방향을 고정한다."));

	// Cannon Rapid relative intent입니다.
	TArray<FCFContentRelativeIntent> CannonRapidIntents{
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("FireRate"), ECFContentRelativeDirection::Increase),
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("Damage"), ECFContentRelativeDirection::Decrease)};
	AppendWeaponRosterItem(
		TEXT("Cannon_Rapid"),
		false,
		TEXT("Cannon Rapid"),
		TEXT("Cannon"),
		TEXT("Rapid"),
		TEXT("RapidVariant"),
		TEXT("Cannon_Standard"),
		TEXT("고연사 Cannon Variant; 초기에는 Autocannon/MachineGun 역할 일부를 흡수."),
		TEXT("Wave1A"),
		ECFContentPlanningReadiness::AuthoringReady,
		CannonRapidIntents,
		{},
		{TEXT("TurretMount.Cannon")},
		{TEXT("WeaponData"), TEXT("ProjectileData"), TEXT("DamageData")},
		7.0,
		TEXT("별도 Machine Gun Family를 즉시 늘리지 않고 Rapid Cannon으로 역할을 검증한다."),
		TEXT("Standard 대비 연사력을 높이고 발당 위력을 낮춘다."));

	// Rocket Standard relative intent입니다.
	TArray<FCFContentRelativeIntent> RocketStandardIntents{
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("ProjectileCount"), ECFContentRelativeDirection::Equal),
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("ReleaseInterval"), ECFContentRelativeDirection::Equal)};
	AppendWeaponRosterItem(
		TEXT("Rocket_Standard"),
		false,
		TEXT("Unguided Rocket Standard"),
		TEXT("UnguidedRocket"),
		TEXT("Standard"),
		TEXT("FamilyBase"),
		TEXT("Rocket_Standard"),
		TEXT("Unguided Rocket Family의 단순 표준 발사 기준점."),
		TEXT("Wave1A"),
		ECFContentPlanningReadiness::AuthoringReady,
		RocketStandardIntents,
		{},
		{TEXT("TurretMount.Rocket"), TEXT("ProjectileData.Rocket"), TEXT("AmmoData.Rocket"), TEXT("DamageData.RocketDirectHit")},
		{TEXT("WeaponData")},
		9.0,
		TEXT("기존 Salvo 중심 RocketLauncher와 분리된 Standard 기준점을 만든다."),
		TEXT("Salvo/Ripple 비교 기준이 되는 단순 발사 패턴을 제공한다."));

	// Rocket Salvo relative intent입니다.
	TArray<FCFContentRelativeIntent> RocketSalvoIntents{
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("ProjectileCount"), ECFContentRelativeDirection::Increase),
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("BurstCommitment"), ECFContentRelativeDirection::Increase)};
	AppendWeaponRosterItem(
		TEXT("RocketLauncher"),
		true,
		TEXT("Rocket Launcher"),
		TEXT("UnguidedRocket"),
		TEXT("Salvo"),
		TEXT("SalvoVariant"),
		TEXT("Rocket_Standard"),
		TEXT("기존 RocketLauncher Product의 Salvo 4발 성격을 Product anchor로 유지."),
		TEXT("Wave1A"),
		ECFContentPlanningReadiness::AuthoringReady,
		RocketSalvoIntents,
		{},
		{TEXT("TurretMount.Rocket"), TEXT("ProjectileData.Rocket"), TEXT("AmmoData.Rocket"), TEXT("DamageData.RocketDirectHit")},
		{TEXT("WeaponData")},
		12.0,
		TEXT("기존 RocketLauncher의 실제 persisted Salvo 성격을 명시적 Variant로 정리한다."),
		TEXT("Standard 대비 한 트리거당 발사량과 burst commitment를 높인다."));

	// Rocket Ripple relative intent입니다.
	TArray<FCFContentRelativeIntent> RocketRippleIntents{
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("ReleaseInterval"), ECFContentRelativeDirection::Increase),
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("BurstCommitment"), ECFContentRelativeDirection::Decrease)};
	AppendWeaponRosterItem(
		TEXT("Rocket_Ripple"),
		false,
		TEXT("Unguided Rocket Ripple"),
		TEXT("UnguidedRocket"),
		TEXT("Ripple"),
		TEXT("RippleVariant"),
		TEXT("Rocket_Standard"),
		TEXT("여러 발을 시간차로 풀어내는 지속 압박형 Rocket Variant."),
		TEXT("Wave1A"),
		ECFContentPlanningReadiness::AuthoringReady,
		RocketRippleIntents,
		{},
		{TEXT("TurretMount.Rocket"), TEXT("ProjectileData.Rocket"), TEXT("AmmoData.Rocket"), TEXT("DamageData.RocketDirectHit")},
		{TEXT("WeaponData")},
		10.0,
		TEXT("Single/Salvo와 다른 현재 Launcher Scheduler의 Ripple 축을 실제 콘텐츠로 사용한다."),
		TEXT("Salvo보다 발사 간격을 늘려 지속 압박과 조준 수정 여지를 만든다."));

	// Guided Missile technology dependency입니다.
	TArray<FCFContentTechnologyDependency> GuidedDependencies{
		CFContentPlanningUserReviewPrivate::MakeTechnologyDependency(
			TEXT("PFP.P0.03.UserTrajectory"),
			ECFContentDependencyState::Pending,
			TEXT("Guided Missile Product acceptance 전에 USER trajectory review가 필요합니다."))};
	// Guided Missile relative intent입니다.
	TArray<FCFContentRelativeIntent> GuidedIntents{
		CFContentPlanningUserReviewPrivate::MakeRelativeIntent(TEXT("GuidanceCapability"), ECFContentRelativeDirection::Increase)};
	AppendWeaponRosterItem(
		TEXT("GuidedMissile_Standard"),
		false,
		TEXT("Guided Missile Standard"),
		TEXT("GuidedMissile"),
		TEXT("Standard"),
		TEXT("FamilyBase"),
		TEXT("GuidedMissile_Standard"),
		TEXT("현재 TargetActor guidance 기반 첫 Guided Missile Product 후보."),
		TEXT("Wave1B"),
		ECFContentPlanningReadiness::Conditional,
		GuidedIntents,
		GuidedDependencies,
		{TEXT("TurretMount.Rocket")},
		{TEXT("WeaponData"), TEXT("ProjectileData.Guided"), TEXT("AmmoData.Guided")},
		13.0,
		TEXT("USER Accepted Roster에서 Guided Missile Standard를 Wave1B conditional로 포함한다."),
		TEXT("비유도 Rocket과 달리 유도 capability를 갖지만 PFP-P0-03 USER trajectory 검수 전 Product acceptance는 열지 않는다."));

	// Snapshot-bound AI proposal입니다.
	FCFContentChangeProposal Proposal =
		CFContentPlanningTestsPrivate::MakeProposal(
			Context.Snapshot,
			TEXT("P006.UserReview.WeaponWave1"),
			Operations);
	Proposal.ChangeSet.Reason =
		TEXT("USER Accepted WeaponContentRosterPlan v0.1.1 exact8 First Production Wave를 CCAS planning candidate로 검토합니다.");
	Proposal.ChangeSet.DesignIntent =
		TEXT("Cannon / Unguided Rocket / Guided Missile exact3 Family를 Family/Variant/Role/Wave/Readiness로 묶어 한 번에 review합니다.");

	for (FCFContentRecordRationale& Rationale : Proposal.RecordRationales)
	{
		// Rationale 대상 content id입니다.
		const FString& ContentId = Rationale.ContentKey.ContentId;
		// USER-facing reason입니다.
		const FString* Reason = ReasonByContentId.Find(ContentId);
		// USER-facing design delta입니다.
		const FString* DesignDelta = DesignDeltaByContentId.Find(ContentId);
		if (Reason != nullptr)
		{
			Rationale.Reason = *Reason;
		}
		if (DesignDelta != nullptr)
		{
			Rationale.DesignIntentDelta = *DesignDelta;
		}
	}

	// 실제 P0-06 review package입니다.
	FCFContentReviewPackage Package;
	// Review diagnostics입니다.
	TArray<FCFContentValidationIssue> Issues;
	TestTrue(
		TEXT("Weapon roster AI Change Proposal -> Review Package succeeds"),
		CFContentPlanningTestsPrivate::BuildReview(
			Context,
			Proposal,
			Package,
			Issues,
			Error));

	TestEqual(TEXT("Weapon roster touched rationales exact8"), Package.RecordRationales.Num(), 8);
	TestEqual(TEXT("Weapon roster typed operations exact38"), Package.PlanningChanges.Num(), 38);
	TestEqual(TEXT("Weapon roster preview adds exact6 Product candidates"), Package.Preview.BatchResult.ReadyToReviewCount, 6);
	TestEqual(TEXT("Existing Heavy/Salvo gameplay payload remains exact2 current"), Package.Preview.BatchResult.AlreadyCurrentCount, 2);
	TestEqual(TEXT("Baseline ExternalReadOnly remains exact1"), Package.Preview.BatchResult.ExternalReadOnlyCount, 1);
	TestEqual(TEXT("No blocked preview items"), Package.Preview.BatchResult.BlockedCount, 0);
	TestEqual(TEXT("No retire candidates"), Package.Preview.BatchResult.RetireCandidateCount, 0);
	TestTrue(TEXT("ReviewPackageFingerprint canonical"), CFDACommonPrimitives::IsCanonicalSha256Fingerprint(Package.ReviewPackageFingerprint));
	TestTrue(TEXT("ExpectedPostSemanticHash canonical"), CFDACommonPrimitives::IsCanonicalSha256Fingerprint(Package.ExpectedPostSemanticHash));
	TestTrue(TEXT("No persistent Workbook write guard"), Package.bNoPersistentWorkbookWriteGuardPassed);
	TestTrue(TEXT("No direct DataAsset write guard"), Package.bNoDirectDataAssetWriteGuardPassed);
	TestEqual(TEXT("Provider.ApplyReviewed exact0"), Context.Provider.IsValid() ? Context.Provider->ApplyCallCount : -1, 0);

	// Exact immutable package binding 검사용 synthetic approval입니다.
	FCFContentReviewApproval SyntheticApproval;
	SyntheticApproval.bApproved = true;
	SyntheticApproval.ReviewPackageFingerprint = Package.ReviewPackageFingerprint;
	// Approval binding diagnostics입니다.
	TArray<FCFContentValidationIssue> ApprovalIssues;
	TestTrue(
		TEXT("Exact ReviewPackage fingerprint can bind approval"),
		FCFContentPlanningService::ValidateApproval(
			Package,
			SyntheticApproval,
			ApprovalIssues));

	UE_LOG(LogTemp, Display, TEXT("CCAS_USER_REVIEW|ChangeSetId=%s"), *Package.ChangeSetId);
	UE_LOG(LogTemp, Display, TEXT("CCAS_USER_REVIEW|BaseCatalogSnapshotFingerprint=%s"), *Package.BaseCatalogSnapshotFingerprint);
	UE_LOG(LogTemp, Display, TEXT("CCAS_USER_REVIEW|BaseWorkbookSemanticHash=%s"), *Package.BaseWorkbookSemanticHash);
	UE_LOG(LogTemp, Display, TEXT("CCAS_USER_REVIEW|ExpectedPostSemanticHash=%s"), *Package.ExpectedPostSemanticHash);
	UE_LOG(LogTemp, Display, TEXT("CCAS_USER_REVIEW|ReviewPackageFingerprint=%s"), *Package.ReviewPackageFingerprint);
	UE_LOG(LogTemp, Display, TEXT("CCAS_USER_REVIEW|OperationCount=%d|RationaleCount=%d|ReviewChangeCount=%d"),
		Proposal.ChangeSet.Operations.Num(),
		Package.RecordRationales.Num(),
		Package.PlanningChanges.Num());
	UE_LOG(LogTemp, Display, TEXT("CCAS_USER_REVIEW|Batch|Desired=%d|Current=%d|Ready=%d|Retire=%d|AlreadyCurrent=%d|ExternalReadOnly=%d|Blocked=%d"),
		Package.Preview.BatchResult.DesiredRecordCount,
		Package.Preview.BatchResult.CurrentRecordCount,
		Package.Preview.BatchResult.ReadyToReviewCount,
		Package.Preview.BatchResult.RetireCandidateCount,
		Package.Preview.BatchResult.AlreadyCurrentCount,
		Package.Preview.BatchResult.ExternalReadOnlyCount,
		Package.Preview.BatchResult.BlockedCount);
	UE_LOG(LogTemp, Display, TEXT("CCAS_USER_REVIEW|Guards|NoPersistentWorkbookWrite=%d|NoDirectDataAssetWrite=%d|ProviderApplyReviewedCalls=%d"),
		Package.bNoPersistentWorkbookWriteGuardPassed ? 1 : 0,
		Package.bNoDirectDataAssetWriteGuardPassed ? 1 : 0,
		Context.Provider.IsValid() ? Context.Provider->ApplyCallCount : -1);
	UE_LOG(LogTemp, Display, TEXT("CCAS_USER_REVIEW|ApprovalBinding=PASS"));

	for (const FCFContentRecordRationale& Rationale : Package.RecordRationales)
	{
		UE_LOG(LogTemp, Display, TEXT("CCAS_USER_REVIEW|Rationale|%s|Reason=%s|Delta=%s|OperationIndexes=%d"),
			*Rationale.ContentKey.ToStableString(),
			*Rationale.Reason,
			*Rationale.DesignIntentDelta,
			Rationale.RelatedOperationIndexes.Num());
	}

	for (const FCFContentBatchItemResult& Item : Package.Preview.BatchResult.Items)
	{
		if (Item.Key.ContentId.Equals(TEXT("HeavyCannon"), ESearchCase::CaseSensitive)
			|| Item.Key.ContentId.Equals(TEXT("RocketLauncher"), ESearchCase::CaseSensitive)
			|| Item.Key.ContentId.StartsWith(TEXT("Cannon_"))
			|| Item.Key.ContentId.StartsWith(TEXT("Rocket_"))
			|| Item.Key.ContentId.StartsWith(TEXT("GuidedMissile_")))
		{
			UE_LOG(LogTemp, Display, TEXT("CCAS_USER_REVIEW|PreviewItem|%s|State=%s|Summary=%s"),
				*Item.Key.ToStableString(),
				CFContentPlanningUserReviewPrivate::BatchStateToText(Item.State),
				*Item.Summary);
		}
	}

	return true;
}
