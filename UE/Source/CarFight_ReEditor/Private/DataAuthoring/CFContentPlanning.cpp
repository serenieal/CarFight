// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentPlanning.cpp
// Version: v1.1.1
// Date: 2026-10-03
// Description: CF-FQ-058 CCAS-P0-06 immutable Catalog Snapshot, snapshot-aware ChangeSet semantic preflight,
// transient Workbook preview, ReviewPackage fingerprint와 two-layer stale guard implementation입니다.
// Changelog:
// - v1.1.1: P0-07 Mid-review 연계 교정으로 ValidateApproval이 ReviewPackage payload fingerprint를 fresh 재계산해 승인 후 payload 변조를 fail-closed.
// - v1.1.0: fresh implementation review 교정으로 모든 operation의 SheetId/ContentType 및 primary/child target 종류를 공통 semantic preflight에서 fail-closed 검증.
// - v1.0.0: frozen P0-06 planning/review layer와 no-direct-DA-write boundary를 최초 구현.
// Migration:
// - Existing P0-01~05 Generic Core/Compiler/Provider는 유지하고 이 계층은 read-only compiler를 조합합니다.
// - Product ApplyReviewed, SavePackage, MarkPackageDirty, asset rename/move/delete를 호출하지 않습니다.

#include "DataAuthoring/CFContentPlanning.h"

#include "DataAuthoring/CFContentCore.h"
#include "DataAuthoring/CFDACommonPrimitives.h"

namespace CFContentPlanningPrivate
{
	/** Typed blocking/non-blocking diagnostic을 추가합니다. */
	void AddIssue(
		TArray<FCFContentValidationIssue>& OutIssues,
		const FString& Code,
		const FString& Path,
		const FString& Message,
		const bool bBlocking = true)
	{
		// 새 typed validation diagnostic입니다.
		FCFContentValidationIssue& Issue = OutIssues.AddDefaulted_GetRef();
		Issue.Code = Code;
		Issue.Path = Path;
		Issue.Message = Message;
		Issue.bBlocking = bBlocking;
	}

	/** Blocking diagnostic이 하나라도 있는지 확인합니다. */
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

	/** Stable string token을 hash payload에 추가합니다. */
	void AppendString(
		TArray<uint8>& OutBytes,
		const TCHAR* Label,
		const FString& Value)
	{
		CFDACommonPrimitives::AppendStringToken(OutBytes, Label, Value);
	}

	/** Stable bool token을 hash payload에 추가합니다. */
	void AppendBool(
		TArray<uint8>& OutBytes,
		const TCHAR* Label,
		const bool bValue)
	{
		CFDACommonPrimitives::AppendBoolToken(OutBytes, Label, bValue);
	}

	/** Stable integer token을 hash payload에 추가합니다. */
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

	/** Canonical ContentKey를 hash payload에 추가합니다. */
	void AppendContentKey(
		TArray<uint8>& OutBytes,
		const TCHAR* Label,
		const FCFContentKey& Key)
	{
		AppendString(OutBytes, Label, Key.ToStableString());
	}

	/** Canonical typed value를 hash payload에 추가합니다. */
	void AppendContentValue(
		TArray<uint8>& OutBytes,
		const FCFContentValue& Value)
	{
		AppendInt(OutBytes, TEXT("ValueType"), static_cast<int64>(Value.Type));
		AppendInt(OutBytes, TEXT("ValueState"), static_cast<int64>(Value.State));

		if (Value.State != ECFContentValueState::Value)
		{
			return;
		}

		switch (Value.Type)
		{
		case ECFContentValueType::Boolean:
			AppendBool(OutBytes, TEXT("Boolean"), Value.bBooleanValue);
			break;
		case ECFContentValueType::SignedInteger:
			AppendInt(OutBytes, TEXT("SignedInteger"), Value.SignedIntegerValue);
			break;
		case ECFContentValueType::UnsignedInteger:
			AppendString(OutBytes, TEXT("UnsignedInteger"), LexToString(Value.UnsignedIntegerValue));
			break;
		case ECFContentValueType::Float:
		case ECFContentValueType::Double:
			AppendString(
				OutBytes,
				TEXT("FloatingPoint"),
				FString::Printf(TEXT("%.17g"), Value.FloatingPointValue));
			break;
		case ECFContentValueType::Enum:
		case ECFContentValueType::NameId:
		case ECFContentValueType::String:
		case ECFContentValueType::ResourceReference:
			AppendString(OutBytes, TEXT("String"), Value.StringValue);
			break;
		case ECFContentValueType::Text:
			AppendInt(OutBytes, TEXT("TextKind"), static_cast<int64>(Value.TextValue.Kind));
			AppendString(OutBytes, TEXT("TextNamespace"), Value.TextValue.Namespace);
			AppendString(OutBytes, TEXT("TextKey"), Value.TextValue.Key);
			AppendString(OutBytes, TEXT("TextSource"), Value.TextValue.SourceString);
			break;
		case ECFContentValueType::ContentReference:
			AppendContentKey(OutBytes, TEXT("ContentReference"), Value.ContentReference);
			break;
		default:
			break;
		}
	}

	/** P0-06 typed planning metadata를 deterministic hash payload에 추가합니다. */
	void AppendPlanningMetadata(
		TArray<uint8>& OutBytes,
		const FCFContentPlanningMetadata& Planning)
	{
		AppendString(OutBytes, TEXT("VariantId"), Planning.VariantId);
		AppendString(OutBytes, TEXT("RoleId"), Planning.RoleId);
		AppendString(OutBytes, TEXT("ProductionWaveId"), Planning.ProductionWaveId);
		AppendInt(OutBytes, TEXT("Readiness"), static_cast<int64>(Planning.Readiness));

		// Stable DimensionId order의 relative intents입니다.
		TArray<const FCFContentRelativeIntent*> SortedRelativeIntents;
		for (const FCFContentRelativeIntent& RelativeIntent : Planning.RelativeIntents)
		{
			SortedRelativeIntents.Add(&RelativeIntent);
		}
		SortedRelativeIntents.Sort(
			[](const FCFContentRelativeIntent& Left, const FCFContentRelativeIntent& Right)
			{
				return Left.DimensionId < Right.DimensionId;
			});
		for (const FCFContentRelativeIntent* RelativeIntent : SortedRelativeIntents)
		{
			if (RelativeIntent != nullptr)
			{
				AppendString(OutBytes, TEXT("RelativeDimensionId"), RelativeIntent->DimensionId);
				AppendInt(OutBytes, TEXT("RelativeDirection"), static_cast<int64>(RelativeIntent->Direction));
			}
		}

		// Stable DependencyId order의 technology dependencies입니다.
		TArray<const FCFContentTechnologyDependency*> SortedDependencies;
		for (const FCFContentTechnologyDependency& Dependency : Planning.TechnologyDependencies)
		{
			SortedDependencies.Add(&Dependency);
		}
		SortedDependencies.Sort(
			[](const FCFContentTechnologyDependency& Left, const FCFContentTechnologyDependency& Right)
			{
				return Left.DependencyId < Right.DependencyId;
			});
		for (const FCFContentTechnologyDependency* Dependency : SortedDependencies)
		{
			if (Dependency != nullptr)
			{
				AppendString(OutBytes, TEXT("DependencyId"), Dependency->DependencyId);
				AppendInt(OutBytes, TEXT("DependencyState"), static_cast<int64>(Dependency->State));
				AppendString(OutBytes, TEXT("DependencyReason"), Dependency->Reason);
			}
		}

		// Stable lexical order의 shared data roles입니다.
		TArray<FString> SharedDataRoleIds = Planning.SharedDataRoleIds;
		SharedDataRoleIds.Sort();
		for (const FString& DataRoleId : SharedDataRoleIds)
		{
			AppendString(OutBytes, TEXT("SharedDataRoleId"), DataRoleId);
		}

		// Stable lexical order의 variant-owned data roles입니다.
		TArray<FString> VariantOwnedDataRoleIds = Planning.VariantOwnedDataRoleIds;
		VariantOwnedDataRoleIds.Sort();
		for (const FString& DataRoleId : VariantOwnedDataRoleIds)
		{
			AppendString(OutBytes, TEXT("VariantOwnedDataRoleId"), DataRoleId);
		}
	}

	/** Dependency edge를 stable semantic string으로 변환합니다. */
	FString MakeDependencyEdgeKey(const FCFContentDependencyEdge& Edge)
	{
		return FString::Printf(
			TEXT("%s|%s|%s"),
			*Edge.From.ToStableString(),
			*Edge.To.ToStableString(),
			*Edge.FieldPath);
	}

	/** Product current truth fingerprint를 Compiler evidence에서 구성합니다. */
	bool BuildProductStateFingerprint(
		const FCFContentCompileResult& CompileResult,
		FString& OutFingerprint,
		FString& OutError)
	{
		OutFingerprint.Reset();
		OutError.Reset();

		if (!CFDACommonPrimitives::IsCanonicalSha256Fingerprint(CompileResult.CurrentProductSemanticHash))
		{
			OutError = TEXT("Compiler result에 canonical CurrentProductSemanticHash가 없습니다.");
			return false;
		}

		// Product state canonical byte stream입니다.
		TArray<uint8> CanonicalBytes;
		AppendString(CanonicalBytes, TEXT("Protocol"), TEXT("CarFight.CCAS.ProductState/v1"));
		AppendString(CanonicalBytes, TEXT("CurrentProductSemanticHash"), CompileResult.CurrentProductSemanticHash);

		// Current provider fingerprint evidence를 stable ContentKey order로 정렬합니다.
		TArray<const FCFContentGeneratedFingerprint*> SortedCurrentFingerprints;
		for (const FCFContentGeneratedFingerprint& Fingerprint : CompileResult.GeneratedFingerprints)
		{
			if (!Fingerprint.CurrentProviderFingerprint.IsEmpty())
			{
				SortedCurrentFingerprints.Add(&Fingerprint);
			}
		}
		SortedCurrentFingerprints.Sort(
			[](const FCFContentGeneratedFingerprint& Left, const FCFContentGeneratedFingerprint& Right)
			{
				return Left.Key.ToStableString() < Right.Key.ToStableString();
			});
		for (const FCFContentGeneratedFingerprint* Fingerprint : SortedCurrentFingerprints)
		{
			if (Fingerprint != nullptr)
			{
				AppendContentKey(CanonicalBytes, TEXT("CurrentKey"), Fingerprint->Key);
				AppendString(CanonicalBytes, TEXT("CurrentProviderFingerprint"), Fingerprint->CurrentProviderFingerprint);
			}
		}

		// Current dependency/referencer graph를 stable edge order로 정렬합니다.
		TArray<FString> CurrentDependencyKeys;
		for (const FCFContentDependencyEdge& Edge : CompileResult.CurrentDependencyEdges)
		{
			CurrentDependencyKeys.Add(MakeDependencyEdgeKey(Edge));
		}
		CurrentDependencyKeys.Sort();
		for (const FString& EdgeKey : CurrentDependencyKeys)
		{
			AppendString(CanonicalBytes, TEXT("CurrentDependencyEdge"), EdgeKey);
		}

		return CFDACommonPrimitives::HashCanonicalBytes(
			CanonicalBytes,
			OutFingerprint,
			OutError);
	}

