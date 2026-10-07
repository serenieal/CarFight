// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCompiler.cpp
// Version: v1.3.0
// Date: 2026-09-30
// Description: CF-FQ-058 CCAS Generic Compiler Core read-only preview implementation입니다.
// Changelog:
// - v1.3.0: P0-06 Catalog Snapshot이 재사용할 verified whole-current ProductSemanticHash를 CompileResult에 보존.
// - v1.2.0: Re-review 보강으로 whole-current pre/post import semantic consistency와 BuildDiff current fingerprint ↔ snapshot readback fingerprint exact consistency guard를 추가.
// - v1.1.0: Mid-review P1 exact3와 snapshot consistency P2를 교정. Provider schema exact compatibility, canonical provider fingerprint fail-closed, current Product reference graph 기반 retire impact, import/snapshot consistency guard를 추가.
// - v1.0.0: Adapter read, common/provider validation, current import, typed diff, reference impact, generated drift, preview batch result를 최초 구현.
// Migration:
// - Product ApplyReviewed/Save/rename/move/delete를 호출하지 않습니다.
// - Drift 판정의 managed semantic scope는 Provider BuildDiff가 소유하고 Core는 결과를 집계합니다.

#include "DataAuthoring/CFContentCompiler.h"

#include "Containers/Queue.h"
#include "DataAuthoring/CFContentCore.h"
#include "DataAuthoring/CFDACommonPrimitives.h"

namespace CFContentCompilerPrivate
{
	/** Stable diagnostic 하나를 compiler result에 추가합니다. */
	void AddIssue(
		TArray<FCFContentValidationIssue>& OutIssues,
		const FString& Code,
		const FString& Path,
		const FString& Message,
		const bool bBlocking = true)
	{
		// 추가할 typed validation issue입니다.
		FCFContentValidationIssue Issue;
		Issue.Code = Code;
		Issue.Path = Path;
		Issue.Message = Message;
		Issue.bBlocking = bBlocking;
		OutIssues.Add(MoveTemp(Issue));
	}

	/** 다른 validator/provider에서 반환된 issue 집합을 보존 순서대로 append합니다. */
	void AppendIssues(
		TArray<FCFContentValidationIssue>& OutIssues,
		const TArray<FCFContentValidationIssue>& SourceIssues)
	{
		OutIssues.Append(SourceIssues);
	}

	/** Blocking validation issue가 하나라도 존재하는지 확인합니다. */
	bool HasBlockingIssues(const TArray<FCFContentValidationIssue>& Issues)
	{
		for (const FCFContentValidationIssue& Issue : Issues)
		{
			if (Issue.bBlocking)
			{
				return true;
			}
		}
		return false;
	}

	/** Compiler failure 상태를 일관되게 기록합니다. */
	bool Fail(
		FCFContentCompileResult& OutResult,
		const FString& Stage,
		const FString& Error)
	{
		OutResult.bSucceeded = false;
		OutResult.FailureStage = Stage;
		OutResult.Error = Error;
		OutResult.BatchResult.BlockedCount = FMath::Max(1, OutResult.Workbook.Records.Num());
		return false;
	}

	/** ContentKey array를 stable machine identity 순서로 정렬합니다. */
	void SortContentKeys(TArray<FCFContentKey>& Keys)
	{
		Keys.Sort([](const FCFContentKey& Left, const FCFContentKey& Right)
		{
			return Left.ToStableString() < Right.ToStableString();
		});
	}

	/** P0-01 WorkbookSemanticHash를 재사용해 record 하나의 canonical projection fingerprint를 생성합니다. */
	bool BuildRecordProjectionFingerprint(
		const FCFContentWorkbookModel& SourceWorkbook,
		const FCFContentRecord& Record,
		FString& OutFingerprint,
		FString& OutError)
	{
		OutFingerprint.Reset();
		OutError.Reset();

		// Source Workbook identity/revision을 보존하는 single-record canonical projection입니다.
		FCFContentWorkbookModel Projection;
		Projection.WorkbookSourceId = SourceWorkbook.WorkbookSourceId;
		Projection.SchemaRevision = SourceWorkbook.SchemaRevision;

		for (const FCFContentSheetDescriptor& Sheet : SourceWorkbook.Sheets)
		{
			if (Sheet.ContentTypeId == Record.Key.ContentTypeId)
			{
				Projection.Sheets.Add(Sheet);
			}
		}

		Projection.Records.Add(Record);
		return FCFContentSemanticHasher::BuildWorkbookSemanticHash(
			Projection,
			OutFingerprint,
			OutError);
	}

	/** Workbook primary sheets에서 provider import 대상 ContentType을 exact1씩 수집합니다. */
	void CollectProviderTypes(
		const FCFContentWorkbookModel& Workbook,
		TArray<FCFContentTypeId>& OutContentTypes)
	{
		OutContentTypes.Reset();

		// 중복 ContentTypeId 제거용 stable key set입니다.
		TSet<FString> SeenTypeIds;
		for (const FCFContentSheetDescriptor& Sheet : Workbook.Sheets)
		{
			if (Sheet.IsChildSheet() || !Sheet.ContentTypeId.IsValid())
			{
				continue;
			}
			if (SeenTypeIds.Contains(Sheet.ContentTypeId.Value))
			{
				continue;
			}

			SeenTypeIds.Add(Sheet.ContentTypeId.Value);
			OutContentTypes.Add(Sheet.ContentTypeId);
		}

		OutContentTypes.Sort([](const FCFContentTypeId& Left, const FCFContentTypeId& Right)
		{
			return Left.ToStableString() < Right.ToStableString();
		});
	}

	/** 두 field descriptor가 presentation label을 제외하고 exact semantic schema가 같은지 확인합니다. */
	bool AreFieldDescriptorsCompatible(
		const FCFContentFieldDescriptor& WorkbookField,
		const FCFContentFieldDescriptor& ProviderField)
	{
		return WorkbookField.ColumnId.Equals(ProviderField.ColumnId, ESearchCase::CaseSensitive)
			&& WorkbookField.ValueType == ProviderField.ValueType
			&& WorkbookField.Ownership == ProviderField.Ownership
			&& WorkbookField.ExternalOwnerId.Equals(ProviderField.ExternalOwnerId, ESearchCase::CaseSensitive)
			&& WorkbookField.CanonicalUnitId.Equals(ProviderField.CanonicalUnitId, ESearchCase::CaseSensitive)
			&& WorkbookField.FieldDomainId.Equals(ProviderField.FieldDomainId, ESearchCase::CaseSensitive)
			&& WorkbookField.TextMode == ProviderField.TextMode
			&& WorkbookField.bAllowNone == ProviderField.bAllowNone
			&& WorkbookField.bRequired == ProviderField.bRequired;
	}

