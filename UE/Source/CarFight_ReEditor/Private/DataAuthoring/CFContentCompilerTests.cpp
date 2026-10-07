// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCompilerTests.cpp
// Version: v1.3.0
// Date: 2026-09-29
// Description: CF-FQ-058 CCAS-P0-02 Generic Compiler Core focused Automation과 disposable synthetic fixture입니다.
// Changelog:
// - v1.3.0: Re-review 보강으로 whole-current second-import drift와 Provider current fingerprint 증거 모순을 기존 exact12 안에서 추가 검증.
// - v1.2.0: Mid-review P1 exact3 + snapshot consistency 교정 회귀 테스트 exact4와 canonical Provider SHA-256 fixture를 추가.
// - v1.1.0: 10k content + 100k child benchmark가 canonical textual cell Parse + hash/diff를 모두 실제 측정하도록 강화.
// - v1.0.0: Workbook reader, provider validation, diff/drift, reference graph, impact, batch/no-apply, fail-closed, 10k/100k benchmark exact8을 최초 구현.
// Migration:
// - Fixture는 in-memory disposable data만 사용하며 Product DataAsset을 읽거나 수정하지 않습니다.

#include "DataAuthoring/CFContentCompiler.h"

#include "DataAuthoring/CFContentCore.h"
#include "DataAuthoring/CFDACommonPrimitives.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"

namespace CFContentCompilerTestsPrivate
{
	/** Test schema field descriptor를 생성합니다. */
	FCFContentFieldDescriptor MakeField(
		const FString& ColumnId,
		const ECFContentValueType ValueType,
		const ECFContentFieldOwnership Ownership,
		const bool bRequired = false,
		const bool bAllowNone = false)
	{
		// 생성할 field descriptor입니다.
		FCFContentFieldDescriptor Field;
		Field.ColumnId = ColumnId;
		Field.DisplayLabel = ColumnId;
		Field.ValueType = ValueType;
		Field.Ownership = Ownership;
		Field.bRequired = bRequired;
		Field.bAllowNone = bAllowNone;
		if (Ownership == ECFContentFieldOwnership::ExternalManaged)
		{
			Field.ExternalOwnerId = TEXT("Test.External");
		}
		return Field;
	}

	/** Canonical String VALUE를 생성합니다. */
	FCFContentValue MakeStringValue(const FString& Value)
	{
		// 생성할 canonical string value입니다.
		FCFContentValue Result;
		Result.Type = ECFContentValueType::String;
		Result.State = ECFContentValueState::Value;
		Result.StringValue = Value;
		return Result;
	}

	/** Canonical Double VALUE를 생성합니다. */
	FCFContentValue MakeDoubleValue(const double Value)
	{
		// 생성할 canonical double value입니다.
		FCFContentValue Result;
		Result.Type = ECFContentValueType::Double;
		Result.State = ECFContentValueState::Value;
		Result.FloatingPointValue = Value;
		return Result;
	}

	/** Canonical SignedInteger VALUE를 생성합니다. */
	FCFContentValue MakeIntegerValue(const int64 Value)
	{
		// 생성할 canonical integer value입니다.
		FCFContentValue Result;
		Result.Type = ECFContentValueType::SignedInteger;
		Result.State = ECFContentValueState::Value;
		Result.SignedIntegerValue = Value;
		return Result;
	}