	/** CompileResult + provider/resource truth를 whole-Catalog SnapshotFingerprint로 구성합니다. */
	bool BuildSnapshotFingerprint(
		const FCFContentCatalogSnapshot& Snapshot,
		FString& OutFingerprint,
		FString& OutError)
	{
		OutFingerprint.Reset();
		OutError.Reset();

		// Whole-Catalog snapshot canonical byte stream입니다.
		TArray<uint8> CanonicalBytes;
		AppendString(CanonicalBytes, TEXT("Protocol"), TEXT("CarFight.CCAS.CatalogSnapshot/v1"));
		AppendString(CanonicalBytes, TEXT("WorkbookSourceId"), Snapshot.WorkbookSourceId);
		AppendString(CanonicalBytes, TEXT("WorkbookSemanticHash"), Snapshot.WorkbookSemanticHash);
		AppendInt(CanonicalBytes, TEXT("WorkbookSchemaVersion"), Snapshot.WorkbookSchemaVersion);
		AppendString(CanonicalBytes, TEXT("ProviderSchemaFingerprint"), Snapshot.ProviderSchemaFingerprint);
		AppendString(CanonicalBytes, TEXT("ProductStateFingerprint"), Snapshot.ProductStateFingerprint);
		AppendString(CanonicalBytes, TEXT("ResourceCatalogFingerprint"), Snapshot.ResourceCatalogFingerprint);

		// Desired dependency graph stable projection입니다.
		TArray<FString> DesiredDependencyKeys;
		for (const FCFContentDependencyEdge& Edge : Snapshot.DependencyEdges)
		{
			DesiredDependencyKeys.Add(MakeDependencyEdgeKey(Edge));
		}
		DesiredDependencyKeys.Sort();
		for (const FString& EdgeKey : DesiredDependencyKeys)
		{
			AppendString(CanonicalBytes, TEXT("DesiredDependencyEdge"), EdgeKey);
		}

		// Current dependency graph stable projection입니다.
		TArray<FString> CurrentDependencyKeys;
		for (const FCFContentDependencyEdge& Edge : Snapshot.CurrentDependencyEdges)
		{
			CurrentDependencyKeys.Add(MakeDependencyEdgeKey(Edge));
		}
		CurrentDependencyKeys.Sort();
		for (const FString& EdgeKey : CurrentDependencyKeys)
		{
			AppendString(CanonicalBytes, TEXT("CurrentDependencyEdge"), EdgeKey);
		}

		// Stable machine diagnostic summary입니다.
		TArray<FString> DiagnosticKeys;
		for (const FCFContentValidationIssue& Issue : Snapshot.ValidationSummary)
		{
			DiagnosticKeys.Add(FString::Printf(
				TEXT("%s|%s|%d"),
				*Issue.Code,
				*Issue.Path,
				Issue.bBlocking ? 1 : 0));
		}
		DiagnosticKeys.Sort();
		for (const FString& DiagnosticKey : DiagnosticKeys)
		{
			AppendString(CanonicalBytes, TEXT("Diagnostic"), DiagnosticKey);
		}

		// Drift summary를 stable ContentKey order로 고정합니다.
		TArray<const FCFContentGeneratedFingerprint*> SortedDrift;
		for (const FCFContentGeneratedFingerprint& Fingerprint : Snapshot.GeneratedDriftSummary)
		{
			SortedDrift.Add(&Fingerprint);
		}
		SortedDrift.Sort(
			[](const FCFContentGeneratedFingerprint& Left, const FCFContentGeneratedFingerprint& Right)
			{
				return Left.Key.ToStableString() < Right.Key.ToStableString();
			});
		for (const FCFContentGeneratedFingerprint* Fingerprint : SortedDrift)
		{
			if (Fingerprint == nullptr)
			{
				continue;
			}

			AppendContentKey(CanonicalBytes, TEXT("DriftKey"), Fingerprint->Key);
			AppendString(CanonicalBytes, TEXT("DriftSourceWorkbookHash"), Fingerprint->SourceWorkbookSemanticHash);
			AppendString(CanonicalBytes, TEXT("DriftDesiredCanonical"), Fingerprint->DesiredCanonicalFingerprint);
			AppendString(CanonicalBytes, TEXT("DriftCurrentProvider"), Fingerprint->CurrentProviderFingerprint);
			AppendString(CanonicalBytes, TEXT("DriftDesiredProvider"), Fingerprint->DesiredProviderFingerprint);
			AppendInt(CanonicalBytes, TEXT("DriftState"), static_cast<int64>(Fingerprint->DriftState));

			// Stable changed-path order입니다.
			TArray<FString> ChangedPaths = Fingerprint->ChangedPaths;
			ChangedPaths.Sort();
			for (const FString& ChangedPath : ChangedPaths)
			{
				AppendString(CanonicalBytes, TEXT("DriftChangedPath"), ChangedPath);
			}
		}

		return CFDACommonPrimitives::HashCanonicalBytes(
			CanonicalBytes,
			OutFingerprint,
			OutError);
	}

	/** ChangeSet operation의 stable semantic target을 반환합니다. */
	FString BuildOperationSemanticTarget(const FCFContentChangeOperation& Operation)
	{
		// Target record stable key입니다.
		const FString RecordKey = Operation.ContentKey.ToStableString();

		switch (Operation.OperationType)
		{
		case ECFContentChangeOperationType::AddRecord:
			return FString::Printf(TEXT("RecordAdd|%s"), *RecordKey);
		case ECFContentChangeOperationType::UpdateField:
		case ECFContentChangeOperationType::SetNone:
		case ECFContentChangeOperationType::SetInherit:
		case ECFContentChangeOperationType::BindResource:
			return FString::Printf(
				TEXT("Field|%s|%s|%s"),
				*Operation.SheetId,
				*RecordKey,
				*Operation.ColumnId);
		case ECFContentChangeOperationType::UpsertChild:
			return FString::Printf(
				TEXT("ChildField|%s|%s|%s|%s"),
				*Operation.SheetId,
				*RecordKey,
				*Operation.ChildItemId,
				*Operation.ColumnId);
		case ECFContentChangeOperationType::RemoveChildIntent:
			return FString::Printf(
				TEXT("ChildRemove|%s|%s|%s"),
				*Operation.SheetId,
				*RecordKey,
				*Operation.ChildItemId);
		case ECFContentChangeOperationType::AssignProfile:
			return FString::Printf(TEXT("Profile|%s|%s"), *RecordKey, *Operation.ColumnId);
		case ECFContentChangeOperationType::SetPlanningMetadata:
			return FString::Printf(TEXT("Planning|%s"), *RecordKey);
		case ECFContentChangeOperationType::RetireRecord:
		case ECFContentChangeOperationType::ReactivateRecord:
			return FString::Printf(TEXT("Lifecycle|%s"), *RecordKey);
		default:
			return FString::Printf(TEXT("Unknown|%s"), *RecordKey);
		}
	}

	/** Child remove/upsert conflict를 비교할 stable child base target을 반환합니다. */
	FString BuildChildBaseTarget(const FCFContentChangeOperation& Operation)
	{
		return FString::Printf(
			TEXT("%s|%s|%s"),
			*Operation.SheetId,
			*Operation.ContentKey.ToStableString(),
			*Operation.ChildItemId);
	}

	/** ChangeSet machine-semantic payload fingerprint를 생성합니다. */
	bool BuildChangeSetFingerprint(
		const FCFContentChangeSet& ChangeSet,
		FString& OutFingerprint,
		FString& OutError)
	{
		OutFingerprint.Reset();
		OutError.Reset();

		// ChangeSet canonical byte stream입니다.
		TArray<uint8> CanonicalBytes;
		AppendString(CanonicalBytes, TEXT("Protocol"), TEXT("CarFight.CCAS.ChangeSet/v1"));
		AppendString(CanonicalBytes, TEXT("SchemaId"), ChangeSet.SchemaId);
		AppendString(CanonicalBytes, TEXT("ChangeSetId"), ChangeSet.ChangeSetId);
		AppendString(CanonicalBytes, TEXT("BaseWorkbookSemanticHash"), ChangeSet.BaseWorkbookSemanticHash);
		AppendString(CanonicalBytes, TEXT("BaseCatalogSnapshotFingerprint"), ChangeSet.BaseCatalogSnapshotFingerprint);
		AppendInt(CanonicalBytes, TEXT("WorkbookSchemaVersion"), ChangeSet.WorkbookSchemaVersion);
		AppendString(CanonicalBytes, TEXT("Reason"), ChangeSet.Reason);
		AppendString(CanonicalBytes, TEXT("DesignIntent"), ChangeSet.DesignIntent);
		AppendString(CanonicalBytes, TEXT("ExpectedPostSemanticHash"), ChangeSet.ExpectedPostSemanticHash);

		for (int32 OperationIndex = 0; OperationIndex < ChangeSet.Operations.Num(); ++OperationIndex)
		{
			// Exact authored operation입니다.
			const FCFContentChangeOperation& Operation = ChangeSet.Operations[OperationIndex];
			AppendInt(CanonicalBytes, TEXT("OperationIndex"), OperationIndex);
			AppendInt(CanonicalBytes, TEXT("OperationType"), static_cast<int64>(Operation.OperationType));
			AppendString(CanonicalBytes, TEXT("SemanticTarget"), BuildOperationSemanticTarget(Operation));
			AppendString(CanonicalBytes, TEXT("SheetId"), Operation.SheetId);
			AppendContentKey(CanonicalBytes, TEXT("ContentKey"), Operation.ContentKey);
			AppendString(CanonicalBytes, TEXT("RowId"), Operation.RowId);
			AppendString(CanonicalBytes, TEXT("ColumnId"), Operation.ColumnId);
			AppendString(CanonicalBytes, TEXT("ChildItemId"), Operation.ChildItemId);
			AppendString(CanonicalBytes, TEXT("ChildDomainKey"), Operation.ChildDomainKey);
			AppendInt(CanonicalBytes, TEXT("ChildOrder"), Operation.ChildOrder);
			AppendString(CanonicalBytes, TEXT("ProfileId"), Operation.ProfileId);
			AppendString(CanonicalBytes, TEXT("ResourceId"), Operation.ResourceId);
			AppendString(CanonicalBytes, TEXT("PlanningFamilyId"), Operation.PlanningFamilyId);
			AppendString(CanonicalBytes, TEXT("PlanningBaseContentId"), Operation.PlanningBaseContentId);
			AppendString(CanonicalBytes, TEXT("PlanningDesignIntent"), Operation.PlanningDesignIntent);
			AppendPlanningMetadata(CanonicalBytes, Operation.PlanningMetadata);
			AppendContentValue(CanonicalBytes, Operation.Value);
		}

		return CFDACommonPrimitives::HashCanonicalBytes(
			CanonicalBytes,
			OutFingerprint,
			OutError);
	}

	/** Exact sheet descriptor를 SheetId로 찾습니다. */
	const FCFContentSheetDescriptor* FindSheet(
		const FCFContentWorkbookModel& Workbook,
		const FString& SheetId)
	{
		for (const FCFContentSheetDescriptor& Sheet : Workbook.Sheets)
		{
			if (Sheet.SheetId.Equals(SheetId, ESearchCase::CaseSensitive))
			{
				return &Sheet;
			}
		}
		return nullptr;
	}

	/** Exact top-level record의 primary sheet descriptor를 찾습니다. */
	const FCFContentSheetDescriptor* FindPrimarySheetForRecord(
		const FCFContentWorkbookModel& Workbook,
		const FCFContentRecord& Record)
	{
		for (const FCFContentSheetDescriptor& Sheet : Workbook.Sheets)
		{
			if (!Sheet.IsChildSheet()
				&& Sheet.ContentTypeId == Record.Key.ContentTypeId)
			{
				return &Sheet;
			}
		}
		return nullptr;
	}

