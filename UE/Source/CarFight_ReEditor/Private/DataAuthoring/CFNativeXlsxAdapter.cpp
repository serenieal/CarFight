// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFNativeXlsxAdapter.cpp
// Version: v1.0.0
// Date: 2026-10-02
// Description: CF-FQ-058 CCAS-P0-07 repository-vendored OpenXLSX 0.5.1 기반
// Editor-only persistent .xlsx Workbook adapter production 구현입니다.
// Changelog:
// - v1.0.0: self-describing Workbook metadata, primary/child sheet roundtrip,
//   typed cell serialization, staged-write/reopen, initial baseline create,
//   external Excel lock guard를 구현.
// - v0.2.0: OpenXLSX 0.5.1 / pugixml 1.15 / miniz 3.0.2 UBT compile/link probe.
// Migration:
// - Runtime module dependency는 추가하지 않습니다.
// - build-time network fetch를 사용하지 않습니다.
// - Base Workbook direct overwrite는 금지합니다.
// - Physical worksheet order/name은 semantic authority가 아니며 __CF_META mapping으로만 해석합니다.

#include "DataAuthoring/CFNativeXlsxAdapter.h"

#include "DataAuthoring/CFContentCore.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#include "OpenXLSX.hpp"

namespace CFNativeXlsxPrivate
{
	/** Concrete physical Workbook format identity입니다. */
	const TCHAR* WorkbookFormatId = TEXT("CarFight.CCAS.XLSX/v1");

	/** Machine metadata worksheet 이름입니다. */
	const TCHAR* MetaSheetName = TEXT("__CF_META");

	/** Data worksheet machine header row입니다. */
	constexpr uint32 DataHeaderRow = 1;

	/** Data worksheet human-facing display label row입니다. */
	constexpr uint32 DataLabelRow = 2;

	/** Data worksheet 첫 data row입니다. */
	constexpr uint32 DataFirstRow = 3;

	/** Metadata descriptor table 첫 row입니다. */
	constexpr uint32 MetaTableHeaderRow = 5;

	/** Metadata descriptor table 첫 payload row입니다. */
	constexpr uint32 MetaTableFirstRow = 6;

	/** Primary record reserved machine columns입니다. */
	const TArray<FString> PrimaryReservedColumns = {
		TEXT("__ContentId"),
		TEXT("__RowId"),
		TEXT("__ManagementState"),
		TEXT("__LifecycleState"),
		TEXT("__FamilyId"),
		TEXT("__BaseContentId"),
		TEXT("__DesignIntent"),
		TEXT("__VariantId"),
		TEXT("__RoleId"),
		TEXT("__ProductionWaveId"),
		TEXT("__Readiness"),
		TEXT("__RelativeIntentsJson"),
		TEXT("__DependenciesJson"),
		TEXT("__SharedRolesJson"),
		TEXT("__VariantOwnedRolesJson"),
		TEXT("__ProfilesJson")
	};

	/** Child collection reserved machine columns입니다. */
	const TArray<FString> ChildReservedColumns = {
		TEXT("__OwnerContentId"),
		TEXT("__ChildItemId"),
		TEXT("__Order"),
		TEXT("__DomainKey")
	};

	/** Metadata descriptor table machine columns입니다. */
	const TArray<FString> MetaColumns = {
		TEXT("Kind"),
		TEXT("SheetId"),
		TEXT("PhysicalSheetName"),
		TEXT("SheetRevision"),
		TEXT("ContentTypeId"),
		TEXT("ParentSheetId"),
		TEXT("CollectionId"),
		TEXT("CollectionKind"),
		TEXT("ColumnId"),
		TEXT("DisplayLabel"),
		TEXT("ValueType"),
		TEXT("Ownership"),
		TEXT("ExternalOwnerId"),
		TEXT("CanonicalUnitId"),
		TEXT("FieldDomainId"),
		TEXT("TextMode"),
		TEXT("AllowNone"),
		TEXT("Required")
	};

	/** FString을 UTF-8 std::string으로 변환합니다. */
	std::string ToUtf8(const FString& Value)
	{
		// UTF-8 conversion storage입니다.
		FTCHARToUTF8 Utf8(*Value);
		return std::string(Utf8.Get(), Utf8.Length());
	}

	/** UTF-8 std::string을 FString으로 변환합니다. */
	FString FromUtf8(const std::string& Value)
	{
		return FString(UTF8_TO_TCHAR(Value.c_str()));
	}

