// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCutover.cpp
// Version: v1.1.0
// Date: 2026-10-03
// Description: CF-FQ-058 CCAS-P0-07 cutover manifest, stale/drift guard,
// durable transaction/provenance와 fail-closed sequencing 구현입니다.
// Changelog:
// - v1.1.0: Mid-review correction. Manifest fresh rehash, fixed execution paths, atomic-style durable writes,
//   pre-promotion freshness/lock guard와 RecoveryRequired exact-candidate finalize 경로를 추가.
// - v1.0.0: Prepared -> ProductApplied -> WorkbookCommitted -> Verified와
//   BlockedBeforeMutation/RecoveryRequired, exact approval binding, no-auto-delete를 최초 구현.
// Migration:
// - Existing P0-01~06 Compiler/Planning/Provider 의미는 변경하지 않습니다.
// - 실제 Workbook Current authority activation은 이 파일에서 수행하지 않습니다.

#include "DataAuthoring/CFContentCutover.h"

#include "DataAuthoring/CFContentCore.h"
#include "DataAuthoring/CFDACommonPrimitives.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace CFContentCutoverPrivate
{
	/** P0-07 production canonical Workbook exact path입니다. */
	const TCHAR* CanonicalWorkbookPath = TEXT("Authoring/Content/CarFight_Content.xlsx");

	/** P0-07 production sibling staged Workbook exact path입니다. */
	const TCHAR* StagedWorkbookPath = TEXT("Authoring/Content/CarFight_Content.staged.xlsx");

	/** P0-07 production provenance exact path입니다. */
	const TCHAR* ProvenancePath = TEXT("Authoring/Content/CarFight_Content.cfsnapshot.json");

	/** Blocking validation issue를 append합니다. */
	void AddIssue(
		TArray<FCFContentValidationIssue>& OutIssues,
		const FString& Code,
		const FString& Path,
		const FString& Message)
	{
		// Append할 blocking issue입니다.
		FCFContentValidationIssue Issue;
		Issue.Code = Code;
		Issue.Path = Path;
		Issue.Message = Message;
		Issue.bBlocking = true;
		OutIssues.Add(MoveTemp(Issue));
	}

	/** Blocking issue가 하나라도 존재하는지 확인합니다. */
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

	/** Stable string token을 deterministic hash payload에 추가합니다. */
	void AppendString(
		TArray<uint8>& OutBytes,
		const TCHAR* Label,
		const FString& Value)
	{
		CFDACommonPrimitives::AppendStringToken(OutBytes, Label, Value);
	}

	/** Stable integer token을 deterministic hash payload에 추가합니다. */
	void AppendInt(
		TArray<uint8>& OutBytes,
		const TCHAR* Label,
		const int64 Value)
	{
		CFDACommonPrimitives::AppendStringToken(
			OutBytes,
			Label,
			LexToString(Value));
	}

	/** ContentKey를 deterministic hash payload에 추가합니다. */
	void AppendContentKey(
		TArray<uint8>& OutBytes,
		const TCHAR* Label,
		const FCFContentKey& ContentKey)
	{
		AppendString(OutBytes, Label, ContentKey.ToStableString());
	}

	/** Slash가 없는 local durable ID인지 확인합니다. */
	bool IsSafeLocalId(const FString& Value)
	{
		return CFIsStableContentId(Value)
			&& !Value.Contains(TEXT("/"), ESearchCase::CaseSensitive)
			&& !Value.Contains(TEXT("\\"), ESearchCase::CaseSensitive);
	}

	/** P0-07 production execution path가 frozen exact3 계약과 일치하는지 검증합니다. */
	bool ValidateExecutionPaths(
		const FCFContentCutoverRequest& Request,
		FString& OutError)
	{
		OutError.Reset();
		if (!Request.CanonicalWorkbookPath.Equals(CanonicalWorkbookPath, ESearchCase::CaseSensitive)
			|| !Request.StagedWorkbookPath.Equals(StagedWorkbookPath, ESearchCase::CaseSensitive)
			|| !Request.ProvenancePath.Equals(ProvenancePath, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("P0-07 execution path는 frozen canonical/staged/provenance exact3 경로만 허용합니다.");
			return false;
		}
		return true;
	}

	/** Canonical Workbook의 Office-style sibling lock file 존재 여부를 확인합니다. */
	bool HasExternalWorkbookLock(const FString& CanonicalPhysicalPath)
	{
		// Canonical Workbook parent directory입니다.
		const FString ParentDirectory = FPaths::GetPath(CanonicalPhysicalPath);
		// Canonical Workbook clean filename입니다.
		const FString WorkbookFilename = FPaths::GetCleanFilename(CanonicalPhysicalPath);
		// Office-style sibling lock path입니다.
		const FString LockPath = FPaths::Combine(
			ParentDirectory,
			TEXT("~$") + WorkbookFilename);
		return IFileManager::Get().FileExists(*LockPath);
	}

	/** Text payload를 sibling temp에 write/readback한 뒤 target으로 교체합니다. */
	bool SaveStringAtomically(
		const FString& TargetPath,
		const FString& TextPayload,
		FString& OutError)
	{
		OutError.Reset();
		// Controlled sibling write path입니다.
		const FString TempPath = TargetPath + TEXT(".cutover-writing");

		if (IFileManager::Get().FileExists(*TempPath)
			&& !IFileManager::Get().Delete(*TempPath, false, true, true))
		{
			OutError = TEXT("이전 cutover temp 파일을 정리하지 못했습니다: ") + TempPath;
			return false;
		}

		if (!FFileHelper::SaveStringToFile(
			TextPayload,
			*TempPath,
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
		{
			OutError = TEXT("Cutover temp durable write가 실패했습니다: ") + TempPath;
			return false;
		}

		// Temp payload readback 검증입니다.
		FString TempReadback;
		if (!FFileHelper::LoadFileToString(TempReadback, *TempPath)
			|| !TempReadback.Equals(TextPayload, ESearchCase::CaseSensitive))
		{
			IFileManager::Get().Delete(*TempPath, false, true, true);
			OutError = TEXT("Cutover temp durable readback이 write payload와 다릅니다.");
			return false;
		}

		if (!IFileManager::Get().Move(
			*TargetPath,
			*TempPath,
			true,
			true,
			false,
			true))
		{
			IFileManager::Get().Delete(*TempPath, false, true, true);
			OutError = TEXT("Cutover durable target replace가 실패했습니다: ") + TargetPath;
			return false;
		}

		// Final target exact readback입니다.
		FString FinalReadback;
		if (!FFileHelper::LoadFileToString(FinalReadback, *TargetPath)
			|| !FinalReadback.Equals(TextPayload, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Cutover durable target readback이 write payload와 다릅니다.");
			return false;
		}
		return true;
	}

	/** Product absence를 exact current state로 표현하는 canonical fingerprint를 생성합니다. */
	bool BuildAbsentProductFingerprint(
		const FCFContentKey& ContentKey,
		FString& OutFingerprint,
		FString& OutError)
	{
		// Absence semantic canonical bytes입니다.
		TArray<uint8> CanonicalBytes;
		AppendString(CanonicalBytes, TEXT("Protocol"), TEXT("CarFight.CCAS.ProductAbsence/v1"));
		AppendContentKey(CanonicalBytes, TEXT("ContentKey"), ContentKey);
		return CFDACommonPrimitives::HashCanonicalBytes(
			CanonicalBytes,
			OutFingerprint,
			OutError);
	}

	/** Product adapter current state를 canonical pre-apply fingerprint로 정규화합니다. */
	bool ReadCanonicalProductState(
		ICFContentCutoverProductAdapter& Adapter,
		const FCFContentKey& ContentKey,
		FCFContentProductState& OutState,
		FString& OutError)
	{
		OutState = FCFContentProductState();
		OutError.Reset();

		if (!Adapter.ReadCurrentProductState(ContentKey, OutState, OutError))
		{
			if (OutError.IsEmpty())
			{
				OutError = TEXT("Product adapter current state read가 실패했습니다.");
			}
			return false;
		}

		if (!OutState.bExists)
		{
			return BuildAbsentProductFingerprint(
				ContentKey,
				OutState.ProductFingerprint,
				OutError);
		}

		if (!CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			OutState.ProductFingerprint))
		{
			OutError = TEXT("Existing Product current fingerprint가 canonical SHA-256이 아닙니다.");
			return false;
		}

		return true;
	}

	/** Review preview에서 exact canonical record를 찾습니다. */
	const FCFContentRecord* FindRecord(
		const FCFContentWorkbookModel& Workbook,
		const FCFContentKey& ContentKey)
	{
		return Workbook.Records.FindByPredicate(
			[&ContentKey](const FCFContentRecord& Record)
			{
				return Record.Key == ContentKey;
			});
	}

	/** Compile result에서 exact generated fingerprint를 찾습니다. */
	const FCFContentGeneratedFingerprint* FindGeneratedFingerprint(
		const FCFContentCompileResult& CompileResult,
		const FCFContentKey& ContentKey)
	{
		return CompileResult.GeneratedFingerprints.FindByPredicate(
			[&ContentKey](const FCFContentGeneratedFingerprint& Fingerprint)
			{
				return Fingerprint.Key == ContentKey;
			});
	}

	/** Manifest semantic fingerprint를 생성합니다. */
	bool BuildManifestFingerprint(
		const FCFContentCutoverManifest& Manifest,
		FString& OutFingerprint,
		FString& OutError)
	{
		OutFingerprint.Reset();
		OutError.Reset();

		// Manifest canonical byte stream입니다.
		TArray<uint8> CanonicalBytes;
		AppendString(CanonicalBytes, TEXT("Protocol"), TEXT("CarFight.CCAS.CutoverManifest/v1"));
		AppendString(CanonicalBytes, TEXT("ManifestId"), Manifest.ManifestId);

		// Stable ContentKey order의 entry pointers입니다.
		TArray<const FCFContentCutoverManifestEntry*> SortedEntries;
		for (const FCFContentCutoverManifestEntry& Entry : Manifest.Entries)
		{
			SortedEntries.Add(&Entry);
		}
		SortedEntries.Sort(
			[](const FCFContentCutoverManifestEntry& Left, const FCFContentCutoverManifestEntry& Right)
			{
				return Left.Binding.ContentKey.ToStableString()
					< Right.Binding.ContentKey.ToStableString();
			});

		for (const FCFContentCutoverManifestEntry* Entry : SortedEntries)
		{
			if (Entry == nullptr)
			{
				continue;
			}

			AppendContentKey(CanonicalBytes, TEXT("ContentKey"), Entry->Binding.ContentKey);
			AppendString(CanonicalBytes, TEXT("ChangeSetId"), Entry->Binding.ChangeSetId);
			AppendString(CanonicalBytes, TEXT("BaseWorkbookSemanticHash"), Entry->Binding.BaseWorkbookSemanticHash);
			AppendString(CanonicalBytes, TEXT("BaseCatalogSnapshotFingerprint"), Entry->Binding.BaseCatalogSnapshotFingerprint);
			AppendString(CanonicalBytes, TEXT("ReviewPackageFingerprint"), Entry->Binding.ReviewPackageFingerprint);
			AppendString(CanonicalBytes, TEXT("ProviderSchemaFingerprint"), Entry->Binding.ProviderSchemaFingerprint);
			AppendString(CanonicalBytes, TEXT("PreApplyProductFingerprint"), Entry->Binding.PreApplyProductFingerprint);
			AppendString(CanonicalBytes, TEXT("DesiredProductFingerprint"), Entry->Binding.DesiredProductFingerprint);
			AppendString(CanonicalBytes, TEXT("DesiredCanonicalFingerprint"), Entry->Binding.DesiredCanonicalFingerprint);
			AppendInt(CanonicalBytes, TEXT("ManagementState"), static_cast<int64>(Entry->ManagementState));
			AppendInt(CanonicalBytes, TEXT("LifecycleState"), static_cast<int64>(Entry->LifecycleState));
			AppendInt(CanonicalBytes, TEXT("ProductMutationRequired"), Entry->bProductMutationRequired ? 1 : 0);
		}

		return CFDACommonPrimitives::HashCanonicalBytes(
			CanonicalBytes,
			OutFingerprint,
			OutError);
	}

	/** Reviewed record가 Product cutover 가능한 planning state인지 검증합니다. */
	bool ValidateCutoverReadiness(
		const FCFContentRecord& Record,
		TArray<FCFContentValidationIssue>& OutIssues)
	{
		// Record diagnostic path입니다.
		const FString RecordPath = Record.Key.ToStableString();

		if (Record.AuthoringMetadata.Planning.Readiness !=
			ECFContentPlanningReadiness::AuthoringReady)
		{
			AddIssue(
				OutIssues,
				TEXT("CutoverReadinessBlocked"),
				RecordPath + TEXT(".Planning.Readiness"),
				TEXT("Product cutover는 AuthoringReady record만 허용합니다. Planned/Conditional/Blocked는 fail-closed 합니다."));
		}

		for (const FCFContentTechnologyDependency& Dependency :
			Record.AuthoringMetadata.Planning.TechnologyDependencies)
		{
			if (Dependency.State != ECFContentDependencyState::Satisfied)
			{
				AddIssue(
					OutIssues,
					TEXT("CutoverDependencyPending"),
					RecordPath + TEXT(".Planning.TechnologyDependencies.") + Dependency.DependencyId,
					TEXT("Pending/Blocked technology dependency가 있는 record는 Product cutover 대상이 될 수 없습니다."));
			}
		}

		return !HasBlockingIssues(OutIssues);
	}

	/** Repository-relative path를 checkout physical path로 안전하게 resolve합니다. */
	bool ResolveRepositoryRelativePath(
		const FString& RelativePath,
		FString& OutPhysicalPath,
		FString& OutError)
	{
		OutPhysicalPath.Reset();
		OutError.Reset();

		// Normalized caller path입니다.
		FString NormalizedPath = RelativePath;
		FPaths::NormalizeFilename(NormalizedPath);

		if (NormalizedPath.IsEmpty()
			|| !FPaths::IsRelative(NormalizedPath)
			|| NormalizedPath.Contains(TEXT(".."), ESearchCase::CaseSensitive)
			|| NormalizedPath.Contains(TEXT(":"), ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Cutover persistence path는 '..' 또는 absolute drive를 포함하지 않는 repository-relative path여야 합니다.");
			return false;
		}

		// UE Project directory의 parent인 canonical CarFight repository root입니다.
		const FString RepositoryRoot = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectDir(), TEXT("..")));
		OutPhysicalPath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(RepositoryRoot, NormalizedPath));
		return true;
	}

	/** ContentKey를 JSON object로 직렬화합니다. */
	TSharedRef<FJsonObject> ContentKeyToJson(const FCFContentKey& ContentKey)
	{
		// JSON object입니다.
		TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		JsonObject->SetStringField(TEXT("ContentTypeId"), ContentKey.ContentTypeId.Value);
		JsonObject->SetStringField(TEXT("ContentId"), ContentKey.ContentId);
		return JsonObject;
	}

	/** JSON object에서 ContentKey를 읽습니다. */
	bool ContentKeyFromJson(
		const TSharedPtr<FJsonObject>& JsonObject,
		FCFContentKey& OutContentKey)
	{
		OutContentKey = FCFContentKey();
		if (!JsonObject.IsValid())
		{
			return false;
		}

		if (!JsonObject->TryGetStringField(TEXT("ContentTypeId"), OutContentKey.ContentTypeId.Value)
			|| !JsonObject->TryGetStringField(TEXT("ContentId"), OutContentKey.ContentId))
		{
			return false;
		}
		return OutContentKey.IsValid();
	}

	/** Transaction target report를 JSON object로 직렬화합니다. */
	TSharedRef<FJsonObject> TargetReportToJson(
		const FCFContentCutoverTargetReport& Target)
	{
		// Target JSON object입니다.
		TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		JsonObject->SetObjectField(TEXT("ContentKey"), ContentKeyToJson(Target.ContentKey));
		JsonObject->SetNumberField(TEXT("State"), static_cast<int32>(Target.State));
		JsonObject->SetStringField(TEXT("FreshPreApplyProductFingerprint"), Target.FreshPreApplyProductFingerprint);
		JsonObject->SetStringField(TEXT("DesiredProductFingerprint"), Target.DesiredProductFingerprint);
		JsonObject->SetStringField(TEXT("PostApplyReadbackFingerprint"), Target.PostApplyReadbackFingerprint);
		JsonObject->SetBoolField(TEXT("ProductMutationPerformed"), Target.bProductMutationPerformed);
		JsonObject->SetStringField(TEXT("Diagnostic"), Target.Diagnostic);
		return JsonObject;
	}

	/** JSON object에서 transaction target report를 읽습니다. */
	bool TargetReportFromJson(
		const TSharedPtr<FJsonObject>& JsonObject,
		FCFContentCutoverTargetReport& OutTarget)
	{
		OutTarget = FCFContentCutoverTargetReport();
		if (!JsonObject.IsValid())
		{
			return false;
		}

		// Serialized ContentKey object입니다.
		const TSharedPtr<FJsonObject>* ContentKeyObject = nullptr;
		if (!JsonObject->TryGetObjectField(TEXT("ContentKey"), ContentKeyObject)
			|| ContentKeyObject == nullptr
			|| !ContentKeyFromJson(*ContentKeyObject, OutTarget.ContentKey))
		{
			return false;
		}

		// Serialized enum number입니다.
		double StateNumber = 0.0;
		if (!JsonObject->TryGetNumberField(TEXT("State"), StateNumber))
		{
			return false;
		}
		if (StateNumber < 0.0
			|| StateNumber > static_cast<double>(ECFContentCutoverTargetState::Failed))
		{
			return false;
		}
		OutTarget.State = static_cast<ECFContentCutoverTargetState>(
			static_cast<uint8>(StateNumber));

		JsonObject->TryGetStringField(TEXT("FreshPreApplyProductFingerprint"), OutTarget.FreshPreApplyProductFingerprint);
		JsonObject->TryGetStringField(TEXT("DesiredProductFingerprint"), OutTarget.DesiredProductFingerprint);
		JsonObject->TryGetStringField(TEXT("PostApplyReadbackFingerprint"), OutTarget.PostApplyReadbackFingerprint);
		JsonObject->TryGetBoolField(TEXT("ProductMutationPerformed"), OutTarget.bProductMutationPerformed);
		JsonObject->TryGetStringField(TEXT("Diagnostic"), OutTarget.Diagnostic);
		return true;
	}

	/** Transaction 전체를 JSON string으로 직렬화합니다. */
	bool SerializeTransaction(
		const FCFContentCutoverTransaction& Transaction,
		FString& OutJson,
		FString& OutError)
	{
		OutJson.Reset();
		OutError.Reset();

		// Root transaction JSON object입니다.
		TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
		RootObject->SetStringField(TEXT("SchemaId"), TEXT("ccas-cutover-transaction/v1"));
		RootObject->SetStringField(TEXT("TransactionId"), Transaction.TransactionId);
		RootObject->SetNumberField(TEXT("State"), static_cast<int32>(Transaction.State));
		RootObject->SetStringField(TEXT("CanonicalWorkbookPath"), Transaction.CanonicalWorkbookPath);
		RootObject->SetStringField(TEXT("StagedWorkbookPath"), Transaction.StagedWorkbookPath);
		RootObject->SetStringField(TEXT("ExpectedWorkbookSemanticHash"), Transaction.ExpectedWorkbookSemanticHash);
		RootObject->SetStringField(TEXT("ManifestFingerprint"), Transaction.ManifestFingerprint);
		RootObject->SetBoolField(TEXT("AuthorityActivationEligible"), Transaction.bAuthorityActivationEligible);
		RootObject->SetBoolField(TEXT("AuthorityActivated"), Transaction.bAuthorityActivated);
		RootObject->SetStringField(TEXT("Diagnostic"), Transaction.Diagnostic);

		// Target JSON array입니다.
		TArray<TSharedPtr<FJsonValue>> TargetValues;
		for (const FCFContentCutoverTargetReport& Target : Transaction.Targets)
		{
			TargetValues.Add(MakeShared<FJsonValueObject>(TargetReportToJson(Target)));
		}
		RootObject->SetArrayField(TEXT("Targets"), TargetValues);

		// Compact JSON writer입니다.
		TSharedRef<TJsonWriter<>> JsonWriter =
			TJsonWriterFactory<>::Create(&OutJson);
		if (!FJsonSerializer::Serialize(RootObject, JsonWriter))
		{
			OutError = TEXT("Cutover transaction JSON serialize에 실패했습니다.");
			return false;
		}
		return true;
	}

	/** JSON string에서 transaction 전체를 역직렬화합니다. */
	bool DeserializeTransaction(
		const FString& JsonText,
		FCFContentCutoverTransaction& OutTransaction,
		FString& OutError)
	{
		OutTransaction = FCFContentCutoverTransaction();
		OutError.Reset();

		// Root JSON object입니다.
		TSharedPtr<FJsonObject> RootObject;
		// JSON reader입니다.
		TSharedRef<TJsonReader<>> JsonReader =
			TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(JsonReader, RootObject)
			|| !RootObject.IsValid())
		{
			OutError = TEXT("Cutover transaction JSON parse에 실패했습니다.");
			return false;
		}

		// Schema identity입니다.
		FString SchemaId;
		if (!RootObject->TryGetStringField(TEXT("SchemaId"), SchemaId)
			|| !SchemaId.Equals(TEXT("ccas-cutover-transaction/v1"), ESearchCase::CaseSensitive)
			|| !RootObject->TryGetStringField(TEXT("TransactionId"), OutTransaction.TransactionId))
		{
			OutError = TEXT("Cutover transaction schema/identity가 유효하지 않습니다.");
			return false;
		}

		// Serialized transaction state입니다.
		double StateNumber = 0.0;
		if (!RootObject->TryGetNumberField(TEXT("State"), StateNumber))
		{
			OutError = TEXT("Cutover transaction state가 없습니다.");
			return false;
		}
		if (StateNumber < 0.0
			|| StateNumber > static_cast<double>(ECFContentCutoverState::RecoveryRequired))
		{
			OutError = TEXT("Cutover transaction state enum range가 유효하지 않습니다.");
			return false;
		}
		OutTransaction.State = static_cast<ECFContentCutoverState>(
			static_cast<uint8>(StateNumber));

		RootObject->TryGetStringField(TEXT("CanonicalWorkbookPath"), OutTransaction.CanonicalWorkbookPath);
		RootObject->TryGetStringField(TEXT("StagedWorkbookPath"), OutTransaction.StagedWorkbookPath);
		RootObject->TryGetStringField(TEXT("ExpectedWorkbookSemanticHash"), OutTransaction.ExpectedWorkbookSemanticHash);
		RootObject->TryGetStringField(TEXT("ManifestFingerprint"), OutTransaction.ManifestFingerprint);
		RootObject->TryGetBoolField(TEXT("AuthorityActivationEligible"), OutTransaction.bAuthorityActivationEligible);
		RootObject->TryGetBoolField(TEXT("AuthorityActivated"), OutTransaction.bAuthorityActivated);
		RootObject->TryGetStringField(TEXT("Diagnostic"), OutTransaction.Diagnostic);

		// Serialized target array입니다.
		const TArray<TSharedPtr<FJsonValue>>* TargetValues = nullptr;
		if (RootObject->TryGetArrayField(TEXT("Targets"), TargetValues)
			&& TargetValues != nullptr)
		{
			for (const TSharedPtr<FJsonValue>& TargetValue : *TargetValues)
			{
				// Serialized target object입니다.
				const TSharedPtr<FJsonObject> TargetObject =
					TargetValue.IsValid() ? TargetValue->AsObject() : nullptr;
				// Parsed target report입니다.
				FCFContentCutoverTargetReport Target;
				if (!TargetReportFromJson(TargetObject, Target))
				{
					OutError = TEXT("Cutover transaction target parse에 실패했습니다.");
					return false;
				}
				OutTransaction.Targets.Add(MoveTemp(Target));
			}
		}

		return true;
	}

	/** Provenance snapshot을 JSON string으로 직렬화합니다. */
	bool SerializeProvenance(
		const FCFContentProvenanceSnapshot& Snapshot,
		FString& OutJson,
		FString& OutError)
	{
		OutJson.Reset();
		OutError.Reset();

		// Root provenance JSON object입니다.
		TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
		RootObject->SetStringField(TEXT("SchemaId"), Snapshot.SchemaId);
		RootObject->SetStringField(TEXT("CutoverTransactionId"), Snapshot.CutoverTransactionId);

		// Stable ContentKey order의 provenance pointers입니다.
		TArray<const FCFContentProvenanceRecord*> SortedRecords;
		for (const FCFContentProvenanceRecord& Record : Snapshot.Records)
		{
			SortedRecords.Add(&Record);
		}
		SortedRecords.Sort(
			[](const FCFContentProvenanceRecord& Left, const FCFContentProvenanceRecord& Right)
			{
				return Left.ContentKey.ToStableString() < Right.ContentKey.ToStableString();
			});

		// Serialized provenance records입니다.
		TArray<TSharedPtr<FJsonValue>> RecordValues;
		for (const FCFContentProvenanceRecord* Record : SortedRecords)
		{
			if (Record == nullptr)
			{
				continue;
			}

			// One provenance record JSON object입니다.
			TSharedRef<FJsonObject> RecordObject = MakeShared<FJsonObject>();
			RecordObject->SetObjectField(TEXT("ContentKey"), ContentKeyToJson(Record->ContentKey));
			RecordObject->SetStringField(TEXT("CutoverTransactionId"), Record->CutoverTransactionId);
			RecordObject->SetStringField(TEXT("SourceWorkbookSemanticHash"), Record->SourceWorkbookSemanticHash);
			RecordObject->SetStringField(TEXT("CatalogSnapshotFingerprint"), Record->CatalogSnapshotFingerprint);
			RecordObject->SetStringField(TEXT("ReviewPackageFingerprint"), Record->ReviewPackageFingerprint);
			RecordObject->SetStringField(TEXT("ProviderSchemaFingerprint"), Record->ProviderSchemaFingerprint);
			RecordObject->SetStringField(TEXT("DesiredCanonicalFingerprint"), Record->DesiredCanonicalFingerprint);
			RecordObject->SetStringField(TEXT("PreApplyProductFingerprint"), Record->PreApplyProductFingerprint);
			RecordObject->SetStringField(TEXT("PostApplyReadbackFingerprint"), Record->PostApplyReadbackFingerprint);
			RecordObject->SetNumberField(TEXT("ManagementState"), static_cast<int32>(Record->ManagementState));
			RecordObject->SetNumberField(TEXT("LifecycleState"), static_cast<int32>(Record->LifecycleState));
			RecordValues.Add(MakeShared<FJsonValueObject>(RecordObject));
		}
		RootObject->SetArrayField(TEXT("Records"), RecordValues);

		// Compact JSON writer입니다.
		TSharedRef<TJsonWriter<>> JsonWriter =
			TJsonWriterFactory<>::Create(&OutJson);
		if (!FJsonSerializer::Serialize(RootObject, JsonWriter))
		{
			OutError = TEXT("Content provenance JSON serialize에 실패했습니다.");
			return false;
		}
		return true;
	}

	/** JSON string에서 provenance snapshot을 역직렬화합니다. */
	bool DeserializeProvenance(
		const FString& JsonText,
		FCFContentProvenanceSnapshot& OutSnapshot,
		FString& OutError)
	{
		OutSnapshot = FCFContentProvenanceSnapshot();
		OutError.Reset();

		// Root provenance JSON object입니다.
		TSharedPtr<FJsonObject> RootObject;
		// JSON reader입니다.
		TSharedRef<TJsonReader<>> JsonReader =
			TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(JsonReader, RootObject)
			|| !RootObject.IsValid())
		{
			OutError = TEXT("Content provenance JSON parse에 실패했습니다.");
			return false;
		}

		if (!RootObject->TryGetStringField(TEXT("SchemaId"), OutSnapshot.SchemaId)
			|| !OutSnapshot.SchemaId.Equals(TEXT("carfight-content-snapshot/v1"), ESearchCase::CaseSensitive)
			|| !RootObject->TryGetStringField(TEXT("CutoverTransactionId"), OutSnapshot.CutoverTransactionId))
		{
			OutError = TEXT("Content provenance schema/transaction identity가 유효하지 않습니다.");
			return false;
		}

		// Serialized record array입니다.
		const TArray<TSharedPtr<FJsonValue>>* RecordValues = nullptr;
		if (!RootObject->TryGetArrayField(TEXT("Records"), RecordValues)
			|| RecordValues == nullptr)
		{
			OutError = TEXT("Content provenance Records array가 없습니다.");
			return false;
		}

		for (const TSharedPtr<FJsonValue>& RecordValue : *RecordValues)
		{
			// Serialized provenance record object입니다.
			const TSharedPtr<FJsonObject> RecordObject =
				RecordValue.IsValid() ? RecordValue->AsObject() : nullptr;
			if (!RecordObject.IsValid())
			{
				OutError = TEXT("Content provenance record object가 유효하지 않습니다.");
				return false;
			}

			// Parsed provenance record입니다.
			FCFContentProvenanceRecord Record;
			// Serialized ContentKey object입니다.
			const TSharedPtr<FJsonObject>* ContentKeyObject = nullptr;
			if (!RecordObject->TryGetObjectField(TEXT("ContentKey"), ContentKeyObject)
				|| ContentKeyObject == nullptr
				|| !ContentKeyFromJson(*ContentKeyObject, Record.ContentKey))
			{
				OutError = TEXT("Content provenance ContentKey parse에 실패했습니다.");
				return false;
			}

			RecordObject->TryGetStringField(TEXT("CutoverTransactionId"), Record.CutoverTransactionId);
			RecordObject->TryGetStringField(TEXT("SourceWorkbookSemanticHash"), Record.SourceWorkbookSemanticHash);
			RecordObject->TryGetStringField(TEXT("CatalogSnapshotFingerprint"), Record.CatalogSnapshotFingerprint);
			RecordObject->TryGetStringField(TEXT("ReviewPackageFingerprint"), Record.ReviewPackageFingerprint);
			RecordObject->TryGetStringField(TEXT("ProviderSchemaFingerprint"), Record.ProviderSchemaFingerprint);
			RecordObject->TryGetStringField(TEXT("DesiredCanonicalFingerprint"), Record.DesiredCanonicalFingerprint);
			RecordObject->TryGetStringField(TEXT("PreApplyProductFingerprint"), Record.PreApplyProductFingerprint);
			RecordObject->TryGetStringField(TEXT("PostApplyReadbackFingerprint"), Record.PostApplyReadbackFingerprint);

			// Serialized management state입니다.
			double ManagementStateNumber = 0.0;
			// Serialized lifecycle state입니다.
			double LifecycleStateNumber = 0.0;
			if (!RecordObject->TryGetNumberField(TEXT("ManagementState"), ManagementStateNumber)
				|| !RecordObject->TryGetNumberField(TEXT("LifecycleState"), LifecycleStateNumber))
			{
				OutError = TEXT("Content provenance management/lifecycle state가 없습니다.");
				return false;
			}
			if (ManagementStateNumber < 0.0
				|| ManagementStateNumber > static_cast<double>(ECFContentManagementState::ExternalReadOnly)
				|| LifecycleStateNumber < 0.0
				|| LifecycleStateNumber > static_cast<double>(ECFContentLifecycleState::Retired))
			{
				OutError = TEXT("Content provenance management/lifecycle enum range가 유효하지 않습니다.");
				return false;
			}
			Record.ManagementState = static_cast<ECFContentManagementState>(
				static_cast<uint8>(ManagementStateNumber));
			Record.LifecycleState = static_cast<ECFContentLifecycleState>(
				static_cast<uint8>(LifecycleStateNumber));
			OutSnapshot.Records.Add(MoveTemp(Record));
		}

		return true;
	}

	/** Transaction을 failure terminal state로 전환하고 durable evidence를 best-effort 저장합니다. */
	bool FailTransaction(
		FCFContentCutoverTransaction& Transaction,
		const ECFContentCutoverState FailureState,
		const FString& Diagnostic,
		ICFContentCutoverStore& Store)
	{
		Transaction.State = FailureState;
		Transaction.Diagnostic = Diagnostic;
		Transaction.bAuthorityActivationEligible = false;
		Transaction.bAuthorityActivated = false;

		// Durable failure evidence 저장 오류입니다.
		FString SaveError;
		if (!Store.SaveTransaction(Transaction, SaveError))
		{
			Transaction.Diagnostic += TEXT(" | CRITICAL: durable failure evidence 저장 실패");
			if (!SaveError.IsEmpty())
			{
				Transaction.Diagnostic += TEXT(": ") + SaveError;
			}
		}
		return false;
	}
}


