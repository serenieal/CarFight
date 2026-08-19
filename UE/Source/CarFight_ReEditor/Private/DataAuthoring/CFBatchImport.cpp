// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchImport.cpp
// Version: v1.1.2
// Date: 2026-08-19
// Description: DAUTH-P0-08K~L Batch 3-way Preview와 B1/B2 atomic Authoring Source Commit 구현입니다.
// Scope: Current truth re-read, exact approval binding, global TOCTOU preflight, Recipe/Profile typed transaction과 rollback을 제공합니다.
// Changelog:
// - v1.1.2: UA-03에서 B2 affected Recipe inventory가 `/Engine/Transient` rollback/prospective scratch Recipe를 실제 managed Recipe로 오인하던 문제를 제외하고 non-transient unsaved Recipe 지원은 유지.
// - v1.1.1: K downstream Definition validation과 L source-commit blocker를 분리하고 B1 Recipe-only/B2 affected-Vehicle fresh validation gate를 고정.
// - v1.1.0: Section 26.44~26.53 B1/B2 approval binding, global preflight, one logical transaction, rollback/no-auto-save를 구현.
// - v1.0.3: UE 5.8 FCString에 없는 Strtod를 제거하고 invariant decimal grammar 검사 + FCString::Atod helper로 교체.
// - v1.0.2: UE 5.8에 없는 Misc/LexFromString.h 의존을 제거하고 FCString strict integer parser helper로 교체.
// - v1.0.1: UE 5.8 TArray에 없는 CountByPredicate 사용을 explicit Diff loop로 교정.
// - v1.0.0: Section 26.31~26.43 Batch Import Session Foundation 최초 구현.
// Migration:
// - B1/B2 source commit만 explicit approval 뒤 허용하며 B3 Definition Apply, UI, disk file I/O는 수행하지 않습니다.
// - successful source commit 후 old Preview/Approval은 source fingerprint mismatch로 stale하며 fresh Preview가 필요합니다.

#include "DataAuthoring/CFBatchImport.h"

#include "CFVehicleData.h"
#include "DataAuthoring/CFBatchColumnRegistry.h"
#include "DataAuthoring/CFBatchExport.h"
#include "DataAuthoring/CFDriveStateProfile.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleAuthoringService.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Dom/JsonObject.h"
#include "Misc/SecureHash.h"
#include "ScopedTransaction.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"

namespace CFBatchImportPrivate
{
	/** CSV state machine가 생성하는 header + physical data rows입니다. */
	struct FCsvTable
	{
		// 첫 physical CSV row의 stable technical headers입니다.
		TArray<FString> Headers;

		// Standard quote escaping을 해제한 physical data rows입니다.
		TArray<TArray<FString>> Rows;
	};

	/** Current authoring UObject와 P0-08J current-row projection을 함께 보존합니다. */
	struct FCurrentRowState
	{
		// Persistent current Recipe 또는 Profile UObject입니다.
		UObject* AuthoringObject = nullptr;

		// Import 시 다시 계산한 current canonical Batch row입니다.
		FCFBatchExportRow CurrentRow;
	};

	/** Dotted property path가 가리키는 mutable numeric leaf view입니다. */
	struct FMutablePropertyView
	{
		// Resolved reflected leaf property입니다.
		FProperty* Property = nullptr;

		// Property actual mutable storage address입니다.
		void* ValueAddress = nullptr;
	};

	// Batch issue를 deterministic field에 추가합니다.
	void AddIssue(
		TArray<FCFBatchIssue>& OutIssues,
		const ECFBatchIssueCode Code,
		const ECFBatchIssueSeverity Severity,
		const FString& Message,
		const FString& RowId = FString(),
		const FString& ColumnId = FString())
	{
		// 새 structured Batch issue입니다.
		FCFBatchIssue& Issue = OutIssues.AddDefaulted_GetRef();
		Issue.Code = Code;
		Issue.Severity = Severity;
		Issue.RowId = RowId;
		Issue.ColumnId = ColumnId;
		Issue.Message = Message;
	}

	// Row에 Blocked/Conflict 수준 issue가 하나 이상 있는지 검사합니다.
	bool HasBlockingIssue(const TArray<FCFBatchIssue>& Issues)
	{
		return Issues.ContainsByPredicate([](const FCFBatchIssue& Issue)
		{
			return Issue.Severity == ECFBatchIssueSeverity::Blocked
				|| Issue.Severity == ECFBatchIssueSeverity::Conflict;
		});
	}

	// Row에 Conflict issue가 하나 이상 있는지 검사합니다.
	bool HasConflictIssue(const TArray<FCFBatchIssue>& Issues)
	{
		return Issues.ContainsByPredicate([](const FCFBatchIssue& Issue)
		{
			return Issue.Severity == ECFBatchIssueSeverity::Conflict;
		});
	}

	// FString canonical payload를 deterministic lowercase MD5로 hash합니다.
	FString HashUtf8Payload(const FString& Payload)
	{
		// Hash authority가 소비할 UTF-8 payload입니다.
		const FTCHARToUTF8 Utf8Payload(*Payload);
		// Deterministic MD5 state입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());
		// Final MD5 bytes입니다.
		uint8 Digest[16];
		Md5.Final(Digest);

		// Lowercase hexadecimal 결과입니다.
		FString Result;
		Result.Reserve(32);
		// Stable lowercase hexadecimal lookup table입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			Result.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			Result.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return Result;
	}