	/** Optional ContentReference VALUE/NONE을 생성합니다. */
	FCFContentValue MakeReferenceValue(const FString& TargetContentId)
	{
		// 생성할 canonical reference value입니다.
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

	/** P0-02 fixture의 shared primary/child sheet schema를 생성합니다. */
	TArray<FCFContentSheetDescriptor> MakeSchema()
	{
		// Weapon primary logical sheet입니다.
		FCFContentSheetDescriptor MainSheet;
		MainSheet.SheetId = TEXT("Content.Main");
		MainSheet.SchemaRevision = 1;
		MainSheet.ContentTypeId.Value = TEXT("Weapon");
		MainSheet.Fields.Add(MakeField(
			TEXT("DisplayName"),
			ECFContentValueType::String,
			ECFContentFieldOwnership::CCASManaged,
			true));
		MainSheet.Fields.Add(MakeField(
			TEXT("Power"),
			ECFContentValueType::Double,
			ECFContentFieldOwnership::CCASManaged,
			true));
		MainSheet.Fields.Add(MakeField(
			TEXT("Target"),
			ECFContentValueType::ContentReference,
			ECFContentFieldOwnership::CCASManaged,
			false,
			true));
		MainSheet.Fields.Add(MakeField(
			TEXT("ExternalNote"),
			ECFContentValueType::String,
			ECFContentFieldOwnership::ExternalManaged));

		// Ordered child collection logical sheet입니다.
		FCFContentSheetDescriptor ChildSheet;
		ChildSheet.SheetId = TEXT("Content.Children");
		ChildSheet.SchemaRevision = 1;
		ChildSheet.ContentTypeId.Value = TEXT("Weapon");
		ChildSheet.ParentSheetId = TEXT("Content.Main");
		ChildSheet.CollectionId = TEXT("Children");
		ChildSheet.CollectionKind = ECFContentCollectionKind::OrderedList;
		ChildSheet.Fields.Add(MakeField(
			TEXT("Value"),
			ECFContentValueType::SignedInteger,
			ECFContentFieldOwnership::CCASManaged,
			true));

		// Workbook schema sheet 집합입니다.
		TArray<FCFContentSheetDescriptor> Sheets;
		Sheets.Add(MoveTemp(MainSheet));
		Sheets.Add(MoveTemp(ChildSheet));
		return Sheets;
	}

	/** Test record 하나를 canonical model로 생성합니다. */
	FCFContentRecord MakeRecord(
		const FString& ContentId,
		const double Power,
		const FString& TargetContentId,
		const ECFContentManagementState ManagementState = ECFContentManagementState::Managed,
		const FString& ExternalNote = TEXT("external"),
		const int32 ChildCount = 0)
	{
		// 생성할 canonical record입니다.
		FCFContentRecord Record;
		Record.Key.ContentTypeId.Value = TEXT("Weapon");
		Record.Key.ContentId = ContentId;
		Record.RowId = Record.Key.ToStableString();
		Record.ManagementState = ManagementState;
		Record.Fields.Add(TEXT("DisplayName"), MakeStringValue(ContentId));
		Record.Fields.Add(TEXT("Power"), MakeDoubleValue(Power));
		Record.Fields.Add(TEXT("Target"), MakeReferenceValue(TargetContentId));
		Record.Fields.Add(TEXT("ExternalNote"), MakeStringValue(ExternalNote));

		if (ChildCount > 0)
		{
			// Ordered synthetic child collection입니다.
			FCFContentCollection Collection;
			Collection.CollectionId = TEXT("Children");
			Collection.Kind = ECFContentCollectionKind::OrderedList;

			for (int32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
			{
				// Synthetic child item입니다.
				FCFContentCollectionItem Item;
				Item.ChildItemId = FString::Printf(TEXT("Child.%s.%05d"), *ContentId, ChildIndex);
				Item.Order = ChildIndex;
				Item.Fields.Add(TEXT("Value"), MakeIntegerValue(ChildIndex));
				Collection.Items.Add(MoveTemp(Item));
			}

			Record.Collections.Add(TEXT("Children"), MoveTemp(Collection));
		}
		return Record;
	}

	/** Desired/current record array로 canonical Workbook fixture를 생성합니다. */
	FCFContentWorkbookModel MakeWorkbook(const TArray<FCFContentRecord>& Records)
	{
		// 생성할 canonical workbook입니다.
		FCFContentWorkbookModel Workbook;
		Workbook.WorkbookSourceId = TEXT("Test.Compiler.Workbook");
		Workbook.SchemaRevision = 1;
		Workbook.Sheets = MakeSchema();
		Workbook.Records = Records;
		return Workbook;
	}

	/** 기본 desired fixture를 생성합니다. */
	FCFContentWorkbookModel MakeDesiredWorkbook()
	{
		// Desired canonical records입니다.
		TArray<FCFContentRecord> Records;
		Records.Add(MakeRecord(TEXT("Base"), 20.0, TEXT("")));
		Records.Add(MakeRecord(TEXT("Child"), 5.0, TEXT("Base")));
		Records.Add(MakeRecord(TEXT("Consumer"), 1.0, TEXT("Child")));
		Records.Add(MakeRecord(TEXT("Added"), 3.0, TEXT("")));
		Records.Add(MakeRecord(
			TEXT("External"),
			9.0,
			TEXT(""),
			ECFContentManagementState::ExternalReadOnly,
			TEXT("desired external")));
		return MakeWorkbook(Records);
	}

	/** 기본 current Product disposable fixture를 생성합니다. */
	TArray<FCFContentRecord> MakeCurrentRecords()
	{
		// Current Product canonical records입니다.
		TArray<FCFContentRecord> Records;
		Records.Add(MakeRecord(TEXT("Base"), 10.0, TEXT("")));
		Records.Add(MakeRecord(TEXT("Child"), 5.0, TEXT("Base")));
		Records.Add(MakeRecord(TEXT("Consumer"), 1.0, TEXT("Child")));
		Records.Add(MakeRecord(
			TEXT("External"),
			8.0,
			TEXT(""),
			ECFContentManagementState::ExternalReadOnly,
			TEXT("current external")));
		Records.Add(MakeRecord(TEXT("Removed"), 4.0, TEXT("")));
		return Records;
	}

	/** Provider-managed fields만 포함한 deterministic fixture fingerprint를 생성합니다. */
	FString BuildManagedSignature(const FCFContentRecord& Record)
	{
		// Provider-managed semantic signature입니다.
		FString Signature = Record.Key.ToStableString();

		// DisplayName managed field입니다.
		const FCFContentValue* DisplayName = Record.Fields.Find(TEXT("DisplayName"));
		// Power managed field입니다.
		const FCFContentValue* Power = Record.Fields.Find(TEXT("Power"));
		// Target managed reference field입니다.
		const FCFContentValue* Target = Record.Fields.Find(TEXT("Target"));

		Signature += TEXT("|Name=");
		Signature += DisplayName != nullptr ? DisplayName->StringValue : TEXT("<missing>");
		Signature += TEXT("|Power=");
		Signature += Power != nullptr
			? FString::Printf(TEXT("%.17g"), Power->FloatingPointValue)
			: TEXT("<missing>");
		Signature += TEXT("|Target=");
		if (Target != nullptr && Target->State == ECFContentValueState::Value)
		{
			Signature += Target->ContentReference.ToStableString();
		}
		else
		{
			Signature += TEXT("<none>");
		}

		// Managed child collection입니다.
		const FCFContentCollection* Children = Record.Collections.Find(TEXT("Children"));
		if (Children != nullptr)
		{
			// Stable Order 기준 child item pointers입니다.
			TArray<const FCFContentCollectionItem*> OrderedItems;
			for (const FCFContentCollectionItem& Item : Children->Items)
			{
				OrderedItems.Add(&Item);
			}
			OrderedItems.Sort(
				[](const FCFContentCollectionItem& Left, const FCFContentCollectionItem& Right)
				{
					return Left.Order < Right.Order;
				});

			for (const FCFContentCollectionItem* Item : OrderedItems)
			{
				// Managed child value입니다.
				const FCFContentValue* ChildValue = Item->Fields.Find(TEXT("Value"));
				Signature += FString::Printf(
					TEXT("|Child:%d=%lld"),
					Item->Order,
					ChildValue != nullptr ? ChildValue->SignedIntegerValue : -1);
			}
		}
		return Signature;
	}

	/** Provider-managed semantic signature를 canonical SHA-256 fingerprint로 변환합니다. */
	FString BuildManagedFingerprint(const FCFContentRecord& Record)
	{
		// Provider-managed semantic signature입니다.
		const FString Signature = BuildManagedSignature(Record);
		// UTF-8 canonical byte source입니다.
		FTCHARToUTF8 Utf8Signature(*Signature);
		// Hash input bytes입니다.
		TArray<uint8> Bytes;
		Bytes.Append(
			reinterpret_cast<const uint8*>(Utf8Signature.Get()),
			Utf8Signature.Length());

		// Canonical fingerprint 결과입니다.
		FString Fingerprint;
		// Hash failure 상세입니다.
		FString Error;
		if (!CFDACommonPrimitives::HashCanonicalBytes(Bytes, Fingerprint, Error))
		{
			return FString();
		}
		return Fingerprint;
	}

	/** Optional reference를 stable comparable string으로 변환합니다. */
	FString GetTargetSignature(const FCFContentRecord& Record)
	{
		// Target field canonical value입니다.
		const FCFContentValue* Target = Record.Fields.Find(TEXT("Target"));
		if (Target == nullptr || Target->State != ECFContentValueState::Value)
		{
			return TEXT("<none>");
		}
		return Target->ContentReference.ToStableString();
	}

	/** Disposable in-memory Workbook adapter입니다. */
	class FMemoryWorkbookAdapter final : public ICFContentWorkbookAdapter
	{
	public:
		/** Adapter가 반환할 canonical Workbook을 설정합니다. */
		explicit FMemoryWorkbookAdapter(const FCFContentWorkbookModel& InWorkbook)
			: Workbook(InWorkbook)
		{
		}

		/** Stable test adapter metadata를 반환합니다. */
		virtual FCFWorkbookAdapterInfo DescribeAdapter() const override
		{
			// Test adapter metadata입니다.
			FCFWorkbookAdapterInfo Info;
			Info.AdapterId = TEXT("Test.MemoryAdapter");
			Info.AdapterVersion = TEXT("1.0");
			Info.bPreservesPresentation = false;
			Info.bPreservesProtection = false;
			return Info;
		}

		/** In-memory Workbook을 read-only copy합니다. */
		virtual bool ReadWorkbook(
			const FString& WorkbookPath,
			FCFContentWorkbookModel& OutWorkbook,
			FString& OutError) override
		{
			++ReadCount;
			OutError.Reset();
			if (WorkbookPath.IsEmpty() || bFailRead)
			{
				OutError = TEXT("Synthetic adapter read failure.");
				return false;
			}
			OutWorkbook = Workbook;
			return true;
		}

		/** P0-02 compiler가 staged write를 호출하는지 감시합니다. */
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
			OutError = TEXT("Synthetic adapter does not write.");
			return false;
		}

		/** P0-02 compiler가 staged reopen을 호출하는지 감시합니다. */
		virtual bool ReopenWorkbook(
			const FString& StagedWorkbookPath,
			FCFContentWorkbookModel& OutWorkbook,
			FString& OutError) override
		{
			(void)StagedWorkbookPath;
			++ReopenCount;
			OutWorkbook = Workbook;
			OutError = TEXT("Synthetic adapter does not reopen.");
			return false;
		}

		// Adapter read source Workbook입니다.
		FCFContentWorkbookModel Workbook;

		// ReadWorkbook 호출 수입니다.
		int32 ReadCount = 0;

		// WriteStagedWorkbook 호출 수입니다.
		int32 WriteCount = 0;

		// ReopenWorkbook 호출 수입니다.
		int32 ReopenCount = 0;

		// Read fail-closed fixture flag입니다.
		bool bFailRead = false;
	};

	/** Disposable provider로 P0-02 provider-backed semantics를 검증합니다. */
	class FMemoryContentProvider final : public ICFContentProvider
	{
	public:
		/** Provider schema와 current Product fixture를 설정합니다. */
		FMemoryContentProvider(
			const TArray<FCFContentSheetDescriptor>& InSchemaSheets,
			const TArray<FCFContentRecord>& InCurrentRecords)
			: SchemaSheets(InSchemaSheets)
		{
			for (const FCFContentRecord& Record : InCurrentRecords)
			{
				CurrentRecordsByKey.Add(Record.Key.ToStableString(), Record);
			}
		}

		/** Stable Provider descriptor를 반환합니다. */
		virtual bool DescribeSchema(
			FCFContentProviderDescriptor& OutDescriptor,
			FString& OutError) const override
		{
			OutError.Reset();
			OutDescriptor = FCFContentProviderDescriptor();
			OutDescriptor.ProviderId = TEXT("Test.Compiler.Provider");
			OutDescriptor.ContentTypeId.Value = TEXT("Weapon");
			OutDescriptor.SchemaRevision = 1;
			OutDescriptor.Sheets = SchemaSheets;
			return true;
		}

		/** Current Product disposable records를 read-only import합니다. */
		virtual bool ImportExisting(
			FCFContentImportResult& OutImport,
			FString& OutError) const override
		{
			OutError.Reset();
			OutImport = FCFContentImportResult();
			++ImportCallCount;
			for (const TPair<FString, FCFContentRecord>& Pair : CurrentRecordsByKey)
			{
				// 이번 import에서 반환할 disposable current record입니다.
				FCFContentRecord ImportedRecord = Pair.Value;
				if (bChangeOnSecondImport
					&& ImportCallCount >= 2
					&& ImportedRecord.Key.ContentId.Equals(
						TEXT("Removed"),
						ESearchCase::CaseSensitive))
				{
					// Preview 도중 retire 후보 current record가 변경된 상황을 모사합니다.
					ImportedRecord.Fields[TEXT("Power")] = MakeDoubleValue(444.0);
				}
				OutImport.Records.Add(MoveTemp(ImportedRecord));
			}
			return true;
		}

		/** Exact current fixture snapshot을 O(1) index로 반환합니다. */
		virtual bool BuildCurrentSnapshot(
			const FCFContentKey& Key,
			FCFContentSnapshot& OutSnapshot,
			FString& OutError) const override
		{
			OutError.Reset();

			// Requested current record입니다.
			const FCFContentRecord* Record = CurrentRecordsByKey.Find(Key.ToStableString());
			if (Record == nullptr)
			{
				OutError = TEXT("Current fixture key not found.");
				return false;
			}

			OutSnapshot = FCFContentSnapshot();
			OutSnapshot.Key = Key;
			OutSnapshot.Record = *Record;
			if (bReturnInconsistentSnapshot
				&& Key.ContentId.Equals(TEXT("Base"), ESearchCase::CaseSensitive))
			{
				// ImportExisting 뒤 Product가 바뀐 상황을 모사하는 snapshot-only Power 변경입니다.
				OutSnapshot.Record.Fields[TEXT("Power")] = MakeDoubleValue(999.0);
			}
			OutSnapshot.ReadbackFingerprint = BuildManagedFingerprint(OutSnapshot.Record);
			return true;
		}

		/** Provider-specific semantic invariant를 검증합니다. */
		virtual bool ValidateRecord(
			const FCFContentRecord& Record,
			TArray<FCFContentValidationIssue>& OutIssues) const override
		{
			OutIssues.Reset();

			// Power managed field입니다.
			const FCFContentValue* Power = Record.Fields.Find(TEXT("Power"));
			if (Power == nullptr || Power->State != ECFContentValueState::Value)
			{
				// Missing Power diagnostic입니다.
				FCFContentValidationIssue Issue;
				Issue.Code = TEXT("MissingPower");
				Issue.Path = Record.Key.ToStableString() + TEXT(".Power");
				Issue.Message = TEXT("Test Provider는 Power VALUE가 필요합니다.");
				OutIssues.Add(MoveTemp(Issue));
				return false;
			}
			if (Power->FloatingPointValue < 0.0)
			{
				// Negative Power diagnostic입니다.
				FCFContentValidationIssue Issue;
				Issue.Code = TEXT("NegativePower");
				Issue.Path = Record.Key.ToStableString() + TEXT(".Power");
				Issue.Message = TEXT("Test Provider는 음수 Power를 허용하지 않습니다.");
				OutIssues.Add(MoveTemp(Issue));
				return false;
			}
			return true;
		}

		/** Target ContentReference에서 dependency edge를 생성합니다. */
		virtual bool BuildDependencyEdges(
			const FCFContentRecord& Record,
			TArray<FCFContentDependencyEdge>& OutEdges,
			FString& OutError) const override
		{
			OutEdges.Reset();
			OutError.Reset();

			// Optional Target reference입니다.
			const FCFContentValue* Target = Record.Fields.Find(TEXT("Target"));
			if (Target != nullptr && Target->State == ECFContentValueState::Value)
			{
				// Target field dependency edge입니다.
				FCFContentDependencyEdge Edge;
				Edge.From = Record.Key;
				Edge.To = Target->ContentReference;
				Edge.FieldPath = TEXT("Target");
				OutEdges.Add(MoveTemp(Edge));
			}
			return true;
		}

		/** Provider-managed fields만 비교해 typed diff를 생성합니다. */
		virtual bool BuildDiff(
			const FCFContentSnapshot& Current,
			const FCFContentRecord& Desired,
			FCFContentDiff& OutDiff,
			FString& OutError) const override
		{
			OutError.Reset();
			OutDiff = FCFContentDiff();
			OutDiff.Key = Desired.Key;
			OutDiff.CurrentFingerprint = bReturnInvalidFingerprints
				? FString()
				: (bReturnConflictingCurrentFingerprint
					? FString(TEXT("sha256:0000000000000000000000000000000000000000000000000000000000000000"))
					: BuildManagedFingerprint(Current.Record));
			OutDiff.DesiredFingerprint = bReturnInvalidFingerprints
				? TEXT("not-a-canonical-fingerprint")
				: BuildManagedFingerprint(Desired);

			// Current managed DisplayName입니다.
			const FCFContentValue* CurrentName = Current.Record.Fields.Find(TEXT("DisplayName"));
			// Desired managed DisplayName입니다.
			const FCFContentValue* DesiredName = Desired.Fields.Find(TEXT("DisplayName"));
			if (CurrentName == nullptr
				|| DesiredName == nullptr
				|| !CurrentName->StringValue.Equals(DesiredName->StringValue, ESearchCase::CaseSensitive))
			{
				OutDiff.ChangedPaths.Add(TEXT("DisplayName"));
			}

			// Current managed Power입니다.
			const FCFContentValue* CurrentPower = Current.Record.Fields.Find(TEXT("Power"));
			// Desired managed Power입니다.
			const FCFContentValue* DesiredPower = Desired.Fields.Find(TEXT("Power"));
			if (CurrentPower == nullptr
				|| DesiredPower == nullptr
				|| CurrentPower->FloatingPointValue != DesiredPower->FloatingPointValue)
			{
				OutDiff.ChangedPaths.Add(TEXT("Power"));
			}

			if (!GetTargetSignature(Current.Record).Equals(
				GetTargetSignature(Desired),
				ESearchCase::CaseSensitive))
			{
				OutDiff.ChangedPaths.Add(TEXT("Target"));
			}

			// Current/desired managed child signatures입니다.
			const FString CurrentSignature = BuildManagedSignature(Current.Record);
			// Desired managed child-inclusive signature입니다.
			const FString DesiredSignature = BuildManagedSignature(Desired);
			if (!CurrentSignature.Equals(DesiredSignature, ESearchCase::CaseSensitive)
				&& OutDiff.ChangedPaths.IsEmpty())
			{
				OutDiff.ChangedPaths.Add(TEXT("Children"));
			}
			return Current.Key == Desired.Key;
		}

		/** P0-02에서 reviewed mutation plan 생성은 사용하지 않지만 interface contract를 구현합니다. */
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
			OutPlan.ChangedPaths = Diff.ChangedPaths;
			OutPlan.bApproved = false;
			return true;
		}

		/** Product Apply가 잘못 호출되면 call count를 남기고 fail-closed 합니다. */
		virtual bool ApplyReviewed(
			const FCFContentReviewedMutationPlan& ReviewedPlan,
			FCFContentApplyResult& OutResult) override
		{
			(void)ReviewedPlan;
			++ApplyCallCount;
			OutResult = FCFContentApplyResult();
			OutResult.Error = TEXT("P0-02 disposable provider has no Product writer.");
			return false;
		}

		/** Exact current fixture의 provider-managed fingerprint를 반환합니다. */
		virtual bool ReadbackFingerprint(
			const FCFContentKey& Key,
			FString& OutFingerprint,
			FString& OutError) const override
		{
			OutError.Reset();

			// Requested current record입니다.
			const FCFContentRecord* Record = CurrentRecordsByKey.Find(Key.ToStableString());
			if (Record == nullptr)
			{
				OutError = TEXT("Current fixture key not found.");
				return false;
			}
			OutFingerprint = bReturnInvalidFingerprints
				? FString()
				: BuildManagedFingerprint(*Record);
			return true;
		}

		// Provider-owned schema sheets입니다.
		TArray<FCFContentSheetDescriptor> SchemaSheets;

		// Current Product disposable record O(1) index입니다.
		TMap<FString, FCFContentRecord> CurrentRecordsByKey;

		// Product ApplyReviewed 호출 감시 count입니다.
		int32 ApplyCallCount = 0;

		// Provider가 invalid/empty fingerprint를 반환하도록 만드는 fail-closed fixture flag입니다.
		bool bReturnInvalidFingerprints = false;

		// BuildDiff current fingerprint를 snapshot readback과 다른 canonical 값으로 반환하는 fixture flag입니다.
		bool bReturnConflictingCurrentFingerprint = false;

		// ImportExisting 이후 BuildCurrentSnapshot에서 다른 canonical state를 반환하는 fixture flag입니다.
		bool bReturnInconsistentSnapshot = false;

		// 두 번째 ImportExisting에서 retire 후보 record를 바꾸는 whole-current consistency fixture flag입니다.
		bool bChangeOnSecondImport = false;

		// ImportExisting 호출 횟수입니다.
		mutable int32 ImportCallCount = 0;
	};