// Provider와 그 exact ContentTypeId를 non-owning으로 결속합니다.
FCFProviderCutoverAdapter::FCFProviderCutoverAdapter(
	ICFContentProvider& InProvider,
	const FCFContentTypeId& InContentTypeId)
	: Provider(&InProvider)
	, ContentTypeId(InContentTypeId)
{
}

// Provider가 소유하는 exact ContentTypeId를 반환합니다.
FCFContentTypeId FCFProviderCutoverAdapter::DescribeContentType() const
{
	return ContentTypeId;
}

// Existing Product current snapshot과 desired record의 provider-local diff에서 desired fingerprint를 계산합니다.
bool FCFProviderCutoverAdapter::BuildDesiredProductFingerprint(
	const FCFContentRecord& DesiredRecord,
	FString& OutFingerprint,
	FString& OutError) const
{
	OutFingerprint.Reset();
	OutError.Reset();

	if (Provider == nullptr
		|| !(DesiredRecord.Key.ContentTypeId == ContentTypeId))
	{
		OutError = TEXT("Provider Cutover adapter의 provider/content type binding이 유효하지 않습니다.");
		return false;
	}

	// Explicit onboarding은 persisted Product exact1만 허용하므로 current typed snapshot을 먼저 요구합니다.
	FCFContentSnapshot CurrentSnapshot;
	if (!Provider->BuildCurrentSnapshot(
		DesiredRecord.Key,
		CurrentSnapshot,
		OutError))
	{
		if (OutError.IsEmpty())
		{
			OutError = TEXT("P0-07 managed onboarding은 existing persisted Product만 허용합니다. 신규 Product Create는 별도 authoring workflow를 사용해야 합니다.");
		}
		return false;
	}

	// Provider-local current -> desired typed diff입니다.
	FCFContentDiff Diff;
	if (!Provider->BuildDiff(
		CurrentSnapshot,
		DesiredRecord,
		Diff,
		OutError))
	{
		if (OutError.IsEmpty())
		{
			OutError = TEXT("Provider-local desired Product fingerprint 생성용 typed diff가 실패했습니다.");
		}
		return false;
	}

	if (!(Diff.Key == DesiredRecord.Key)
		|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
			Diff.DesiredFingerprint))
	{
		OutError = TEXT("Provider-local desired Product fingerprint/diff identity가 canonical하지 않습니다.");
		return false;
	}

	OutFingerprint = Diff.DesiredFingerprint;
	return true;
}