	// Delimiter collision 없이 deterministic hash token을 추가합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("\n");
	}

	// Dataset Kind를 manifest protocol stable string으로 변환합니다.
	FString DatasetToString(const ECFBatchDatasetKind DatasetKind)
	{
		switch (DatasetKind)
		{
		case ECFBatchDatasetKind::VehicleSummaryReport:
			return TEXT("VehicleSummaryReport");
		case ECFBatchDatasetKind::ResolvedFieldReport:
			return TEXT("ResolvedFieldReport");
		case ECFBatchDatasetKind::RecipeNumericEdit:
			return TEXT("RecipeNumericEdit");
		case ECFBatchDatasetKind::ProfileNumericEdit:
			return TEXT("ProfileNumericEdit");
		default:
			return TEXT("Unknown");
		}
	}

	// Manifest Dataset Kind stable string을 enum으로 parse합니다.
	bool ParseDataset(const FString& Text, ECFBatchDatasetKind& OutDataset)
	{
		if (Text == TEXT("VehicleSummaryReport"))
		{
			OutDataset = ECFBatchDatasetKind::VehicleSummaryReport;
			return true;
		}
		if (Text == TEXT("ResolvedFieldReport"))
		{
			OutDataset = ECFBatchDatasetKind::ResolvedFieldReport;
			return true;
		}
		if (Text == TEXT("RecipeNumericEdit"))
		{
			OutDataset = ECFBatchDatasetKind::RecipeNumericEdit;
			return true;
		}
		if (Text == TEXT("ProfileNumericEdit"))
		{
			OutDataset = ECFBatchDatasetKind::ProfileNumericEdit;
			return true;
		}
		return false;
	}

	// Profile Domain을 manifest protocol stable string으로 변환합니다.
	FString DomainToString(const ECFVehicleProfileDomain Domain)
	{
		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			return TEXT("VehicleBase");
		case ECFVehicleProfileDomain::Drivetrain:
			return TEXT("Drivetrain");
		case ECFVehicleProfileDomain::Handling:
			return TEXT("Handling");
		case ECFVehicleProfileDomain::Performance:
			return TEXT("Performance");
		case ECFVehicleProfileDomain::DriveState:
			return TEXT("DriveState");
		default:
			return TEXT("None");
		}
	}

	// Manifest Profile Domain stable string을 enum으로 parse합니다.
	bool ParseDomain(const FString& Text, ECFVehicleProfileDomain& OutDomain)
	{
		if (Text == TEXT("None"))
		{
			OutDomain = ECFVehicleProfileDomain::None;
			return true;
		}
		if (Text == TEXT("VehicleBase"))
		{
			OutDomain = ECFVehicleProfileDomain::VehicleBase;
			return true;
		}
		if (Text == TEXT("Drivetrain"))
		{
			OutDomain = ECFVehicleProfileDomain::Drivetrain;
			return true;
		}
		if (Text == TEXT("Handling"))
		{
			OutDomain = ECFVehicleProfileDomain::Handling;
			return true;
		}
		if (Text == TEXT("Performance"))
		{
			OutDomain = ECFVehicleProfileDomain::Performance;
			return true;
		}
		if (Text == TEXT("DriveState"))
		{
			OutDomain = ECFVehicleProfileDomain::DriveState;
			return true;
		}
		return false;
	}

	// Manifest ValueType stable string을 enum으로 parse합니다.
	bool ParseValueType(const FString& Text, ECFBatchValueType& OutValueType)
	{
		if (Text == TEXT("String"))
		{
			OutValueType = ECFBatchValueType::String;
			return true;
		}
		if (Text == TEXT("Integer"))
		{
			OutValueType = ECFBatchValueType::Integer;
			return true;
		}
		if (Text == TEXT("Number"))
		{
			OutValueType = ECFBatchValueType::Number;
			return true;
		}
		if (Text == TEXT("Boolean"))
		{
			OutValueType = ECFBatchValueType::Boolean;
			return true;
		}
		if (Text == TEXT("CanonicalText"))
		{
			OutValueType = ECFBatchValueType::CanonicalText;
			return true;
		}
		return false;
	}

	// Manifest Access stable string을 enum으로 parse합니다.
	bool ParseAccess(const FString& Text, ECFBatchColumnAccess& OutAccess)
	{
		if (Text == TEXT("Editable"))
		{
			OutAccess = ECFBatchColumnAccess::Editable;
			return true;
		}
		if (Text == TEXT("ReadOnly"))
		{
			OutAccess = ECFBatchColumnAccess::ReadOnly;
			return true;
		}
		return false;
	}

	// Manifest AuthoringOwner stable string을 enum으로 parse합니다.
	bool ParseOwner(const FString& Text, ECFBatchAuthoringOwner& OutOwner)
	{
		if (Text == TEXT("Metadata"))
		{
			OutOwner = ECFBatchAuthoringOwner::Metadata;
			return true;
		}
		if (Text == TEXT("ResolvedFieldRegistry"))
		{
			OutOwner = ECFBatchAuthoringOwner::ResolvedFieldRegistry;
			return true;
		}
		if (Text == TEXT("Recipe"))
		{
			OutOwner = ECFBatchAuthoringOwner::Recipe;
			return true;
		}
		if (Text == TEXT("Profile"))
		{
			OutOwner = ECFBatchAuthoringOwner::Profile;
			return true;
		}
		return false;
	}

	// Manifest TypedMutationKind stable string을 enum으로 parse합니다.
	bool ParseMutation(const FString& Text, ECFBatchMutationKind& OutMutation)
	{
		if (Text == TEXT("None"))
		{
			OutMutation = ECFBatchMutationKind::None;
			return true;
		}
		if (Text == TEXT("RecipeDrivingFeelAxis"))
		{
			OutMutation = ECFBatchMutationKind::RecipeDrivingFeelAxis;
			return true;
		}
		if (Text == TEXT("RecipeMassExplicitValue"))
		{
			OutMutation = ECFBatchMutationKind::RecipeMassExplicitValue;
			return true;
		}
		if (Text == TEXT("RecipeDurabilityExplicitValue"))
		{
			OutMutation = ECFBatchMutationKind::RecipeDurabilityExplicitValue;
			return true;
		}
		if (Text == TEXT("ProfileNumericLeaf"))
		{
			OutMutation = ECFBatchMutationKind::ProfileNumericLeaf;
			return true;
		}
		return false;
	}

	// Manifest BlankPolicy stable string을 enum으로 parse합니다.
	bool ParseBlankPolicy(const FString& Text, ECFBatchBlankPolicy& OutBlankPolicy)
	{
		if (Text == TEXT("NoChange"))
		{
			OutBlankPolicy = ECFBatchBlankPolicy::NoChange;
			return true;
		}
		if (Text == TEXT("NotApplicable"))
		{
			OutBlankPolicy = ECFBatchBlankPolicy::NotApplicable;
			return true;
		}
		return false;
	}

	// Required JSON string field를 fail-closed로 읽습니다.
	bool ReadRequiredString(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, FString& OutValue)
	{
		return Object.IsValid() && Object->TryGetStringField(FieldName, OutValue);
	}

	// Required JSON integer field를 exact int32로 읽습니다.
	bool ReadRequiredInt(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, int32& OutValue)
	{
		// JSON number representation입니다.
		double NumberValue = 0.0;
		if (!Object.IsValid() || !Object->TryGetNumberField(FieldName, NumberValue)
			|| !FMath::IsFinite(NumberValue)
			|| NumberValue < static_cast<double>(MIN_int32)
			|| NumberValue > static_cast<double>(MAX_int32)
			|| FMath::FloorToDouble(NumberValue) != NumberValue)
		{
			return false;
		}
		OutValue = static_cast<int32>(NumberValue);
		return true;
	}

	// Required JSON bool field를 읽습니다.
	bool ReadRequiredBool(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, bool& OutValue)
	{
		return Object.IsValid() && Object->TryGetBoolField(FieldName, OutValue);
	}

	// Editable technical descriptor JSON object를 current manifest context로 parse합니다.
	bool ParseDescriptorObject(
		const TSharedPtr<FJsonObject>& Object,
		const ECFBatchDatasetKind DatasetKind,
		const ECFVehicleProfileDomain ProfileDomain,
		FCFBatchColumnDescriptor& OutDescriptor)
	{
		OutDescriptor = FCFBatchColumnDescriptor();
		OutDescriptor.DatasetKind = DatasetKind;
		OutDescriptor.ProfileDomain = ProfileDomain;
		OutDescriptor.bReservedMetadata = false;

		// Protocol enum/string fields입니다.
		FString ValueTypeText;
		FString AccessText;
		FString OwnerText;
		FString MutationText;
		FString BlankPolicyText;
		if (!ReadRequiredString(Object, TEXT("ColumnId"), OutDescriptor.ColumnId)
			|| !ReadRequiredString(Object, TEXT("ValueType"), ValueTypeText)
			|| !ReadRequiredString(Object, TEXT("Unit"), OutDescriptor.Unit)
			|| !ReadRequiredString(Object, TEXT("Access"), AccessText)
			|| !ReadRequiredString(Object, TEXT("AuthoringOwner"), OwnerText)
			|| !ReadRequiredString(Object, TEXT("TypedMutationKind"), MutationText)
			|| !ReadRequiredString(Object, TEXT("PropertyOrSemanticTarget"), OutDescriptor.PropertyOrSemanticTarget)
			|| !ReadRequiredString(Object, TEXT("BlankPolicy"), BlankPolicyText)
			|| !ReadRequiredString(Object, TEXT("ValidationMetadata"), OutDescriptor.ValidationMetadata)
			|| !ReadRequiredString(Object, TEXT("EditConditionTarget"), OutDescriptor.EditConditionTarget)
			|| !ReadRequiredString(Object, TEXT("RequiredEditConditionValue"), OutDescriptor.RequiredEditConditionValue))
		{
			return false;
		}
		return ParseValueType(ValueTypeText, OutDescriptor.ValueType)
			&& ParseAccess(AccessText, OutDescriptor.Access)
			&& ParseOwner(OwnerText, OutDescriptor.AuthoringOwner)
			&& ParseMutation(MutationText, OutDescriptor.TypedMutationKind)
			&& ParseBlankPolicy(BlankPolicyText, OutDescriptor.BlankPolicy);
	}

	// Manifest baseline cell JSON object를 parse합니다.
	bool ParseBaselineCellObject(const TSharedPtr<FJsonObject>& Object, FCFBatchBaselineCell& OutCell)
	{
		OutCell = FCFBatchBaselineCell();
		return ReadRequiredString(Object, TEXT("ColumnId"), OutCell.ColumnId)
			&& ReadRequiredString(Object, TEXT("CanonicalValue"), OutCell.CanonicalValue)
			&& ReadRequiredBool(Object, TEXT("EditableAtExport"), OutCell.bEditableAtExport)
			&& ReadRequiredString(Object, TEXT("OwnershipSourceMode"), OutCell.OwnershipSourceMode);
	}

	// Manifest row JSON object를 parse합니다.
	bool ParseManifestRowObject(const TSharedPtr<FJsonObject>& Object, FCFBatchManifestRow& OutRow)
	{
		OutRow = FCFBatchManifestRow();
		// Stable Profile Domain protocol string입니다.
		FString DomainText;
		if (!ReadRequiredString(Object, TEXT("RowId"), OutRow.RowId)
			|| !ReadRequiredString(Object, TEXT("TargetPath"), OutRow.TargetPath)
			|| !ReadRequiredString(Object, TEXT("RecipePath"), OutRow.RecipePath)
			|| !ReadRequiredString(Object, TEXT("ProfilePath"), OutRow.ProfilePath)
			|| !ReadRequiredString(Object, TEXT("ProfileDomain"), DomainText)
			|| !ReadRequiredString(Object, TEXT("ObjectFingerprint"), OutRow.ObjectFingerprint)
			|| !ReadRequiredString(Object, TEXT("TargetDefinitionHash"), OutRow.TargetDefinitionHash)
			|| !ParseDomain(DomainText, OutRow.ProfileDomain))
		{
			return false;
		}

		// Required baseline cell JSON values입니다.
		const TArray<TSharedPtr<FJsonValue>>* CellValues = nullptr;
		if (!Object->TryGetArrayField(TEXT("BaselineCells"), CellValues) || !CellValues)
		{
			return false;
		}
		for (const TSharedPtr<FJsonValue>& CellValue : *CellValues)
		{
			// One baseline cell object입니다.
			const TSharedPtr<FJsonObject> CellObject = CellValue.IsValid() ? CellValue->AsObject() : nullptr;
			// Parsed manifest baseline cell입니다.
			FCFBatchBaselineCell Cell;
			if (!CellObject.IsValid() || !ParseBaselineCellObject(CellObject, Cell))
			{
				return false;
			}
			OutRow.BaselineCells.Add(MoveTemp(Cell));
		}
		OutRow.BaselineCells.Sort([](const FCFBatchBaselineCell& Left, const FCFBatchBaselineCell& Right)
		{
			return Left.ColumnId < Right.ColumnId;
		});
		return true;
	}

	// Standard CSV quote/newline 규칙을 physical table로 parse합니다.
	bool ParseCsvTable(const FString& CsvText, FCsvTable& OutTable, FString& OutError)
	{
		OutTable = FCsvTable();
		if (CsvText.IsEmpty())
		{
			OutError = TEXT("CSV text가 비어 있습니다.");
			return false;
		}

		// 현재 physical row cells입니다.
		TArray<FString> CurrentRow;
		// 현재 cell unescaped text입니다.
		FString CurrentCell;
		// Quoted field 내부인지 여부입니다.
		bool bInQuotes = false;
		// Closing quote 이후 delimiter/newline만 허용하는 상태입니다.
		bool bAfterClosingQuote = false;
		// 현재 row에 delimiter 또는 cell content가 존재했는지 여부입니다.
		bool bRowStarted = false;

		// 현재 cell을 row에 확정합니다.
		auto CommitCell = [&CurrentRow, &CurrentCell, &bAfterClosingQuote]()
		{
			CurrentRow.Add(CurrentCell);
			CurrentCell.Reset();
			bAfterClosingQuote = false;
		};

		// 현재 row를 table에 확정합니다.
		auto CommitRow = [&OutTable, &CurrentRow, &CommitCell, &bRowStarted]()
		{
			CommitCell();
			OutTable.Rows.Add(CurrentRow);
			CurrentRow.Reset();
			bRowStarted = false;
		};

		for (int32 CharacterIndex = 0; CharacterIndex < CsvText.Len(); ++CharacterIndex)
		{
			// 현재 CSV TCHAR입니다.
			const TCHAR Character = CsvText[CharacterIndex];
			if (bInQuotes)
			{
				if (Character == TEXT('"'))
				{
					// Doubled quote는 quoted cell 내부 literal quote입니다.
					const bool bEscapedQuote = CharacterIndex + 1 < CsvText.Len() && CsvText[CharacterIndex + 1] == TEXT('"');
					if (bEscapedQuote)
					{
						CurrentCell.AppendChar(TEXT('"'));
						++CharacterIndex;
					}
					else
					{
						bInQuotes = false;
						bAfterClosingQuote = true;
					}
				}
				else
				{
					CurrentCell.AppendChar(Character);
				}
				bRowStarted = true;
				continue;
			}

			if (bAfterClosingQuote)
			{
				if (Character == TEXT(','))
				{
					CommitCell();
					bRowStarted = true;
					continue;
				}
				if (Character == TEXT('\r') || Character == TEXT('\n'))
				{
					CommitRow();
					if (Character == TEXT('\r') && CharacterIndex + 1 < CsvText.Len() && CsvText[CharacterIndex + 1] == TEXT('\n'))
					{
						++CharacterIndex;
					}
					continue;
				}
				OutError = FString::Printf(TEXT("CSV closing quote 뒤에 delimiter/newline 외 문자가 있습니다. Index=%d"), CharacterIndex);
				return false;
			}

			if (Character == TEXT('"'))
			{
				if (!CurrentCell.IsEmpty())
				{
					OutError = FString::Printf(TEXT("CSV unquoted cell 중간에 quote가 있습니다. Index=%d"), CharacterIndex);
					return false;
				}
				bInQuotes = true;
				bRowStarted = true;
				continue;
			}
			if (Character == TEXT(','))
			{
				CommitCell();
				bRowStarted = true;
				continue;
			}
			if (Character == TEXT('\r') || Character == TEXT('\n'))
			{
				CommitRow();
				if (Character == TEXT('\r') && CharacterIndex + 1 < CsvText.Len() && CsvText[CharacterIndex + 1] == TEXT('\n'))
				{
					++CharacterIndex;
				}
				continue;
			}
			CurrentCell.AppendChar(Character);
			bRowStarted = true;
		}

		if (bInQuotes)
		{
			OutError = TEXT("CSV quoted cell이 닫히지 않았습니다.");
			return false;
		}
		if (bAfterClosingQuote || bRowStarted || !CurrentRow.IsEmpty() || !CurrentCell.IsEmpty())
		{
			CommitRow();
		}
		if (OutTable.Rows.IsEmpty())
		{
			OutError = TEXT("CSV header row가 없습니다.");
			return false;
		}

		OutTable.Headers = MoveTemp(OutTable.Rows[0]);
		OutTable.Rows.RemoveAt(0);
		if (OutTable.Headers.IsEmpty())
		{
			OutError = TEXT("CSV header가 비어 있습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Parsed row에서 exact stable ColumnId cell을 찾습니다.
	const FCFBatchParsedCell* FindParsedCell(const FCFBatchParsedRow& Row, const FString& ColumnId)
	{
		return Row.Cells.FindByPredicate([&ColumnId](const FCFBatchParsedCell& Cell)
		{
			return Cell.ColumnId == ColumnId;
		});
	}

	// Manifest row에서 exact baseline cell을 찾습니다.
	const FCFBatchBaselineCell* FindBaselineCell(const FCFBatchManifestRow& Row, const FString& ColumnId)
	{
		return Row.BaselineCells.FindByPredicate([&ColumnId](const FCFBatchBaselineCell& Cell)
		{
			return Cell.ColumnId == ColumnId;
		});
	}

	// Current P0-08J row에서 exact canonical cell을 찾습니다.
	const FCFBatchBaselineCell* FindCurrentCell(const FCFBatchExportRow& Row, const FString& ColumnId)
	{
		return Row.BaselineCells.FindByPredicate([&ColumnId](const FCFBatchBaselineCell& Cell)
		{
			return Cell.ColumnId == ColumnId;
		});
	}

	// Current Registry에서 exact ColumnId descriptor를 찾습니다.
	const FCFBatchColumnDescriptor* FindDescriptor(const TArray<FCFBatchColumnDescriptor>& Columns, const FString& ColumnId)
	{
		return Columns.FindByPredicate([&ColumnId](const FCFBatchColumnDescriptor& Descriptor)
		{
			return Descriptor.ColumnId == ColumnId;
		});
	}

	// Reserved/read-only cell의 export baseline value를 manifest에서 재구성합니다.
	FString GetReservedBaselineValue(
		const FCFBatchColumnDescriptor& Descriptor,
		const FCFBatchManifest& Manifest,
		const FCFBatchManifestRow& Row)
	{
		if (Descriptor.ColumnId == TEXT("__cf_row_id"))
		{
			return Row.RowId;
		}
		if (Descriptor.ColumnId == TEXT("__cf_target"))
		{
			return Row.TargetPath;
		}
		if (Descriptor.ColumnId == TEXT("__cf_recipe"))
		{
			return Row.RecipePath;
		}
		if (Descriptor.ColumnId == TEXT("__cf_profile"))
		{
			return Row.ProfilePath;
		}
		if (Descriptor.ColumnId == TEXT("__cf_profile_domain"))
		{
			return DomainToString(Row.ProfileDomain);
		}
		if (Descriptor.ColumnId == TEXT("__cf_export_recipe_fingerprint")
			|| Descriptor.ColumnId == TEXT("__cf_export_profile_fingerprint"))
		{
			return Row.ObjectFingerprint;
		}
		if (Descriptor.ColumnId == TEXT("__cf_export_target_hash"))
		{
			return Row.TargetDefinitionHash;
		}
		if (Descriptor.ColumnId == TEXT("__cf_schema_id"))
		{
			return Manifest.SchemaId;
		}
		if (Descriptor.ColumnId == TEXT("__cf_schema_revision"))
		{
			return FString::FromInt(Manifest.SchemaRevision);
		}
		return FString();
	}

	// UObject dotted property path를 mutable leaf property/address로 resolve합니다.
	bool ResolveMutablePropertyPath(UObject& Object, const FString& PropertyPath, FMutablePropertyView& OutView, FString& OutError)
	{
		OutView = FMutablePropertyView();
		// Dot으로 분리한 reflected property segment입니다.
		TArray<FString> PropertyParts;
		PropertyPath.ParseIntoArray(PropertyParts, TEXT("."), true);
		if (PropertyParts.IsEmpty())
		{
			OutError = TEXT("Batch mutation target property path가 비어 있습니다.");
			return false;
		}

		// 현재 reflected struct/class입니다.
		UStruct* CurrentStruct = Object.GetClass();
		// 현재 container storage입니다.
		void* CurrentContainer = &Object;
		for (int32 PartIndex = 0; PartIndex < PropertyParts.Num(); ++PartIndex)
		{
			// 현재 path segment property입니다.
			FProperty* Property = FindFProperty<FProperty>(CurrentStruct, FName(*PropertyParts[PartIndex]));
			if (!Property)
			{
				OutError = FString::Printf(TEXT("Batch mutation target property를 찾지 못했습니다: %s"), *PropertyPath);
				return false;
			}
			// 현재 property actual storage입니다.
			void* ValueAddress = Property->ContainerPtrToValuePtr<void>(CurrentContainer);
			if (PartIndex == PropertyParts.Num() - 1)
			{
				OutView.Property = Property;
				OutView.ValueAddress = ValueAddress;
				OutError.Reset();
				return true;
			}

			// 중간 path는 nested USTRUCT만 허용합니다.
			FStructProperty* StructProperty = CastField<FStructProperty>(Property);
			if (!StructProperty || !StructProperty->Struct)
			{
				OutError = FString::Printf(TEXT("Batch mutation path 중간 segment가 struct가 아닙니다: %s"), *PropertyPath);
				return false;
			}
			CurrentStruct = StructProperty->Struct;
			CurrentContainer = ValueAddress;
		}

		OutError = FString::Printf(TEXT("Batch mutation property path가 terminal leaf 없이 끝났습니다: %s"), *PropertyPath);
		return false;
	}

		// Signed integer canonical text를 trailing character 없이 exact parse합니다.
	bool ParseStrictInt64(const FString& Text, int64& OutValue)
	{
		// Integer parser가 멈춘 위치입니다.
		TCHAR* ParseEnd = nullptr;
		OutValue = FCString::Strtoi64(*Text, &ParseEnd, 10);
		return ParseEnd && ParseEnd != *Text && *ParseEnd == TEXT('\0');
	}

	// Unsigned integer canonical text를 trailing character 없이 exact parse합니다.
	bool ParseStrictUInt64(const FString& Text, uint64& OutValue)
	{
		if (Text.StartsWith(TEXT("-"), ESearchCase::CaseSensitive))
		{
			return false;
		}
		// Unsigned integer parser가 멈춘 위치입니다.
		TCHAR* ParseEnd = nullptr;
		OutValue = FCString::Strtoui64(*Text, &ParseEnd, 10);
		return ParseEnd && ParseEnd != *Text && *ParseEnd == TEXT('\0');
	}

		// Invariant decimal/scientific notation을 trailing garbage 없이 exact parse합니다.
	bool ParseStrictDouble(const FString& Text, double& OutValue)
	{
		if (Text.IsEmpty())
		{
			return false;
		}

		// Decimal grammar를 순회할 current character index입니다.
		int32 CharacterIndex = 0;
		if (Text[CharacterIndex] == TEXT('+') || Text[CharacterIndex] == TEXT('-'))
		{
			++CharacterIndex;
		}

		// Decimal point 앞/뒤를 합쳐 최소 하나의 digit이 있었는지 여부입니다.
		bool bHasMantissaDigit = false;
		while (CharacterIndex < Text.Len() && FChar::IsDigit(Text[CharacterIndex]))
		{
			bHasMantissaDigit = true;
			++CharacterIndex;
		}
		if (CharacterIndex < Text.Len() && Text[CharacterIndex] == TEXT('.'))
		{
			++CharacterIndex;
			while (CharacterIndex < Text.Len() && FChar::IsDigit(Text[CharacterIndex]))
			{
				bHasMantissaDigit = true;
				++CharacterIndex;
			}
		}
		if (!bHasMantissaDigit)
		{
			return false;
		}

		if (CharacterIndex < Text.Len() && (Text[CharacterIndex] == TEXT('e') || Text[CharacterIndex] == TEXT('E')))
		{
			++CharacterIndex;
			if (CharacterIndex < Text.Len() && (Text[CharacterIndex] == TEXT('+') || Text[CharacterIndex] == TEXT('-')))
			{
				++CharacterIndex;
			}
			// Scientific notation에는 최소 하나의 exponent digit이 필요합니다.
			bool bHasExponentDigit = false;
			while (CharacterIndex < Text.Len() && FChar::IsDigit(Text[CharacterIndex]))
			{
				bHasExponentDigit = true;
				++CharacterIndex;
			}
			if (!bHasExponentDigit)
			{
				return false;
			}
		}
		if (CharacterIndex != Text.Len())
		{
			return false;
		}

		OutValue = FCString::Atod(*Text);
		return FMath::IsFinite(OutValue);
	}

	// Reflected numeric property가 unsigned integer인지 concrete class로 판정합니다.
	bool IsUnsignedIntegerProperty(const FProperty& Property)
	{
		return CastField<FByteProperty>(&Property) != nullptr
			|| CastField<FUInt16Property>(&Property) != nullptr
			|| CastField<FUInt32Property>(&Property) != nullptr
			|| CastField<FUInt64Property>(&Property) != nullptr;
	}

	// Plain integer reflected property가 authored value를 수용하는지 검사합니다.
	bool IsIntegerInRange(const FProperty& Property, const int64 SignedValue, const uint64 UnsignedValue, const bool bUnsigned)
	{
		if (CastField<FInt8Property>(&Property))
		{
			return !bUnsigned && SignedValue >= MIN_int8 && SignedValue <= MAX_int8;
		}
		if (CastField<FInt16Property>(&Property))
		{
			return !bUnsigned && SignedValue >= MIN_int16 && SignedValue <= MAX_int16;
		}
		if (CastField<FIntProperty>(&Property))
		{
			return !bUnsigned && SignedValue >= MIN_int32 && SignedValue <= MAX_int32;
		}
		if (CastField<FInt64Property>(&Property))
		{
			return !bUnsigned;
		}
		if (CastField<FByteProperty>(&Property))
		{
			return bUnsigned && UnsignedValue <= MAX_uint8;
		}
		if (CastField<FUInt16Property>(&Property))
		{
			return bUnsigned && UnsignedValue <= MAX_uint16;
		}
		if (CastField<FUInt32Property>(&Property))
		{
			return bUnsigned && UnsignedValue <= MAX_uint32;
		}
		if (CastField<FUInt64Property>(&Property))
		{
			return bUnsigned;
		}
		return false;
	}

	// Spreadsheet numeric text를 actual reflected property type 기준 canonical text로 normalize합니다.
	bool CanonicalizeEditedNumeric(
		UObject& CurrentObject,
		const FCFBatchColumnDescriptor& Descriptor,
		const FString& RawText,
		FString& OutCanonicalValue,
		FString& OutError)
	{
		// Excel export 후 leading/trailing whitespace를 값 identity로 취급하지 않는 trimmed numeric input입니다.
		const FString TrimmedText = RawText.TrimStartAndEnd();
		if (TrimmedText.IsEmpty())
		{
			OutError = TEXT("Blank는 caller가 NoChange로 먼저 처리해야 합니다.");
			return false;
		}
		if (TrimmedText.StartsWith(TEXT("="), ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Excel formula/macro expression은 canonical numeric value가 아닙니다.");
			return false;
		}

		// Descriptor target이 가리키는 current reflected numeric property입니다.
		FMutablePropertyView PropertyView;
		if (!ResolveMutablePropertyPath(CurrentObject, Descriptor.PropertyOrSemanticTarget, PropertyView, OutError)
			|| !PropertyView.Property
			|| !PropertyView.ValueAddress
			|| CastField<FBoolProperty>(PropertyView.Property)
			|| CastField<FEnumProperty>(PropertyView.Property))
		{
			if (OutError.IsEmpty())
			{
				OutError = FString::Printf(TEXT("Batch editable target이 plain numeric property가 아닙니다: %s"), *Descriptor.ColumnId);
			}
			return false;
		}
		// Plain reflected numeric property입니다.
		FNumericProperty* NumericProperty = CastField<FNumericProperty>(PropertyView.Property);
		if (!NumericProperty)
		{
			OutError = FString::Printf(TEXT("Batch editable target이 numeric property가 아닙니다: %s"), *Descriptor.ColumnId);
			return false;
		}
		if (FByteProperty* ByteProperty = CastField<FByteProperty>(PropertyView.Property))
		{
			if (ByteProperty->Enum)
			{
				OutError = FString::Printf(TEXT("Enum-backed byte는 Batch numeric editable이 아닙니다: %s"), *Descriptor.ColumnId);
				return false;
			}
		}

		if (NumericProperty->IsInteger())
		{
			// Reflected target의 signed/unsigned 성격입니다.
			const bool bUnsigned = IsUnsignedIntegerProperty(*PropertyView.Property);
			// Signed parse 결과입니다.
			int64 SignedValue = 0;
			// Unsigned parse 결과입니다.
			uint64 UnsignedValue = 0;
						if (bUnsigned)
			{
				if (!ParseStrictUInt64(TrimmedText, UnsignedValue))
				{
					OutError = FString::Printf(TEXT("Unsigned integer parse에 실패했습니다: %s"), *Descriptor.ColumnId);
					return false;
				}
			}
						else if (!ParseStrictInt64(TrimmedText, SignedValue))
			{
				OutError = FString::Printf(TEXT("Signed integer parse에 실패했습니다: %s"), *Descriptor.ColumnId);
				return false;
			}
			if (!IsIntegerInRange(*PropertyView.Property, SignedValue, UnsignedValue, bUnsigned))
			{
				OutError = FString::Printf(TEXT("Integer value가 reflected property 범위를 벗어났습니다: %s"), *Descriptor.ColumnId);
				return false;
			}
			OutCanonicalValue = bUnsigned
				? FString::Printf(TEXT("%llu"), static_cast<unsigned long long>(UnsignedValue))
				: FString::Printf(TEXT("%lld"), static_cast<long long>(SignedValue));
			OutError.Reset();
			return true;
		}

				// Invariant decimal grammar와 finite 조건을 모두 통과한 parsed value입니다.
		double ParsedValue = 0.0;
		if (!ParseStrictDouble(TrimmedText, ParsedValue))
		{
			OutError = FString::Printf(TEXT("Finite plain numeric parse에 실패했습니다: %s"), *Descriptor.ColumnId);
			return false;
		}

		if (CastField<FFloatProperty>(PropertyView.Property))
		{
			// 실제 float property에 저장될 precision으로 round한 candidate입니다.
			const float FloatValue = static_cast<float>(ParsedValue);
			if (!FMath::IsFinite(FloatValue))
			{
				OutError = FString::Printf(TEXT("Float property 범위를 벗어났습니다: %s"), *Descriptor.ColumnId);
				return false;
			}
			OutCanonicalValue = FString::Printf(TEXT("%.9g"), FloatValue == 0.0f ? 0.0 : static_cast<double>(FloatValue));
			OutCanonicalValue.ReplaceInline(TEXT(","), TEXT("."), ESearchCase::CaseSensitive);
			OutError.Reset();
			return true;
		}
		if (CastField<FDoubleProperty>(PropertyView.Property))
		{
			OutCanonicalValue = FString::Printf(TEXT("%.17g"), ParsedValue == 0.0 ? 0.0 : ParsedValue);
			OutCanonicalValue.ReplaceInline(TEXT(","), TEXT("."), ESearchCase::CaseSensitive);
			OutError.Reset();
			return true;
		}

		OutError = FString::Printf(TEXT("지원하지 않는 floating numeric property type입니다: %s"), *Descriptor.ColumnId);
		return false;
	}

	// Canonical numeric text를 transient reflected property에 typed value로 적용합니다.
	bool ApplyCanonicalNumeric(
		UObject& TransientObject,
		const FCFBatchColumnDescriptor& Descriptor,
		const FString& CanonicalValue,
		FString& OutError)
	{
		// Transient object의 actual reflected numeric leaf입니다.
		FMutablePropertyView PropertyView;
		if (!ResolveMutablePropertyPath(TransientObject, Descriptor.PropertyOrSemanticTarget, PropertyView, OutError)
			|| !PropertyView.Property
			|| !PropertyView.ValueAddress)
		{
			return false;
		}

				if (FInt8Property* Property = CastField<FInt8Property>(PropertyView.Property))
		{
			int64 Value = 0;
			if (!ParseStrictInt64(CanonicalValue, Value) || Value < MIN_int8 || Value > MAX_int8)
			{
				OutError = TEXT("int8 canonical value apply에 실패했습니다.");
				return false;
			}
			Property->SetPropertyValue(PropertyView.ValueAddress, static_cast<int8>(Value));
			return true;
		}
				if (FInt16Property* Property = CastField<FInt16Property>(PropertyView.Property))
		{
			int64 Value = 0;
			if (!ParseStrictInt64(CanonicalValue, Value) || Value < MIN_int16 || Value > MAX_int16)
			{
				OutError = TEXT("int16 canonical value apply에 실패했습니다.");
				return false;
			}
			Property->SetPropertyValue(PropertyView.ValueAddress, static_cast<int16>(Value));
			return true;
		}
				if (FIntProperty* Property = CastField<FIntProperty>(PropertyView.Property))
		{
			int64 Value = 0;
			if (!ParseStrictInt64(CanonicalValue, Value) || Value < MIN_int32 || Value > MAX_int32)
			{
				OutError = TEXT("int32 canonical value apply에 실패했습니다.");
				return false;
			}
			Property->SetPropertyValue(PropertyView.ValueAddress, static_cast<int32>(Value));
			return true;
		}
				if (FInt64Property* Property = CastField<FInt64Property>(PropertyView.Property))
		{
			int64 Value = 0;
			if (!ParseStrictInt64(CanonicalValue, Value))
			{
				OutError = TEXT("int64 canonical value apply에 실패했습니다.");
				return false;
			}
			Property->SetPropertyValue(PropertyView.ValueAddress, Value);
			return true;
		}
				if (FByteProperty* Property = CastField<FByteProperty>(PropertyView.Property))
		{
			uint64 Value = 0;
			if (Property->Enum || !ParseStrictUInt64(CanonicalValue, Value) || Value > MAX_uint8)
			{
				OutError = TEXT("uint8 canonical value apply에 실패했습니다.");
				return false;
			}
			Property->SetPropertyValue(PropertyView.ValueAddress, static_cast<uint8>(Value));
			return true;
		}
				if (FUInt16Property* Property = CastField<FUInt16Property>(PropertyView.Property))
		{
			uint64 Value = 0;
			if (!ParseStrictUInt64(CanonicalValue, Value) || Value > MAX_uint16)
			{
				OutError = TEXT("uint16 canonical value apply에 실패했습니다.");
				return false;
			}
			Property->SetPropertyValue(PropertyView.ValueAddress, static_cast<uint16>(Value));
			return true;
		}
				if (FUInt32Property* Property = CastField<FUInt32Property>(PropertyView.Property))
		{
			uint64 Value = 0;
			if (!ParseStrictUInt64(CanonicalValue, Value) || Value > MAX_uint32)
			{
				OutError = TEXT("uint32 canonical value apply에 실패했습니다.");
				return false;
			}
			Property->SetPropertyValue(PropertyView.ValueAddress, static_cast<uint32>(Value));
			return true;
		}
				if (FUInt64Property* Property = CastField<FUInt64Property>(PropertyView.Property))
		{
			uint64 Value = 0;
			if (!ParseStrictUInt64(CanonicalValue, Value))
			{
				OutError = TEXT("uint64 canonical value apply에 실패했습니다.");
				return false;
			}
			Property->SetPropertyValue(PropertyView.ValueAddress, Value);
			return true;
		}
				if (FFloatProperty* Property = CastField<FFloatProperty>(PropertyView.Property))
		{
			// Canonical decimal text의 strict parsed double입니다.
			double ParsedValue = 0.0;
			const bool bParsed = ParseStrictDouble(CanonicalValue, ParsedValue);
			const float FloatValue = static_cast<float>(ParsedValue);
			if (!bParsed || !FMath::IsFinite(FloatValue))
			{
				OutError = TEXT("float canonical value apply에 실패했습니다.");
				return false;
			}
			Property->SetPropertyValue(PropertyView.ValueAddress, FloatValue);
			return true;
		}
				if (FDoubleProperty* Property = CastField<FDoubleProperty>(PropertyView.Property))
		{
			// Canonical decimal text의 strict parsed double입니다.
			double ParsedValue = 0.0;
			if (!ParseStrictDouble(CanonicalValue, ParsedValue))
			{
				OutError = TEXT("double canonical value apply에 실패했습니다.");
				return false;
			}
			Property->SetPropertyValue(PropertyView.ValueAddress, ParsedValue);
			return true;
		}

		OutError = FString::Printf(TEXT("Batch candidate target은 scalar numeric leaf여야 합니다: %s"), *Descriptor.ColumnId);
		return false;
	}

	// FSoftObjectPath를 current loaded object 또는 TryLoad로 resolve합니다.
	UObject* ResolveObjectPath(const FString& ObjectPath)
	{
		// String identity에서 만든 soft path입니다.
		const FSoftObjectPath SoftPath(ObjectPath);
		if (!SoftPath.IsValid())
		{
			return nullptr;
		}
		// 이미 load된 current object입니다.
		UObject* Object = SoftPath.ResolveObject();
		if (!Object)
		{
			Object = SoftPath.TryLoad();
		}
		return Object;
	}

	// Manifest row identity로 current Recipe/Profile truth를 다시 projection합니다.
	bool BuildCurrentRowState(
		const FCFBatchManifest& Manifest,
		const FCFBatchManifestRow& ManifestRow,
		FCurrentRowState& OutState,
		FString& OutError)
	{
		OutState = FCurrentRowState();
		// P0-08J current row projection diagnostics입니다.
		TArray<FString> ExportErrors;
		if (Manifest.DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit)
		{
			// Current persistent Recipe입니다.
			UCFVehicleRecipeData* Recipe = Cast<UCFVehicleRecipeData>(ResolveObjectPath(ManifestRow.RecipePath));
			if (!Recipe)
			{
				OutError = FString::Printf(TEXT("Current Recipe를 찾지 못했습니다: %s"), *ManifestRow.RecipePath);
				return false;
			}
			OutState.AuthoringObject = Recipe;
			if (!FCFBatchExportService::BuildRecipeNumericRow(*Recipe, nullptr, OutState.CurrentRow, ExportErrors))
			{
				OutError = FString::Join(ExportErrors, TEXT(" | "));
				return false;
			}
		}
		else if (Manifest.DatasetKind == ECFBatchDatasetKind::ProfileNumericEdit)
		{
			// Current persistent typed Profile UObject입니다.
			UObject* ProfileObject = ResolveObjectPath(ManifestRow.ProfilePath);
			if (!ProfileObject)
			{
				OutError = FString::Printf(TEXT("Current Profile을 찾지 못했습니다: %s"), *ManifestRow.ProfilePath);
				return false;
			}
			OutState.AuthoringObject = ProfileObject;
			if (!FCFBatchExportService::BuildProfileNumericRow(*ProfileObject, Manifest.ProfileDomain, OutState.CurrentRow, ExportErrors))
			{
				OutError = FString::Join(ExportErrors, TEXT(" | "));
				return false;
			}
		}
		else
		{
			OutError = TEXT("Batch Import Preview는 RecipeNumericEdit/ProfileNumericEdit만 지원합니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Parsed row가 어느 immutable manifest row에 해당하는지 reserved identities로 복구합니다.
	const FCFBatchManifestRow* MatchManifestRow(const FCFBatchParsedRow& ParsedRow, const FCFBatchManifest& Manifest)
	{
		// Stable RowId exact match입니다.
		const FCFBatchManifestRow* RowIdMatch = Manifest.Rows.FindByPredicate([&ParsedRow](const FCFBatchManifestRow& Row)
		{
			return Row.RowId == ParsedRow.RowId;
		});
		if (RowIdMatch)
		{
			return RowIdMatch;
		}
		if (!ParsedRow.SourceIdentity.IsEmpty())
		{
			return Manifest.Rows.FindByPredicate([&ParsedRow, &Manifest](const FCFBatchManifestRow& Row)
			{
				return Manifest.DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit
					? Row.RecipePath == ParsedRow.SourceIdentity
					: Row.ProfilePath == ParsedRow.SourceIdentity;
			});
		}
		return nullptr;
	}

	// Recipe/Profile binding이 exact Profile identity를 사용하는지 검사합니다.
	bool IsRecipeBoundToProfile(
		const UCFVehicleRecipeData& Recipe,
		const ECFVehicleProfileDomain Domain,
		const FSoftObjectPath& ProfilePath)
	{
		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			return Recipe.ProfileBindings.VehicleBaseProfile.ToSoftObjectPath() == ProfilePath;
		case ECFVehicleProfileDomain::Drivetrain:
			return Recipe.ProfileBindings.DrivetrainProfile.ToSoftObjectPath() == ProfilePath;
		case ECFVehicleProfileDomain::Handling:
			return Recipe.ProfileBindings.HandlingProfile.ToSoftObjectPath() == ProfilePath;
		case ECFVehicleProfileDomain::Performance:
			return Recipe.ProfileBindings.PerformanceProfile.ToSoftObjectPath() == ProfilePath;
		case ECFVehicleProfileDomain::DriveState:
			return Recipe.ProfileBindings.DriveStateProfile.ToSoftObjectPath() == ProfilePath;
		default:
			return false;
		}
	}

	// Asset Registry + current live objects에서 exact shared Profile을 참조하는 Recipe inventory를 수집합니다.
	void GatherAffectedRecipes(
		const ECFVehicleProfileDomain Domain,
		const FString& ProfilePathText,
		TArray<UCFVehicleRecipeData*>& OutRecipes)
	{
		OutRecipes.Reset();
		// Exact shared Profile identity입니다.
		const FSoftObjectPath ProfilePath(ProfilePathText);
		// Recipe object path dedupe set입니다.
		TSet<FString> SeenRecipePaths;

		// Project Asset Registry 기반 managed Recipe inventory입니다.
		FCFVehicleListRequest ListRequest;
		ListRequest.bIncludeRecipeRecords = true;
		ListRequest.bIncludeUnmanagedDefinitions = false;
		// Existing Authoring facade R0 inventory 결과입니다.
		FCFVehicleListResult ListResult;
		if (FCFVehicleAuthoringService::ListVehicles(ListRequest, ListResult))
		{
			for (const FCFVehicleListEntry& Entry : ListResult.Vehicles)
			{
				if (!Entry.RecipePath.IsValid())
				{
					continue;
				}
				// Asset Registry entry의 current Recipe UObject입니다.
				UCFVehicleRecipeData* Recipe = Cast<UCFVehicleRecipeData>(Entry.RecipePath.ResolveObject());
				if (!Recipe)
				{
					Recipe = Cast<UCFVehicleRecipeData>(Entry.RecipePath.TryLoad());
				}
				if (!Recipe || !IsRecipeBoundToProfile(*Recipe, Domain, ProfilePath))
				{
					continue;
				}
				// Dedup에 사용할 current Recipe object path입니다.
				const FString RecipePath = FSoftObjectPath(Recipe).ToString();
				if (!SeenRecipePaths.Contains(RecipePath))
				{
					SeenRecipePaths.Add(RecipePath);
					OutRecipes.Add(Recipe);
				}
			}
		}

				// Automation / unsaved Editor lifetime의 non-transient live Recipe도 prospective inventory에서 누락하지 않습니다.
		for (TObjectIterator<UCFVehicleRecipeData> RecipeIt; RecipeIt; ++RecipeIt)
		{
			// Current live Recipe candidate입니다.
			UCFVehicleRecipeData* Recipe = *RecipeIt;
			// Rollback/prospective scratch는 live UObject여도 Authoring inventory의 실제 Recipe가 아닙니다.
			const bool bTransientScratch = Recipe
				&& (Recipe->HasAnyFlags(RF_Transient) || Recipe->GetOutermost() == GetTransientPackage());
			if (!Recipe
				|| Recipe->IsTemplate()
				|| Recipe->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject)
				|| bTransientScratch
				|| !IsRecipeBoundToProfile(*Recipe, Domain, ProfilePath))
			{
				continue;
			}
			// Live Recipe stable object identity입니다.
			const FString RecipePath = FSoftObjectPath(Recipe).ToString();
			if (!SeenRecipePaths.Contains(RecipePath))
			{
				SeenRecipePaths.Add(RecipePath);
				OutRecipes.Add(Recipe);
			}
		}

		OutRecipes.Sort([](const UCFVehicleRecipeData& Left, const UCFVehicleRecipeData& Right)
		{
			return FSoftObjectPath(&Left).ToString() < FSoftObjectPath(&Right).ToString();
		});
	}

	// Transient changed Profile UObject에서 one-domain prospective Snapshot을 만든 뒤 original asset identity를 복구합니다.
	bool BuildProspectiveProfileSnapshot(
		UObject& TransientProfile,
		const ECFVehicleProfileDomain Domain,
		const FString& OriginalProfilePath,
		FCFVehicleProfileSnapshotSet& OutSnapshot,
		FString& OutError)
	{
		OutSnapshot = FCFVehicleProfileSnapshotSet();
		bool bBuilt = false;
		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			bBuilt = FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(Cast<UCFVehicleBaseProfile>(&TransientProfile), nullptr, nullptr, nullptr, nullptr, OutSnapshot, OutError);
			if (bBuilt)
			{
				OutSnapshot.BaseSource.SourceObjectPath = FSoftObjectPath(OriginalProfilePath);
			}
			break;
		case ECFVehicleProfileDomain::Drivetrain:
			bBuilt = FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(nullptr, Cast<UCFDrivetrainProfile>(&TransientProfile), nullptr, nullptr, nullptr, OutSnapshot, OutError);
			if (bBuilt)
			{
				OutSnapshot.DrivetrainSource.SourceObjectPath = FSoftObjectPath(OriginalProfilePath);
			}
			break;
		case ECFVehicleProfileDomain::Handling:
			bBuilt = FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(nullptr, nullptr, Cast<UCFHandlingProfile>(&TransientProfile), nullptr, nullptr, OutSnapshot, OutError);
			if (bBuilt)
			{
				OutSnapshot.HandlingSource.SourceObjectPath = FSoftObjectPath(OriginalProfilePath);
			}
			break;
		case ECFVehicleProfileDomain::Performance:
			bBuilt = FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(nullptr, nullptr, nullptr, Cast<UCFPerformanceProfile>(&TransientProfile), nullptr, OutSnapshot, OutError);
			if (bBuilt)
			{
				OutSnapshot.PerformanceSource.SourceObjectPath = FSoftObjectPath(OriginalProfilePath);
			}
			break;
		case ECFVehicleProfileDomain::DriveState:
			bBuilt = FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(nullptr, nullptr, nullptr, nullptr, Cast<UCFDriveStateProfile>(&TransientProfile), OutSnapshot, OutError);
			if (bBuilt)
			{
				OutSnapshot.DriveStateSource.SourceObjectPath = FSoftObjectPath(OriginalProfilePath);
			}
			break;
		default:
			OutError = TEXT("지원하지 않는 Profile Domain입니다.");
			return false;
		}
		return bBuilt;
	}

	// Current resolve request의 exact Domain Profile snapshot만 prospective one-domain payload로 교체합니다.
	void SubstituteProfileSnapshot(
		FCFVehicleResolveRequest& InOutRequest,
		const ECFVehicleProfileDomain Domain,
		const FCFVehicleProfileSnapshotSet& ProspectiveProfile)
	{
		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			InOutRequest.Profiles.BaseSource = ProspectiveProfile.BaseSource;
			InOutRequest.Profiles.BaseData = ProspectiveProfile.BaseData;
			break;
		case ECFVehicleProfileDomain::Drivetrain:
			InOutRequest.Profiles.DrivetrainSource = ProspectiveProfile.DrivetrainSource;
			InOutRequest.Profiles.DrivetrainData = ProspectiveProfile.DrivetrainData;
			break;
		case ECFVehicleProfileDomain::Handling:
			InOutRequest.Profiles.HandlingSource = ProspectiveProfile.HandlingSource;
			InOutRequest.Profiles.HandlingData = ProspectiveProfile.HandlingData;
			break;
		case ECFVehicleProfileDomain::Performance:
			InOutRequest.Profiles.PerformanceSource = ProspectiveProfile.PerformanceSource;
			InOutRequest.Profiles.PerformanceData = ProspectiveProfile.PerformanceData;
			break;
		case ECFVehicleProfileDomain::DriveState:
			InOutRequest.Profiles.DriveStateSource = ProspectiveProfile.DriveStateSource;
			InOutRequest.Profiles.DriveStateData = ProspectiveProfile.DriveStateData;
			break;
		default:
			break;
		}
	}

	// Resolver validation 3 layer를 one prospective Vehicle summary에 집계합니다.
	void AccumulateValidation(const FCFVehicleResolveResult& ResolveResult, FCFBatchVehiclePreview& OutPreview)
	{
		// 한 validation 배열을 severity count로 누적하는 helper입니다.
		auto AccumulateIssues = [&OutPreview](const TArray<FCFVehicleValidationIssue>& Issues)
		{
			for (const FCFVehicleValidationIssue& Issue : Issues)
			{
				switch (Issue.Severity)
				{
				case ECFVehicleValidationSeverity::Info:
					++OutPreview.ValidationInfoCount;
					break;
				case ECFVehicleValidationSeverity::Warning:
					++OutPreview.ValidationWarningCount;
					break;
				case ECFVehicleValidationSeverity::Blocked:
					++OutPreview.ValidationBlockedCount;
					break;
				case ECFVehicleValidationSeverity::Error:
					++OutPreview.ValidationErrorCount;
					break;
				default:
					break;
				}
			}
		};
		AccumulateIssues(ResolveResult.RecipeValidation);
		AccumulateIssues(ResolveResult.ResolverValidation);
		AccumulateIssues(ResolveResult.DefinitionValidation);
	}

	// Resolver result를 Section 26.43 per-Vehicle compact preview로 투영합니다.
	FCFBatchVehiclePreview BuildVehiclePreview(
		const FString& RecipePath,
		const FString& TargetPath,
		const FCFVehicleResolveResult& CurrentResolve,
		const FCFVehicleResolveResult& ProspectiveResolve)
	{
		// Prospective one-Vehicle review입니다.
		FCFBatchVehiclePreview Preview;
		Preview.RecipePath = RecipePath;
		Preview.TargetPath = TargetPath;
		Preview.CurrentResolvedDefinitionHash = CurrentResolve.ResolvedDefinitionHash;
		Preview.ProspectiveResolvedDefinitionHash = ProspectiveResolve.ResolvedDefinitionHash;
		Preview.ProspectiveSourceSignature = ProspectiveResolve.SourceSignature;
				Preview.DefinitionDiffCount = ProspectiveResolve.FieldDiff.Num();
		Preview.ArrayStructuralChangeCount = 0;
		for (const FCFVehicleFieldDiff& Diff : ProspectiveResolve.FieldDiff)
		{
			if (Diff.Operation != ECFVehicleDiffOp::SetLeaf)
			{
				++Preview.ArrayStructuralChangeCount;
			}
		}
		Preview.bExternalDrift = ProspectiveResolve.StaleReport.bHasExternalDrift;
		Preview.ResolveStatus = ProspectiveResolve.ResolveStatus;
		AccumulateValidation(ProspectiveResolve, Preview);
		return Preview;
	}

	// SafeCandidate cells만 transient UObject에 descriptor-typed numeric patch로 적용합니다.
	bool ApplyCandidateCells(
		UObject& TransientObject,
		const TArray<FCFBatchColumnDescriptor>& Columns,
		const FCFBatchRowPreview& RowPreview,
		FString& OutError)
	{
		for (const FCFBatchCellReview& CellReview : RowPreview.CellReviews)
		{
			if (CellReview.MergeState != ECFBatchCellMergeState::SafeCandidate)
			{
				continue;
			}
			// Stable ColumnId에 대응하는 current projection descriptor입니다.
			const FCFBatchColumnDescriptor* Descriptor = FindDescriptor(Columns, CellReview.ColumnId);
			if (!Descriptor
				|| Descriptor->Access != ECFBatchColumnAccess::Editable
				|| Descriptor->TypedMutationKind == ECFBatchMutationKind::None)
			{
				OutError = FString::Printf(TEXT("SafeCandidate Column descriptor가 typed editable contract가 아닙니다: %s"), *CellReview.ColumnId);
				return false;
			}
			if (!ApplyCanonicalNumeric(TransientObject, *Descriptor, CellReview.EditedCanonicalValue, OutError))
			{
				return false;
			}
		}
		OutError.Reset();
		return true;
	}

	// Recipe numeric candidate를 transient duplicate에 적용하고 existing Authoring facade/Pure Resolver로 preview합니다.
	bool BuildRecipeProspectivePreview(
		UCFVehicleRecipeData& CurrentRecipe,
		const TArray<FCFBatchColumnDescriptor>& Columns,
		FCFBatchRowPreview& InOutRow,
		FString& OutError)
	{
		// Current persistent Recipe의 shared Resolver baseline입니다.
		FCFVehicleAuthoringReadRequest CurrentRequest;
		CurrentRequest.Recipe = &CurrentRecipe;
		CurrentRequest.CallerKind = ECFAuthoringCallerKind::Automation;
		// Current shared Resolver result입니다.
		FCFVehicleResolveReadResult CurrentResolve;
		if (!FCFVehicleAuthoringService::ResolveVehiclePreview(CurrentRequest, CurrentResolve))
		{
			OutError = CurrentResolve.Operation.Message;
			return false;
		}

		// Persistent Recipe를 절대 Modify하지 않는 transient duplicate입니다.
		TStrongObjectPtr<UCFVehicleRecipeData> ProspectiveRecipe(DuplicateObject<UCFVehicleRecipeData>(&CurrentRecipe, GetTransientPackage(), NAME_None));
		if (!ProspectiveRecipe.IsValid())
		{
			OutError = TEXT("Transient Recipe duplicate를 만들지 못했습니다.");
			return false;
		}
		ProspectiveRecipe->SetFlags(RF_Transient);
		// UCFVehicleRecipeData::PostDuplicate가 새 ID를 만들므로 prospective value copy에는 원본 identity를 복구합니다.
		ProspectiveRecipe->RecipeId = CurrentRecipe.RecipeId;
		if (!ApplyCandidateCells(*ProspectiveRecipe.Get(), Columns, InOutRow, OutError))
		{
			return false;
		}

		// Transient patched Recipe를 같은 facade/Pure Resolver path로 계산합니다.
		FCFVehicleAuthoringReadRequest ProspectiveRequest;
		ProspectiveRequest.Recipe = ProspectiveRecipe.Get();
		ProspectiveRequest.CallerKind = ECFAuthoringCallerKind::Automation;
		// Prospective shared Resolver output입니다.
		FCFVehicleResolveReadResult ProspectiveResolve;
		if (!FCFVehicleAuthoringService::ResolveVehiclePreview(ProspectiveRequest, ProspectiveResolve))
		{
			OutError = ProspectiveResolve.Operation.Message;
			return false;
		}

		InOutRow.VehiclePreviews.Add(BuildVehiclePreview(
			InOutRow.RecipePath,
			InOutRow.TargetPath,
			CurrentResolve.ResolveResult,
			ProspectiveResolve.ResolveResult));
		OutError.Reset();
		return true;
	}

	// Profile numeric candidate를 transient duplicate에 적용하고 affected Recipe 각각을 same Pure Resolver로 preview합니다.
	bool BuildProfileProspectivePreview(
		UObject& CurrentProfile,
		const TArray<FCFBatchColumnDescriptor>& Columns,
		FCFBatchRowPreview& InOutRow,
		FString& OutError)
	{
		// Persistent Profile을 절대 Modify하지 않는 transient dynamic-class duplicate입니다.
		TStrongObjectPtr<UObject> ProspectiveProfile(DuplicateObject<UObject>(&CurrentProfile, GetTransientPackage(), NAME_None));
		if (!ProspectiveProfile.IsValid())
		{
			OutError = TEXT("Transient Profile duplicate를 만들지 못했습니다.");
			return false;
		}
		ProspectiveProfile->SetFlags(RF_Transient);
		if (!ApplyCandidateCells(*ProspectiveProfile.Get(), Columns, InOutRow, OutError))
		{
			return false;
		}

		// Same Profile identity에 prospective typed payload/fingerprint만 가진 one-domain snapshot입니다.
		FCFVehicleProfileSnapshotSet ProspectiveProfileSnapshot;
		if (!BuildProspectiveProfileSnapshot(
			*ProspectiveProfile.Get(),
			InOutRow.ProfileDomain,
			InOutRow.ProfilePath,
			ProspectiveProfileSnapshot,
			OutError))
		{
			return false;
		}

		// Current Project + live Editor lifetime에서 exact shared Profile을 참조하는 affected Recipe inventory입니다.
		TArray<UCFVehicleRecipeData*> AffectedRecipes;
		GatherAffectedRecipes(InOutRow.ProfileDomain, InOutRow.ProfilePath, AffectedRecipes);
		for (UCFVehicleRecipeData* Recipe : AffectedRecipes)
		{
			if (!Recipe)
			{
				continue;
			}
			// Current persistent Recipe의 existing facade Resolver baseline입니다.
			FCFVehicleAuthoringReadRequest CurrentRequest;
			CurrentRequest.Recipe = Recipe;
			CurrentRequest.CallerKind = ECFAuthoringCallerKind::Automation;
			// Current immutable request/result입니다.
			FCFVehicleResolveReadResult CurrentResolve;
			if (!FCFVehicleAuthoringService::ResolveVehiclePreview(CurrentRequest, CurrentResolve))
			{
				OutError = CurrentResolve.Operation.Message;
				return false;
			}

			// Current immutable request를 복사하고 exact Domain Profile snapshot만 prospective payload로 교체합니다.
			FCFVehicleResolveRequest ProspectiveRequest = CurrentResolve.ResolveRequest;
			SubstituteProfileSnapshot(ProspectiveRequest, InOutRow.ProfileDomain, ProspectiveProfileSnapshot);
			ProspectiveRequest.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
			// Existing Pure Resolver의 prospective result입니다.
			FCFVehicleResolveResult ProspectiveResolve;
			if (!FCFVehicleResolver::Resolve(ProspectiveRequest, ProspectiveResolve))
			{
				OutError = FString::Printf(TEXT("Affected Recipe prospective Pure Resolver internal failure: %s"), *FSoftObjectPath(Recipe).ToString());
				return false;
			}

			InOutRow.VehiclePreviews.Add(BuildVehiclePreview(
				FSoftObjectPath(Recipe).ToString(),
				ProspectiveRequest.Recipe.TargetVehicleDataPath.ToString(),
				CurrentResolve.ResolveResult,
				ProspectiveResolve));
		}
		InOutRow.VehiclePreviews.Sort([](const FCFBatchVehiclePreview& Left, const FCFBatchVehiclePreview& Right)
		{
			return Left.RecipePath + TEXT("|") + Left.TargetPath < Right.RecipePath + TEXT("|") + Right.TargetPath;
		});
		OutError.Reset();
		return true;
	}

	// Prospective output을 바탕으로 Candidate/Shadow/Blocked/ExternalDrift terminal row status를 정합니다.
	void FinalizeProspectiveRowStatus(FCFBatchRowPreview& InOutRow)
	{
		if (HasConflictIssue(InOutRow.Issues))
		{
			InOutRow.Status = ECFBatchRowStatus::Conflict;
			return;
		}
		if (HasBlockingIssue(InOutRow.Issues))
		{
			InOutRow.Status = ECFBatchRowStatus::Blocked;
			return;
		}
		if (InOutRow.CandidateCellCount <= 0)
		{
			InOutRow.Status = InOutRow.bTargetHashChangedSinceExport
				? ECFBatchRowStatus::ExternalDrift
				: ECFBatchRowStatus::Unchanged;
			return;
		}

		// Prospective validation/block 여부입니다.
		const bool bValidationBlocked = InOutRow.VehiclePreviews.ContainsByPredicate([](const FCFBatchVehiclePreview& Preview)
		{
			return Preview.ResolveStatus == ECFVehicleResolveStatus::Blocked
				|| Preview.ResolveStatus == ECFVehicleResolveStatus::Error
				|| Preview.ValidationBlockedCount > 0
				|| Preview.ValidationErrorCount > 0;
		});
		if (bValidationBlocked)
		{
			AddIssue(InOutRow.Issues, ECFBatchIssueCode::ValidationBlocked, ECFBatchIssueSeverity::Blocked,
				TEXT("Prospective Resolver/Definition validation이 candidate authoring change를 Block했습니다."), InOutRow.RowId);
			InOutRow.Status = ECFBatchRowStatus::Blocked;
			return;
		}

		// Numeric edit에서 예상하지 않은 array structural Diff 여부입니다.
		const bool bHasArrayStructuralChange = InOutRow.VehiclePreviews.ContainsByPredicate([](const FCFBatchVehiclePreview& Preview)
		{
			return Preview.ArrayStructuralChangeCount > 0;
		});
		if (bHasArrayStructuralChange)
		{
			AddIssue(InOutRow.Issues, ECFBatchIssueCode::UnexpectedArrayStructuralChange, ECFBatchIssueSeverity::Blocked,
				TEXT("P0 numeric Batch prospective preview에서 예상하지 않은 array structural change가 발생했습니다."), InOutRow.RowId);
			InOutRow.Status = ECFBatchRowStatus::Blocked;
			return;
		}

		// Candidate authoring change가 effective resolved Definition을 실제 바꾸는지 여부입니다.
		const bool bHasEffectiveImpact = InOutRow.VehiclePreviews.ContainsByPredicate([](const FCFBatchVehiclePreview& Preview)
		{
			return Preview.CurrentResolvedDefinitionHash != Preview.ProspectiveResolvedDefinitionHash;
		});
		// Affected Vehicle 중 external drift가 존재하는지 여부입니다.
		const bool bHasExternalDrift = InOutRow.VehiclePreviews.ContainsByPredicate([](const FCFBatchVehiclePreview& Preview)
		{
			return Preview.bExternalDrift;
		});
		if (bHasExternalDrift || InOutRow.bTargetHashChangedSinceExport)
		{
			InOutRow.Status = ECFBatchRowStatus::ExternalDrift;
			return;
		}
		InOutRow.Status = bHasEffectiveImpact ? ECFBatchRowStatus::Candidate : ECFBatchRowStatus::ShadowOnly;
	}

	// Session rows/prospective vehicles를 Section 26.42 summary로 집계합니다.
	void BuildSummary(FCFBatchImportSession& InOutSession)
	{
		InOutSession.Summary = FCFBatchPreviewSummary();
		InOutSession.Summary.DatasetKind = InOutSession.DatasetKind;
		// Distinct affected Recipe identities입니다.
		TSet<FString> AffectedRecipes;
		// Distinct affected Profile identities입니다.
		TSet<FString> AffectedProfiles;
		// Distinct affected Vehicle identities입니다.
		TSet<FString> AffectedVehicles;

		for (const FCFBatchRowPreview& Row : InOutSession.Rows)
		{
			if (Row.SpreadsheetChangedCellCount > 0)
			{
				++InOutSession.Summary.EditedRowCount;
			}
			switch (Row.Status)
			{
			case ECFBatchRowStatus::Candidate:
				++InOutSession.Summary.CandidateRowCount;
				break;
			case ECFBatchRowStatus::Unchanged:
				++InOutSession.Summary.UnchangedRowCount;
				break;
			case ECFBatchRowStatus::ShadowOnly:
				++InOutSession.Summary.ShadowOnlyRowCount;
				break;
			case ECFBatchRowStatus::Warning:
				++InOutSession.Summary.WarningRowCount;
				break;
			case ECFBatchRowStatus::Blocked:
				++InOutSession.Summary.BlockedRowCount;
				break;
			case ECFBatchRowStatus::Conflict:
				++InOutSession.Summary.ConflictRowCount;
				break;
			case ECFBatchRowStatus::ExternalDrift:
				++InOutSession.Summary.ExternalDriftRowCount;
				break;
			default:
				break;
			}

			if (!Row.RecipePath.IsEmpty())
			{
				AffectedRecipes.Add(Row.RecipePath);
			}
			if (!Row.ProfilePath.IsEmpty())
			{
				AffectedProfiles.Add(Row.ProfilePath);
			}
			for (const FCFBatchVehiclePreview& VehiclePreview : Row.VehiclePreviews)
			{
				if (!VehiclePreview.RecipePath.IsEmpty())
				{
					AffectedRecipes.Add(VehiclePreview.RecipePath);
				}
				if (!VehiclePreview.TargetPath.IsEmpty())
				{
					AffectedVehicles.Add(VehiclePreview.TargetPath);
				}
				InOutSession.Summary.ProspectiveDefinitionChangedFieldCount += VehiclePreview.DefinitionDiffCount;
				InOutSession.Summary.ArrayStructuralChangeCount += VehiclePreview.ArrayStructuralChangeCount;
				InOutSession.Summary.ValidationWarningCount += VehiclePreview.ValidationWarningCount;
				InOutSession.Summary.ValidationBlockedCount += VehiclePreview.ValidationBlockedCount;
				InOutSession.Summary.ValidationErrorCount += VehiclePreview.ValidationErrorCount;
				if (VehiclePreview.bExternalDrift)
				{
					++InOutSession.Summary.ExternalDriftCount;
				}
			}
		}
				InOutSession.Summary.AffectedRecipeCount = AffectedRecipes.Num();
		InOutSession.Summary.AffectedProfileCount = AffectedProfiles.Num();
		InOutSession.Summary.AffectedVehicleCount = AffectedVehicles.Num();
	}

	/** Global preflight가 mutation 전에 확정하는 한 persistent Authoring Source 실행 계획입니다. */
	struct FCommitObjectPlan
	{
		// Exact reviewed Batch row입니다.
		const FCFBatchRowPreview* Row = nullptr;

		// Current persistent Recipe/Profile UObject입니다.
		UObject* SourceObject = nullptr;

		// Deterministic transaction order에 사용할 source identity입니다.
		FString SourceIdentity;

		// Typed patch 후 예상되는 semantic Recipe/Profile fingerprint입니다.
		FString ExpectedPostFingerprint;

		// Commit 전 diagnostic AuthoringRevision입니다.
		int32 OriginalAuthoringRevision = 0;

		// Commit 전 package dirty 상태입니다.
		bool bPackageWasDirty = false;

		// Rollback backup 배열 index입니다.
		int32 BackupIndex = INDEX_NONE;
	};

		// K의 downstream Definition validation issue를 제외하고 B1/B2 source transaction 자체를 막는 row issue인지 판정합니다.
	bool HasSourceCommitBlockingIssue(const FCFBatchRowPreview& Row)
	{
		for (const FCFBatchIssue& Issue : Row.Issues)
		{
			if (Issue.Severity == ECFBatchIssueSeverity::Conflict)
			{
				return true;
			}
			if (Issue.Severity == ECFBatchIssueSeverity::Blocked
				&& Issue.Code != ECFBatchIssueCode::ValidationBlocked)
			{
				return true;
			}
		}
		return false;
	}

	// B1/B2 source transaction 대상 row인지 판정합니다. ExternalDrift/ShadowOnly/downstream Definition blocker는 source blocker가 아닙니다.
	bool IsCommitCandidateRow(const FCFBatchRowPreview& Row)
	{
		return Row.CandidateCellCount > 0
			&& !HasSourceCommitBlockingIssue(Row);
	}


	// Dataset/Domain이 기대하는 exact persistent source class인지 검사합니다.
	bool IsExpectedSourceType(
		const UObject& SourceObject,
		const ECFBatchDatasetKind DatasetKind,
		const ECFVehicleProfileDomain ProfileDomain)
	{
		if (DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit)
		{
			return SourceObject.IsA<UCFVehicleRecipeData>();
		}
		if (DatasetKind != ECFBatchDatasetKind::ProfileNumericEdit)
		{
			return false;
		}

		switch (ProfileDomain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			return SourceObject.IsA<UCFVehicleBaseProfile>();
		case ECFVehicleProfileDomain::Drivetrain:
			return SourceObject.IsA<UCFDrivetrainProfile>();
		case ECFVehicleProfileDomain::Handling:
			return SourceObject.IsA<UCFHandlingProfile>();
		case ECFVehicleProfileDomain::Performance:
			return SourceObject.IsA<UCFPerformanceProfile>();
		case ECFVehicleProfileDomain::DriveState:
			return SourceObject.IsA<UCFDriveStateProfile>();
		default:
			return false;
		}
	}

	// Profile UObject에서 Domain별 shared authoring metadata 주소를 반환합니다.
	FCFVehicleProfileMeta* GetProfileMeta(UObject& ProfileObject, const ECFVehicleProfileDomain ProfileDomain)
	{
		switch (ProfileDomain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			if (UCFVehicleBaseProfile* Profile = Cast<UCFVehicleBaseProfile>(&ProfileObject))
			{
				return &Profile->Meta;
			}
			break;
		case ECFVehicleProfileDomain::Drivetrain:
			if (UCFDrivetrainProfile* Profile = Cast<UCFDrivetrainProfile>(&ProfileObject))
			{
				return &Profile->Meta;
			}
			break;
		case ECFVehicleProfileDomain::Handling:
			if (UCFHandlingProfile* Profile = Cast<UCFHandlingProfile>(&ProfileObject))
			{
				return &Profile->Meta;
			}
			break;
		case ECFVehicleProfileDomain::Performance:
			if (UCFPerformanceProfile* Profile = Cast<UCFPerformanceProfile>(&ProfileObject))
			{
				return &Profile->Meta;
			}
			break;
		case ECFVehicleProfileDomain::DriveState:
			if (UCFDriveStateProfile* Profile = Cast<UCFDriveStateProfile>(&ProfileObject))
			{
				return &Profile->Meta;
			}
			break;
		default:
			break;
		}
		return nullptr;
	}

	// Current Recipe/Profile AuthoringRevision을 공통 형태로 읽습니다.
	bool GetSourceAuthoringRevision(
		UObject& SourceObject,
		const ECFBatchDatasetKind DatasetKind,
		const ECFVehicleProfileDomain ProfileDomain,
		int32& OutRevision,
		FString& OutError)
	{
		if (DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit)
		{
			// Current persistent Recipe입니다.
			UCFVehicleRecipeData* Recipe = Cast<UCFVehicleRecipeData>(&SourceObject);
			if (!Recipe)
			{
				OutError = TEXT("B1 source object가 UCFVehicleRecipeData가 아닙니다.");
				return false;
			}
			OutRevision = Recipe->AuthoringRevision;
			OutError.Reset();
			return true;
		}

		// Current Profile metadata입니다.
		FCFVehicleProfileMeta* ProfileMeta = GetProfileMeta(SourceObject, ProfileDomain);
		if (!ProfileMeta)
		{
			OutError = TEXT("B2 source object의 Profile Domain/type이 일치하지 않습니다.");
			return false;
		}
		OutRevision = ProfileMeta->AuthoringRevision;
		OutError.Reset();
		return true;
	}

	// B1/B2 성공 source object의 diagnostic AuthoringRevision을 정확히 한 번 증가시킵니다.
	bool IncrementSourceAuthoringRevision(
		UObject& SourceObject,
		const ECFBatchDatasetKind DatasetKind,
		const ECFVehicleProfileDomain ProfileDomain,
		FString& OutError)
	{
		if (DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit)
		{
			// Current persistent Recipe입니다.
			UCFVehicleRecipeData* Recipe = Cast<UCFVehicleRecipeData>(&SourceObject);
			if (!Recipe)
			{
				OutError = TEXT("B1 revision update source type이 잘못됐습니다.");
				return false;
			}
			++Recipe->AuthoringRevision;
			OutError.Reset();
			return true;
		}

		// Current Profile metadata입니다.
		FCFVehicleProfileMeta* ProfileMeta = GetProfileMeta(SourceObject, ProfileDomain);
		if (!ProfileMeta)
		{
			OutError = TEXT("B2 revision update Profile Domain/type이 잘못됐습니다.");
			return false;
		}
		++ProfileMeta->AuthoringRevision;
		OutError.Reset();
		return true;
	}

	// 이미 resolve된 Recipe/Profile UObject를 P0-08J current row projection으로 다시 읽습니다.
	bool BuildCurrentAuthoringRow(
		UObject& SourceObject,
		const ECFBatchDatasetKind DatasetKind,
		const ECFVehicleProfileDomain ProfileDomain,
		FCFBatchExportRow& OutRow,
		FString& OutError)
	{
		// P0-08J row projection diagnostics입니다.
		TArray<FString> ExportErrors;
		if (DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit)
		{
			// Exact B1 Recipe입니다.
			UCFVehicleRecipeData* Recipe = Cast<UCFVehicleRecipeData>(&SourceObject);
			if (!Recipe || !FCFBatchExportService::BuildRecipeNumericRow(*Recipe, nullptr, OutRow, ExportErrors))
			{
				OutError = !ExportErrors.IsEmpty() ? FString::Join(ExportErrors, TEXT(" | ")) : TEXT("B1 current Recipe row projection에 실패했습니다.");
				return false;
			}
			OutError.Reset();
			return true;
		}
		if (DatasetKind == ECFBatchDatasetKind::ProfileNumericEdit)
		{
			if (!FCFBatchExportService::BuildProfileNumericRow(SourceObject, ProfileDomain, OutRow, ExportErrors))
			{
				OutError = !ExportErrors.IsEmpty() ? FString::Join(ExportErrors, TEXT(" | ")) : TEXT("B2 current Profile row projection에 실패했습니다.");
				return false;
			}
			OutError.Reset();
			return true;
		}
		OutError = TEXT("B1/B2가 아닌 Dataset은 source row projection을 지원하지 않습니다.");
		return false;
	}

	// Same-class reflected UPROPERTY storage 전체를 transient backup에서 persistent source로 복원합니다.
	bool RestoreSourceFromBackup(UObject& SourceObject, const UObject& BackupObject, FString& OutError)
	{
		if (SourceObject.GetClass() != BackupObject.GetClass())
		{
			OutError = TEXT("Batch rollback backup class가 current source class와 다릅니다.");
			return false;
		}
		for (TFieldIterator<FProperty> PropertyIt(SourceObject.GetClass(), EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
		{
			// Same-class backup에서 복원할 reflected property입니다.
			FProperty* Property = *PropertyIt;
			Property->CopyCompleteValue_InContainer(&SourceObject, &BackupObject);
		}
		OutError.Reset();
		return true;
	}

	// Current source를 transient duplicate한 뒤 reviewed SafeCandidate patch를 적용해 expected post fingerprint를 계산합니다.
	bool BuildExpectedPostFingerprint(
		UObject& CurrentSource,
		const ECFBatchDatasetKind DatasetKind,
		const ECFVehicleProfileDomain ProfileDomain,
		const TArray<FCFBatchColumnDescriptor>& Columns,
		const FCFBatchRowPreview& Row,
		FString& OutFingerprint,
		FString& OutError)
	{
		// Persistent source를 수정하지 않는 transient duplicate입니다.
		TStrongObjectPtr<UObject> ProspectiveSource(DuplicateObject<UObject>(&CurrentSource, GetTransientPackage(), NAME_None));
		if (!ProspectiveSource.IsValid())
		{
			OutError = TEXT("B1/B2 expected post fingerprint용 transient source duplicate 생성에 실패했습니다.");
			return false;
		}
		ProspectiveSource->SetFlags(RF_Transient);
		if (DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit)
		{
			// DuplicateObject PostDuplicate가 바꾼 RecipeId를 source semantic identity로 복구합니다.
			UCFVehicleRecipeData* ProspectiveRecipe = Cast<UCFVehicleRecipeData>(ProspectiveSource.Get());
			// Original persistent Recipe입니다.
			const UCFVehicleRecipeData* CurrentRecipe = Cast<UCFVehicleRecipeData>(&CurrentSource);
			if (!ProspectiveRecipe || !CurrentRecipe)
			{
				OutError = TEXT("B1 prospective duplicate Recipe type이 잘못됐습니다.");
				return false;
			}
			ProspectiveRecipe->RecipeId = CurrentRecipe->RecipeId;
		}
		if (!ApplyCandidateCells(*ProspectiveSource.Get(), Columns, Row, OutError))
		{
			return false;
		}

		// Patched transient source의 canonical current-row projection입니다.
		FCFBatchExportRow ProspectiveRow;
		if (!BuildCurrentAuthoringRow(*ProspectiveSource.Get(), DatasetKind, ProfileDomain, ProspectiveRow, OutError))
		{
			return false;
		}
		OutFingerprint = ProspectiveRow.ObjectFingerprint;
		if (OutFingerprint.IsEmpty())
		{
			OutError = TEXT("B1/B2 expected post fingerprint가 비어 있습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Profile preview에 기록된 affected Recipe identity set을 canonical ascending unique list로 만듭니다.
	TArray<FString> BuildAffectedRecipeSet(const FCFBatchRowPreview& Row)
	{
		// Duplicate affected vehicle preview를 제거할 identity set입니다.
		TSet<FString> UniqueRecipePaths;
		for (const FCFBatchVehiclePreview& Preview : Row.VehiclePreviews)
		{
			if (!Preview.RecipePath.IsEmpty())
			{
				UniqueRecipePaths.Add(Preview.RecipePath);
			}
		}
		// Canonical ascending affected Recipe list입니다.
		TArray<FString> Result = UniqueRecipePaths.Array();
		Result.Sort();
		return Result;
	}

		// B1 commit 직전 patched Recipe의 Input/Recipe validation layer만 fresh 재검사합니다.
	bool ValidateRecipeCommitPreflight(
		UCFVehicleRecipeData& CurrentRecipe,
		const TArray<FCFBatchColumnDescriptor>& Columns,
		const FCFBatchRowPreview& ApprovedRow,
		FString& OutError)
	{
		// Persistent Recipe를 수정하지 않는 fresh prospective duplicate입니다.
		TStrongObjectPtr<UCFVehicleRecipeData> ProspectiveRecipe(DuplicateObject<UCFVehicleRecipeData>(&CurrentRecipe, GetTransientPackage(), NAME_None));
		if (!ProspectiveRecipe.IsValid())
		{
			OutError = TEXT("B1 Recipe validation용 transient duplicate 생성에 실패했습니다.");
			return false;
		}
		ProspectiveRecipe->SetFlags(RF_Transient);
		ProspectiveRecipe->RecipeId = CurrentRecipe.RecipeId;
		if (!ApplyCandidateCells(*ProspectiveRecipe.Get(), Columns, ApprovedRow, OutError))
		{
			return false;
		}

		// Existing Authoring facade가 current Target/Profiles/Assets를 fresh read하도록 요청합니다.
		FCFVehicleAuthoringReadRequest ResolveRequest;
		ResolveRequest.Recipe = ProspectiveRecipe.Get();
		ResolveRequest.CallerKind = ECFAuthoringCallerKind::Automation;
		// Existing Pure Resolver 결과를 포함한 fresh read입니다.
		FCFVehicleResolveReadResult ResolveRead;
		if (!FCFVehicleAuthoringService::ResolveVehiclePreview(ResolveRequest, ResolveRead))
		{
			OutError = ResolveRead.Operation.Message.IsEmpty()
				? TEXT("B1 Recipe fresh prospective Resolver request 생성에 실패했습니다.")
				: ResolveRead.Operation.Message;
			return false;
		}

		for (const FCFVehicleValidationIssue& Issue : ResolveRead.ResolveResult.RecipeValidation)
		{
			if (Issue.Severity == ECFVehicleValidationSeverity::Blocked
				|| Issue.Severity == ECFVehicleValidationSeverity::Error)
			{
								OutError = Issue.ValidatorFieldPath.IsEmpty()
					? TEXT("B1 patched Recipe validation이 Block됐습니다.")
					: FString::Printf(TEXT("B1 patched Recipe validation이 Block됐습니다: Field=%s"), *Issue.ValidatorFieldPath);

				return false;
			}
		}

		// Resolver/Definition validation과 External Drift는 B1 source edit blocker로 승격하지 않습니다.
		OutError.Reset();
		return true;
	}

	// B2 commit 직전 affected Recipe inventory와 prospective validation을 fresh current state로 재검사합니다.
	bool ValidateProfileCommitPreflight(
		UObject& CurrentProfile,
		const TArray<FCFBatchColumnDescriptor>& Columns,
		const FCFBatchRowPreview& ApprovedRow,
		ECFBatchCommitErrorCode& OutErrorCode,
		FString& OutError)
	{
		// Current state에서 다시 계산할 prospective Profile row copy입니다.
		FCFBatchRowPreview FreshRow = ApprovedRow;
		FreshRow.VehiclePreviews.Reset();
		if (!BuildProfileProspectivePreview(CurrentProfile, Columns, FreshRow, OutError))
		{
			OutErrorCode = ECFBatchCommitErrorCode::ProspectiveValidationBlocked;
			return false;
		}

		// Approved Preview 당시 affected Recipe exact identity set입니다.
		const TArray<FString> ApprovedAffectedRecipes = BuildAffectedRecipeSet(ApprovedRow);
		// Commit 직전 fresh affected Recipe exact identity set입니다.
		const TArray<FString> FreshAffectedRecipes = BuildAffectedRecipeSet(FreshRow);
		if (ApprovedAffectedRecipes != FreshAffectedRecipes)
		{
			OutErrorCode = ECFBatchCommitErrorCode::AffectedRecipeInventoryChanged;
			OutError = TEXT("Shared Profile의 affected Recipe inventory가 Preview 이후 변경됐습니다. 새 Batch Preview가 필요합니다.");
			return false;
		}

		for (const FCFBatchVehiclePreview& Preview : FreshRow.VehiclePreviews)
		{
			if (Preview.ResolveStatus == ECFVehicleResolveStatus::Blocked
				|| Preview.ResolveStatus == ECFVehicleResolveStatus::Error
				|| Preview.ValidationBlockedCount > 0
				|| Preview.ValidationErrorCount > 0
				|| Preview.ArrayStructuralChangeCount > 0)
			{
				OutErrorCode = ECFBatchCommitErrorCode::ProspectiveValidationBlocked;
				OutError = FString::Printf(TEXT("Shared Profile prospective validation이 commit 직전 Block됐습니다: Recipe=%s"), *Preview.RecipePath);
				return false;
			}
		}

		// External Drift/Effective Stale/Shadow-only는 B2 source commit blocker로 승격하지 않습니다.
		OutErrorCode = ECFBatchCommitErrorCode::None;
		OutError.Reset();
		return true;
	}

	// Commit result를 mutation 0 Blocked terminal로 설정합니다.
	void SetCommitBlocked(
		FCFBatchAuthoringCommitResult& OutResult,
		const ECFBatchCommitErrorCode ErrorCode,
		const FString& Message)
	{
		OutResult.Status = ECFBatchCommitStatus::Blocked;
		OutResult.ErrorCode = ErrorCode;
		OutResult.Message = Message;
		OutResult.bApprovalConsumed = false;
		OutResult.bRequiresFreshPreview = true;
		OutResult.bTargetMutationPerformed = false;
		OutResult.bSavePerformed = false;
	}
}

// Companion JSON을 FCFBatchManifest로 parse하고 current Registry/schema/hash contract까지 검증합니다.
bool FCFBatchImportService::ParseAndValidateManifest(
	const FString& ManifestJsonText,
	FCFBatchManifest& OutManifest,
	TArray<FCFBatchIssue>& OutIssues)
{
	OutManifest = FCFBatchManifest();
	OutIssues.Reset();
	// Leading Unicode BOM을 허용하되 semantic JSON에는 포함하지 않는 input copy입니다.
	FString JsonText = ManifestJsonText;
	if (!JsonText.IsEmpty() && JsonText[0] == 0xFEFF)
	{
		JsonText.RightChopInline(1, EAllowShrinking::No);
	}
	if (JsonText.IsEmpty())
	{
		CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ManifestParseError, ECFBatchIssueSeverity::Blocked,
			TEXT(".cfbatch.json text가 비어 있습니다."));
		return false;
	}

	// Manifest JSON reader입니다.
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	// Parsed root JSON object입니다.
	TSharedPtr<FJsonObject> RootObject;
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ManifestParseError, ECFBatchIssueSeverity::Blocked,
			TEXT(".cfbatch.json을 valid JSON object로 parse하지 못했습니다."));
		return false;
	}

	// Manifest protocol scalar fields입니다.
	FString BatchExportIdText;
	FString DatasetText;
	FString ProfileDomainText;
	if (!CFBatchImportPrivate::ReadRequiredString(RootObject, TEXT("BatchExportId"), BatchExportIdText)
		|| !CFBatchImportPrivate::ReadRequiredString(RootObject, TEXT("DatasetKind"), DatasetText)
		|| !CFBatchImportPrivate::ReadRequiredString(RootObject, TEXT("ProfileDomain"), ProfileDomainText)
		|| !CFBatchImportPrivate::ReadRequiredString(RootObject, TEXT("SchemaId"), OutManifest.SchemaId)
		|| !CFBatchImportPrivate::ReadRequiredInt(RootObject, TEXT("SchemaRevision"), OutManifest.SchemaRevision)
		|| !CFBatchImportPrivate::ReadRequiredInt(RootObject, TEXT("ResolverContractRevision"), OutManifest.ResolverContractRevision)
		|| !CFBatchImportPrivate::ReadRequiredString(RootObject, TEXT("ExportSetHash"), OutManifest.ExportSetHash)
		|| !FGuid::Parse(BatchExportIdText, OutManifest.BatchExportId)
		|| !CFBatchImportPrivate::ParseDataset(DatasetText, OutManifest.DatasetKind)
		|| !CFBatchImportPrivate::ParseDomain(ProfileDomainText, OutManifest.ProfileDomain))
	{
		CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ManifestParseError, ECFBatchIssueSeverity::Blocked,
			TEXT(".cfbatch.json required scalar field 또는 enum format이 잘못되었습니다."));
		return false;
	}

	// Canonical CSV header id JSON array입니다.
	const TArray<TSharedPtr<FJsonValue>>* CsvColumnValues = nullptr;
	if (!RootObject->TryGetArrayField(TEXT("CsvColumnIds"), CsvColumnValues) || !CsvColumnValues)
	{
		CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ManifestParseError, ECFBatchIssueSeverity::Blocked,
			TEXT("Manifest CsvColumnIds array가 없습니다."));
		return false;
	}
	for (const TSharedPtr<FJsonValue>& ColumnValue : *CsvColumnValues)
	{
		if (!ColumnValue.IsValid() || ColumnValue->Type != EJson::String)
		{
			CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ManifestParseError, ECFBatchIssueSeverity::Blocked,
				TEXT("Manifest CsvColumnIds에는 string만 허용됩니다."));
			return false;
		}
		OutManifest.CsvColumnIds.Add(ColumnValue->AsString());
	}

	// Editable descriptor JSON array입니다.
	const TArray<TSharedPtr<FJsonValue>>* DescriptorValues = nullptr;
	if (!RootObject->TryGetArrayField(TEXT("EditableColumnDescriptors"), DescriptorValues) || !DescriptorValues)
	{
		CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ManifestParseError, ECFBatchIssueSeverity::Blocked,
			TEXT("Manifest EditableColumnDescriptors array가 없습니다."));
		return false;
	}
	for (const TSharedPtr<FJsonValue>& DescriptorValue : *DescriptorValues)
	{
		// One editable descriptor JSON object입니다.
		const TSharedPtr<FJsonObject> DescriptorObject = DescriptorValue.IsValid() ? DescriptorValue->AsObject() : nullptr;
		// Parsed descriptor입니다.
		FCFBatchColumnDescriptor Descriptor;
		if (!DescriptorObject.IsValid()
			|| !CFBatchImportPrivate::ParseDescriptorObject(DescriptorObject, OutManifest.DatasetKind, OutManifest.ProfileDomain, Descriptor))
		{
			CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ManifestParseError, ECFBatchIssueSeverity::Blocked,
				TEXT("Manifest editable Column descriptor parse에 실패했습니다."));
			return false;
		}
		OutManifest.EditableColumns.Add(MoveTemp(Descriptor));
	}

	// Immutable export baseline row JSON array입니다.
	const TArray<TSharedPtr<FJsonValue>>* RowValues = nullptr;
	if (!RootObject->TryGetArrayField(TEXT("Rows"), RowValues) || !RowValues)
	{
		CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ManifestParseError, ECFBatchIssueSeverity::Blocked,
			TEXT("Manifest Rows array가 없습니다."));
		return false;
	}
	for (const TSharedPtr<FJsonValue>& RowValue : *RowValues)
	{
		// One manifest row object입니다.
		const TSharedPtr<FJsonObject> RowObject = RowValue.IsValid() ? RowValue->AsObject() : nullptr;
		// Parsed manifest row입니다.
		FCFBatchManifestRow Row;
		if (!RowObject.IsValid() || !CFBatchImportPrivate::ParseManifestRowObject(RowObject, Row))
		{
			CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ManifestParseError, ECFBatchIssueSeverity::Blocked,
				TEXT("Manifest baseline row parse에 실패했습니다."));
			return false;
		}
		OutManifest.Rows.Add(MoveTemp(Row));
	}

	OutManifest.EditableColumns.Sort([](const FCFBatchColumnDescriptor& Left, const FCFBatchColumnDescriptor& Right)
	{
		return Left.ColumnId < Right.ColumnId;
	});
	OutManifest.Rows.Sort([](const FCFBatchManifestRow& Left, const FCFBatchManifestRow& Right)
	{
		return Left.RowId < Right.RowId;
	});

	// P0-08J current Registry/schema/ExportSetHash consistency validation입니다.
	TArray<FString> ValidationErrors;
	if (!FCFBatchExportService::ValidateManifestBaseline(OutManifest, ValidationErrors))
	{
		for (const FString& ValidationError : ValidationErrors)
		{
			CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ManifestInvalid, ECFBatchIssueSeverity::Blocked, ValidationError);
		}
		return false;
	}
	return true;
}