	/** Workbook/provider fixture를 registry에 등록합니다. */
	bool RegisterProvider(
		const FCFContentWorkbookModel& Workbook,
		const TArray<FCFContentRecord>& CurrentRecords,
		FCFContentProviderRegistry& OutRegistry,
		TSharedPtr<FMemoryContentProvider>& OutProvider,
		FString& OutError)
	{
		// Disposable provider shared instance입니다.
		TSharedRef<FMemoryContentProvider> Provider =
			MakeShared<FMemoryContentProvider>(Workbook.Sheets, CurrentRecords);
		if (!OutRegistry.RegisterProvider(Provider, OutError))
		{
			return false;
		}
		OutProvider = Provider;
		return true;
	}

	/** Compile result에서 exact key의 diff를 찾습니다. */
	const FCFContentCompileDiff* FindDiff(
		const FCFContentCompileResult& Result,
		const FString& ContentId)
	{
		return Result.Diffs.FindByPredicate(
			[&ContentId](const FCFContentCompileDiff& Diff)
			{
				return Diff.Key.ContentId.Equals(ContentId, ESearchCase::CaseSensitive);
			});
	}

	/** Compile result에서 exact key의 generated fingerprint를 찾습니다. */
	const FCFContentGeneratedFingerprint* FindGenerated(
		const FCFContentCompileResult& Result,
		const FString& ContentId)
	{
		return Result.GeneratedFingerprints.FindByPredicate(
			[&ContentId](const FCFContentGeneratedFingerprint& Entry)
			{
				return Entry.Key.ContentId.Equals(ContentId, ESearchCase::CaseSensitive);
			});
	}