// Provider import/readback으로 existing Product 또는 explicit absence를 mutation 없이 확인합니다.
bool FCFProviderCutoverAdapter::ReadCurrentProductState(
	const FCFContentKey& ContentKey,
	FCFContentProductState& OutState,
	FString& OutError) const
{
	OutState = FCFContentProductState();
	OutError.Reset();

	if (Provider == nullptr
		|| !(ContentKey.ContentTypeId == ContentTypeId))
	{
		OutError = TEXT("Provider Cutover adapter의 current-state content type binding이 유효하지 않습니다.");
		return false;
	}

	// Provider-owned read-only existing Product projection입니다.
	FCFContentImportResult ImportResult;
	if (!Provider->ImportExisting(ImportResult, OutError))
	{
		if (OutError.IsEmpty())
		{
			OutError = TEXT("Provider existing Product import가 실패했습니다.");
		}
		return false;
	}

	// Exact ContentKey match count입니다.
	int32 MatchCount = 0;
	for (const FCFContentRecord& Record : ImportResult.Records)
	{
		if (Record.Key == ContentKey)
		{
			++MatchCount;
		}
	}

	if (MatchCount > 1)
	{
		OutError = TEXT("Provider existing Product import에 duplicate ContentKey가 있습니다: ")
			+ ContentKey.ToStableString();
		return false;
	}

	if (MatchCount == 0)
	{
		OutState.bExists = false;
		return true;
	}

	OutState.bExists = true;
	if (!Provider->ReadbackFingerprint(
		ContentKey,
		OutState.ProductFingerprint,
		OutError))
	{
		if (OutError.IsEmpty())
		{
			OutError = TEXT("Provider existing Product readback fingerprint가 실패했습니다.");
		}
		return false;
	}

	if (!CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
		OutState.ProductFingerprint))
	{
		OutError = TEXT("Provider existing Product readback fingerprint가 canonical SHA-256이 아닙니다.");
		return false;
	}
	return true;
}

// Cutover binding을 재확인한 뒤 기존 Provider BuildReviewedMutationPlan/ApplyReviewed를 그대로 호출합니다.
bool FCFProviderCutoverAdapter::ApplyReviewedProduct(
	const FCFContentCutoverBinding& Binding,
	const FCFContentRecord& DesiredRecord,
	FCFContentApplyResult& OutResult)
{
	OutResult = FCFContentApplyResult();

	if (Provider == nullptr
		|| !(Binding.ContentKey == DesiredRecord.Key)
		|| !(Binding.ContentKey.ContentTypeId == ContentTypeId))
	{
		OutResult.Error = TEXT("Provider Cutover apply의 ContentKey/provider binding이 유효하지 않습니다.");
		return false;
	}

	// Mutation 직전 provider current typed snapshot입니다.
	FCFContentSnapshot CurrentSnapshot;
	if (!Provider->BuildCurrentSnapshot(
		Binding.ContentKey,
		CurrentSnapshot,
		OutResult.Error))
	{
		if (OutResult.Error.IsEmpty())
		{
			OutResult.Error = TEXT("Provider Cutover apply는 existing Product current snapshot이 필요합니다.");
		}
		return false;
	}

	if (!CurrentSnapshot.ReadbackFingerprint.Equals(
		Binding.PreApplyProductFingerprint,
		ESearchCase::CaseSensitive))
	{
		OutResult.Error = TEXT("Provider current snapshot fingerprint가 P0-07 reviewed pre-apply binding과 다릅니다.");
		return false;
	}

	// Provider-local fresh typed diff입니다.
	FCFContentDiff Diff;
	if (!Provider->BuildDiff(
		CurrentSnapshot,
		DesiredRecord,
		Diff,
		OutResult.Error))
	{
		if (OutResult.Error.IsEmpty())
		{
			OutResult.Error = TEXT("Provider Cutover apply용 fresh typed diff가 실패했습니다.");
		}
		return false;
	}

	if (!(Diff.Key == Binding.ContentKey)
		|| !Diff.CurrentFingerprint.Equals(
			Binding.PreApplyProductFingerprint,
			ESearchCase::CaseSensitive)
		|| !Diff.DesiredFingerprint.Equals(
			Binding.DesiredProductFingerprint,
			ESearchCase::CaseSensitive))
	{
		OutResult.Error = TEXT("Provider fresh typed diff가 P0-07 reviewed Product fingerprint binding과 다릅니다.");
		return false;
	}

	// Existing provider가 소유하는 immutable-style reviewed mutation plan입니다.
	FCFContentReviewedMutationPlan ProviderPlan;
	if (!Provider->BuildReviewedMutationPlan(
		Diff,
		Binding.BaseWorkbookSemanticHash,
		ProviderPlan,
		OutResult.Error))
	{
		if (OutResult.Error.IsEmpty())
		{
			OutResult.Error = TEXT("Existing provider reviewed mutation plan 생성이 실패했습니다.");
		}
		return false;
	}

	if (!(ProviderPlan.Key == Binding.ContentKey)
		|| !ProviderPlan.BaseWorkbookSemanticHash.Equals(
			Binding.BaseWorkbookSemanticHash,
			ESearchCase::CaseSensitive)
		|| !ProviderPlan.DesiredFingerprint.Equals(
			Binding.DesiredProductFingerprint,
			ESearchCase::CaseSensitive))
	{
		OutResult.Error = TEXT("Existing provider reviewed mutation plan이 P0-07 binding과 exact 일치하지 않습니다.");
		return false;
	}

	ProviderPlan.bApproved = true;
	if (!Provider->ApplyReviewed(
		ProviderPlan,
		OutResult))
	{
		if (OutResult.Error.IsEmpty())
		{
			OutResult.Error = TEXT("Existing provider reviewed Product apply가 실패했습니다.");
		}
		return false;
	}

	// Existing provider가 보고한 readback과 별도 fresh readback이 모두 desired exact인지 확인합니다.
	FString FreshReadbackFingerprint;
	FString ReadbackError;
	if (!Provider->ReadbackFingerprint(
		Binding.ContentKey,
		FreshReadbackFingerprint,
		ReadbackError)
		|| !FreshReadbackFingerprint.Equals(
			Binding.DesiredProductFingerprint,
			ESearchCase::CaseSensitive))
	{
		OutResult.Error = ReadbackError.IsEmpty()
			? TEXT("Existing provider post-apply fresh readback이 desired fingerprint와 다릅니다.")
			: ReadbackError;
		return false;
	}

	if (!OutResult.ReadbackFingerprint.IsEmpty()
		&& !OutResult.ReadbackFingerprint.Equals(
			FreshReadbackFingerprint,
			ESearchCase::CaseSensitive))
	{
		OutResult.Error = TEXT("Existing provider apply result readback과 fresh readback이 다릅니다.");
		return false;
	}

	OutResult.ReadbackFingerprint = FreshReadbackFingerprint;
	return true;
}

