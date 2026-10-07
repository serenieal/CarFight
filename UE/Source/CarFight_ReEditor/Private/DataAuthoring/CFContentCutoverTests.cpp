// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCutoverTests.cpp
// Version: v1.4.1
// Date: 2026-10-03
// Description: CF-FQ-058 CCAS-P0-07 cutover coordinator + concrete persistent Workbook/provider/FileStore focused Automation exact16입니다.
// Changelog:
// - v1.4.1: DurableFileStore에 atomic temp residue exact0 및 filename/payload TransactionId mismatch fail-closed 검증 추가.
// - v1.4.0: Mid-review P1 교정 회귀 exact6 추가 — Manifest tamper, execution path redirect, pre-promotion drift,
//   recovery finalize, transient journal failure, failure-journal write failure visibility.
// - v1.3.0: production FCFContentCutoverFileStore transaction/provenance JSON durable save→load roundtrip을 disposable UE/Saved 경로에서 검증.
// - v1.2.0: FCFProviderCutoverAdapter가 existing ICFContentProvider의 typed diff/reviewed apply/readback을 재사용하고 absent Product Create를 fail-closed하는지 검증.
// - v1.1.0: repository-vendored OpenXLSX production adapter의 base create/read, staged-write, reopen,
//   semantic hash deterministic roundtrip, base no-overwrite를 검증하는 PersistentXlsxRoundtrip을 추가.
// - v1.0.0: ManagedCutover, StaleApproval, DirectProductDrift, PartialRecovery,
//   ConditionalBlocked, RetireNoDelete, Provenance exact7 disposable tests를 추가.
// Migration:
// - PersistentXlsxRoundtrip은 UE/Saved/CCAS/P007Tests 아래 disposable .xlsx만 생성/정리합니다.
// - production Workbook/DataAsset authority는 변경하지 않습니다.

#include "DataAuthoring/CFContentCutover.h"

#include "DataAuthoring/CFContentCore.h"
#include "DataAuthoring/CFDACommonPrimitives.h"
#include "DataAuthoring/CFNativeXlsxAdapter.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"

namespace CFContentCutoverTestsPrivate
{
	/** Fixture text를 canonical SHA-256으로 변환합니다. */
	FString HashText(const FString& Text)
	{
		// UTF-8 source입니다.
		FTCHARToUTF8 Utf8(*Text);
		// Hash input bytes입니다.
		TArray<uint8> Bytes;
		Bytes.Append(
			reinterpret_cast<const uint8*>(Utf8.Get()),
			Utf8.Length());

		// Canonical fingerprint 결과입니다.
		FString Fingerprint;
		// Hash failure 상세입니다.
		FString Error;
		return CFDACommonPrimitives::HashCanonicalBytes(
			Bytes,
			Fingerprint,
			Error)
				? Fingerprint
				: FString();
	}

	/** Test Power VALUE를 생성합니다. */
	FCFContentValue MakePowerValue(const double Power)
	{
		// Canonical double value입니다.
		FCFContentValue Value;
		Value.Type = ECFContentValueType::Double;
		Value.State = ECFContentValueState::Value;
		Value.FloatingPointValue = Power;
		return Value;
	}

	/** P0-07 test schema를 생성합니다. */
	TArray<FCFContentSheetDescriptor> MakeSchema()
	{
		// Primary test sheet입니다.
		FCFContentSheetDescriptor Sheet;
		Sheet.SheetId = TEXT("Content.Main");
		Sheet.SchemaRevision = 1;
		Sheet.ContentTypeId.Value = TEXT("Weapon");

		// Provider-managed Power field입니다.
		FCFContentFieldDescriptor PowerField;
		PowerField.ColumnId = TEXT("Power");
		PowerField.DisplayLabel = TEXT("Power");
		PowerField.ValueType = ECFContentValueType::Double;
		PowerField.Ownership = ECFContentFieldOwnership::CCASManaged;
		PowerField.bRequired = true;
		Sheet.Fields.Add(MoveTemp(PowerField));

		// Schema array입니다.
		TArray<FCFContentSheetDescriptor> Sheets;
		Sheets.Add(MoveTemp(Sheet));
		return Sheets;
	}

	/** Disposable canonical record를 생성합니다. */
	FCFContentRecord MakeRecord(
		const FString& ContentId,
		const double Power,
		const ECFContentPlanningReadiness Readiness =
			ECFContentPlanningReadiness::AuthoringReady,
		const ECFContentLifecycleState Lifecycle =
			ECFContentLifecycleState::Active)
	{
		// Canonical record입니다.
		FCFContentRecord Record;
		Record.Key.ContentTypeId.Value = TEXT("Weapon");
		Record.Key.ContentId = ContentId;
		Record.RowId = Record.Key.ToStableString();
		Record.ManagementState = ECFContentManagementState::Managed;
		Record.LifecycleState = Lifecycle;
		Record.AuthoringMetadata.Planning.Readiness = Readiness;
		Record.AuthoringMetadata.Planning.ProductionWaveId = TEXT("Wave1A");
		Record.Fields.Add(TEXT("Power"), MakePowerValue(Power));
		return Record;
	}

	/** Record provider-managed Product fingerprint를 생성합니다. */
	FString BuildProductFingerprint(const FCFContentRecord& Record)
	{
		// Power field입니다.
		const FCFContentValue* Power = Record.Fields.Find(TEXT("Power"));
		// Stable provider semantic text입니다.
		const FString SemanticText = FString::Printf(
			TEXT("%s|Power=%.17g"),
			*Record.Key.ToStableString(),
			Power != nullptr ? Power->FloatingPointValue : -1.0);
		return HashText(SemanticText);
	}

	/** Canonical Workbook fixture를 생성합니다. */
	FCFContentWorkbookModel MakeWorkbook(
		const TArray<FCFContentRecord>& Records)
	{
		// Workbook model입니다.
		FCFContentWorkbookModel Workbook;
		Workbook.WorkbookSourceId = TEXT("Test.P007.Workbook");
		Workbook.SchemaRevision = 1;
		Workbook.Sheets = MakeSchema();
		Workbook.Records = Records;
		return Workbook;
	}

	/** Provider/Product adapter가 공유하는 disposable persistent Product state입니다. */
	struct FSharedProductState
	{
		// Stable ContentKey -> current Product record map입니다.
		TMap<FString, FCFContentRecord> RecordsByKey;

		// Provider BuildDiff가 reviewed apply fixture에 넘길 desired record를 보존하는 disposable map입니다.
		TMap<FString, FCFContentRecord> PendingDesiredByKey;

		// Reviewed Product apply invocation count입니다.
		int32 ApplyCallCount = 0;

		// 이 ContentId에서 deterministic apply failure를 강제합니다.
		FString FailApplyContentId;

		// Product apply 직후 실행할 disposable test callback입니다.
		TFunction<void()> AfterApplyCallback;
	};

	/** Compiler read-only provider fixture입니다. */
	class FReadProvider final : public ICFContentProvider
	{
	public:
		/** Shared Product state와 schema를 설정합니다. */
		FReadProvider(
			const TSharedRef<FSharedProductState>& InState,
			const TArray<FCFContentSheetDescriptor>& InSchema)
			: State(InState)
			, Schema(InSchema)
		{
		}

		/** Stable provider schema를 반환합니다. */
		virtual bool DescribeSchema(
			FCFContentProviderDescriptor& OutDescriptor,
			FString& OutError) const override
		{
			OutError.Reset();
			OutDescriptor = FCFContentProviderDescriptor();
			OutDescriptor.ProviderId = TEXT("Test.P007.Provider");
			OutDescriptor.ContentTypeId.Value = TEXT("Weapon");
			OutDescriptor.SchemaRevision = 1;
			OutDescriptor.Sheets = Schema;
			return true;
		}

		/** Current Product records를 read-only import합니다. */
		virtual bool ImportExisting(
			FCFContentImportResult& OutImport,
			FString& OutError) const override
		{
			OutImport = FCFContentImportResult();
			OutError.Reset();
			for (const TPair<FString, FCFContentRecord>& Pair :
				State->RecordsByKey)
			{
				OutImport.Records.Add(Pair.Value);
			}
			return true;
		}

		/** Exact current Product snapshot을 반환합니다. */
		virtual bool BuildCurrentSnapshot(
			const FCFContentKey& Key,
			FCFContentSnapshot& OutSnapshot,
			FString& OutError) const override
		{
			OutSnapshot = FCFContentSnapshot();
			OutError.Reset();

			// Current record입니다.
			const FCFContentRecord* Record =
				State->RecordsByKey.Find(Key.ToStableString());
			if (Record == nullptr)
			{
				OutError = TEXT("Current Product record not found.");
				return false;
			}

			OutSnapshot.Key = Key;
			OutSnapshot.Record = *Record;
			OutSnapshot.ReadbackFingerprint =
				BuildProductFingerprint(*Record);
			return true;
		}

		/** Power field 존재 여부를 검증합니다. */
		virtual bool ValidateRecord(
			const FCFContentRecord& Record,
			TArray<FCFContentValidationIssue>& OutIssues) const override
		{
			OutIssues.Reset();
			if (Record.Fields.Contains(TEXT("Power")))
			{
				return true;
			}

			// Missing Power diagnostic입니다.
			FCFContentValidationIssue Issue;
			Issue.Code = TEXT("MissingPower");
			Issue.Path = Record.Key.ToStableString() + TEXT(".Power");
			Issue.Message = TEXT("Power field is required.");
			Issue.bBlocking = true;
			OutIssues.Add(MoveTemp(Issue));
			return false;
		}

		/** Test fixture는 dependency edge를 생성하지 않습니다. */
		virtual bool BuildDependencyEdges(
			const FCFContentRecord& Record,
			TArray<FCFContentDependencyEdge>& OutEdges,
			FString& OutError) const override
		{
			(void)Record;
			OutEdges.Reset();
			OutError.Reset();
			return true;
		}