	/** Compile result에서 exact changed root의 impact를 찾습니다. */
	const FCFContentImpactEntry* FindImpact(
		const FCFContentCompileResult& Result,
		const FString& ContentId)
	{
		return Result.Impacts.FindByPredicate(
			[&ContentId](const FCFContentImpactEntry& Impact)
			{
				return Impact.ChangedKey.ContentId.Equals(ContentId, ESearchCase::CaseSensitive);
			});
	}

	/** 10k content + 100k child synthetic textual cells를 실제 canonical parse해서 benchmark fixture를 생성합니다. */
	bool MakeScaleFixture(
		FCFContentWorkbookModel& OutWorkbook,
		TArray<FCFContentRecord>& OutCurrentRecords,
		double& OutParseSeconds,
		FString& OutError)
	{
		OutParseSeconds = 0.0;
		OutError.Reset();

		// Benchmark Power textual cell parser descriptor입니다.
		const FCFContentFieldDescriptor PowerField = MakeField(
			TEXT("Power"),
			ECFContentValueType::Double,
			ECFContentFieldOwnership::CCASManaged,
			true);
		// Benchmark child Value textual cell parser descriptor입니다.
		const FCFContentFieldDescriptor ChildValueField = MakeField(
			TEXT("Value"),
			ECFContentValueType::SignedInteger,
			ECFContentFieldOwnership::CCASManaged,
			true);
		// Textual canonicalization benchmark 시작 timestamp입니다.
		const double ParseStartSeconds = FPlatformTime::Seconds();

		// Benchmark desired records입니다.
		TArray<FCFContentRecord> DesiredRecords;
		DesiredRecords.Reserve(10000);
		OutCurrentRecords.Reset();
		OutCurrentRecords.Reserve(10000);

		for (int32 RecordIndex = 0; RecordIndex < 10000; ++RecordIndex)
		{
			// Stable synthetic content ID입니다.
			const FString ContentId = FString::Printf(TEXT("Bench%05d"), RecordIndex);
			// 10 child rows를 가진 synthetic record입니다.
			FCFContentRecord Record = MakeRecord(
				ContentId,
				0.0,
				TEXT(""),
				ECFContentManagementState::Managed,
				TEXT("benchmark external"),
				10);

			// Textual Power cell parse 결과입니다.
			FCFContentValue ParsedPower;
			// Textual Power source입니다.
			const FString PowerText = FString::Printf(TEXT("%d.25"), RecordIndex);
			if (!FCFContentCanonicalizer::ParseCell(
				PowerText,
				PowerField,
				ParsedPower,
				OutError))
			{
				return false;
			}
			Record.Fields[TEXT("Power")] = MoveTemp(ParsedPower);

			// Synthetic ordered child collection입니다.
			FCFContentCollection* Children = Record.Collections.Find(TEXT("Children"));
			if (Children == nullptr || Children->Items.Num() != 10)
			{
				OutError = TEXT("Synthetic benchmark child collection shape mismatch.");
				return false;
			}

			for (int32 ChildIndex = 0; ChildIndex < Children->Items.Num(); ++ChildIndex)
			{
				// Textual child integer parse 결과입니다.
				FCFContentValue ParsedChildValue;
				// Stable synthetic child numeric source입니다.
				const FString ChildValueText = FString::FromInt(RecordIndex * 10 + ChildIndex);
				if (!FCFContentCanonicalizer::ParseCell(
					ChildValueText,
					ChildValueField,
					ParsedChildValue,
					OutError))
				{
					return false;
				}
				Children->Items[ChildIndex].Fields[TEXT("Value")] = MoveTemp(ParsedChildValue);
			}

			DesiredRecords.Add(Record);
			OutCurrentRecords.Add(MoveTemp(Record));
		}

		OutParseSeconds = FPlatformTime::Seconds() - ParseStartSeconds;
		OutWorkbook = MakeWorkbook(DesiredRecords);
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFCompilerWorkbookReaderTest,
	"CarFight.CCAS.CF_FQ_058.P0_02.WorkbookReader",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFCompilerProviderValidationTest,
	"CarFight.CCAS.CF_FQ_058.P0_02.ProviderValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFCompilerDiffDriftTest,
	"CarFight.CCAS.CF_FQ_058.P0_02.DiffDrift",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFCompilerReferenceGraphTest,
	"CarFight.CCAS.CF_FQ_058.P0_02.ReferenceGraph",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFCompilerImpactAnalysisTest,
	"CarFight.CCAS.CF_FQ_058.P0_02.ImpactAnalysis",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFCompilerBatchNoApplyTest,
	"CarFight.CCAS.CF_FQ_058.P0_02.BatchNoApply",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFCompilerFailClosedTest,
	"CarFight.CCAS.CF_FQ_058.P0_02.FailClosed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFCompilerScaleBenchmarkTest,
	"CarFight.CCAS.CF_FQ_058.P0_02.ScaleBenchmark",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFCompilerCurrentRetireImpactTest,
	"CarFight.CCAS.CF_FQ_058.P0_02.CurrentRetireImpact",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFCompilerFingerprintGuardTest,
	"CarFight.CCAS.CF_FQ_058.P0_02.FingerprintGuard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFCompilerSchemaCompatibilityTest,
	"CarFight.CCAS.CF_FQ_058.P0_02.SchemaCompatibility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFCompilerSnapshotConsistencyTest,
	"CarFight.CCAS.CF_FQ_058.P0_02.SnapshotConsistency",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

/** Workbook adapter read-only route와 source semantic hash를 검증합니다. */
bool FCFCompilerWorkbookReaderTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Desired Workbook fixture입니다.
	const FCFContentWorkbookModel Workbook = CFContentCompilerTestsPrivate::MakeDesiredWorkbook();
	// Current Product fixture입니다.
	const TArray<FCFContentRecord> CurrentRecords = CFContentCompilerTestsPrivate::MakeCurrentRecords();
	// Disposable adapter입니다.
	CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(Workbook);
	// Provider registry입니다.
	FCFContentProviderRegistry Registry;
	// Registered disposable provider입니다.
	TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
	// Provider registration error입니다.
	FString Error;
	TestTrue(TEXT("Provider registration must succeed"),
		CFContentCompilerTestsPrivate::RegisterProvider(Workbook, CurrentRecords, Registry, Provider, Error));

	// Compiler preview result입니다.
	FCFContentCompileResult Result;
	TestTrue(TEXT("Generic Compiler preview must succeed"),
		FCFContentCompiler::CompilePreview(TEXT("Memory://CompilerFixture"), Adapter, Registry, Result));

	TestTrue(TEXT("Compiler result must be succeeded"), Result.bSucceeded);
	TestEqual(TEXT("Adapter read exact1"), Adapter.ReadCount, 1);
	TestEqual(TEXT("Adapter staged write must stay zero"), Adapter.WriteCount, 0);
	TestEqual(TEXT("Adapter reopen must stay zero"), Adapter.ReopenCount, 0);
	TestTrue(TEXT("Workbook semantic hash must be canonical sha256"),
		Result.WorkbookSemanticHash.StartsWith(TEXT("sha256:")));
	TestEqual(TEXT("Compiler must preserve WorkbookSourceId"),
		Result.Workbook.WorkbookSourceId,
		FString(TEXT("Test.Compiler.Workbook")));
	return true;
}

/** Provider semantic validation이 blocking issue를 fail-closed 하는지 검증합니다. */
bool FCFCompilerProviderValidationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Invalid desired Workbook fixture입니다.
	FCFContentWorkbookModel Workbook = CFContentCompilerTestsPrivate::MakeDesiredWorkbook();
	Workbook.Records[0].Fields[TEXT("Power")] =
		CFContentCompilerTestsPrivate::MakeDoubleValue(-1.0);