// Exact ContentTypeId에 adapter를 중복 없이 등록합니다.
bool FCFContentCutoverProductRegistry::RegisterAdapter(
	const TSharedRef<ICFContentCutoverProductAdapter>& Adapter,
	FString& OutError)
{
	OutError.Reset();

	// Adapter-owned ContentTypeId입니다.
	const FCFContentTypeId ContentTypeId = Adapter->DescribeContentType();
	if (!ContentTypeId.IsValid())
	{
		OutError = TEXT("Cutover Product adapter ContentTypeId가 canonical하지 않습니다.");
		return false;
	}
	if (AdaptersByContentType.Contains(ContentTypeId.Value))
	{
		OutError = FString::Printf(
			TEXT("Cutover Product adapter가 이미 등록되어 있습니다: %s"),
			*ContentTypeId.Value);
		return false;
	}

	AdaptersByContentType.Add(ContentTypeId.Value, Adapter);
	return true;
}

// Exact ContentTypeId의 adapter를 반환하며 없으면 nullptr입니다.
ICFContentCutoverProductAdapter* FCFContentCutoverProductRegistry::FindAdapter(
	const FCFContentTypeId& ContentTypeId) const
{
	const TSharedRef<ICFContentCutoverProductAdapter>* FoundAdapter =
		AdaptersByContentType.Find(ContentTypeId.Value);
	return FoundAdapter != nullptr ? &FoundAdapter->Get() : nullptr;
}

// Transaction JSON을 UE/Saved/CCAS/Cutover 아래에 저장합니다.
bool FCFContentCutoverFileStore::SaveTransaction(
	const FCFContentCutoverTransaction& Transaction,
	FString& OutError)
{
	OutError.Reset();
	if (!CFContentCutoverPrivate::IsSafeLocalId(Transaction.TransactionId))
	{
		OutError = TEXT("TransactionId는 slash 없는 stable local ID여야 합니다.");
		return false;
	}

	// Serialized transaction JSON입니다.
	FString JsonText;
	if (!CFContentCutoverPrivate::SerializeTransaction(
		Transaction,
		JsonText,
		OutError))
	{
		return false;
	}

	// Durable transaction directory입니다.
	const FString TransactionDirectory =
		FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("CCAS"), TEXT("Cutover"));
	if (!IFileManager::Get().MakeDirectory(*TransactionDirectory, true)
		&& !IFileManager::Get().DirectoryExists(*TransactionDirectory))
	{
		OutError = TEXT("Cutover transaction directory 생성에 실패했습니다.");
		return false;
	}

	// Exact transaction file path입니다.
	const FString TransactionPath = FPaths::Combine(
		TransactionDirectory,
		Transaction.TransactionId + TEXT(".json"));
	if (!CFContentCutoverPrivate::SaveStringAtomically(
		TransactionPath,
		JsonText,
		OutError))
	{
		if (OutError.IsEmpty())
		{
			OutError = TEXT("Cutover transaction JSON durable 저장에 실패했습니다.");
		}
		return false;
	}
	return true;
}

// UE/Saved/CCAS/Cutover의 exact transaction JSON을 읽습니다.
bool FCFContentCutoverFileStore::LoadTransaction(
	const FString& TransactionId,
	FCFContentCutoverTransaction& OutTransaction,
	FString& OutError) const
{
	OutTransaction = FCFContentCutoverTransaction();
	OutError.Reset();
	if (!CFContentCutoverPrivate::IsSafeLocalId(TransactionId))
	{
		OutError = TEXT("TransactionId는 slash 없는 stable local ID여야 합니다.");
		return false;
	}

	// Exact transaction file path입니다.
	const FString TransactionPath = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("CCAS"),
		TEXT("Cutover"),
		TransactionId + TEXT(".json"));

	// Durable transaction JSON text입니다.
	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *TransactionPath))
	{
		OutError = TEXT("Cutover transaction JSON을 읽지 못했습니다.");
		return false;
	}
	if (!CFContentCutoverPrivate::DeserializeTransaction(
		JsonText,
		OutTransaction,
		OutError))
	{
		return false;
	}
	if (!OutTransaction.TransactionId.Equals(
		TransactionId,
		ESearchCase::CaseSensitive))
	{
		OutError = TEXT("Cutover transaction filename identity와 payload TransactionId가 다릅니다.");
		OutTransaction = FCFContentCutoverTransaction();
		return false;
	}
	return true;
}

// Validated staged Workbook을 repository-relative canonical path로 승격합니다.
bool FCFContentCutoverFileStore::PromoteStagedWorkbook(
	const FString& StagedWorkbookPath,
	const FString& CanonicalWorkbookPath,
	FString& OutError)
{
	OutError.Reset();
	if (!StagedWorkbookPath.Equals(
		CFContentCutoverPrivate::StagedWorkbookPath,
		ESearchCase::CaseSensitive)
		|| !CanonicalWorkbookPath.Equals(
			CFContentCutoverPrivate::CanonicalWorkbookPath,
			ESearchCase::CaseSensitive))
	{
		OutError = TEXT("Workbook promotion은 P0-07 frozen canonical/staged exact2 path만 허용합니다.");
		return false;
	}

	// Resolved staged physical path입니다.
	FString StagedPhysicalPath;
	if (!CFContentCutoverPrivate::ResolveRepositoryRelativePath(
		StagedWorkbookPath,
		StagedPhysicalPath,
		OutError))
	{
		return false;
	}

	// Resolved canonical physical path입니다.
	FString CanonicalPhysicalPath;
	if (!CFContentCutoverPrivate::ResolveRepositoryRelativePath(
		CanonicalWorkbookPath,
		CanonicalPhysicalPath,
		OutError))
	{
		return false;
	}

	if (!IFileManager::Get().FileExists(*StagedPhysicalPath))
	{
		OutError = TEXT("Validated staged Workbook file이 존재하지 않습니다.");
		return false;
	}

	// Canonical parent directory입니다.
	const FString CanonicalDirectory = FPaths::GetPath(CanonicalPhysicalPath);
	if (!IFileManager::Get().MakeDirectory(*CanonicalDirectory, true)
		&& !IFileManager::Get().DirectoryExists(*CanonicalDirectory))
	{
		OutError = TEXT("Canonical Workbook parent directory 생성에 실패했습니다.");
		return false;
	}

	if (CFContentCutoverPrivate::HasExternalWorkbookLock(CanonicalPhysicalPath))
	{
		OutError = TEXT("Canonical Workbook의 Office lock file(~$...)이 존재합니다. promotion을 중단합니다.");
		return false;
	}

	// Exact staged Workbook bytes입니다.
	TArray<uint8> StagedBytes;
	if (!FFileHelper::LoadFileToArray(StagedBytes, *StagedPhysicalPath))
	{
		OutError = TEXT("Validated staged Workbook bytes를 읽지 못했습니다.");
		return false;
	}

	// Same-directory atomic replace candidate path입니다.
	const FString PromotionTempPath = CanonicalPhysicalPath + TEXT(".cutover-promoting");
	if (IFileManager::Get().FileExists(*PromotionTempPath)
		&& !IFileManager::Get().Delete(*PromotionTempPath, false, true, true))
	{
		OutError = TEXT("이전 Workbook promotion temp 파일을 정리하지 못했습니다.");
		return false;
	}

	if (!FFileHelper::SaveArrayToFile(StagedBytes, *PromotionTempPath))
	{
		OutError = TEXT("Workbook promotion temp write가 실패했습니다.");
		return false;
	}

	// Promotion temp exact byte readback입니다.
	TArray<uint8> TempBytes;
	if (!FFileHelper::LoadFileToArray(TempBytes, *PromotionTempPath)
		|| TempBytes != StagedBytes)
	{
		IFileManager::Get().Delete(*PromotionTempPath, false, true, true);
		OutError = TEXT("Workbook promotion temp readback이 staged bytes와 다릅니다.");
		return false;
	}

	if (CFContentCutoverPrivate::HasExternalWorkbookLock(CanonicalPhysicalPath))
	{
		IFileManager::Get().Delete(*PromotionTempPath, false, true, true);
		OutError = TEXT("Workbook promotion 직전 external editor lock이 감지됐습니다.");
		return false;
	}

	if (!IFileManager::Get().Move(
		*CanonicalPhysicalPath,
		*PromotionTempPath,
		true,
		true,
		false,
		true))
	{
		IFileManager::Get().Delete(*PromotionTempPath, false, true, true);
		OutError = TEXT("Workbook canonical atomic replace가 실패했습니다.");
		return false;
	}

	// Final canonical raw bytes readback입니다.
	TArray<uint8> CanonicalBytes;
	if (!FFileHelper::LoadFileToArray(CanonicalBytes, *CanonicalPhysicalPath)
		|| CanonicalBytes != StagedBytes)
	{
		OutError = TEXT("Workbook canonical raw readback이 staged bytes와 다릅니다.");
		return false;
	}
	return true;
}

// Repository-relative provenance JSON을 저장합니다.
bool FCFContentCutoverFileStore::SaveProvenance(
	const FString& ProvenancePath,
	const FCFContentProvenanceSnapshot& Snapshot,
	FString& OutError)
{
	OutError.Reset();

	// Resolved provenance physical path입니다.
	FString PhysicalPath;
	if (!CFContentCutoverPrivate::ResolveRepositoryRelativePath(
		ProvenancePath,
		PhysicalPath,
		OutError))
	{
		return false;
	}

	// Serialized provenance JSON입니다.
	FString JsonText;
	if (!CFContentCutoverPrivate::SerializeProvenance(
		Snapshot,
		JsonText,
		OutError))
	{
		return false;
	}

	// Provenance parent directory입니다.
	const FString ParentDirectory = FPaths::GetPath(PhysicalPath);
	if (!IFileManager::Get().MakeDirectory(*ParentDirectory, true)
		&& !IFileManager::Get().DirectoryExists(*ParentDirectory))
	{
		OutError = TEXT("Provenance parent directory 생성에 실패했습니다.");
		return false;
	}

	if (!CFContentCutoverPrivate::SaveStringAtomically(
		PhysicalPath,
		JsonText,
		OutError))
	{
		if (OutError.IsEmpty())
		{
			OutError = TEXT("Generated provenance JSON 저장에 실패했습니다.");
		}
		return false;
	}
	return true;
}

// Repository-relative provenance JSON을 읽습니다.
bool FCFContentCutoverFileStore::LoadProvenance(
	const FString& ProvenancePath,
	FCFContentProvenanceSnapshot& OutSnapshot,
	FString& OutError) const
{
	OutSnapshot = FCFContentProvenanceSnapshot();
	OutError.Reset();

	// Resolved provenance physical path입니다.
	FString PhysicalPath;
	if (!CFContentCutoverPrivate::ResolveRepositoryRelativePath(
		ProvenancePath,
		PhysicalPath,
		OutError))
	{
		return false;
	}

	// Persisted provenance JSON text입니다.
	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *PhysicalPath))
	{
		OutError = TEXT("Generated provenance JSON을 읽지 못했습니다.");
		return false;
	}
	return CFContentCutoverPrivate::DeserializeProvenance(
		JsonText,
		OutSnapshot,
		OutError);
}