	/** 두 logical sheet descriptor가 physical presentation을 제외하고 exact semantic schema가 같은지 확인합니다. */
	bool AreSheetDescriptorsCompatible(
		const FCFContentSheetDescriptor& WorkbookSheet,
		const FCFContentSheetDescriptor& ProviderSheet,
		FString& OutMismatch)
	{
		OutMismatch.Reset();

		if (!WorkbookSheet.SheetId.Equals(ProviderSheet.SheetId, ESearchCase::CaseSensitive)
			|| WorkbookSheet.SchemaRevision != ProviderSheet.SchemaRevision
			|| !(WorkbookSheet.ContentTypeId == ProviderSheet.ContentTypeId)
			|| !WorkbookSheet.ParentSheetId.Equals(ProviderSheet.ParentSheetId, ESearchCase::CaseSensitive)
			|| !WorkbookSheet.CollectionId.Equals(ProviderSheet.CollectionId, ESearchCase::CaseSensitive)
			|| WorkbookSheet.CollectionKind != ProviderSheet.CollectionKind)
		{
			OutMismatch = TEXT("Sheet identity/revision/collection contract mismatch.");
			return false;
		}

		if (WorkbookSheet.Fields.Num() != ProviderSheet.Fields.Num())
		{
			OutMismatch = TEXT("Field count mismatch.");
			return false;
		}

		for (const FCFContentFieldDescriptor& WorkbookField : WorkbookSheet.Fields)
		{
			// Same stable ColumnId의 provider field입니다.
			const FCFContentFieldDescriptor* ProviderField = ProviderSheet.Fields.FindByPredicate(
				[&WorkbookField](const FCFContentFieldDescriptor& Candidate)
				{
					return Candidate.ColumnId.Equals(
						WorkbookField.ColumnId,
						ESearchCase::CaseSensitive);
				});
			if (ProviderField == nullptr)
			{
				OutMismatch = FString::Printf(
					TEXT("Provider schema에 ColumnId '%s'가 없습니다."),
					*WorkbookField.ColumnId);
				return false;
			}
			if (!AreFieldDescriptorsCompatible(WorkbookField, *ProviderField))
			{
				OutMismatch = FString::Printf(
					TEXT("ColumnId '%s' semantic descriptor가 일치하지 않습니다."),
					*WorkbookField.ColumnId);
				return false;
			}
		}

		return true;
	}

	/** Workbook logical schema와 registered Provider descriptor가 exact compatible인지 검증합니다. */
	bool ValidateProviderSchemaCompatibility(
		const FCFContentWorkbookModel& Workbook,
		const FCFContentProviderRegistry& ProviderRegistry,
		TArray<FCFContentValidationIssue>& OutIssues,
		FString& OutError)
	{
		OutError.Reset();

		// Workbook primary ContentType 목록입니다.
		TArray<FCFContentTypeId> ProviderTypes;
		CollectProviderTypes(Workbook, ProviderTypes);

		for (const FCFContentTypeId& ContentTypeId : ProviderTypes)
		{
			// Exact ContentType provider입니다.
			ICFContentProvider* Provider = ProviderRegistry.FindProvider(ContentTypeId);
			if (Provider == nullptr)
			{
				AddIssue(
					OutIssues,
					TEXT("MissingProvider"),
					ContentTypeId.ToStableString(),
					TEXT("Schema compatibility 검증에 필요한 Provider가 없습니다."));
				continue;
			}

			// Provider-owned schema descriptor입니다.
			FCFContentProviderDescriptor Descriptor;
			// DescribeSchema failure 상세입니다.
			FString DescribeError;
			if (!Provider->DescribeSchema(Descriptor, DescribeError))
			{
				AddIssue(
					OutIssues,
					TEXT("ProviderSchemaDescribeFailed"),
					ContentTypeId.ToStableString(),
					DescribeError.IsEmpty()
						? TEXT("Provider DescribeSchema가 실패했습니다.")
						: DescribeError);
				continue;
			}

			// 이 ContentType의 Workbook sheet 목록입니다.
			TArray<const FCFContentSheetDescriptor*> WorkbookSheets;
			for (const FCFContentSheetDescriptor& Sheet : Workbook.Sheets)
			{
				if (Sheet.ContentTypeId == ContentTypeId)
				{
					WorkbookSheets.Add(&Sheet);
				}
			}

			if (WorkbookSheets.Num() != Descriptor.Sheets.Num())
			{
				AddIssue(
					OutIssues,
					TEXT("ProviderSchemaSheetCountMismatch"),
					ContentTypeId.ToStableString(),
					TEXT("Workbook과 Provider의 logical sheet 수가 일치하지 않습니다. Schema migration preview가 필요합니다."));
				continue;
			}

			// Provider revision과 비교할 Workbook primary sheet입니다.
			const FCFContentSheetDescriptor* WorkbookPrimary = nullptr;
			for (const FCFContentSheetDescriptor* Sheet : WorkbookSheets)
			{
				if (Sheet != nullptr && !Sheet->IsChildSheet())
				{
					WorkbookPrimary = Sheet;
					break;
				}
			}

			if (WorkbookPrimary == nullptr
				|| Descriptor.SchemaRevision != WorkbookPrimary->SchemaRevision)
			{
				AddIssue(
					OutIssues,
					TEXT("ProviderSchemaRevisionMismatch"),
					ContentTypeId.ToStableString(),
					TEXT("Workbook primary Sheet SchemaRevision과 Provider SchemaRevision이 일치하지 않습니다. 암묵적 해석 대신 migration이 필요합니다."));
				continue;
			}

			for (const FCFContentSheetDescriptor* WorkbookSheet : WorkbookSheets)
			{
				if (WorkbookSheet == nullptr)
				{
					continue;
				}

				// 같은 stable SheetId의 provider descriptor입니다.
				const FCFContentSheetDescriptor* ProviderSheet = Descriptor.Sheets.FindByPredicate(
					[WorkbookSheet](const FCFContentSheetDescriptor& Candidate)
					{
						return Candidate.SheetId.Equals(
							WorkbookSheet->SheetId,
							ESearchCase::CaseSensitive);
					});
				if (ProviderSheet == nullptr)
				{
					AddIssue(
						OutIssues,
						TEXT("ProviderSchemaMissingSheet"),
						WorkbookSheet->SheetId,
						TEXT("Provider schema에 Workbook SheetId가 없습니다. Schema migration이 필요합니다."));
					continue;
				}

				// Exact sheet mismatch 상세입니다.
				FString Mismatch;
				if (!AreSheetDescriptorsCompatible(*WorkbookSheet, *ProviderSheet, Mismatch))
				{
					AddIssue(
						OutIssues,
						TEXT("ProviderSchemaMismatch"),
						WorkbookSheet->SheetId,
						Mismatch);
				}
			}
		}

		if (HasBlockingIssues(OutIssues))
		{
			OutError = TEXT("Workbook/Provider schema compatibility가 fail-closed 됐습니다.");
			return false;
		}
		return true;
	}