		/** Current/desired provider semantic fingerprint diff를 생성합니다. */
		virtual bool BuildDiff(
			const FCFContentSnapshot& Current,
			const FCFContentRecord& Desired,
			FCFContentDiff& OutDiff,
			FString& OutError) const override
		{
			OutDiff = FCFContentDiff();
			OutError.Reset();
			OutDiff.Key = Desired.Key;
			OutDiff.CurrentFingerprint =
				BuildProductFingerprint(Current.Record);
			OutDiff.DesiredFingerprint =
				BuildProductFingerprint(Desired);
			State->PendingDesiredByKey.Add(
				Desired.Key.ToStableString(),
				Desired);
			if (!OutDiff.CurrentFingerprint.Equals(
				OutDiff.DesiredFingerprint,
				ESearchCase::CaseSensitive))
			{
				OutDiff.ChangedPaths.Add(TEXT("Power"));
			}
			return Current.Key == Desired.Key;
		}

		/** Interface compatibility용 reviewed mutation plan을 생성합니다. */
		virtual bool BuildReviewedMutationPlan(
			const FCFContentDiff& Diff,
			const FString& BaseWorkbookSemanticHash,
			FCFContentReviewedMutationPlan& OutPlan,
			FString& OutError) const override
		{
			OutPlan = FCFContentReviewedMutationPlan();
			OutError.Reset();
			OutPlan.Key = Diff.Key;
			OutPlan.BaseWorkbookSemanticHash =
				BaseWorkbookSemanticHash;
			OutPlan.DesiredFingerprint = Diff.DesiredFingerprint;
			OutPlan.ChangedPaths = Diff.ChangedPaths;
			return true;
		}

		/** Existing provider reviewed apply seam을 disposable persistent state에 구현합니다. */
		virtual bool ApplyReviewed(
			const FCFContentReviewedMutationPlan& ReviewedPlan,
			FCFContentApplyResult& OutResult) override
		{
			OutResult = FCFContentApplyResult();
			++State->ApplyCallCount;

			if (!ReviewedPlan.bApproved)
			{
				OutResult.Error = TEXT("Reviewed provider plan is not approved.");
				return false;
			}

			// BuildDiff가 보존한 exact desired record입니다.
			const FCFContentRecord* DesiredRecord =
				State->PendingDesiredByKey.Find(
					ReviewedPlan.Key.ToStableString());
			if (DesiredRecord == nullptr
				|| !BuildProductFingerprint(*DesiredRecord).Equals(
					ReviewedPlan.DesiredFingerprint,
					ESearchCase::CaseSensitive))
			{
				OutResult.Error = TEXT("Reviewed provider desired record/fingerprint mismatch.");
				return false;
			}

			if (!State->FailApplyContentId.IsEmpty()
				&& DesiredRecord->Key.ContentId.Equals(
					State->FailApplyContentId,
					ESearchCase::CaseSensitive))
			{
				OutResult.Error = TEXT("Synthetic provider apply failure.");
				return false;
			}

			State->RecordsByKey.Add(
				DesiredRecord->Key.ToStableString(),
				*DesiredRecord);
			OutResult.bApplied = true;
			OutResult.ReadbackFingerprint =
				ReviewedPlan.DesiredFingerprint;
			return true;
		}

		/** Exact current provider fingerprint를 반환합니다. */
		virtual bool ReadbackFingerprint(
			const FCFContentKey& Key,
			FString& OutFingerprint,
			FString& OutError) const override
		{
			// Current record입니다.
			const FCFContentRecord* Record =
				State->RecordsByKey.Find(Key.ToStableString());
			if (Record == nullptr)
			{
				OutFingerprint.Reset();
				OutError = TEXT("Current Product record not found.");
				return false;
			}
			OutFingerprint = BuildProductFingerprint(*Record);
			OutError.Reset();
			return true;
		}

	private:
		// Shared disposable Product state입니다.
		TSharedRef<FSharedProductState> State;

		// Provider schema입니다.
		TArray<FCFContentSheetDescriptor> Schema;
	};

	/** P0-07 typed Product apply bridge fixture입니다. */
	class FCutoverProductAdapter final :
		public ICFContentCutoverProductAdapter
	{
	public:
		/** Shared Product state를 설정합니다. */
		explicit FCutoverProductAdapter(
			const TSharedRef<FSharedProductState>& InState)
			: State(InState)
		{
		}

		/** Owned ContentTypeId를 반환합니다. */
		virtual FCFContentTypeId DescribeContentType() const override
		{
			// Weapon content type입니다.
			FCFContentTypeId ContentTypeId;
			ContentTypeId.Value = TEXT("Weapon");
			return ContentTypeId;
		}

		/** Desired Product semantic fingerprint를 계산합니다. */
		virtual bool BuildDesiredProductFingerprint(
			const FCFContentRecord& DesiredRecord,
			FString& OutFingerprint,
			FString& OutError) const override
		{
			OutFingerprint = BuildProductFingerprint(DesiredRecord);
			OutError.Reset();
			return !OutFingerprint.IsEmpty();
		}

		/** Product current truth 또는 explicit absence를 반환합니다. */
		virtual bool ReadCurrentProductState(
			const FCFContentKey& ContentKey,
			FCFContentProductState& OutState,
			FString& OutError) const override
		{
			OutState = FCFContentProductState();
			OutError.Reset();

			// Current record입니다.
			const FCFContentRecord* Record =
				State->RecordsByKey.Find(ContentKey.ToStableString());
			if (Record == nullptr)
			{
				OutState.bExists = false;
				return true;
			}

			OutState.bExists = true;
			OutState.ProductFingerprint =
				BuildProductFingerprint(*Record);
			return true;
		}

		/** Binding-checked desired record를 disposable persistent state에 적용합니다. */
		virtual bool ApplyReviewedProduct(
			const FCFContentCutoverBinding& Binding,
			const FCFContentRecord& DesiredRecord,
			FCFContentApplyResult& OutResult) override
		{
			OutResult = FCFContentApplyResult();
			++State->ApplyCallCount;

			if (!State->FailApplyContentId.IsEmpty()
				&& DesiredRecord.Key.ContentId.Equals(
					State->FailApplyContentId,
					ESearchCase::CaseSensitive))
			{
				OutResult.Error =
					TEXT("Synthetic per-Product apply failure.");
				return false;
			}

			// Adapter-computed desired fingerprint입니다.
			const FString DesiredFingerprint =
				BuildProductFingerprint(DesiredRecord);
			if (!(Binding.ContentKey == DesiredRecord.Key)
				|| !Binding.DesiredProductFingerprint.Equals(
					DesiredFingerprint,
					ESearchCase::CaseSensitive))
			{
				OutResult.Error =
					TEXT("Synthetic adapter binding mismatch.");
				return false;
			}

			State->RecordsByKey.Add(
				DesiredRecord.Key.ToStableString(),
				DesiredRecord);
			OutResult.bApplied = true;
			OutResult.ReadbackFingerprint = DesiredFingerprint;
			if (State->AfterApplyCallback)
			{
				State->AfterApplyCallback();
			}
			return true;
		}

	private:
		// Shared disposable Product state입니다.
		TSharedRef<FSharedProductState> State;
	};

	/** File path별 canonical Workbook model을 보존하는 staged/reopen adapter입니다. */
	class FMemoryWorkbookAdapter final :
		public ICFContentWorkbookAdapter
	{
	public:
		/** Canonical base Workbook을 설정합니다. */
		FMemoryWorkbookAdapter(
			const FString& BasePath,
			const FCFContentWorkbookModel& BaseWorkbook)
		{
			Files.Add(BasePath, BaseWorkbook);
		}

		/** Test persistent adapter metadata를 반환합니다. */
		virtual FCFWorkbookAdapterInfo DescribeAdapter() const override
		{
			// Test adapter metadata입니다.
			FCFWorkbookAdapterInfo Info;
			Info.AdapterId = TEXT("Test.P007.PersistentMemory");
			Info.AdapterVersion = TEXT("1.0");
			Info.bPreservesPresentation = true;
			Info.bPreservesProtection = true;
			return Info;
		}

		/** Exact path Workbook을 읽습니다. */
		virtual bool ReadWorkbook(
			const FString& WorkbookPath,
			FCFContentWorkbookModel& OutWorkbook,
			FString& OutError) override
		{
			++ReadCount;

			// Exact in-memory file model입니다.
			const FCFContentWorkbookModel* Workbook =
				Files.Find(WorkbookPath);
			if (Workbook == nullptr)
			{
				OutError = TEXT("Memory Workbook path not found.");
				return false;
			}

			OutWorkbook = *Workbook;
			OutError.Reset();
			return true;
		}

		/** Reviewed Workbook을 exact staged path에 기록합니다. */
		virtual bool WriteStagedWorkbook(
			const FString& BaseWorkbookPath,
			const FString& StagedWorkbookPath,
			const FCFContentWorkbookModel& ReviewedWorkbook,
			FString& OutError) override
		{
			++WriteCount;
			if (!Files.Contains(BaseWorkbookPath)
				|| BaseWorkbookPath.Equals(
					StagedWorkbookPath,
					ESearchCase::CaseSensitive))
			{
				OutError = TEXT("Invalid staged Workbook path.");
				return false;
			}

			Files.Add(StagedWorkbookPath, ReviewedWorkbook);
			OutError.Reset();
			return true;
		}

		/** Exact path Workbook을 reopen합니다. */
		virtual bool ReopenWorkbook(
			const FString& StagedWorkbookPath,
			FCFContentWorkbookModel& OutWorkbook,
			FString& OutError) override
		{
			++ReopenCount;

			// Exact in-memory file model입니다.
			const FCFContentWorkbookModel* Workbook =
				Files.Find(StagedWorkbookPath);
			if (Workbook == nullptr)
			{
				OutError = TEXT("Memory reopen path not found.");
				return false;
			}

			OutWorkbook = *Workbook;
			OutError.Reset();
			return true;
		}

		// Exact path -> Workbook model map입니다.
		TMap<FString, FCFContentWorkbookModel> Files;

		// ReadWorkbook invocation count입니다.
		int32 ReadCount = 0;

		// WriteStagedWorkbook invocation count입니다.
		int32 WriteCount = 0;

		// ReopenWorkbook invocation count입니다.
		int32 ReopenCount = 0;
	};