	/** Mutable Workbook에서 exact ContentKey record index를 찾습니다. */
	int32 FindRecordIndex(
		const FCFContentWorkbookModel& Workbook,
		const FCFContentKey& Key)
	{
		for (int32 RecordIndex = 0; RecordIndex < Workbook.Records.Num(); ++RecordIndex)
		{
			if (Workbook.Records[RecordIndex].Key == Key)
			{
				return RecordIndex;
			}
		}
		return INDEX_NONE;
	}

	/** Sheet descriptor에서 exact ColumnId field를 찾습니다. */
	const FCFContentFieldDescriptor* FindField(
		const FCFContentSheetDescriptor& Sheet,
		const FString& ColumnId)
	{
		for (const FCFContentFieldDescriptor& Field : Sheet.Fields)
		{
			if (Field.ColumnId.Equals(ColumnId, ESearchCase::CaseSensitive))
			{
				return &Field;
			}
		}
		return nullptr;
	}

	/** Collection에서 exact ChildItemId index를 찾습니다. */
	int32 FindChildIndex(
		const FCFContentCollection& Collection,
		const FString& ChildItemId)
	{
		for (int32 ItemIndex = 0; ItemIndex < Collection.Items.Num(); ++ItemIndex)
		{
			if (Collection.Items[ItemIndex].ChildItemId.Equals(ChildItemId, ESearchCase::CaseSensitive))
			{
				return ItemIndex;
			}
		}
		return INDEX_NONE;
	}

	/** P0-06 operation target field가 exact CCASManaged인지 검증합니다. */
	bool ValidateManagedField(
		const FCFContentFieldDescriptor* Field,
		const FString& DiagnosticPath,
		TArray<FCFContentValidationIssue>& OutIssues)
	{
		if (Field == nullptr)
		{
			AddIssue(
				OutIssues,
				TEXT("UnknownChangeColumnId"),
				DiagnosticPath,
				TEXT("Change Set target ColumnId가 frozen schema에 없습니다."));
			return false;
		}
		if (Field->Ownership != ECFContentFieldOwnership::CCASManaged)
		{
			AddIssue(
				OutIssues,
				TEXT("NonCCASManagedFieldMutation"),
				DiagnosticPath,
				TEXT("P0-06 Change Set은 CCASManaged field만 mutation할 수 있습니다."));
			return false;
		}
		return true;
	}

	/** Touched record rationale가 exact1이고 모든 operation index를 설명하는지 검증합니다. */
	bool ValidateRationales(
		const FCFContentChangeProposal& Proposal,
		TArray<FCFContentValidationIssue>& OutIssues)
	{
		// ChangeSet에서 실제 touched record key 집합입니다.
		TSet<FString> TouchedRecordKeys;
		for (const FCFContentChangeOperation& Operation : Proposal.ChangeSet.Operations)
		{
			TouchedRecordKeys.Add(Operation.ContentKey.ToStableString());
		}

		// Rationale duplicate 검사용 record key 집합입니다.
		TSet<FString> RationaleRecordKeys;
		// Rationale가 실제로 설명한 operation indexes입니다.
		TSet<int32> CoveredOperationIndexes;
		for (int32 RationaleIndex = 0; RationaleIndex < Proposal.RecordRationales.Num(); ++RationaleIndex)
		{
			// Exact rationale입니다.
			const FCFContentRecordRationale& Rationale = Proposal.RecordRationales[RationaleIndex];
			// Rationale diagnostic path입니다.
			const FString RationalePath = FString::Printf(TEXT("RecordRationales[%d]"), RationaleIndex);
			// Rationale target stable key입니다.
			const FString RecordKey = Rationale.ContentKey.ToStableString();

			if (!Rationale.ContentKey.IsValid()
				|| !TouchedRecordKeys.Contains(RecordKey))
			{
				AddIssue(
					OutIssues,
					TEXT("InvalidRationaleTarget"),
					RationalePath,
					TEXT("Rationale는 Change Set이 실제로 touch하는 valid ContentKey exact1만 대상으로 해야 합니다."));
			}
			if (RationaleRecordKeys.Contains(RecordKey))
			{
				AddIssue(
					OutIssues,
					TEXT("DuplicateRecordRationale"),
					RationalePath,
					TEXT("Touched record 하나에는 rationale exact1만 허용됩니다."));
			}
			RationaleRecordKeys.Add(RecordKey);

			if (Rationale.Reason.TrimStartAndEnd().IsEmpty())
			{
				AddIssue(
					OutIssues,
					TEXT("MissingRecordRationaleReason"),
					RationalePath,
					TEXT("Touched record rationale에는 사용자-facing 변경 이유가 필요합니다."));
			}
			if (Rationale.RelatedOperationIndexes.IsEmpty())
			{
				AddIssue(
					OutIssues,
					TEXT("EmptyRationaleOperationSet"),
					RationalePath,
					TEXT("Rationale는 최소 하나의 exact Change Set operation을 설명해야 합니다."));
			}

			for (const int32 OperationIndex : Rationale.RelatedOperationIndexes)
			{
				if (!Proposal.ChangeSet.Operations.IsValidIndex(OperationIndex))
				{
					AddIssue(
						OutIssues,
						TEXT("InvalidRationaleOperationIndex"),
						RationalePath,
						TEXT("Rationale가 존재하지 않는 Change Set operation index를 가리킵니다."));
					continue;
				}

				// Rationale가 설명해야 할 exact operation입니다.
				const FCFContentChangeOperation& Operation = Proposal.ChangeSet.Operations[OperationIndex];
				if (!(Operation.ContentKey == Rationale.ContentKey))
				{
					AddIssue(
						OutIssues,
						TEXT("RationaleOperationTargetMismatch"),
						RationalePath,
						TEXT("Rationale operation index의 ContentKey가 rationale target과 다릅니다."));
				}
				if (CoveredOperationIndexes.Contains(OperationIndex))
				{
					AddIssue(
						OutIssues,
						TEXT("DuplicateRationaleOperationCoverage"),
						RationalePath,
						TEXT("Change Set operation 하나를 여러 rationale가 중복 소유할 수 없습니다."));
				}
				CoveredOperationIndexes.Add(OperationIndex);
			}
		}

		for (const FString& TouchedRecordKey : TouchedRecordKeys)
		{
			if (!RationaleRecordKeys.Contains(TouchedRecordKey))
			{
				AddIssue(
					OutIssues,
					TEXT("MissingRecordRationale"),
					TouchedRecordKey,
					TEXT("Touched record마다 rationale exact1이 필요합니다."));
			}
		}

		for (int32 OperationIndex = 0; OperationIndex < Proposal.ChangeSet.Operations.Num(); ++OperationIndex)
		{
			if (!CoveredOperationIndexes.Contains(OperationIndex))
			{
				AddIssue(
					OutIssues,
					TEXT("UnexplainedChangeOperation"),
					FString::Printf(TEXT("ChangeSet.Operations[%d]"), OperationIndex),
					TEXT("모든 Change Set operation은 touched record rationale exact1에 포함되어야 합니다."));
			}
		}

		return !HasBlockingIssues(OutIssues);
	}

	/** Record 하나의 operation conflict 상태입니다. */
	struct FRecordOperationFlags
	{
		// 같은 ChangeSet에서 AddRecord가 존재하는지 여부입니다.
		bool bAdd = false;

		// 같은 ChangeSet에서 field/profile/planning/child mutation이 존재하는지 여부입니다.
		bool bMutation = false;

		// 같은 ChangeSet에서 RetireRecord가 존재하는지 여부입니다.
		bool bRetire = false;

		// 같은 ChangeSet에서 ReactivateRecord가 존재하는지 여부입니다.
		bool bReactivate = false;
	};