// USER가 선택한 exact keys를 approval-bindable managed manifest로 동결합니다.
bool FCFContentCutoverCoordinator::BuildManifest(
	const FString& ManifestId,
	const FCFContentReviewPackage& ReviewPackage,
	const FCFContentCatalogSnapshot& BaseSnapshot,
	const TArray<FCFContentKey>& SelectedContentKeys,
	const FCFContentCutoverProductRegistry& ProductRegistry,
	FCFContentCutoverManifest& OutManifest,
	TArray<FCFContentValidationIssue>& OutIssues,
	FString& OutError)
{
	OutManifest = FCFContentCutoverManifest();
	OutIssues.Reset();
	OutError.Reset();

	if (!CFContentCutoverPrivate::IsSafeLocalId(ManifestId))
	{
		CFContentCutoverPrivate::AddIssue(
			OutIssues,
			TEXT("InvalidManifestId"),
			TEXT("Manifest.ManifestId"),
			TEXT("Cutover ManifestId는 slash 없는 stable local ID여야 합니다."));
	}
	if (SelectedContentKeys.IsEmpty())
	{
		CFContentCutoverPrivate::AddIssue(
			OutIssues,
			TEXT("EmptyManagedOnboarding"),
			TEXT("Manifest.Entries"),
			TEXT("Cutover Manifest에는 USER가 명시적으로 선택한 ContentKey가 하나 이상 필요합니다."));
	}
	if (!ReviewPackage.BaseCatalogSnapshotFingerprint.Equals(
		BaseSnapshot.SnapshotFingerprint,
		ESearchCase::CaseSensitive)
		|| !ReviewPackage.BaseWorkbookSemanticHash.Equals(
			BaseSnapshot.WorkbookSemanticHash,
			ESearchCase::CaseSensitive))
	{
		CFContentCutoverPrivate::AddIssue(
			OutIssues,
			TEXT("ManifestBaseSnapshotMismatch"),
			TEXT("Manifest.BaseSnapshot"),
			TEXT("Review Package와 manifest 생성 기준 Catalog/Workbook snapshot이 exact 일치하지 않습니다."));
	}

	// Duplicate onboarding key 검사용 set입니다.
	TSet<FString> SelectedKeySet;
	for (const FCFContentKey& SelectedKey : SelectedContentKeys)
	{
		// Stable selected key text입니다.
		const FString SelectedKeyText = SelectedKey.ToStableString();
		if (!SelectedKey.IsValid())
		{
			CFContentCutoverPrivate::AddIssue(
				OutIssues,
				TEXT("InvalidManagedContentKey"),
				TEXT("Manifest.Entries"),
				TEXT("Cutover Manifest의 ContentKey가 canonical하지 않습니다."));
			continue;
		}
		if (SelectedKeySet.Contains(SelectedKeyText))
		{
			CFContentCutoverPrivate::AddIssue(
				OutIssues,
				TEXT("DuplicateManagedContentKey"),
				SelectedKeyText,
				TEXT("동일 ContentKey를 Cutover Manifest에 중복 등록할 수 없습니다."));
			continue;
		}
		SelectedKeySet.Add(SelectedKeyText);

		// Reviewed desired canonical record입니다.
		const FCFContentRecord* DesiredRecord =
			CFContentCutoverPrivate::FindRecord(
				ReviewPackage.Preview.Workbook,
				SelectedKey);
		if (DesiredRecord == nullptr)
		{
			CFContentCutoverPrivate::AddIssue(
				OutIssues,
				TEXT("ManagedRecordMissing"),
				SelectedKeyText,
				TEXT("Review Package desired Workbook에 onboarding 대상 record가 없습니다."));
			continue;
		}
		if (DesiredRecord->ManagementState != ECFContentManagementState::Managed)
		{
			CFContentCutoverPrivate::AddIssue(
				OutIssues,
				TEXT("ExternalReadOnlyOnboardingBlocked"),
				SelectedKeyText,
				TEXT("ExternalReadOnly record를 CCAS-managed Product mutation 대상으로 onboarding할 수 없습니다."));
			continue;
		}

		// Readiness/dependency check 시작 전 issue count입니다.
		const int32 IssueCountBeforeReadiness = OutIssues.Num();
		CFContentCutoverPrivate::ValidateCutoverReadiness(
			*DesiredRecord,
			OutIssues);
		if (OutIssues.Num() != IssueCountBeforeReadiness)
		{
			continue;
		}

		// Exact content-type Product adapter입니다.
		ICFContentCutoverProductAdapter* ProductAdapter =
			ProductRegistry.FindAdapter(SelectedKey.ContentTypeId);
		if (ProductAdapter == nullptr)
		{
			CFContentCutoverPrivate::AddIssue(
				OutIssues,
				TEXT("MissingCutoverProductAdapter"),
				SelectedKeyText,
				TEXT("Explicit managed onboarding target의 typed Product apply adapter가 없습니다."));
			continue;
		}

		// Adapter-computed desired Product fingerprint입니다.
		FString DesiredProductFingerprint;
		// Desired Product fingerprint error입니다.
		FString AdapterError;
		if (!ProductAdapter->BuildDesiredProductFingerprint(
			*DesiredRecord,
			DesiredProductFingerprint,
			AdapterError)
			|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				DesiredProductFingerprint))
		{
			CFContentCutoverPrivate::AddIssue(
				OutIssues,
				TEXT("DesiredProductFingerprintFailed"),
				SelectedKeyText,
				AdapterError.IsEmpty()
					? TEXT("Desired Product fingerprint가 canonical하지 않습니다.")
					: AdapterError);
			continue;
		}

		// Review 시점 current/absence Product state입니다.
		FCFContentProductState ProductState;
		if (!CFContentCutoverPrivate::ReadCanonicalProductState(
			*ProductAdapter,
			SelectedKey,
			ProductState,
			AdapterError))
		{
			CFContentCutoverPrivate::AddIssue(
				OutIssues,
				TEXT("PreApplyProductFingerprintFailed"),
				SelectedKeyText,
				AdapterError);
			continue;
		}

		// Review Preview가 보존하는 canonical/product fingerprint evidence입니다.
		const FCFContentGeneratedFingerprint* GeneratedFingerprint =
			CFContentCutoverPrivate::FindGeneratedFingerprint(
				ReviewPackage.Preview,
				SelectedKey);
		if (GeneratedFingerprint == nullptr
			|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(
				GeneratedFingerprint->DesiredCanonicalFingerprint))
		{
			CFContentCutoverPrivate::AddIssue(
				OutIssues,
				TEXT("DesiredCanonicalFingerprintMissing"),
				SelectedKeyText,
				TEXT("Review Package에 desired canonical fingerprint evidence가 없습니다."));
			continue;
		}
		if (!GeneratedFingerprint->DesiredProviderFingerprint.IsEmpty()
			&& !GeneratedFingerprint->DesiredProviderFingerprint.Equals(
				DesiredProductFingerprint,
				ESearchCase::CaseSensitive))
		{
			CFContentCutoverPrivate::AddIssue(
				OutIssues,
				TEXT("DesiredProductFingerprintMismatch"),
				SelectedKeyText,
				TEXT("Review Preview와 Product cutover adapter의 desired fingerprint가 다릅니다."));
			continue;
		}
		if (ProductState.bExists
			&& !GeneratedFingerprint->CurrentProviderFingerprint.IsEmpty()
			&& !GeneratedFingerprint->CurrentProviderFingerprint.Equals(
				ProductState.ProductFingerprint,
				ESearchCase::CaseSensitive))
		{
			CFContentCutoverPrivate::AddIssue(
				OutIssues,
				TEXT("PreApplyProductFingerprintMismatch"),
				SelectedKeyText,
				TEXT("Review Preview current fingerprint와 fresh Product adapter current fingerprint가 다릅니다."));
			continue;
		}

		// New manifest entry입니다.
		FCFContentCutoverManifestEntry Entry;
		Entry.Binding.ChangeSetId = ReviewPackage.ChangeSetId;
		Entry.Binding.BaseWorkbookSemanticHash = ReviewPackage.BaseWorkbookSemanticHash;
		Entry.Binding.BaseCatalogSnapshotFingerprint = ReviewPackage.BaseCatalogSnapshotFingerprint;
		Entry.Binding.ReviewPackageFingerprint = ReviewPackage.ReviewPackageFingerprint;
		Entry.Binding.ProviderSchemaFingerprint = BaseSnapshot.ProviderSchemaFingerprint;
		Entry.Binding.ContentKey = SelectedKey;
		Entry.Binding.PreApplyProductFingerprint = ProductState.ProductFingerprint;
		Entry.Binding.DesiredProductFingerprint = DesiredProductFingerprint;
		Entry.Binding.DesiredCanonicalFingerprint =
			GeneratedFingerprint->DesiredCanonicalFingerprint;
		Entry.ManagementState = DesiredRecord->ManagementState;
		Entry.LifecycleState = DesiredRecord->LifecycleState;
		Entry.bProductMutationRequired =
			!Entry.Binding.PreApplyProductFingerprint.Equals(
				Entry.Binding.DesiredProductFingerprint,
				ESearchCase::CaseSensitive);
		OutManifest.Entries.Add(MoveTemp(Entry));
	}

	if (CFContentCutoverPrivate::HasBlockingIssues(OutIssues))
	{
		OutError = TEXT("Cutover Manifest explicit onboarding preflight가 fail-closed 됐습니다.");
		return false;
	}

	OutManifest.ManifestId = ManifestId;
	OutManifest.Entries.Sort(
		[](const FCFContentCutoverManifestEntry& Left, const FCFContentCutoverManifestEntry& Right)
		{
			return Left.Binding.ContentKey.ToStableString()
				< Right.Binding.ContentKey.ToStableString();
		});
	if (!CFContentCutoverPrivate::BuildManifestFingerprint(
		OutManifest,
		OutManifest.ManifestFingerprint,
		OutError))
	{
		return false;
	}
	return true;
}

// Provenance readback과 current Product를 비교해 direct Product drift를 fail-visible하게 찾습니다.
bool FCFContentCutoverCoordinator::DetectDirectProductDrift(
	const FCFContentProvenanceSnapshot& Snapshot,
	const FCFContentCutoverProductRegistry& ProductRegistry,
	TArray<FCFContentKey>& OutDriftedKeys,
	FString& OutError)
{
	OutDriftedKeys.Reset();
	OutError.Reset();

	for (const FCFContentProvenanceRecord& Record : Snapshot.Records)
	{
		// Exact content-type Product adapter입니다.
		ICFContentCutoverProductAdapter* ProductAdapter =
			ProductRegistry.FindAdapter(Record.ContentKey.ContentTypeId);
		if (ProductAdapter == nullptr)
		{
			OutError = FString::Printf(
				TEXT("Drift 검사 대상 Product adapter가 없습니다: %s"),
				*Record.ContentKey.ToStableString());
			return false;
		}

		// Fresh Product current/absence state입니다.
		FCFContentProductState CurrentState;
		if (!CFContentCutoverPrivate::ReadCanonicalProductState(
			*ProductAdapter,
			Record.ContentKey,
			CurrentState,
			OutError))
		{
			return false;
		}

		if (!CurrentState.bExists
			|| !CurrentState.ProductFingerprint.Equals(
				Record.PostApplyReadbackFingerprint,
				ESearchCase::CaseSensitive))
		{
			OutDriftedKeys.Add(Record.ContentKey);
		}
	}

	OutDriftedKeys.Sort(
		[](const FCFContentKey& Left, const FCFContentKey& Right)
		{
			return Left.ToStableString() < Right.ToStableString();
		});
	return true;
}