// Standard quoted canonical CSV text를 stable ColumnId/header 기반 parsed rows로 변환합니다.
bool FCFBatchImportService::ParseCanonicalCsv(
	const FString& CsvText,
	const FCFBatchManifest& Manifest,
	TArray<FCFBatchParsedRow>& OutRows,
	TArray<FCFBatchIssue>& OutIssues)
{
	OutRows.Reset();
	OutIssues.Reset();
	// Physical CSV parse result입니다.
	CFBatchImportPrivate::FCsvTable CsvTable;
	// CSV state-machine parse diagnostic입니다.
	FString ParseError;
	if (!CFBatchImportPrivate::ParseCsvTable(CsvText, CsvTable, ParseError))
	{
		CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::CsvParseError, ECFBatchIssueSeverity::Blocked, ParseError);
		return false;
	}

	// Header duplicate/lookup map입니다.
	TMap<FString, int32> HeaderIndexById;
	for (int32 HeaderIndex = 0; HeaderIndex < CsvTable.Headers.Num(); ++HeaderIndex)
	{
		// Stable technical header id입니다.
		const FString& HeaderId = CsvTable.Headers[HeaderIndex];
		if (HeaderIndexById.Contains(HeaderId))
		{
			CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::CsvManifestColumnMismatch, ECFBatchIssueSeverity::Blocked,
				FString::Printf(TEXT("CSV header에 duplicate ColumnId가 있습니다: %s"), *HeaderId), FString(), HeaderId);
			return false;
		}
		HeaderIndexById.Add(HeaderId, HeaderIndex);
	}
	if (CsvTable.Headers.Num() != Manifest.CsvColumnIds.Num())
	{
		CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::CsvManifestColumnMismatch, ECFBatchIssueSeverity::Blocked,
			TEXT("CSV header column 수가 manifest와 다릅니다."));
		return false;
	}
	for (const FString& ManifestColumnId : Manifest.CsvColumnIds)
	{
		if (!HeaderIndexById.Contains(ManifestColumnId))
		{
			CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::CsvManifestColumnMismatch, ECFBatchIssueSeverity::Blocked,
				FString::Printf(TEXT("CSV header가 manifest ColumnId를 포함하지 않습니다: %s"), *ManifestColumnId), FString(), ManifestColumnId);
			return false;
		}
	}

	for (const TArray<FString>& PhysicalRow : CsvTable.Rows)
	{
		if (PhysicalRow.Num() != CsvTable.Headers.Num())
		{
			CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::CsvParseError, ECFBatchIssueSeverity::Blocked,
				FString::Printf(TEXT("CSV data row column 수가 header와 다릅니다. Expected=%d Actual=%d"), CsvTable.Headers.Num(), PhysicalRow.Num()));
			return false;
		}
		// Stable manifest ColumnId 순서로 다시 projection할 parsed row입니다.
		FCFBatchParsedRow ParsedRow;
		for (const FString& ManifestColumnId : Manifest.CsvColumnIds)
		{
			// Physical CSV의 exact header index입니다.
			const int32* PhysicalIndex = HeaderIndexById.Find(ManifestColumnId);
			if (!PhysicalIndex || !PhysicalRow.IsValidIndex(*PhysicalIndex))
			{
				CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::CsvParseError, ECFBatchIssueSeverity::Blocked,
					FString::Printf(TEXT("CSV header index resolve에 실패했습니다: %s"), *ManifestColumnId));
				return false;
			}
			// One canonical parsed cell입니다.
			FCFBatchParsedCell& ParsedCell = ParsedRow.Cells.AddDefaulted_GetRef();
			ParsedCell.ColumnId = ManifestColumnId;
			ParsedCell.RawValue = PhysicalRow[*PhysicalIndex];
		}
		// Reserved stable row identity입니다.
		const FCFBatchParsedCell* RowIdCell = CFBatchImportPrivate::FindParsedCell(ParsedRow, TEXT("__cf_row_id"));
		ParsedRow.RowId = RowIdCell ? RowIdCell->RawValue : FString();
		// Dataset-specific authoring source identity입니다.
		const TCHAR* SourceColumnId = Manifest.DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit
			? TEXT("__cf_recipe")
			: TEXT("__cf_profile");
		const FCFBatchParsedCell* SourceCell = CFBatchImportPrivate::FindParsedCell(ParsedRow, SourceColumnId);
		ParsedRow.SourceIdentity = SourceCell ? SourceCell->RawValue : FString();
		OutRows.Add(MoveTemp(ParsedRow));
	}

	// Input physical row order를 semantic precedence로 사용하지 않는 canonical identity sort입니다.
	OutRows.Sort([](const FCFBatchParsedRow& Left, const FCFBatchParsedRow& Right)
	{
		const FString LeftKey = Left.SourceIdentity + TEXT("|") + Left.RowId;
		const FString RightKey = Right.SourceIdentity + TEXT("|") + Right.RowId;
		return LeftKey < RightKey;
	});
	return true;
}