	// Disposable adapter입니다.
	CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(Workbook);
	// Provider registry입니다.
	FCFContentProviderRegistry Registry;
	// Registered disposable provider입니다.
	TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
	// Provider registration error입니다.
	FString Error;
	TestTrue(TEXT("Provider registration must succeed"),
		CFContentCompilerTestsPrivate::RegisterProvider(
			Workbook,
			CFContentCompilerTestsPrivate::MakeCurrentRecords(),
			Registry,
			Provider,
			Error));

	// Compiler preview result입니다.
	FCFContentCompileResult Result;
	TestFalse(TEXT("Negative provider semantic value must fail"),
		FCFContentCompiler::CompilePreview(TEXT("Memory://InvalidProvider"), Adapter, Registry, Result));
	TestEqual(TEXT("Failure stage must be ProviderValidation"),
		Result.FailureStage,
		FString(TEXT("ProviderValidation")));

	// NegativePower issue 존재 여부입니다.
	const bool bHasNegativePower = Result.Issues.ContainsByPredicate(
		[](const FCFContentValidationIssue& Issue)
		{
			return Issue.Code.Equals(TEXT("NegativePower"), ESearchCase::CaseSensitive);
		});
	TestTrue(TEXT("Provider diagnostic must be preserved"), bHasNegativePower);
	TestEqual(TEXT("Provider Apply must stay zero"), Provider->ApplyCallCount, 0);
	return true;
}

/** Added/Removed/Modified/Unchanged와 provider-scoped drift를 검증합니다. */
bool FCFCompilerDiffDriftTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Desired Workbook fixture입니다.
	const FCFContentWorkbookModel Workbook = CFContentCompilerTestsPrivate::MakeDesiredWorkbook();
	// Disposable adapter입니다.
	CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(Workbook);
	// Provider registry입니다.
	FCFContentProviderRegistry Registry;
	// Registered disposable provider입니다.
	TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
	// Provider registration error입니다.
	FString Error;
	TestTrue(TEXT("Provider registration must succeed"),
		CFContentCompilerTestsPrivate::RegisterProvider(
			Workbook,
			CFContentCompilerTestsPrivate::MakeCurrentRecords(),
			Registry,
			Provider,
			Error));

	// Compiler preview result입니다.
	FCFContentCompileResult Result;
	TestTrue(TEXT("Diff preview must succeed"),
		FCFContentCompiler::CompilePreview(TEXT("Memory://DiffFixture"), Adapter, Registry, Result));

	// Modified Base diff입니다.
	const FCFContentCompileDiff* BaseDiff =
		CFContentCompilerTestsPrivate::FindDiff(Result, TEXT("Base"));
	// Added diff입니다.
	const FCFContentCompileDiff* AddedDiff =
		CFContentCompilerTestsPrivate::FindDiff(Result, TEXT("Added"));
	// Removed diff입니다.
	const FCFContentCompileDiff* RemovedDiff =
		CFContentCompilerTestsPrivate::FindDiff(Result, TEXT("Removed"));
	// Unchanged Child diff입니다.
	const FCFContentCompileDiff* ChildDiff =
		CFContentCompilerTestsPrivate::FindDiff(Result, TEXT("Child"));
	TestNotNull(TEXT("Base diff must exist"), BaseDiff);
	TestNotNull(TEXT("Added diff must exist"), AddedDiff);
	TestNotNull(TEXT("Removed diff must exist"), RemovedDiff);
	TestNotNull(TEXT("Child diff must exist"), ChildDiff);
	TestTrue(TEXT("Base must be Modified"), BaseDiff != nullptr && BaseDiff->Kind == ECFContentCompileDiffKind::Modified);
	TestTrue(TEXT("Added must be Added"), AddedDiff != nullptr && AddedDiff->Kind == ECFContentCompileDiffKind::Added);
	TestTrue(TEXT("Removed must be Removed"), RemovedDiff != nullptr && RemovedDiff->Kind == ECFContentCompileDiffKind::Removed);
	TestTrue(TEXT("Child must be Unchanged"), ChildDiff != nullptr && ChildDiff->Kind == ECFContentCompileDiffKind::Unchanged);

	// Base drift provenance입니다.
	const FCFContentGeneratedFingerprint* BaseGenerated =
		CFContentCompilerTestsPrivate::FindGenerated(Result, TEXT("Base"));
	// Added drift provenance입니다.
	const FCFContentGeneratedFingerprint* AddedGenerated =
		CFContentCompilerTestsPrivate::FindGenerated(Result, TEXT("Added"));
	// Removed drift provenance입니다.
	const FCFContentGeneratedFingerprint* RemovedGenerated =
		CFContentCompilerTestsPrivate::FindGenerated(Result, TEXT("Removed"));
	// External read-only provenance입니다.
	const FCFContentGeneratedFingerprint* ExternalGenerated =
		CFContentCompilerTestsPrivate::FindGenerated(Result, TEXT("External"));
	TestTrue(TEXT("Base drift must be Changed"),
		BaseGenerated != nullptr && BaseGenerated->DriftState == ECFContentDriftState::Changed);
	TestTrue(TEXT("Added drift must be MissingCurrent"),
		AddedGenerated != nullptr && AddedGenerated->DriftState == ECFContentDriftState::MissingCurrent);
	TestTrue(TEXT("Removed drift must be RetiredCurrent"),
		RemovedGenerated != nullptr && RemovedGenerated->DriftState == ECFContentDriftState::RetiredCurrent);
	TestTrue(TEXT("External record must stay ExternalReadOnly"),
		ExternalGenerated != nullptr && ExternalGenerated->DriftState == ECFContentDriftState::ExternalReadOnly);
	return true;
}