// Staged Workbook validation -> fresh stale guard -> reviewed Product apply -> Workbook commit -> provenance까지 실행합니다.
bool FCFContentCutoverCoordinator::Execute(
	const FCFContentCutoverRequest& Request,
	ICFContentWorkbookAdapter& WorkbookAdapter,
	const FCFContentProviderRegistry& ProviderRegistry,
	const FCFResourceCatalog& ResourceCatalog,
	const FCFResourcePickerRegistry& PickerRegistry,
	const FCFContentCutoverProductRegistry& ProductRegistry,
	ICFContentCutoverStore& Store,
	FCFContentCutoverTransaction& OutTransaction)
{
	OutTransaction = FCFContentCutoverTransaction();
	OutTransaction.TransactionId = Request.TransactionId;
	OutTransaction.CanonicalWorkbookPath = Request.CanonicalWorkbookPath;
	OutTransaction.StagedWorkbookPath = Request.StagedWorkbookPath;
	OutTransaction.ExpectedWorkbookSemanticHash = Request.ReviewPackage.ExpectedPostSemanticHash;
	OutTransaction.ManifestFingerprint = Request.Manifest.ManifestFingerprint;

	if (!CFContentCutoverPrivate::IsSafeLocalId(Request.TransactionId)
		|| Request.Manifest.Entries.IsEmpty())
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			TEXT("TransactionId 또는 explicit managed manifest가 유효하지 않습니다."),
			Store);
	}

	// Frozen production execution path validation 상세입니다.
	FString ExecutionPathError;
	if (!CFContentCutoverPrivate::ValidateExecutionPaths(
		Request,
		ExecutionPathError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			ExecutionPathError,
			Store);
	}

	// Request payload에서 fresh 재계산한 exact Manifest fingerprint입니다.
	FString FreshManifestFingerprint;
	// Manifest fingerprint 재계산 오류입니다.
	FString ManifestFingerprintError;
	if (!CFContentCutoverPrivate::BuildManifestFingerprint(
		Request.Manifest,
		FreshManifestFingerprint,
		ManifestFingerprintError)
		|| !FreshManifestFingerprint.Equals(
			Request.Manifest.ManifestFingerprint,
			ESearchCase::CaseSensitive))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			ManifestFingerprintError.IsEmpty()
				? TEXT("Cutover Manifest payload가 approved ManifestFingerprint와 다릅니다.")
				: ManifestFingerprintError,
			Store);
	}

	if (!Request.Approval.bApproved
		|| !Request.Approval.ReviewPackageFingerprint.Equals(
			Request.ReviewPackage.ReviewPackageFingerprint,
			ESearchCase::CaseSensitive)
		|| !Request.Approval.ManifestFingerprint.Equals(
			Request.Manifest.ManifestFingerprint,
			ESearchCase::CaseSensitive)
		|| !Request.Approval.ManifestFingerprint.Equals(
			FreshManifestFingerprint,
			ESearchCase::CaseSensitive))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			TEXT("USER approval이 exact ReviewPackage + Cutover Manifest pair에 결속되지 않았습니다."),
			Store);
	}

	// Existing P0-06 ReviewPackage approval validator input입니다.
	FCFContentReviewApproval ReviewApproval;
	ReviewApproval.ReviewPackageFingerprint =
		Request.Approval.ReviewPackageFingerprint;
	ReviewApproval.bApproved = Request.Approval.bApproved;

	// Existing P0-06 approval diagnostics입니다.
	TArray<FCFContentValidationIssue> ApprovalIssues;
	if (!FCFContentPlanningService::ValidateApproval(
		Request.ReviewPackage,
		ReviewApproval,
		ApprovalIssues))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			TEXT("P0-06 immutable ReviewPackage approval validation이 실패했습니다."),
			Store);
	}

	// Expected reviewed post Workbook semantic hash입니다.
	FString ExpectedPostSemanticHash;
	// Reviewed Workbook hash error입니다.
	FString WorkbookHashError;
	if (!FCFContentSemanticHasher::BuildWorkbookSemanticHash(
		Request.ReviewPackage.Preview.Workbook,
		ExpectedPostSemanticHash,
		WorkbookHashError)
		|| !ExpectedPostSemanticHash.Equals(
			Request.ReviewPackage.ExpectedPostSemanticHash,
			ESearchCase::CaseSensitive))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			TEXT("Review Package expected post Workbook semantic hash가 reviewed model과 일치하지 않습니다."),
			Store);
	}

	// Staged Workbook write/reopen failure 상세입니다.
	FString WorkbookError;
	if (!WorkbookAdapter.WriteStagedWorkbook(
		Request.CanonicalWorkbookPath,
		Request.StagedWorkbookPath,
		Request.ReviewPackage.Preview.Workbook,
		WorkbookError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			WorkbookError.IsEmpty()
				? TEXT("Concrete Workbook staged write가 실패했습니다.")
				: WorkbookError,
			Store);
	}

	// Staged file을 실제 reopen한 canonical model입니다.
	FCFContentWorkbookModel ReopenedStagedWorkbook;
	if (!WorkbookAdapter.ReopenWorkbook(
		Request.StagedWorkbookPath,
		ReopenedStagedWorkbook,
		WorkbookError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			WorkbookError.IsEmpty()
				? TEXT("Staged Workbook reopen이 실패했습니다.")
				: WorkbookError,
			Store);
	}

	// Reopened staged schema/content diagnostics입니다.
	TArray<FCFContentValidationIssue> StagedValidationIssues;
	if (!FCFContentSchemaValidator::ValidateWorkbook(
		ReopenedStagedWorkbook,
		StagedValidationIssues))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			TEXT("Reopened staged Workbook full canonical validation이 실패했습니다."),
			Store);
	}

	// Reopened staged semantic hash입니다.
	FString ReopenedStagedSemanticHash;
	if (!FCFContentSemanticHasher::BuildWorkbookSemanticHash(
		ReopenedStagedWorkbook,
		ReopenedStagedSemanticHash,
		WorkbookHashError)
		|| !ReopenedStagedSemanticHash.Equals(
			Request.ReviewPackage.ExpectedPostSemanticHash,
			ESearchCase::CaseSensitive))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			TEXT("Staged Workbook reopen semantic hash가 reviewed expected hash와 다릅니다."),
			Store);
	}

	// Fresh canonical base Workbook + Product truth compiler result입니다.
	FCFContentCompileResult FreshCompileResult;
	if (!FCFContentCompiler::CompilePreview(
		Request.CanonicalWorkbookPath,
		WorkbookAdapter,
		ProviderRegistry,
		FreshCompileResult))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			FreshCompileResult.Error.IsEmpty()
				? TEXT("Apply 직전 fresh Compiler Preview가 실패했습니다.")
				: FreshCompileResult.Error,
			Store);
	}

	// Apply 직전 whole-Catalog fresh snapshot입니다.
	FCFContentCatalogSnapshot FreshSnapshot;
	// Fresh snapshot build error입니다.
	FString SnapshotError;
	if (!FCFContentPlanningService::BuildCatalogSnapshot(
		FreshCompileResult,
		ProviderRegistry,
		ResourceCatalog,
		PickerRegistry,
		FreshSnapshot,
		SnapshotError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			SnapshotError.IsEmpty()
				? TEXT("Apply 직전 fresh Catalog Snapshot 생성이 실패했습니다.")
				: SnapshotError,
			Store);
	}

	// Two-layer P0-06 freshness diagnostics입니다.
	TArray<FCFContentValidationIssue> FreshnessIssues;
	if (!FCFContentPlanningService::ValidateReviewFreshness(
		Request.ReviewPackage,
		FreshSnapshot,
		FreshnessIssues))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			TEXT("Review 뒤 Workbook/Catalog/Product/Resource/Provider truth가 변경됐습니다. fresh Review Package가 필요합니다."),
			Store);
	}

	// Fresh registered provider schema fingerprint입니다.
	FString FreshProviderSchemaFingerprint;
	// Provider schema fingerprint error입니다.
	FString ProviderError;
	if (!ProviderRegistry.BuildSchemaFingerprint(
		FreshProviderSchemaFingerprint,
		ProviderError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::BlockedBeforeMutation,
			ProviderError.IsEmpty()
				? TEXT("Fresh ProviderSchemaFingerprint 생성이 실패했습니다.")
				: ProviderError,
			Store);
	}

	// Global preflight에서 exact target reports를 미리 구성합니다.
	for (const FCFContentCutoverManifestEntry& Entry : Request.Manifest.Entries)
	{
		// New target transaction report입니다.
		FCFContentCutoverTargetReport TargetReport;
		TargetReport.ContentKey = Entry.Binding.ContentKey;
		TargetReport.DesiredProductFingerprint =
			Entry.Binding.DesiredProductFingerprint;
		OutTransaction.Targets.Add(MoveTemp(TargetReport));
	}

	for (int32 EntryIndex = 0;
		EntryIndex < Request.Manifest.Entries.Num();
		++EntryIndex)
	{
		// Approved manifest entry입니다.
		const FCFContentCutoverManifestEntry& Entry =
			Request.Manifest.Entries[EntryIndex];
		// Matching transaction target report입니다.
		FCFContentCutoverTargetReport& TargetReport =
			OutTransaction.Targets[EntryIndex];

		if (!Entry.Binding.ChangeSetId.Equals(
				Request.ReviewPackage.ChangeSetId,
				ESearchCase::CaseSensitive)
			|| !Entry.Binding.BaseWorkbookSemanticHash.Equals(
				Request.ReviewPackage.BaseWorkbookSemanticHash,
				ESearchCase::CaseSensitive)
			|| !Entry.Binding.BaseCatalogSnapshotFingerprint.Equals(
				Request.ReviewPackage.BaseCatalogSnapshotFingerprint,
				ESearchCase::CaseSensitive)
			|| !Entry.Binding.ReviewPackageFingerprint.Equals(
				Request.ReviewPackage.ReviewPackageFingerprint,
				ESearchCase::CaseSensitive)
			|| !Entry.Binding.ProviderSchemaFingerprint.Equals(
				FreshProviderSchemaFingerprint,
				ESearchCase::CaseSensitive))
		{
			TargetReport.State = ECFContentCutoverTargetState::Blocked;
			TargetReport.Diagnostic = TEXT("P0-07 approval binding identity가 fresh state와 exact 일치하지 않습니다.");
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				ECFContentCutoverState::BlockedBeforeMutation,
				TargetReport.Diagnostic,
				Store);
		}

		// Reviewed desired record입니다.
		const FCFContentRecord* DesiredRecord =
			CFContentCutoverPrivate::FindRecord(
				Request.ReviewPackage.Preview.Workbook,
				Entry.Binding.ContentKey);
		if (DesiredRecord == nullptr
			|| DesiredRecord->ManagementState != ECFContentManagementState::Managed)
		{
			TargetReport.State = ECFContentCutoverTargetState::Blocked;
			TargetReport.Diagnostic = TEXT("Reviewed desired managed record를 찾지 못했습니다.");
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				ECFContentCutoverState::BlockedBeforeMutation,
				TargetReport.Diagnostic,
				Store);
		}

		if (Entry.ManagementState != DesiredRecord->ManagementState
			|| Entry.LifecycleState != DesiredRecord->LifecycleState)
		{
			TargetReport.State = ECFContentCutoverTargetState::Blocked;
			TargetReport.Diagnostic = TEXT("Manifest management/lifecycle state가 reviewed desired record와 다릅니다.");
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				ECFContentCutoverState::BlockedBeforeMutation,
				TargetReport.Diagnostic,
				Store);
		}

		// Product cutover readiness diagnostics입니다.
		TArray<FCFContentValidationIssue> ReadinessIssues;
		if (!CFContentCutoverPrivate::ValidateCutoverReadiness(
			*DesiredRecord,
			ReadinessIssues))
		{
			TargetReport.State = ECFContentCutoverTargetState::Blocked;
			TargetReport.Diagnostic = TEXT("Reviewed desired record의 readiness/dependency가 cutover를 허용하지 않습니다.");
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				ECFContentCutoverState::BlockedBeforeMutation,
				TargetReport.Diagnostic,
				Store);
		}

		// Exact content-type Product adapter입니다.
		ICFContentCutoverProductAdapter* ProductAdapter =
			ProductRegistry.FindAdapter(Entry.Binding.ContentKey.ContentTypeId);
		if (ProductAdapter == nullptr)
		{
			TargetReport.State = ECFContentCutoverTargetState::Blocked;
			TargetReport.Diagnostic = TEXT("Typed Cutover Product adapter가 없습니다.");
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				ECFContentCutoverState::BlockedBeforeMutation,
				TargetReport.Diagnostic,
				Store);
		}

		// Fresh desired Product fingerprint입니다.
		FString FreshDesiredFingerprint;
		// Product adapter failure 상세입니다.
		FString ProductError;
		if (!ProductAdapter->BuildDesiredProductFingerprint(
			*DesiredRecord,
			FreshDesiredFingerprint,
			ProductError)
			|| !FreshDesiredFingerprint.Equals(
				Entry.Binding.DesiredProductFingerprint,
				ESearchCase::CaseSensitive))
		{
			TargetReport.State = ECFContentCutoverTargetState::Blocked;
			TargetReport.Diagnostic = ProductError.IsEmpty()
				? TEXT("Fresh desired Product fingerprint가 reviewed binding과 다릅니다.")
				: ProductError;
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				ECFContentCutoverState::BlockedBeforeMutation,
				TargetReport.Diagnostic,
				Store);
		}

		// Global preflight fresh Product current/absence state입니다.
		FCFContentProductState FreshProductState;
		if (!CFContentCutoverPrivate::ReadCanonicalProductState(
			*ProductAdapter,
			Entry.Binding.ContentKey,
			FreshProductState,
			ProductError))
		{
			TargetReport.State = ECFContentCutoverTargetState::Blocked;
			TargetReport.Diagnostic = ProductError;
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				ECFContentCutoverState::BlockedBeforeMutation,
				TargetReport.Diagnostic,
				Store);
		}
		TargetReport.FreshPreApplyProductFingerprint =
			FreshProductState.ProductFingerprint;

		if (!FreshProductState.ProductFingerprint.Equals(
			Entry.Binding.PreApplyProductFingerprint,
			ESearchCase::CaseSensitive))
		{
			TargetReport.State = ECFContentCutoverTargetState::Blocked;
			TargetReport.Diagnostic = TEXT("Product current truth가 review 이후 변경됐습니다. stale approval reuse 없이 새 Review Package가 필요합니다.");
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				ECFContentCutoverState::BlockedBeforeMutation,
				TargetReport.Diagnostic,
				Store);
		}

		// Fresh current/desired에서 재계산한 mutation requirement입니다.
		const bool bFreshProductMutationRequired =
			!FreshProductState.ProductFingerprint.Equals(
				FreshDesiredFingerprint,
				ESearchCase::CaseSensitive);
		if (Entry.bProductMutationRequired != bFreshProductMutationRequired)
		{
			TargetReport.State = ECFContentCutoverTargetState::Blocked;
			TargetReport.Diagnostic = TEXT("Manifest ProductMutationRequired가 fresh current/desired truth와 다릅니다.");
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				ECFContentCutoverState::BlockedBeforeMutation,
				TargetReport.Diagnostic,
				Store);
		}
	}

	OutTransaction.State = ECFContentCutoverState::Prepared;
	OutTransaction.Diagnostic = TEXT("Staged Workbook validation + fresh approval/stale guard PASS.");
	// First Product mutation 전 durable Prepared evidence 저장 오류입니다.
	FString StoreError;
	if (!Store.SaveTransaction(OutTransaction, StoreError))
	{
		OutTransaction.Diagnostic = StoreError.IsEmpty()
			? TEXT("Prepared transaction durable 저장이 실패했습니다.")
			: StoreError;
		return false;
	}

	// 이미 durable mutation된 target 수입니다.
	int32 AppliedTargetCount = 0;
	for (int32 EntryIndex = 0;
		EntryIndex < Request.Manifest.Entries.Num();
		++EntryIndex)
	{
		// Approved manifest entry입니다.
		const FCFContentCutoverManifestEntry& Entry =
			Request.Manifest.Entries[EntryIndex];
		// Matching target report입니다.
		FCFContentCutoverTargetReport& TargetReport =
			OutTransaction.Targets[EntryIndex];
		// Exact desired record입니다.
		const FCFContentRecord* DesiredRecord =
			CFContentCutoverPrivate::FindRecord(
				Request.ReviewPackage.Preview.Workbook,
				Entry.Binding.ContentKey);
		// Exact typed Product adapter입니다.
		ICFContentCutoverProductAdapter* ProductAdapter =
			ProductRegistry.FindAdapter(Entry.Binding.ContentKey.ContentTypeId);
		if (DesiredRecord == nullptr || ProductAdapter == nullptr)
		{
			TargetReport.State = ECFContentCutoverTargetState::Failed;
			TargetReport.Diagnostic = TEXT("Apply execution target를 재확정하지 못했습니다.");
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				AppliedTargetCount > 0
					? ECFContentCutoverState::RecoveryRequired
					: ECFContentCutoverState::BlockedBeforeMutation,
				TargetReport.Diagnostic,
				Store);
		}

		// Mutation 직전 exact Product state입니다.
		FCFContentProductState ImmediateProductState;
		// Immediate Product state error입니다.
		FString ProductError;
		if (!CFContentCutoverPrivate::ReadCanonicalProductState(
			*ProductAdapter,
			Entry.Binding.ContentKey,
			ImmediateProductState,
			ProductError)
			|| !ImmediateProductState.ProductFingerprint.Equals(
				Entry.Binding.PreApplyProductFingerprint,
				ESearchCase::CaseSensitive))
		{
			TargetReport.State = ECFContentCutoverTargetState::Blocked;
			TargetReport.Diagnostic = ProductError.IsEmpty()
				? TEXT("Global preflight 이후 Product current truth가 변경됐습니다.")
				: ProductError;
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				AppliedTargetCount > 0
					? ECFContentCutoverState::RecoveryRequired
					: ECFContentCutoverState::BlockedBeforeMutation,
				TargetReport.Diagnostic,
				Store);
		}

		if (ImmediateProductState.bExists
			&& ImmediateProductState.ProductFingerprint.Equals(
				Entry.Binding.DesiredProductFingerprint,
				ESearchCase::CaseSensitive))
		{
			TargetReport.State = ECFContentCutoverTargetState::NoChange;
			TargetReport.PostApplyReadbackFingerprint =
				ImmediateProductState.ProductFingerprint;
			continue;
		}

		// Typed provider durable apply result입니다.
		FCFContentApplyResult ApplyResult;
		if (!ProductAdapter->ApplyReviewedProduct(
			Entry.Binding,
			*DesiredRecord,
			ApplyResult))
		{
			TargetReport.State = ECFContentCutoverTargetState::Failed;
			TargetReport.bProductMutationPerformed = ApplyResult.bApplied;
			TargetReport.PostApplyReadbackFingerprint =
				ApplyResult.ReadbackFingerprint;
			TargetReport.Diagnostic = ApplyResult.Error.IsEmpty()
				? TEXT("Typed Product reviewed apply가 실패했습니다.")
				: ApplyResult.Error;
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				(AppliedTargetCount > 0 || ApplyResult.bApplied)
					? ECFContentCutoverState::RecoveryRequired
					: ECFContentCutoverState::BlockedBeforeMutation,
				TargetReport.Diagnostic,
				Store);
		}
		TargetReport.bProductMutationPerformed = ApplyResult.bApplied;
		if (ApplyResult.bApplied)
		{
			++AppliedTargetCount;
			TargetReport.State = ECFContentCutoverTargetState::Applied;
		}

		// Apply 뒤 exact persisted Product state입니다.
		FCFContentProductState PostApplyState;
		if (!CFContentCutoverPrivate::ReadCanonicalProductState(
			*ProductAdapter,
			Entry.Binding.ContentKey,
			PostApplyState,
			ProductError)
			|| !PostApplyState.bExists
			|| !PostApplyState.ProductFingerprint.Equals(
				Entry.Binding.DesiredProductFingerprint,
				ESearchCase::CaseSensitive))
		{
			TargetReport.State = ECFContentCutoverTargetState::Failed;
			TargetReport.Diagnostic = ProductError.IsEmpty()
				? TEXT("Product Apply 뒤 persisted readback fingerprint가 desired와 exact 일치하지 않습니다.")
				: ProductError;
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				ECFContentCutoverState::RecoveryRequired,
				TargetReport.Diagnostic,
				Store);
		}

		TargetReport.State = ECFContentCutoverTargetState::Verified;
		TargetReport.PostApplyReadbackFingerprint =
			PostApplyState.ProductFingerprint;

		if (!Store.SaveTransaction(OutTransaction, StoreError))
		{
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				ECFContentCutoverState::RecoveryRequired,
				StoreError.IsEmpty()
					? TEXT("Per-Product durable transaction journal 저장이 실패했습니다.")
					: StoreError,
				Store);
		}
	}

	OutTransaction.State = ECFContentCutoverState::ProductApplied;
	OutTransaction.Diagnostic = TEXT("Required Product targets가 desired fingerprint와 exact 일치합니다.");
	if (!Store.SaveTransaction(OutTransaction, StoreError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			StoreError.IsEmpty()
				? TEXT("ProductApplied durable transaction 저장이 실패했습니다.")
				: StoreError,
			Store);
	}

	// Product batch 동안 staged candidate가 바뀌지 않았는지 promotion 직전에 다시 검증합니다.
	FCFContentWorkbookModel FinalStagedWorkbook;
	if (!WorkbookAdapter.ReopenWorkbook(
		Request.StagedWorkbookPath,
		FinalStagedWorkbook,
		WorkbookError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			WorkbookError.IsEmpty()
				? TEXT("Promotion 직전 staged Workbook reopen이 실패했습니다.")
				: WorkbookError,
			Store);
	}

	// Promotion 직전 staged semantic hash입니다.
	FString FinalStagedSemanticHash;
	if (!FCFContentSemanticHasher::BuildWorkbookSemanticHash(
		FinalStagedWorkbook,
		FinalStagedSemanticHash,
		WorkbookHashError)
		|| !FinalStagedSemanticHash.Equals(
			Request.ReviewPackage.ExpectedPostSemanticHash,
			ESearchCase::CaseSensitive))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			TEXT("Promotion 직전 staged Workbook semantic hash가 reviewed candidate와 다릅니다."),
			Store);
	}

	// Product batch 동안 canonical base Workbook이 바뀌지 않았는지 promotion 직전에 다시 읽습니다.
	FCFContentWorkbookModel FinalBaseWorkbook;
	if (!WorkbookAdapter.ReopenWorkbook(
		Request.CanonicalWorkbookPath,
		FinalBaseWorkbook,
		WorkbookError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			WorkbookError.IsEmpty()
				? TEXT("Promotion 직전 canonical Workbook reopen이 실패했습니다.")
				: WorkbookError,
			Store);
	}

	// Promotion 직전 canonical base semantic hash입니다.
	FString FinalBaseSemanticHash;
	if (!FCFContentSemanticHasher::BuildWorkbookSemanticHash(
		FinalBaseWorkbook,
		FinalBaseSemanticHash,
		WorkbookHashError)
		|| !FinalBaseSemanticHash.Equals(
			Request.ReviewPackage.BaseWorkbookSemanticHash,
			ESearchCase::CaseSensitive))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			TEXT("Product apply 동안 canonical Workbook이 변경됐습니다. staged promotion을 중단합니다."),
			Store);
	}

	if (!Store.PromoteStagedWorkbook(
		Request.StagedWorkbookPath,
		Request.CanonicalWorkbookPath,
		StoreError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			StoreError.IsEmpty()
				? TEXT("Validated staged Workbook canonical promotion이 실패했습니다.")
				: StoreError,
			Store);
	}

	// Promoted canonical Workbook reopen model입니다.
	FCFContentWorkbookModel ReopenedCanonicalWorkbook;
	if (!WorkbookAdapter.ReopenWorkbook(
		Request.CanonicalWorkbookPath,
		ReopenedCanonicalWorkbook,
		WorkbookError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			WorkbookError.IsEmpty()
				? TEXT("Promoted canonical Workbook reopen이 실패했습니다.")
				: WorkbookError,
			Store);
	}

	// Promoted canonical Workbook semantic hash입니다.
	FString CanonicalSemanticHash;
	if (!FCFContentSemanticHasher::BuildWorkbookSemanticHash(
		ReopenedCanonicalWorkbook,
		CanonicalSemanticHash,
		WorkbookHashError)
		|| !CanonicalSemanticHash.Equals(
			Request.ReviewPackage.ExpectedPostSemanticHash,
			ESearchCase::CaseSensitive))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			TEXT("Promoted canonical Workbook semantic hash가 reviewed expected hash와 다릅니다."),
			Store);
	}

	OutTransaction.State = ECFContentCutoverState::WorkbookCommitted;
	OutTransaction.Diagnostic = TEXT("Canonical Workbook promotion + reopen semantic validation PASS.");
	if (!Store.SaveTransaction(OutTransaction, StoreError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			StoreError.IsEmpty()
				? TEXT("WorkbookCommitted durable transaction 저장이 실패했습니다.")
				: StoreError,
			Store);
	}

	// Generated durable provenance snapshot입니다.
	FCFContentProvenanceSnapshot ProvenanceSnapshot;
	ProvenanceSnapshot.CutoverTransactionId = Request.TransactionId;
	for (int32 EntryIndex = 0;
		EntryIndex < Request.Manifest.Entries.Num();
		++EntryIndex)
	{
		// Approved manifest entry입니다.
		const FCFContentCutoverManifestEntry& Entry =
			Request.Manifest.Entries[EntryIndex];
		// Matching terminal target report입니다.
		const FCFContentCutoverTargetReport& TargetReport =
			OutTransaction.Targets[EntryIndex];

		// Durable provenance record입니다.
		FCFContentProvenanceRecord ProvenanceRecord;
		ProvenanceRecord.ContentKey = Entry.Binding.ContentKey;
		ProvenanceRecord.CutoverTransactionId = Request.TransactionId;
		ProvenanceRecord.SourceWorkbookSemanticHash =
			Request.ReviewPackage.ExpectedPostSemanticHash;
		ProvenanceRecord.CatalogSnapshotFingerprint =
			Entry.Binding.BaseCatalogSnapshotFingerprint;
		ProvenanceRecord.ReviewPackageFingerprint =
			Entry.Binding.ReviewPackageFingerprint;
		ProvenanceRecord.ProviderSchemaFingerprint =
			Entry.Binding.ProviderSchemaFingerprint;
		ProvenanceRecord.DesiredCanonicalFingerprint =
			Entry.Binding.DesiredCanonicalFingerprint;
		ProvenanceRecord.PreApplyProductFingerprint =
			Entry.Binding.PreApplyProductFingerprint;
		ProvenanceRecord.PostApplyReadbackFingerprint =
			TargetReport.PostApplyReadbackFingerprint;
		ProvenanceRecord.ManagementState = Entry.ManagementState;
		ProvenanceRecord.LifecycleState = Entry.LifecycleState;
		ProvenanceSnapshot.Records.Add(MoveTemp(ProvenanceRecord));
	}

	if (!Store.SaveProvenance(
		Request.ProvenancePath,
		ProvenanceSnapshot,
		StoreError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			StoreError.IsEmpty()
				? TEXT("Generated provenance snapshot durable 저장이 실패했습니다.")
				: StoreError,
			Store);
	}

	OutTransaction.State = ECFContentCutoverState::Verified;
	OutTransaction.bAuthorityActivationEligible = true;
	OutTransaction.bAuthorityActivated = false;
	OutTransaction.Diagnostic =
		TEXT("P0-07 technical transaction Verified. USER authority activation acceptance는 아직 적용되지 않았습니다.");
	if (!Store.SaveTransaction(OutTransaction, StoreError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			StoreError.IsEmpty()
				? TEXT("Verified durable transaction 저장이 실패했습니다.")
				: StoreError,
			Store);
	}

	return true;
}