// Current Unreal truth를 다시 읽어 3-way merge와 Recipe/Profile prospective Pure Resolver preview Session을 만듭니다.
bool FCFBatchImportService::BuildPreview(
	const FCFBatchImportRequest& Request,
	FCFBatchImportSession& OutSession)
{
	OutSession = FCFBatchImportSession();
	// Manifest parse/schema issues입니다.
	TArray<FCFBatchIssue> ManifestIssues;
	if (!ParseAndValidateManifest(Request.ManifestJsonText, OutSession.Manifest, ManifestIssues))
	{
		OutSession.Issues = MoveTemp(ManifestIssues);
		OutSession.bFileBlocked = true;
		return false;
	}
	OutSession.DatasetKind = OutSession.Manifest.DatasetKind;
	OutSession.ProfileDomain = OutSession.Manifest.ProfileDomain;
	if (OutSession.DatasetKind != ECFBatchDatasetKind::RecipeNumericEdit
		&& OutSession.DatasetKind != ECFBatchDatasetKind::ProfileNumericEdit)
	{
		CFBatchImportPrivate::AddIssue(OutSession.Issues, ECFBatchIssueCode::UnsupportedDataset, ECFBatchIssueSeverity::Blocked,
			TEXT("P0 Batch Import Preview는 RecipeNumericEdit/ProfileNumericEdit만 지원합니다."));
		OutSession.bFileBlocked = true;
		return false;
	}

	// Canonical CSV parse issues입니다.
	TArray<FCFBatchIssue> CsvIssues;
	if (!ParseCanonicalCsv(Request.CsvText, OutSession.Manifest, OutSession.ParsedRows, CsvIssues))
	{
		OutSession.Issues.Append(CsvIssues);
		OutSession.bFileBlocked = true;
		return false;
	}

	// Current projection Registry descriptors입니다.
	TArray<FCFBatchColumnDescriptor> Columns;
	// Registry invariant diagnostics입니다.
	TArray<FString> RegistryErrors;
	if (!FCFBatchColumnRegistry::GetDatasetColumns(OutSession.DatasetKind, OutSession.ProfileDomain, Columns, RegistryErrors))
	{
		for (const FString& RegistryError : RegistryErrors)
		{
			CFBatchImportPrivate::AddIssue(OutSession.Issues, ECFBatchIssueCode::ManifestInvalid, ECFBatchIssueSeverity::Blocked, RegistryError);
		}
		OutSession.bFileBlocked = true;
		return false;
	}

	// Manifest 자체의 duplicate Recipe/Profile source identity를 탐지합니다.
	TSet<FString> ManifestSourceIdentities;
	for (const FCFBatchManifestRow& ManifestRow : OutSession.Manifest.Rows)
	{
		// Dataset-specific source identity입니다.
		const FString SourceIdentity = OutSession.DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit
			? ManifestRow.RecipePath
			: ManifestRow.ProfilePath;
		if (SourceIdentity.IsEmpty() || ManifestSourceIdentities.Contains(SourceIdentity))
		{
			CFBatchImportPrivate::AddIssue(OutSession.Issues, ECFBatchIssueCode::DuplicateRowIdentity, ECFBatchIssueSeverity::Blocked,
				FString::Printf(TEXT("Manifest에 empty/duplicate Recipe/Profile identity가 있습니다: %s"), *SourceIdentity), ManifestRow.RowId);
			OutSession.bFileBlocked = true;
		}
		else
		{
			ManifestSourceIdentities.Add(SourceIdentity);
		}
	}

	// CSV duplicate stable RowId 탐지 set입니다.
	TSet<FString> ParsedRowIds;
	// CSV duplicate Recipe/Profile identity 탐지 set입니다.
	TSet<FString> ParsedSourceIdentities;
	for (const FCFBatchParsedRow& ParsedRow : OutSession.ParsedRows)
	{
		if (ParsedRow.RowId.IsEmpty())
		{
			CFBatchImportPrivate::AddIssue(OutSession.Issues, ECFBatchIssueCode::MissingRowIdentity, ECFBatchIssueSeverity::Blocked,
				TEXT("CSV row의 __cf_row_id가 비어 있습니다."));
			OutSession.bFileBlocked = true;
		}
		else if (ParsedRowIds.Contains(ParsedRow.RowId))
		{
			CFBatchImportPrivate::AddIssue(OutSession.Issues, ECFBatchIssueCode::DuplicateRowIdentity, ECFBatchIssueSeverity::Blocked,
				FString::Printf(TEXT("CSV에 duplicate __cf_row_id가 있습니다: %s"), *ParsedRow.RowId), ParsedRow.RowId);
			OutSession.bFileBlocked = true;
		}
		else
		{
			ParsedRowIds.Add(ParsedRow.RowId);
		}

		if (ParsedRow.SourceIdentity.IsEmpty())
		{
			CFBatchImportPrivate::AddIssue(OutSession.Issues, ECFBatchIssueCode::MissingRowIdentity, ECFBatchIssueSeverity::Blocked,
				TEXT("CSV row의 Recipe/Profile reserved identity가 비어 있습니다."), ParsedRow.RowId);
			OutSession.bFileBlocked = true;
		}
		else if (ParsedSourceIdentities.Contains(ParsedRow.SourceIdentity))
		{
			CFBatchImportPrivate::AddIssue(OutSession.Issues, ECFBatchIssueCode::DuplicateRowIdentity, ECFBatchIssueSeverity::Blocked,
				FString::Printf(TEXT("CSV에 동일 Recipe/Profile identity가 두 번 존재합니다: %s"), *ParsedRow.SourceIdentity), ParsedRow.RowId);
			OutSession.bFileBlocked = true;
		}
		else
		{
			ParsedSourceIdentities.Add(ParsedRow.SourceIdentity);
		}
	}

	// 실제 CSV row가 match한 immutable manifest RowId 집합입니다.
	TSet<FString> MatchedManifestRowIds;
	for (const FCFBatchParsedRow& ParsedRow : OutSession.ParsedRows)
	{
		// Reserved identities로 복구한 export baseline row입니다.
		const FCFBatchManifestRow* ManifestRow = CFBatchImportPrivate::MatchManifestRow(ParsedRow, OutSession.Manifest);
		if (!ManifestRow)
		{
			CFBatchImportPrivate::AddIssue(OutSession.Issues, ECFBatchIssueCode::UnknownRowIdentity, ECFBatchIssueSeverity::Blocked,
				FString::Printf(TEXT("CSV row가 export manifest identity와 일치하지 않습니다: RowId=%s Source=%s"), *ParsedRow.RowId, *ParsedRow.SourceIdentity), ParsedRow.RowId);
			OutSession.bFileBlocked = true;
			continue;
		}
		if (MatchedManifestRowIds.Contains(ManifestRow->RowId))
		{
			CFBatchImportPrivate::AddIssue(OutSession.Issues, ECFBatchIssueCode::DuplicateRowIdentity, ECFBatchIssueSeverity::Blocked,
				FString::Printf(TEXT("둘 이상의 CSV row가 같은 export baseline row를 가리킵니다: %s"), *ManifestRow->RowId), ManifestRow->RowId);
			OutSession.bFileBlocked = true;
			continue;
		}
		MatchedManifestRowIds.Add(ManifestRow->RowId);

		// Section 26.43 row review입니다.
		FCFBatchRowPreview RowPreview;
		RowPreview.RowId = ManifestRow->RowId;
		RowPreview.TargetPath = ManifestRow->TargetPath;
		RowPreview.RecipePath = ManifestRow->RecipePath;
		RowPreview.ProfilePath = ManifestRow->ProfilePath;
		RowPreview.ProfileDomain = ManifestRow->ProfileDomain;
		RowPreview.ExportObjectFingerprint = ManifestRow->ObjectFingerprint;

		// Reserved/read-only modification은 silent ignore하지 않습니다.
		for (const FCFBatchColumnDescriptor& Descriptor : Columns)
		{
			if (!Descriptor.bReservedMetadata && Descriptor.Access != ECFBatchColumnAccess::ReadOnly)
			{
				continue;
			}
			// CSV가 실제 제공한 read-only/reserved text입니다.
			const FCFBatchParsedCell* ParsedCell = CFBatchImportPrivate::FindParsedCell(ParsedRow, Descriptor.ColumnId);
			if (!ParsedCell)
			{
				CFBatchImportPrivate::AddIssue(RowPreview.Issues, ECFBatchIssueCode::CsvManifestColumnMismatch, ECFBatchIssueSeverity::Blocked,
					TEXT("Read-only/reserved ColumnId cell을 찾지 못했습니다."), RowPreview.RowId, Descriptor.ColumnId);
				continue;
			}
			// Export baseline read-only value입니다.
			FString BaselineValue;
			if (Descriptor.bReservedMetadata)
			{
				BaselineValue = CFBatchImportPrivate::GetReservedBaselineValue(Descriptor, OutSession.Manifest, *ManifestRow);
			}
			else if (const FCFBatchBaselineCell* BaselineCell = CFBatchImportPrivate::FindBaselineCell(*ManifestRow, Descriptor.ColumnId))
			{
				BaselineValue = BaselineCell->CanonicalValue;
			}
			if (ParsedCell->RawValue != BaselineValue)
			{
				CFBatchImportPrivate::AddIssue(RowPreview.Issues, ECFBatchIssueCode::ReadOnlyColumnModified, ECFBatchIssueSeverity::Blocked,
					FString::Printf(TEXT("Read-only/reserved cell이 export baseline과 달라졌습니다: %s"), *Descriptor.ColumnId), RowPreview.RowId, Descriptor.ColumnId);
			}
		}

		// Import 시점 current Unreal authoring truth를 다시 읽습니다.
		CFBatchImportPrivate::FCurrentRowState CurrentState;
		// Current row projection diagnostic입니다.
		FString CurrentError;
		if (!CFBatchImportPrivate::BuildCurrentRowState(OutSession.Manifest, *ManifestRow, CurrentState, CurrentError))
		{
			CFBatchImportPrivate::AddIssue(RowPreview.Issues, ECFBatchIssueCode::CurrentObjectMissing, ECFBatchIssueSeverity::Blocked,
				CurrentError, RowPreview.RowId);
			RowPreview.Status = ECFBatchRowStatus::Blocked;
			OutSession.Rows.Add(MoveTemp(RowPreview));
			OutSession.bFileBlocked = true;
			continue;
		}
		RowPreview.CurrentObjectFingerprint = CurrentState.CurrentRow.ObjectFingerprint;
		RowPreview.bFingerprintChangedSinceExport = RowPreview.CurrentObjectFingerprint != RowPreview.ExportObjectFingerprint;
		RowPreview.bTargetHashChangedSinceExport = OutSession.DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit
			&& CurrentState.CurrentRow.TargetDefinitionHash != ManifestRow->TargetDefinitionHash;

		for (const FCFBatchColumnDescriptor& Descriptor : Columns)
		{
			if (Descriptor.bReservedMetadata || Descriptor.Access != ECFBatchColumnAccess::Editable)
			{
				continue;
			}
			// Export immutable baseline cell입니다.
			const FCFBatchBaselineCell* BaselineCell = CFBatchImportPrivate::FindBaselineCell(*ManifestRow, Descriptor.ColumnId);
			// Current Unreal authoring value/source mode cell입니다.
			const FCFBatchBaselineCell* CurrentCell = CFBatchImportPrivate::FindCurrentCell(CurrentState.CurrentRow, Descriptor.ColumnId);
			// Spreadsheet raw edited cell입니다.
			const FCFBatchParsedCell* ParsedCell = CFBatchImportPrivate::FindParsedCell(ParsedRow, Descriptor.ColumnId);
			if (!BaselineCell || !CurrentCell || !ParsedCell)
			{
				CFBatchImportPrivate::AddIssue(RowPreview.Issues, ECFBatchIssueCode::ManifestInvalid, ECFBatchIssueSeverity::Blocked,
					TEXT("Editable cell의 baseline/current/CSV projection 중 하나가 누락됐습니다."), RowPreview.RowId, Descriptor.ColumnId);
				continue;
			}

			// Section 26.34 one-cell 3-way review입니다.
			FCFBatchCellReview& CellReview = RowPreview.CellReviews.AddDefaulted_GetRef();
			CellReview.ColumnId = Descriptor.ColumnId;
			CellReview.ExportBaselineValue = BaselineCell->CanonicalValue;
			CellReview.CurrentUnrealValue = CurrentCell->CanonicalValue;
			CellReview.ExportOwnershipSourceMode = BaselineCell->OwnershipSourceMode;
			CellReview.CurrentOwnershipSourceMode = CurrentCell->OwnershipSourceMode;
			CellReview.bEditableAtExport = BaselineCell->bEditableAtExport;
			CellReview.bEditableNow = CurrentCell->bEditableAtExport;

			// Section 26.30 blank는 No Change이며 0/clear/reset으로 해석하지 않습니다.
			const bool bBlankNoChange = ParsedCell->RawValue.TrimStartAndEnd().IsEmpty();
			if (!CellReview.bEditableAtExport && !bBlankNoChange)
			{
				CellReview.MergeState = ECFBatchCellMergeState::BlockedInvalid;
				++RowPreview.SpreadsheetChangedCellCount;
				CFBatchImportPrivate::AddIssue(RowPreview.Issues, ECFBatchIssueCode::CellNotEditableAtExport, ECFBatchIssueSeverity::Blocked,
					TEXT("Export 시점 source mode에서 edit 불가였던 shadow numeric cell에 값이 입력됐습니다. Source Mode는 자동 전환하지 않습니다."), RowPreview.RowId, Descriptor.ColumnId);
				continue;
			}

			if (bBlankNoChange)
			{
				CellReview.EditedCanonicalValue = CellReview.ExportBaselineValue;
				CellReview.bSpreadsheetChanged = false;
				CellReview.MergeState = CellReview.CurrentUnrealValue == CellReview.ExportBaselineValue
					? ECFBatchCellMergeState::Unchanged
					: ECFBatchCellMergeState::NoSpreadsheetChange;
				continue;
			}

			// Current reflected property type 기준으로 plain numeric input을 canonicalize합니다.
			FString CanonicalEditedValue;
			FString NumericError;
			if (!CFBatchImportPrivate::CanonicalizeEditedNumeric(*CurrentState.AuthoringObject, Descriptor, ParsedCell->RawValue, CanonicalEditedValue, NumericError))
			{
				CellReview.MergeState = ECFBatchCellMergeState::BlockedInvalid;
				++RowPreview.SpreadsheetChangedCellCount;
				CFBatchImportPrivate::AddIssue(RowPreview.Issues, ECFBatchIssueCode::InvalidNumericValue, ECFBatchIssueSeverity::Blocked,
					NumericError, RowPreview.RowId, Descriptor.ColumnId);
				continue;
			}
			CellReview.EditedCanonicalValue = CanonicalEditedValue;
			CellReview.bSpreadsheetChanged = CellReview.EditedCanonicalValue != CellReview.ExportBaselineValue;
			if (!CellReview.bSpreadsheetChanged)
			{
				CellReview.MergeState = CellReview.CurrentUnrealValue == CellReview.ExportBaselineValue
					? ECFBatchCellMergeState::Unchanged
					: ECFBatchCellMergeState::NoSpreadsheetChange;
				continue;
			}
			++RowPreview.SpreadsheetChangedCellCount;

			// Spreadsheet가 실제 edit한 경우 ownership/source-mode change가 value merge보다 우선합니다.
			const bool bOwnershipChanged = CellReview.ExportOwnershipSourceMode != CellReview.CurrentOwnershipSourceMode
				|| CellReview.bEditableAtExport != CellReview.bEditableNow;
			if (bOwnershipChanged)
			{
				CellReview.MergeState = ECFBatchCellMergeState::OwnershipChangedSinceExport;
				CFBatchImportPrivate::AddIssue(RowPreview.Issues, ECFBatchIssueCode::OwnershipChangedSinceExport, ECFBatchIssueSeverity::Conflict,
					TEXT("Export 이후 Authoring ownership/source mode가 바뀌어 Spreadsheet numeric edit를 자동 적용할 수 없습니다."), RowPreview.RowId, Descriptor.ColumnId);
				continue;
			}

			if (CellReview.CurrentUnrealValue == CellReview.ExportBaselineValue)
			{
				CellReview.MergeState = ECFBatchCellMergeState::SafeCandidate;
				++RowPreview.CandidateCellCount;
				continue;
			}
			if (CellReview.EditedCanonicalValue == CellReview.CurrentUnrealValue)
			{
				CellReview.MergeState = ECFBatchCellMergeState::ConvergedNoChange;
				++RowPreview.ConvergedCellCount;
				continue;
			}

			CellReview.MergeState = ECFBatchCellMergeState::ConcurrentEditConflict;
			CFBatchImportPrivate::AddIssue(RowPreview.Issues, ECFBatchIssueCode::ConcurrentEditConflict, ECFBatchIssueSeverity::Conflict,
				TEXT("Export 이후 Spreadsheet와 Unreal이 같은 cell을 서로 다른 값으로 수정했습니다."), RowPreview.RowId, Descriptor.ColumnId);
		}
		RowPreview.CellReviews.Sort([](const FCFBatchCellReview& Left, const FCFBatchCellReview& Right)
		{
			return Left.ColumnId < Right.ColumnId;
		});

		if (CFBatchImportPrivate::HasConflictIssue(RowPreview.Issues))
		{
			RowPreview.Status = ECFBatchRowStatus::Conflict;
			OutSession.bFileBlocked = true;
		}
		else if (CFBatchImportPrivate::HasBlockingIssue(RowPreview.Issues))
		{
			RowPreview.Status = ECFBatchRowStatus::Blocked;
			OutSession.bFileBlocked = true;
		}
		else if (RowPreview.CandidateCellCount > 0)
		{
			// Transient-only prospective patch/Resolver preview diagnostic입니다.
			FString ProspectiveError;
			const bool bProspectiveBuilt = OutSession.DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit
				? CFBatchImportPrivate::BuildRecipeProspectivePreview(*CastChecked<UCFVehicleRecipeData>(CurrentState.AuthoringObject), Columns, RowPreview, ProspectiveError)
				: CFBatchImportPrivate::BuildProfileProspectivePreview(*CurrentState.AuthoringObject, Columns, RowPreview, ProspectiveError);
			if (!bProspectiveBuilt)
			{
				CFBatchImportPrivate::AddIssue(RowPreview.Issues, ECFBatchIssueCode::ProspectiveResolveFailed, ECFBatchIssueSeverity::Blocked,
					ProspectiveError, RowPreview.RowId);
				RowPreview.Status = ECFBatchRowStatus::Blocked;
				OutSession.bFileBlocked = true;
			}
			else if (OutSession.DatasetKind == ECFBatchDatasetKind::ProfileNumericEdit && RowPreview.VehiclePreviews.IsEmpty())
			{
				CFBatchImportPrivate::AddIssue(RowPreview.Issues, ECFBatchIssueCode::NoAffectedRecipe, ECFBatchIssueSeverity::Warning,
					TEXT("Shared Profile candidate를 참조하는 affected Recipe가 현재 inventory에 없습니다."), RowPreview.RowId);
				RowPreview.Status = ECFBatchRowStatus::Warning;
			}
			else
			{
				CFBatchImportPrivate::FinalizeProspectiveRowStatus(RowPreview);
				if (RowPreview.Status == ECFBatchRowStatus::Blocked || RowPreview.Status == ECFBatchRowStatus::Conflict)
				{
					OutSession.bFileBlocked = true;
				}
			}
		}
		else
		{
			CFBatchImportPrivate::FinalizeProspectiveRowStatus(RowPreview);
		}

		OutSession.Rows.Add(MoveTemp(RowPreview));
	}

	for (const FCFBatchManifestRow& ManifestRow : OutSession.Manifest.Rows)
	{
		if (!MatchedManifestRowIds.Contains(ManifestRow.RowId))
		{
			CFBatchImportPrivate::AddIssue(OutSession.Issues, ECFBatchIssueCode::MissingExportedRow, ECFBatchIssueSeverity::Blocked,
				FString::Printf(TEXT("Export manifest row가 CSV에서 사라졌습니다: %s"), *ManifestRow.RowId), ManifestRow.RowId);
			OutSession.bFileBlocked = true;
		}
	}

	OutSession.Rows.Sort([](const FCFBatchRowPreview& Left, const FCFBatchRowPreview& Right)
	{
		const FString LeftSource = !Left.RecipePath.IsEmpty() ? Left.RecipePath : Left.ProfilePath;
		const FString RightSource = !Right.RecipePath.IsEmpty() ? Right.RecipePath : Right.ProfilePath;
		return LeftSource + TEXT("|") + Left.RowId < RightSource + TEXT("|") + Right.RowId;
	});
	CFBatchImportPrivate::BuildSummary(OutSession);
	OutSession.BatchPlanHash = BuildBatchPlanHash(OutSession);
	return true;
}