	/** ChangeSet semantic conflicts와 target existence/ownership을 mutation 전에 검증합니다. */
	bool ValidateSemanticPreflight(
		const FCFContentCatalogSnapshot& BaseSnapshot,
		const FCFContentChangeProposal& Proposal,
		TArray<FCFContentValidationIssue>& OutIssues)
	{
		// Existing Core shape validation diagnostics입니다.
		TArray<FCFContentValidationIssue> ShapeIssues;
		if (!FCFContentSchemaValidator::ValidateChangeSet(Proposal.ChangeSet, ShapeIssues))
		{
			OutIssues.Append(ShapeIssues);
		}
		else
		{
			OutIssues.Append(ShapeIssues);
		}

		if (!CFDACommonPrimitives::IsCanonicalSha256Fingerprint(Proposal.ChangeSet.BaseCatalogSnapshotFingerprint))
		{
			AddIssue(
				OutIssues,
				TEXT("MissingBaseCatalogSnapshotFingerprint"),
				TEXT("ChangeSet.BaseCatalogSnapshotFingerprint"),
				TEXT("P0-06 Change Set은 canonical BaseCatalogSnapshotFingerprint에 반드시 결속되어야 합니다."));
		}
		if (!Proposal.BaseCatalogSnapshotFingerprint.Equals(BaseSnapshot.SnapshotFingerprint, ESearchCase::CaseSensitive)
			|| !Proposal.ChangeSet.BaseCatalogSnapshotFingerprint.Equals(BaseSnapshot.SnapshotFingerprint, ESearchCase::CaseSensitive))
		{
			AddIssue(
				OutIssues,
				TEXT("StalePlanningSnapshot"),
				TEXT("ChangeSet.BaseCatalogSnapshotFingerprint"),
				TEXT("Change Set/Proposal의 base Catalog Snapshot이 current review base와 다릅니다."));
		}
		if (!Proposal.ChangeSet.BaseWorkbookSemanticHash.Equals(BaseSnapshot.WorkbookSemanticHash, ESearchCase::CaseSensitive))
		{
			AddIssue(
				OutIssues,
				TEXT("StaleWorkbook"),
				TEXT("ChangeSet.BaseWorkbookSemanticHash"),
				TEXT("Change Set의 BaseWorkbookSemanticHash가 snapshot Workbook과 다릅니다."));
		}
		if (Proposal.ChangeSet.WorkbookSchemaVersion != BaseSnapshot.WorkbookSchemaVersion)
		{
			AddIssue(
				OutIssues,
				TEXT("ChangeSetSchemaRevisionMismatch"),
				TEXT("ChangeSet.WorkbookSchemaVersion"),
				TEXT("Change Set WorkbookSchemaVersion이 snapshot schema revision과 다릅니다."));
		}

		ValidateRationales(Proposal, OutIssues);

		// Base snapshot record keys입니다.
		TSet<FString> ExistingRecordKeys;
		for (const FCFContentRecord& Record : BaseSnapshot.Workbook.Records)
		{
			ExistingRecordKeys.Add(Record.Key.ToStableString());
		}

		// 이번 ChangeSet이 새로 만드는 record keys입니다. authored operation 순서와 무관하게 먼저 수집합니다.
		TSet<FString> AddedRecordKeys;
		for (int32 OperationIndex = 0; OperationIndex < Proposal.ChangeSet.Operations.Num(); ++OperationIndex)
		{
			// Pre-scan 대상 operation입니다.
			const FCFContentChangeOperation& Operation = Proposal.ChangeSet.Operations[OperationIndex];
			if (Operation.OperationType != ECFContentChangeOperationType::AddRecord)
			{
				continue;
			}

			// AddRecord target stable key입니다.
			const FString RecordKey = Operation.ContentKey.ToStableString();
			// Operation diagnostic path입니다.
			const FString OperationPath = FString::Printf(TEXT("ChangeSet.Operations[%d]"), OperationIndex);
			if (ExistingRecordKeys.Contains(RecordKey))
			{
				AddIssue(OutIssues, TEXT("ExistingRecordAdd"), OperationPath, TEXT("이미 존재하는 ContentKey에 AddRecord를 사용할 수 없습니다."));
			}
			if (AddedRecordKeys.Contains(RecordKey))
			{
				AddIssue(OutIssues, TEXT("DuplicateRecordAdd"), OperationPath, TEXT("같은 ContentKey의 AddRecord exact2 이상은 허용되지 않습니다."));
			}
			AddedRecordKeys.Add(RecordKey);
		}

		// Exact semantic target duplicate 검사용 set입니다.
		TSet<FString> SeenSemanticTargets;
		// Child remove target conflict 검사용 set입니다.
		TSet<FString> RemovedChildTargets;
		// Child upsert target conflict 검사용 set입니다.
		TSet<FString> UpsertedChildTargets;
		// Record별 conflict flags입니다.
		TMap<FString, FRecordOperationFlags> FlagsByRecord;

		for (int32 OperationIndex = 0; OperationIndex < Proposal.ChangeSet.Operations.Num(); ++OperationIndex)
		{
			// Semantic preflight 대상 operation입니다.
			const FCFContentChangeOperation& Operation = Proposal.ChangeSet.Operations[OperationIndex];
			// Operation diagnostic path입니다.
			const FString OperationPath = FString::Printf(TEXT("ChangeSet.Operations[%d]"), OperationIndex);
			// Operation target stable record key입니다.
			const FString RecordKey = Operation.ContentKey.ToStableString();
			// Operation exact semantic target입니다.
			const FString SemanticTarget = BuildOperationSemanticTarget(Operation);
			// Record conflict flags입니다.
			FRecordOperationFlags& RecordFlags = FlagsByRecord.FindOrAdd(RecordKey);

			// 모든 operation은 frozen Workbook schema의 exact SheetId + ContentType target에 결속되어야 합니다.
			const FCFContentSheetDescriptor* TargetSheet = FindSheet(BaseSnapshot.Workbook, Operation.SheetId);
			if (TargetSheet == nullptr)
			{
				AddIssue(
					OutIssues,
					TEXT("UnknownChangeSheetId"),
					OperationPath,
					TEXT("Change Set operation의 SheetId가 frozen Workbook schema에 없습니다."));
			}
			else
			{
				// Child semantic operation인지 여부입니다.
				const bool bChildOperation =
					Operation.OperationType == ECFContentChangeOperationType::UpsertChild
					|| Operation.OperationType == ECFContentChangeOperationType::RemoveChildIntent;
				if (!(TargetSheet->ContentTypeId == Operation.ContentKey.ContentTypeId))
				{
					AddIssue(
						OutIssues,
						TEXT("ChangeSheetContentTypeMismatch"),
						OperationPath,
						TEXT("Change Set SheetId의 ContentType과 target ContentKey의 ContentType이 다릅니다."));
				}
				if (bChildOperation && !TargetSheet->IsChildSheet())
				{
					AddIssue(
						OutIssues,
						TEXT("ExpectedChildChangeSheet"),
						OperationPath,
						TEXT("Child operation은 registered child SheetId를 사용해야 합니다."));
				}
				if (!bChildOperation && TargetSheet->IsChildSheet())
				{
					AddIssue(
						OutIssues,
						TEXT("ExpectedPrimaryChangeSheet"),
						OperationPath,
						TEXT("Top-level/planning/lifecycle operation은 primary SheetId를 사용해야 합니다."));
				}
			}

			if (SeenSemanticTargets.Contains(SemanticTarget))
			{
				AddIssue(
					OutIssues,
					TEXT("DuplicateSemanticTarget"),
					OperationPath,
					TEXT("P0-06에서는 같은 semantic target operation exact2 이상을 자동 dedupe하지 않습니다."));
			}
			SeenSemanticTargets.Add(SemanticTarget);

			if (Operation.OperationType == ECFContentChangeOperationType::AddRecord)
			{
				RecordFlags.bAdd = true;
			}
			else if (!ExistingRecordKeys.Contains(RecordKey)
				&& !AddedRecordKeys.Contains(RecordKey))
			{
				AddIssue(
					OutIssues,
					TEXT("MissingChangeRecord"),
					OperationPath,
					TEXT("AddRecord로 생성되지 않은 missing record를 update/retire/reactivate할 수 없습니다."));
			}

			// Existing Product/Catalog record의 mutation protection입니다.
			const int32 BaseRecordIndex = FindRecordIndex(BaseSnapshot.Workbook, Operation.ContentKey);
			if (BaseSnapshot.Workbook.Records.IsValidIndex(BaseRecordIndex))
			{
				// Exact base record입니다.
				const FCFContentRecord& BaseRecord = BaseSnapshot.Workbook.Records[BaseRecordIndex];
				if (BaseRecord.ManagementState != ECFContentManagementState::Managed)
				{
					AddIssue(
						OutIssues,
						TEXT("ExternalReadOnlyMutation"),
						OperationPath,
						TEXT("ExternalReadOnly record는 P0-06 Change Set mutation 대상이 아닙니다."));
				}
			}

			switch (Operation.OperationType)
			{
			case ECFContentChangeOperationType::UpdateField:
			case ECFContentChangeOperationType::SetNone:
			case ECFContentChangeOperationType::SetInherit:
			case ECFContentChangeOperationType::UpsertChild:
			case ECFContentChangeOperationType::RemoveChildIntent:
			case ECFContentChangeOperationType::AssignProfile:
			case ECFContentChangeOperationType::SetPlanningMetadata:
			case ECFContentChangeOperationType::BindResource:
				RecordFlags.bMutation = true;
				break;
			case ECFContentChangeOperationType::RetireRecord:
				RecordFlags.bRetire = true;
				break;
			case ECFContentChangeOperationType::ReactivateRecord:
				RecordFlags.bReactivate = true;
				break;
			default:
				break;
			}

			if (Operation.OperationType == ECFContentChangeOperationType::UpsertChild)
			{
				// Remove/Upsert child conflict key입니다.
				const FString ChildBaseTarget = BuildChildBaseTarget(Operation);
				if (RemovedChildTargets.Contains(ChildBaseTarget))
				{
					AddIssue(
						OutIssues,
						TEXT("RemoveUpsertChildConflict"),
						OperationPath,
						TEXT("같은 child item을 한 Change Set에서 Remove와 Upsert할 수 없습니다."));
				}
				UpsertedChildTargets.Add(ChildBaseTarget);
			}
			else if (Operation.OperationType == ECFContentChangeOperationType::RemoveChildIntent)
			{
				// Remove/Upsert child conflict key입니다.
				const FString ChildBaseTarget = BuildChildBaseTarget(Operation);
				if (UpsertedChildTargets.Contains(ChildBaseTarget))
				{
					AddIssue(
						OutIssues,
						TEXT("RemoveUpsertChildConflict"),
						OperationPath,
						TEXT("같은 child item을 한 Change Set에서 Remove와 Upsert할 수 없습니다."));
				}
				RemovedChildTargets.Add(ChildBaseTarget);
			}
		}

		for (const TPair<FString, FRecordOperationFlags>& Pair : FlagsByRecord)
		{
			// Record-level operation flags입니다.
			const FRecordOperationFlags& Flags = Pair.Value;
			if (Flags.bAdd && Flags.bRetire)
			{
				AddIssue(
					OutIssues,
					TEXT("AddRetireConflict"),
					Pair.Key,
					TEXT("같은 record를 한 Change Set에서 Add와 Retire할 수 없습니다."));
			}
			if (Flags.bAdd && Flags.bReactivate)
			{
				AddIssue(
					OutIssues,
					TEXT("AddReactivateConflict"),
					Pair.Key,
					TEXT("새 record에 ReactivateRecord를 사용할 수 없습니다."));
			}
			if (Flags.bRetire && Flags.bMutation)
			{
				AddIssue(
					OutIssues,
					TEXT("RetireMutationConflict"),
					Pair.Key,
					TEXT("RetireRecord와 field/profile/planning/child mutation을 같은 Change Set에서 혼합할 수 없습니다."));
			}
			if (Flags.bRetire && Flags.bReactivate)
			{
				AddIssue(
					OutIssues,
					TEXT("RetireReactivateConflict"),
					Pair.Key,
					TEXT("RetireRecord와 ReactivateRecord를 같은 Change Set에서 함께 사용할 수 없습니다."));
			}

			// Base record가 이미 Retired인 경우 Reactivate 없이 payload/planning mutation을 금지합니다.
			for (const FCFContentRecord& BaseRecord : BaseSnapshot.Workbook.Records)
			{
				if (BaseRecord.Key.ToStableString().Equals(Pair.Key, ESearchCase::CaseSensitive)
					&& BaseRecord.LifecycleState == ECFContentLifecycleState::Retired
					&& Flags.bMutation
					&& !Flags.bReactivate)
				{
					AddIssue(
						OutIssues,
						TEXT("RetiredRecordMutation"),
						Pair.Key,
						TEXT("이미 Retired인 record는 Reactivate 없이 field/planning mutation할 수 없습니다."));
				}
			}
		}

		return !HasBlockingIssues(OutIssues);
	}

	/** Field value operation을 mutable record에 적용합니다. */
	bool ApplyTopLevelFieldOperation(
		const FCFContentChangeOperation& Operation,
		const FCFContentSheetDescriptor& Sheet,
		FCFContentRecord& Record,
		TArray<FCFContentValidationIssue>& OutIssues,
		const FString& OperationPath)
	{
		// Exact target field descriptor입니다.
		const FCFContentFieldDescriptor* Field = FindField(Sheet, Operation.ColumnId);
		if (!ValidateManagedField(Field, OperationPath, OutIssues))
		{
			return false;
		}

		// Operation 결과로 저장할 canonical value입니다.
		FCFContentValue NewValue;
		if (Field != nullptr)
		{
			NewValue.Type = Field->ValueType;
		}

		switch (Operation.OperationType)
		{
		case ECFContentChangeOperationType::UpdateField:
			NewValue = Operation.Value;
			break;
		case ECFContentChangeOperationType::SetNone:
			NewValue.State = ECFContentValueState::None;
			break;
		case ECFContentChangeOperationType::SetInherit:
			NewValue.State = ECFContentValueState::Inherit;
			break;
		case ECFContentChangeOperationType::BindResource:
			if (Field == nullptr || Field->ValueType != ECFContentValueType::ResourceReference)
			{
				AddIssue(
					OutIssues,
					TEXT("BindResourceTypeMismatch"),
					OperationPath,
					TEXT("BindResource target은 ResourceReference typed field여야 합니다."));
				return false;
			}
			NewValue.State = ECFContentValueState::Value;
			NewValue.StringValue = Operation.ResourceId;
			break;
		default:
			AddIssue(
				OutIssues,
				TEXT("InvalidTopLevelFieldOperation"),
				OperationPath,
				TEXT("Top-level field helper에 지원되지 않는 operation이 전달됐습니다."));
			return false;
		}

		Record.Fields.Add(Operation.ColumnId, NewValue);
		return true;
	}