	/** Durable cutover store의 disposable in-memory fixture입니다. */
	class FMemoryCutoverStore final :
		public ICFContentCutoverStore
	{
	public:
		/** Workbook adapter와 promotion path를 공유합니다. */
		explicit FMemoryCutoverStore(
			FMemoryWorkbookAdapter& InWorkbookAdapter)
			: WorkbookAdapter(InWorkbookAdapter)
		{
		}

		/** Transaction latest state를 memory에 저장합니다. */
		virtual bool SaveTransaction(
			const FCFContentCutoverTransaction& Transaction,
			FString& OutError) override
		{
			++TransactionSaveCount;
			if (bFailAllTransactionSaves
				|| TransactionSaveCount == FailTransactionSaveCall)
			{
				OutError = TEXT("Synthetic transaction journal failure.");
				return false;
			}
			LastTransaction = Transaction;
			OutError.Reset();
			return true;
		}

		/** Memory latest transaction을 읽습니다. */
		virtual bool LoadTransaction(
			const FString& TransactionId,
			FCFContentCutoverTransaction& OutTransaction,
			FString& OutError) const override
		{
			if (!LastTransaction.TransactionId.Equals(
				TransactionId,
				ESearchCase::CaseSensitive))
			{
				OutError = TEXT("Transaction not found.");
				return false;
			}
			OutTransaction = LastTransaction;
			OutError.Reset();
			return true;
		}

		/** Staged Workbook model을 canonical path로 promote합니다. */
		virtual bool PromoteStagedWorkbook(
			const FString& StagedWorkbookPath,
			const FString& CanonicalWorkbookPath,
			FString& OutError) override
		{
			++PromotionCount;
			if (bFailNextPromotion)
			{
				bFailNextPromotion = false;
				OutError = TEXT("Synthetic Workbook promotion failure.");
				return false;
			}

			// Staged model입니다.
			const FCFContentWorkbookModel* StagedWorkbook =
				WorkbookAdapter.Files.Find(StagedWorkbookPath);
			if (StagedWorkbook == nullptr)
			{
				OutError = TEXT("Staged Workbook missing.");
				return false;
			}

			WorkbookAdapter.Files.Add(
				CanonicalWorkbookPath,
				*StagedWorkbook);
			OutError.Reset();
			return true;
		}

		/** Generated provenance snapshot을 memory에 저장합니다. */
		virtual bool SaveProvenance(
			const FString& ProvenancePath,
			const FCFContentProvenanceSnapshot& Snapshot,
			FString& OutError) override
		{
			(void)ProvenancePath;
			LastProvenance = Snapshot;
			++ProvenanceSaveCount;
			OutError.Reset();
			return true;
		}

		/** Memory provenance snapshot을 읽습니다. */
		virtual bool LoadProvenance(
			const FString& ProvenancePath,
			FCFContentProvenanceSnapshot& OutSnapshot,
			FString& OutError) const override
		{
			(void)ProvenancePath;
			if (LastProvenance.CutoverTransactionId.IsEmpty())
			{
				OutError = TEXT("Provenance not found.");
				return false;
			}
			OutSnapshot = LastProvenance;
			OutError.Reset();
			return true;
		}

		// Shared Workbook adapter입니다.
		FMemoryWorkbookAdapter& WorkbookAdapter;

		// Latest durable transaction projection입니다.
		FCFContentCutoverTransaction LastTransaction;

		// Latest generated provenance snapshot입니다.
		FCFContentProvenanceSnapshot LastProvenance;

		// Transaction durable save count입니다.
		int32 TransactionSaveCount = 0;

		// 이 exact transaction save call에서 deterministic failure를 강제합니다.
		int32 FailTransactionSaveCall = INDEX_NONE;

		// 모든 transaction durable save를 실패시키는 test switch입니다.
		bool bFailAllTransactionSaves = false;

		// 다음 canonical Workbook promotion exact1을 실패시키는 test switch입니다.
		bool bFailNextPromotion = false;

		// Canonical Workbook promotion count입니다.
		int32 PromotionCount = 0;

		// Provenance durable save count입니다.
		int32 ProvenanceSaveCount = 0;
	};

	/** Common P0-07 disposable fixture 전체를 구성합니다. */
	struct FFixture
	{
		/** Base/desired records로 compiler/review/manifest/request를 구성합니다. */
		bool Initialize(
			const TArray<FCFContentRecord>& BaseRecords,
			const TArray<FCFContentRecord>& DesiredRecords,
			FString& OutError)
		{
			OutError.Reset();
			BaseWorkbook = MakeWorkbook(BaseRecords);
			DesiredWorkbook = MakeWorkbook(DesiredRecords);

			SharedState = MakeShared<FSharedProductState>();
			for (const FCFContentRecord& Record : BaseRecords)
			{
				SharedState->RecordsByKey.Add(
					Record.Key.ToStableString(),
					Record);
			}

			// Read-only compiler provider입니다.
			TSharedRef<FReadProvider> ReadProvider =
				MakeShared<FReadProvider>(
					SharedState.ToSharedRef(),
					BaseWorkbook.Sheets);
			if (!ProviderRegistry.RegisterProvider(
				ReadProvider,
				OutError))
			{
				return false;
			}

			// P0-07 typed Product bridge입니다.
			TSharedRef<FCutoverProductAdapter> ProductAdapter =
				MakeShared<FCutoverProductAdapter>(
					SharedState.ToSharedRef());
			if (!ProductRegistry.RegisterAdapter(
				ProductAdapter,
				OutError))
			{
				return false;
			}

			WorkbookAdapter = MakeUnique<FMemoryWorkbookAdapter>(
				CanonicalPath,
				BaseWorkbook);
			Store = MakeUnique<FMemoryCutoverStore>(
				*WorkbookAdapter);

			// Base compiler result입니다.
			FCFContentCompileResult BaseCompileResult;
			if (!FCFContentCompiler::CompilePreview(
				CanonicalPath,
				*WorkbookAdapter,
				ProviderRegistry,
				BaseCompileResult))
			{
				OutError = BaseCompileResult.Error;
				return false;
			}

			if (!FCFContentPlanningService::BuildCatalogSnapshot(
				BaseCompileResult,
				ProviderRegistry,
				ResourceCatalog,
				PickerRegistry,
				BaseSnapshot,
				OutError))
			{
				return false;
			}

			// Desired preview adapter입니다.
			FMemoryWorkbookAdapter DesiredAdapter(
				TEXT("Memory://Desired"),
				DesiredWorkbook);
			// Desired compiler result입니다.
			FCFContentCompileResult DesiredCompileResult;
			if (!FCFContentCompiler::CompilePreview(
				TEXT("Memory://Desired"),
				DesiredAdapter,
				ProviderRegistry,
				DesiredCompileResult))
			{
				OutError = DesiredCompileResult.Error;
				return false;
			}

			ReviewPackage = FCFContentReviewPackage();
			ReviewPackage.ChangeSetId = TEXT("P007.ChangeSet");
			ReviewPackage.BaseCatalogSnapshotFingerprint =
				BaseSnapshot.SnapshotFingerprint;
			ReviewPackage.BaseWorkbookSemanticHash =
				BaseSnapshot.WorkbookSemanticHash;
			ReviewPackage.ExpectedPostSemanticHash =
				DesiredCompileResult.WorkbookSemanticHash;
			ReviewPackage.Preview = DesiredCompileResult;
			ReviewPackage.bNoPersistentWorkbookWriteGuardPassed = true;
			ReviewPackage.bNoDirectDataAssetWriteGuardPassed = true;

			if (!FCFContentPlanningService::ComputeReviewPackageFingerprint(
				ReviewPackage,
				ReviewPackage.ReviewPackageFingerprint,
				OutError))
			{
				return false;
			}
			return true;
		}

		/** USER-selected exact keys로 manifest + approval + request를 생성합니다. */
		bool BuildRequest(
			const TArray<FCFContentKey>& SelectedKeys,
			FString& OutError)
		{
			OutError.Reset();

			// Manifest build diagnostics입니다.
			TArray<FCFContentValidationIssue> Issues;
			if (!FCFContentCutoverCoordinator::BuildManifest(
				TEXT("P007.Manifest"),
				ReviewPackage,
				BaseSnapshot,
				SelectedKeys,
				ProductRegistry,
				Manifest,
				Issues,
				OutError))
			{
				return false;
			}

			Request = FCFContentCutoverRequest();
			Request.TransactionId = TEXT("P007.Transaction");
			Request.CanonicalWorkbookPath = CanonicalPath;
			Request.StagedWorkbookPath = StagedPath;
			Request.ProvenancePath = ProvenancePath;
			Request.ReviewPackage = ReviewPackage;
			Request.Manifest = Manifest;
			Request.Approval.ReviewPackageFingerprint =
				ReviewPackage.ReviewPackageFingerprint;
			Request.Approval.ManifestFingerprint =
				Manifest.ManifestFingerprint;
			Request.Approval.bApproved = true;
			return true;
		}

		/** Current disposable Product record의 Power를 direct-edit합니다. */
		void SetCurrentPower(
			const FString& ContentId,
			const double Power)
		{
			// Stable key text입니다.
			FCFContentKey Key;
			Key.ContentTypeId.Value = TEXT("Weapon");
			Key.ContentId = ContentId;

			// Current record입니다.
			FCFContentRecord* Record =
				SharedState->RecordsByKey.Find(Key.ToStableString());
			if (Record != nullptr)
			{
				Record->Fields[TEXT("Power")] =
					MakePowerValue(Power);
			}
		}