// Row order와 localized message에 독립적인 deterministic Batch plan hash를 계산합니다.
FString FCFBatchImportService::BuildBatchPlanHash(const FCFBatchImportSession& Session)
{
	// Canonical row order local copy입니다.
	TArray<FCFBatchRowPreview> Rows = Session.Rows;
	Rows.Sort([](const FCFBatchRowPreview& Left, const FCFBatchRowPreview& Right)
	{
		const FString LeftSource = !Left.RecipePath.IsEmpty() ? Left.RecipePath : Left.ProfilePath;
		const FString RightSource = !Right.RecipePath.IsEmpty() ? Right.RecipePath : Right.ProfilePath;
		return LeftSource + TEXT("|") + Left.RowId < RightSource + TEXT("|") + Right.RowId;
	});

	// Localized message를 제외한 file-level issue canonical copy입니다.
	TArray<FCFBatchIssue> FileIssues = Session.Issues;
	FileIssues.Sort([](const FCFBatchIssue& Left, const FCFBatchIssue& Right)
	{
		const FString LeftKey = FString::FromInt(static_cast<int32>(Left.Code)) + TEXT("|") + Left.RowId + TEXT("|") + Left.ColumnId;
		const FString RightKey = FString::FromInt(static_cast<int32>(Right.Code)) + TEXT("|") + Right.RowId + TEXT("|") + Right.ColumnId;
		return LeftKey < RightKey;
	});

	// Deterministic Batch Plan semantic payload입니다.
	FString Payload;
	CFBatchImportPrivate::AppendToken(Payload, TEXT("Kind"), TEXT("CarFightBatchPlan"));
	CFBatchImportPrivate::AppendToken(Payload, TEXT("FormatRevision"), TEXT("1"));
	CFBatchImportPrivate::AppendToken(Payload, TEXT("DatasetKind"), CFBatchImportPrivate::DatasetToString(Session.DatasetKind));
	CFBatchImportPrivate::AppendToken(Payload, TEXT("ProfileDomain"), CFBatchImportPrivate::DomainToString(Session.ProfileDomain));
	CFBatchImportPrivate::AppendToken(Payload, TEXT("ExportSetHash"), Session.Manifest.ExportSetHash);
	CFBatchImportPrivate::AppendToken(Payload, TEXT("FileBlocked"), Session.bFileBlocked ? TEXT("1") : TEXT("0"));
	CFBatchImportPrivate::AppendToken(Payload, TEXT("FileIssueCount"), FString::FromInt(FileIssues.Num()));
	for (const FCFBatchIssue& Issue : FileIssues)
	{
		CFBatchImportPrivate::AppendToken(Payload, TEXT("IssueCode"), FString::FromInt(static_cast<int32>(Issue.Code)));
		CFBatchImportPrivate::AppendToken(Payload, TEXT("IssueSeverity"), FString::FromInt(static_cast<int32>(Issue.Severity)));
		CFBatchImportPrivate::AppendToken(Payload, TEXT("IssueRow"), Issue.RowId);
		CFBatchImportPrivate::AppendToken(Payload, TEXT("IssueColumn"), Issue.ColumnId);
	}

	CFBatchImportPrivate::AppendToken(Payload, TEXT("RowCount"), FString::FromInt(Rows.Num()));
	for (FCFBatchRowPreview& Row : Rows)
	{
		Row.CellReviews.Sort([](const FCFBatchCellReview& Left, const FCFBatchCellReview& Right)
		{
			return Left.ColumnId < Right.ColumnId;
		});
		Row.VehiclePreviews.Sort([](const FCFBatchVehiclePreview& Left, const FCFBatchVehiclePreview& Right)
		{
			return Left.RecipePath + TEXT("|") + Left.TargetPath < Right.RecipePath + TEXT("|") + Right.TargetPath;
		});
		Row.Issues.Sort([](const FCFBatchIssue& Left, const FCFBatchIssue& Right)
		{
			const FString LeftKey = FString::FromInt(static_cast<int32>(Left.Code)) + TEXT("|") + Left.ColumnId;
			const FString RightKey = FString::FromInt(static_cast<int32>(Right.Code)) + TEXT("|") + Right.ColumnId;
			return LeftKey < RightKey;
		});

		CFBatchImportPrivate::AppendToken(Payload, TEXT("RowId"), Row.RowId);
		CFBatchImportPrivate::AppendToken(Payload, TEXT("TargetPath"), Row.TargetPath);
		CFBatchImportPrivate::AppendToken(Payload, TEXT("RecipePath"), Row.RecipePath);
		CFBatchImportPrivate::AppendToken(Payload, TEXT("ProfilePath"), Row.ProfilePath);
		CFBatchImportPrivate::AppendToken(Payload, TEXT("RowProfileDomain"), CFBatchImportPrivate::DomainToString(Row.ProfileDomain));
		CFBatchImportPrivate::AppendToken(Payload, TEXT("ExportFingerprint"), Row.ExportObjectFingerprint);
		CFBatchImportPrivate::AppendToken(Payload, TEXT("CurrentFingerprint"), Row.CurrentObjectFingerprint);
		CFBatchImportPrivate::AppendToken(Payload, TEXT("FingerprintChanged"), Row.bFingerprintChangedSinceExport ? TEXT("1") : TEXT("0"));
		CFBatchImportPrivate::AppendToken(Payload, TEXT("TargetHashChanged"), Row.bTargetHashChangedSinceExport ? TEXT("1") : TEXT("0"));
		CFBatchImportPrivate::AppendToken(Payload, TEXT("RowStatus"), FString::FromInt(static_cast<int32>(Row.Status)));
		CFBatchImportPrivate::AppendToken(Payload, TEXT("CellCount"), FString::FromInt(Row.CellReviews.Num()));
		for (const FCFBatchCellReview& Cell : Row.CellReviews)
		{
			CFBatchImportPrivate::AppendToken(Payload, TEXT("ColumnId"), Cell.ColumnId);
			CFBatchImportPrivate::AppendToken(Payload, TEXT("Baseline"), Cell.ExportBaselineValue);
			CFBatchImportPrivate::AppendToken(Payload, TEXT("Edited"), Cell.EditedCanonicalValue);
			CFBatchImportPrivate::AppendToken(Payload, TEXT("Current"), Cell.CurrentUnrealValue);
			CFBatchImportPrivate::AppendToken(Payload, TEXT("ExportOwner"), Cell.ExportOwnershipSourceMode);
			CFBatchImportPrivate::AppendToken(Payload, TEXT("CurrentOwner"), Cell.CurrentOwnershipSourceMode);
			CFBatchImportPrivate::AppendToken(Payload, TEXT("EditableAtExport"), Cell.bEditableAtExport ? TEXT("1") : TEXT("0"));
			CFBatchImportPrivate::AppendToken(Payload, TEXT("EditableNow"), Cell.bEditableNow ? TEXT("1") : TEXT("0"));
			CFBatchImportPrivate::AppendToken(Payload, TEXT("SpreadsheetChanged"), Cell.bSpreadsheetChanged ? TEXT("1") : TEXT("0"));
			CFBatchImportPrivate::AppendToken(Payload, TEXT("MergeState"), FString::FromInt(static_cast<int32>(Cell.MergeState)));
		}
		CFBatchImportPrivate::AppendToken(Payload, TEXT("VehiclePreviewCount"), FString::FromInt(Row.VehiclePreviews.Num()));
		for (const FCFBatchVehiclePreview& Preview : Row.VehiclePreviews)
		{
			CFBatchImportPrivate::AppendToken(Payload, TEXT("AffectedRecipe"), Preview.RecipePath);
			CFBatchImportPrivate::AppendToken(Payload, TEXT("AffectedTarget"), Preview.TargetPath);
			CFBatchImportPrivate::AppendToken(Payload, TEXT("CurrentResolvedHash"), Preview.CurrentResolvedDefinitionHash);
			CFBatchImportPrivate::AppendToken(Payload, TEXT("ProspectiveResolvedHash"), Preview.ProspectiveResolvedDefinitionHash);
			CFBatchImportPrivate::AppendToken(Payload, TEXT("ProspectiveSourceSignature"), Preview.ProspectiveSourceSignature);
			CFBatchImportPrivate::AppendToken(Payload, TEXT("DefinitionDiffCount"), FString::FromInt(Preview.DefinitionDiffCount));
			CFBatchImportPrivate::AppendToken(Payload, TEXT("ArrayStructuralCount"), FString::FromInt(Preview.ArrayStructuralChangeCount));
			CFBatchImportPrivate::AppendToken(Payload, TEXT("ValidationInfo"), FString::FromInt(Preview.ValidationInfoCount));
			CFBatchImportPrivate::AppendToken(Payload, TEXT("ValidationWarning"), FString::FromInt(Preview.ValidationWarningCount));
			CFBatchImportPrivate::AppendToken(Payload, TEXT("ValidationBlocked"), FString::FromInt(Preview.ValidationBlockedCount));
			CFBatchImportPrivate::AppendToken(Payload, TEXT("ValidationError"), FString::FromInt(Preview.ValidationErrorCount));
			CFBatchImportPrivate::AppendToken(Payload, TEXT("ExternalDrift"), Preview.bExternalDrift ? TEXT("1") : TEXT("0"));
			CFBatchImportPrivate::AppendToken(Payload, TEXT("ResolveStatus"), FString::FromInt(static_cast<int32>(Preview.ResolveStatus)));
		}
		CFBatchImportPrivate::AppendToken(Payload, TEXT("RowIssueCount"), FString::FromInt(Row.Issues.Num()));
		for (const FCFBatchIssue& Issue : Row.Issues)
		{
			CFBatchImportPrivate::AppendToken(Payload, TEXT("RowIssueCode"), FString::FromInt(static_cast<int32>(Issue.Code)));
			CFBatchImportPrivate::AppendToken(Payload, TEXT("RowIssueSeverity"), FString::FromInt(static_cast<int32>(Issue.Severity)));
			CFBatchImportPrivate::AppendToken(Payload, TEXT("RowIssueColumn"), Issue.ColumnId);
		}
	}
		return CFBatchImportPrivate::HashUtf8Payload(Payload);
}