	/** Repository-relative path를 physical path로 안전하게 resolve합니다. */
	bool ResolveRepositoryPath(
		const FString& RepositoryRelativePath,
		FString& OutPhysicalPath,
		FString& OutError)
	{
		OutPhysicalPath.Reset();
		OutError.Reset();

		// Normalized repository-relative path입니다.
		FString NormalizedPath = RepositoryRelativePath;
		FPaths::NormalizeFilename(NormalizedPath);

		if (NormalizedPath.IsEmpty()
			|| !FPaths::IsRelative(NormalizedPath)
			|| NormalizedPath.Contains(TEXT(".."), ESearchCase::CaseSensitive)
			|| NormalizedPath.Contains(TEXT(":"), ESearchCase::CaseSensitive))
		{
			OutError = TEXT("Workbook path는 '..' 또는 drive prefix가 없는 repository-relative path여야 합니다.");
			return false;
		}

		if (!FPaths::GetExtension(NormalizedPath, true).Equals(TEXT(".xlsx"), ESearchCase::IgnoreCase))
		{
			OutError = TEXT("Workbook path는 .xlsx 확장자여야 합니다.");
			return false;
		}

		// CarFight repository root입니다.
		const FString RepositoryRoot = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectDir(), TEXT("..")));
		OutPhysicalPath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(RepositoryRoot, NormalizedPath));

		// OpenXLSX 0.5.1의 Windows narrow-path backend를 안전하게 제한합니다.
		for (const TCHAR Character : OutPhysicalPath)
		{
			if (Character > 0x7f)
			{
				OutError = TEXT("현재 OpenXLSX 0.5.1 vendored build는 Workbook physical path를 ASCII로 제한합니다. Cell 내용의 Unicode는 허용됩니다.");
				return false;
			}
		}
		return true;
	}

	/** Excel/LibreOffice가 생성하는 sibling lock file이 존재하는지 확인합니다. */
	bool HasExternalWorkbookLock(
		const FString& PhysicalWorkbookPath)
	{
		// Workbook parent directory입니다.
		const FString ParentDirectory = FPaths::GetPath(PhysicalWorkbookPath);

		// Workbook clean filename입니다.
		const FString WorkbookFilename = FPaths::GetCleanFilename(PhysicalWorkbookPath);

		// Office-style sibling lock path입니다.
		const FString LockPath = FPaths::Combine(
			ParentDirectory,
			TEXT("~$") + WorkbookFilename);
		return IFileManager::Get().FileExists(*LockPath);
	}

	/** OpenXLSX cell에 UTF-8 string 값을 기록합니다. */
	void WriteCell(
		OpenXLSX::XLWorksheet& Worksheet,
		const uint32 Row,
		const uint16 Column,
		const FString& Value)
	{
		Worksheet.cell(Row, Column).value() = ToUtf8(Value);
	}

	/** OpenXLSX cell을 canonical authored text로 읽습니다. */
	bool ReadCell(
		const OpenXLSX::XLWorksheet& Worksheet,
		const uint32 Row,
		const uint16 Column,
		FString& OutValue,
		FString& OutError)
	{
		OutValue.Reset();
		OutError.Reset();

		// Cell value proxy입니다.
		const OpenXLSX::XLCellValue CellValue = Worksheet.cell(Row, Column).value();
		switch (CellValue.type())
		{
		case OpenXLSX::XLValueType::Empty:
			return true;
		case OpenXLSX::XLValueType::String:
			OutValue = FromUtf8(CellValue.get<std::string>());
			return true;
		case OpenXLSX::XLValueType::Boolean:
			OutValue = CellValue.get<bool>() ? TEXT("true") : TEXT("false");
			return true;
		case OpenXLSX::XLValueType::Integer:
			OutValue = LexToString(CellValue.get<int64_t>());
			return true;
		case OpenXLSX::XLValueType::Float:
			OutValue = LexToString(CellValue.get<double>());
			return true;
		case OpenXLSX::XLValueType::Error:
			OutError = FString::Printf(
				TEXT("Workbook cell에 Excel error value가 있습니다: row=%u col=%u"),
				Row,
				static_cast<uint32>(Column));
			return false;
		default:
			OutError = TEXT("지원되지 않는 OpenXLSX cell value type입니다.");
			return false;
		}
	}

	/** Integer authored text를 strict int32로 parse합니다. */
	bool ParseInt32Text(
		const FString& Text,
		int32& OutValue,
		const FString& FieldName,
		FString& OutError)
	{
		if (!LexTryParseString(OutValue, *Text))
		{
			OutError = FieldName + TEXT(" 값을 int32로 parse할 수 없습니다: ") + Text;
			return false;
		}
		return true;
	}

	/** Boolean machine cell의 0/1 representation을 parse합니다. */
	bool ParseBoolText(
		const FString& Text,
		bool& OutValue,
		const FString& FieldName,
		FString& OutError)
	{
		if (Text.Equals(TEXT("1"), ESearchCase::CaseSensitive))
		{
			OutValue = true;
			return true;
		}
		if (Text.Equals(TEXT("0"), ESearchCase::CaseSensitive))
		{
			OutValue = false;
			return true;
		}
		OutError = FieldName + TEXT(" 값은 0 또는 1이어야 합니다.");
		return false;
	}

	/** Json object를 compact FString으로 직렬화합니다. */
	bool SerializeJsonObject(
		const TSharedRef<FJsonObject>& JsonObject,
		FString& OutJson,
		FString& OutError)
	{
		OutJson.Reset();
		OutError.Reset();

		// Compact JSON writer입니다.
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
		if (!FJsonSerializer::Serialize(JsonObject, Writer))
		{
			OutError = TEXT("Workbook machine JSON serialize에 실패했습니다.");
			return false;
		}
		return true;
	}

	/** FString에서 Json object를 parse합니다. */
	bool ParseJsonObject(
		const FString& JsonText,
		TSharedPtr<FJsonObject>& OutJsonObject,
		FString& OutError)
	{
		OutJsonObject.Reset();
		OutError.Reset();

		// JSON reader입니다.
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, OutJsonObject)
			|| !OutJsonObject.IsValid())
		{
			OutError = TEXT("Workbook machine JSON object parse에 실패했습니다.");
			return false;
		}
		return true;
	}

	/** String array를 compact JSON array wrapper로 직렬화합니다. */
	bool SerializeStringArray(
		const TArray<FString>& Values,
		FString& OutJson,
		FString& OutError)
	{
		// Wrapper JSON object입니다.
		TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();

		// JSON string values입니다.
		TArray<TSharedPtr<FJsonValue>> JsonValues;
		for (const FString& Value : Values)
		{
			JsonValues.Add(MakeShared<FJsonValueString>(Value));
		}
		Root->SetArrayField(TEXT("Values"), JsonValues);
		return SerializeJsonObject(Root, OutJson, OutError);
	}

	/** Compact JSON wrapper에서 string array를 parse합니다. */
	bool ParseStringArray(
		const FString& JsonText,
		TArray<FString>& OutValues,
		FString& OutError)
	{
		OutValues.Reset();
		if (JsonText.IsEmpty())
		{
			return true;
		}

		// Parsed wrapper object입니다.
		TSharedPtr<FJsonObject> Root;
		if (!ParseJsonObject(JsonText, Root, OutError))
		{
			return false;
		}

		// JSON string value array입니다.
		const TArray<TSharedPtr<FJsonValue>>* JsonValues = nullptr;
		if (!Root->TryGetArrayField(TEXT("Values"), JsonValues)
			|| JsonValues == nullptr)
		{
			OutError = TEXT("Workbook string array JSON에 Values array가 없습니다.");
			return false;
		}

		for (const TSharedPtr<FJsonValue>& JsonValue : *JsonValues)
		{
			if (!JsonValue.IsValid() || JsonValue->Type != EJson::String)
			{
				OutError = TEXT("Workbook string array JSON에 non-string value가 있습니다.");
				return false;
			}
			OutValues.Add(JsonValue->AsString());
		}
		return true;
	}

	/** RelativeIntent 배열을 machine JSON으로 직렬화합니다. */
	bool SerializeRelativeIntents(
		const TArray<FCFContentRelativeIntent>& Intents,
		FString& OutJson,
		FString& OutError)
	{
		// Wrapper object입니다.
		TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();

		// Relative intent JSON array입니다.
		TArray<TSharedPtr<FJsonValue>> JsonValues;
		for (const FCFContentRelativeIntent& Intent : Intents)
		{
			// One relative intent JSON object입니다.
			TSharedRef<FJsonObject> JsonIntent = MakeShared<FJsonObject>();
			JsonIntent->SetStringField(TEXT("DimensionId"), Intent.DimensionId);
			JsonIntent->SetNumberField(TEXT("Direction"), static_cast<int32>(Intent.Direction));
			JsonValues.Add(MakeShared<FJsonValueObject>(JsonIntent));
		}
		Root->SetArrayField(TEXT("Values"), JsonValues);
		return SerializeJsonObject(Root, OutJson, OutError);
	}

	/** Machine JSON에서 RelativeIntent 배열을 parse합니다. */
	bool ParseRelativeIntents(
		const FString& JsonText,
		TArray<FCFContentRelativeIntent>& OutIntents,
		FString& OutError)
	{
		OutIntents.Reset();
		if (JsonText.IsEmpty())
		{
			return true;
		}

		// Parsed wrapper object입니다.
		TSharedPtr<FJsonObject> Root;
		if (!ParseJsonObject(JsonText, Root, OutError))
		{
			return false;
		}

		// Relative intent JSON array입니다.
		const TArray<TSharedPtr<FJsonValue>>* JsonValues = nullptr;
		if (!Root->TryGetArrayField(TEXT("Values"), JsonValues)
			|| JsonValues == nullptr)
		{
			OutError = TEXT("RelativeIntents JSON에 Values array가 없습니다.");
			return false;
		}

		for (const TSharedPtr<FJsonValue>& JsonValue : *JsonValues)
		{
			// One relative intent JSON object입니다.
			const TSharedPtr<FJsonObject> JsonIntent =
				JsonValue.IsValid() ? JsonValue->AsObject() : nullptr;
			if (!JsonIntent.IsValid())
			{
				OutError = TEXT("RelativeIntents JSON item이 object가 아닙니다.");
				return false;
			}

			// Parsed relative intent입니다.
			FCFContentRelativeIntent Intent;
			// Numeric enum value입니다.
			double DirectionNumber = 0.0;
			if (!JsonIntent->TryGetStringField(TEXT("DimensionId"), Intent.DimensionId)
				|| !JsonIntent->TryGetNumberField(TEXT("Direction"), DirectionNumber)
				|| DirectionNumber < 0.0
				|| DirectionNumber > static_cast<double>(ECFContentRelativeDirection::Increase))
			{
				OutError = TEXT("RelativeIntents JSON item contract가 유효하지 않습니다.");
				return false;
			}

			Intent.Direction = static_cast<ECFContentRelativeDirection>(
				static_cast<uint8>(DirectionNumber));
			OutIntents.Add(MoveTemp(Intent));
		}
		return true;
	}

	/** TechnologyDependency 배열을 machine JSON으로 직렬화합니다. */
	bool SerializeDependencies(
		const TArray<FCFContentTechnologyDependency>& Dependencies,
		FString& OutJson,
		FString& OutError)
	{
		// Wrapper object입니다.
		TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();

		// Dependency JSON array입니다.
		TArray<TSharedPtr<FJsonValue>> JsonValues;
		for (const FCFContentTechnologyDependency& Dependency : Dependencies)
		{
			// One dependency JSON object입니다.
			TSharedRef<FJsonObject> JsonDependency = MakeShared<FJsonObject>();
			JsonDependency->SetStringField(TEXT("DependencyId"), Dependency.DependencyId);
			JsonDependency->SetNumberField(TEXT("State"), static_cast<int32>(Dependency.State));
			JsonDependency->SetStringField(TEXT("Reason"), Dependency.Reason);
			JsonValues.Add(MakeShared<FJsonValueObject>(JsonDependency));
		}
		Root->SetArrayField(TEXT("Values"), JsonValues);
		return SerializeJsonObject(Root, OutJson, OutError);
	}

	/** Machine JSON에서 TechnologyDependency 배열을 parse합니다. */
	bool ParseDependencies(
		const FString& JsonText,
		TArray<FCFContentTechnologyDependency>& OutDependencies,
		FString& OutError)
	{
		OutDependencies.Reset();
		if (JsonText.IsEmpty())
		{
			return true;
		}

		// Parsed wrapper object입니다.
		TSharedPtr<FJsonObject> Root;
		if (!ParseJsonObject(JsonText, Root, OutError))
		{
			return false;
		}

		// Dependency JSON array입니다.
		const TArray<TSharedPtr<FJsonValue>>* JsonValues = nullptr;
		if (!Root->TryGetArrayField(TEXT("Values"), JsonValues)
			|| JsonValues == nullptr)
		{
			OutError = TEXT("Dependencies JSON에 Values array가 없습니다.");
			return false;
		}

		for (const TSharedPtr<FJsonValue>& JsonValue : *JsonValues)
		{
			// One dependency JSON object입니다.
			const TSharedPtr<FJsonObject> JsonDependency =
				JsonValue.IsValid() ? JsonValue->AsObject() : nullptr;
			if (!JsonDependency.IsValid())
			{
				OutError = TEXT("Dependencies JSON item이 object가 아닙니다.");
				return false;
			}

			// Parsed dependency입니다.
			FCFContentTechnologyDependency Dependency;
			// Numeric dependency state입니다.
			double StateNumber = 0.0;
			if (!JsonDependency->TryGetStringField(TEXT("DependencyId"), Dependency.DependencyId)
				|| !JsonDependency->TryGetNumberField(TEXT("State"), StateNumber)
				|| !JsonDependency->TryGetStringField(TEXT("Reason"), Dependency.Reason)
				|| StateNumber < 0.0
				|| StateNumber > static_cast<double>(ECFContentDependencyState::Blocked))
			{
				OutError = TEXT("Dependencies JSON item contract가 유효하지 않습니다.");
				return false;
			}

			Dependency.State = static_cast<ECFContentDependencyState>(
				static_cast<uint8>(StateNumber));
			OutDependencies.Add(MoveTemp(Dependency));
		}
		return true;
	}

	/** Assigned Profile map을 machine JSON으로 직렬화합니다. */
	bool SerializeProfiles(
		const TMap<FString, FString>& Profiles,
		FString& OutJson,
		FString& OutError)
	{
		// Wrapper object입니다.
		TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();

		// Stable domain order입니다.
		TArray<FString> DomainIds;
		Profiles.GetKeys(DomainIds);
		DomainIds.Sort();

		// Profile mapping object입니다.
		TSharedRef<FJsonObject> Mapping = MakeShared<FJsonObject>();
		for (const FString& DomainId : DomainIds)
		{
			// Assigned profile id입니다.
			const FString* ProfileId = Profiles.Find(DomainId);
			Mapping->SetStringField(
				DomainId,
				ProfileId != nullptr ? *ProfileId : FString());
		}
		Root->SetObjectField(TEXT("Values"), Mapping);
		return SerializeJsonObject(Root, OutJson, OutError);
	}

	/** Machine JSON에서 Assigned Profile map을 parse합니다. */
	bool ParseProfiles(
		const FString& JsonText,
		TMap<FString, FString>& OutProfiles,
		FString& OutError)
	{
		OutProfiles.Reset();
		if (JsonText.IsEmpty())
		{
			return true;
		}

		// Parsed wrapper object입니다.
		TSharedPtr<FJsonObject> Root;
		if (!ParseJsonObject(JsonText, Root, OutError))
		{
			return false;
		}

		// Profile mapping object입니다.
		const TSharedPtr<FJsonObject>* Mapping = nullptr;
		if (!Root->TryGetObjectField(TEXT("Values"), Mapping)
			|| Mapping == nullptr
			|| !Mapping->IsValid())
		{
			OutError = TEXT("Profiles JSON에 Values object가 없습니다.");
			return false;
		}

		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*Mapping)->Values)
		{
			if (!Pair.Value.IsValid() || Pair.Value->Type != EJson::String)
			{
				OutError = TEXT("Profiles JSON value가 string이 아닙니다.");
				return false;
			}
			OutProfiles.Add(Pair.Key, Pair.Value->AsString());
		}
		return true;
	}

	/** Localized FText semantic을 adapter-private JSON text로 직렬화합니다. */
	bool SerializeLocalizedText(
		const FCFContentTextValue& TextValue,
		FString& OutCellText,
		FString& OutError)
	{
		// Localized text JSON object입니다.
		TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
		Root->SetStringField(TEXT("Namespace"), TextValue.Namespace);
		Root->SetStringField(TEXT("Key"), TextValue.Key);
		Root->SetStringField(TEXT("Source"), TextValue.SourceString);

		// Compact localized JSON입니다.
		FString JsonText;
		if (!SerializeJsonObject(Root, JsonText, OutError))
		{
			return false;
		}
		OutCellText = TEXT("@localized:") + JsonText;
		return true;
	}

	/** Adapter-private localized FText cell을 parse합니다. */
	bool ParseLocalizedText(
		const FString& CellText,
		const FCFContentFieldDescriptor& Descriptor,
		FCFContentValue& OutValue,
		FString& OutError)
	{
		if (Descriptor.ValueType != ECFContentValueType::Text
			|| Descriptor.TextMode == ECFContentTextMode::LiteralInvariant
			|| !CellText.StartsWith(TEXT("@localized:"), ESearchCase::CaseSensitive))
		{
			return false;
		}

		// Parsed localized text object입니다.
		TSharedPtr<FJsonObject> Root;
		if (!ParseJsonObject(CellText.Mid(11), Root, OutError))
		{
			return false;
		}

		OutValue = FCFContentValue();
		OutValue.Type = ECFContentValueType::Text;
		OutValue.State = ECFContentValueState::Value;
		OutValue.TextValue.Kind = ECFContentTextKind::Localized;

		if (!Root->TryGetStringField(TEXT("Namespace"), OutValue.TextValue.Namespace)
			|| !Root->TryGetStringField(TEXT("Key"), OutValue.TextValue.Key)
			|| !Root->TryGetStringField(TEXT("Source"), OutValue.TextValue.SourceString)
			|| OutValue.TextValue.Namespace.IsEmpty()
			|| OutValue.TextValue.Key.IsEmpty())
		{
			OutError = TEXT("Localized FText cell의 Namespace/Key/Source contract가 유효하지 않습니다.");
			return false;
		}
		return true;
	}

	/** Canonical typed value를 Workbook authored text로 직렬화합니다. */
	bool SerializeContentValue(
		const FCFContentValue& Value,
		const FCFContentFieldDescriptor& Descriptor,
		FString& OutCellText,
		FString& OutError)
	{
		OutCellText.Reset();
		OutError.Reset();

		if (Value.Type != Descriptor.ValueType)
		{
			OutError = TEXT("Workbook write value type이 field descriptor와 다릅니다.");
			return false;
		}

		if (Value.State == ECFContentValueState::Inherit)
		{
			return true;
		}
		if (Value.State == ECFContentValueState::None)
		{
			OutCellText = TEXT("@none");
			return true;
		}

		switch (Value.Type)
		{
		case ECFContentValueType::Boolean:
			OutCellText = Value.bBooleanValue ? TEXT("true") : TEXT("false");
			return true;
		case ECFContentValueType::SignedInteger:
			OutCellText = LexToString(Value.SignedIntegerValue);
			return true;
		case ECFContentValueType::UnsignedInteger:
			OutCellText = LexToString(Value.UnsignedIntegerValue);
			return true;
		case ECFContentValueType::Float:
			OutCellText = LexToString(static_cast<float>(Value.FloatingPointValue));
			return true;
		case ECFContentValueType::Double:
			OutCellText = LexToString(Value.FloatingPointValue);
			return true;
		case ECFContentValueType::Enum:
		case ECFContentValueType::NameId:
		case ECFContentValueType::ResourceReference:
			OutCellText = Value.StringValue;
			return true;
		case ECFContentValueType::ContentReference:
			OutCellText = Value.ContentReference.ToStableString();
			return true;
		case ECFContentValueType::String:
		{
			// String authored text입니다.
			const FString& StringValue = Value.StringValue;
			if (StringValue.IsEmpty())
			{
				OutCellText = TEXT("@empty");
			}
			else if (StringValue.StartsWith(TEXT("@"), ESearchCase::CaseSensitive))
			{
				OutCellText = TEXT("@") + StringValue;
			}
			else
			{
				OutCellText = StringValue;
			}
			return true;
		}
		case ECFContentValueType::Text:
			if (Value.TextValue.Kind == ECFContentTextKind::Localized)
			{
				return SerializeLocalizedText(Value.TextValue, OutCellText, OutError);
			}
			if (Value.TextValue.SourceString.IsEmpty())
			{
				OutCellText = TEXT("@empty");
			}
			else if (Value.TextValue.SourceString.StartsWith(TEXT("@"), ESearchCase::CaseSensitive))
			{
				OutCellText = TEXT("@") + Value.TextValue.SourceString;
			}
			else
			{
				OutCellText = Value.TextValue.SourceString;
			}
			return true;
		default:
			OutError = TEXT("지원되지 않는 canonical Workbook value type입니다.");
			return false;
		}
	}

	/** Workbook authored text를 canonical typed value로 parse합니다. */
	bool ParseContentValue(
		const FString& CellText,
		const FCFContentFieldDescriptor& Descriptor,
		FCFContentValue& OutValue,
		FString& OutError)
	{
		OutError.Reset();

		if (Descriptor.ValueType == ECFContentValueType::Text
			&& CellText.StartsWith(TEXT("@localized:"), ESearchCase::CaseSensitive))
		{
			return ParseLocalizedText(CellText, Descriptor, OutValue, OutError);
		}

		return FCFContentCanonicalizer::ParseCell(
			CellText,
			Descriptor,
			OutValue,
			OutError);
	}

	/** Worksheet row 1의 machine header를 exact map으로 읽습니다. */
	bool BuildHeaderMap(
		const OpenXLSX::XLWorksheet& Worksheet,
		TMap<FString, uint16>& OutHeaderMap,
		FString& OutError)
	{
		OutHeaderMap.Reset();
		OutError.Reset();

		// Worksheet used column count입니다.
		const uint16 ColumnCount = Worksheet.columnCount();
		for (uint16 Column = 1; Column <= ColumnCount; ++Column)
		{
			// Machine header text입니다.
			FString Header;
			if (!ReadCell(Worksheet, DataHeaderRow, Column, Header, OutError))
			{
				return false;
			}
			if (Header.IsEmpty())
			{
				continue;
			}
			if (OutHeaderMap.Contains(Header))
			{
				OutError = TEXT("Workbook data sheet에 duplicate machine header가 있습니다: ") + Header;
				return false;
			}
			OutHeaderMap.Add(Header, Column);
		}
		return true;
	}

	/** Required machine header의 exact column을 찾습니다. */
	bool RequireHeader(
		const TMap<FString, uint16>& HeaderMap,
		const FString& Header,
		uint16& OutColumn,
		FString& OutError)
	{
		// Exact header column입니다.
		const uint16* FoundColumn = HeaderMap.Find(Header);
		if (FoundColumn == nullptr)
		{
			OutError = TEXT("Workbook data sheet에 required machine header가 없습니다: ") + Header;
			return false;
		}
		OutColumn = *FoundColumn;
		return true;
	}

	/** Sheet descriptor를 exact SheetId로 찾습니다. */
	FCFContentSheetDescriptor* FindSheetMutable(
		FCFContentWorkbookModel& Workbook,
		const FString& SheetId)
	{
		return Workbook.Sheets.FindByPredicate(
			[&SheetId](const FCFContentSheetDescriptor& Sheet)
			{
				return Sheet.SheetId.Equals(SheetId, ESearchCase::CaseSensitive);
			});
	}

	/** Const sheet descriptor를 exact SheetId로 찾습니다. */
	const FCFContentSheetDescriptor* FindSheet(
		const FCFContentWorkbookModel& Workbook,
		const FString& SheetId)
	{
		return Workbook.Sheets.FindByPredicate(
			[&SheetId](const FCFContentSheetDescriptor& Sheet)
			{
				return Sheet.SheetId.Equals(SheetId, ESearchCase::CaseSensitive);
			});
	}

	/** Exact ContentKey record를 mutable pointer로 찾습니다. */
	FCFContentRecord* FindRecordMutable(
		FCFContentWorkbookModel& Workbook,
		const FCFContentKey& ContentKey)
	{
		return Workbook.Records.FindByPredicate(
			[&ContentKey](const FCFContentRecord& Record)
			{
				return Record.Key == ContentKey;
			});
	}

	/** Metadata sheet를 작성하고 logical -> physical sheet map을 반환합니다. */
	bool WriteMetadataSheet(
		OpenXLSX::XLWorksheet& MetaSheet,
		const FCFContentWorkbookModel& Workbook,
		TMap<FString, FString>& OutPhysicalBySheetId,
		FString& OutError)
	{
		OutPhysicalBySheetId.Reset();
		OutError.Reset();

		WriteCell(MetaSheet, 1, 1, TEXT("FormatId"));
		WriteCell(MetaSheet, 1, 2, WorkbookFormatId);
		WriteCell(MetaSheet, 2, 1, TEXT("WorkbookSourceId"));
		WriteCell(MetaSheet, 2, 2, Workbook.WorkbookSourceId);
		WriteCell(MetaSheet, 3, 1, TEXT("SchemaRevision"));
		WriteCell(MetaSheet, 3, 2, LexToString(Workbook.SchemaRevision));

		for (int32 ColumnIndex = 0; ColumnIndex < MetaColumns.Num(); ++ColumnIndex)
		{
			WriteCell(
				MetaSheet,
				MetaTableHeaderRow,
				static_cast<uint16>(ColumnIndex + 1),
				MetaColumns[ColumnIndex]);
		}

		// Next metadata payload row입니다.
		uint32 MetaRow = MetaTableFirstRow;
		for (int32 SheetIndex = 0; SheetIndex < Workbook.Sheets.Num(); ++SheetIndex)
		{
			// Logical sheet descriptor입니다.
			const FCFContentSheetDescriptor& Sheet = Workbook.Sheets[SheetIndex];

			// Deterministic physical worksheet name입니다.
			const FString PhysicalSheetName = FString::Printf(
				TEXT("CF%03d"),
				SheetIndex + 1);
			OutPhysicalBySheetId.Add(Sheet.SheetId, PhysicalSheetName);

			WriteCell(MetaSheet, MetaRow, 1, TEXT("Sheet"));
			WriteCell(MetaSheet, MetaRow, 2, Sheet.SheetId);
			WriteCell(MetaSheet, MetaRow, 3, PhysicalSheetName);
			WriteCell(MetaSheet, MetaRow, 4, LexToString(Sheet.SchemaRevision));
			WriteCell(MetaSheet, MetaRow, 5, Sheet.ContentTypeId.Value);
			WriteCell(MetaSheet, MetaRow, 6, Sheet.ParentSheetId);
			WriteCell(MetaSheet, MetaRow, 7, Sheet.CollectionId);
			WriteCell(MetaSheet, MetaRow, 8, LexToString(static_cast<int32>(Sheet.CollectionKind)));
			++MetaRow;

			for (const FCFContentFieldDescriptor& Field : Sheet.Fields)
			{
				WriteCell(MetaSheet, MetaRow, 1, TEXT("Field"));
				WriteCell(MetaSheet, MetaRow, 2, Sheet.SheetId);
				WriteCell(MetaSheet, MetaRow, 9, Field.ColumnId);
				WriteCell(MetaSheet, MetaRow, 10, Field.DisplayLabel);
				WriteCell(MetaSheet, MetaRow, 11, LexToString(static_cast<int32>(Field.ValueType)));
				WriteCell(MetaSheet, MetaRow, 12, LexToString(static_cast<int32>(Field.Ownership)));
				WriteCell(MetaSheet, MetaRow, 13, Field.ExternalOwnerId);
				WriteCell(MetaSheet, MetaRow, 14, Field.CanonicalUnitId);
				WriteCell(MetaSheet, MetaRow, 15, Field.FieldDomainId);
				WriteCell(MetaSheet, MetaRow, 16, LexToString(static_cast<int32>(Field.TextMode)));
				WriteCell(MetaSheet, MetaRow, 17, Field.bAllowNone ? TEXT("1") : TEXT("0"));
				WriteCell(MetaSheet, MetaRow, 18, Field.bRequired ? TEXT("1") : TEXT("0"));
				++MetaRow;
			}
		}
		return true;
	}

	/** Primary logical sheet의 rows를 작성합니다. */
	bool WritePrimarySheet(
		OpenXLSX::XLWorksheet& Worksheet,
		const FCFContentWorkbookModel& Workbook,
		const FCFContentSheetDescriptor& Sheet,
		FString& OutError)
	{
		OutError.Reset();

		// Next physical column index입니다.
		uint16 Column = 1;
		for (const FString& ReservedColumn : PrimaryReservedColumns)
		{
			WriteCell(Worksheet, DataHeaderRow, Column, ReservedColumn);
			WriteCell(Worksheet, DataLabelRow, Column, ReservedColumn);
			++Column;
		}
		for (const FCFContentFieldDescriptor& Field : Sheet.Fields)
		{
			WriteCell(Worksheet, DataHeaderRow, Column, Field.ColumnId);
			WriteCell(
				Worksheet,
				DataLabelRow,
				Column,
				Field.DisplayLabel.IsEmpty() ? Field.ColumnId : Field.DisplayLabel);
			++Column;
		}

		// Stable ContentId order의 record pointers입니다.
		TArray<const FCFContentRecord*> Records;
		for (const FCFContentRecord& Record : Workbook.Records)
		{
			if (Record.Key.ContentTypeId == Sheet.ContentTypeId)
			{
				Records.Add(&Record);
			}
		}
		Records.Sort(
			[](const FCFContentRecord& Left, const FCFContentRecord& Right)
			{
				return Left.Key.ContentId < Right.Key.ContentId;
			});

		// Next data row입니다.
		uint32 Row = DataFirstRow;
		for (const FCFContentRecord* Record : Records)
		{
			if (Record == nullptr)
			{
				continue;
			}

			WriteCell(Worksheet, Row, 1, Record->Key.ContentId);
			WriteCell(Worksheet, Row, 2, Record->RowId);
			WriteCell(Worksheet, Row, 3, LexToString(static_cast<int32>(Record->ManagementState)));
			WriteCell(Worksheet, Row, 4, LexToString(static_cast<int32>(Record->LifecycleState)));
			WriteCell(Worksheet, Row, 5, Record->AuthoringMetadata.FamilyId);
			WriteCell(Worksheet, Row, 6, Record->AuthoringMetadata.BaseContentId);
			WriteCell(Worksheet, Row, 7, Record->AuthoringMetadata.DesignIntent);
			WriteCell(Worksheet, Row, 8, Record->AuthoringMetadata.Planning.VariantId);
			WriteCell(Worksheet, Row, 9, Record->AuthoringMetadata.Planning.RoleId);
			WriteCell(Worksheet, Row, 10, Record->AuthoringMetadata.Planning.ProductionWaveId);
			WriteCell(Worksheet, Row, 11, LexToString(static_cast<int32>(Record->AuthoringMetadata.Planning.Readiness)));

			// Relative intents JSON cell입니다.
			FString RelativeIntentsJson;
			if (!SerializeRelativeIntents(
				Record->AuthoringMetadata.Planning.RelativeIntents,
				RelativeIntentsJson,
				OutError))
			{
				return false;
			}
			WriteCell(Worksheet, Row, 12, RelativeIntentsJson);

			// Dependencies JSON cell입니다.
			FString DependenciesJson;
			if (!SerializeDependencies(
				Record->AuthoringMetadata.Planning.TechnologyDependencies,
				DependenciesJson,
				OutError))
			{
				return false;
			}
			WriteCell(Worksheet, Row, 13, DependenciesJson);

			// Shared role IDs JSON cell입니다.
			FString SharedRolesJson;
			if (!SerializeStringArray(
				Record->AuthoringMetadata.Planning.SharedDataRoleIds,
				SharedRolesJson,
				OutError))
			{
				return false;
			}
			WriteCell(Worksheet, Row, 14, SharedRolesJson);

			// Variant-owned role IDs JSON cell입니다.
			FString VariantRolesJson;
			if (!SerializeStringArray(
				Record->AuthoringMetadata.Planning.VariantOwnedDataRoleIds,
				VariantRolesJson,
				OutError))
			{
				return false;
			}
			WriteCell(Worksheet, Row, 15, VariantRolesJson);

			// Assigned profiles JSON cell입니다.
			FString ProfilesJson;
			if (!SerializeProfiles(
				Record->AuthoringMetadata.AssignedProfileIdsByDomain,
				ProfilesJson,
				OutError))
			{
				return false;
			}
			WriteCell(Worksheet, Row, 16, ProfilesJson);

			// First provider field column입니다.
			uint16 FieldColumn = static_cast<uint16>(PrimaryReservedColumns.Num() + 1);
			for (const FCFContentFieldDescriptor& Field : Sheet.Fields)
			{
				// Record field value입니다.
				const FCFContentValue* Value = Record->Fields.Find(Field.ColumnId);

				// Missing map entry는 explicit INHERIT cell로 직렬화합니다.
				FCFContentValue InheritValue;
				InheritValue.Type = Field.ValueType;
				InheritValue.State = ECFContentValueState::Inherit;

				// Serialized authored cell text입니다.
				FString CellText;
				if (!SerializeContentValue(
					Value != nullptr ? *Value : InheritValue,
					Field,
					CellText,
					OutError))
				{
					OutError = Record->Key.ToStableString() + TEXT(".") + Field.ColumnId + TEXT(": ") + OutError;
					return false;
				}

				WriteCell(Worksheet, Row, FieldColumn, CellText);
				++FieldColumn;
			}
			++Row;
		}
		return true;
	}

	/** Child collection logical sheet의 rows를 작성합니다. */
	bool WriteChildSheet(
		OpenXLSX::XLWorksheet& Worksheet,
		const FCFContentWorkbookModel& Workbook,
		const FCFContentSheetDescriptor& Sheet,
		FString& OutError)
	{
		OutError.Reset();

		// Next physical column index입니다.
		uint16 Column = 1;
		for (const FString& ReservedColumn : ChildReservedColumns)
		{
			WriteCell(Worksheet, DataHeaderRow, Column, ReservedColumn);
			WriteCell(Worksheet, DataLabelRow, Column, ReservedColumn);
			++Column;
		}
		for (const FCFContentFieldDescriptor& Field : Sheet.Fields)
		{
			WriteCell(Worksheet, DataHeaderRow, Column, Field.ColumnId);
			WriteCell(
				Worksheet,
				DataLabelRow,
				Column,
				Field.DisplayLabel.IsEmpty() ? Field.ColumnId : Field.DisplayLabel);
			++Column;
		}

		// Stable owner ContentId order의 record pointers입니다.
		TArray<const FCFContentRecord*> Records;
		for (const FCFContentRecord& Record : Workbook.Records)
		{
			if (Record.Key.ContentTypeId == Sheet.ContentTypeId)
			{
				Records.Add(&Record);
			}
		}
		Records.Sort(
			[](const FCFContentRecord& Left, const FCFContentRecord& Right)
			{
				return Left.Key.ContentId < Right.Key.ContentId;
			});

		// Next child data row입니다.
		uint32 Row = DataFirstRow;
		for (const FCFContentRecord* Record : Records)
		{
			if (Record == nullptr)
			{
				continue;
			}

			// Exact collection on this record입니다.
			const FCFContentCollection* Collection = Record->Collections.Find(Sheet.CollectionId);
			if (Collection == nullptr)
			{
				continue;
			}

			if (Collection->Kind != Sheet.CollectionKind)
			{
				OutError = Record->Key.ToStableString()
					+ TEXT(".")
					+ Sheet.CollectionId
					+ TEXT(": collection kind가 sheet descriptor와 다릅니다.");
				return false;
			}

			// Stable physical output ordering입니다.
			TArray<const FCFContentCollectionItem*> Items;
			for (const FCFContentCollectionItem& Item : Collection->Items)
			{
				Items.Add(&Item);
			}
			Items.Sort(
				[&Sheet](const FCFContentCollectionItem& Left, const FCFContentCollectionItem& Right)
				{
					if (Sheet.CollectionKind == ECFContentCollectionKind::OrderedList
						&& Left.Order != Right.Order)
					{
						return Left.Order < Right.Order;
					}
					if (Sheet.CollectionKind == ECFContentCollectionKind::KeyedCollection
						&& Left.DomainKey != Right.DomainKey)
					{
						return Left.DomainKey < Right.DomainKey;
					}
					return Left.ChildItemId < Right.ChildItemId;
				});

			for (const FCFContentCollectionItem* Item : Items)
			{
				if (Item == nullptr)
				{
					continue;
				}

				WriteCell(Worksheet, Row, 1, Record->Key.ContentId);
				WriteCell(Worksheet, Row, 2, Item->ChildItemId);
				WriteCell(
					Worksheet,
					Row,
					3,
					Item->Order == INDEX_NONE ? FString() : LexToString(Item->Order));
				WriteCell(Worksheet, Row, 4, Item->DomainKey);

				// First child provider field column입니다.
				uint16 FieldColumn = static_cast<uint16>(ChildReservedColumns.Num() + 1);
				for (const FCFContentFieldDescriptor& Field : Sheet.Fields)
				{
					// Child field value입니다.
					const FCFContentValue* Value = Item->Fields.Find(Field.ColumnId);

					// Missing map entry는 explicit INHERIT cell로 직렬화합니다.
					FCFContentValue InheritValue;
					InheritValue.Type = Field.ValueType;
					InheritValue.State = ECFContentValueState::Inherit;

					// Serialized authored cell text입니다.
					FString CellText;
					if (!SerializeContentValue(
						Value != nullptr ? *Value : InheritValue,
						Field,
						CellText,
						OutError))
					{
						OutError = Record->Key.ToStableString()
							+ TEXT(".")
							+ Sheet.CollectionId
							+ TEXT(".")
							+ Item->ChildItemId
							+ TEXT(".")
							+ Field.ColumnId
							+ TEXT(": ")
							+ OutError;
						return false;
					}

					WriteCell(Worksheet, Row, FieldColumn, CellText);
					++FieldColumn;
				}
				++Row;
			}
		}
		return true;
	}

	/** Canonical model을 새 physical .xlsx file로 작성합니다. Existing file overwrite는 caller가 통제합니다. */
	bool WriteWorkbookPhysical(
		const FString& PhysicalWorkbookPath,
		const FCFContentWorkbookModel& Workbook,
		FString& OutError)
	{
		OutError.Reset();

		// Full canonical validation diagnostics입니다.
		TArray<FCFContentValidationIssue> ValidationIssues;
		if (!FCFContentSchemaValidator::ValidateWorkbook(Workbook, ValidationIssues))
		{
			OutError = TEXT("Persistent .xlsx write 전에 canonical Workbook validation이 실패했습니다.");
			if (!ValidationIssues.IsEmpty())
			{
				OutError += TEXT(" FirstIssue=") + ValidationIssues[0].Code
					+ TEXT(" @ ")
					+ ValidationIssues[0].Path;
			}
			return false;
		}

		// Output parent directory입니다.
		const FString ParentDirectory = FPaths::GetPath(PhysicalWorkbookPath);
		if (!IFileManager::Get().MakeDirectory(*ParentDirectory, true)
			&& !IFileManager::Get().DirectoryExists(*ParentDirectory))
		{
			OutError = TEXT("Workbook output parent directory 생성에 실패했습니다.");
			return false;
		}

		try
		{
			// OpenXLSX document입니다.
			OpenXLSX::XLDocument Document;
			Document.create(ToUtf8(PhysicalWorkbookPath), OpenXLSX::XLForceOverwrite);

			// Default worksheet를 machine metadata sheet로 재사용합니다.
			OpenXLSX::XLWorksheet MetaSheet = Document.workbook().worksheet("Sheet1");
			MetaSheet.setName(ToUtf8(MetaSheetName));

			// Logical -> physical sheet mapping입니다.
			TMap<FString, FString> PhysicalBySheetId;
			if (!WriteMetadataSheet(
				MetaSheet,
				Workbook,
				PhysicalBySheetId,
				OutError))
			{
				Document.close();
				return false;
			}

			for (const FCFContentSheetDescriptor& Sheet : Workbook.Sheets)
			{
				// Physical sheet name입니다.
				const FString* PhysicalSheetName = PhysicalBySheetId.Find(Sheet.SheetId);
				if (PhysicalSheetName == nullptr)
				{
					Document.close();
					OutError = TEXT("Logical -> physical worksheet mapping 생성이 누락됐습니다: ") + Sheet.SheetId;
					return false;
				}

				Document.workbook().addWorksheet(ToUtf8(*PhysicalSheetName));

				// Newly created data worksheet입니다.
				OpenXLSX::XLWorksheet DataSheet =
					Document.workbook().worksheet(ToUtf8(*PhysicalSheetName));

				if (Sheet.IsChildSheet())
				{
					if (!WriteChildSheet(DataSheet, Workbook, Sheet, OutError))
					{
						Document.close();
						return false;
					}
				}
				else
				{
					if (!WritePrimarySheet(DataSheet, Workbook, Sheet, OutError))
					{
						Document.close();
						return false;
					}
				}
			}

			Document.save();
			Document.close();
		}
		catch (const std::exception& Exception)
		{
			OutError = TEXT("OpenXLSX persistent write exception: ")
				+ FromUtf8(Exception.what());
			return false;
		}

		return IFileManager::Get().FileExists(*PhysicalWorkbookPath);
	}

	/** Metadata sheet에서 Workbook/schema descriptor와 physical mapping을 읽습니다. */
	bool ReadMetadataSheet(
		const OpenXLSX::XLWorksheet& MetaSheet,
		FCFContentWorkbookModel& OutWorkbook,
		TMap<FString, FString>& OutPhysicalBySheetId,
		FString& OutError)
	{
		OutWorkbook = FCFContentWorkbookModel();
		OutPhysicalBySheetId.Reset();
		OutError.Reset();

		// Format id입니다.
		FString FormatId;
		if (!ReadCell(MetaSheet, 1, 2, FormatId, OutError)
			|| !FormatId.Equals(WorkbookFormatId, ESearchCase::CaseSensitive))
		{
			if (OutError.IsEmpty())
			{
				OutError = TEXT("Unsupported Workbook format identity: ") + FormatId;
			}
			return false;
		}

		if (!ReadCell(MetaSheet, 2, 2, OutWorkbook.WorkbookSourceId, OutError))
		{
			return false;
		}

		// Workbook schema revision text입니다.
		FString WorkbookRevisionText;
		if (!ReadCell(MetaSheet, 3, 2, WorkbookRevisionText, OutError)
			|| !ParseInt32Text(
				WorkbookRevisionText,
				OutWorkbook.SchemaRevision,
				TEXT("Workbook.SchemaRevision"),
				OutError))
		{
			return false;
		}

		// Meta machine header map입니다.
		TMap<FString, uint16> MetaHeaderMap;
		for (uint16 Column = 1; Column <= MetaSheet.columnCount(); ++Column)
		{
			// Meta header text입니다.
			FString Header;
			if (!ReadCell(MetaSheet, MetaTableHeaderRow, Column, Header, OutError))
			{
				return false;
			}
			if (!Header.IsEmpty())
			{
				if (MetaHeaderMap.Contains(Header))
				{
					OutError = TEXT("Metadata table에 duplicate header가 있습니다: ") + Header;
					return false;
				}
				MetaHeaderMap.Add(Header, Column);
			}
		}

		for (const FString& RequiredHeader : MetaColumns)
		{
			if (!MetaHeaderMap.Contains(RequiredHeader))
			{
				OutError = TEXT("Metadata table에 required header가 없습니다: ") + RequiredHeader;
				return false;
			}
		}

		// First pass에서 logical sheet descriptors를 생성합니다.
		for (uint32 Row = MetaTableFirstRow; Row <= MetaSheet.rowCount(); ++Row)
		{
			// Metadata row kind입니다.
			FString Kind;
			if (!ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("Kind")], Kind, OutError))
			{
				return false;
			}
			if (!Kind.Equals(TEXT("Sheet"), ESearchCase::CaseSensitive))
			{
				continue;
			}

			// New logical sheet descriptor입니다.
			FCFContentSheetDescriptor Sheet;
			// Physical worksheet name입니다.
			FString PhysicalSheetName;
			// Sheet revision text입니다.
			FString RevisionText;
			// Collection kind text입니다.
			FString CollectionKindText;

			if (!ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("SheetId")], Sheet.SheetId, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("PhysicalSheetName")], PhysicalSheetName, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("SheetRevision")], RevisionText, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("ContentTypeId")], Sheet.ContentTypeId.Value, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("ParentSheetId")], Sheet.ParentSheetId, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("CollectionId")], Sheet.CollectionId, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("CollectionKind")], CollectionKindText, OutError))
			{
				return false;
			}

			if (FindSheet(OutWorkbook, Sheet.SheetId) != nullptr)
			{
				OutError = TEXT("Metadata table에 duplicate SheetId가 있습니다: ") + Sheet.SheetId;
				return false;
			}
			if (PhysicalSheetName.IsEmpty() || OutPhysicalBySheetId.FindKey(PhysicalSheetName) != nullptr)
			{
				OutError = TEXT("Metadata table physical worksheet name이 비었거나 중복입니다: ") + PhysicalSheetName;
				return false;
			}

			if (!ParseInt32Text(
				RevisionText,
				Sheet.SchemaRevision,
				Sheet.SheetId + TEXT(".SchemaRevision"),
				OutError))
			{
				return false;
			}

			// Parsed collection kind ordinal입니다.
			int32 CollectionKindOrdinal = 0;
			if (!ParseInt32Text(
				CollectionKindText,
				CollectionKindOrdinal,
				Sheet.SheetId + TEXT(".CollectionKind"),
				OutError)
				|| CollectionKindOrdinal < 0
				|| CollectionKindOrdinal > static_cast<int32>(ECFContentCollectionKind::UnorderedSet))
			{
				OutError = Sheet.SheetId + TEXT(".CollectionKind enum ordinal이 범위를 벗어났습니다.");
				return false;
			}
			Sheet.CollectionKind = static_cast<ECFContentCollectionKind>(
				static_cast<uint8>(CollectionKindOrdinal));

			OutPhysicalBySheetId.Add(Sheet.SheetId, PhysicalSheetName);
			OutWorkbook.Sheets.Add(MoveTemp(Sheet));
		}

		// Second pass에서 field descriptors를 각 logical sheet에 추가합니다.
		for (uint32 Row = MetaTableFirstRow; Row <= MetaSheet.rowCount(); ++Row)
		{
			// Metadata row kind입니다.
			FString Kind;
			if (!ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("Kind")], Kind, OutError))
			{
				return false;
			}
			if (!Kind.Equals(TEXT("Field"), ESearchCase::CaseSensitive))
			{
				continue;
			}

			// Field owner logical SheetId입니다.
			FString SheetId;
			if (!ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("SheetId")], SheetId, OutError))
			{
				return false;
			}

			// Mutable owner sheet descriptor입니다.
			FCFContentSheetDescriptor* Sheet = FindSheetMutable(OutWorkbook, SheetId);
			if (Sheet == nullptr)
			{
				OutError = TEXT("Field metadata가 존재하지 않는 SheetId를 참조합니다: ") + SheetId;
				return false;
			}

			// New field descriptor입니다.
			FCFContentFieldDescriptor Field;
			// Enum/bool machine texts입니다.
			FString ValueTypeText;
			FString OwnershipText;
			FString TextModeText;
			FString AllowNoneText;
			FString RequiredText;

			if (!ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("ColumnId")], Field.ColumnId, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("DisplayLabel")], Field.DisplayLabel, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("ValueType")], ValueTypeText, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("Ownership")], OwnershipText, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("ExternalOwnerId")], Field.ExternalOwnerId, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("CanonicalUnitId")], Field.CanonicalUnitId, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("FieldDomainId")], Field.FieldDomainId, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("TextMode")], TextModeText, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("AllowNone")], AllowNoneText, OutError)
				|| !ReadCell(MetaSheet, Row, MetaHeaderMap[TEXT("Required")], RequiredText, OutError))
			{
				return false;
			}

			// Field value type ordinal입니다.
			int32 ValueTypeOrdinal = 0;
			// Field ownership ordinal입니다.
			int32 OwnershipOrdinal = 0;
			// Field text mode ordinal입니다.
			int32 TextModeOrdinal = 0;
			if (!ParseInt32Text(ValueTypeText, ValueTypeOrdinal, Field.ColumnId + TEXT(".ValueType"), OutError)
				|| !ParseInt32Text(OwnershipText, OwnershipOrdinal, Field.ColumnId + TEXT(".Ownership"), OutError)
				|| !ParseInt32Text(TextModeText, TextModeOrdinal, Field.ColumnId + TEXT(".TextMode"), OutError)
				|| ValueTypeOrdinal < 0
				|| ValueTypeOrdinal > static_cast<int32>(ECFContentValueType::ResourceReference)
				|| OwnershipOrdinal < 0
				|| OwnershipOrdinal > static_cast<int32>(ECFContentFieldOwnership::Deprecated)
				|| TextModeOrdinal < 0
				|| TextModeOrdinal > static_cast<int32>(ECFContentTextMode::LiteralInvariant))
			{
				if (OutError.IsEmpty())
				{
					OutError = TEXT("Field metadata enum ordinal이 범위를 벗어났습니다: ") + Field.ColumnId;
				}
				return false;
			}

			Field.ValueType = static_cast<ECFContentValueType>(static_cast<uint8>(ValueTypeOrdinal));
			Field.Ownership = static_cast<ECFContentFieldOwnership>(static_cast<uint8>(OwnershipOrdinal));
			Field.TextMode = static_cast<ECFContentTextMode>(static_cast<uint8>(TextModeOrdinal));

			if (!ParseBoolText(AllowNoneText, Field.bAllowNone, Field.ColumnId + TEXT(".AllowNone"), OutError)
				|| !ParseBoolText(RequiredText, Field.bRequired, Field.ColumnId + TEXT(".Required"), OutError))
			{
				return false;
			}

			Sheet->Fields.Add(MoveTemp(Field));
		}

		return true;
	}

	/** Primary sheet rows를 canonical records로 parse합니다. */
	bool ReadPrimarySheet(
		const OpenXLSX::XLWorksheet& Worksheet,
		const FCFContentSheetDescriptor& Sheet,
		FCFContentWorkbookModel& OutWorkbook,
		FString& OutError)
	{
		OutError.Reset();

		// Machine header map입니다.
		TMap<FString, uint16> HeaderMap;
		if (!BuildHeaderMap(Worksheet, HeaderMap, OutError))
		{
			return false;
		}

		for (const FString& RequiredHeader : PrimaryReservedColumns)
		{
			// Required reserved column입니다.
			uint16 RequiredColumn = 0;
			if (!RequireHeader(HeaderMap, RequiredHeader, RequiredColumn, OutError))
			{
				return false;
			}
		}
		for (const FCFContentFieldDescriptor& Field : Sheet.Fields)
		{
			// Required provider field column입니다.
			uint16 RequiredColumn = 0;
			if (!RequireHeader(HeaderMap, Field.ColumnId, RequiredColumn, OutError))
			{
				return false;
			}
		}

		for (uint32 Row = DataFirstRow; Row <= Worksheet.rowCount(); ++Row)
		{
			// ContentId cell입니다.
			FString ContentId;
			if (!ReadCell(Worksheet, Row, HeaderMap[TEXT("__ContentId")], ContentId, OutError))
			{
				return false;
			}
			if (ContentId.IsEmpty())
			{
				continue;
			}

			// Parsed canonical record입니다.
			FCFContentRecord Record;
			Record.Key.ContentTypeId = Sheet.ContentTypeId;
			Record.Key.ContentId = ContentId;

			// Reserved record cells입니다.
			FString ManagementStateText;
			FString LifecycleStateText;
			FString ReadinessText;
			FString RelativeIntentsJson;
			FString DependenciesJson;
			FString SharedRolesJson;
			FString VariantRolesJson;
			FString ProfilesJson;

			if (!ReadCell(Worksheet, Row, HeaderMap[TEXT("__RowId")], Record.RowId, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__ManagementState")], ManagementStateText, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__LifecycleState")], LifecycleStateText, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__FamilyId")], Record.AuthoringMetadata.FamilyId, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__BaseContentId")], Record.AuthoringMetadata.BaseContentId, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__DesignIntent")], Record.AuthoringMetadata.DesignIntent, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__VariantId")], Record.AuthoringMetadata.Planning.VariantId, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__RoleId")], Record.AuthoringMetadata.Planning.RoleId, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__ProductionWaveId")], Record.AuthoringMetadata.Planning.ProductionWaveId, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__Readiness")], ReadinessText, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__RelativeIntentsJson")], RelativeIntentsJson, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__DependenciesJson")], DependenciesJson, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__SharedRolesJson")], SharedRolesJson, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__VariantOwnedRolesJson")], VariantRolesJson, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__ProfilesJson")], ProfilesJson, OutError))
			{
				return false;
			}

			// Record enum ordinals입니다.
			int32 ManagementStateOrdinal = 0;
			int32 LifecycleStateOrdinal = 0;
			int32 ReadinessOrdinal = 0;
			if (!ParseInt32Text(ManagementStateText, ManagementStateOrdinal, Record.Key.ToStableString() + TEXT(".ManagementState"), OutError)
				|| !ParseInt32Text(LifecycleStateText, LifecycleStateOrdinal, Record.Key.ToStableString() + TEXT(".LifecycleState"), OutError)
				|| !ParseInt32Text(ReadinessText, ReadinessOrdinal, Record.Key.ToStableString() + TEXT(".Readiness"), OutError)
				|| ManagementStateOrdinal < 0
				|| ManagementStateOrdinal > static_cast<int32>(ECFContentManagementState::ExternalReadOnly)
				|| LifecycleStateOrdinal < 0
				|| LifecycleStateOrdinal > static_cast<int32>(ECFContentLifecycleState::Retired)
				|| ReadinessOrdinal < 0
				|| ReadinessOrdinal > static_cast<int32>(ECFContentPlanningReadiness::Blocked))
			{
				if (OutError.IsEmpty())
				{
					OutError = Record.Key.ToStableString() + TEXT(": record enum ordinal이 범위를 벗어났습니다.");
				}
				return false;
			}

			Record.ManagementState = static_cast<ECFContentManagementState>(static_cast<uint8>(ManagementStateOrdinal));
			Record.LifecycleState = static_cast<ECFContentLifecycleState>(static_cast<uint8>(LifecycleStateOrdinal));
			Record.AuthoringMetadata.Planning.Readiness =
				static_cast<ECFContentPlanningReadiness>(static_cast<uint8>(ReadinessOrdinal));

			if (!ParseRelativeIntents(
				RelativeIntentsJson,
				Record.AuthoringMetadata.Planning.RelativeIntents,
				OutError)
				|| !ParseDependencies(
					DependenciesJson,
					Record.AuthoringMetadata.Planning.TechnologyDependencies,
					OutError)
				|| !ParseStringArray(
					SharedRolesJson,
					Record.AuthoringMetadata.Planning.SharedDataRoleIds,
					OutError)
				|| !ParseStringArray(
					VariantRolesJson,
					Record.AuthoringMetadata.Planning.VariantOwnedDataRoleIds,
					OutError)
				|| !ParseProfiles(
					ProfilesJson,
					Record.AuthoringMetadata.AssignedProfileIdsByDomain,
					OutError))
			{
				OutError = Record.Key.ToStableString() + TEXT(": ") + OutError;
				return false;
			}

			for (const FCFContentFieldDescriptor& Field : Sheet.Fields)
			{
				// Authored field cell text입니다.
				FString CellText;
				if (!ReadCell(Worksheet, Row, HeaderMap[Field.ColumnId], CellText, OutError))
				{
					return false;
				}

				// Parsed canonical field value입니다.
				FCFContentValue Value;
				if (!ParseContentValue(CellText, Field, Value, OutError))
				{
					OutError = Record.Key.ToStableString() + TEXT(".") + Field.ColumnId + TEXT(": ") + OutError;
					return false;
				}
				Record.Fields.Add(Field.ColumnId, MoveTemp(Value));
			}

			OutWorkbook.Records.Add(MoveTemp(Record));
		}
		return true;
	}

	/** Child collection sheet rows를 canonical collections로 parse합니다. */
	bool ReadChildSheet(
		const OpenXLSX::XLWorksheet& Worksheet,
		const FCFContentSheetDescriptor& Sheet,
		FCFContentWorkbookModel& OutWorkbook,
		FString& OutError)
	{
		OutError.Reset();

		// Machine header map입니다.
		TMap<FString, uint16> HeaderMap;
		if (!BuildHeaderMap(Worksheet, HeaderMap, OutError))
		{
			return false;
		}

		for (const FString& RequiredHeader : ChildReservedColumns)
		{
			// Required child reserved column입니다.
			uint16 RequiredColumn = 0;
			if (!RequireHeader(HeaderMap, RequiredHeader, RequiredColumn, OutError))
			{
				return false;
			}
		}
		for (const FCFContentFieldDescriptor& Field : Sheet.Fields)
		{
			// Required child provider field column입니다.
			uint16 RequiredColumn = 0;
			if (!RequireHeader(HeaderMap, Field.ColumnId, RequiredColumn, OutError))
			{
				return false;
			}
		}

		for (uint32 Row = DataFirstRow; Row <= Worksheet.rowCount(); ++Row)
		{
			// Owner ContentId cell입니다.
			FString OwnerContentId;
			if (!ReadCell(Worksheet, Row, HeaderMap[TEXT("__OwnerContentId")], OwnerContentId, OutError))
			{
				return false;
			}
			if (OwnerContentId.IsEmpty())
			{
				continue;
			}

			// Owner logical ContentKey입니다.
			FCFContentKey OwnerKey;
			OwnerKey.ContentTypeId = Sheet.ContentTypeId;
			OwnerKey.ContentId = OwnerContentId;

			// Existing primary owner record입니다.
			FCFContentRecord* OwnerRecord = FindRecordMutable(OutWorkbook, OwnerKey);
			if (OwnerRecord == nullptr)
			{
				OutError = TEXT("Child sheet가 존재하지 않는 primary record를 참조합니다: ")
					+ OwnerKey.ToStableString();
				return false;
			}

			// Parsed child item입니다.
			FCFContentCollectionItem Item;
			// Order authored text입니다.
			FString OrderText;

			if (!ReadCell(Worksheet, Row, HeaderMap[TEXT("__ChildItemId")], Item.ChildItemId, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__Order")], OrderText, OutError)
				|| !ReadCell(Worksheet, Row, HeaderMap[TEXT("__DomainKey")], Item.DomainKey, OutError))
			{
				return false;
			}

			Item.Order = INDEX_NONE;
			if (!OrderText.IsEmpty()
				&& !ParseInt32Text(
					OrderText,
					Item.Order,
					OwnerKey.ToStableString() + TEXT(".") + Sheet.CollectionId + TEXT(".Order"),
					OutError))
			{
				return false;
			}

			for (const FCFContentFieldDescriptor& Field : Sheet.Fields)
			{
				// Authored child field cell text입니다.
				FString CellText;
				if (!ReadCell(Worksheet, Row, HeaderMap[Field.ColumnId], CellText, OutError))
				{
					return false;
				}

				// Parsed child field value입니다.
				FCFContentValue Value;
				if (!ParseContentValue(CellText, Field, Value, OutError))
				{
					OutError = OwnerKey.ToStableString()
						+ TEXT(".")
						+ Sheet.CollectionId
						+ TEXT(".")
						+ Item.ChildItemId
						+ TEXT(".")
						+ Field.ColumnId
						+ TEXT(": ")
						+ OutError;
					return false;
				}
				Item.Fields.Add(Field.ColumnId, MoveTemp(Value));
			}

			// Existing or new collection container입니다.
			FCFContentCollection* Collection = OwnerRecord->Collections.Find(Sheet.CollectionId);
			if (Collection == nullptr)
			{
				FCFContentCollection NewCollection;
				NewCollection.CollectionId = Sheet.CollectionId;
				NewCollection.Kind = Sheet.CollectionKind;
				OwnerRecord->Collections.Add(Sheet.CollectionId, MoveTemp(NewCollection));
				Collection = OwnerRecord->Collections.Find(Sheet.CollectionId);
			}

			if (Collection == nullptr || Collection->Kind != Sheet.CollectionKind)
			{
				OutError = OwnerKey.ToStableString()
					+ TEXT(".")
					+ Sheet.CollectionId
					+ TEXT(": child collection kind mismatch.");
				return false;
			}

			Collection->Items.Add(MoveTemp(Item));
		}
		return true;
	}
}