	/** Child collection upsert operation을 mutable record에 적용합니다. */
	bool ApplyChildUpsert(
		const FCFContentChangeOperation& Operation,
		const FCFContentSheetDescriptor& ChildSheet,
		FCFContentRecord& Record,
		TArray<FCFContentValidationIssue>& OutIssues,
		const FString& OperationPath)
	{
		// Exact child field descriptor입니다.
		const FCFContentFieldDescriptor* Field = FindField(ChildSheet, Operation.ColumnId);
		if (!ValidateManagedField(Field, OperationPath, OutIssues))
		{
			return false;
		}

		// Parent record의 target child collection입니다.
		FCFContentCollection* Collection = Record.Collections.Find(ChildSheet.CollectionId);
		if (Collection == nullptr)
		{
			// 새 collection container입니다.
			FCFContentCollection NewCollection;
			NewCollection.CollectionId = ChildSheet.CollectionId;
			NewCollection.Kind = ChildSheet.CollectionKind;
			Record.Collections.Add(ChildSheet.CollectionId, NewCollection);
			Collection = Record.Collections.Find(ChildSheet.CollectionId);
		}
		if (Collection == nullptr)
		{
			AddIssue(
				OutIssues,
				TEXT("ChildCollectionCreateFailed"),
				OperationPath,
				TEXT("Transient Workbook에 child collection을 생성하지 못했습니다."));
			return false;
		}
		if (Collection->Kind != ChildSheet.CollectionKind)
		{
			AddIssue(
				OutIssues,
				TEXT("ChildCollectionKindMismatch"),
				OperationPath,
				TEXT("Existing child collection kind와 frozen child sheet schema가 다릅니다."));
			return false;
		}

		// Existing child item index입니다.
		int32 ChildIndex = FindChildIndex(*Collection, Operation.ChildItemId);
		if (ChildIndex == INDEX_NONE)
		{
			// 새 child item입니다.
			FCFContentCollectionItem NewItem;
			NewItem.ChildItemId = Operation.ChildItemId;
			NewItem.DomainKey = Operation.ChildDomainKey;
			NewItem.Order = Operation.ChildOrder;

			if (ChildSheet.CollectionKind == ECFContentCollectionKind::KeyedCollection
				&& !CFIsStableContentId(NewItem.DomainKey))
			{
				AddIssue(
					OutIssues,
					TEXT("MissingChildDomainKey"),
					OperationPath,
					TEXT("새 KeyedCollection child에는 stable ChildDomainKey가 필요합니다."));
				return false;
			}
			if (ChildSheet.CollectionKind == ECFContentCollectionKind::OrderedList
				&& NewItem.Order < 0)
			{
				AddIssue(
					OutIssues,
					TEXT("MissingChildOrder"),
					OperationPath,
					TEXT("새 OrderedList child에는 0 이상의 explicit ChildOrder가 필요합니다."));
				return false;
			}

			Collection->Items.Add(NewItem);
			ChildIndex = Collection->Items.Num() - 1;
		}

		if (!Collection->Items.IsValidIndex(ChildIndex))
		{
			AddIssue(
				OutIssues,
				TEXT("InvalidChildIndex"),
				OperationPath,
				TEXT("Transient child item index가 유효하지 않습니다."));
			return false;
		}

		// Upsert할 exact child item입니다.
		FCFContentCollectionItem& ChildItem = Collection->Items[ChildIndex];
		ChildItem.Fields.Add(Operation.ColumnId, Operation.Value);
		return true;
	}

	/** Child removal intent를 mutable record에 적용합니다. */
	bool ApplyChildRemoval(
		const FCFContentChangeOperation& Operation,
		const FCFContentSheetDescriptor& ChildSheet,
		FCFContentRecord& Record,
		TArray<FCFContentValidationIssue>& OutIssues,
		const FString& OperationPath)
	{
		// Parent record의 target child collection입니다.
		FCFContentCollection* Collection = Record.Collections.Find(ChildSheet.CollectionId);
		if (Collection == nullptr)
		{
			AddIssue(
				OutIssues,
				TEXT("MissingChildCollection"),
				OperationPath,
				TEXT("RemoveChildIntent target collection이 존재하지 않습니다."));
			return false;
		}

		// Existing child item index입니다.
		const int32 ChildIndex = FindChildIndex(*Collection, Operation.ChildItemId);
		if (!Collection->Items.IsValidIndex(ChildIndex))
		{
			AddIssue(
				OutIssues,
				TEXT("MissingChildRemoveTarget"),
				OperationPath,
				TEXT("RemoveChildIntent target ChildItemId가 존재하지 않습니다."));
			return false;
		}

		Collection->Items.RemoveAt(ChildIndex);
		return true;
	}

	/** Snapshot base에 ChangeSet을 deterministic phase/order로 적용해 transient post Workbook을 만듭니다. */
	bool BuildPostWorkbook(
		const FCFContentCatalogSnapshot& BaseSnapshot,
		const FCFContentChangeProposal& Proposal,
		FCFContentWorkbookModel& OutPostWorkbook,
		TArray<FCFContentReviewChange>& OutPlanningChanges,
		TArray<FCFContentValidationIssue>& OutIssues)
	{
		OutPostWorkbook = BaseSnapshot.Workbook;
		OutPlanningChanges.Reset();

		if (!ValidateSemanticPreflight(BaseSnapshot, Proposal, OutIssues))
		{
			return false;
		}

		// Deterministic application order용 operation indexes입니다.
		TArray<int32> AddOperationIndexes;
		// 일반 payload/planning/child mutation indexes입니다.
		TArray<int32> MutationOperationIndexes;
		// Lifecycle mutation indexes입니다.
		TArray<int32> LifecycleOperationIndexes;

		for (int32 OperationIndex = 0; OperationIndex < Proposal.ChangeSet.Operations.Num(); ++OperationIndex)
		{
			// Exact operation입니다.
			const FCFContentChangeOperation& Operation = Proposal.ChangeSet.Operations[OperationIndex];
			if (Operation.OperationType == ECFContentChangeOperationType::AddRecord)
			{
				AddOperationIndexes.Add(OperationIndex);
			}
			else if (Operation.OperationType == ECFContentChangeOperationType::RetireRecord
				|| Operation.OperationType == ECFContentChangeOperationType::ReactivateRecord)
			{
				LifecycleOperationIndexes.Add(OperationIndex);
			}
			else
			{
				MutationOperationIndexes.Add(OperationIndex);
			}
		}

		// Semantic target order로 stable sort합니다.
		auto SortOperationIndexes =
			[&Proposal](TArray<int32>& OperationIndexes)
			{
				OperationIndexes.Sort(
					[&Proposal](const int32 LeftIndex, const int32 RightIndex)
					{
						return BuildOperationSemanticTarget(Proposal.ChangeSet.Operations[LeftIndex])
							< BuildOperationSemanticTarget(Proposal.ChangeSet.Operations[RightIndex]);
					});
			};
		SortOperationIndexes(AddOperationIndexes);
		SortOperationIndexes(MutationOperationIndexes);
		SortOperationIndexes(LifecycleOperationIndexes);

		// AddRecord를 먼저 적용합니다.
		for (const int32 OperationIndex : AddOperationIndexes)
		{
			// Exact AddRecord operation입니다.
			const FCFContentChangeOperation& Operation = Proposal.ChangeSet.Operations[OperationIndex];
			// AddRecord target primary sheet입니다.
			const FCFContentSheetDescriptor* Sheet = FindSheet(OutPostWorkbook, Operation.SheetId);
			// Operation diagnostic path입니다.
			const FString OperationPath = FString::Printf(TEXT("ChangeSet.Operations[%d]"), OperationIndex);

			if (Sheet == nullptr
				|| Sheet->IsChildSheet()
				|| !(Sheet->ContentTypeId == Operation.ContentKey.ContentTypeId))
			{
				AddIssue(
					OutIssues,
					TEXT("InvalidAddRecordSheet"),
					OperationPath,
					TEXT("AddRecord는 target ContentType의 primary SheetId를 사용해야 합니다."));
				continue;
			}

			// 새 managed/active canonical record입니다.
			FCFContentRecord NewRecord;
			NewRecord.Key = Operation.ContentKey;
			NewRecord.RowId = Operation.ContentKey.ToStableString();
			NewRecord.ManagementState = ECFContentManagementState::Managed;
			NewRecord.LifecycleState = ECFContentLifecycleState::Active;
			OutPostWorkbook.Records.Add(NewRecord);
		}

		// Field/profile/planning/child mutation을 두 번째 phase에서 적용합니다.
		for (const int32 OperationIndex : MutationOperationIndexes)
		{
			// Exact mutation operation입니다.
			const FCFContentChangeOperation& Operation = Proposal.ChangeSet.Operations[OperationIndex];
			// Operation diagnostic path입니다.
			const FString OperationPath = FString::Printf(TEXT("ChangeSet.Operations[%d]"), OperationIndex);
			// Mutable target record index입니다.
			const int32 RecordIndex = FindRecordIndex(OutPostWorkbook, Operation.ContentKey);
			if (!OutPostWorkbook.Records.IsValidIndex(RecordIndex))
			{
				AddIssue(
					OutIssues,
					TEXT("MissingTransientRecord"),
					OperationPath,
					TEXT("Semantic preflight 뒤 transient target record를 찾지 못했습니다."));
				continue;
			}

			// Mutable target record입니다.
			FCFContentRecord& Record = OutPostWorkbook.Records[RecordIndex];
			if (Record.ManagementState != ECFContentManagementState::Managed)
			{
				AddIssue(
					OutIssues,
					TEXT("ExternalReadOnlyMutation"),
					OperationPath,
					TEXT("ExternalReadOnly record는 transient Change Set mutation 대상이 아닙니다."));
				continue;
			}

			switch (Operation.OperationType)
			{
			case ECFContentChangeOperationType::UpdateField:
			case ECFContentChangeOperationType::SetNone:
			case ECFContentChangeOperationType::SetInherit:
			case ECFContentChangeOperationType::BindResource:
			{
				// Exact primary sheet입니다.
				const FCFContentSheetDescriptor* Sheet = FindSheet(OutPostWorkbook, Operation.SheetId);
				if (Sheet == nullptr
					|| Sheet->IsChildSheet()
					|| !(Sheet->ContentTypeId == Record.Key.ContentTypeId))
				{
					AddIssue(
						OutIssues,
						TEXT("InvalidFieldSheet"),
						OperationPath,
						TEXT("Top-level field operation은 target ContentType의 primary sheet를 사용해야 합니다."));
					break;
				}
				ApplyTopLevelFieldOperation(Operation, *Sheet, Record, OutIssues, OperationPath);
				break;
			}
			case ECFContentChangeOperationType::AssignProfile:
			{
				if (!CFIsStableContentId(Operation.ColumnId))
				{
					AddIssue(
						OutIssues,
						TEXT("MissingProfileDomainId"),
						OperationPath,
						TEXT("AssignProfile은 ColumnId slot에 stable FieldDomainId를 제공해야 합니다."));
					break;
				}

				// Record primary sheet입니다.
				const FCFContentSheetDescriptor* PrimarySheet = FindPrimarySheetForRecord(OutPostWorkbook, Record);
				// FieldDomainId가 실제 schema에 존재하는지 여부입니다.
				bool bDomainExists = false;
				if (PrimarySheet != nullptr)
				{
					for (const FCFContentFieldDescriptor& Field : PrimarySheet->Fields)
					{
						if (Field.FieldDomainId.Equals(Operation.ColumnId, ESearchCase::CaseSensitive))
						{
							bDomainExists = true;
							break;
						}
					}
				}
				if (!bDomainExists)
				{
					AddIssue(
						OutIssues,
						TEXT("UnknownProfileDomain"),
						OperationPath,
						TEXT("AssignProfile FieldDomainId가 target provider schema에 없습니다."));
					break;
				}

				Record.AuthoringMetadata.AssignedProfileIdsByDomain.Add(
					Operation.ColumnId,
					Operation.ProfileId);
				break;
			}
			case ECFContentChangeOperationType::SetPlanningMetadata:
			{
				if (!Operation.PlanningFamilyId.IsEmpty()
					&& !CFIsStableContentId(Operation.PlanningFamilyId))
				{
					AddIssue(
						OutIssues,
						TEXT("InvalidPlanningFamilyId"),
						OperationPath,
						TEXT("Planning FamilyId는 비어 있거나 canonical stable ID여야 합니다."));
					break;
				}
				if (!Operation.PlanningBaseContentId.IsEmpty()
					&& !CFIsStableContentId(Operation.PlanningBaseContentId))
				{
					AddIssue(
						OutIssues,
						TEXT("InvalidPlanningBaseContentId"),
						OperationPath,
						TEXT("Planning BaseContentId는 비어 있거나 canonical stable ID여야 합니다."));
					break;
				}
				Record.AuthoringMetadata.FamilyId = Operation.PlanningFamilyId;
				Record.AuthoringMetadata.BaseContentId = Operation.PlanningBaseContentId;
				Record.AuthoringMetadata.DesignIntent = Operation.PlanningDesignIntent;
				Record.AuthoringMetadata.Planning = Operation.PlanningMetadata;
				break;
			}
			case ECFContentChangeOperationType::UpsertChild:
			{
				// Exact child sheet입니다.
				const FCFContentSheetDescriptor* ChildSheet = FindSheet(OutPostWorkbook, Operation.SheetId);
				if (ChildSheet == nullptr
					|| !ChildSheet->IsChildSheet()
					|| !(ChildSheet->ContentTypeId == Record.Key.ContentTypeId))
				{
					AddIssue(
						OutIssues,
						TEXT("InvalidChildSheet"),
						OperationPath,
						TEXT("UpsertChild는 target ContentType의 registered child SheetId를 사용해야 합니다."));
					break;
				}
				if (!CFIsStableContentId(Operation.ColumnId))
				{
					AddIssue(
						OutIssues,
						TEXT("MissingChildColumnId"),
						OperationPath,
						TEXT("UpsertChild는 stable child ColumnId가 필요합니다."));
					break;
				}
				ApplyChildUpsert(Operation, *ChildSheet, Record, OutIssues, OperationPath);
				break;
			}
			case ECFContentChangeOperationType::RemoveChildIntent:
			{
				// Exact child sheet입니다.
				const FCFContentSheetDescriptor* ChildSheet = FindSheet(OutPostWorkbook, Operation.SheetId);
				if (ChildSheet == nullptr
					|| !ChildSheet->IsChildSheet()
					|| !(ChildSheet->ContentTypeId == Record.Key.ContentTypeId))
				{
					AddIssue(
						OutIssues,
						TEXT("InvalidChildSheet"),
						OperationPath,
						TEXT("RemoveChildIntent는 target ContentType의 registered child SheetId를 사용해야 합니다."));
					break;
				}
				ApplyChildRemoval(Operation, *ChildSheet, Record, OutIssues, OperationPath);
				break;
			}
			default:
				break;
			}
		}

		// Lifecycle mutation을 마지막 phase에서 적용합니다.
		for (const int32 OperationIndex : LifecycleOperationIndexes)
		{
			// Exact lifecycle operation입니다.
			const FCFContentChangeOperation& Operation = Proposal.ChangeSet.Operations[OperationIndex];
			// Operation diagnostic path입니다.
			const FString OperationPath = FString::Printf(TEXT("ChangeSet.Operations[%d]"), OperationIndex);
			// Mutable target record index입니다.
			const int32 RecordIndex = FindRecordIndex(OutPostWorkbook, Operation.ContentKey);
			if (!OutPostWorkbook.Records.IsValidIndex(RecordIndex))
			{
				AddIssue(
					OutIssues,
					TEXT("MissingLifecycleRecord"),
					OperationPath,
					TEXT("Lifecycle target record가 존재하지 않습니다."));
				continue;
			}

			// Mutable lifecycle target record입니다.
			FCFContentRecord& Record = OutPostWorkbook.Records[RecordIndex];
			if (Operation.OperationType == ECFContentChangeOperationType::RetireRecord)
			{
				if (Record.LifecycleState == ECFContentLifecycleState::Retired)
				{
					AddIssue(
						OutIssues,
						TEXT("AlreadyRetiredRecord"),
						OperationPath,
						TEXT("이미 Retired인 record를 다시 Retire할 수 없습니다."));
					continue;
				}
				Record.LifecycleState = ECFContentLifecycleState::Retired;
			}
			else if (Operation.OperationType == ECFContentChangeOperationType::ReactivateRecord)
			{
				if (Record.LifecycleState != ECFContentLifecycleState::Retired)
				{
					AddIssue(
						OutIssues,
						TEXT("ReactivateNonRetiredRecord"),
						OperationPath,
						TEXT("ReactivateRecord는 Retired record에만 사용할 수 있습니다."));
					continue;
				}
				Record.LifecycleState = ECFContentLifecycleState::Active;
			}
		}

		// Base managed row가 physical removal로 사라지지 않았는지 fail-closed 검증합니다.
		for (const FCFContentRecord& BaseRecord : BaseSnapshot.Workbook.Records)
		{
			if (BaseRecord.ManagementState == ECFContentManagementState::Managed
				&& FindRecordIndex(OutPostWorkbook, BaseRecord.Key) == INDEX_NONE)
			{
				AddIssue(
					OutIssues,
					TEXT("ImplicitRecordRemoval"),
					BaseRecord.Key.ToStableString(),
					TEXT("Managed record physical removal은 Retire authority가 아니며 P0-06에서 금지됩니다."));
			}
		}

		// Full canonical schema/value/planning validation diagnostics입니다.
		TArray<FCFContentValidationIssue> WorkbookIssues;
		if (!FCFContentSchemaValidator::ValidateWorkbook(OutPostWorkbook, WorkbookIssues))
		{
			OutIssues.Append(WorkbookIssues);
		}
		else
		{
			OutIssues.Append(WorkbookIssues);
		}

		// USER review에 보존할 typed operation projection입니다.
		for (int32 OperationIndex = 0; OperationIndex < Proposal.ChangeSet.Operations.Num(); ++OperationIndex)
		{
			// Exact authored operation입니다.
			const FCFContentChangeOperation& Operation = Proposal.ChangeSet.Operations[OperationIndex];
			// Review projection item입니다.
			FCFContentReviewChange& ReviewChange = OutPlanningChanges.AddDefaulted_GetRef();
			ReviewChange.OperationIndex = OperationIndex;
			ReviewChange.ContentKey = Operation.ContentKey;
			ReviewChange.OperationType = Operation.OperationType;
			ReviewChange.SemanticTarget = BuildOperationSemanticTarget(Operation);
		}

		return !HasBlockingIssues(OutIssues);
	}