// SafeCandidate ColumnId/value set만 canonicalize해 Section 26.51 per-row Proposed Patch Hash를 계산합니다.
FString FCFBatchImportService::BuildRowProposedPatchHash(const FCFBatchRowPreview& Row)
{
	// Storage order와 무관하게 canonicalize할 SafeCandidate cell pointers입니다.
	TArray<const FCFBatchCellReview*> CandidateCells;
	for (const FCFBatchCellReview& Cell : Row.CellReviews)
	{
		if (Cell.MergeState == ECFBatchCellMergeState::SafeCandidate)
		{
			CandidateCells.Add(&Cell);
		}
	}
	CandidateCells.Sort([](const FCFBatchCellReview& Left, const FCFBatchCellReview& Right)
	{
		return Left.ColumnId < Right.ColumnId;
	});

	// Per-row typed patch semantic payload입니다.
	FString Payload;
	CFBatchImportPrivate::AppendToken(Payload, TEXT("Kind"), TEXT("CarFightBatchRowPatch"));
	CFBatchImportPrivate::AppendToken(Payload, TEXT("FormatRevision"), TEXT("1"));
	CFBatchImportPrivate::AppendToken(Payload, TEXT("RowId"), Row.RowId);
	CFBatchImportPrivate::AppendToken(Payload, TEXT("RecipePath"), Row.RecipePath);
	CFBatchImportPrivate::AppendToken(Payload, TEXT("ProfilePath"), Row.ProfilePath);
	CFBatchImportPrivate::AppendToken(Payload, TEXT("ProfileDomain"), CFBatchImportPrivate::DomainToString(Row.ProfileDomain));
	CFBatchImportPrivate::AppendToken(Payload, TEXT("CandidateCount"), FString::FromInt(CandidateCells.Num()));
	for (const FCFBatchCellReview* Cell : CandidateCells)
	{
		if (!Cell)
		{
			continue;
		}
		CFBatchImportPrivate::AppendToken(Payload, TEXT("ColumnId"), Cell->ColumnId);
		CFBatchImportPrivate::AppendToken(Payload, TEXT("CanonicalValue"), Cell->EditedCanonicalValue);
	}
	return CFBatchImportPrivate::HashUtf8Payload(Payload);
}