	/** Provider별 Existing Product records를 한 번씩 import해서 O(N) current index를 구축합니다. */
	bool BuildCurrentRecordIndex(
		const FCFContentWorkbookModel& Workbook,
		const FCFContentProviderRegistry& ProviderRegistry,
		TMap<FString, FCFContentRecord>& OutCurrentRecordsByKey,
		TArray<FCFContentValidationIssue>& OutIssues,
		FString& OutError)
	{
		OutCurrentRecordsByKey.Reset();
		OutError.Reset();

		// Workbook primary sheets가 요구하는 provider ContentType 목록입니다.
		TArray<FCFContentTypeId> ProviderTypes;
		CollectProviderTypes(Workbook, ProviderTypes);

		for (const FCFContentTypeId& ContentTypeId : ProviderTypes)
		{
			// Exact ContentType의 registered provider입니다.
			ICFContentProvider* Provider = ProviderRegistry.FindProvider(ContentTypeId);
			if (Provider == nullptr)
			{
				AddIssue(
					OutIssues,
					TEXT("MissingProvider"),
					ContentTypeId.ToStableString(),
					TEXT("Workbook primary ContentType에 등록된 Provider가 없습니다."));
				continue;
			}

			// Provider read-only existing Product import 결과입니다.
			FCFContentImportResult ImportResult;
			// Provider import failure 상세 원인입니다.
			FString ImportError;
			if (!Provider->ImportExisting(ImportResult, ImportError))
			{
				AddIssue(
					OutIssues,
					TEXT("ProviderImportFailed"),
					ContentTypeId.ToStableString(),
					ImportError.IsEmpty()
						? TEXT("Provider ImportExisting가 실패했습니다.")
						: ImportError);
				continue;
			}

			AppendIssues(OutIssues, ImportResult.Issues);

			for (const FCFContentRecord& CurrentRecord : ImportResult.Records)
			{
				// Imported record의 canonical key text입니다.
				const FString CurrentKeyText = CurrentRecord.Key.ToStableString();
				if (!CurrentRecord.Key.IsValid()
					|| !(CurrentRecord.Key.ContentTypeId == ContentTypeId))
				{
					AddIssue(
						OutIssues,
						TEXT("InvalidImportedRecord"),
						CurrentKeyText,
						TEXT("Provider ImportExisting record의 ContentType/ContentKey가 provider scope와 일치하지 않습니다."));
					continue;
				}
				if (OutCurrentRecordsByKey.Contains(CurrentKeyText))
				{
					AddIssue(
						OutIssues,
						TEXT("DuplicateImportedRecord"),
						CurrentKeyText,
						TEXT("Provider import 결과에 duplicate current ContentKey가 있습니다."));
					continue;
				}

				OutCurrentRecordsByKey.Add(CurrentKeyText, CurrentRecord);
			}
		}

		if (HasBlockingIssues(OutIssues))
		{
			OutError = TEXT("Provider current import validation이 fail-closed 됐습니다.");
			return false;
		}
		return true;
	}

	/** Imported current Product 전체를 deterministic canonical semantic hash로 묶습니다. */
	bool BuildCurrentImportSemanticHash(
		const FCFContentWorkbookModel& DesiredWorkbook,
		const TMap<FString, FCFContentRecord>& CurrentRecordsByKey,
		FString& OutSemanticHash,
		FString& OutError)
	{
		// Current import 전체를 나타내는 canonical in-memory model입니다.
		FCFContentWorkbookModel CurrentModel;
		CurrentModel.WorkbookSourceId = DesiredWorkbook.WorkbookSourceId;
		CurrentModel.SchemaRevision = DesiredWorkbook.SchemaRevision;
		CurrentModel.Sheets = DesiredWorkbook.Sheets;
		for (const TPair<FString, FCFContentRecord>& Pair : CurrentRecordsByKey)
		{
			CurrentModel.Records.Add(Pair.Value);
		}

		return FCFContentSemanticHasher::BuildWorkbookSemanticHash(
			CurrentModel,
			OutSemanticHash,
			OutError);
	}

	/** Imported current Product records에서 provider dependency graph를 구축하고 missing source/target를 fail-closed 합니다. */
	bool BuildCurrentDependencyEdges(
		const FCFContentWorkbookModel& DesiredWorkbook,
		const FCFContentProviderRegistry& ProviderRegistry,
		const TMap<FString, FCFContentRecord>& CurrentRecordsByKey,
		TArray<FCFContentDependencyEdge>& OutDependencyEdges,
		TArray<FCFContentValidationIssue>& OutIssues,
		FString& OutError)
	{
		OutDependencyEdges.Reset();
		OutError.Reset();

		for (const TPair<FString, FCFContentRecord>& Pair : CurrentRecordsByKey)
		{
			// Imported current canonical record입니다.
			const FCFContentRecord& Record = Pair.Value;
			// Current record exact provider입니다.
			ICFContentProvider* Provider = ProviderRegistry.FindProvider(Record.Key.ContentTypeId);
			if (Provider == nullptr)
			{
				AddIssue(
					OutIssues,
					TEXT("MissingProvider"),
					Record.Key.ToStableString(),
					TEXT("Current reference graph 구축에 필요한 Provider가 없습니다."));
				continue;
			}

			// Current record가 만드는 provider-specific dependency edges입니다.
			TArray<FCFContentDependencyEdge> RecordEdges;
			// Dependency build failure 상세입니다.
			FString DependencyError;
			if (!Provider->BuildDependencyEdges(Record, RecordEdges, DependencyError))
			{
				AddIssue(
					OutIssues,
					TEXT("CurrentDependencyBuildFailed"),
					Record.Key.ToStableString(),
					DependencyError.IsEmpty()
						? TEXT("Current Product record의 BuildDependencyEdges가 실패했습니다.")
						: DependencyError);
				continue;
			}
			OutDependencyEdges.Append(RecordEdges);
		}

		if (HasBlockingIssues(OutIssues))
		{
			OutError = TEXT("Current Product dependency graph build가 fail-closed 됐습니다.");
			return false;
		}

		// Current graph endpoint 검증용 canonical model입니다.
		FCFContentWorkbookModel CurrentGraphModel;
		CurrentGraphModel.WorkbookSourceId = DesiredWorkbook.WorkbookSourceId;
		CurrentGraphModel.SchemaRevision = DesiredWorkbook.SchemaRevision;
		CurrentGraphModel.Sheets = DesiredWorkbook.Sheets;
		for (const TPair<FString, FCFContentRecord>& Pair : CurrentRecordsByKey)
		{
			CurrentGraphModel.Records.Add(Pair.Value);
		}

		// Current graph missing source/target/cycle diagnostics입니다.
		TArray<FCFContentValidationIssue> CurrentDependencyIssues;
		if (!FCFContentSchemaValidator::ValidateDependencyEdges(
			CurrentGraphModel,
			OutDependencyEdges,
			CurrentDependencyIssues))
		{
			AppendIssues(OutIssues, CurrentDependencyIssues);
			OutError = TEXT("Current Product reference graph validation이 fail-closed 됐습니다.");
			return false;
		}

		return true;
	}