		// Canonical Workbook path입니다.
		FString CanonicalPath = TEXT("Authoring/Content/CarFight_Content.xlsx");

		// Staged Workbook path입니다.
		FString StagedPath = TEXT("Authoring/Content/CarFight_Content.staged.xlsx");

		// Provenance path입니다.
		FString ProvenancePath = TEXT("Authoring/Content/CarFight_Content.cfsnapshot.json");

		// Base Workbook입니다.
		FCFContentWorkbookModel BaseWorkbook;

		// Reviewed desired Workbook입니다.
		FCFContentWorkbookModel DesiredWorkbook;

		// Shared Product state입니다.
		TSharedPtr<FSharedProductState> SharedState;

		// Existing CCAS provider registry입니다.
		FCFContentProviderRegistry ProviderRegistry;

		// P0-07 Product bridge registry입니다.
		FCFContentCutoverProductRegistry ProductRegistry;

		// Empty resource catalog입니다.
		FCFResourceCatalog ResourceCatalog;

		// Empty picker registry입니다.
		FCFResourcePickerRegistry PickerRegistry;

		// Base immutable Catalog snapshot입니다.
		FCFContentCatalogSnapshot BaseSnapshot;

		// Synthetic immutable review package입니다.
		FCFContentReviewPackage ReviewPackage;

		// Explicit managed manifest입니다.
		FCFContentCutoverManifest Manifest;

		// Exact approved cutover request입니다.
		FCFContentCutoverRequest Request;

		// Staged/reopen workbook adapter입니다.
		TUniquePtr<FMemoryWorkbookAdapter> WorkbookAdapter;

		// Durable transaction/provenance store입니다.
		TUniquePtr<FMemoryCutoverStore> Store;
	};