// Current Session의 모든 commit-candidate row를 exact BatchPlanHash/current fingerprint에 binding한 B1/B2 approval evidence로 만듭니다.
bool FCFBatchImportService::BuildCommitApproval(
	const FCFBatchImportSession& Session,
	FCFBatchCommitApproval& OutApproval,
	TArray<FCFBatchIssue>& OutIssues)
{
	OutApproval = FCFBatchCommitApproval();
	OutIssues.Reset();
	if (Session.DatasetKind != ECFBatchDatasetKind::RecipeNumericEdit
		&& Session.DatasetKind != ECFBatchDatasetKind::ProfileNumericEdit)
	{
		CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::UnsupportedDataset, ECFBatchIssueSeverity::Blocked,
			TEXT("B1/B2 Authoring Source Commit은 RecipeNumericEdit/ProfileNumericEdit만 지원합니다."));
		return false;
	}
		if (Session.Summary.ConflictRowCount > 0)
	{
		CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ConcurrentEditConflict, ECFBatchIssueSeverity::Conflict,
			TEXT("Conflict row가 있는 Batch Session은 B1/B2 approval을 만들 수 없습니다."));
		return false;
	}
	for (const FCFBatchIssue& FileIssue : Session.Issues)
	{
		if (FileIssue.Severity == ECFBatchIssueSeverity::Blocked
			|| FileIssue.Severity == ECFBatchIssueSeverity::Conflict)
		{
			CFBatchImportPrivate::AddIssue(OutIssues, FileIssue.Code, FileIssue.Severity,
				TEXT("File-level Batch parse/schema/identity blocker가 있어 B1/B2 approval을 만들 수 없습니다."), FileIssue.RowId, FileIssue.ColumnId);
			return false;
		}
	}
	for (const FCFBatchRowPreview& Row : Session.Rows)
	{
		if (CFBatchImportPrivate::HasSourceCommitBlockingIssue(Row))
		{
			CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ValidationBlocked, ECFBatchIssueSeverity::Blocked,
				TEXT("Read-only/input/ownership/conflict 등 Authoring Source 자체 blocker가 있어 B1/B2 approval을 만들 수 없습니다."), Row.RowId);
			return false;
		}
	}

	if (Session.BatchPlanHash.IsEmpty() || BuildBatchPlanHash(Session) != Session.BatchPlanHash)
	{
		CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ManifestInvalid, ECFBatchIssueSeverity::Blocked,
			TEXT("BatchPlanHash가 비어 있거나 Session current content와 일치하지 않습니다."));
		return false;
	}

	OutApproval.DatasetKind = Session.DatasetKind;
	OutApproval.ProfileDomain = Session.ProfileDomain;
	OutApproval.SchemaId = Session.Manifest.SchemaId;
	OutApproval.SchemaRevision = Session.Manifest.SchemaRevision;
	OutApproval.BatchExportId = Session.Manifest.BatchExportId;
	OutApproval.BatchPlanHash = Session.BatchPlanHash;
	OutApproval.ConflictRowCount = Session.Summary.ConflictRowCount;
	OutApproval.WarningRowCount = Session.Summary.WarningRowCount;
	OutApproval.ExternalDriftRowCount = Session.Summary.ExternalDriftRowCount;
	OutApproval.ShadowOnlyRowCount = Session.Summary.ShadowOnlyRowCount;
	OutApproval.ValidationWarningCount = Session.Summary.ValidationWarningCount;

	for (const FCFBatchRowPreview& Row : Session.Rows)
	{
		if (!CFBatchImportPrivate::IsCommitCandidateRow(Row))
		{
			continue;
		}
		if (Row.RowId.IsEmpty() || Row.CurrentObjectFingerprint.IsEmpty())
		{
			CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ManifestInvalid, ECFBatchIssueSeverity::Blocked,
				TEXT("Commit candidate row의 RowId/current source fingerprint가 비어 있습니다."), Row.RowId);
			return false;
		}

		// Exact included row approval evidence입니다.
		FCFBatchCommitApprovalRow& ApprovalRow = OutApproval.IncludedRows.AddDefaulted_GetRef();
		ApprovalRow.RowId = Row.RowId;
		ApprovalRow.ProposedPatchHash = BuildRowProposedPatchHash(Row);
		ApprovalRow.CurrentFingerprint = Row.CurrentObjectFingerprint;
	}
	OutApproval.IncludedRows.Sort([](const FCFBatchCommitApprovalRow& Left, const FCFBatchCommitApprovalRow& Right)
	{
		return Left.RowId < Right.RowId;
	});
	if (OutApproval.IncludedRows.IsEmpty())
	{
		CFBatchImportPrivate::AddIssue(OutIssues, ECFBatchIssueCode::ProspectivePatchFailed, ECFBatchIssueSeverity::Blocked,
			TEXT("B1/B2 commit candidate row가 없습니다. 부분 선택은 old approval을 축소하지 말고 새 Batch Preview/Plan을 만들어야 합니다."));
		return false;
	}
	return true;
}