	/** Desired records에 common provider semantic validation과 dependency edge build를 적용합니다. */
	bool ValidateDesiredRecords(
		const FCFContentWorkbookModel& Workbook,
		const FCFContentProviderRegistry& ProviderRegistry,
		TArray<FCFContentDependencyEdge>& OutDependencyEdges,
		TArray<FCFContentValidationIssue>& OutIssues,
		FString& OutError)
	{
		OutDependencyEdges.Reset();
		OutError.Reset();

		for (const FCFContentRecord& Record : Workbook.Records)
		{
			// Desired record exact provider입니다.
			ICFContentProvider* Provider = ProviderRegistry.FindProvider(Record.Key.ContentTypeId);
			// Desired record stable diagnostic path입니다.
			const FString RecordPath = Record.Key.ToStableString();
			if (Provider == nullptr)
			{
				AddIssue(
					OutIssues,
					TEXT("MissingProvider"),
					RecordPath,
					TEXT("Desired record의 ContentType을 소유하는 Provider가 없습니다."));
				continue;
			}

			// Provider-specific semantic diagnostics입니다.
			TArray<FCFContentValidationIssue> ProviderIssues;
			if (!Provider->ValidateRecord(Record, ProviderIssues))
			{
				if (ProviderIssues.IsEmpty())
				{
					AddIssue(
						OutIssues,
						TEXT("ProviderValidationFailed"),
						RecordPath,
						TEXT("Provider ValidateRecord가 실패했지만 상세 issue를 반환하지 않았습니다."));
				}
			}
			AppendIssues(OutIssues, ProviderIssues);

			// 이 record가 만드는 provider-specific dependency edges입니다.
			TArray<FCFContentDependencyEdge> RecordEdges;
			// Dependency build failure 상세 원인입니다.
			FString DependencyError;
			if (!Provider->BuildDependencyEdges(Record, RecordEdges, DependencyError))
			{
				AddIssue(
					OutIssues,
					TEXT("DependencyBuildFailed"),
					RecordPath,
					DependencyError.IsEmpty()
						? TEXT("Provider BuildDependencyEdges가 실패했습니다.")
						: DependencyError);
				continue;
			}

			OutDependencyEdges.Append(RecordEdges);
		}

		if (HasBlockingIssues(OutIssues))
		{
			OutError = TEXT("Provider semantic/dependency validation이 fail-closed 됐습니다.");
			return false;
		}
		return true;
	}