/** Provider reference edges가 validated graph로 수집되는지 검증합니다. */
bool FCFCompilerReferenceGraphTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Desired Workbook fixture입니다.
	const FCFContentWorkbookModel Workbook = CFContentCompilerTestsPrivate::MakeDesiredWorkbook();
	// Disposable adapter입니다.
	CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(Workbook);
	// Provider registry입니다.
	FCFContentProviderRegistry Registry;
	// Registered disposable provider입니다.
	TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
	// Provider registration error입니다.
	FString Error;
	TestTrue(TEXT("Provider registration must succeed"),
		CFContentCompilerTestsPrivate::RegisterProvider(
			Workbook,
			CFContentCompilerTestsPrivate::MakeCurrentRecords(),
			Registry,
			Provider,
			Error));

	// Compiler preview result입니다.
	FCFContentCompileResult Result;
	TestTrue(TEXT("Reference graph preview must succeed"),
		FCFContentCompiler::CompilePreview(TEXT("Memory://ReferenceFixture"), Adapter, Registry, Result));
	TestEqual(TEXT("Reference graph must have exact2 edges"), Result.DependencyEdges.Num(), 2);

	// Child -> Base edge 존재 여부입니다.
	const bool bHasChildToBase = Result.DependencyEdges.ContainsByPredicate(
		[](const FCFContentDependencyEdge& Edge)
		{
			return Edge.From.ContentId == TEXT("Child") && Edge.To.ContentId == TEXT("Base");
		});
	// Consumer -> Child edge 존재 여부입니다.
	const bool bHasConsumerToChild = Result.DependencyEdges.ContainsByPredicate(
		[](const FCFContentDependencyEdge& Edge)
		{
			return Edge.From.ContentId == TEXT("Consumer") && Edge.To.ContentId == TEXT("Child");
		});
	TestTrue(TEXT("Child must reference Base"), bHasChildToBase);
	TestTrue(TEXT("Consumer must reference Child"), bHasConsumerToChild);
	return true;
}

/** Changed root의 direct/transitive referencer impact를 검증합니다. */
bool FCFCompilerImpactAnalysisTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Desired Workbook fixture입니다.
	const FCFContentWorkbookModel Workbook = CFContentCompilerTestsPrivate::MakeDesiredWorkbook();
	// Disposable adapter입니다.
	CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(Workbook);
	// Provider registry입니다.
	FCFContentProviderRegistry Registry;
	// Registered disposable provider입니다.
	TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
	// Provider registration error입니다.
	FString Error;
	TestTrue(TEXT("Provider registration must succeed"),
		CFContentCompilerTestsPrivate::RegisterProvider(
			Workbook,
			CFContentCompilerTestsPrivate::MakeCurrentRecords(),
			Registry,
			Provider,
			Error));

	// Compiler preview result입니다.
	FCFContentCompileResult Result;
	TestTrue(TEXT("Impact preview must succeed"),
		FCFContentCompiler::CompilePreview(TEXT("Memory://ImpactFixture"), Adapter, Registry, Result));

	// Base changed-root impact입니다.
	const FCFContentImpactEntry* BaseImpact =
		CFContentCompilerTestsPrivate::FindImpact(Result, TEXT("Base"));
	TestNotNull(TEXT("Base impact must exist"), BaseImpact);
	TestEqual(TEXT("Base direct dependents exact1"),
		BaseImpact != nullptr ? BaseImpact->DirectDependents.Num() : 0,
		1);
	TestEqual(TEXT("Base transitive dependents exact2"),
		BaseImpact != nullptr ? BaseImpact->TransitiveDependents.Num() : 0,
		2);
	TestTrue(TEXT("Base direct dependent must be Child"),
		BaseImpact != nullptr
			&& BaseImpact->DirectDependents[0].ContentId == TEXT("Child"));
	return true;
}

/** Preview batch 집계와 Product Apply exact0 guard를 검증합니다. */
bool FCFCompilerBatchNoApplyTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Desired Workbook fixture입니다.
	const FCFContentWorkbookModel Workbook = CFContentCompilerTestsPrivate::MakeDesiredWorkbook();
	// Disposable adapter입니다.
	CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(Workbook);
	// Provider registry입니다.
	FCFContentProviderRegistry Registry;
	// Registered disposable provider입니다.
	TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
	// Provider registration error입니다.
	FString Error;
	TestTrue(TEXT("Provider registration must succeed"),
		CFContentCompilerTestsPrivate::RegisterProvider(
			Workbook,
			CFContentCompilerTestsPrivate::MakeCurrentRecords(),
			Registry,
			Provider,
			Error));

	// Compiler preview result입니다.
	FCFContentCompileResult Result;
	TestTrue(TEXT("Batch preview must succeed"),
		FCFContentCompiler::CompilePreview(TEXT("Memory://BatchFixture"), Adapter, Registry, Result));

	TestEqual(TEXT("Desired count exact5"), Result.BatchResult.DesiredRecordCount, 5);
	TestEqual(TEXT("Current count exact5"), Result.BatchResult.CurrentRecordCount, 5);
	TestEqual(TEXT("ReadyToReview exact2"), Result.BatchResult.ReadyToReviewCount, 2);
	TestEqual(TEXT("RetireCandidate exact1"), Result.BatchResult.RetireCandidateCount, 1);
	TestEqual(TEXT("AlreadyCurrent exact2"), Result.BatchResult.AlreadyCurrentCount, 2);
	TestEqual(TEXT("ExternalReadOnly exact1"), Result.BatchResult.ExternalReadOnlyCount, 1);
	TestEqual(TEXT("Blocked exact0"), Result.BatchResult.BlockedCount, 0);
	TestEqual(TEXT("Provider ApplyReviewed must remain exact0"), Provider->ApplyCallCount, 0);
	TestEqual(TEXT("Workbook staged write must remain exact0"), Adapter.WriteCount, 0);
	TestEqual(TEXT("Workbook reopen must remain exact0"), Adapter.ReopenCount, 0);
	return true;
}

/** Missing reference를 ReferenceValidation에서 fail-closed 하는지 검증합니다. */
bool FCFCompilerFailClosedTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Missing target를 가진 desired Workbook입니다.
	FCFContentWorkbookModel Workbook = CFContentCompilerTestsPrivate::MakeDesiredWorkbook();
	Workbook.Records[1].Fields[TEXT("Target")] =
		CFContentCompilerTestsPrivate::MakeReferenceValue(TEXT("Missing"));

	// Disposable adapter입니다.
	CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(Workbook);
	// Provider registry입니다.
	FCFContentProviderRegistry Registry;
	// Registered disposable provider입니다.
	TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
	// Provider registration error입니다.
	FString Error;
	TestTrue(TEXT("Provider registration must succeed"),
		CFContentCompilerTestsPrivate::RegisterProvider(
			Workbook,
			CFContentCompilerTestsPrivate::MakeCurrentRecords(),
			Registry,
			Provider,
			Error));

	// Compiler preview result입니다.
	FCFContentCompileResult Result;
	TestFalse(TEXT("Missing target must fail closed"),
		FCFContentCompiler::CompilePreview(TEXT("Memory://MissingTarget"), Adapter, Registry, Result));
	TestEqual(TEXT("Missing target failure stage"),
		Result.FailureStage,
		FString(TEXT("ReferenceValidation")));
	TestEqual(TEXT("Product Apply must remain exact0 on failure"), Provider->ApplyCallCount, 0);
	return true;
}