// Global preflight를 모두 통과한 뒤 B1 Recipe 또는 B2 Profile source를 one logical transaction으로 all-or-nothing commit합니다.
bool FCFBatchImportService::CommitAuthoringSources(
	const FCFBatchAuthoringCommitRequest& Request,
	FCFBatchAuthoringCommitResult& OutResult)
{
	OutResult = FCFBatchAuthoringCommitResult();
	if (!Request.Session || !Request.Approval)
	{
		CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::InvalidRequest,
			TEXT("B1/B2 commit에는 exact Batch Session과 approval evidence가 필요합니다."));
		return false;
	}
	if (!Request.bAuthoringCommitApproved)
	{
		CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::ApprovalRequired,
			TEXT("B1/B2 Authoring Source Commit은 명시적 Batch Authoring approval이 필요합니다."));
		return false;
	}

	// Reviewed transient Batch Session입니다.
	const FCFBatchImportSession& Session = *Request.Session;
	// Caller가 제시한 exact B1/B2 approval evidence입니다.
	const FCFBatchCommitApproval& Approval = *Request.Approval;
	// Session content에서 다시 만든 server-side exact approval evidence입니다.
	FCFBatchCommitApproval ExpectedApproval;
	// Approval rebuild diagnostics입니다.
	TArray<FCFBatchIssue> ApprovalIssues;
	if (!BuildCommitApproval(Session, ExpectedApproval, ApprovalIssues))
	{
		CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::SessionBlocked,
			!ApprovalIssues.IsEmpty() ? ApprovalIssues[0].Message : TEXT("Batch Session이 B1/B2 commit 가능한 상태가 아닙니다."));
		return false;
	}

	const bool bApprovalHeaderMatches = Approval.DatasetKind == ExpectedApproval.DatasetKind
		&& Approval.ProfileDomain == ExpectedApproval.ProfileDomain
		&& Approval.SchemaId == ExpectedApproval.SchemaId
		&& Approval.SchemaRevision == ExpectedApproval.SchemaRevision
		&& Approval.BatchExportId == ExpectedApproval.BatchExportId
		&& Approval.BatchPlanHash == ExpectedApproval.BatchPlanHash
		&& Approval.ConflictRowCount == ExpectedApproval.ConflictRowCount
		&& Approval.WarningRowCount == ExpectedApproval.WarningRowCount
		&& Approval.ExternalDriftRowCount == ExpectedApproval.ExternalDriftRowCount
		&& Approval.ShadowOnlyRowCount == ExpectedApproval.ShadowOnlyRowCount
		&& Approval.ValidationWarningCount == ExpectedApproval.ValidationWarningCount;
	if (!bApprovalHeaderMatches)
	{
		CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::ApprovalMismatch,
			TEXT("B1/B2 approval header/BatchPlanHash/summary가 current reviewed Session과 일치하지 않습니다."));
		return false;
	}
	if (Approval.IncludedRows.Num() != ExpectedApproval.IncludedRows.Num())
	{
		CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::IncludedRowSetMismatch,
			TEXT("Approval Included Row set이 current Batch Plan과 다릅니다. old approval의 부분 축소 재사용은 허용하지 않습니다."));
		return false;
	}
	for (int32 RowIndex = 0; RowIndex < ExpectedApproval.IncludedRows.Num(); ++RowIndex)
	{
		// Server-side expected approval row입니다.
		const FCFBatchCommitApprovalRow& ExpectedRow = ExpectedApproval.IncludedRows[RowIndex];
		// Caller approval row입니다.
		const FCFBatchCommitApprovalRow& ApprovedRow = Approval.IncludedRows[RowIndex];
		if (ExpectedRow.RowId != ApprovedRow.RowId)
		{
			CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::IncludedRowSetMismatch,
				TEXT("Approval Included Row Id set/order가 exact Batch Plan과 일치하지 않습니다."));
			return false;
		}
		if (ExpectedRow.ProposedPatchHash != ApprovedRow.ProposedPatchHash)
		{
			CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::ProposedPatchMismatch,
				FString::Printf(TEXT("Per-row Proposed Patch Hash가 일치하지 않습니다: %s"), *ExpectedRow.RowId));
			return false;
		}
		if (ExpectedRow.CurrentFingerprint != ApprovedRow.CurrentFingerprint)
		{
			CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::ApprovalMismatch,
				FString::Printf(TEXT("Per-row approval current fingerprint가 일치하지 않습니다: %s"), *ExpectedRow.RowId));
			return false;
		}
	}

	// Current Batch Registry typed patch descriptors입니다.
	TArray<FCFBatchColumnDescriptor> Columns;
	// Registry failure diagnostics입니다.
	TArray<FString> RegistryErrors;
	if (!FCFBatchColumnRegistry::GetDatasetColumns(Session.DatasetKind, Session.ProfileDomain, Columns, RegistryErrors))
	{
		CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::InvalidRequest,
			!RegistryErrors.IsEmpty() ? RegistryErrors[0] : TEXT("B1/B2 current Batch Registry를 읽지 못했습니다."));
		return false;
	}

	// 모든 source precondition을 mutation 전에 확정할 execution plans입니다.
	TArray<CFBatchImportPrivate::FCommitObjectPlan> ObjectPlans;
	ObjectPlans.Reserve(ExpectedApproval.IncludedRows.Num());
	for (const FCFBatchCommitApprovalRow& ApprovalRow : ExpectedApproval.IncludedRows)
	{
		// Session에서 exact reviewed row를 찾습니다.
		const FCFBatchRowPreview* Row = Session.Rows.FindByPredicate([&ApprovalRow](const FCFBatchRowPreview& CandidateRow)
		{
			return CandidateRow.RowId == ApprovalRow.RowId;
		});
		if (!Row || !CFBatchImportPrivate::IsCommitCandidateRow(*Row))
		{
			CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::IncludedRowSetMismatch,
				FString::Printf(TEXT("Included Row가 current commit candidate set에서 사라졌습니다: %s"), *ApprovalRow.RowId));
			return false;
		}

		// Dataset-specific persistent source identity입니다.
		const FString SourceIdentity = Session.DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit ? Row->RecipePath : Row->ProfilePath;
		// Commit 직전 current persistent source object입니다.
		UObject* SourceObject = CFBatchImportPrivate::ResolveObjectPath(SourceIdentity);
		if (!SourceObject)
		{
			CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::SourceObjectMissing,
				FString::Printf(TEXT("Commit 직전 current Authoring Source를 찾지 못했습니다: %s"), *SourceIdentity));
			OutResult.StaleRowIds.Add(ApprovalRow.RowId);
			return false;
		}
		if (!CFBatchImportPrivate::IsExpectedSourceType(*SourceObject, Session.DatasetKind, Session.ProfileDomain))
		{
			CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::SourceTypeMismatch,
				FString::Printf(TEXT("Commit 직전 Authoring Source type/domain이 Preview와 다릅니다: %s"), *SourceIdentity));
			OutResult.StaleRowIds.Add(ApprovalRow.RowId);
			return false;
		}

		// Commit 직전 P0-08J current row projection입니다.
		FCFBatchExportRow CurrentRow;
		// Current source readback diagnostic입니다.
		FString PreflightError;
		if (!CFBatchImportPrivate::BuildCurrentAuthoringRow(*SourceObject, Session.DatasetKind, Session.ProfileDomain, CurrentRow, PreflightError))
		{
			CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::SourceObjectMissing, PreflightError);
			OutResult.StaleRowIds.Add(ApprovalRow.RowId);
			return false;
		}
		if (CurrentRow.ObjectFingerprint != ApprovalRow.CurrentFingerprint
			|| CurrentRow.ObjectFingerprint != Row->CurrentObjectFingerprint)
		{
			CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::SourceFingerprintMismatch,
				FString::Printf(TEXT("Authoring Source가 approval 이후 변경됐습니다: Row=%s"), *ApprovalRow.RowId));
			OutResult.StaleRowIds.Add(ApprovalRow.RowId);
			return false;
		}
		if (BuildRowProposedPatchHash(*Row) != ApprovalRow.ProposedPatchHash)
		{
			CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::ProposedPatchMismatch,
				FString::Printf(TEXT("Commit 직전 Proposed Patch Hash가 approval과 다릅니다: Row=%s"), *ApprovalRow.RowId));
			return false;
		}

		// Typed patch 후 semantic expected fingerprint입니다.
		FString ExpectedPostFingerprint;
		if (!CFBatchImportPrivate::BuildExpectedPostFingerprint(
			*SourceObject,
			Session.DatasetKind,
			Session.ProfileDomain,
			Columns,
			*Row,
			ExpectedPostFingerprint,
			PreflightError))
		{
			CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::PatchApplyFailed, PreflightError);
			return false;
		}

		// Commit 전 diagnostic revision입니다.
		int32 CurrentAuthoringRevision = 0;
		if (!CFBatchImportPrivate::GetSourceAuthoringRevision(
			*SourceObject,
			Session.DatasetKind,
			Session.ProfileDomain,
			CurrentAuthoringRevision,
			PreflightError))
		{
			CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::SourceTypeMismatch, PreflightError);
			return false;
		}

				if (Session.DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit)
		{
			// Frozen 26.47 B1은 Input/Recipe validation layer만 source commit blocker로 fresh 검사합니다.
			UCFVehicleRecipeData* CurrentRecipe = Cast<UCFVehicleRecipeData>(SourceObject);
			if (!CurrentRecipe
				|| !CFBatchImportPrivate::ValidateRecipeCommitPreflight(*CurrentRecipe, Columns, *Row, PreflightError))
			{
				CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::ProspectiveValidationBlocked,
					PreflightError.IsEmpty() ? TEXT("B1 patched Recipe validation preflight에 실패했습니다.") : PreflightError);
				return false;
			}
		}
		else if (Session.DatasetKind == ECFBatchDatasetKind::ProfileNumericEdit)
		{
			// Frozen 26.49 B2 fresh affected Recipe inventory/validation taxonomy입니다.
			ECFBatchCommitErrorCode ProfilePreflightErrorCode = ECFBatchCommitErrorCode::None;
			if (!CFBatchImportPrivate::ValidateProfileCommitPreflight(
				*SourceObject,
				Columns,
				*Row,
				ProfilePreflightErrorCode,
				PreflightError))
			{
				CFBatchImportPrivate::SetCommitBlocked(OutResult, ProfilePreflightErrorCode, PreflightError);
				OutResult.StaleRowIds.Add(ApprovalRow.RowId);
				return false;
			}
		}


		// Mutation 전 완성된 one-source execution plan입니다.
		CFBatchImportPrivate::FCommitObjectPlan& ObjectPlan = ObjectPlans.AddDefaulted_GetRef();
		ObjectPlan.Row = Row;
		ObjectPlan.SourceObject = SourceObject;
		ObjectPlan.SourceIdentity = SourceIdentity;
		ObjectPlan.ExpectedPostFingerprint = ExpectedPostFingerprint;
		ObjectPlan.OriginalAuthoringRevision = CurrentAuthoringRevision;
		ObjectPlan.bPackageWasDirty = SourceObject->GetOutermost() && SourceObject->GetOutermost()->IsDirty();
	}
	ObjectPlans.Sort([](const CFBatchImportPrivate::FCommitObjectPlan& Left, const CFBatchImportPrivate::FCommitObjectPlan& Right)
	{
		return Left.SourceIdentity + TEXT("|") + (Left.Row ? Left.Row->RowId : FString())
			< Right.SourceIdentity + TEXT("|") + (Right.Row ? Right.Row->RowId : FString());
	});

	// Transaction 전에 모든 source의 full reflected rollback backup을 먼저 생성합니다.
	TArray<TStrongObjectPtr<UObject>> SourceBackups;
	SourceBackups.Reserve(ObjectPlans.Num());
	for (CFBatchImportPrivate::FCommitObjectPlan& ObjectPlan : ObjectPlans)
	{
		if (!ObjectPlan.SourceObject)
		{
			CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::SourceObjectMissing,
				TEXT("Global preflight 뒤 source pointer가 null이 됐습니다."));
			return false;
		}
		// Full reflected rollback backup object입니다.
		TStrongObjectPtr<UObject> Backup(DuplicateObject<UObject>(ObjectPlan.SourceObject, GetTransientPackage(), NAME_None));
		if (!Backup.IsValid())
		{
			CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::InvalidRequest,
				FString::Printf(TEXT("Source rollback backup 생성에 실패했습니다: %s"), *ObjectPlan.SourceIdentity));
			return false;
		}
		Backup->SetFlags(RF_Transient);
		if (Session.DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit)
		{
			// Backup PostDuplicate가 바꾼 RecipeId를 original identity로 복구합니다.
			UCFVehicleRecipeData* BackupRecipe = Cast<UCFVehicleRecipeData>(Backup.Get());
			// Original persistent Recipe입니다.
			const UCFVehicleRecipeData* CurrentRecipe = Cast<UCFVehicleRecipeData>(ObjectPlan.SourceObject);
			if (!BackupRecipe || !CurrentRecipe)
			{
				CFBatchImportPrivate::SetCommitBlocked(OutResult, ECFBatchCommitErrorCode::SourceTypeMismatch,
					TEXT("Recipe rollback backup type이 잘못됐습니다."));
				return false;
			}
			BackupRecipe->RecipeId = CurrentRecipe->RecipeId;
		}
		ObjectPlan.BackupIndex = SourceBackups.Num();
		SourceBackups.Add(MoveTemp(Backup));
	}

	// B1 또는 B2 전체 source set을 묶는 one logical Editor transaction입니다.
	FScopedTransaction BatchTransaction(NSLOCTEXT("CarFightDataAuthoring", "CommitBatchAuthoringSources", "Batch Authoring Source 변경"));
	for (CFBatchImportPrivate::FCommitObjectPlan& ObjectPlan : ObjectPlans)
	{
		ObjectPlan.SourceObject->Modify();
	}

	// Transaction failure 시 모든 included source를 pre-batch snapshot으로 복원하는 rollback helper입니다.
	auto RollbackAllSources = [&]() -> bool
	{
		bool bAllRestored = true;
		for (CFBatchImportPrivate::FCommitObjectPlan& ObjectPlan : ObjectPlans)
		{
			if (!ObjectPlan.SourceObject
				|| !SourceBackups.IsValidIndex(ObjectPlan.BackupIndex)
				|| !SourceBackups[ObjectPlan.BackupIndex].IsValid())
			{
				bAllRestored = false;
				continue;
			}
			// One source reflected restore diagnostic입니다.
			FString RestoreError;
			if (!CFBatchImportPrivate::RestoreSourceFromBackup(
				*ObjectPlan.SourceObject,
				*SourceBackups[ObjectPlan.BackupIndex].Get(),
				RestoreError))
			{
				bAllRestored = false;
			}
		}
		BatchTransaction.Cancel();

		for (CFBatchImportPrivate::FCommitObjectPlan& ObjectPlan : ObjectPlans)
		{
			if (!ObjectPlan.SourceObject)
			{
				bAllRestored = false;
				continue;
			}
			ObjectPlan.SourceObject->PostEditChange();
			if (UPackage* SourcePackage = ObjectPlan.SourceObject->GetOutermost())
			{
				SourcePackage->SetDirtyFlag(ObjectPlan.bPackageWasDirty);
			}

			// Rollback semantic fingerprint readback입니다.
			FCFBatchExportRow RestoredRow;
			// Rollback readback diagnostic입니다.
			FString VerifyError;
			// Rollback AuthoringRevision readback입니다.
			int32 RestoredRevision = 0;
			if (!CFBatchImportPrivate::BuildCurrentAuthoringRow(
				*ObjectPlan.SourceObject,
				Session.DatasetKind,
				Session.ProfileDomain,
				RestoredRow,
				VerifyError)
				|| RestoredRow.ObjectFingerprint != ObjectPlan.Row->CurrentObjectFingerprint
				|| !CFBatchImportPrivate::GetSourceAuthoringRevision(
					*ObjectPlan.SourceObject,
					Session.DatasetKind,
					Session.ProfileDomain,
					RestoredRevision,
					VerifyError)
				|| RestoredRevision != ObjectPlan.OriginalAuthoringRevision)
			{
				bAllRestored = false;
			}
		}
		return bAllRestored;
	};

	// Deterministic source patch 순서에서 몇 object가 성공 적용됐는지 추적합니다.
	int32 PatchedObjectCount = 0;
	for (CFBatchImportPrivate::FCommitObjectPlan& ObjectPlan : ObjectPlans)
	{
		// Persistent typed numeric patch diagnostic입니다.
		FString PatchError;
		if (!CFBatchImportPrivate::ApplyCandidateCells(*ObjectPlan.SourceObject, Columns, *ObjectPlan.Row, PatchError)
			|| !CFBatchImportPrivate::IncrementSourceAuthoringRevision(
				*ObjectPlan.SourceObject,
				Session.DatasetKind,
				Session.ProfileDomain,
				PatchError))
		{
			const bool bRollbackSucceeded = RollbackAllSources();
			OutResult.Status = bRollbackSucceeded ? ECFBatchCommitStatus::FailedRolledBack : ECFBatchCommitStatus::FailedUnknownState;
			OutResult.ErrorCode = bRollbackSucceeded ? ECFBatchCommitErrorCode::PatchApplyFailed : ECFBatchCommitErrorCode::RollbackFailed;
			OutResult.Message = bRollbackSucceeded
				? FString::Printf(TEXT("Batch source typed patch가 실패해 전체 rollback했습니다: %s"), *PatchError)
				: TEXT("Batch source patch 실패 후 전체 rollback 검증에도 실패했습니다.");
			OutResult.bRequiresFreshPreview = true;
			return false;
		}
		++PatchedObjectCount;

		// Persistent patch 직후 semantic fingerprint readback입니다.
		FCFBatchExportRow PatchedRow;
		// Persistent patch readback diagnostic입니다.
		FString ReadbackError;
		// Persistent patch diagnostic revision readback입니다.
		int32 PatchedRevision = 0;
		if (!CFBatchImportPrivate::BuildCurrentAuthoringRow(
			*ObjectPlan.SourceObject,
			Session.DatasetKind,
			Session.ProfileDomain,
			PatchedRow,
			ReadbackError)
			|| PatchedRow.ObjectFingerprint != ObjectPlan.ExpectedPostFingerprint
			|| !CFBatchImportPrivate::GetSourceAuthoringRevision(
				*ObjectPlan.SourceObject,
				Session.DatasetKind,
				Session.ProfileDomain,
				PatchedRevision,
				ReadbackError)
			|| PatchedRevision != ObjectPlan.OriginalAuthoringRevision + 1)
		{
			const bool bRollbackSucceeded = RollbackAllSources();
			OutResult.Status = bRollbackSucceeded ? ECFBatchCommitStatus::FailedRolledBack : ECFBatchCommitStatus::FailedUnknownState;
			OutResult.ErrorCode = bRollbackSucceeded ? ECFBatchCommitErrorCode::PostCheckFailed : ECFBatchCommitErrorCode::RollbackFailed;
			OutResult.Message = bRollbackSucceeded
				? TEXT("Batch source post-check fingerprint/revision mismatch로 전체 rollback했습니다.")
				: TEXT("Batch source post-check mismatch 후 rollback 검증도 실패했습니다.");
			OutResult.bRequiresFreshPreview = true;
			return false;
		}

#if WITH_DEV_AUTOMATION_TESTS
		if (Request.AutomationFailureAfterPatchedObjectCount != INDEX_NONE
			&& PatchedObjectCount == Request.AutomationFailureAfterPatchedObjectCount)
		{
			const bool bRollbackSucceeded = RollbackAllSources();
			OutResult.Status = bRollbackSucceeded ? ECFBatchCommitStatus::FailedRolledBack : ECFBatchCommitStatus::FailedUnknownState;
			OutResult.ErrorCode = bRollbackSucceeded ? ECFBatchCommitErrorCode::AutomationInjectedFailure : ECFBatchCommitErrorCode::RollbackFailed;
			OutResult.Message = bRollbackSucceeded
				? TEXT("Automation injected Batch failure 뒤 전체 source rollback을 완료했습니다.")
				: TEXT("Automation injected failure 뒤 rollback 검증에 실패했습니다.");
			OutResult.bRequiresFreshPreview = true;
			return false;
		}
#endif
	}

	// 모든 typed patch/post-check 성공 뒤에만 Editor notification을 발행합니다.
	for (CFBatchImportPrivate::FCommitObjectPlan& ObjectPlan : ObjectPlans)
	{
		ObjectPlan.SourceObject->PostEditChange();
		// PostEditChange가 semantic payload를 바꾸지 않았는지 final readback합니다.
		FCFBatchExportRow FinalRow;
		// Final notification readback diagnostic입니다.
		FString FinalError;
		if (!CFBatchImportPrivate::BuildCurrentAuthoringRow(
			*ObjectPlan.SourceObject,
			Session.DatasetKind,
			Session.ProfileDomain,
			FinalRow,
			FinalError)
			|| FinalRow.ObjectFingerprint != ObjectPlan.ExpectedPostFingerprint)
		{
			const bool bRollbackSucceeded = RollbackAllSources();
			OutResult.Status = bRollbackSucceeded ? ECFBatchCommitStatus::FailedRolledBack : ECFBatchCommitStatus::FailedUnknownState;
			OutResult.ErrorCode = bRollbackSucceeded ? ECFBatchCommitErrorCode::PostCheckFailed : ECFBatchCommitErrorCode::RollbackFailed;
			OutResult.Message = bRollbackSucceeded
				? TEXT("PostEditChange 이후 semantic fingerprint가 바뀌어 전체 rollback했습니다.")
				: TEXT("PostEditChange semantic mismatch 뒤 rollback 검증에 실패했습니다.");
			OutResult.bRequiresFreshPreview = true;
			return false;
		}
	}

	// Successful source transaction만 package dirty를 남기고 자동 저장은 하지 않습니다.
	for (CFBatchImportPrivate::FCommitObjectPlan& ObjectPlan : ObjectPlans)
	{
		ObjectPlan.SourceObject->MarkPackageDirty();
		OutResult.CommittedSourceIdentities.Add(ObjectPlan.SourceIdentity);
	}
	OutResult.CommittedSourceIdentities.Sort();
	OutResult.Status = ECFBatchCommitStatus::Succeeded;
	OutResult.ErrorCode = ECFBatchCommitErrorCode::None;
	OutResult.CommittedRowCount = ObjectPlans.Num();
	OutResult.CommittedSourceObjectCount = ObjectPlans.Num();
	OutResult.bApprovalConsumed = true;
	OutResult.bRequiresFreshPreview = true;
	OutResult.bTargetMutationPerformed = false;
	OutResult.bSavePerformed = false;
	OutResult.Message = Session.DatasetKind == ECFBatchDatasetKind::RecipeNumericEdit
		? TEXT("B1 Recipe Numeric Authoring Source Commit을 one transaction으로 완료했습니다. B3 전 fresh read/resolve가 필요합니다.")
		: TEXT("B2 Shared Profile Numeric Commit을 one transaction으로 완료했습니다. dependent Definition은 자동 Apply하지 않았으며 B3 전 fresh read/resolve가 필요합니다.");
	return true;
}