	/** Provider diff와 canonical projection hash를 사용해 deterministic record-level diff/drift를 생성합니다. */
	bool BuildDiffsAndFingerprints(
		const FCFContentWorkbookModel& Workbook,
		const FString& WorkbookSemanticHash,
		const FCFContentProviderRegistry& ProviderRegistry,
		const TMap<FString, FCFContentRecord>& CurrentRecordsByKey,
		TArray<FCFContentCompileDiff>& OutDiffs,
		TArray<FCFContentGeneratedFingerprint>& OutGeneratedFingerprints,
		TArray<FCFContentValidationIssue>& OutIssues,
		FString& OutError)
	{
		OutDiffs.Reset();
		OutGeneratedFingerprints.Reset();
		OutError.Reset();

		// Desired에 소비되지 않은 current record를 Removed로 판정하기 위한 key set입니다.
		TSet<FString> RemainingCurrentKeys;
		for (const TPair<FString, FCFContentRecord>& Pair : CurrentRecordsByKey)
		{
			RemainingCurrentKeys.Add(Pair.Key);
		}

		for (const FCFContentRecord& DesiredRecord : Workbook.Records)
		{
			// Desired record canonical key text입니다.
			const FString DesiredKeyText = DesiredRecord.Key.ToStableString();
			// Desired record exact provider입니다.
			ICFContentProvider* Provider = ProviderRegistry.FindProvider(DesiredRecord.Key.ContentTypeId);
			if (Provider == nullptr)
			{
				AddIssue(
					OutIssues,
					TEXT("MissingProvider"),
					DesiredKeyText,
					TEXT("Diff 단계에서 Desired record Provider를 찾을 수 없습니다."));
				continue;
			}

			// Desired record canonical projection fingerprint입니다.
			FString DesiredCanonicalFingerprint;
			// Canonical projection hash failure 상세입니다.
			FString DesiredHashError;
			if (!BuildRecordProjectionFingerprint(
				Workbook,
				DesiredRecord,
				DesiredCanonicalFingerprint,
				DesiredHashError))
			{
				AddIssue(
					OutIssues,
					TEXT("DesiredFingerprintFailed"),
					DesiredKeyText,
					DesiredHashError.IsEmpty()
						? TEXT("Desired record canonical fingerprint 생성에 실패했습니다.")
						: DesiredHashError);
				continue;
			}

			// Record-level compiler diff 결과입니다.
			FCFContentCompileDiff CompileDiff;
			CompileDiff.Key = DesiredRecord.Key;
			CompileDiff.ManagementState = DesiredRecord.ManagementState;

			// Workbook/Product provenance를 연결하는 generated fingerprint record입니다.
			FCFContentGeneratedFingerprint GeneratedFingerprint;
			GeneratedFingerprint.Key = DesiredRecord.Key;
			GeneratedFingerprint.SourceWorkbookSemanticHash = WorkbookSemanticHash;
			GeneratedFingerprint.DesiredCanonicalFingerprint = DesiredCanonicalFingerprint;

			// Imported current record가 존재하는지 확인합니다.
			const FCFContentRecord* ImportedCurrent = CurrentRecordsByKey.Find(DesiredKeyText);
			if (ImportedCurrent == nullptr)
			{
				CompileDiff.Kind = ECFContentCompileDiffKind::Added;
				CompileDiff.ChangedPaths.Add(TEXT("$record"));
				GeneratedFingerprint.ChangedPaths = CompileDiff.ChangedPaths;
				GeneratedFingerprint.DriftState =
					DesiredRecord.ManagementState == ECFContentManagementState::ExternalReadOnly
						? ECFContentDriftState::ExternalReadOnly
						: ECFContentDriftState::MissingCurrent;

				OutDiffs.Add(MoveTemp(CompileDiff));
				OutGeneratedFingerprints.Add(MoveTemp(GeneratedFingerprint));
				continue;
			}

			RemainingCurrentKeys.Remove(DesiredKeyText);

			// Exact Product current snapshot입니다.
			FCFContentSnapshot CurrentSnapshot;
			// Snapshot build failure 상세입니다.
			FString SnapshotError;
			if (!Provider->BuildCurrentSnapshot(
				DesiredRecord.Key,
				CurrentSnapshot,
				SnapshotError))
			{
				AddIssue(
					OutIssues,
					TEXT("CurrentSnapshotFailed"),
					DesiredKeyText,
					SnapshotError.IsEmpty()
						? TEXT("Provider BuildCurrentSnapshot이 실패했습니다.")
						: SnapshotError);
				continue;
			}

			if (!(CurrentSnapshot.Key == DesiredRecord.Key)
				|| !(CurrentSnapshot.Record.Key == DesiredRecord.Key))
			{
				AddIssue(
					OutIssues,
					TEXT("CurrentSnapshotIdentityMismatch"),
					DesiredKeyText,
					TEXT("Provider current snapshot의 Key/Record.Key가 requested ContentKey와 일치하지 않습니다."));
				continue;
			}

			// ImportExisting 시점 current record의 canonical projection fingerprint입니다.
			FString ImportedCurrentFingerprint;
			// Import current fingerprint failure 상세입니다.
			FString ImportedHashError;
			if (!BuildRecordProjectionFingerprint(
				Workbook,
				*ImportedCurrent,
				ImportedCurrentFingerprint,
				ImportedHashError))
			{
				AddIssue(
					OutIssues,
					TEXT("ImportedCurrentFingerprintFailed"),
					DesiredKeyText,
					ImportedHashError);
				continue;
			}

			// BuildCurrentSnapshot 재조회 record의 canonical projection fingerprint입니다.
			FString SnapshotRecordFingerprint;
			// Snapshot record hash failure 상세입니다.
			FString SnapshotRecordHashError;
			if (!BuildRecordProjectionFingerprint(
				Workbook,
				CurrentSnapshot.Record,
				SnapshotRecordFingerprint,
				SnapshotRecordHashError))
			{
				AddIssue(
					OutIssues,
					TEXT("CurrentSnapshotFingerprintFailed"),
					DesiredKeyText,
					SnapshotRecordHashError);
				continue;
			}

			if (!ImportedCurrentFingerprint.Equals(
				SnapshotRecordFingerprint,
				ESearchCase::CaseSensitive))
			{
				AddIssue(
					OutIssues,
					TEXT("CurrentSnapshotChanged"),
					DesiredKeyText,
					TEXT("ImportExisting 이후 BuildCurrentSnapshot 사이에 current Product canonical state가 변경됐습니다. 혼합 시점 Preview를 만들지 않고 fresh preview가 필요합니다."));
				continue;
			}

			// Provider-managed semantic scope의 typed diff입니다.
			FCFContentDiff ProviderDiff;
			// Provider diff failure 상세입니다.
			FString DiffError;
			if (!Provider->BuildDiff(
				CurrentSnapshot,
				DesiredRecord,
				ProviderDiff,
				DiffError))
			{
				AddIssue(
					OutIssues,
					TEXT("ProviderDiffFailed"),
					DesiredKeyText,
					DiffError.IsEmpty()
						? TEXT("Provider BuildDiff가 실패했습니다.")
						: DiffError);
				continue;
			}

			if (!(ProviderDiff.Key == DesiredRecord.Key))
			{
				AddIssue(
					OutIssues,
					TEXT("ProviderDiffIdentityMismatch"),
					DesiredKeyText,
					TEXT("Provider diff의 Key가 requested ContentKey와 일치하지 않습니다."));
				continue;
			}

			// ProviderDiff가 직접 준 current fingerprint가 canonical한지 여부입니다.
			const bool bDiffCurrentFingerprintValid =
				CFDACommonPrimitives::IsCanonicalSha256Fingerprint(ProviderDiff.CurrentFingerprint);
			// Snapshot readback fallback fingerprint가 canonical한지 여부입니다.
			const bool bSnapshotFingerprintValid =
				CFDACommonPrimitives::IsCanonicalSha256Fingerprint(CurrentSnapshot.ReadbackFingerprint);
			// Desired provider fingerprint가 canonical한지 여부입니다.
			const bool bDesiredFingerprintValid =
				CFDACommonPrimitives::IsCanonicalSha256Fingerprint(ProviderDiff.DesiredFingerprint);

			if ((!bDiffCurrentFingerprintValid && !bSnapshotFingerprintValid)
				|| !bDesiredFingerprintValid)
			{
				AddIssue(
					OutIssues,
					TEXT("InvalidProviderFingerprint"),
					DesiredKeyText,
					TEXT("Provider BuildDiff는 canonical SHA-256 current/desired semantic fingerprint를 제공해야 합니다. 비교 증거가 없으면 Unchanged로 축소하지 않습니다."));
				continue;
			}

			if (bDiffCurrentFingerprintValid
				&& bSnapshotFingerprintValid
				&& !ProviderDiff.CurrentFingerprint.Equals(
					CurrentSnapshot.ReadbackFingerprint,
					ESearchCase::CaseSensitive))
			{
				AddIssue(
					OutIssues,
					TEXT("ProviderCurrentFingerprintMismatch"),
					DesiredKeyText,
					TEXT("Provider BuildDiff current fingerprint와 BuildCurrentSnapshot readback fingerprint가 서로 다릅니다. 동일 current snapshot에 대한 모순된 증거를 허용하지 않습니다."));
				continue;
			}

			CompileDiff.CurrentProviderFingerprint =
				bDiffCurrentFingerprintValid
					? ProviderDiff.CurrentFingerprint
					: CurrentSnapshot.ReadbackFingerprint;
			CompileDiff.DesiredProviderFingerprint = ProviderDiff.DesiredFingerprint;
			CompileDiff.ChangedPaths = ProviderDiff.ChangedPaths;

			// Provider fingerprints가 차이나는지 확인합니다.
			const bool bProviderFingerprintChanged =
				!CompileDiff.CurrentProviderFingerprint.Equals(
					CompileDiff.DesiredProviderFingerprint,
					ESearchCase::CaseSensitive);
			// Provider changed paths 또는 semantic fingerprint mismatch가 있는지 확인합니다.
			const bool bChanged = CompileDiff.ChangedPaths.Num() > 0 || bProviderFingerprintChanged;
			if (bProviderFingerprintChanged && CompileDiff.ChangedPaths.IsEmpty())
			{
				CompileDiff.ChangedPaths.Add(TEXT("$providerFingerprint"));
			}

			CompileDiff.Kind = bChanged
				? ECFContentCompileDiffKind::Modified
				: ECFContentCompileDiffKind::Unchanged;

			GeneratedFingerprint.CurrentProviderFingerprint = CompileDiff.CurrentProviderFingerprint;
			GeneratedFingerprint.DesiredProviderFingerprint = CompileDiff.DesiredProviderFingerprint;
			GeneratedFingerprint.ChangedPaths = CompileDiff.ChangedPaths;
			GeneratedFingerprint.DriftState =
				DesiredRecord.ManagementState == ECFContentManagementState::ExternalReadOnly
					? ECFContentDriftState::ExternalReadOnly
					: (bChanged ? ECFContentDriftState::Changed : ECFContentDriftState::InSync);

			OutDiffs.Add(MoveTemp(CompileDiff));
			OutGeneratedFingerprints.Add(MoveTemp(GeneratedFingerprint));
		}

		// Stable order로 처리할 removed current key 목록입니다.
		TArray<FString> RemovedCurrentKeys = RemainingCurrentKeys.Array();
		RemovedCurrentKeys.Sort();

		for (const FString& RemovedKeyText : RemovedCurrentKeys)
		{
			// Desired Workbook에서 사라진 current record입니다.
			const FCFContentRecord* CurrentRecord = CurrentRecordsByKey.Find(RemovedKeyText);
			if (CurrentRecord == nullptr)
			{
				continue;
			}

			// Removed current record exact provider입니다.
			ICFContentProvider* Provider = ProviderRegistry.FindProvider(CurrentRecord->Key.ContentTypeId);
			if (Provider == nullptr)
			{
				AddIssue(
					OutIssues,
					TEXT("MissingProvider"),
					RemovedKeyText,
					TEXT("Removed current record의 Provider를 찾을 수 없습니다."));
				continue;
			}

			// Removed current record readback fingerprint입니다.
			FString CurrentFingerprint;
			// Readback failure 상세입니다.
			FString ReadbackError;
			if (!Provider->ReadbackFingerprint(
				CurrentRecord->Key,
				CurrentFingerprint,
				ReadbackError))
			{
				AddIssue(
					OutIssues,
					TEXT("ReadbackFingerprintFailed"),
					RemovedKeyText,
					ReadbackError.IsEmpty()
						? TEXT("Removed current record의 readback fingerprint를 읽지 못했습니다.")
						: ReadbackError);
				continue;
			}
			if (!CFDACommonPrimitives::IsCanonicalSha256Fingerprint(CurrentFingerprint))
			{
				AddIssue(
					OutIssues,
					TEXT("InvalidReadbackFingerprint"),
					RemovedKeyText,
					TEXT("Removed current record의 ReadbackFingerprint가 canonical SHA-256이 아닙니다."));
				continue;
			}

			// Removed record compiler diff입니다.
			FCFContentCompileDiff CompileDiff;
			CompileDiff.Key = CurrentRecord->Key;
			CompileDiff.Kind = ECFContentCompileDiffKind::Removed;
			CompileDiff.ManagementState = CurrentRecord->ManagementState;
			CompileDiff.CurrentProviderFingerprint = CurrentFingerprint;
			CompileDiff.ChangedPaths.Add(TEXT("$record"));

			// Removed record generated drift/provenance입니다.
			FCFContentGeneratedFingerprint GeneratedFingerprint;
			GeneratedFingerprint.Key = CurrentRecord->Key;
			GeneratedFingerprint.SourceWorkbookSemanticHash = WorkbookSemanticHash;
			GeneratedFingerprint.CurrentProviderFingerprint = CurrentFingerprint;
			GeneratedFingerprint.ChangedPaths = CompileDiff.ChangedPaths;
			GeneratedFingerprint.DriftState =
				CurrentRecord->ManagementState == ECFContentManagementState::ExternalReadOnly
					? ECFContentDriftState::ExternalReadOnly
					: ECFContentDriftState::RetiredCurrent;

			OutDiffs.Add(MoveTemp(CompileDiff));
			OutGeneratedFingerprints.Add(MoveTemp(GeneratedFingerprint));
		}

		OutDiffs.Sort([](const FCFContentCompileDiff& Left, const FCFContentCompileDiff& Right)
		{
			return Left.Key.ToStableString() < Right.Key.ToStableString();
		});
		OutGeneratedFingerprints.Sort(
			[](const FCFContentGeneratedFingerprint& Left, const FCFContentGeneratedFingerprint& Right)
			{
				return Left.Key.ToStableString() < Right.Key.ToStableString();
			});

		if (HasBlockingIssues(OutIssues))
		{
			OutError = TEXT("Provider diff/fingerprint 단계가 fail-closed 됐습니다.");
			return false;
		}
		return true;
	}