// RecoveryRequired transaction에서 Product를 다시 쓰지 않고 exact staged candidate finalize만 재개합니다.
bool FCFContentCutoverCoordinator::ResumeRecovery(
	const FCFContentCutoverRequest& Request,
	ICFContentWorkbookAdapter& WorkbookAdapter,
	const FCFContentCutoverProductRegistry& ProductRegistry,
	ICFContentCutoverStore& Store,
	FCFContentCutoverTransaction& OutTransaction)
{
	OutTransaction = FCFContentCutoverTransaction();

	// Frozen production execution path validation 상세입니다.
	FString RecoveryError;
	if (!CFContentCutoverPrivate::ValidateExecutionPaths(Request, RecoveryError))
	{
		OutTransaction.TransactionId = Request.TransactionId;
		OutTransaction.State = ECFContentCutoverState::RecoveryRequired;
		OutTransaction.Diagnostic = RecoveryError;
		return false;
	}

	// Durable recovery transaction입니다.
	FCFContentCutoverTransaction LoadedTransaction;
	if (!Store.LoadTransaction(
		Request.TransactionId,
		LoadedTransaction,
		RecoveryError))
	{
		OutTransaction.TransactionId = Request.TransactionId;
		OutTransaction.State = ECFContentCutoverState::RecoveryRequired;
		OutTransaction.Diagnostic = RecoveryError.IsEmpty()
			? TEXT("Recovery transaction durable evidence를 읽지 못했습니다.")
			: RecoveryError;
		return false;
	}
	OutTransaction = LoadedTransaction;

	if (OutTransaction.State != ECFContentCutoverState::RecoveryRequired)
	{
		OutTransaction.Diagnostic = TEXT("ResumeRecovery는 durable RecoveryRequired transaction만 허용합니다.");
		return false;
	}

	// Request payload에서 fresh 재계산한 manifest fingerprint입니다.
	FString FreshManifestFingerprint;
	if (!CFContentCutoverPrivate::BuildManifestFingerprint(
		Request.Manifest,
		FreshManifestFingerprint,
		RecoveryError)
		|| !FreshManifestFingerprint.Equals(
			Request.Manifest.ManifestFingerprint,
			ESearchCase::CaseSensitive)
		|| !FreshManifestFingerprint.Equals(
			Request.Approval.ManifestFingerprint,
			ESearchCase::CaseSensitive)
		|| !FreshManifestFingerprint.Equals(
			OutTransaction.ManifestFingerprint,
			ESearchCase::CaseSensitive))
	{
		OutTransaction.Diagnostic = RecoveryError.IsEmpty()
			? TEXT("Recovery request Manifest payload/fingerprint가 durable approved transaction과 다릅니다.")
			: RecoveryError;
		return false;
	}

	// Recovery에서도 P0-06 immutable ReviewPackage approval을 fresh payload 기준으로 다시 검증합니다.
	FCFContentReviewApproval ReviewApproval;
	ReviewApproval.ReviewPackageFingerprint = Request.Approval.ReviewPackageFingerprint;
	ReviewApproval.bApproved = Request.Approval.bApproved;
	// Recovery ReviewPackage approval diagnostics입니다.
	TArray<FCFContentValidationIssue> ApprovalIssues;
	if (!FCFContentPlanningService::ValidateApproval(
		Request.ReviewPackage,
		ReviewApproval,
		ApprovalIssues))
	{
		OutTransaction.Diagnostic = TEXT("Recovery ReviewPackage approval/payload validation이 실패했습니다.");
		return false;
	}

	if (!Request.Approval.bApproved
		|| !Request.Approval.ReviewPackageFingerprint.Equals(
			Request.ReviewPackage.ReviewPackageFingerprint,
			ESearchCase::CaseSensitive)
		|| !OutTransaction.CanonicalWorkbookPath.Equals(
			Request.CanonicalWorkbookPath,
			ESearchCase::CaseSensitive)
		|| !OutTransaction.StagedWorkbookPath.Equals(
			Request.StagedWorkbookPath,
			ESearchCase::CaseSensitive)
		|| !OutTransaction.ExpectedWorkbookSemanticHash.Equals(
			Request.ReviewPackage.ExpectedPostSemanticHash,
			ESearchCase::CaseSensitive))
	{
		OutTransaction.Diagnostic = TEXT("Recovery request identity/path/hash가 durable transaction과 exact 일치하지 않습니다.");
		return false;
	}

	// Recovery는 Product 재적용 없이 모든 exact target이 already-desired일 때만 finalize합니다.
	if (OutTransaction.Targets.Num() != Request.Manifest.Entries.Num())
	{
		OutTransaction.Diagnostic = TEXT("Recovery durable target count와 approved manifest count가 다릅니다.");
		return false;
	}

	for (const FCFContentCutoverManifestEntry& Entry : Request.Manifest.Entries)
	{
		// Reviewed desired record입니다.
		const FCFContentRecord* DesiredRecord =
			CFContentCutoverPrivate::FindRecord(
				Request.ReviewPackage.Preview.Workbook,
				Entry.Binding.ContentKey);
		if (DesiredRecord == nullptr
			|| Entry.ManagementState != DesiredRecord->ManagementState
			|| Entry.LifecycleState != DesiredRecord->LifecycleState)
		{
			OutTransaction.Diagnostic = TEXT("Recovery manifest state가 reviewed desired record와 다릅니다.");
			return false;
		}

		// Exact Product adapter입니다.
		ICFContentCutoverProductAdapter* ProductAdapter =
			ProductRegistry.FindAdapter(Entry.Binding.ContentKey.ContentTypeId);
		if (ProductAdapter == nullptr)
		{
			OutTransaction.Diagnostic = TEXT("Recovery target Product adapter가 없습니다.");
			return false;
		}

		// Fresh desired Product fingerprint입니다.
		FString FreshDesiredFingerprint;
		if (!ProductAdapter->BuildDesiredProductFingerprint(
			*DesiredRecord,
			FreshDesiredFingerprint,
			RecoveryError)
			|| !FreshDesiredFingerprint.Equals(
				Entry.Binding.DesiredProductFingerprint,
				ESearchCase::CaseSensitive))
		{
			OutTransaction.Diagnostic = RecoveryError.IsEmpty()
				? TEXT("Recovery fresh desired Product fingerprint가 approved binding과 다릅니다.")
				: RecoveryError;
			return false;
		}

		// Fresh Product current state입니다.
		FCFContentProductState CurrentProductState;
		if (!CFContentCutoverPrivate::ReadCanonicalProductState(
			*ProductAdapter,
			Entry.Binding.ContentKey,
			CurrentProductState,
			RecoveryError)
			|| !CurrentProductState.bExists
			|| !CurrentProductState.ProductFingerprint.Equals(
				Entry.Binding.DesiredProductFingerprint,
				ESearchCase::CaseSensitive))
		{
			OutTransaction.Diagnostic = RecoveryError.IsEmpty()
				? TEXT("Recovery finalize는 모든 Product가 reviewed desired fingerprint와 이미 exact 일치할 때만 허용합니다. 별도 reviewed recovery/reverse plan이 필요합니다.")
				: RecoveryError;
			FString SaveError;
			Store.SaveTransaction(OutTransaction, SaveError);
			return false;
		}

		// Durable matching target count입니다.
		int32 MatchingTargetCount = 0;
		for (FCFContentCutoverTargetReport& TargetReport : OutTransaction.Targets)
		{
			if (TargetReport.ContentKey == Entry.Binding.ContentKey)
			{
				++MatchingTargetCount;
				TargetReport.State = ECFContentCutoverTargetState::Verified;
				TargetReport.DesiredProductFingerprint = Entry.Binding.DesiredProductFingerprint;
				TargetReport.PostApplyReadbackFingerprint = CurrentProductState.ProductFingerprint;
				TargetReport.Diagnostic = TEXT("Recovery fresh Product readback exact desired.");
			}
		}
		if (MatchingTargetCount != 1)
		{
			OutTransaction.Diagnostic = TEXT("Recovery durable target identity가 exact1이 아닙니다.");
			return false;
		}
	}

	// Exact staged candidate를 다시 열어 reviewed expected hash를 검증합니다.
	FCFContentWorkbookModel RecoveryStagedWorkbook;
	if (!WorkbookAdapter.ReopenWorkbook(
		Request.StagedWorkbookPath,
		RecoveryStagedWorkbook,
		RecoveryError))
	{
		OutTransaction.Diagnostic = RecoveryError.IsEmpty()
			? TEXT("Recovery staged Workbook reopen이 실패했습니다.")
			: RecoveryError;
		return false;
	}

	// Recovery staged semantic hash입니다.
	FString RecoveryStagedHash;
	if (!FCFContentSemanticHasher::BuildWorkbookSemanticHash(
		RecoveryStagedWorkbook,
		RecoveryStagedHash,
		RecoveryError)
		|| !RecoveryStagedHash.Equals(
			Request.ReviewPackage.ExpectedPostSemanticHash,
			ESearchCase::CaseSensitive))
	{
		OutTransaction.Diagnostic = TEXT("Recovery staged Workbook이 approved exact candidate가 아닙니다.");
		return false;
	}

	// Current canonical Workbook model입니다.
	FCFContentWorkbookModel RecoveryCanonicalWorkbook;
	if (!WorkbookAdapter.ReopenWorkbook(
		Request.CanonicalWorkbookPath,
		RecoveryCanonicalWorkbook,
		RecoveryError))
	{
		OutTransaction.Diagnostic = RecoveryError.IsEmpty()
			? TEXT("Recovery canonical Workbook reopen이 실패했습니다.")
			: RecoveryError;
		return false;
	}

	// Current canonical Workbook semantic hash입니다.
	FString RecoveryCanonicalHash;
	if (!FCFContentSemanticHasher::BuildWorkbookSemanticHash(
		RecoveryCanonicalWorkbook,
		RecoveryCanonicalHash,
		RecoveryError))
	{
		OutTransaction.Diagnostic = RecoveryError;
		return false;
	}

	// Canonical이 아직 base라면 exact staged candidate만 finalize합니다.
	if (RecoveryCanonicalHash.Equals(
		Request.ReviewPackage.BaseWorkbookSemanticHash,
		ESearchCase::CaseSensitive))
	{
		if (!Store.PromoteStagedWorkbook(
			Request.StagedWorkbookPath,
			Request.CanonicalWorkbookPath,
			RecoveryError))
		{
			OutTransaction.Diagnostic = RecoveryError.IsEmpty()
				? TEXT("Recovery staged Workbook finalize promotion이 실패했습니다.")
				: RecoveryError;
			FString SaveError;
			Store.SaveTransaction(OutTransaction, SaveError);
			return false;
		}

		// Promoted canonical semantic readback입니다.
		FCFContentWorkbookModel PromotedWorkbook;
		if (!WorkbookAdapter.ReopenWorkbook(
			Request.CanonicalWorkbookPath,
			PromotedWorkbook,
			RecoveryError)
			|| !FCFContentSemanticHasher::BuildWorkbookSemanticHash(
				PromotedWorkbook,
				RecoveryCanonicalHash,
				RecoveryError))
		{
			OutTransaction.Diagnostic = RecoveryError.IsEmpty()
				? TEXT("Recovery promoted canonical Workbook readback이 실패했습니다.")
				: RecoveryError;
			return false;
		}
	}

	if (!RecoveryCanonicalHash.Equals(
		Request.ReviewPackage.ExpectedPostSemanticHash,
		ESearchCase::CaseSensitive))
	{
		OutTransaction.Diagnostic = TEXT("Recovery canonical Workbook이 reviewed base도 expected candidate도 아닙니다. 자동 finalize를 금지합니다.");
		return false;
	}

	OutTransaction.State = ECFContentCutoverState::WorkbookCommitted;
	OutTransaction.Diagnostic = TEXT("Recovery exact staged candidate finalize + canonical semantic validation PASS.");
	FString StoreError;
	if (!Store.SaveTransaction(OutTransaction, StoreError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			StoreError.IsEmpty()
				? TEXT("Recovery WorkbookCommitted durable 저장이 실패했습니다.")
				: StoreError,
			Store);
	}

	// Recovery-generated provenance snapshot입니다.
	FCFContentProvenanceSnapshot ProvenanceSnapshot;
	ProvenanceSnapshot.CutoverTransactionId = Request.TransactionId;
	for (const FCFContentCutoverManifestEntry& Entry : Request.Manifest.Entries)
	{
		// Exact matching terminal target입니다.
		const FCFContentCutoverTargetReport* TargetReport =
			OutTransaction.Targets.FindByPredicate(
				[&Entry](const FCFContentCutoverTargetReport& Candidate)
				{
					return Candidate.ContentKey == Entry.Binding.ContentKey;
				});
		if (TargetReport == nullptr)
		{
			return CFContentCutoverPrivate::FailTransaction(
				OutTransaction,
				ECFContentCutoverState::RecoveryRequired,
				TEXT("Recovery provenance target evidence를 찾지 못했습니다."),
				Store);
		}

		// Durable provenance record입니다.
		FCFContentProvenanceRecord ProvenanceRecord;
		ProvenanceRecord.ContentKey = Entry.Binding.ContentKey;
		ProvenanceRecord.CutoverTransactionId = Request.TransactionId;
		ProvenanceRecord.SourceWorkbookSemanticHash = Request.ReviewPackage.ExpectedPostSemanticHash;
		ProvenanceRecord.CatalogSnapshotFingerprint = Entry.Binding.BaseCatalogSnapshotFingerprint;
		ProvenanceRecord.ReviewPackageFingerprint = Entry.Binding.ReviewPackageFingerprint;
		ProvenanceRecord.ProviderSchemaFingerprint = Entry.Binding.ProviderSchemaFingerprint;
		ProvenanceRecord.DesiredCanonicalFingerprint = Entry.Binding.DesiredCanonicalFingerprint;
		ProvenanceRecord.PreApplyProductFingerprint = Entry.Binding.PreApplyProductFingerprint;
		ProvenanceRecord.PostApplyReadbackFingerprint = TargetReport->PostApplyReadbackFingerprint;
		ProvenanceRecord.ManagementState = Entry.ManagementState;
		ProvenanceRecord.LifecycleState = Entry.LifecycleState;
		ProvenanceSnapshot.Records.Add(MoveTemp(ProvenanceRecord));
	}

	if (!Store.SaveProvenance(
		Request.ProvenancePath,
		ProvenanceSnapshot,
		StoreError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			StoreError.IsEmpty()
				? TEXT("Recovery provenance durable 저장이 실패했습니다.")
				: StoreError,
			Store);
	}

	OutTransaction.State = ECFContentCutoverState::Verified;
	OutTransaction.bAuthorityActivationEligible = true;
	OutTransaction.bAuthorityActivated = false;
	OutTransaction.Diagnostic = TEXT("P0-07 recovery finalize Verified. USER authority activation acceptance는 아직 적용되지 않았습니다.");
	if (!Store.SaveTransaction(OutTransaction, StoreError))
	{
		return CFContentCutoverPrivate::FailTransaction(
			OutTransaction,
			ECFContentCutoverState::RecoveryRequired,
			StoreError.IsEmpty()
				? TEXT("Recovery Verified durable transaction 저장이 실패했습니다.")
				: StoreError,
			Store);
	}
	return true;
}