	/** In-memory staged Workbook만 제공하고 write/reopen을 호출하면 fail-closed하는 adapter입니다. */
	class FReviewMemoryWorkbookAdapter final : public ICFContentWorkbookAdapter
	{
	public:
		/** Transient review candidate model로 adapter를 생성합니다. */
		explicit FReviewMemoryWorkbookAdapter(const FCFContentWorkbookModel& InWorkbook)
			: Workbook(InWorkbook)
		{
		}

		/** In-memory review adapter identity를 반환합니다. */
		virtual FCFWorkbookAdapterInfo DescribeAdapter() const override
		{
			// Review-only adapter info입니다.
			FCFWorkbookAdapterInfo Info;
			Info.AdapterId = TEXT("CCAS.P006.ReviewMemory");
			Info.AdapterVersion = TEXT("1.0.0");
			Info.bPreservesPresentation = false;
			Info.bPreservesProtection = false;
			return Info;
		}

		/** Transient candidate model을 read-only로 반환합니다. */
		virtual bool ReadWorkbook(
			const FString& WorkbookPath,
			FCFContentWorkbookModel& OutWorkbook,
			FString& OutError) override
		{
			(void)WorkbookPath;
			OutError.Reset();
			OutWorkbook = Workbook;
			++ReadCallCount;
			return true;
		}

		/** P0-06 review path의 persistent staged write를 fail-closed합니다. */
		virtual bool WriteStagedWorkbook(
			const FString& BaseWorkbookPath,
			const FString& StagedWorkbookPath,
			const FCFContentWorkbookModel& ReviewedWorkbook,
			FString& OutError) override
		{
			(void)BaseWorkbookPath;
			(void)StagedWorkbookPath;
			(void)ReviewedWorkbook;
			++WriteCallCount;
			OutError = TEXT("P0-06 review memory adapter는 persistent Workbook write를 허용하지 않습니다.");
			return false;
		}

		/** P0-06 review path의 disk reopen을 fail-closed합니다. */
		virtual bool ReopenWorkbook(
			const FString& StagedWorkbookPath,
			FCFContentWorkbookModel& OutWorkbook,
			FString& OutError) override
		{
			(void)StagedWorkbookPath;
			(void)OutWorkbook;
			++ReopenCallCount;
			OutError = TEXT("P0-06 review memory adapter는 persistent Workbook reopen을 허용하지 않습니다.");
			return false;
		}

		// Compiler preview read 호출 수입니다.
		int32 ReadCallCount = 0;

		// Persistent staged write 시도 수입니다.
		int32 WriteCallCount = 0;

		// Persistent disk reopen 시도 수입니다.
		int32 ReopenCallCount = 0;

	private:
		// Review 대상 transient candidate Workbook입니다.
		FCFContentWorkbookModel Workbook;
	};

	/** Retired target이 active desired referencer에게 계속 요구되는지 conservative fail-closed 검증합니다. */
	bool ValidateRetiredReferences(
		const FCFContentCompileResult& Preview,
		TArray<FCFContentValidationIssue>& OutIssues)
	{
		for (const FCFContentDependencyEdge& Edge : Preview.DependencyEdges)
		{
			// Desired source record index입니다.
			const int32 SourceIndex = FindRecordIndex(Preview.Workbook, Edge.From);
			// Desired target record index입니다.
			const int32 TargetIndex = FindRecordIndex(Preview.Workbook, Edge.To);
			if (!Preview.Workbook.Records.IsValidIndex(SourceIndex)
				|| !Preview.Workbook.Records.IsValidIndex(TargetIndex))
			{
				continue;
			}

			// Desired source record입니다.
			const FCFContentRecord& SourceRecord = Preview.Workbook.Records[SourceIndex];
			// Desired target record입니다.
			const FCFContentRecord& TargetRecord = Preview.Workbook.Records[TargetIndex];
			if (SourceRecord.LifecycleState != ECFContentLifecycleState::Retired
				&& TargetRecord.LifecycleState == ECFContentLifecycleState::Retired)
			{
				AddIssue(
					OutIssues,
					TEXT("RetiredRequiredReference"),
					Edge.FieldPath,
					FString::Printf(
						TEXT("Active/Deprecated record %s가 Retired target %s를 계속 참조합니다. Provider optional-reference 정책 seam이 없으므로 P0-06은 fail-closed합니다."),
						*Edge.From.ToStableString(),
						*Edge.To.ToStableString()));
			}
		}
		return !HasBlockingIssues(OutIssues);
	}