	/** Changed root별 direct/transitive referencer impact를 desired + current reverse adjacency union으로 계산합니다. */
	void BuildImpactAnalysis(
		const TArray<FCFContentDependencyEdge>& DesiredDependencyEdges,
		const TArray<FCFContentDependencyEdge>& CurrentDependencyEdges,
		const TArray<FCFContentCompileDiff>& Diffs,
		TArray<FCFContentImpactEntry>& OutImpacts)
	{
		OutImpacts.Reset();

		// Target stable key -> unique direct referencer stable keys reverse adjacency입니다.
		TMap<FString, TSet<FString>> ReverseAdjacency;
		// Stable key -> typed ContentKey lookup입니다.
		TMap<FString, FCFContentKey> KeysByStableId;

		// Desired/Current edge를 동일 안전 그래프에 추가하는 helper입니다.
		auto AddEdgesToReverseAdjacency =
			[&ReverseAdjacency, &KeysByStableId](const TArray<FCFContentDependencyEdge>& Edges)
			{
				for (const FCFContentDependencyEdge& Edge : Edges)
				{
					// Edge source stable key입니다.
					const FString FromKeyText = Edge.From.ToStableString();
					// Edge target stable key입니다.
					const FString ToKeyText = Edge.To.ToStableString();
					ReverseAdjacency.FindOrAdd(ToKeyText).Add(FromKeyText);
					KeysByStableId.FindOrAdd(FromKeyText) = Edge.From;
					KeysByStableId.FindOrAdd(ToKeyText) = Edge.To;
				}
			};

		AddEdgesToReverseAdjacency(DesiredDependencyEdges);
		AddEdgesToReverseAdjacency(CurrentDependencyEdges);

		for (const FCFContentCompileDiff& Diff : Diffs)
		{
			if (Diff.Kind == ECFContentCompileDiffKind::Unchanged)
			{
				continue;
			}

			// Changed root stable key입니다.
			const FString RootKeyText = Diff.Key.ToStableString();
			// 이 root의 impact result입니다.
			FCFContentImpactEntry Impact;
			Impact.ChangedKey = Diff.Key;

			// Direct dependent stable keys입니다.
			TArray<FString> DirectKeyTexts;
			if (const TSet<FString>* DirectSet = ReverseAdjacency.Find(RootKeyText))
			{
				DirectKeyTexts = DirectSet->Array();
				DirectKeyTexts.Sort();
			}

			for (const FString& DirectKeyText : DirectKeyTexts)
			{
				if (const FCFContentKey* DirectKey = KeysByStableId.Find(DirectKeyText))
				{
					Impact.DirectDependents.Add(*DirectKey);
				}
			}

			// BFS에서 이미 방문한 referencer stable keys입니다.
			TSet<FString> Visited;
			// Transitive reverse graph 탐색 queue입니다.
			TQueue<FString> Pending;
			for (const FString& DirectKeyText : DirectKeyTexts)
			{
				if (!DirectKeyText.Equals(RootKeyText, ESearchCase::CaseSensitive)
					&& !Visited.Contains(DirectKeyText))
				{
					Visited.Add(DirectKeyText);
					Pending.Enqueue(DirectKeyText);
				}
			}

			// BFS에서 현재 처리할 referencer stable key입니다.
			FString CurrentKeyText;
			while (Pending.Dequeue(CurrentKeyText))
			{
				if (const TSet<FString>* NextSet = ReverseAdjacency.Find(CurrentKeyText))
				{
					for (const FString& NextKeyText : *NextSet)
					{
						if (NextKeyText.Equals(RootKeyText, ESearchCase::CaseSensitive)
							|| Visited.Contains(NextKeyText))
						{
							continue;
						}
						Visited.Add(NextKeyText);
						Pending.Enqueue(NextKeyText);
					}
				}
			}

			// Stable order로 변환할 transitive referencer keys입니다.
			TArray<FString> TransitiveKeyTexts = Visited.Array();
			TransitiveKeyTexts.Sort();
			for (const FString& TransitiveKeyText : TransitiveKeyTexts)
			{
				if (const FCFContentKey* TransitiveKey = KeysByStableId.Find(TransitiveKeyText))
				{
					Impact.TransitiveDependents.Add(*TransitiveKey);
				}
			}

			SortContentKeys(Impact.DirectDependents);
			SortContentKeys(Impact.TransitiveDependents);
			OutImpacts.Add(MoveTemp(Impact));
		}

		OutImpacts.Sort([](const FCFContentImpactEntry& Left, const FCFContentImpactEntry& Right)
		{
			return Left.ChangedKey.ToStableString() < Right.ChangedKey.ToStableString();
		});
	}