// Adapter identity와 verified persistence capability를 반환합니다.
FCFWorkbookAdapterInfo FCFNativeXlsxAdapter::DescribeAdapter() const
{
	// Adapter capability metadata입니다.
	FCFWorkbookAdapterInfo Info;
	Info.AdapterId = TEXT("CarFight.OpenXLSX");
	Info.AdapterVersion = TEXT("0.5.1+cf-v1");
	Info.bPreservesPresentation = false;
	Info.bPreservesProtection = false;
	return Info;
}

// Canonical .xlsx Workbook을 typed canonical model로 read-only parse합니다.
bool FCFNativeXlsxAdapter::ReadWorkbook(
	const FString& WorkbookPath,
	FCFContentWorkbookModel& OutWorkbook,
	FString& OutError)
{
	OutWorkbook = FCFContentWorkbookModel();
	OutError.Reset();

	// Resolved physical Workbook path입니다.
	FString PhysicalWorkbookPath;
	if (!CFNativeXlsxPrivate::ResolveRepositoryPath(
		WorkbookPath,
		PhysicalWorkbookPath,
		OutError))
	{
		return false;
	}

	if (!IFileManager::Get().FileExists(*PhysicalWorkbookPath))
	{
		OutError = TEXT("Workbook file이 존재하지 않습니다: ") + WorkbookPath;
		return false;
	}

	try
	{
		// OpenXLSX document입니다.
		OpenXLSX::XLDocument Document;
		Document.open(CFNativeXlsxPrivate::ToUtf8(PhysicalWorkbookPath));

		if (!Document.workbook().sheetExists(
			CFNativeXlsxPrivate::ToUtf8(CFNativeXlsxPrivate::MetaSheetName)))
		{
			Document.close();
			OutError = TEXT("CCAS machine metadata worksheet(__CF_META)가 없습니다.");
			return false;
		}

		// Machine metadata worksheet입니다.
		OpenXLSX::XLWorksheet MetaSheet =
			Document.workbook().worksheet(
				CFNativeXlsxPrivate::ToUtf8(CFNativeXlsxPrivate::MetaSheetName));

		// Logical -> physical worksheet mapping입니다.
		TMap<FString, FString> PhysicalBySheetId;
		if (!CFNativeXlsxPrivate::ReadMetadataSheet(
			MetaSheet,
			OutWorkbook,
			PhysicalBySheetId,
			OutError))
		{
			Document.close();
			return false;
		}

		// Primary sheets must be parsed before child collection sheets.
		for (const FCFContentSheetDescriptor& Sheet : OutWorkbook.Sheets)
		{
			if (Sheet.IsChildSheet())
			{
				continue;
			}

			// Physical data sheet name입니다.
			const FString* PhysicalSheetName = PhysicalBySheetId.Find(Sheet.SheetId);
			if (PhysicalSheetName == nullptr
				|| !Document.workbook().sheetExists(
					CFNativeXlsxPrivate::ToUtf8(*PhysicalSheetName)))
			{
				Document.close();
				OutError = TEXT("Metadata가 가리키는 primary physical worksheet가 없습니다: ") + Sheet.SheetId;
				return false;
			}

			// Primary worksheet입니다.
			OpenXLSX::XLWorksheet DataSheet =
				Document.workbook().worksheet(
					CFNativeXlsxPrivate::ToUtf8(*PhysicalSheetName));
			if (!CFNativeXlsxPrivate::ReadPrimarySheet(
				DataSheet,
				Sheet,
				OutWorkbook,
				OutError))
			{
				Document.close();
				return false;
			}
		}

		for (const FCFContentSheetDescriptor& Sheet : OutWorkbook.Sheets)
		{
			if (!Sheet.IsChildSheet())
			{
				continue;
			}

			// Physical child data sheet name입니다.
			const FString* PhysicalSheetName = PhysicalBySheetId.Find(Sheet.SheetId);
			if (PhysicalSheetName == nullptr
				|| !Document.workbook().sheetExists(
					CFNativeXlsxPrivate::ToUtf8(*PhysicalSheetName)))
			{
				Document.close();
				OutError = TEXT("Metadata가 가리키는 child physical worksheet가 없습니다: ") + Sheet.SheetId;
				return false;
			}

			// Child worksheet입니다.
			OpenXLSX::XLWorksheet DataSheet =
				Document.workbook().worksheet(
					CFNativeXlsxPrivate::ToUtf8(*PhysicalSheetName));
			if (!CFNativeXlsxPrivate::ReadChildSheet(
				DataSheet,
				Sheet,
				OutWorkbook,
				OutError))
			{
				Document.close();
				return false;
			}
		}

		Document.close();
	}
	catch (const std::exception& Exception)
	{
		OutError = TEXT("OpenXLSX persistent read exception: ")
			+ CFNativeXlsxPrivate::FromUtf8(Exception.what());
		return false;
	}

	// Read result full canonical validation diagnostics입니다.
	TArray<FCFContentValidationIssue> ValidationIssues;
	if (!FCFContentSchemaValidator::ValidateWorkbook(
		OutWorkbook,
		ValidationIssues))
	{
		OutError = TEXT("Persistent .xlsx read 뒤 canonical Workbook validation이 실패했습니다.");
		if (!ValidationIssues.IsEmpty())
		{
			OutError += TEXT(" FirstIssue=") + ValidationIssues[0].Code
				+ TEXT(" @ ")
				+ ValidationIssues[0].Path;
		}
		return false;
	}

	return true;
}