	/** One Weapon key를 생성합니다. */
	FCFContentKey MakeKey(const FString& ContentId)
	{
		// Stable key입니다.
		FCFContentKey Key;
		Key.ContentTypeId.Value = TEXT("Weapon");
		Key.ContentId = ContentId;
		return Key;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007ManagedCutoverTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.ManagedCutover",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Staged/reopen + Product + Workbook + Verified 정상 경로를 검증합니다.
bool FCFP007ManagedCutoverTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Disposable fixture입니다.
	FFixture Fixture;
	// Fixture setup error입니다.
	FString Error;
	TestTrue(
		TEXT("Fixture initializes"),
		Fixture.Initialize(
			{ MakeRecord(TEXT("A"), 1.0) },
			{ MakeRecord(TEXT("A"), 2.0) },
			Error));
	if (!Error.IsEmpty())
	{
		AddError(Error);
		return false;
	}

	TestTrue(
		TEXT("Manifest/request builds"),
		Fixture.BuildRequest(
			{ MakeKey(TEXT("A")) },
			Error));
	if (!Error.IsEmpty())
	{
		AddError(Error);
		return false;
	}

	// Terminal cutover transaction입니다.
	FCFContentCutoverTransaction Transaction;
	TestTrue(
		TEXT("Cutover executes"),
		FCFContentCutoverCoordinator::Execute(
			Fixture.Request,
			*Fixture.WorkbookAdapter,
			Fixture.ProviderRegistry,
			Fixture.ResourceCatalog,
			Fixture.PickerRegistry,
			Fixture.ProductRegistry,
			*Fixture.Store,
			Transaction));

	TestEqual(
		TEXT("Transaction reaches Verified"),
		Transaction.State,
		ECFContentCutoverState::Verified);
	TestTrue(
		TEXT("Authority is only eligible"),
		Transaction.bAuthorityActivationEligible);
	TestFalse(
		TEXT("Authority is not activated automatically"),
		Transaction.bAuthorityActivated);
	TestEqual(
		TEXT("Product apply exact1"),
		Fixture.SharedState->ApplyCallCount,
		1);
	TestEqual(
		TEXT("Workbook staged write exact1"),
		Fixture.WorkbookAdapter->WriteCount,
		1);
	TestTrue(
		TEXT("Workbook reopen occurs"),
		Fixture.WorkbookAdapter->ReopenCount >= 2);
	TestEqual(
		TEXT("Canonical promotion exact1"),
		Fixture.Store->PromotionCount,
		1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007StaleApprovalTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.StaleApproval",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Review 뒤 direct Product 변화가 mutation0으로 차단되는지 검증합니다.
bool FCFP007StaleApprovalTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Disposable fixture입니다.
	FFixture Fixture;
	// Fixture setup error입니다.
	FString Error;
	TestTrue(
		TEXT("Fixture initializes"),
		Fixture.Initialize(
			{ MakeRecord(TEXT("A"), 1.0) },
			{ MakeRecord(TEXT("A"), 2.0) },
			Error));
	TestTrue(
		TEXT("Request builds"),
		Fixture.BuildRequest(
			{ MakeKey(TEXT("A")) },
			Error));

	Fixture.SetCurrentPower(TEXT("A"), 9.0);

	// Terminal blocked transaction입니다.
	FCFContentCutoverTransaction Transaction;
	TestFalse(
		TEXT("Stale approval fails closed"),
		FCFContentCutoverCoordinator::Execute(
			Fixture.Request,
			*Fixture.WorkbookAdapter,
			Fixture.ProviderRegistry,
			Fixture.ResourceCatalog,
			Fixture.PickerRegistry,
			Fixture.ProductRegistry,
			*Fixture.Store,
			Transaction));
	TestEqual(
		TEXT("Blocked before mutation"),
		Transaction.State,
		ECFContentCutoverState::BlockedBeforeMutation);
	TestEqual(
		TEXT("Product mutation exact0"),
		Fixture.SharedState->ApplyCallCount,
		0);
	TestEqual(
		TEXT("Canonical promotion exact0"),
		Fixture.Store->PromotionCount,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007DirectDriftTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.DirectProductDrift",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Verified provenance 뒤 Product direct edit를 Drifted로 탐지하는지 검증합니다.
bool FCFP007DirectDriftTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Disposable fixture입니다.
	FFixture Fixture;
	// Fixture setup error입니다.
	FString Error;
	TestTrue(
		TEXT("Fixture initializes"),
		Fixture.Initialize(
			{ MakeRecord(TEXT("A"), 1.0) },
			{ MakeRecord(TEXT("A"), 2.0) },
			Error));
	TestTrue(
		TEXT("Request builds"),
		Fixture.BuildRequest(
			{ MakeKey(TEXT("A")) },
			Error));

	// Verified transaction입니다.
	FCFContentCutoverTransaction Transaction;
	TestTrue(
		TEXT("Cutover executes"),
		FCFContentCutoverCoordinator::Execute(
			Fixture.Request,
			*Fixture.WorkbookAdapter,
			Fixture.ProviderRegistry,
			Fixture.ResourceCatalog,
			Fixture.PickerRegistry,
			Fixture.ProductRegistry,
			*Fixture.Store,
			Transaction));

	Fixture.SetCurrentPower(TEXT("A"), 77.0);

	// Detected drift keys입니다.
	TArray<FCFContentKey> DriftedKeys;
	TestTrue(
		TEXT("Drift inspection succeeds"),
		FCFContentCutoverCoordinator::DetectDirectProductDrift(
			Fixture.Store->LastProvenance,
			Fixture.ProductRegistry,
			DriftedKeys,
			Error));
	TestEqual(
		TEXT("Direct edit drift exact1"),
		DriftedKeys.Num(),
		1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007PartialRecoveryTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.PartialRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// First Product durable success 뒤 second failure가 RecoveryRequired가 되고 Workbook promotion을 막는지 검증합니다.
bool FCFP007PartialRecoveryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Disposable fixture입니다.
	FFixture Fixture;
	// Fixture setup error입니다.
	FString Error;
	TestTrue(
		TEXT("Fixture initializes"),
		Fixture.Initialize(
			{
				MakeRecord(TEXT("A"), 1.0),
				MakeRecord(TEXT("B"), 1.0)
			},
			{
				MakeRecord(TEXT("A"), 2.0),
				MakeRecord(TEXT("B"), 2.0)
			},
			Error));
	TestTrue(
		TEXT("Request builds"),
		Fixture.BuildRequest(
			{
				MakeKey(TEXT("A")),
				MakeKey(TEXT("B"))
			},
			Error));

	Fixture.SharedState->FailApplyContentId = TEXT("B");

	// Terminal recovery transaction입니다.
	FCFContentCutoverTransaction Transaction;
	TestFalse(
		TEXT("Partial batch fails"),
		FCFContentCutoverCoordinator::Execute(
			Fixture.Request,
			*Fixture.WorkbookAdapter,
			Fixture.ProviderRegistry,
			Fixture.ResourceCatalog,
			Fixture.PickerRegistry,
			Fixture.ProductRegistry,
			*Fixture.Store,
			Transaction));
	TestEqual(
		TEXT("Partial success requires recovery"),
		Transaction.State,
		ECFContentCutoverState::RecoveryRequired);
	TestEqual(
		TEXT("Product apply reached second target"),
		Fixture.SharedState->ApplyCallCount,
		2);
	TestEqual(
		TEXT("Workbook promotion forbidden"),
		Fixture.Store->PromotionCount,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007ConditionalBlockedTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.ConditionalBlocked",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Conditional + Pending dependency가 explicit onboarding 단계에서 차단되는지 검증합니다.
bool FCFP007ConditionalBlockedTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Conditional guided record입니다.
	FCFContentRecord GuidedRecord = MakeRecord(
		TEXT("GuidedMissile_Standard"),
		1.0,
		ECFContentPlanningReadiness::Conditional);
	// Pending trajectory dependency입니다.
	FCFContentTechnologyDependency Dependency;
	Dependency.DependencyId = TEXT("PFP.P0.03.UserTrajectory");
	Dependency.State = ECFContentDependencyState::Pending;
	Dependency.Reason = TEXT("USER trajectory acceptance pending.");
	GuidedRecord.AuthoringMetadata.Planning.TechnologyDependencies.Add(
		MoveTemp(Dependency));

	// Disposable fixture입니다.
	FFixture Fixture;
	// Fixture setup error입니다.
	FString Error;
	TestTrue(
		TEXT("Fixture initializes"),
		Fixture.Initialize(
			{ GuidedRecord },
			{ GuidedRecord },
			Error));

	// Manifest result입니다.
	FCFContentCutoverManifest Manifest;
	// Manifest diagnostics입니다.
	TArray<FCFContentValidationIssue> Issues;
	TestFalse(
		TEXT("Conditional guided missile onboarding is blocked"),
		FCFContentCutoverCoordinator::BuildManifest(
			TEXT("P007.Guided"),
			Fixture.ReviewPackage,
			Fixture.BaseSnapshot,
			{ MakeKey(TEXT("GuidedMissile_Standard")) },
			Fixture.ProductRegistry,
			Manifest,
			Issues,
			Error));
	TestTrue(
		TEXT("Blocking diagnostics exist"),
		Issues.Num() > 0);
	TestEqual(
		TEXT("Product mutation exact0"),
		Fixture.SharedState->ApplyCallCount,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007RetireNoDeleteTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.RetireNoDelete",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Retired lifecycle가 physical Product delete로 번역되지 않는지 검증합니다.
bool FCFP007RetireNoDeleteTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Base active Product입니다.
	const FCFContentRecord BaseRecord =
		MakeRecord(TEXT("A"), 1.0);
	// Desired logical retired record입니다.
	const FCFContentRecord RetiredRecord =
		MakeRecord(
			TEXT("A"),
			1.0,
			ECFContentPlanningReadiness::AuthoringReady,
			ECFContentLifecycleState::Retired);

	// Disposable fixture입니다.
	FFixture Fixture;
	// Fixture setup error입니다.
	FString Error;
	TestTrue(
		TEXT("Fixture initializes"),
		Fixture.Initialize(
			{ BaseRecord },
			{ RetiredRecord },
			Error));
	TestTrue(
		TEXT("Request builds"),
		Fixture.BuildRequest(
			{ MakeKey(TEXT("A")) },
			Error));

	// Verified transaction입니다.
	FCFContentCutoverTransaction Transaction;
	TestTrue(
		TEXT("Logical retire cutover executes"),
		FCFContentCutoverCoordinator::Execute(
			Fixture.Request,
			*Fixture.WorkbookAdapter,
			Fixture.ProviderRegistry,
			Fixture.ResourceCatalog,
			Fixture.PickerRegistry,
			Fixture.ProductRegistry,
			*Fixture.Store,
			Transaction));
	TestTrue(
		TEXT("Physical Product remains"),
		Fixture.SharedState->RecordsByKey.Contains(
			MakeKey(TEXT("A")).ToStableString()));
	TestEqual(
		TEXT("No gameplay payload mutation needed"),
		Fixture.SharedState->ApplyCallCount,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007ProvenanceTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.Provenance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Verified managed record의 mandatory provenance exact fields를 검증합니다.
bool FCFP007ProvenanceTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Disposable fixture입니다.
	FFixture Fixture;
	// Fixture setup error입니다.
	FString Error;
	TestTrue(
		TEXT("Fixture initializes"),
		Fixture.Initialize(
			{ MakeRecord(TEXT("A"), 1.0) },
			{ MakeRecord(TEXT("A"), 2.0) },
			Error));
	TestTrue(
		TEXT("Request builds"),
		Fixture.BuildRequest(
			{ MakeKey(TEXT("A")) },
			Error));

	// Verified transaction입니다.
	FCFContentCutoverTransaction Transaction;
	TestTrue(
		TEXT("Cutover executes"),
		FCFContentCutoverCoordinator::Execute(
			Fixture.Request,
			*Fixture.WorkbookAdapter,
			Fixture.ProviderRegistry,
			Fixture.ResourceCatalog,
			Fixture.PickerRegistry,
			Fixture.ProductRegistry,
			*Fixture.Store,
			Transaction));

	TestEqual(
		TEXT("Provenance record exact1"),
		Fixture.Store->LastProvenance.Records.Num(),
		1);
	if (Fixture.Store->LastProvenance.Records.Num() != 1)
	{
		return false;
	}

	// Exact generated provenance record입니다.
	const FCFContentProvenanceRecord& Record =
		Fixture.Store->LastProvenance.Records[0];
	TestEqual(
		TEXT("ContentKey preserved"),
		Record.ContentKey.ToStableString(),
		MakeKey(TEXT("A")).ToStableString());
	TestEqual(
		TEXT("TransactionId preserved"),
		Record.CutoverTransactionId,
		Fixture.Request.TransactionId);
	TestTrue(
		TEXT("SourceWorkbookSemanticHash canonical"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Record.SourceWorkbookSemanticHash));
	TestTrue(
		TEXT("CatalogSnapshotFingerprint canonical"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Record.CatalogSnapshotFingerprint));
	TestTrue(
		TEXT("ReviewPackageFingerprint canonical"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Record.ReviewPackageFingerprint));
	TestTrue(
		TEXT("ProviderSchemaFingerprint canonical"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Record.ProviderSchemaFingerprint));
	TestTrue(
		TEXT("DesiredCanonicalFingerprint canonical"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Record.DesiredCanonicalFingerprint));
	TestTrue(
		TEXT("PreApplyProductFingerprint canonical"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Record.PreApplyProductFingerprint));
	TestTrue(
		TEXT("PostApplyReadbackFingerprint canonical"),
		CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Record.PostApplyReadbackFingerprint));
	TestEqual(
		TEXT("ManagementState preserved"),
		Record.ManagementState,
		ECFContentManagementState::Managed);
	TestEqual(
		TEXT("LifecycleState preserved"),
		Record.LifecycleState,
		ECFContentLifecycleState::Active);
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007ManifestTamperTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.ManifestTamperBlocked",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// USER approval 뒤 Manifest payload가 바뀌면 stored fingerprint 문자열을 재사용해도 mutation0으로 차단되는지 검증합니다.
bool FCFP007ManifestTamperTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Disposable fixture입니다.
	FFixture Fixture;
	// Fixture setup error입니다.
	FString Error;
	TestTrue(
		TEXT("Fixture initializes"),
		Fixture.Initialize(
			{ MakeRecord(TEXT("A"), 1.0) },
			{ MakeRecord(TEXT("A"), 2.0) },
			Error));
	TestTrue(
		TEXT("Request builds"),
		Fixture.BuildRequest(
			{ MakeKey(TEXT("A")) },
			Error));

	Fixture.Request.Manifest.ManifestId = TEXT("P007.Manifest.Tampered");

	// Terminal blocked transaction입니다.
	FCFContentCutoverTransaction Transaction;
	TestFalse(
		TEXT("Tampered manifest fails closed"),
		FCFContentCutoverCoordinator::Execute(
			Fixture.Request,
			*Fixture.WorkbookAdapter,
			Fixture.ProviderRegistry,
			Fixture.ResourceCatalog,
			Fixture.PickerRegistry,
			Fixture.ProductRegistry,
			*Fixture.Store,
			Transaction));
	TestEqual(
		TEXT("Tamper blocked before mutation"),
		Transaction.State,
		ECFContentCutoverState::BlockedBeforeMutation);
	TestEqual(
		TEXT("Tamper Product mutation exact0"),
		Fixture.SharedState->ApplyCallCount,
		0);
	TestEqual(
		TEXT("Tamper promotion exact0"),
		Fixture.Store->PromotionCount,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007ExecutionPathTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.ExecutionPathBound",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Approved request의 canonical/staged/provenance path redirect를 mutation 전에 차단하는지 검증합니다.
bool FCFP007ExecutionPathTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Disposable fixture입니다.
	FFixture Fixture;
	// Fixture setup error입니다.
	FString Error;
	TestTrue(
		TEXT("Fixture initializes"),
		Fixture.Initialize(
			{ MakeRecord(TEXT("A"), 1.0) },
			{ MakeRecord(TEXT("A"), 2.0) },
			Error));
	TestTrue(
		TEXT("Request builds"),
		Fixture.BuildRequest(
			{ MakeKey(TEXT("A")) },
			Error));

	Fixture.Request.CanonicalWorkbookPath =
		TEXT("Authoring/Content/Redirect.xlsx");

	// Terminal blocked transaction입니다.
	FCFContentCutoverTransaction Transaction;
	TestFalse(
		TEXT("Execution path redirect fails closed"),
		FCFContentCutoverCoordinator::Execute(
			Fixture.Request,
			*Fixture.WorkbookAdapter,
			Fixture.ProviderRegistry,
			Fixture.ResourceCatalog,
			Fixture.PickerRegistry,
			Fixture.ProductRegistry,
			*Fixture.Store,
			Transaction));
	TestEqual(
		TEXT("Redirect blocked before mutation"),
		Transaction.State,
		ECFContentCutoverState::BlockedBeforeMutation);
	TestEqual(
		TEXT("Redirect staged write exact0"),
		Fixture.WorkbookAdapter->WriteCount,
		0);
	TestEqual(
		TEXT("Redirect Product mutation exact0"),
		Fixture.SharedState->ApplyCallCount,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007PromotionFreshnessTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.PromotionFreshnessGuard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Product apply 도중 canonical Workbook이 외부 변경되면 promotion 직전 재검증이 승격을 막는지 검증합니다.
bool FCFP007PromotionFreshnessTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Disposable fixture입니다.
	FFixture Fixture;
	// Fixture setup error입니다.
	FString Error;
	TestTrue(
		TEXT("Fixture initializes"),
		Fixture.Initialize(
			{ MakeRecord(TEXT("A"), 1.0) },
			{ MakeRecord(TEXT("A"), 2.0) },
			Error));
	TestTrue(
		TEXT("Request builds"),
		Fixture.BuildRequest(
			{ MakeKey(TEXT("A")) },
			Error));

	Fixture.SharedState->AfterApplyCallback = [&Fixture]()
	{
		Fixture.WorkbookAdapter->Files.Add(
			Fixture.CanonicalPath,
			MakeWorkbook({ MakeRecord(TEXT("A"), 99.0) }));
	};

	// Terminal recovery transaction입니다.
	FCFContentCutoverTransaction Transaction;
	TestFalse(
		TEXT("Canonical drift after Product apply blocks promotion"),
		FCFContentCutoverCoordinator::Execute(
			Fixture.Request,
			*Fixture.WorkbookAdapter,
			Fixture.ProviderRegistry,
			Fixture.ResourceCatalog,
			Fixture.PickerRegistry,
			Fixture.ProductRegistry,
			*Fixture.Store,
			Transaction));
	TestEqual(
		TEXT("Post-apply canonical drift requires recovery"),
		Transaction.State,
		ECFContentCutoverState::RecoveryRequired);
	TestEqual(
		TEXT("Product apply occurred exact1"),
		Fixture.SharedState->ApplyCallCount,
		1);
	TestEqual(
		TEXT("Drifted canonical promotion exact0"),
		Fixture.Store->PromotionCount,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007RecoveryFinalizeTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.RecoveryFinalize",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Product가 desired까지 적용된 뒤 promotion failure가 발생하면 exact staged candidate만 재개해 Verified로 수렴하는지 검증합니다.
bool FCFP007RecoveryFinalizeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Disposable fixture입니다.
	FFixture Fixture;
	// Fixture setup error입니다.
	FString Error;
	TestTrue(
		TEXT("Fixture initializes"),
		Fixture.Initialize(
			{ MakeRecord(TEXT("A"), 1.0) },
			{ MakeRecord(TEXT("A"), 2.0) },
			Error));
	TestTrue(
		TEXT("Request builds"),
		Fixture.BuildRequest(
			{ MakeKey(TEXT("A")) },
			Error));

	Fixture.Store->bFailNextPromotion = true;

	// First execution recovery result입니다.
	FCFContentCutoverTransaction FailedTransaction;
	TestFalse(
		TEXT("First promotion fails into RecoveryRequired"),
		FCFContentCutoverCoordinator::Execute(
			Fixture.Request,
			*Fixture.WorkbookAdapter,
			Fixture.ProviderRegistry,
			Fixture.ResourceCatalog,
			Fixture.PickerRegistry,
			Fixture.ProductRegistry,
			*Fixture.Store,
			FailedTransaction));
	TestEqual(
		TEXT("Durable failure is RecoveryRequired"),
		FailedTransaction.State,
		ECFContentCutoverState::RecoveryRequired);
	TestEqual(
		TEXT("Product applied before promotion failure exact1"),
		Fixture.SharedState->ApplyCallCount,
		1);

	// Recovery terminal transaction입니다.
	FCFContentCutoverTransaction RecoveredTransaction;
	TestTrue(
		TEXT("Exact candidate recovery finalize succeeds"),
		FCFContentCutoverCoordinator::ResumeRecovery(
			Fixture.Request,
			*Fixture.WorkbookAdapter,
			Fixture.ProductRegistry,
			*Fixture.Store,
			RecoveredTransaction));
	TestEqual(
		TEXT("Recovery reaches Verified"),
		RecoveredTransaction.State,
		ECFContentCutoverState::Verified);
	TestTrue(
		TEXT("Recovery becomes authority-activation eligible only"),
		RecoveredTransaction.bAuthorityActivationEligible);
	TestFalse(
		TEXT("Recovery does not auto-activate authority"),
		RecoveredTransaction.bAuthorityActivated);
	TestEqual(
		TEXT("Recovery does not reapply Product"),
		Fixture.SharedState->ApplyCallCount,
		1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007JournalRecoveryTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.JournalFailureRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Product durable mutation 뒤 per-Product journal save가 일시 실패해도 RecoveryRequired evidence가 다음 save로 보존되는지 검증합니다.
bool FCFP007JournalRecoveryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Disposable fixture입니다.
	FFixture Fixture;
	// Fixture setup error입니다.
	FString Error;
	TestTrue(
		TEXT("Fixture initializes"),
		Fixture.Initialize(
			{ MakeRecord(TEXT("A"), 1.0) },
			{ MakeRecord(TEXT("A"), 2.0) },
			Error));
	TestTrue(
		TEXT("Request builds"),
		Fixture.BuildRequest(
			{ MakeKey(TEXT("A")) },
			Error));

	Fixture.Store->FailTransactionSaveCall = 2;

	// Recovery transaction입니다.
	FCFContentCutoverTransaction Transaction;
	TestFalse(
		TEXT("Transient journal failure stops execution"),
		FCFContentCutoverCoordinator::Execute(
			Fixture.Request,
			*Fixture.WorkbookAdapter,
			Fixture.ProviderRegistry,
			Fixture.ResourceCatalog,
			Fixture.PickerRegistry,
			Fixture.ProductRegistry,
			*Fixture.Store,
			Transaction));
	TestEqual(
		TEXT("Transient journal failure requires recovery"),
		Transaction.State,
		ECFContentCutoverState::RecoveryRequired);
	TestEqual(
		TEXT("RecoveryRequired durable evidence is retained"),
		Fixture.Store->LastTransaction.State,
		ECFContentCutoverState::RecoveryRequired);
	TestEqual(
		TEXT("Product mutation happened exact1"),
		Fixture.SharedState->ApplyCallCount,
		1);
	TestEqual(
		TEXT("Workbook promotion remains forbidden"),
		Fixture.Store->PromotionCount,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007JournalHardFailureTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.JournalHardFailureVisible",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Product mutation 이후 durable journal 자체가 계속 실패하면 caller transaction에 CRITICAL evidence loss가 fail-visible한지 검증합니다.
bool FCFP007JournalHardFailureTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Disposable fixture입니다.
	FFixture Fixture;
	// Fixture setup error입니다.
	FString Error;
	TestTrue(
		TEXT("Fixture initializes"),
		Fixture.Initialize(
			{ MakeRecord(TEXT("A"), 1.0) },
			{ MakeRecord(TEXT("A"), 2.0) },
			Error));
	TestTrue(
		TEXT("Request builds"),
		Fixture.BuildRequest(
			{ MakeKey(TEXT("A")) },
			Error));

	Fixture.SharedState->AfterApplyCallback = [&Fixture]()
	{
		Fixture.Store->bFailAllTransactionSaves = true;
	};

	// Fail-visible transaction입니다.
	FCFContentCutoverTransaction Transaction;
	TestFalse(
		TEXT("Persistent journal failure stops execution"),
		FCFContentCutoverCoordinator::Execute(
			Fixture.Request,
			*Fixture.WorkbookAdapter,
			Fixture.ProviderRegistry,
			Fixture.ResourceCatalog,
			Fixture.PickerRegistry,
			Fixture.ProductRegistry,
			*Fixture.Store,
			Transaction));
	TestEqual(
		TEXT("Caller state remains RecoveryRequired"),
		Transaction.State,
		ECFContentCutoverState::RecoveryRequired);
	TestTrue(
		TEXT("Durable evidence loss is explicitly critical"),
		Transaction.Diagnostic.Contains(
			TEXT("CRITICAL: durable failure evidence 저장 실패"),
			ESearchCase::CaseSensitive));
	TestEqual(
		TEXT("Product mutation happened exact1"),
		Fixture.SharedState->ApplyCallCount,
		1);
	TestEqual(
		TEXT("Workbook promotion remains exact0"),
		Fixture.Store->PromotionCount,
		0);
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007PersistentXlsxTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.PersistentXlsxRoundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Concrete OpenXLSX adapter의 base create/read -> staged write -> reopen semantic roundtrip과 base no-overwrite를 검증합니다.
bool FCFP007PersistentXlsxTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Repository-relative disposable base Workbook path입니다.
	const FString BaseWorkbookPath =
		TEXT("UE/Saved/CCAS/P007Tests/P007_Base.xlsx");

	// Repository-relative disposable staged Workbook path입니다.
	const FString StagedWorkbookPath =
		TEXT("UE/Saved/CCAS/P007Tests/P007_Base.staged.xlsx");

	// Physical disposable base Workbook path입니다.
	const FString BasePhysicalPath = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CCAS/P007Tests/P007_Base.xlsx"));

	// Physical disposable staged Workbook path입니다.
	const FString StagedPhysicalPath = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CCAS/P007Tests/P007_Base.staged.xlsx"));

	// Test-owned disposable file cleanup function입니다.
	const auto CleanupFiles = [&]()
	{
		IFileManager::Get().Delete(*StagedPhysicalPath, false, true, true);
		IFileManager::Get().Delete(*BasePhysicalPath, false, true, true);
	};

	CleanupFiles();

	// Canonical base Workbook model입니다.
	const FCFContentWorkbookModel BaseWorkbook = MakeWorkbook(
		{ MakeRecord(TEXT("A"), 1.0) });

	// Reviewed desired Workbook model입니다.
	const FCFContentWorkbookModel DesiredWorkbook = MakeWorkbook(
		{ MakeRecord(TEXT("A"), 2.0) });

	// Concrete repository-vendored OpenXLSX adapter입니다.
	FCFNativeXlsxAdapter Adapter;

	// Adapter operation diagnostic입니다.
	FString Error;

	if (!TestTrue(
		TEXT("Initial baseline concrete .xlsx is created"),
		Adapter.CreateWorkbookFile(
			BaseWorkbookPath,
			BaseWorkbook,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}

	// Base Workbook semantic hash입니다.
	FString ExpectedBaseHash;
	if (!TestTrue(
		TEXT("Base semantic hash builds"),
		FCFContentSemanticHasher::BuildWorkbookSemanticHash(
			BaseWorkbook,
			ExpectedBaseHash,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}

	// Concrete file readback model입니다.
	FCFContentWorkbookModel ReadBaseWorkbook;
	if (!TestTrue(
		TEXT("Concrete base .xlsx read succeeds"),
		Adapter.ReadWorkbook(
			BaseWorkbookPath,
			ReadBaseWorkbook,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}

	// Concrete base readback semantic hash입니다.
	FString ReadBaseHash;
	if (!TestTrue(
		TEXT("Concrete base readback semantic hash builds"),
		FCFContentSemanticHasher::BuildWorkbookSemanticHash(
			ReadBaseWorkbook,
			ReadBaseHash,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}
	TestEqual(
		TEXT("Concrete base semantic hash roundtrip exact"),
		ReadBaseHash,
		ExpectedBaseHash);

	if (!TestTrue(
		TEXT("Sibling staged .xlsx write succeeds"),
		Adapter.WriteStagedWorkbook(
			BaseWorkbookPath,
			StagedWorkbookPath,
			DesiredWorkbook,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}

	// Desired semantic hash입니다.
	FString ExpectedDesiredHash;
	if (!TestTrue(
		TEXT("Desired semantic hash builds"),
		FCFContentSemanticHasher::BuildWorkbookSemanticHash(
			DesiredWorkbook,
			ExpectedDesiredHash,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}

	// First staged reopen model입니다.
	FCFContentWorkbookModel FirstReopenWorkbook;
	if (!TestTrue(
		TEXT("Staged concrete .xlsx first reopen succeeds"),
		Adapter.ReopenWorkbook(
			StagedWorkbookPath,
			FirstReopenWorkbook,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}

	// First staged reopen semantic hash입니다.
	FString FirstReopenHash;
	if (!TestTrue(
		TEXT("First reopen semantic hash builds"),
		FCFContentSemanticHasher::BuildWorkbookSemanticHash(
			FirstReopenWorkbook,
			FirstReopenHash,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}
	TestEqual(
		TEXT("Staged reopen matches reviewed desired semantic hash"),
		FirstReopenHash,
		ExpectedDesiredHash);

	// Second staged reopen model입니다.
	FCFContentWorkbookModel SecondReopenWorkbook;
	if (!TestTrue(
		TEXT("Staged concrete .xlsx second reopen succeeds"),
		Adapter.ReopenWorkbook(
			StagedWorkbookPath,
			SecondReopenWorkbook,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}

	// Second staged reopen semantic hash입니다.
	FString SecondReopenHash;
	if (!TestTrue(
		TEXT("Second reopen semantic hash builds"),
		FCFContentSemanticHasher::BuildWorkbookSemanticHash(
			SecondReopenWorkbook,
			SecondReopenHash,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}
	TestEqual(
		TEXT("Repeated staged reopen is semantically deterministic"),
		SecondReopenHash,
		FirstReopenHash);

	// Existing base overwrite rejection diagnostic입니다.
	FString OverwriteError;
	TestFalse(
		TEXT("Initial baseline helper refuses existing Base overwrite"),
		Adapter.CreateWorkbookFile(
			BaseWorkbookPath,
			DesiredWorkbook,
			OverwriteError));
	TestTrue(
		TEXT("Overwrite rejection is fail-visible"),
		!OverwriteError.IsEmpty());

	// Base Workbook after staged write/no-overwrite test입니다.
	FCFContentWorkbookModel BaseAfterStage;
	if (!TestTrue(
		TEXT("Base remains readable after staged write"),
		Adapter.ReadWorkbook(
			BaseWorkbookPath,
			BaseAfterStage,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}

	// Base semantic hash after staged write입니다.
	FString BaseAfterStageHash;
	if (!TestTrue(
		TEXT("Base post-stage semantic hash builds"),
		FCFContentSemanticHasher::BuildWorkbookSemanticHash(
			BaseAfterStage,
			BaseAfterStageHash,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}
	TestEqual(
		TEXT("Base Workbook is not overwritten by staged write"),
		BaseAfterStageHash,
		ExpectedBaseHash);

	CleanupFiles();
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007ProviderBridgeTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.ProviderCutoverBridge",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Existing ICFContentProvider reviewed seam을 P0-07 binding으로 재사용하고 absent Product Create를 fail-closed하는지 검증합니다.
bool FCFP007ProviderBridgeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Disposable shared Product state입니다.
	TSharedRef<FSharedProductState> SharedState =
		MakeShared<FSharedProductState>();

	// Existing persisted Product record입니다.
	const FCFContentRecord CurrentRecord =
		MakeRecord(TEXT("A"), 1.0);
	SharedState->RecordsByKey.Add(
		CurrentRecord.Key.ToStableString(),
		CurrentRecord);

	// Existing generic provider seam fixture입니다.
	TSharedRef<FReadProvider> Provider =
		MakeShared<FReadProvider>(
			SharedState,
			MakeSchema());

	// Provider-owned logical content type입니다.
	FCFContentTypeId ContentTypeId;
	ContentTypeId.Value = TEXT("Weapon");

	// Production-shape provider cutover adapter입니다.
	FCFProviderCutoverAdapter Adapter(
		*Provider,
		ContentTypeId);

	// Reviewed desired Product record입니다.
	const FCFContentRecord DesiredRecord =
		MakeRecord(TEXT("A"), 2.0);

	// Current Product state입니다.
	FCFContentProductState CurrentState;

	// Adapter diagnostic입니다.
	FString Error;
	if (!TestTrue(
		TEXT("Provider bridge reads existing Product"),
		Adapter.ReadCurrentProductState(
			CurrentRecord.Key,
			CurrentState,
			Error)))
	{
		AddError(Error);
		return false;
	}
	TestTrue(
		TEXT("Existing Product is present"),
		CurrentState.bExists);

	// Provider-local desired Product fingerprint입니다.
	FString DesiredFingerprint;
	if (!TestTrue(
		TEXT("Provider bridge builds desired fingerprint"),
		Adapter.BuildDesiredProductFingerprint(
			DesiredRecord,
			DesiredFingerprint,
			Error)))
	{
		AddError(Error);
		return false;
	}

	// Exact P0-07 Product binding입니다.
	FCFContentCutoverBinding Binding;
	Binding.ContentKey = DesiredRecord.Key;
	Binding.BaseWorkbookSemanticHash =
		HashText(TEXT("P007.ProviderBridge.BaseWorkbook"));
	Binding.PreApplyProductFingerprint =
		CurrentState.ProductFingerprint;
	Binding.DesiredProductFingerprint =
		DesiredFingerprint;

	// Existing provider reviewed apply result입니다.
	FCFContentApplyResult ApplyResult;
	if (!TestTrue(
		TEXT("Provider bridge reuses reviewed apply seam"),
		Adapter.ApplyReviewedProduct(
			Binding,
			DesiredRecord,
			ApplyResult)))
	{
		AddError(ApplyResult.Error);
		return false;
	}
	TestTrue(
		TEXT("Existing provider reports persistent mutation"),
		ApplyResult.bApplied);
	TestEqual(
		TEXT("Provider bridge apply exact1"),
		SharedState->ApplyCallCount,
		1);
	TestEqual(
		TEXT("Provider bridge readback matches desired"),
		ApplyResult.ReadbackFingerprint,
		DesiredFingerprint);

	// Absent Product key입니다.
	const FCFContentKey AbsentKey =
		MakeKey(TEXT("B"));

	// Explicit absence Product state입니다.
	FCFContentProductState AbsentState;
	TestTrue(
		TEXT("Provider bridge reports explicit absence"),
		Adapter.ReadCurrentProductState(
			AbsentKey,
			AbsentState,
			Error));
	TestFalse(
		TEXT("Absent Product is not treated as existing"),
		AbsentState.bExists);

	// Absent Product desired record입니다.
	const FCFContentRecord AbsentDesired =
		MakeRecord(TEXT("B"), 3.0);

	// Absent desired fingerprint should remain unset on fail-closed.
	FString AbsentDesiredFingerprint;
	TestFalse(
		TEXT("P0-07 provider bridge does not invent new Product writer"),
		Adapter.BuildDesiredProductFingerprint(
			AbsentDesired,
			AbsentDesiredFingerprint,
			Error));
	TestTrue(
		TEXT("Absent Product create block is fail-visible"),
		!Error.IsEmpty());
	TestEqual(
		TEXT("Absent Product does not trigger another apply"),
		SharedState->ApplyCallCount,
		1);
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFP007FileStoreTest,
	"CarFight.CCAS.CF_FQ_058.P0_07.DurableFileStore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Production FileStore의 transaction/provenance JSON durable save→load roundtrip을 disposable UE/Saved 경로에서 검증합니다.
bool FCFP007FileStoreTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	using namespace CFContentCutoverTestsPrivate;

	// Disposable transaction identity입니다.
	const FString TransactionId = TEXT("P007FileStoreTest");

	// Repository-relative disposable provenance path입니다.
	const FString ProvenancePath =
		TEXT("UE/Saved/CCAS/P007Tests/P007_FileStore.cfsnapshot.json");

	// Transaction JSON physical path입니다.
	const FString TransactionPhysicalPath = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CCAS/Cutover/P007FileStoreTest.json"));

	// Provenance JSON physical path입니다.
	const FString ProvenancePhysicalPath = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CCAS/P007Tests/P007_FileStore.cfsnapshot.json"));

	// Transaction atomic sibling temp path입니다.
	const FString TransactionTempPhysicalPath =
		TransactionPhysicalPath + TEXT(".cutover-writing");

	// Provenance atomic sibling temp path입니다.
	const FString ProvenanceTempPhysicalPath =
		ProvenancePhysicalPath + TEXT(".cutover-writing");

	// Transaction identity mismatch fixture path입니다.
	const FString AliasTransactionPhysicalPath = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CCAS/Cutover/P007FileStoreAlias.json"));

	// Test-owned durable residue cleanup function입니다.
	const auto CleanupFiles = [&]()
	{
		IFileManager::Get().Delete(
			*TransactionPhysicalPath,
			false,
			true,
			true);
		IFileManager::Get().Delete(
			*ProvenancePhysicalPath,
			false,
			true,
			true);
		IFileManager::Get().Delete(
			*TransactionTempPhysicalPath,
			false,
			true,
			true);
		IFileManager::Get().Delete(
			*ProvenanceTempPhysicalPath,
			false,
			true,
			true);
		IFileManager::Get().Delete(
			*AliasTransactionPhysicalPath,
			false,
			true,
			true);
	};

	CleanupFiles();

	// Production durable store입니다.
	FCFContentCutoverFileStore Store;

	// Canonical test target key입니다.
	const FCFContentKey ContentKey =
		MakeKey(TEXT("A"));

	// Durable target report입니다.
	FCFContentCutoverTargetReport Target;
	Target.ContentKey = ContentKey;
	Target.State = ECFContentCutoverTargetState::Verified;
	Target.FreshPreApplyProductFingerprint =
		HashText(TEXT("P007.FileStore.Pre"));
	Target.DesiredProductFingerprint =
		HashText(TEXT("P007.FileStore.Desired"));
	Target.PostApplyReadbackFingerprint =
		Target.DesiredProductFingerprint;
	Target.bProductMutationPerformed = true;
	Target.Diagnostic = TEXT("Verified disposable target.");

	// Durable transaction fixture입니다.
	FCFContentCutoverTransaction Transaction;
	Transaction.TransactionId = TransactionId;
	Transaction.State = ECFContentCutoverState::Verified;
	Transaction.CanonicalWorkbookPath =
		TEXT("Authoring/Content/CarFight_Content.xlsx");
	Transaction.StagedWorkbookPath =
		TEXT("Authoring/Content/CarFight_Content.staged.xlsx");
	Transaction.ExpectedWorkbookSemanticHash =
		HashText(TEXT("P007.FileStore.Workbook"));
	Transaction.ManifestFingerprint =
		HashText(TEXT("P007.FileStore.Manifest"));
	Transaction.Targets.Add(Target);
	Transaction.bAuthorityActivationEligible = true;
	Transaction.bAuthorityActivated = false;
	Transaction.Diagnostic = TEXT("Disposable durable transaction.");

	// Durable operation diagnostic입니다.
	FString Error;
	if (!TestTrue(
		TEXT("Production FileStore transaction save succeeds"),
		Store.SaveTransaction(
			Transaction,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}
	TestTrue(
		TEXT("Transaction JSON physically exists"),
		IFileManager::Get().FileExists(*TransactionPhysicalPath));
	TestFalse(
		TEXT("Transaction atomic temp residue exact0"),
		IFileManager::Get().FileExists(*TransactionTempPhysicalPath));

	// Reloaded durable transaction입니다.
	FCFContentCutoverTransaction ReloadedTransaction;
	if (!TestTrue(
		TEXT("Production FileStore transaction load succeeds"),
		Store.LoadTransaction(
			TransactionId,
			ReloadedTransaction,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}

	TestEqual(
		TEXT("TransactionId roundtrip exact"),
		ReloadedTransaction.TransactionId,
		Transaction.TransactionId);
	TestEqual(
		TEXT("Transaction state roundtrip exact"),
		ReloadedTransaction.State,
		Transaction.State);
	TestEqual(
		TEXT("Manifest fingerprint roundtrip exact"),
		ReloadedTransaction.ManifestFingerprint,
		Transaction.ManifestFingerprint);
	TestTrue(
		TEXT("Authority eligibility roundtrip exact"),
		ReloadedTransaction.bAuthorityActivationEligible);
	TestFalse(
		TEXT("Authority activation remains false"),
		ReloadedTransaction.bAuthorityActivated);
	TestEqual(
		TEXT("Target evidence exact1"),
		ReloadedTransaction.Targets.Num(),
		1);
	if (ReloadedTransaction.Targets.Num() != 1)
	{
		CleanupFiles();
		return false;
	}
	TestEqual(
		TEXT("Target post-readback roundtrip exact"),
		ReloadedTransaction.Targets[0].PostApplyReadbackFingerprint,
		Target.PostApplyReadbackFingerprint);

	// Exact bytes를 다른 transaction filename으로 복제해 payload identity mismatch를 만듭니다.
	const uint32 AliasCopyResult = IFileManager::Get().Copy(
		*AliasTransactionPhysicalPath,
		*TransactionPhysicalPath,
		true,
		true);
	TestEqual(
		TEXT("Transaction alias fixture copy succeeds"),
		AliasCopyResult,
		static_cast<uint32>(COPY_OK));
	// Alias filename으로 읽은 mismatched payload transaction입니다.
	FCFContentCutoverTransaction AliasTransaction;
	FString AliasError;
	TestFalse(
		TEXT("Transaction filename/payload identity mismatch fails closed"),
		Store.LoadTransaction(
			TEXT("P007FileStoreAlias"),
			AliasTransaction,
			AliasError));
	TestTrue(
		TEXT("Transaction identity mismatch is fail-visible"),
		!AliasError.IsEmpty());

	// Mandatory generated provenance record입니다.
	FCFContentProvenanceRecord ProvenanceRecord;
	ProvenanceRecord.ContentKey = ContentKey;
	ProvenanceRecord.CutoverTransactionId = TransactionId;
	ProvenanceRecord.SourceWorkbookSemanticHash =
		HashText(TEXT("P007.FileStore.SourceWorkbook"));
	ProvenanceRecord.CatalogSnapshotFingerprint =
		HashText(TEXT("P007.FileStore.Catalog"));
	ProvenanceRecord.ReviewPackageFingerprint =
		HashText(TEXT("P007.FileStore.Review"));
	ProvenanceRecord.ProviderSchemaFingerprint =
		HashText(TEXT("P007.FileStore.Schema"));
	ProvenanceRecord.DesiredCanonicalFingerprint =
		HashText(TEXT("P007.FileStore.Canonical"));
	ProvenanceRecord.PreApplyProductFingerprint =
		Target.FreshPreApplyProductFingerprint;
	ProvenanceRecord.PostApplyReadbackFingerprint =
		Target.PostApplyReadbackFingerprint;
	ProvenanceRecord.ManagementState =
		ECFContentManagementState::Managed;
	ProvenanceRecord.LifecycleState =
		ECFContentLifecycleState::Active;

	// Durable generated provenance snapshot입니다.
	FCFContentProvenanceSnapshot Snapshot;
	Snapshot.CutoverTransactionId = TransactionId;
	Snapshot.Records.Add(ProvenanceRecord);

	if (!TestTrue(
		TEXT("Production FileStore provenance save succeeds"),
		Store.SaveProvenance(
			ProvenancePath,
			Snapshot,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}
	TestTrue(
		TEXT("Provenance JSON physically exists"),
		IFileManager::Get().FileExists(*ProvenancePhysicalPath));
	TestFalse(
		TEXT("Provenance atomic temp residue exact0"),
		IFileManager::Get().FileExists(*ProvenanceTempPhysicalPath));

	// Reloaded durable provenance snapshot입니다.
	FCFContentProvenanceSnapshot ReloadedSnapshot;
	if (!TestTrue(
		TEXT("Production FileStore provenance load succeeds"),
		Store.LoadProvenance(
			ProvenancePath,
			ReloadedSnapshot,
			Error)))
	{
		AddError(Error);
		CleanupFiles();
		return false;
	}

	TestEqual(
		TEXT("Provenance schema exact"),
		ReloadedSnapshot.SchemaId,
		Snapshot.SchemaId);
	TestEqual(
		TEXT("Provenance transaction exact"),
		ReloadedSnapshot.CutoverTransactionId,
		TransactionId);
	TestEqual(
		TEXT("Provenance records exact1"),
		ReloadedSnapshot.Records.Num(),
		1);
	if (ReloadedSnapshot.Records.Num() != 1)
	{
		CleanupFiles();
		return false;
	}

	// Reloaded mandatory provenance record입니다.
	const FCFContentProvenanceRecord& ReloadedRecord =
		ReloadedSnapshot.Records[0];
	TestEqual(
		TEXT("Provenance ContentKey exact"),
		ReloadedRecord.ContentKey.ToStableString(),
		ContentKey.ToStableString());
	TestEqual(
		TEXT("Provenance source Workbook hash exact"),
		ReloadedRecord.SourceWorkbookSemanticHash,
		ProvenanceRecord.SourceWorkbookSemanticHash);
	TestEqual(
		TEXT("Provenance Catalog fingerprint exact"),
		ReloadedRecord.CatalogSnapshotFingerprint,
		ProvenanceRecord.CatalogSnapshotFingerprint);
	TestEqual(
		TEXT("Provenance Review fingerprint exact"),
		ReloadedRecord.ReviewPackageFingerprint,
		ProvenanceRecord.ReviewPackageFingerprint);
	TestEqual(
		TEXT("Provenance Provider schema exact"),
		ReloadedRecord.ProviderSchemaFingerprint,
		ProvenanceRecord.ProviderSchemaFingerprint);
	TestEqual(
		TEXT("Provenance desired canonical exact"),
		ReloadedRecord.DesiredCanonicalFingerprint,
		ProvenanceRecord.DesiredCanonicalFingerprint);
	TestEqual(
		TEXT("Provenance pre-apply Product exact"),
		ReloadedRecord.PreApplyProductFingerprint,
		ProvenanceRecord.PreApplyProductFingerprint);
	TestEqual(
		TEXT("Provenance post-apply Product exact"),
		ReloadedRecord.PostApplyReadbackFingerprint,
		ProvenanceRecord.PostApplyReadbackFingerprint);
	TestEqual(
		TEXT("Provenance management state exact"),
		ReloadedRecord.ManagementState,
		ECFContentManagementState::Managed);
	TestEqual(
		TEXT("Provenance lifecycle state exact"),
		ReloadedRecord.LifecycleState,
		ECFContentLifecycleState::Active);

	CleanupFiles();
	return true;
}