	/** ReviewPackage 전체 semantic payload fingerprint를 생성합니다. */
	bool BuildReviewPackageFingerprint(
		const FCFContentReviewPackage& ReviewPackage,
		FString& OutFingerprint,
		FString& OutError)
	{
		OutFingerprint.Reset();
		OutError.Reset();

		// Immutable review package canonical byte stream입니다.
		TArray<uint8> CanonicalBytes;
		AppendString(CanonicalBytes, TEXT("Protocol"), TEXT("CarFight.CCAS.ReviewPackage/v1"));
		AppendString(CanonicalBytes, TEXT("ChangeSetId"), ReviewPackage.ChangeSetId);
		AppendString(CanonicalBytes, TEXT("ChangeSetFingerprint"), ReviewPackage.ChangeSetFingerprint);
		AppendString(CanonicalBytes, TEXT("BaseCatalogSnapshotFingerprint"), ReviewPackage.BaseCatalogSnapshotFingerprint);
		AppendString(CanonicalBytes, TEXT("BaseWorkbookSemanticHash"), ReviewPackage.BaseWorkbookSemanticHash);
		AppendString(CanonicalBytes, TEXT("ExpectedPostSemanticHash"), ReviewPackage.ExpectedPostSemanticHash);
		AppendString(CanonicalBytes, TEXT("ExecutionBindingFingerprint"), ReviewPackage.ExecutionBindingFingerprint);
		AppendBool(CanonicalBytes, TEXT("NoPersistentWorkbookWrite"), ReviewPackage.bNoPersistentWorkbookWriteGuardPassed);
		AppendBool(CanonicalBytes, TEXT("NoDirectDAWrite"), ReviewPackage.bNoDirectDataAssetWriteGuardPassed);

		// Stable ContentKey order의 rationale projection입니다.
		TArray<const FCFContentRecordRationale*> SortedRationales;
		for (const FCFContentRecordRationale& Rationale : ReviewPackage.RecordRationales)
		{
			SortedRationales.Add(&Rationale);
		}
		SortedRationales.Sort(
			[](const FCFContentRecordRationale& Left, const FCFContentRecordRationale& Right)
			{
				return Left.ContentKey.ToStableString() < Right.ContentKey.ToStableString();
			});
		for (const FCFContentRecordRationale* Rationale : SortedRationales)
		{
			if (Rationale == nullptr)
			{
				continue;
			}

			AppendContentKey(CanonicalBytes, TEXT("RationaleKey"), Rationale->ContentKey);
			AppendString(CanonicalBytes, TEXT("RationaleReason"), Rationale->Reason);
			AppendString(CanonicalBytes, TEXT("RationaleDesignIntentDelta"), Rationale->DesignIntentDelta);

			// Stable operation index order입니다.
			TArray<int32> RelatedIndexes = Rationale->RelatedOperationIndexes;
			RelatedIndexes.Sort();
			for (const int32 OperationIndex : RelatedIndexes)
			{
				AppendInt(CanonicalBytes, TEXT("RationaleOperationIndex"), OperationIndex);
			}
		}

		// Authored operation order를 보존하는 typed planning change projection입니다.
		for (const FCFContentReviewChange& Change : ReviewPackage.PlanningChanges)
		{
			AppendInt(CanonicalBytes, TEXT("ReviewChangeIndex"), Change.OperationIndex);
			AppendContentKey(CanonicalBytes, TEXT("ReviewChangeKey"), Change.ContentKey);
			AppendInt(CanonicalBytes, TEXT("ReviewChangeType"), static_cast<int64>(Change.OperationType));
			AppendString(CanonicalBytes, TEXT("ReviewChangeTarget"), Change.SemanticTarget);
		}

		// Compiler Product diff를 stable ContentKey order로 정렬합니다.
		TArray<const FCFContentCompileDiff*> SortedDiffs;
		for (const FCFContentCompileDiff& Diff : ReviewPackage.Preview.Diffs)
		{
			SortedDiffs.Add(&Diff);
		}
		SortedDiffs.Sort(
			[](const FCFContentCompileDiff& Left, const FCFContentCompileDiff& Right)
			{
				return Left.Key.ToStableString() < Right.Key.ToStableString();
			});
		for (const FCFContentCompileDiff* Diff : SortedDiffs)
		{
			if (Diff == nullptr)
			{
				continue;
			}
			AppendContentKey(CanonicalBytes, TEXT("DiffKey"), Diff->Key);
			AppendInt(CanonicalBytes, TEXT("DiffKind"), static_cast<int64>(Diff->Kind));
			AppendInt(CanonicalBytes, TEXT("DiffManagementState"), static_cast<int64>(Diff->ManagementState));
			AppendString(CanonicalBytes, TEXT("DiffCurrentFingerprint"), Diff->CurrentProviderFingerprint);
			AppendString(CanonicalBytes, TEXT("DiffDesiredFingerprint"), Diff->DesiredProviderFingerprint);

			// Stable changed path order입니다.
			TArray<FString> ChangedPaths = Diff->ChangedPaths;
			ChangedPaths.Sort();
			for (const FString& ChangedPath : ChangedPaths)
			{
				AppendString(CanonicalBytes, TEXT("DiffChangedPath"), ChangedPath);
			}
		}

		// Compiler impact summary를 stable changed-root order로 정렬합니다.
		TArray<const FCFContentImpactEntry*> SortedImpacts;
		for (const FCFContentImpactEntry& Impact : ReviewPackage.Preview.Impacts)
		{
			SortedImpacts.Add(&Impact);
		}
		SortedImpacts.Sort(
			[](const FCFContentImpactEntry& Left, const FCFContentImpactEntry& Right)
			{
				return Left.ChangedKey.ToStableString() < Right.ChangedKey.ToStableString();
			});
		for (const FCFContentImpactEntry* Impact : SortedImpacts)
		{
			if (Impact == nullptr)
			{
				continue;
			}
			AppendContentKey(CanonicalBytes, TEXT("ImpactRoot"), Impact->ChangedKey);

			// Direct dependent stable keys입니다.
			TArray<FString> DirectDependentKeys;
			for (const FCFContentKey& Key : Impact->DirectDependents)
			{
				DirectDependentKeys.Add(Key.ToStableString());
			}
			DirectDependentKeys.Sort();
			for (const FString& Key : DirectDependentKeys)
			{
				AppendString(CanonicalBytes, TEXT("DirectDependent"), Key);
			}

			// Transitive dependent stable keys입니다.
			TArray<FString> TransitiveDependentKeys;
			for (const FCFContentKey& Key : Impact->TransitiveDependents)
			{
				TransitiveDependentKeys.Add(Key.ToStableString());
			}
			TransitiveDependentKeys.Sort();
			for (const FString& Key : TransitiveDependentKeys)
			{
				AppendString(CanonicalBytes, TEXT("TransitiveDependent"), Key);
			}
		}

		// Validation machine summary를 stable sort합니다.
		TArray<FString> DiagnosticKeys;
		for (const FCFContentValidationIssue& Issue : ReviewPackage.Preview.Issues)
		{
			DiagnosticKeys.Add(FString::Printf(
				TEXT("%s|%s|%d"),
				*Issue.Code,
				*Issue.Path,
				Issue.bBlocking ? 1 : 0));
		}
		DiagnosticKeys.Sort();
		for (const FString& DiagnosticKey : DiagnosticKeys)
		{
			AppendString(CanonicalBytes, TEXT("PreviewDiagnostic"), DiagnosticKey);
		}

		return CFDACommonPrimitives::HashCanonicalBytes(
			CanonicalBytes,
			OutFingerprint,
			OutError);
	}
}

bool FCFContentPlanningService::BuildCatalogSnapshot(
	const FCFContentCompileResult& CompileResult,
	const FCFContentProviderRegistry& ProviderRegistry,
	const FCFResourceCatalog& ResourceCatalog,
	const FCFResourcePickerRegistry& PickerRegistry,
	FCFContentCatalogSnapshot& OutSnapshot,
	FString& OutError)
{
	OutSnapshot = FCFContentCatalogSnapshot();
	OutError.Reset();

	if (!CompileResult.bSucceeded)
	{
		OutError = TEXT("Successful Generic Compiler Preview만 immutable Catalog Snapshot의 base가 될 수 있습니다.");
		return false;
	}
	if (!CFDACommonPrimitives::IsCanonicalSha256Fingerprint(CompileResult.WorkbookSemanticHash)
		|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(CompileResult.CurrentProductSemanticHash))
	{
		OutError = TEXT("Snapshot base Workbook/Product fingerprint가 canonical SHA-256이 아닙니다.");
		return false;
	}

	// Current registered Resource Catalog semantic fingerprint입니다.
	FString ResourceCatalogFingerprint;
	// Resource existence/type/capability diagnostics입니다.
	TArray<FCFContentValidationIssue> ResourceCatalogIssues;
	if (!ResourceCatalog.BuildSemanticFingerprint(
		PickerRegistry,
		ResourceCatalogFingerprint,
		ResourceCatalogIssues))
	{
		OutError = ResourceCatalogIssues.Num() > 0
			? ResourceCatalogIssues[0].Message
			: TEXT("Resource Catalog fingerprint 생성에 실패했습니다.");
		return false;
	}

	// Workbook ResourceReference가 current Resource Catalog와 일치하는지 검증합니다.
	TArray<FCFContentValidationIssue> ResourceReferenceIssues;
	FCFContentResourceValidator::ValidateWorkbook(
		CompileResult.Workbook,
		ResourceCatalog,
		PickerRegistry,
		ResourceReferenceIssues);

	// Provider schema/ownership fingerprint입니다.
	FString ProviderSchemaFingerprint;
	if (!ProviderRegistry.BuildSchemaFingerprint(ProviderSchemaFingerprint, OutError))
	{
		return false;
	}

	// Current Product/ExternalReadOnly semantic state fingerprint입니다.
	FString ProductStateFingerprint;
	if (!CFContentPlanningPrivate::BuildProductStateFingerprint(
		CompileResult,
		ProductStateFingerprint,
		OutError))
	{
		return false;
	}

	OutSnapshot.WorkbookSourceId = CompileResult.Workbook.WorkbookSourceId;
	OutSnapshot.WorkbookSemanticHash = CompileResult.WorkbookSemanticHash;
	OutSnapshot.WorkbookSchemaVersion = CompileResult.Workbook.SchemaRevision;
	OutSnapshot.ProviderSchemaFingerprint = ProviderSchemaFingerprint;
	OutSnapshot.ProductStateFingerprint = ProductStateFingerprint;
	OutSnapshot.ResourceCatalogFingerprint = ResourceCatalogFingerprint;
	OutSnapshot.Workbook = CompileResult.Workbook;
	OutSnapshot.DependencyEdges = CompileResult.DependencyEdges;
	OutSnapshot.CurrentDependencyEdges = CompileResult.CurrentDependencyEdges;
	OutSnapshot.ValidationSummary = CompileResult.Issues;
	OutSnapshot.ValidationSummary.Append(ResourceCatalogIssues);
	OutSnapshot.ValidationSummary.Append(ResourceReferenceIssues);
	OutSnapshot.GeneratedDriftSummary = CompileResult.GeneratedFingerprints;

	if (!CFContentPlanningPrivate::BuildSnapshotFingerprint(
		OutSnapshot,
		OutSnapshot.SnapshotFingerprint,
		OutError))
	{
		return false;
	}

	return true;
}