/** Current-only referencer가 retire impact에서 사라지지 않는지 검증합니다. */
bool FCFCompilerCurrentRetireImpactTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Desired Workbook은 Consumer가 Child를 참조하고 Removed는 존재하지 않습니다.
	const FCFContentWorkbookModel Workbook = CFContentCompilerTestsPrivate::MakeDesiredWorkbook();
	// Current Product fixture입니다.
	TArray<FCFContentRecord> CurrentRecords = CFContentCompilerTestsPrivate::MakeCurrentRecords();
	// Current Consumer record입니다.
	FCFContentRecord* CurrentConsumer = CurrentRecords.FindByPredicate(
		[](const FCFContentRecord& Record)
		{
			return Record.Key.ContentId.Equals(TEXT("Consumer"), ESearchCase::CaseSensitive);
		});
	TestNotNull(TEXT("Current Consumer fixture must exist"), CurrentConsumer);
	if (CurrentConsumer == nullptr)
	{
		return false;
	}
	CurrentConsumer->Fields[TEXT("Target")] =
		CFContentCompilerTestsPrivate::MakeReferenceValue(TEXT("Removed"));

	// Disposable adapter입니다.
	CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(Workbook);
	// Provider registry입니다.
	FCFContentProviderRegistry Registry;
	// Registered disposable provider입니다.
	TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
	// Provider registration error입니다.
	FString Error;
	TestTrue(TEXT("Provider registration must succeed"),
		CFContentCompilerTestsPrivate::RegisterProvider(
			Workbook,
			CurrentRecords,
			Registry,
			Provider,
			Error));

	// Compiler preview result입니다.
	FCFContentCompileResult Result;
	TestTrue(TEXT("Current-only retire referencer preview must succeed"),
		FCFContentCompiler::CompilePreview(
			TEXT("Memory://CurrentRetireImpact"),
			Adapter,
			Registry,
			Result));

	// Current graph에서 Consumer -> Removed edge가 보존됐는지 확인합니다.
	const bool bHasCurrentConsumerToRemoved = Result.CurrentDependencyEdges.ContainsByPredicate(
		[](const FCFContentDependencyEdge& Edge)
		{
			return Edge.From.ContentId == TEXT("Consumer")
				&& Edge.To.ContentId == TEXT("Removed");
		});
	TestTrue(TEXT("Current reference graph must preserve Consumer -> Removed"), bHasCurrentConsumerToRemoved);

	// Removed retire impact입니다.
	const FCFContentImpactEntry* RemovedImpact =
		CFContentCompilerTestsPrivate::FindImpact(Result, TEXT("Removed"));
	TestNotNull(TEXT("Removed retire impact must exist"), RemovedImpact);
	const bool bConsumerIsDirectReferencer = RemovedImpact != nullptr
		&& RemovedImpact->DirectDependents.ContainsByPredicate(
			[](const FCFContentKey& Key)
			{
				return Key.ContentId.Equals(TEXT("Consumer"), ESearchCase::CaseSensitive);
			});
	TestTrue(TEXT("Removed retire impact must include current-only Consumer referencer"),
		bConsumerIsDirectReferencer);
	TestEqual(TEXT("Product Apply must remain exact0"), Provider->ApplyCallCount, 0);
	return true;
}

/** Invalid/empty 또는 서로 모순된 Provider fingerprint가 fail-closed 되는지 검증합니다. */
bool FCFCompilerFingerprintGuardTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Desired Workbook fixture입니다.
	const FCFContentWorkbookModel Workbook = CFContentCompilerTestsPrivate::MakeDesiredWorkbook();

	{
		// Invalid fingerprint subcase adapter입니다.
		CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(Workbook);
		// Invalid fingerprint subcase registry입니다.
		FCFContentProviderRegistry Registry;
		// Invalid fingerprint subcase provider입니다.
		TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
		// Provider registration error입니다.
		FString Error;
		TestTrue(TEXT("Invalid-fingerprint provider registration must succeed"),
			CFContentCompilerTestsPrivate::RegisterProvider(
				Workbook,
				CFContentCompilerTestsPrivate::MakeCurrentRecords(),
				Registry,
				Provider,
				Error));
		Provider->bReturnInvalidFingerprints = true;

		// Invalid fingerprint compiler result입니다.
		FCFContentCompileResult Result;
		TestFalse(TEXT("Invalid provider fingerprints must fail closed"),
			FCFContentCompiler::CompilePreview(
				TEXT("Memory://InvalidFingerprint"),
				Adapter,
				Registry,
				Result));
		TestEqual(TEXT("Invalid fingerprint failure stage must be Diff"),
			Result.FailureStage,
			FString(TEXT("Diff")));

		// InvalidProviderFingerprint diagnostic 존재 여부입니다.
		const bool bHasInvalidFingerprint = Result.Issues.ContainsByPredicate(
			[](const FCFContentValidationIssue& Issue)
			{
				return Issue.Code.Equals(
					TEXT("InvalidProviderFingerprint"),
					ESearchCase::CaseSensitive);
			});
		TestTrue(TEXT("Invalid fingerprint diagnostic must be preserved"), bHasInvalidFingerprint);
		TestEqual(TEXT("Invalid fingerprint subcase Apply exact0"), Provider->ApplyCallCount, 0);
	}

	{
		// Conflicting fingerprint subcase adapter입니다.
		CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(Workbook);
		// Conflicting fingerprint subcase registry입니다.
		FCFContentProviderRegistry Registry;
		// Conflicting fingerprint subcase provider입니다.
		TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
		// Provider registration error입니다.
		FString Error;
		TestTrue(TEXT("Conflicting-fingerprint provider registration must succeed"),
			CFContentCompilerTestsPrivate::RegisterProvider(
				Workbook,
				CFContentCompilerTestsPrivate::MakeCurrentRecords(),
				Registry,
				Provider,
				Error));
		Provider->bReturnConflictingCurrentFingerprint = true;

		// Conflicting fingerprint compiler result입니다.
		FCFContentCompileResult Result;
		TestFalse(TEXT("Conflicting current fingerprint evidence must fail closed"),
			FCFContentCompiler::CompilePreview(
				TEXT("Memory://ConflictingFingerprint"),
				Adapter,
				Registry,
				Result));
		TestEqual(TEXT("Conflicting fingerprint failure stage must be Diff"),
			Result.FailureStage,
			FString(TEXT("Diff")));

		// ProviderCurrentFingerprintMismatch diagnostic 존재 여부입니다.
		const bool bHasFingerprintMismatch = Result.Issues.ContainsByPredicate(
			[](const FCFContentValidationIssue& Issue)
			{
				return Issue.Code.Equals(
					TEXT("ProviderCurrentFingerprintMismatch"),
					ESearchCase::CaseSensitive);
			});
		TestTrue(TEXT("Conflicting current fingerprint diagnostic must be preserved"),
			bHasFingerprintMismatch);
		TestEqual(TEXT("Conflicting fingerprint subcase Apply exact0"), Provider->ApplyCallCount, 0);
	}

	return true;
}