	/** Diff 집합을 Product mutation 없는 preview batch result로 변환합니다. */
	void BuildBatchResult(
		const FCFContentWorkbookModel& Workbook,
		const int32 CurrentRecordCount,
		const TArray<FCFContentCompileDiff>& Diffs,
		FCFContentBatchResult& OutBatchResult)
	{
		OutBatchResult = FCFContentBatchResult();
		OutBatchResult.DesiredRecordCount = Workbook.Records.Num();
		OutBatchResult.CurrentRecordCount = CurrentRecordCount;

		for (const FCFContentCompileDiff& Diff : Diffs)
		{
			// Per-record preview batch result입니다.
			FCFContentBatchItemResult Item;
			Item.Key = Diff.Key;

			if (Diff.ManagementState == ECFContentManagementState::ExternalReadOnly)
			{
				Item.State = ECFContentBatchItemState::ExternalReadOnly;
				Item.Summary = TEXT("ExternalReadOnly — CCAS Product mutation 대상이 아닙니다.");
				++OutBatchResult.ExternalReadOnlyCount;
			}
			else
			{
				switch (Diff.Kind)
				{
				case ECFContentCompileDiffKind::Added:
					Item.State = ECFContentBatchItemState::ReadyToReview;
					Item.Summary = TEXT("Added — review 후 future typed Apply 후보입니다.");
					++OutBatchResult.ReadyToReviewCount;
					break;
				case ECFContentCompileDiffKind::Removed:
					Item.State = ECFContentBatchItemState::RetireCandidate;
					Item.Summary = TEXT("Removed — retire intent review가 필요합니다.");
					++OutBatchResult.RetireCandidateCount;
					break;
				case ECFContentCompileDiffKind::Modified:
					Item.State = ECFContentBatchItemState::ReadyToReview;
					Item.Summary = TEXT("Modified — provider typed diff review가 필요합니다.");
					++OutBatchResult.ReadyToReviewCount;
					break;
				case ECFContentCompileDiffKind::Unchanged:
				default:
					Item.State = ECFContentBatchItemState::AlreadyCurrent;
					Item.Summary = TEXT("AlreadyCurrent — persistent mutation이 필요하지 않습니다.");
					++OutBatchResult.AlreadyCurrentCount;
					break;
				}
			}

			OutBatchResult.Items.Add(MoveTemp(Item));
		}

		OutBatchResult.Items.Sort(
			[](const FCFContentBatchItemResult& Left, const FCFContentBatchItemResult& Right)
			{
				return Left.Key.ToStableString() < Right.Key.ToStableString();
			});
	}
}