// Base Workbook을 직접 덮어쓰지 않고 sibling staged .xlsx로 작성합니다.
bool FCFNativeXlsxAdapter::WriteStagedWorkbook(
	const FString& BaseWorkbookPath,
	const FString& StagedWorkbookPath,
	const FCFContentWorkbookModel& ReviewedWorkbook,
	FString& OutError)
{
	OutError.Reset();

	if (BaseWorkbookPath.Equals(StagedWorkbookPath, ESearchCase::CaseSensitive))
	{
		OutError = TEXT("Base Workbook direct overwrite는 금지됩니다.");
		return false;
	}

	// Resolved base physical path입니다.
	FString BasePhysicalPath;
	if (!CFNativeXlsxPrivate::ResolveRepositoryPath(
		BaseWorkbookPath,
		BasePhysicalPath,
		OutError))
	{
		return false;
	}

	// Resolved staged physical path입니다.
	FString StagedPhysicalPath;
	if (!CFNativeXlsxPrivate::ResolveRepositoryPath(
		StagedWorkbookPath,
		StagedPhysicalPath,
		OutError))
	{
		return false;
	}

	if (!FPaths::GetPath(BasePhysicalPath).Equals(
		FPaths::GetPath(StagedPhysicalPath),
		ESearchCase::IgnoreCase))
	{
		OutError = TEXT("Staged Workbook은 Base Workbook과 동일 directory의 sibling .xlsx여야 합니다.");
		return false;
	}

	if (CFNativeXlsxPrivate::HasExternalWorkbookLock(BasePhysicalPath))
	{
		OutError = TEXT("Base Workbook의 Office lock file(~$...)이 존재합니다. 외부 편집기를 닫은 뒤 새 Review가 필요합니다.");
		return false;
	}

	if (IFileManager::Get().FileExists(*StagedPhysicalPath))
	{
		OutError = TEXT("동일 staged Workbook candidate가 이미 존재합니다. blind overwrite/retry는 금지됩니다.");
		return false;
	}

	// Fresh base Workbook canonical model입니다.
	FCFContentWorkbookModel BaseWorkbook;
	if (!ReadWorkbook(BaseWorkbookPath, BaseWorkbook, OutError))
	{
		OutError = TEXT("Base Workbook read가 실패했습니다: ") + OutError;
		return false;
	}

	if (!BaseWorkbook.WorkbookSourceId.Equals(
		ReviewedWorkbook.WorkbookSourceId,
		ESearchCase::CaseSensitive))
	{
		OutError = TEXT("Reviewed Workbook의 WorkbookSourceId가 Base Workbook과 다릅니다.");
		return false;
	}

	return CFNativeXlsxPrivate::WriteWorkbookPhysical(
		StagedPhysicalPath,
		ReviewedWorkbook,
		OutError);
}