bool FCFContentPlanningService::BuildReviewPackage(
	const FCFContentCatalogSnapshot& BaseSnapshot,
	const FCFContentChangeProposal& Proposal,
	const FCFContentProviderRegistry& ProviderRegistry,
	const FCFResourceCatalog& ResourceCatalog,
	const FCFResourcePickerRegistry& PickerRegistry,
	FCFContentReviewPackage& OutReviewPackage,
	TArray<FCFContentValidationIssue>& OutIssues,
	FString& OutError)
{
	OutReviewPackage = FCFContentReviewPackage();
	OutIssues.Reset();
	OutError.Reset();

	if (!CFDACommonPrimitives::IsCanonicalSha256Fingerprint(BaseSnapshot.SnapshotFingerprint)
		|| !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(BaseSnapshot.WorkbookSemanticHash))
	{
		CFContentPlanningPrivate::AddIssue(
			OutIssues,
			TEXT("InvalidBaseSnapshot"),
			TEXT("BaseSnapshot"),
			TEXT("Review base Catalog Snapshot/Workbook fingerprint가 canonical하지 않습니다."));
		OutError = TEXT("Invalid base Catalog Snapshot.");
		return false;
	}

	// Change Set을 적용한 transient post Workbook입니다.
	FCFContentWorkbookModel PostWorkbook;
	// Planning/Lifecycle까지 포함한 typed review operation projection입니다.
	TArray<FCFContentReviewChange> PlanningChanges;
	if (!CFContentPlanningPrivate::BuildPostWorkbook(
		BaseSnapshot,
		Proposal,
		PostWorkbook,
		PlanningChanges,
		OutIssues))
	{
		OutError = TEXT("Snapshot-aware Change Set semantic preflight가 실패했습니다.");
		return false;
	}

	// Deterministic post Workbook semantic hash입니다.
	FString ExpectedPostSemanticHash;
	// Semantic hash generation error입니다.
	FString SemanticHashError;
	if (!FCFContentSemanticHasher::BuildWorkbookSemanticHash(
		PostWorkbook,
		ExpectedPostSemanticHash,
		SemanticHashError))
	{
		CFContentPlanningPrivate::AddIssue(
			OutIssues,
			TEXT("PostSemanticHashFailed"),
			TEXT("PostWorkbook"),
			SemanticHashError.IsEmpty()
				? TEXT("Post Workbook semantic hash 생성에 실패했습니다.")
				: SemanticHashError);
		OutError = TEXT("Post Workbook semantic hash 생성 실패.");
		return false;
	}

	if (!Proposal.ChangeSet.ExpectedPostSemanticHash.IsEmpty()
		&& !Proposal.ChangeSet.ExpectedPostSemanticHash.Equals(
			ExpectedPostSemanticHash,
			ESearchCase::CaseSensitive))
	{
		CFContentPlanningPrivate::AddIssue(
			OutIssues,
			TEXT("ExpectedPostSemanticHashMismatch"),
			TEXT("ChangeSet.ExpectedPostSemanticHash"),
			TEXT("AI가 제시한 expected post hash와 deterministic transient Workbook hash가 다릅니다."));
		OutError = TEXT("Expected post semantic hash mismatch.");
		return false;
	}

	// Post Workbook의 ResourceReference를 current snapshot Resource Catalog 기준으로 검증합니다.
	TArray<FCFContentValidationIssue> ResourceIssues;
	FCFContentResourceValidator::ValidateWorkbook(
		PostWorkbook,
		ResourceCatalog,
		PickerRegistry,
		ResourceIssues);
	OutIssues.Append(ResourceIssues);
	if (CFContentPlanningPrivate::HasBlockingIssues(ResourceIssues))
	{
		OutError = TEXT("Post Workbook ResourceReference validation이 실패했습니다.");
		return false;
	}

	// Product writer가 없는 transient review-only Workbook adapter입니다.
	CFContentPlanningPrivate::FReviewMemoryWorkbookAdapter ReviewAdapter(PostWorkbook);
	// Existing Compiler가 다시 계산하는 authoritative Product diff/impact preview입니다.
	FCFContentCompileResult ReviewPreview;
	if (!FCFContentCompiler::CompilePreview(
		TEXT("Memory://CCAS/P006/Review"),
		ReviewAdapter,
		ProviderRegistry,
		ReviewPreview))
	{
		OutIssues.Append(ReviewPreview.Issues);
		OutError = ReviewPreview.Error.IsEmpty()
			? TEXT("Generic Compiler review preview가 실패했습니다.")
			: ReviewPreview.Error;
		return false;
	}
	OutIssues.Append(ReviewPreview.Issues);

	if (!CFContentPlanningPrivate::ValidateRetiredReferences(
		ReviewPreview,
		OutIssues))
	{
		OutError = TEXT("Retired content reference safety validation이 실패했습니다.");
		return false;
	}

	// Exact ChangeSet semantic fingerprint입니다.
	FString ChangeSetFingerprint;
	// ChangeSet hash error입니다.
	FString ChangeSetHashError;
	if (!CFContentPlanningPrivate::BuildChangeSetFingerprint(
		Proposal.ChangeSet,
		ChangeSetFingerprint,
		ChangeSetHashError))
	{
		CFContentPlanningPrivate::AddIssue(
			OutIssues,
			TEXT("ChangeSetFingerprintFailed"),
			TEXT("ChangeSet"),
			ChangeSetHashError.IsEmpty()
				? TEXT("Change Set fingerprint 생성에 실패했습니다.")
				: ChangeSetHashError);
		OutError = TEXT("Change Set fingerprint 생성 실패.");
		return false;
	}

	OutReviewPackage.ChangeSetId = Proposal.ChangeSet.ChangeSetId;
	OutReviewPackage.ChangeSetFingerprint = ChangeSetFingerprint;
	OutReviewPackage.BaseCatalogSnapshotFingerprint = BaseSnapshot.SnapshotFingerprint;
	OutReviewPackage.BaseWorkbookSemanticHash = BaseSnapshot.WorkbookSemanticHash;
	OutReviewPackage.ExpectedPostSemanticHash = ExpectedPostSemanticHash;
	OutReviewPackage.RecordRationales = Proposal.RecordRationales;
	OutReviewPackage.PlanningChanges = PlanningChanges;
	OutReviewPackage.Preview = ReviewPreview;
	OutReviewPackage.bNoPersistentWorkbookWriteGuardPassed =
		ReviewAdapter.WriteCallCount == 0
		&& ReviewAdapter.ReopenCallCount == 0;
	OutReviewPackage.bNoDirectDataAssetWriteGuardPassed = true;

	if (!OutReviewPackage.bNoPersistentWorkbookWriteGuardPassed)
	{
		CFContentPlanningPrivate::AddIssue(
			OutIssues,
			TEXT("UnexpectedWorkbookWrite"),
			TEXT("P0-06Review"),
			TEXT("Review path에서 staged Workbook write/reopen 호출이 감지됐습니다."));
		OutError = TEXT("P0-06 review write guard 실패.");
		return false;
	}

	// Immutable review package fingerprint generation error입니다.
	FString ReviewHashError;
	if (!CFContentPlanningPrivate::BuildReviewPackageFingerprint(
		OutReviewPackage,
		OutReviewPackage.ReviewPackageFingerprint,
		ReviewHashError))
	{
		CFContentPlanningPrivate::AddIssue(
			OutIssues,
			TEXT("ReviewPackageFingerprintFailed"),
			TEXT("ReviewPackage"),
			ReviewHashError.IsEmpty()
				? TEXT("Review Package fingerprint 생성에 실패했습니다.")
				: ReviewHashError);
		OutError = TEXT("Review Package fingerprint 생성 실패.");
		return false;
	}

	return !CFContentPlanningPrivate::HasBlockingIssues(OutIssues);
}

bool FCFContentPlanningService::ValidateReviewFreshness(
	const FCFContentReviewPackage& ReviewPackage,
	const FCFContentCatalogSnapshot& FreshSnapshot,
	TArray<FCFContentValidationIssue>& OutIssues)
{
	OutIssues.Reset();

	if (!ReviewPackage.BaseWorkbookSemanticHash.Equals(
		FreshSnapshot.WorkbookSemanticHash,
		ESearchCase::CaseSensitive))
	{
		CFContentPlanningPrivate::AddIssue(
			OutIssues,
			TEXT("StaleWorkbook"),
			TEXT("ReviewPackage.BaseWorkbookSemanticHash"),
			TEXT("Review 뒤 Workbook semantic state가 변경됐습니다. fresh replan/review가 필요합니다."));
	}

	if (!ReviewPackage.BaseCatalogSnapshotFingerprint.Equals(
		FreshSnapshot.SnapshotFingerprint,
		ESearchCase::CaseSensitive))
	{
		CFContentPlanningPrivate::AddIssue(
			OutIssues,
			TEXT("StalePlanningSnapshot"),
			TEXT("ReviewPackage.BaseCatalogSnapshotFingerprint"),
			TEXT("Workbook/Product/Resource/Provider planning truth 중 하나 이상이 변경됐습니다. 자동 rebase 없이 fresh replan이 필요합니다."));
	}

	return !CFContentPlanningPrivate::HasBlockingIssues(OutIssues);
}

// ReviewPackage current payload의 deterministic canonical fingerprint를 read-only로 계산합니다.
bool FCFContentPlanningService::ComputeReviewPackageFingerprint(
	const FCFContentReviewPackage& ReviewPackage,
	FString& OutFingerprint,
	FString& OutError)
{
	return CFContentPlanningPrivate::BuildReviewPackageFingerprint(
		ReviewPackage,
		OutFingerprint,
		OutError);
}

bool FCFContentPlanningService::ValidateApproval(
	const FCFContentReviewPackage& ReviewPackage,
	const FCFContentReviewApproval& Approval,
	TArray<FCFContentValidationIssue>& OutIssues)
{
	OutIssues.Reset();

	if (!Approval.bApproved)
	{
		CFContentPlanningPrivate::AddIssue(
			OutIssues,
			TEXT("ReviewNotApproved"),
			TEXT("Approval"),
			TEXT("USER가 exact Review Package를 승인하지 않았습니다."));
	}
	if (!CFDACommonPrimitives::IsCanonicalSha256Fingerprint(ReviewPackage.ReviewPackageFingerprint))
	{
		CFContentPlanningPrivate::AddIssue(
			OutIssues,
			TEXT("InvalidReviewPackageFingerprint"),
			TEXT("ReviewPackage.ReviewPackageFingerprint"),
			TEXT("ReviewPackageFingerprint가 canonical SHA-256이 아닙니다."));
	}

	// Current ReviewPackage payload에서 fresh 재계산한 immutable fingerprint입니다.
	FString FreshReviewPackageFingerprint;
	// ReviewPackage payload fingerprint 재계산 오류입니다.
	FString FingerprintError;
	if (!CFContentPlanningPrivate::BuildReviewPackageFingerprint(
		ReviewPackage,
		FreshReviewPackageFingerprint,
		FingerprintError)
		|| !FreshReviewPackageFingerprint.Equals(
			ReviewPackage.ReviewPackageFingerprint,
			ESearchCase::CaseSensitive))
	{
		CFContentPlanningPrivate::AddIssue(
			OutIssues,
			TEXT("ReviewPackagePayloadFingerprintMismatch"),
			TEXT("ReviewPackage"),
			FingerprintError.IsEmpty()
				? TEXT("Review Package payload가 approved immutable fingerprint와 다릅니다.")
				: FingerprintError);
	}
	if (!Approval.ReviewPackageFingerprint.Equals(
		ReviewPackage.ReviewPackageFingerprint,
		ESearchCase::CaseSensitive))
	{
		CFContentPlanningPrivate::AddIssue(
			OutIssues,
			TEXT("ReviewApprovalFingerprintMismatch"),
			TEXT("Approval.ReviewPackageFingerprint"),
			TEXT("USER approval이 현재 immutable ReviewPackageFingerprint와 exact 일치하지 않습니다."));
	}

	return !CFContentPlanningPrivate::HasBlockingIssues(OutIssues);
}