bool FCFContentCompiler::CompilePreview(
	const FString& WorkbookPath,
	ICFContentWorkbookAdapter& WorkbookAdapter,
	const FCFContentProviderRegistry& ProviderRegistry,
	FCFContentCompileResult& OutResult)
{
	OutResult = FCFContentCompileResult();

	OutResult.AdapterInfo = WorkbookAdapter.DescribeAdapter();
	if (!CFIsStableContentId(OutResult.AdapterInfo.AdapterId)
		|| OutResult.AdapterInfo.AdapterVersion.IsEmpty())
	{
		CFContentCompilerPrivate::AddIssue(
			OutResult.Issues,
			TEXT("InvalidWorkbookAdapter"),
			TEXT("WorkbookAdapter"),
			TEXT("Workbook adapter는 stable AdapterId와 non-empty version을 제공해야 합니다."));
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("WorkbookRead"),
			TEXT("Workbook adapter identity validation에 실패했습니다."));
	}

	if (WorkbookPath.IsEmpty())
	{
		CFContentCompilerPrivate::AddIssue(
			OutResult.Issues,
			TEXT("EmptyWorkbookPath"),
			TEXT("WorkbookPath"),
			TEXT("Generic Compiler preview에는 explicit Workbook path가 필요합니다."));
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("WorkbookRead"),
			TEXT("Workbook path가 비어 있습니다."));
	}

	// Adapter read failure 상세입니다.
	FString ReadError;
	if (!WorkbookAdapter.ReadWorkbook(
		WorkbookPath,
		OutResult.Workbook,
		ReadError))
	{
		CFContentCompilerPrivate::AddIssue(
			OutResult.Issues,
			TEXT("WorkbookReadFailed"),
			WorkbookPath,
			ReadError.IsEmpty()
				? TEXT("Workbook adapter ReadWorkbook이 실패했습니다.")
				: ReadError);
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("WorkbookRead"),
			ReadError.IsEmpty()
				? TEXT("Workbook read에 실패했습니다.")
				: ReadError);
	}

	// Registry structural validation failure 상세입니다.
	FString RegistryError;
	if (!ProviderRegistry.ValidateRegistry(RegistryError))
	{
		CFContentCompilerPrivate::AddIssue(
			OutResult.Issues,
			TEXT("InvalidProviderRegistry"),
			TEXT("ProviderRegistry"),
			RegistryError.IsEmpty()
				? TEXT("Provider Registry validation이 실패했습니다.")
				: RegistryError);
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("RegistryValidation"),
			RegistryError.IsEmpty()
				? TEXT("Provider Registry validation이 실패했습니다.")
				: RegistryError);
	}

	// P0-01 common Workbook validation diagnostics입니다.
	TArray<FCFContentValidationIssue> CommonIssues;
	if (!FCFContentSchemaValidator::ValidateWorkbook(
		OutResult.Workbook,
		CommonIssues))
	{
		CFContentCompilerPrivate::AppendIssues(OutResult.Issues, CommonIssues);
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("SchemaValidation"),
			TEXT("Canonical Workbook common validation이 fail-closed 됐습니다."));
	}
	CFContentCompilerPrivate::AppendIssues(OutResult.Issues, CommonIssues);

	// Workbook와 registered Provider exact schema compatibility failure 상세입니다.
	FString SchemaCompatibilityError;
	if (!CFContentCompilerPrivate::ValidateProviderSchemaCompatibility(
		OutResult.Workbook,
		ProviderRegistry,
		OutResult.Issues,
		SchemaCompatibilityError))
	{
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("ProviderSchemaCompatibility"),
			SchemaCompatibilityError);
	}

	// Authoritative source Workbook semantic hash build failure 상세입니다.
	FString WorkbookHashError;
	if (!FCFContentSemanticHasher::BuildWorkbookSemanticHash(
		OutResult.Workbook,
		OutResult.WorkbookSemanticHash,
		WorkbookHashError))
	{
		CFContentCompilerPrivate::AddIssue(
			OutResult.Issues,
			TEXT("WorkbookSemanticHashFailed"),
			OutResult.Workbook.WorkbookSourceId,
			WorkbookHashError.IsEmpty()
				? TEXT("WorkbookSemanticHash 생성에 실패했습니다.")
				: WorkbookHashError);
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("SemanticHash"),
			WorkbookHashError.IsEmpty()
				? TEXT("WorkbookSemanticHash 생성에 실패했습니다.")
				: WorkbookHashError);
	}

	// Provider import 결과를 ContentKey로 인덱스한 current Product records입니다.
	TMap<FString, FCFContentRecord> CurrentRecordsByKey;
	// Current import failure 상세입니다.
	FString ImportError;
	if (!CFContentCompilerPrivate::BuildCurrentRecordIndex(
		OutResult.Workbook,
		ProviderRegistry,
		CurrentRecordsByKey,
		OutResult.Issues,
		ImportError))
	{
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("CurrentImport"),
			ImportError);
	}

	// Preview 시작 시점의 whole-current canonical semantic hash입니다.
	FString InitialCurrentImportSemanticHash;
	// Initial current import hash failure 상세입니다.
	FString InitialCurrentImportHashError;
	if (!CFContentCompilerPrivate::BuildCurrentImportSemanticHash(
		OutResult.Workbook,
		CurrentRecordsByKey,
		InitialCurrentImportSemanticHash,
		InitialCurrentImportHashError))
	{
		CFContentCompilerPrivate::AddIssue(
			OutResult.Issues,
			TEXT("CurrentImportFingerprintFailed"),
			TEXT("CurrentProduct"),
			InitialCurrentImportHashError.IsEmpty()
				? TEXT("Current Product import semantic hash 생성에 실패했습니다.")
				: InitialCurrentImportHashError);
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("CurrentImport"),
			TEXT("Current Product import semantic hash 생성에 실패했습니다."));
	}

	// Imported current Product reference graph build/validation failure 상세입니다.
	FString CurrentDependencyError;
	if (!CFContentCompilerPrivate::BuildCurrentDependencyEdges(
		OutResult.Workbook,
		ProviderRegistry,
		CurrentRecordsByKey,
		OutResult.CurrentDependencyEdges,
		OutResult.Issues,
		CurrentDependencyError))
	{
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("CurrentReferenceValidation"),
			CurrentDependencyError);
	}

	// Provider semantic/dependency validation failure 상세입니다.
	FString ProviderValidationError;
	if (!CFContentCompilerPrivate::ValidateDesiredRecords(
		OutResult.Workbook,
		ProviderRegistry,
		OutResult.DependencyEdges,
		OutResult.Issues,
		ProviderValidationError))
	{
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("ProviderValidation"),
			ProviderValidationError);
	}

	// Common dependency graph validation diagnostics입니다.
	TArray<FCFContentValidationIssue> DependencyIssues;
	if (!FCFContentSchemaValidator::ValidateDependencyEdges(
		OutResult.Workbook,
		OutResult.DependencyEdges,
		DependencyIssues))
	{
		CFContentCompilerPrivate::AppendIssues(OutResult.Issues, DependencyIssues);
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("ReferenceValidation"),
			TEXT("Reference graph missing-target/cycle validation이 fail-closed 됐습니다."));
	}
	CFContentCompilerPrivate::AppendIssues(OutResult.Issues, DependencyIssues);

	// Provider typed diff/fingerprint failure 상세입니다.
	FString DiffError;
	if (!CFContentCompilerPrivate::BuildDiffsAndFingerprints(
		OutResult.Workbook,
		OutResult.WorkbookSemanticHash,
		ProviderRegistry,
		CurrentRecordsByKey,
		OutResult.Diffs,
		OutResult.GeneratedFingerprints,
		OutResult.Issues,
		DiffError))
	{
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("Diff"),
			DiffError);
	}

	// Preview 종료 직전 current Product 전체를 다시 import한 records입니다.
	TMap<FString, FCFContentRecord> FinalCurrentRecordsByKey;
	// Final current import failure 상세입니다.
	FString FinalCurrentImportError;
	if (!CFContentCompilerPrivate::BuildCurrentRecordIndex(
		OutResult.Workbook,
		ProviderRegistry,
		FinalCurrentRecordsByKey,
		OutResult.Issues,
		FinalCurrentImportError))
	{
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("CurrentSnapshotConsistency"),
			FinalCurrentImportError);
	}

	// Preview 종료 시점 whole-current canonical semantic hash입니다.
	FString FinalCurrentImportSemanticHash;
	// Final current import hash failure 상세입니다.
	FString FinalCurrentImportHashError;
	if (!CFContentCompilerPrivate::BuildCurrentImportSemanticHash(
		OutResult.Workbook,
		FinalCurrentRecordsByKey,
		FinalCurrentImportSemanticHash,
		FinalCurrentImportHashError))
	{
		CFContentCompilerPrivate::AddIssue(
			OutResult.Issues,
			TEXT("CurrentImportFingerprintFailed"),
			TEXT("CurrentProduct"),
			FinalCurrentImportHashError.IsEmpty()
				? TEXT("Final current Product import semantic hash 생성에 실패했습니다.")
				: FinalCurrentImportHashError);
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("CurrentSnapshotConsistency"),
			TEXT("Final current Product import semantic hash 생성에 실패했습니다."));
	}

	if (!InitialCurrentImportSemanticHash.Equals(
		FinalCurrentImportSemanticHash,
		ESearchCase::CaseSensitive))
	{
		CFContentCompilerPrivate::AddIssue(
			OutResult.Issues,
			TEXT("CurrentImportChanged"),
			TEXT("CurrentProduct"),
			TEXT("Generic Compiler preview 도중 current Product 전체 canonical state가 변경됐습니다. 혼합 시점 결과를 폐기하고 fresh preview가 필요합니다."));
		return CFContentCompilerPrivate::Fail(
			OutResult,
			TEXT("CurrentSnapshotConsistency"),
			TEXT("Current Product가 preview 도중 변경됐습니다."));
	}

	OutResult.CurrentProductSemanticHash = InitialCurrentImportSemanticHash;

	CFContentCompilerPrivate::BuildImpactAnalysis(
		OutResult.DependencyEdges,
		OutResult.CurrentDependencyEdges,
		OutResult.Diffs,
		OutResult.Impacts);

	CFContentCompilerPrivate::BuildBatchResult(
		OutResult.Workbook,
		CurrentRecordsByKey.Num(),
		OutResult.Diffs,
		OutResult.BatchResult);

	OutResult.bSucceeded = true;
	OutResult.FailureStage.Reset();
	OutResult.Error.Reset();
	return true;
}