// Staged .xlsx를 실제 파일에서 다시 열어 canonical model을 반환합니다.
bool FCFNativeXlsxAdapter::ReopenWorkbook(
	const FString& StagedWorkbookPath,
	FCFContentWorkbookModel& OutWorkbook,
	FString& OutError)
{
	return ReadWorkbook(StagedWorkbookPath, OutWorkbook, OutError);
}

// Existing file overwrite 없이 initial migration baseline .xlsx를 생성합니다.
bool FCFNativeXlsxAdapter::CreateWorkbookFile(
	const FString& WorkbookPath,
	const FCFContentWorkbookModel& Workbook,
	FString& OutError)
{
	OutError.Reset();

	// Resolved output physical path입니다.
	FString PhysicalWorkbookPath;
	if (!CFNativeXlsxPrivate::ResolveRepositoryPath(
		WorkbookPath,
		PhysicalWorkbookPath,
		OutError))
	{
		return false;
	}

	if (IFileManager::Get().FileExists(*PhysicalWorkbookPath))
	{
		OutError = TEXT("Initial baseline CreateWorkbookFile은 existing file overwrite를 허용하지 않습니다.");
		return false;
	}

	return CFNativeXlsxPrivate::WriteWorkbookPhysical(
		PhysicalWorkbookPath,
		Workbook,
		OutError);
}