/** Workbook logical schema와 registered Provider schema가 다르면 explicit migration 전 fail-closed 되는지 검증합니다. */
bool FCFCompilerSchemaCompatibilityTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Provider가 등록될 accepted schema Workbook입니다.
	const FCFContentWorkbookModel ProviderWorkbook =
		CFContentCompilerTestsPrivate::MakeDesiredWorkbook();
	// Adapter가 반환할 incompatible Workbook입니다.
	FCFContentWorkbookModel IncompatibleWorkbook = ProviderWorkbook;
	IncompatibleWorkbook.Sheets[0].SchemaRevision = 2;

	// Disposable adapter입니다.
	CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(IncompatibleWorkbook);
	// Provider registry입니다.
	FCFContentProviderRegistry Registry;
	// Registered provider는 revision 1 schema를 유지합니다.
	TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
	// Provider registration error입니다.
	FString Error;
	TestTrue(TEXT("Revision1 provider registration must succeed"),
		CFContentCompilerTestsPrivate::RegisterProvider(
			ProviderWorkbook,
			CFContentCompilerTestsPrivate::MakeCurrentRecords(),
			Registry,
			Provider,
			Error));

	// Compiler preview result입니다.
	FCFContentCompileResult Result;
	TestFalse(TEXT("Workbook/provider schema mismatch must fail closed"),
		FCFContentCompiler::CompilePreview(
			TEXT("Memory://SchemaMismatch"),
			Adapter,
			Registry,
			Result));
	TestEqual(TEXT("Schema mismatch failure stage"),
		Result.FailureStage,
		FString(TEXT("ProviderSchemaCompatibility")));

	// Revision mismatch diagnostic 존재 여부입니다.
	const bool bHasRevisionMismatch = Result.Issues.ContainsByPredicate(
		[](const FCFContentValidationIssue& Issue)
		{
			return Issue.Code.Equals(
				TEXT("ProviderSchemaRevisionMismatch"),
				ESearchCase::CaseSensitive);
		});
	TestTrue(TEXT("Schema revision mismatch diagnostic must be preserved"), bHasRevisionMismatch);
	TestEqual(TEXT("Product Apply must remain exact0"), Provider->ApplyCallCount, 0);
	return true;
}

/** Per-record snapshot과 whole-current import의 혼합 시점 Product 변화를 모두 fail-closed 검증합니다. */
bool FCFCompilerSnapshotConsistencyTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Desired Workbook fixture입니다.
	const FCFContentWorkbookModel Workbook = CFContentCompilerTestsPrivate::MakeDesiredWorkbook();

	{
		// Per-record snapshot mismatch subcase adapter입니다.
		CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(Workbook);
		// Per-record snapshot mismatch subcase registry입니다.
		FCFContentProviderRegistry Registry;
		// Registered provider입니다.
		TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
		// Provider registration error입니다.
		FString Error;
		TestTrue(TEXT("Snapshot-mismatch provider registration must succeed"),
			CFContentCompilerTestsPrivate::RegisterProvider(
				Workbook,
				CFContentCompilerTestsPrivate::MakeCurrentRecords(),
				Registry,
				Provider,
				Error));
		Provider->bReturnInconsistentSnapshot = true;

		// Per-record consistency compiler result입니다.
		FCFContentCompileResult Result;
		TestFalse(TEXT("Mixed-time current snapshot must fail closed"),
			FCFContentCompiler::CompilePreview(
				TEXT("Memory://SnapshotConsistency"),
				Adapter,
				Registry,
				Result));
		TestEqual(TEXT("Snapshot consistency failure stage must be Diff"),
			Result.FailureStage,
			FString(TEXT("Diff")));

		// CurrentSnapshotChanged diagnostic 존재 여부입니다.
		const bool bHasSnapshotChanged = Result.Issues.ContainsByPredicate(
			[](const FCFContentValidationIssue& Issue)
			{
				return Issue.Code.Equals(
					TEXT("CurrentSnapshotChanged"),
					ESearchCase::CaseSensitive);
			});
		TestTrue(TEXT("Current snapshot changed diagnostic must be preserved"), bHasSnapshotChanged);
		TestEqual(TEXT("Snapshot mismatch subcase Apply exact0"), Provider->ApplyCallCount, 0);
	}

	{
		// Whole-current second-import drift subcase adapter입니다.
		CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(Workbook);
		// Whole-current second-import drift subcase registry입니다.
		FCFContentProviderRegistry Registry;
		// Registered provider입니다.
		TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
		// Provider registration error입니다.
		FString Error;
		TestTrue(TEXT("Whole-current consistency provider registration must succeed"),
			CFContentCompilerTestsPrivate::RegisterProvider(
				Workbook,
				CFContentCompilerTestsPrivate::MakeCurrentRecords(),
				Registry,
				Provider,
				Error));
		Provider->bChangeOnSecondImport = true;

		// Whole-current consistency compiler result입니다.
		FCFContentCompileResult Result;
		TestFalse(TEXT("Whole-current import drift must fail closed"),
			FCFContentCompiler::CompilePreview(
				TEXT("Memory://WholeCurrentConsistency"),
				Adapter,
				Registry,
				Result));
		TestEqual(TEXT("Whole-current drift failure stage"),
			Result.FailureStage,
			FString(TEXT("CurrentSnapshotConsistency")));

		// CurrentImportChanged diagnostic 존재 여부입니다.
		const bool bHasImportChanged = Result.Issues.ContainsByPredicate(
			[](const FCFContentValidationIssue& Issue)
			{
				return Issue.Code.Equals(
					TEXT("CurrentImportChanged"),
					ESearchCase::CaseSensitive);
			});
		TestTrue(TEXT("Whole-current drift diagnostic must be preserved"), bHasImportChanged);
		TestTrue(TEXT("Whole-current consistency must perform at least two imports"),
			Provider->ImportCallCount >= 2);
		TestEqual(TEXT("Whole-current drift subcase Apply exact0"), Provider->ApplyCallCount, 0);
	}

	return true;
}

/** 10k content + 100k child synthetic Workbook의 read/hash/diff scale evidence를 생성합니다. */
bool FCFCompilerScaleBenchmarkTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// 10k/100k desired Workbook입니다.
	FCFContentWorkbookModel Workbook;
	// 10k current Product disposable records입니다.
	TArray<FCFContentRecord> CurrentRecords;
	// 110k textual numeric cell canonical parse elapsed seconds입니다.
	double ParseSeconds = 0.0;
	// Synthetic fixture parse failure 상세입니다.
	FString ParseError;
	TestTrue(
		TEXT("10k/100k textual canonical parse fixture must build"),
		CFContentCompilerTestsPrivate::MakeScaleFixture(
			Workbook,
			CurrentRecords,
			ParseSeconds,
			ParseError));
	if (!ParseError.IsEmpty())
	{
		AddError(ParseError);
		return false;
	}

	// Disposable in-memory adapter입니다.
	CFContentCompilerTestsPrivate::FMemoryWorkbookAdapter Adapter(Workbook);
	// Provider registry입니다.
	FCFContentProviderRegistry Registry;
	// Registered disposable provider입니다.
	TSharedPtr<CFContentCompilerTestsPrivate::FMemoryContentProvider> Provider;
	// Provider registration error입니다.
	FString Error;
	TestTrue(TEXT("Benchmark provider registration must succeed"),
		CFContentCompilerTestsPrivate::RegisterProvider(
			Workbook,
			CurrentRecords,
			Registry,
			Provider,
			Error));

	// Benchmark start timestamp입니다.
	const double StartSeconds = FPlatformTime::Seconds();
	// Compiler benchmark result입니다.
	FCFContentCompileResult Result;
	// Benchmark compile success입니다.
	const bool bCompiled =
		FCFContentCompiler::CompilePreview(TEXT("Memory://Scale10000x100000"), Adapter, Registry, Result);
	// Benchmark elapsed seconds입니다.
	const double ElapsedSeconds = FPlatformTime::Seconds() - StartSeconds;

	AddInfo(FString::Printf(
		TEXT("CCAS-P0-02 SCALE_BENCH content=10000 child=100000 parse_seconds=%.3f hash_diff_seconds=%.3f total_seconds=%.3f"),
		ParseSeconds,
		ElapsedSeconds,
		ParseSeconds + ElapsedSeconds));

	TestTrue(TEXT("10k/100k Generic Compiler benchmark must complete"), bCompiled);
	TestEqual(TEXT("Benchmark desired count exact10000"), Result.BatchResult.DesiredRecordCount, 10000);
	TestEqual(TEXT("Benchmark current count exact10000"), Result.BatchResult.CurrentRecordCount, 10000);
	TestEqual(TEXT("Benchmark already-current exact10000"), Result.BatchResult.AlreadyCurrentCount, 10000);
	TestEqual(TEXT("Benchmark Product Apply exact0"), Provider->ApplyCallCount, 0);
	TestEqual(TEXT("Benchmark adapter write exact0"), Adapter.WriteCount, 0);
	return true;
}
