// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCore.cpp
// Version: v1.2.0
// Date: 2026-09-30
// Description: CF-FQ-058 CCAS canonical Content Core와 P0-06 planning metadata/lifecycle implementation입니다.
// Changelog:
// - v1.2.0: PlanningMetadata/Lifecycle semantic validation과 hashing, BaseCatalogSnapshotFingerprint shape validation을 추가.
// - v1.0.1: Float/Double authored text를 ASCII decimal/exponent strict grammar로 선검증해 locale comma와 trailing junk를 fail-closed 처리.
// - v1.0.0: stable identity, tri-state typed cell parse, collection/schema validation, dependency cycle guard,
//   presentation-invariant WorkbookSemanticHash와 deterministic migration preview를 최초 구현.
// Migration:
// - Product UObject를 읽거나 쓰지 않습니다.
// - Existing CFBatch/CFDA durable writer를 중복 구현하지 않고 canonical in-memory contract만 제공합니다.

#include "DataAuthoring/CFContentCore.h"

#include "DataAuthoring/CFDACommonPrimitives.h"

#include <limits>

namespace CFContentCorePrivate
{
	// Stable machine ID에서 허용하는 ASCII 문자 집합인지 확인합니다.
	bool IsAllowedStableIdCharacter(const TCHAR Character)
	{
		return (Character >= TEXT('a') && Character <= TEXT('z'))
			|| (Character >= TEXT('A') && Character <= TEXT('Z'))
			|| (Character >= TEXT('0') && Character <= TEXT('9'))
			|| Character == TEXT('_')
			|| Character == TEXT('-')
			|| Character == TEXT('.')
			|| Character == TEXT('/');
	}

	// Stable machine ID 공통 grammar를 검증합니다.
	bool IsStableId(const FString& Value)
	{
		if (Value.IsEmpty()
			|| Value.StartsWith(TEXT("/"), ESearchCase::CaseSensitive)
			|| Value.EndsWith(TEXT("/"), ESearchCase::CaseSensitive)
			|| Value.Contains(TEXT("//"), ESearchCase::CaseSensitive))
		{
			return false;
		}

		for (const TCHAR Character : Value)
		{
			if (!IsAllowedStableIdCharacter(Character))
			{
				return false;
			}
		}
		return true;
	}

	// Blocking validation issue를 append합니다.
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

	// uint32 값을 canonical big-endian bytes로 append합니다.
	void AppendUInt32(TArray<uint8>& OutBytes, const uint32 Value)
	{
		OutBytes.Add(static_cast<uint8>((Value >> 24) & 0xff));
		OutBytes.Add(static_cast<uint8>((Value >> 16) & 0xff));
		OutBytes.Add(static_cast<uint8>((Value >> 8) & 0xff));
		OutBytes.Add(static_cast<uint8>(Value & 0xff));
	}

	// uint64 값을 canonical big-endian bytes로 append합니다.
	void AppendUInt64(TArray<uint8>& OutBytes, const uint64 Value)
	{
		for (int32 ByteShift = 56; ByteShift >= 0; ByteShift -= 8)
		{
			OutBytes.Add(static_cast<uint8>((Value >> ByteShift) & 0xff));
		}
	}

	// int32 값을 deterministic integer bytes로 append합니다.
	void AppendInt32(TArray<uint8>& OutBytes, const int32 Value)
	{
		AppendUInt32(OutBytes, static_cast<uint32>(Value));
	}

	// enum/byte tag를 append합니다.
	void AppendByte(TArray<uint8>& OutBytes, const uint8 Value)
	{
		OutBytes.Add(Value);
	}

	// UTF-8 string을 length-prefix framing으로 append합니다.
	void AppendString(TArray<uint8>& OutBytes, const FString& Value)
	{
		// UTF-8 encoded canonical string입니다.
		FTCHARToUTF8 Utf8(*Value);
		// UTF-8 byte count입니다.
		const uint32 ByteCount = static_cast<uint32>(Utf8.Length());
		AppendUInt32(OutBytes, ByteCount);
		if (ByteCount > 0)
		{
			OutBytes.Append(reinterpret_cast<const uint8*>(Utf8.Get()), static_cast<int32>(ByteCount));
		}
	}

	// IEEE-754 float를 -0 -> +0 정규화 후 big-endian으로 append합니다.
	void AppendFloat(TArray<uint8>& OutBytes, const float Value)
	{
		// Negative zero와 positive zero를 동일 semantic으로 만드는 canonical value입니다.
		const float CanonicalValue = Value == 0.0f ? 0.0f : Value;
		// Float raw bit pattern입니다.
		uint32 FloatBits = 0;
		static_assert(sizeof(FloatBits) == sizeof(CanonicalValue), "float canonicalization requires 32-bit IEEE-754.");
		FMemory::Memcpy(&FloatBits, &CanonicalValue, sizeof(FloatBits));
		AppendUInt32(OutBytes, FloatBits);
	}

	// IEEE-754 double을 -0 -> +0 정규화 후 big-endian으로 append합니다.
	void AppendDouble(TArray<uint8>& OutBytes, const double Value)
	{
		// Negative zero와 positive zero를 동일 semantic으로 만드는 canonical value입니다.
		const double CanonicalValue = Value == 0.0 ? 0.0 : Value;
		// Double raw bit pattern입니다.
		uint64 DoubleBits = 0;
		static_assert(sizeof(DoubleBits) == sizeof(CanonicalValue), "double canonicalization requires 64-bit IEEE-754.");
		FMemory::Memcpy(&DoubleBits, &CanonicalValue, sizeof(DoubleBits));
		AppendUInt64(OutBytes, DoubleBits);
	}

	// Canonical ContentKey를 hash byte stream에 append합니다.
	void AppendContentKey(TArray<uint8>& OutBytes, const FCFContentKey& Key)
	{
		AppendString(OutBytes, Key.ContentTypeId.Value);
		AppendString(OutBytes, Key.ContentId);
	}

	// Canonical typed value를 hash byte stream에 append합니다.
	void AppendContentValue(TArray<uint8>& OutBytes, const FCFContentValue& Value)
	{
		AppendByte(OutBytes, static_cast<uint8>(Value.Type));
		AppendByte(OutBytes, static_cast<uint8>(Value.State));
		if (Value.State != ECFContentValueState::Value)
		{
			return;
		}

		switch (Value.Type)
		{
		case ECFContentValueType::Boolean:
			AppendByte(OutBytes, Value.bBooleanValue ? 1 : 0);
			break;
		case ECFContentValueType::SignedInteger:
			AppendUInt64(OutBytes, static_cast<uint64>(Value.SignedIntegerValue));
			break;
		case ECFContentValueType::UnsignedInteger:
			AppendUInt64(OutBytes, Value.UnsignedIntegerValue);
			break;
		case ECFContentValueType::Float:
			AppendFloat(OutBytes, static_cast<float>(Value.FloatingPointValue));
			break;
		case ECFContentValueType::Double:
			AppendDouble(OutBytes, Value.FloatingPointValue);
			break;
		case ECFContentValueType::NameId:
			AppendString(OutBytes, Value.StringValue.ToLower());
			break;
		case ECFContentValueType::String:
		case ECFContentValueType::Enum:
		case ECFContentValueType::ResourceReference:
			AppendString(OutBytes, Value.StringValue);
			break;
		case ECFContentValueType::Text:
			AppendByte(OutBytes, static_cast<uint8>(Value.TextValue.Kind));
			AppendString(OutBytes, Value.TextValue.Namespace);
			AppendString(OutBytes, Value.TextValue.Key);
			AppendString(OutBytes, Value.TextValue.SourceString);
			break;
		case ECFContentValueType::ContentReference:
			AppendContentKey(OutBytes, Value.ContentReference);
			break;
		default:
			break;
		}
	}

	// Scalar field map을 stable ColumnId 순서로 hash byte stream에 append합니다.
	void AppendFieldMap(
		TArray<uint8>& OutBytes,
		const TMap<FString, FCFContentValue>& Fields)
	{
		// Stable ColumnId keys입니다.
		TArray<FString> FieldIds;
		Fields.GetKeys(FieldIds);
		FieldIds.Sort();

		AppendUInt32(OutBytes, static_cast<uint32>(FieldIds.Num()));
		for (const FString& FieldId : FieldIds)
		{
			// Stable ColumnId에 대응하는 canonical value입니다.
			const FCFContentValue* Value = Fields.Find(FieldId);
			AppendString(OutBytes, FieldId);
			if (Value != nullptr)
			{
				AppendContentValue(OutBytes, *Value);
			}
		}
	}

	// UnorderedSet canonical ordering에 사용할 field-map semantic fingerprint를 생성합니다.
	FString BuildFieldMapSemanticKey(const TMap<FString, FCFContentValue>& Fields)
	{
		// Canonical field-map bytes입니다.
		TArray<uint8> CanonicalBytes;
		AppendFieldMap(CanonicalBytes, Fields);

		// Canonical sha256 fingerprint입니다.
		FString SemanticHash;
		// Hash primitive error text입니다.
		FString HashError;
		if (!CFDACommonPrimitives::HashCanonicalBytes(CanonicalBytes, SemanticHash, HashError))
		{
			return FString();
		}
		return SemanticHash;
	}

	// Collection kind에 맞는 canonical child item 순서를 생성합니다.
	void SortCollectionItems(
		const FCFContentCollection& Collection,
		TArray<const FCFContentCollectionItem*>& OutItems)
	{
		OutItems.Reset();
		for (const FCFContentCollectionItem& Item : Collection.Items)
		{
			OutItems.Add(&Item);
		}

		switch (Collection.Kind)
		{
		case ECFContentCollectionKind::OrderedList:
			OutItems.Sort([](const FCFContentCollectionItem& Left, const FCFContentCollectionItem& Right)
			{
				if (Left.Order != Right.Order)
				{
					return Left.Order < Right.Order;
				}
				return Left.ChildItemId < Right.ChildItemId;
			});
			break;
		case ECFContentCollectionKind::KeyedCollection:
			OutItems.Sort([](const FCFContentCollectionItem& Left, const FCFContentCollectionItem& Right)
			{
				if (Left.DomainKey != Right.DomainKey)
				{
					return Left.DomainKey < Right.DomainKey;
				}
				return Left.ChildItemId < Right.ChildItemId;
			});
			break;
		case ECFContentCollectionKind::UnorderedSet:
			OutItems.Sort([](const FCFContentCollectionItem& Left, const FCFContentCollectionItem& Right)
			{
				// Left item의 canonical typed-value fingerprint입니다.
				const FString LeftSemanticKey = BuildFieldMapSemanticKey(Left.Fields);
				// Right item의 canonical typed-value fingerprint입니다.
				const FString RightSemanticKey = BuildFieldMapSemanticKey(Right.Fields);
				if (LeftSemanticKey != RightSemanticKey)
				{
					return LeftSemanticKey < RightSemanticKey;
				}
				return Left.ChildItemId < Right.ChildItemId;
			});
			break;
		default:
			break;
		}
	}

	// Collection 하나를 semantic ordering contract에 따라 hash byte stream에 append합니다.
	void AppendCollection(TArray<uint8>& OutBytes, const FCFContentCollection& Collection)
	{
		AppendString(OutBytes, Collection.CollectionId);
		AppendByte(OutBytes, static_cast<uint8>(Collection.Kind));

		// Collection semantic order로 정렬된 child pointer 집합입니다.
		TArray<const FCFContentCollectionItem*> SortedItems;
		SortCollectionItems(Collection, SortedItems);
		AppendUInt32(OutBytes, static_cast<uint32>(SortedItems.Num()));

		for (const FCFContentCollectionItem* Item : SortedItems)
		{
			if (Item == nullptr)
			{
				continue;
			}

			switch (Collection.Kind)
			{
			case ECFContentCollectionKind::OrderedList:
				AppendString(OutBytes, Item->ChildItemId);
				AppendInt32(OutBytes, Item->Order);
				break;
			case ECFContentCollectionKind::KeyedCollection:
				AppendString(OutBytes, Item->DomainKey);
				break;
			case ECFContentCollectionKind::UnorderedSet:
				break;
			default:
				break;
			}
			AppendFieldMap(OutBytes, Item->Fields);
		}
	}

	// Schema descriptor 하나를 presentation-only label/physical position 없이 append합니다.
	void AppendSheetDescriptor(TArray<uint8>& OutBytes, const FCFContentSheetDescriptor& Sheet)
	{
		AppendString(OutBytes, Sheet.SheetId);
		AppendInt32(OutBytes, Sheet.SchemaRevision);
		AppendString(OutBytes, Sheet.ContentTypeId.Value);
		AppendString(OutBytes, Sheet.ParentSheetId);
		AppendString(OutBytes, Sheet.CollectionId);
		AppendByte(OutBytes, static_cast<uint8>(Sheet.CollectionKind));

		// ColumnId canonical order로 정렬된 descriptor pointer입니다.
		TArray<const FCFContentFieldDescriptor*> SortedFields;
		for (const FCFContentFieldDescriptor& Field : Sheet.Fields)
		{
			SortedFields.Add(&Field);
		}
		SortedFields.Sort([](const FCFContentFieldDescriptor& Left, const FCFContentFieldDescriptor& Right)
		{
			return Left.ColumnId < Right.ColumnId;
		});

		AppendUInt32(OutBytes, static_cast<uint32>(SortedFields.Num()));
		for (const FCFContentFieldDescriptor* Field : SortedFields)
		{
			if (Field == nullptr)
			{
				continue;
			}

			AppendString(OutBytes, Field->ColumnId);
			AppendByte(OutBytes, static_cast<uint8>(Field->ValueType));
			AppendByte(OutBytes, static_cast<uint8>(Field->Ownership));
			AppendString(OutBytes, Field->ExternalOwnerId);
			AppendString(OutBytes, Field->CanonicalUnitId);
			AppendString(OutBytes, Field->FieldDomainId);
			AppendByte(OutBytes, static_cast<uint8>(Field->TextMode));
			AppendByte(OutBytes, Field->bAllowNone ? 1 : 0);
			AppendByte(OutBytes, Field->bRequired ? 1 : 0);
		}
	}

	// Workbook record 하나를 physical row/column order와 독립적으로 append합니다.
	void AppendRecord(TArray<uint8>& OutBytes, const FCFContentRecord& Record)
	{
		AppendContentKey(OutBytes, Record.Key);
		AppendString(OutBytes, Record.RowId);
		AppendByte(OutBytes, static_cast<uint8>(Record.ManagementState));
		AppendByte(OutBytes, static_cast<uint8>(Record.LifecycleState));
		AppendString(OutBytes, Record.AuthoringMetadata.FamilyId);
		AppendString(OutBytes, Record.AuthoringMetadata.BaseContentId);
		AppendString(OutBytes, Record.AuthoringMetadata.DesignIntent);

		// P0-06 typed planning metadata입니다.
		const FCFContentPlanningMetadata& Planning = Record.AuthoringMetadata.Planning;
		AppendString(OutBytes, Planning.VariantId);
		AppendString(OutBytes, Planning.RoleId);
		AppendString(OutBytes, Planning.ProductionWaveId);
		AppendByte(OutBytes, static_cast<uint8>(Planning.Readiness));

		// Stable DimensionId 순서의 relative intents입니다.
		TArray<const FCFContentRelativeIntent*> SortedRelativeIntents;
		for (const FCFContentRelativeIntent& RelativeIntent : Planning.RelativeIntents)
		{
			SortedRelativeIntents.Add(&RelativeIntent);
		}
		SortedRelativeIntents.Sort([](const FCFContentRelativeIntent& Left, const FCFContentRelativeIntent& Right)
		{
			return Left.DimensionId < Right.DimensionId;
		});
		AppendUInt32(OutBytes, static_cast<uint32>(SortedRelativeIntents.Num()));
		for (const FCFContentRelativeIntent* RelativeIntent : SortedRelativeIntents)
		{
			if (RelativeIntent != nullptr)
			{
				AppendString(OutBytes, RelativeIntent->DimensionId);
				AppendByte(OutBytes, static_cast<uint8>(RelativeIntent->Direction));
			}
		}

		// Stable DependencyId 순서의 technology dependencies입니다.
		TArray<const FCFContentTechnologyDependency*> SortedDependencies;
		for (const FCFContentTechnologyDependency& Dependency : Planning.TechnologyDependencies)
		{
			SortedDependencies.Add(&Dependency);
		}
		SortedDependencies.Sort([](const FCFContentTechnologyDependency& Left, const FCFContentTechnologyDependency& Right)
		{
			return Left.DependencyId < Right.DependencyId;
		});
		AppendUInt32(OutBytes, static_cast<uint32>(SortedDependencies.Num()));
		for (const FCFContentTechnologyDependency* Dependency : SortedDependencies)
		{
			if (Dependency != nullptr)
			{
				AppendString(OutBytes, Dependency->DependencyId);
				AppendByte(OutBytes, static_cast<uint8>(Dependency->State));
				AppendString(OutBytes, Dependency->Reason);
			}
		}

		// Stable lexical order의 shared data roles입니다.
		TArray<FString> SharedDataRoleIds = Planning.SharedDataRoleIds;
		SharedDataRoleIds.Sort();
		AppendUInt32(OutBytes, static_cast<uint32>(SharedDataRoleIds.Num()));
		for (const FString& DataRoleId : SharedDataRoleIds)
		{
			AppendString(OutBytes, DataRoleId);
		}

		// Stable lexical order의 variant-owned data roles입니다.
		TArray<FString> VariantOwnedDataRoleIds = Planning.VariantOwnedDataRoleIds;
		VariantOwnedDataRoleIds.Sort();
		AppendUInt32(OutBytes, static_cast<uint32>(VariantOwnedDataRoleIds.Num()));
		for (const FString& DataRoleId : VariantOwnedDataRoleIds)
		{
			AppendString(OutBytes, DataRoleId);
		}

		// Stable field-domain order로 정렬할 assigned Profile domain keys입니다.
		TArray<FString> ProfileDomainIds;
		Record.AuthoringMetadata.AssignedProfileIdsByDomain.GetKeys(ProfileDomainIds);
		ProfileDomainIds.Sort();
		AppendUInt32(OutBytes, static_cast<uint32>(ProfileDomainIds.Num()));
		for (const FString& ProfileDomainId : ProfileDomainIds)
		{
			// Stable field-domain에 할당된 Profile identity입니다.
			const FString* ProfileId = Record.AuthoringMetadata.AssignedProfileIdsByDomain.Find(ProfileDomainId);
			AppendString(OutBytes, ProfileDomainId);
			AppendString(OutBytes, ProfileId != nullptr ? *ProfileId : FString());
		}

		AppendFieldMap(OutBytes, Record.Fields);

		// Stable CollectionId keys입니다.
		TArray<FString> CollectionIds;
		Record.Collections.GetKeys(CollectionIds);
		CollectionIds.Sort();

		AppendUInt32(OutBytes, static_cast<uint32>(CollectionIds.Num()));
		for (const FString& CollectionId : CollectionIds)
		{
			// Stable CollectionId에 대응하는 canonical collection입니다.
			const FCFContentCollection* Collection = Record.Collections.Find(CollectionId);
			if (Collection != nullptr)
			{
				AppendCollection(OutBytes, *Collection);
			}
		}
	}

	// Field value가 descriptor의 declared type/state contract와 일치하는지 확인합니다.
	bool ValidateValueAgainstDescriptor(
		const FCFContentValue& Value,
		const FCFContentFieldDescriptor& Descriptor,
		const FString& Path,
		TArray<FCFContentValidationIssue>& OutIssues)
	{
		if (Value.Type != Descriptor.ValueType)
		{
			AddIssue(OutIssues, TEXT("ValueTypeMismatch"), Path, TEXT("Canonical value type이 field descriptor와 다릅니다."));
			return false;
		}

		if (Value.State == ECFContentValueState::None && !Descriptor.bAllowNone)
		{
			AddIssue(OutIssues, TEXT("NoneNotAllowed"), Path, TEXT("이 field schema는 NONE 상태를 허용하지 않습니다."));
			return false;
		}

		if (Value.State != ECFContentValueState::Value)
		{
			return true;
		}

		switch (Value.Type)
		{
		case ECFContentValueType::Float:
		case ECFContentValueType::Double:
			if (!FMath::IsFinite(Value.FloatingPointValue))
			{
				AddIssue(OutIssues, TEXT("NonFiniteNumber"), Path, TEXT("NaN 또는 Infinity는 canonical numeric value로 허용하지 않습니다."));
				return false;
			}
			break;
		case ECFContentValueType::Enum:
		case ECFContentValueType::NameId:
			if (!CFIsStableContentId(Value.StringValue))
			{
				AddIssue(OutIssues, TEXT("InvalidStableId"), Path, TEXT("Enum/NameId VALUE가 canonical stable ID grammar를 따르지 않습니다."));
				return false;
			}
			break;
		case ECFContentValueType::ResourceReference:
			if (!CFIsStableResourceId(Value.StringValue))
			{
				AddIssue(OutIssues, TEXT("InvalidResourceId"), Path, TEXT("ResourceReference VALUE가 canonical ResourceId grammar를 따르지 않습니다."));
				return false;
			}
			break;
		case ECFContentValueType::ContentReference:
			if (!Value.ContentReference.IsValid())
			{
				AddIssue(OutIssues, TEXT("InvalidContentReference"), Path, TEXT("ContentReference VALUE의 ContentKey가 invalid입니다."));
				return false;
			}
			break;
		case ECFContentValueType::Text:
			if (Value.TextValue.Kind == ECFContentTextKind::Literal)
			{
				if (!Value.TextValue.Namespace.IsEmpty() || !Value.TextValue.Key.IsEmpty())
				{
					AddIssue(OutIssues, TEXT("InvalidLiteralText"), Path, TEXT("Literal FText semantic은 Namespace/Key를 소유하지 않습니다."));
					return false;
				}
				if (Descriptor.TextMode != ECFContentTextMode::LiteralInvariant)
				{
					AddIssue(OutIssues, TEXT("TextModeMismatch"), Path, TEXT("Literal FText VALUE는 LiteralInvariant descriptor에서만 허용합니다."));
					return false;
				}
			}
			else
			{
				if (Value.TextValue.Namespace.IsEmpty() || Value.TextValue.Key.IsEmpty())
				{
					AddIssue(OutIssues, TEXT("InvalidLocalizedText"), Path, TEXT("Localized FText semantic에는 Namespace와 Key가 모두 필요합니다."));
					return false;
				}
				if (Descriptor.TextMode != ECFContentTextMode::PreserveIdentity
					&& Descriptor.TextMode != ECFContentTextMode::StableLocalized)
				{
					AddIssue(OutIssues, TEXT("TextModeMismatch"), Path, TEXT("Localized FText VALUE는 PreserveIdentity 또는 StableLocalized descriptor가 필요합니다."));
					return false;
				}
			}
			break;
		default:
			break;
		}
		return true;
	}

	// Exact primary sheet descriptor를 ContentTypeId 기준으로 찾습니다.
	const FCFContentSheetDescriptor* FindPrimarySheet(
		const FCFContentWorkbookModel& Workbook,
		const FCFContentTypeId& ContentTypeId)
	{
		for (const FCFContentSheetDescriptor& Sheet : Workbook.Sheets)
		{
			if (!Sheet.IsChildSheet() && Sheet.ContentTypeId == ContentTypeId)
			{
				return &Sheet;
			}
		}
		return nullptr;
	}

	// Stable ColumnId descriptor를 sheet에서 찾습니다.
	const FCFContentFieldDescriptor* FindFieldDescriptor(
		const FCFContentSheetDescriptor& Sheet,
		const FString& ColumnId)
	{
		return Sheet.Fields.FindByPredicate([&ColumnId](const FCFContentFieldDescriptor& Field)
		{
			return Field.ColumnId.Equals(ColumnId, ESearchCase::CaseSensitive);
		});
	}

	// Parent primary sheet와 CollectionId에 해당하는 child sheet descriptor를 찾습니다.
	const FCFContentSheetDescriptor* FindChildSheet(
		const FCFContentWorkbookModel& Workbook,
		const FCFContentSheetDescriptor& ParentSheet,
		const FString& CollectionId)
	{
		for (const FCFContentSheetDescriptor& Sheet : Workbook.Sheets)
		{
			if (Sheet.IsChildSheet()
				&& Sheet.ParentSheetId.Equals(ParentSheet.SheetId, ESearchCase::CaseSensitive)
				&& Sheet.CollectionId.Equals(CollectionId, ESearchCase::CaseSensitive))
			{
				return &Sheet;
			}
		}
		return nullptr;
	}

	// Float/Double authored text가 locale-independent ASCII decimal grammar인지 확인합니다.
	bool IsCanonicalDecimalNumberText(const FString& Text)
	{
		if (Text.IsEmpty())
		{
			return false;
		}

		// 현재 parse 위치입니다.
		int32 Index = 0;
		// 입력 전체 길이입니다.
		const int32 Length = Text.Len();

		if (Text[Index] == TEXT('+') || Text[Index] == TEXT('-'))
		{
			++Index;
			if (Index >= Length)
			{
				return false;
			}
		}

		// Mantissa 전체에서 확인한 ASCII 숫자 개수입니다.
		int32 MantissaDigitCount = 0;
		while (Index < Length && Text[Index] >= TEXT('0') && Text[Index] <= TEXT('9'))
		{
			++MantissaDigitCount;
			++Index;
		}

		if (Index < Length && Text[Index] == TEXT('.'))
		{
			++Index;
			while (Index < Length && Text[Index] >= TEXT('0') && Text[Index] <= TEXT('9'))
			{
				++MantissaDigitCount;
				++Index;
			}
		}

		if (MantissaDigitCount == 0)
		{
			return false;
		}

		if (Index < Length && (Text[Index] == TEXT('e') || Text[Index] == TEXT('E')))
		{
			++Index;
			if (Index < Length && (Text[Index] == TEXT('+') || Text[Index] == TEXT('-')))
			{
				++Index;
			}

			// Exponent에서 확인한 ASCII 숫자 개수입니다.
			int32 ExponentDigitCount = 0;
			while (Index < Length && Text[Index] >= TEXT('0') && Text[Index] <= TEXT('9'))
			{
				++ExponentDigitCount;
				++Index;
			}
			if (ExponentDigitCount == 0)
			{
				return false;
			}
		}

		return Index == Length;
	}

	// Strict signed integer parser입니다.
	bool ParseSignedInteger(const FString& Text, int64& OutValue)
	{
		if (Text.IsEmpty())
		{
			return false;
		}

		// Strict parser가 멈춘 위치입니다.
		TCHAR* ParseEnd = nullptr;
		OutValue = FCString::Strtoi64(*Text, &ParseEnd, 10);
		return ParseEnd != nullptr && *ParseEnd == TEXT('\0');
	}

	// Strict unsigned integer parser입니다.
	bool ParseUnsignedInteger(const FString& Text, uint64& OutValue)
	{
		if (Text.IsEmpty() || Text.StartsWith(TEXT("-"), ESearchCase::CaseSensitive))
		{
			return false;
		}

		// Strict parser가 멈춘 위치입니다.
		TCHAR* ParseEnd = nullptr;
		OutValue = FCString::Strtoui64(*Text, &ParseEnd, 10);
		return ParseEnd != nullptr && *ParseEnd == TEXT('\0');
	}

	// ContentReference의 exact ContentTypeId:ContentId representation을 parse합니다.
	bool ParseContentReference(const FString& Text, FCFContentKey& OutKey)
	{
		// ContentTypeId와 ContentId를 나누는 separator 위치입니다.
		int32 SeparatorIndex = INDEX_NONE;
		if (!Text.FindChar(TEXT(':'), SeparatorIndex)
			|| SeparatorIndex <= 0
			|| SeparatorIndex >= Text.Len() - 1
			|| Text.Mid(SeparatorIndex + 1).Contains(TEXT(":"), ESearchCase::CaseSensitive))
		{
			return false;
		}

		OutKey.ContentTypeId.Value = Text.Left(SeparatorIndex);
		OutKey.ContentId = Text.Mid(SeparatorIndex + 1);
		return OutKey.IsValid();
	}

	// Workbook record key가 canonical key set에 존재하는지 확인합니다.
	bool ContainsRecordKey(
		const TSet<FString>& RecordKeys,
		const FCFContentKey& Key)
	{
		return RecordKeys.Contains(Key.ToStableString());
	}

	// Directed dependency graph cycle DFS를 수행합니다.
	bool VisitDependencyNode(
		const FString& Node,
		const TMap<FString, TArray<FString>>& Adjacency,
		TSet<FString>& Visiting,
		TSet<FString>& Visited)
	{
		if (Visiting.Contains(Node))
		{
			return false;
		}
		if (Visited.Contains(Node))
		{
			return true;
		}

		Visiting.Add(Node);
		if (const TArray<FString>* Targets = Adjacency.Find(Node))
		{
			for (const FString& Target : *Targets)
			{
				if (!VisitDependencyNode(Target, Adjacency, Visiting, Visited))
				{
					return false;
				}
			}
		}
		Visiting.Remove(Node);
		Visited.Add(Node);
		return true;
	}
}

bool CFIsStableContentId(const FString& Value)
{
	return CFContentCorePrivate::IsStableId(Value);
}

bool CFIsStableResourceId(const FString& Value)
{
	return CFContentCorePrivate::IsStableId(Value);
}

bool FCFContentTypeId::IsValid() const
{
	return CFIsStableContentId(Value);
}

FString FCFContentTypeId::ToStableString() const
{
	return Value;
}

bool FCFContentKey::IsValid() const
{
	return ContentTypeId.IsValid() && CFIsStableContentId(ContentId);
}

FString FCFContentKey::ToStableString() const
{
	return ContentTypeId.Value + TEXT(":") + ContentId;
}

bool FCFContentCanonicalizer::ParseCell(
	const FString& CellText,
	const FCFContentFieldDescriptor& FieldDescriptor,
	FCFContentValue& OutValue,
	FString& OutError)
{
	OutValue = FCFContentValue();
	OutValue.Type = FieldDescriptor.ValueType;
	OutError.Reset();

	if (CellText.IsEmpty())
	{
		OutValue.State = ECFContentValueState::Inherit;
		return true;
	}

	if (CellText.Equals(TEXT("@none"), ESearchCase::CaseSensitive))
	{
		if (!FieldDescriptor.bAllowNone)
		{
			OutError = TEXT("@none은 이 field schema에서 허용되지 않습니다.");
			return false;
		}
		OutValue.State = ECFContentValueState::None;
		return true;
	}

	if (CellText.Equals(TEXT("@empty"), ESearchCase::CaseSensitive))
	{
		if (FieldDescriptor.ValueType != ECFContentValueType::String
			&& FieldDescriptor.ValueType != ECFContentValueType::Text)
		{
			OutError = TEXT("@empty는 String/FText semantic field에서만 사용할 수 있습니다.");
			return false;
		}

		OutValue.State = ECFContentValueState::Value;
		if (FieldDescriptor.ValueType == ECFContentValueType::Text)
		{
			OutValue.TextValue.Kind = ECFContentTextKind::Literal;
			OutValue.TextValue.SourceString.Reset();
		}
		else
		{
			OutValue.StringValue.Reset();
		}
		return true;
	}

	// Escaped string/text의 actual literal value입니다.
	FString ValueText = CellText;
	if (CellText.StartsWith(TEXT("@@"), ESearchCase::CaseSensitive))
	{
		if (FieldDescriptor.ValueType != ECFContentValueType::String
			&& FieldDescriptor.ValueType != ECFContentValueType::Text)
		{
			OutError = TEXT("@@ escape는 String/FText semantic field에서만 사용할 수 있습니다.");
			return false;
		}
		ValueText = CellText.Mid(1);
	}
	else if ((FieldDescriptor.ValueType == ECFContentValueType::String
		|| FieldDescriptor.ValueType == ECFContentValueType::Text)
		&& CellText.StartsWith(TEXT("@"), ESearchCase::CaseSensitive))
	{
		OutError = TEXT("'@'로 시작하는 literal String/FText는 '@@' escape를 사용해야 합니다.");
		return false;
	}

	OutValue.State = ECFContentValueState::Value;

	switch (FieldDescriptor.ValueType)
	{
	case ECFContentValueType::Boolean:
		if (ValueText.Equals(TEXT("true"), ESearchCase::IgnoreCase))
		{
			OutValue.bBooleanValue = true;
			return true;
		}
		if (ValueText.Equals(TEXT("false"), ESearchCase::IgnoreCase))
		{
			OutValue.bBooleanValue = false;
			return true;
		}
		OutError = TEXT("Boolean은 true 또는 false만 허용합니다.");
		return false;

	case ECFContentValueType::SignedInteger:
		if (!CFContentCorePrivate::ParseSignedInteger(ValueText, OutValue.SignedIntegerValue))
		{
			OutError = TEXT("Signed Integer를 locale-independent decimal로 parse할 수 없습니다.");
			return false;
		}
		return true;

	case ECFContentValueType::UnsignedInteger:
		if (!CFContentCorePrivate::ParseUnsignedInteger(ValueText, OutValue.UnsignedIntegerValue))
		{
			OutError = TEXT("Unsigned Integer를 locale-independent decimal로 parse할 수 없습니다.");
			return false;
		}
		return true;

	case ECFContentValueType::Float:
	{
		if (!CFContentCorePrivate::IsCanonicalDecimalNumberText(ValueText))
		{
			OutError = TEXT("Float는 '.' 소수점과 e/E 지수만 사용하는 locale-independent ASCII decimal이어야 합니다.");
			return false;
		}

		// Workbook authored float parse result입니다.
		float ParsedValue = 0.0f;
		if (!LexTryParseString(ParsedValue, *ValueText) || !FMath::IsFinite(ParsedValue))
		{
			OutError = TEXT("Float는 finite locale-independent numeric value여야 합니다.");
			return false;
		}
		OutValue.FloatingPointValue = static_cast<double>(ParsedValue == 0.0f ? 0.0f : ParsedValue);
		return true;
	}

	case ECFContentValueType::Double:
	{
		if (!CFContentCorePrivate::IsCanonicalDecimalNumberText(ValueText))
		{
			OutError = TEXT("Double은 '.' 소수점과 e/E 지수만 사용하는 locale-independent ASCII decimal이어야 합니다.");
			return false;
		}

		// Workbook authored double parse result입니다.
		double ParsedValue = 0.0;
		if (!LexTryParseString(ParsedValue, *ValueText) || !FMath::IsFinite(ParsedValue))
		{
			OutError = TEXT("Double은 finite locale-independent numeric value여야 합니다.");
			return false;
		}
		OutValue.FloatingPointValue = ParsedValue == 0.0 ? 0.0 : ParsedValue;
		return true;
	}

	case ECFContentValueType::Enum:
		if (!CFIsStableContentId(ValueText))
		{
			OutError = TEXT("Enum은 ordinal이 아닌 canonical stable symbolic token이어야 합니다.");
			return false;
		}
		OutValue.StringValue = ValueText;
		return true;

	case ECFContentValueType::NameId:
	{
		// FName-style ID의 outer whitespace를 제거한 canonical authored token입니다.
		const FString CanonicalNameId = ValueText.TrimStartAndEnd();
		if (!CFIsStableContentId(CanonicalNameId))
		{
			OutError = TEXT("FName-style ID는 outer trim 뒤 canonical stable ID grammar를 따라야 합니다.");
			return false;
		}
		OutValue.StringValue = CanonicalNameId;
		return true;
	}

	case ECFContentValueType::String:
		OutValue.StringValue = ValueText;
		return true;

	case ECFContentValueType::Text:
		OutValue.TextValue.Kind = ECFContentTextKind::Literal;
		OutValue.TextValue.SourceString = ValueText;
		return true;

	case ECFContentValueType::ContentReference:
		if (!CFContentCorePrivate::ParseContentReference(ValueText, OutValue.ContentReference))
		{
			OutError = TEXT("ContentReference는 canonical 'ContentTypeId:ContentId' 형식이어야 합니다.");
			return false;
		}
		return true;

	case ECFContentValueType::ResourceReference:
		if (!CFIsStableResourceId(ValueText))
		{
			OutError = TEXT("ResourceReference는 raw ObjectPath가 아니라 canonical ResourceId여야 합니다.");
			return false;
		}
		OutValue.StringValue = ValueText;
		return true;

	default:
		OutError = TEXT("지원되지 않는 canonical value type입니다.");
		return false;
	}
}

bool FCFContentSchemaValidator::ValidateWorkbook(
	const FCFContentWorkbookModel& Workbook,
	TArray<FCFContentValidationIssue>& OutIssues)
{
	OutIssues.Reset();

	if (!CFIsStableContentId(Workbook.WorkbookSourceId))
	{
		CFContentCorePrivate::AddIssue(
			OutIssues,
			TEXT("InvalidWorkbookSourceId"),
			TEXT("Workbook.WorkbookSourceId"),
			TEXT("WorkbookSourceId는 physical filename과 독립적인 canonical stable ID여야 합니다."));
	}

	if (Workbook.SchemaRevision <= 0)
	{
		CFContentCorePrivate::AddIssue(
			OutIssues,
			TEXT("InvalidWorkbookRevision"),
			TEXT("Workbook.SchemaRevision"),
			TEXT("Workbook SchemaRevision은 1 이상의 positive integer여야 합니다."));
	}

	// Duplicate SheetId 검사용 set입니다.
	TSet<FString> SeenSheetIds;
	// ContentType별 primary sheet exact1 검사용 set입니다.
	TSet<FString> SeenPrimaryContentTypes;

	for (const FCFContentSheetDescriptor& Sheet : Workbook.Sheets)
	{
		if (!CFIsStableContentId(Sheet.SheetId))
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("InvalidSheetId"),
				Sheet.SheetId,
				TEXT("SheetId가 canonical stable ID grammar를 따르지 않습니다."));
			continue;
		}

		if (SeenSheetIds.Contains(Sheet.SheetId))
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("DuplicateSheetId"),
				Sheet.SheetId,
				TEXT("Duplicate SheetId는 허용하지 않습니다."));
		}
		SeenSheetIds.Add(Sheet.SheetId);

		if (Sheet.SchemaRevision <= 0)
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("InvalidSheetRevision"),
				Sheet.SheetId,
				TEXT("Sheet SchemaRevision은 1 이상의 positive integer여야 합니다."));
		}

		if (!Sheet.ContentTypeId.IsValid())
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("InvalidContentTypeId"),
				Sheet.SheetId,
				TEXT("Sheet ContentTypeId가 canonical stable ID가 아닙니다."));
		}

		if (!Sheet.IsChildSheet())
		{
			if (SeenPrimaryContentTypes.Contains(Sheet.ContentTypeId.Value))
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("DuplicatePrimarySheet"),
					Sheet.ContentTypeId.Value,
					TEXT("같은 ContentTypeId에 primary sheet를 둘 이상 등록할 수 없습니다."));
			}
			SeenPrimaryContentTypes.Add(Sheet.ContentTypeId.Value);

			if (!Sheet.CollectionId.IsEmpty())
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("PrimaryCollectionId"),
					Sheet.SheetId,
					TEXT("Primary sheet는 CollectionId를 소유하지 않습니다."));
			}
		}
		else
		{
			if (!CFIsStableContentId(Sheet.ParentSheetId) || !CFIsStableContentId(Sheet.CollectionId))
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("InvalidChildSheetIdentity"),
					Sheet.SheetId,
					TEXT("Child sheet에는 canonical ParentSheetId와 CollectionId가 모두 필요합니다."));
			}
		}

		// Duplicate ColumnId 검사용 set입니다.
		TSet<FString> SeenColumnIds;
		for (const FCFContentFieldDescriptor& Field : Sheet.Fields)
		{
			// Stable field diagnostic path입니다.
			const FString FieldPath = Sheet.SheetId + TEXT(".") + Field.ColumnId;
			if (!CFIsStableContentId(Field.ColumnId))
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("InvalidColumnId"),
					FieldPath,
					TEXT("ColumnId가 canonical stable ID grammar를 따르지 않습니다."));
			}
			if (SeenColumnIds.Contains(Field.ColumnId))
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("DuplicateColumnId"),
					FieldPath,
					TEXT("같은 logical sheet 안의 Duplicate ColumnId는 허용하지 않습니다."));
			}
			SeenColumnIds.Add(Field.ColumnId);

			if (Field.Ownership == ECFContentFieldOwnership::Unspecified)
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("UnclassifiedOwnership"),
					FieldPath,
					TEXT("미분류 field는 CCASManaged로 자동 취급하지 않으며 schema가 fail-closed 됩니다."));
			}
			else if (Field.Ownership == ECFContentFieldOwnership::ExternalManaged)
			{
				if (!CFIsStableContentId(Field.ExternalOwnerId))
				{
					CFContentCorePrivate::AddIssue(
						OutIssues,
						TEXT("MissingExternalOwner"),
						FieldPath,
						TEXT("ExternalManaged field에는 canonical OwnerId가 필요합니다."));
				}
			}
			else if (!Field.ExternalOwnerId.IsEmpty())
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("UnexpectedExternalOwner"),
					FieldPath,
					TEXT("ExternalManaged가 아닌 field는 ExternalOwnerId를 가질 수 없습니다."));
			}

			if (!Field.CanonicalUnitId.IsEmpty() && !CFIsStableContentId(Field.CanonicalUnitId))
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("InvalidCanonicalUnit"),
					FieldPath,
					TEXT("CanonicalUnitId는 stable machine ID여야 합니다."));
			}
			if (!Field.FieldDomainId.IsEmpty() && !CFIsStableContentId(Field.FieldDomainId))
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("InvalidFieldDomain"),
					FieldPath,
					TEXT("FieldDomainId는 stable machine ID여야 합니다."));
			}
			if (Field.ValueType == ECFContentValueType::Text)
			{
				if (Field.TextMode == ECFContentTextMode::NotText)
				{
					CFContentCorePrivate::AddIssue(
						OutIssues,
						TEXT("MissingTextMode"),
						FieldPath,
						TEXT("FText field descriptor는 PreserveIdentity/StableLocalized/LiteralInvariant 중 exact1 mode가 필요합니다."));
				}
			}
			else if (Field.TextMode != ECFContentTextMode::NotText)
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("UnexpectedTextMode"),
					FieldPath,
					TEXT("비-Text field는 FText authoring mode를 가질 수 없습니다."));
			}
		}
	}

	for (const FCFContentSheetDescriptor& Sheet : Workbook.Sheets)
	{
		if (Sheet.IsChildSheet() && !SeenSheetIds.Contains(Sheet.ParentSheetId))
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("MissingParentSheet"),
				Sheet.SheetId,
				TEXT("Child sheet의 ParentSheetId가 등록된 logical sheet를 가리키지 않습니다."));
		}
	}

	// Duplicate canonical ContentKey 검사용 set입니다.
	TSet<FString> SeenContentKeys;
	// Duplicate stable RowId 검사용 set입니다.
	TSet<FString> SeenRowIds;

	for (const FCFContentRecord& Record : Workbook.Records)
	{
		// Record canonical diagnostic key입니다.
		const FString RecordKeyText = Record.Key.ToStableString();
		if (!Record.Key.IsValid())
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("InvalidContentKey"),
				RecordKeyText,
				TEXT("Content record의 FCFContentKey가 invalid입니다."));
			continue;
		}
		if (SeenContentKeys.Contains(RecordKeyText))
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("DuplicateContentKey"),
				RecordKeyText,
				TEXT("Duplicate ContentTypeId + ContentId logical identity는 허용하지 않습니다."));
		}
		SeenContentKeys.Add(RecordKeyText);

		if (!Record.RowId.Equals(RecordKeyText, ESearchCase::CaseSensitive))
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("InvalidRowId"),
				RecordKeyText,
				TEXT("Top-level RowId는 별도 경쟁 identity가 아니라 canonical FCFContentKey token과 exact 일치해야 합니다."));
		}
		else if (SeenRowIds.Contains(Record.RowId))
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("DuplicateRowId"),
				Record.RowId,
				TEXT("Workbook 전체에서 Duplicate RowId를 허용하지 않습니다."));
		}
		SeenRowIds.Add(Record.RowId);

		if (!Record.AuthoringMetadata.FamilyId.IsEmpty()
			&& !CFIsStableContentId(Record.AuthoringMetadata.FamilyId))
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("InvalidFamilyId"),
				RecordKeyText,
				TEXT("FamilyId는 canonical stable ID여야 합니다."));
		}
		if (!Record.AuthoringMetadata.BaseContentId.IsEmpty()
			&& !CFIsStableContentId(Record.AuthoringMetadata.BaseContentId))
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("InvalidBaseContentId"),
				RecordKeyText,
				TEXT("BaseContentId는 비교/디자인 기준용 canonical stable ID여야 합니다."));
		}

		switch (Record.LifecycleState)
		{
		case ECFContentLifecycleState::Active:
		case ECFContentLifecycleState::Deprecated:
		case ECFContentLifecycleState::Retired:
			break;
		default:
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("InvalidLifecycleState"),
				RecordKeyText,
				TEXT("Content lifecycle은 Active/Deprecated/Retired exact3 중 하나여야 합니다."));
			break;
		}

		// P0-06 planning metadata exact contract입니다.
		const FCFContentPlanningMetadata& Planning = Record.AuthoringMetadata.Planning;
		if (!Planning.VariantId.IsEmpty() && !CFIsStableContentId(Planning.VariantId))
		{
			CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidVariantId"), RecordKeyText, TEXT("VariantId는 비어 있거나 canonical stable ID여야 합니다."));
		}
		if (!Planning.RoleId.IsEmpty() && !CFIsStableContentId(Planning.RoleId))
		{
			CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidPlanningRoleId"), RecordKeyText, TEXT("RoleId는 비어 있거나 canonical stable ID여야 합니다."));
		}
		if (!Planning.ProductionWaveId.IsEmpty() && !CFIsStableContentId(Planning.ProductionWaveId))
		{
			CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidProductionWaveId"), RecordKeyText, TEXT("ProductionWaveId는 비어 있거나 canonical stable ID여야 합니다."));
		}

		switch (Planning.Readiness)
		{
		case ECFContentPlanningReadiness::Planned:
		case ECFContentPlanningReadiness::AuthoringReady:
		case ECFContentPlanningReadiness::Conditional:
		case ECFContentPlanningReadiness::Blocked:
			break;
		default:
			CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidPlanningReadiness"), RecordKeyText, TEXT("Planning Readiness가 frozen exact4 contract 밖의 값입니다."));
			break;
		}

		// RelativeIntent duplicate DimensionId 검사용 set입니다.
		TSet<FString> SeenRelativeDimensions;
		for (const FCFContentRelativeIntent& RelativeIntent : Planning.RelativeIntents)
		{
			if (!CFIsStableContentId(RelativeIntent.DimensionId)
				|| SeenRelativeDimensions.Contains(RelativeIntent.DimensionId))
			{
				CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidRelativeIntent"), RecordKeyText, TEXT("RelativeIntent DimensionId는 canonical하고 중복이 없어야 합니다."));
			}
			SeenRelativeDimensions.Add(RelativeIntent.DimensionId);
			switch (RelativeIntent.Direction)
			{
			case ECFContentRelativeDirection::Decrease:
			case ECFContentRelativeDirection::Equal:
			case ECFContentRelativeDirection::Increase:
				break;
			default:
				CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidRelativeDirection"), RecordKeyText, TEXT("RelativeIntent Direction이 frozen exact3 contract 밖의 값입니다."));
				break;
			}
		}

		// TechnologyDependency duplicate identity 검사용 set입니다.
		TSet<FString> SeenDependencyIds;
		for (const FCFContentTechnologyDependency& Dependency : Planning.TechnologyDependencies)
		{
			if (!CFIsStableContentId(Dependency.DependencyId)
				|| SeenDependencyIds.Contains(Dependency.DependencyId))
			{
				CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidTechnologyDependency"), RecordKeyText, TEXT("TechnologyDependency ID는 canonical하고 중복이 없어야 합니다."));
			}
			SeenDependencyIds.Add(Dependency.DependencyId);
			switch (Dependency.State)
			{
			case ECFContentDependencyState::Satisfied:
			case ECFContentDependencyState::Pending:
			case ECFContentDependencyState::Blocked:
				break;
			default:
				CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidDependencyState"), RecordKeyText, TEXT("TechnologyDependency State가 frozen exact3 contract 밖의 값입니다."));
				break;
			}
		}

		// Shared와 Variant-owned DataRole의 duplicate/overlap을 검증합니다.
		TSet<FString> SharedRoleIds;
		for (const FString& DataRoleId : Planning.SharedDataRoleIds)
		{
			if (!CFIsStableContentId(DataRoleId) || SharedRoleIds.Contains(DataRoleId))
			{
				CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidSharedDataRole"), RecordKeyText, TEXT("SharedDataRoleId는 canonical하고 중복이 없어야 합니다."));
			}
			SharedRoleIds.Add(DataRoleId);
		}
		TSet<FString> VariantOwnedRoleIds;
		for (const FString& DataRoleId : Planning.VariantOwnedDataRoleIds)
		{
			if (!CFIsStableContentId(DataRoleId) || VariantOwnedRoleIds.Contains(DataRoleId))
			{
				CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidVariantOwnedDataRole"), RecordKeyText, TEXT("VariantOwnedDataRoleId는 canonical하고 중복이 없어야 합니다."));
			}
			if (SharedRoleIds.Contains(DataRoleId))
			{
				CFContentCorePrivate::AddIssue(OutIssues, TEXT("OverlappingDataRolePolicy"), RecordKeyText, TEXT("같은 DataRoleId를 Shared와 Variant-owned에 동시에 둘 수 없습니다."));
			}
			VariantOwnedRoleIds.Add(DataRoleId);
		}

		for (const TPair<FString, FString>& ProfilePair : Record.AuthoringMetadata.AssignedProfileIdsByDomain)
		{
			if (!CFIsStableContentId(ProfilePair.Key) || !CFIsStableContentId(ProfilePair.Value))
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("InvalidAssignedProfile"),
					RecordKeyText,
					TEXT("Assigned Profile mapping의 field-domain과 ProfileId는 모두 canonical stable ID여야 합니다."));
			}
		}

		// Record ContentType의 primary logical sheet입니다.
		const FCFContentSheetDescriptor* PrimarySheet =
			CFContentCorePrivate::FindPrimarySheet(Workbook, Record.Key.ContentTypeId);
		if (PrimarySheet == nullptr)
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("MissingPrimarySheet"),
				RecordKeyText,
				TEXT("Content record의 ContentTypeId를 소유하는 primary sheet schema가 없습니다."));
			continue;
		}

		for (const TPair<FString, FCFContentValue>& FieldPair : Record.Fields)
		{
			// Scalar field descriptor입니다.
			const FCFContentFieldDescriptor* Descriptor =
				CFContentCorePrivate::FindFieldDescriptor(*PrimarySheet, FieldPair.Key);
			// Record scalar stable path입니다.
			const FString FieldPath = RecordKeyText + TEXT(".") + FieldPair.Key;
			if (Descriptor == nullptr)
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("UnknownColumnId"),
					FieldPath,
					TEXT("Record field가 primary sheet의 stable ColumnId registry에 없습니다."));
				continue;
			}
			CFContentCorePrivate::ValidateValueAgainstDescriptor(
				FieldPair.Value,
				*Descriptor,
				FieldPath,
				OutIssues);
		}

		for (const FCFContentFieldDescriptor& Descriptor : PrimarySheet->Fields)
		{
			if (Descriptor.bRequired && !Record.Fields.Contains(Descriptor.ColumnId))
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("MissingRequiredField"),
					RecordKeyText + TEXT(".") + Descriptor.ColumnId,
					TEXT("Required field의 canonical value/state가 record에 없습니다."));
			}
		}

		for (const TPair<FString, FCFContentCollection>& CollectionPair : Record.Collections)
		{
			// Stable collection map key입니다.
			const FString& CollectionId = CollectionPair.Key;
			// Canonical collection value입니다.
			const FCFContentCollection& Collection = CollectionPair.Value;
			// Collection diagnostic path입니다.
			const FString CollectionPath = RecordKeyText + TEXT(".") + CollectionId;

			if (!Collection.CollectionId.Equals(CollectionId, ESearchCase::CaseSensitive)
				|| !CFIsStableContentId(CollectionId))
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("InvalidCollectionId"),
					CollectionPath,
					TEXT("Collection map key와 embedded CollectionId는 동일한 canonical stable ID여야 합니다."));
				continue;
			}

			// Collection을 정의하는 child sheet schema입니다.
			const FCFContentSheetDescriptor* ChildSheet =
				CFContentCorePrivate::FindChildSheet(Workbook, *PrimarySheet, CollectionId);
			if (ChildSheet == nullptr)
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("MissingChildSheet"),
					CollectionPath,
					TEXT("CollectionId에 대응하는 child sheet schema가 없습니다."));
				continue;
			}
			if (ChildSheet->CollectionKind != Collection.Kind)
			{
				CFContentCorePrivate::AddIssue(
					OutIssues,
					TEXT("CollectionKindMismatch"),
					CollectionPath,
					TEXT("Collection kind가 child sheet descriptor와 다릅니다."));
			}

			// Duplicate ChildItemId 검사용 set입니다.
			TSet<FString> SeenChildItemIds;
			// OrderedList duplicate Order 검사용 set입니다.
			TSet<int32> SeenOrders;
			// KeyedCollection case-insensitive duplicate domain key 검사용 set입니다.
			TSet<FString> SeenDomainKeys;
			// UnorderedSet canonical typed-value duplicate 검사용 set입니다.
			TSet<FString> SeenSetSemanticKeys;

			for (const FCFContentCollectionItem& Item : Collection.Items)
			{
				// Child diagnostic path입니다.
				const FString ItemPath = CollectionPath + TEXT("[") + Item.ChildItemId + TEXT("]");
				if (!CFIsStableContentId(Item.ChildItemId))
				{
					CFContentCorePrivate::AddIssue(
						OutIssues,
						TEXT("InvalidChildItemId"),
						ItemPath,
						TEXT("ChildItemId가 canonical stable ID가 아닙니다."));
				}
				if (SeenChildItemIds.Contains(Item.ChildItemId))
				{
					CFContentCorePrivate::AddIssue(
						OutIssues,
						TEXT("DuplicateChildItemId"),
						ItemPath,
						TEXT("같은 collection 안의 Duplicate ChildItemId는 허용하지 않습니다."));
				}
				SeenChildItemIds.Add(Item.ChildItemId);

				switch (Collection.Kind)
				{
				case ECFContentCollectionKind::OrderedList:
					if (Item.Order < 0 || SeenOrders.Contains(Item.Order))
					{
						CFContentCorePrivate::AddIssue(
							OutIssues,
							TEXT("InvalidCollectionOrder"),
							ItemPath,
							TEXT("OrderedList item은 중복 없는 explicit non-negative Order를 가져야 합니다."));
					}
					SeenOrders.Add(Item.Order);
					if (!Item.DomainKey.IsEmpty())
					{
						CFContentCorePrivate::AddIssue(
							OutIssues,
							TEXT("UnexpectedDomainKey"),
							ItemPath,
							TEXT("OrderedList item은 DomainKey를 semantic identity로 사용하지 않습니다."));
					}
					break;
				case ECFContentCollectionKind::KeyedCollection:
				{
					// FName-style/domain ID duplicate 비교용 lowercase canonical key입니다.
					const FString CanonicalDomainKey = Item.DomainKey.ToLower();
					if (!CFIsStableContentId(Item.DomainKey) || SeenDomainKeys.Contains(CanonicalDomainKey))
					{
						CFContentCorePrivate::AddIssue(
							OutIssues,
							TEXT("InvalidDomainKey"),
							ItemPath,
							TEXT("KeyedCollection item은 case-insensitive 기준으로 중복 없는 canonical DomainKey를 가져야 합니다."));
					}
					SeenDomainKeys.Add(CanonicalDomainKey);
					if (Item.Order != INDEX_NONE)
					{
						CFContentCorePrivate::AddIssue(
							OutIssues,
							TEXT("UnexpectedOrder"),
							ItemPath,
							TEXT("KeyedCollection item은 physical Order를 semantic authority로 사용하지 않습니다."));
					}
					break;
				}
				case ECFContentCollectionKind::UnorderedSet:
				{
					if (!Item.DomainKey.IsEmpty() || Item.Order != INDEX_NONE)
					{
						CFContentCorePrivate::AddIssue(
							OutIssues,
							TEXT("UnexpectedSetOrdering"),
							ItemPath,
							TEXT("UnorderedSet item은 DomainKey/Order를 semantic authority로 사용하지 않습니다."));
					}
					// UnorderedSet semantic identity는 hidden ChildItemId가 아니라 canonical typed value입니다.
					const FString SetSemanticKey = CFContentCorePrivate::BuildFieldMapSemanticKey(Item.Fields);
					if (SetSemanticKey.IsEmpty() || SeenSetSemanticKeys.Contains(SetSemanticKey))
					{
						CFContentCorePrivate::AddIssue(
							OutIssues,
							TEXT("DuplicateSetValue"),
							ItemPath,
							TEXT("UnorderedSet은 canonical typed value 기준 중복을 허용하지 않습니다."));
					}
					SeenSetSemanticKeys.Add(SetSemanticKey);
					break;
				}
				default:
					break;
				}

				for (const TPair<FString, FCFContentValue>& FieldPair : Item.Fields)
				{
					// Child field descriptor입니다.
					const FCFContentFieldDescriptor* Descriptor =
						CFContentCorePrivate::FindFieldDescriptor(*ChildSheet, FieldPair.Key);
					// Child scalar stable path입니다.
					const FString FieldPath = ItemPath + TEXT(".") + FieldPair.Key;
					if (Descriptor == nullptr)
					{
						CFContentCorePrivate::AddIssue(
							OutIssues,
							TEXT("UnknownChildColumnId"),
							FieldPath,
							TEXT("Child item field가 child sheet stable ColumnId registry에 없습니다."));
						continue;
					}
					CFContentCorePrivate::ValidateValueAgainstDescriptor(
						FieldPair.Value,
						*Descriptor,
						FieldPath,
						OutIssues);
				}
			}
		}
	}

	return OutIssues.Num() == 0;
}

bool FCFContentSchemaValidator::ValidateChangeSet(
	const FCFContentChangeSet& ChangeSet,
	TArray<FCFContentValidationIssue>& OutIssues)
{
	OutIssues.Reset();

	if (!ChangeSet.SchemaId.Equals(TEXT("cfcontent-change/v1"), ESearchCase::CaseSensitive))
	{
		CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidChangeSetSchema"), TEXT("ChangeSet.SchemaId"), TEXT("P0 Change Set schema는 cfcontent-change/v1 exact contract여야 합니다."));
	}
	if (!CFIsStableContentId(ChangeSet.ChangeSetId))
	{
		CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidChangeSetId"), TEXT("ChangeSet.ChangeSetId"), TEXT("ChangeSetId는 canonical stable ID여야 합니다."));
	}
	if (!CFDACommonPrimitives::IsCanonicalSha256Fingerprint(ChangeSet.BaseWorkbookSemanticHash))
	{
		CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidBaseSemanticHash"), TEXT("ChangeSet.BaseWorkbookSemanticHash"), TEXT("BaseWorkbookSemanticHash는 authoritative canonical SHA-256 fingerprint여야 합니다."));
	}
	if (!ChangeSet.BaseCatalogSnapshotFingerprint.IsEmpty()
		&& !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(ChangeSet.BaseCatalogSnapshotFingerprint))
	{
		CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidBaseCatalogSnapshotFingerprint"), TEXT("ChangeSet.BaseCatalogSnapshotFingerprint"), TEXT("BaseCatalogSnapshotFingerprint는 비어 있거나 canonical SHA-256 fingerprint여야 합니다."));
	}
	if (ChangeSet.WorkbookSchemaVersion <= 0)
	{
		CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidChangeSetRevision"), TEXT("ChangeSet.WorkbookSchemaVersion"), TEXT("WorkbookSchemaVersion은 positive integer여야 합니다."));
	}
	if (!ChangeSet.ExpectedPostSemanticHash.IsEmpty()
		&& !CFDACommonPrimitives::IsCanonicalSha256Fingerprint(ChangeSet.ExpectedPostSemanticHash))
	{
		CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidPostSemanticHash"), TEXT("ChangeSet.ExpectedPostSemanticHash"), TEXT("ExpectedPostSemanticHash는 비어 있거나 canonical SHA-256 fingerprint여야 합니다."));
	}
	if (ChangeSet.Operations.IsEmpty())
	{
		CFContentCorePrivate::AddIssue(OutIssues, TEXT("EmptyChangeSet"), TEXT("ChangeSet.Operations"), TEXT("Reviewable Change Set은 최소 하나의 typed operation을 가져야 합니다."));
	}

	for (int32 OperationIndex = 0; OperationIndex < ChangeSet.Operations.Num(); ++OperationIndex)
	{
		// Operation diagnostic path입니다.
		const FString OperationPath = FString::Printf(TEXT("ChangeSet.Operations[%d]"), OperationIndex);
		// Validate할 exact typed operation입니다.
		const FCFContentChangeOperation& Operation = ChangeSet.Operations[OperationIndex];

		if (!CFIsStableContentId(Operation.SheetId))
		{
			CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidChangeSheetId"), OperationPath, TEXT("Change Set operation은 stable SheetId로 target을 식별해야 합니다."));
		}
		if (!Operation.ContentKey.IsValid())
		{
			CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidChangeContentKey"), OperationPath, TEXT("Change Set operation에는 valid FCFContentKey가 필요합니다."));
		}
		else if (!Operation.RowId.Equals(Operation.ContentKey.ToStableString(), ESearchCase::CaseSensitive))
		{
			CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidChangeRowId"), OperationPath, TEXT("Top-level RowId는 operation ContentKey token과 exact 일치해야 합니다."));
		}

		switch (Operation.OperationType)
		{
		case ECFContentChangeOperationType::UpdateField:
		case ECFContentChangeOperationType::SetNone:
		case ECFContentChangeOperationType::SetInherit:
			if (!CFIsStableContentId(Operation.ColumnId))
			{
				CFContentCorePrivate::AddIssue(OutIssues, TEXT("MissingChangeColumnId"), OperationPath, TEXT("Field operation에는 stable ColumnId가 필요합니다."));
			}
			break;
		case ECFContentChangeOperationType::UpsertChild:
		case ECFContentChangeOperationType::RemoveChildIntent:
			if (!CFIsStableContentId(Operation.ChildItemId))
			{
				CFContentCorePrivate::AddIssue(OutIssues, TEXT("MissingChangeChildItemId"), OperationPath, TEXT("Child operation에는 stable ChildItemId가 필요합니다."));
			}
			break;
		case ECFContentChangeOperationType::AssignProfile:
			if (!CFIsStableContentId(Operation.ProfileId))
			{
				CFContentCorePrivate::AddIssue(OutIssues, TEXT("MissingChangeProfileId"), OperationPath, TEXT("AssignProfile operation에는 stable ProfileId가 필요합니다."));
			}
			break;
		case ECFContentChangeOperationType::SetPlanningMetadata:
			break;
		case ECFContentChangeOperationType::BindResource:
			if (!CFIsStableResourceId(Operation.ResourceId))
			{
				CFContentCorePrivate::AddIssue(OutIssues, TEXT("MissingChangeResourceId"), OperationPath, TEXT("BindResource operation에는 raw ObjectPath가 아닌 stable ResourceId가 필요합니다."));
			}
			break;
		case ECFContentChangeOperationType::AddRecord:
		case ECFContentChangeOperationType::RetireRecord:
		case ECFContentChangeOperationType::ReactivateRecord:
			break;
		default:
			CFContentCorePrivate::AddIssue(OutIssues, TEXT("InvalidChangeOperation"), OperationPath, TEXT("지원되지 않는 Change Set operation입니다."));
			break;
		}
	}

	return OutIssues.Num() == 0;
}

bool FCFContentSchemaValidator::ValidateDependencyEdges(
	const FCFContentWorkbookModel& Workbook,
	const TArray<FCFContentDependencyEdge>& Edges,
	TArray<FCFContentValidationIssue>& OutIssues)
{
	OutIssues.Reset();

	// Existing canonical record key set입니다.
	TSet<FString> RecordKeys;
	for (const FCFContentRecord& Record : Workbook.Records)
	{
		if (Record.Key.IsValid())
		{
			RecordKeys.Add(Record.Key.ToStableString());
		}
	}

	// Cycle detection용 adjacency map입니다.
	TMap<FString, TArray<FString>> Adjacency;
	for (const FCFContentDependencyEdge& Edge : Edges)
	{
		// Source key string입니다.
		const FString FromKey = Edge.From.ToStableString();
		// Target key string입니다.
		const FString ToKey = Edge.To.ToStableString();
		if (!Edge.From.IsValid() || !CFContentCorePrivate::ContainsRecordKey(RecordKeys, Edge.From))
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("MissingDependencySource"),
				FromKey,
				TEXT("Dependency source ContentKey가 canonical workbook에 없습니다."));
			continue;
		}
		if (!Edge.To.IsValid() || !CFContentCorePrivate::ContainsRecordKey(RecordKeys, Edge.To))
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("MissingDependencyTarget"),
				ToKey,
				TEXT("Dependency target ContentKey가 canonical workbook에 없습니다."));
			continue;
		}
		Adjacency.FindOrAdd(FromKey).Add(ToKey);
	}

	if (OutIssues.Num() > 0)
	{
		return false;
	}

	// DFS active stack set입니다.
	TSet<FString> Visiting;
	// DFS completed set입니다.
	TSet<FString> Visited;
	for (const FString& RecordKey : RecordKeys)
	{
		if (!CFContentCorePrivate::VisitDependencyNode(RecordKey, Adjacency, Visiting, Visited))
		{
			CFContentCorePrivate::AddIssue(
				OutIssues,
				TEXT("DependencyCycle"),
				RecordKey,
				TEXT("Content dependency graph에 directed cycle이 존재합니다."));
			return false;
		}
	}
	return true;
}

bool FCFContentSchemaValidator::CanMutateRecord(const FCFContentRecord& Record)
{
	return Record.ManagementState == ECFContentManagementState::Managed;
}

bool FCFContentSemanticHasher::BuildWorkbookSemanticHash(
	const FCFContentWorkbookModel& Workbook,
	FString& OutSemanticHash,
	FString& OutError)
{
	// Hash 전 fail-closed schema/content diagnostics입니다.
	TArray<FCFContentValidationIssue> ValidationIssues;
	if (!FCFContentSchemaValidator::ValidateWorkbook(Workbook, ValidationIssues))
	{
		OutSemanticHash.Reset();
		OutError = ValidationIssues.Num() > 0
			? FString::Printf(TEXT("Workbook semantic hash 전에 validation 실패: %s (%s)"), *ValidationIssues[0].Code, *ValidationIssues[0].Path)
			: TEXT("Workbook semantic hash 전에 validation 실패.");
		return false;
	}

	// Canonical semantic byte stream입니다.
	TArray<uint8> CanonicalBytes;
	CFContentCorePrivate::AppendString(CanonicalBytes, TEXT("CarFight.CCAS.WorkbookSemantic/v2"));
	CFContentCorePrivate::AppendString(CanonicalBytes, Workbook.WorkbookSourceId);
	CFContentCorePrivate::AppendInt32(CanonicalBytes, Workbook.SchemaRevision);

	// Sheet physical position을 제거한 canonical SheetId order입니다.
	TArray<const FCFContentSheetDescriptor*> SortedSheets;
	for (const FCFContentSheetDescriptor& Sheet : Workbook.Sheets)
	{
		SortedSheets.Add(&Sheet);
	}
	SortedSheets.Sort([](const FCFContentSheetDescriptor& Left, const FCFContentSheetDescriptor& Right)
	{
		return Left.SheetId < Right.SheetId;
	});

	CFContentCorePrivate::AppendUInt32(CanonicalBytes, static_cast<uint32>(SortedSheets.Num()));
	for (const FCFContentSheetDescriptor* Sheet : SortedSheets)
	{
		if (Sheet != nullptr)
		{
			CFContentCorePrivate::AppendSheetDescriptor(CanonicalBytes, *Sheet);
		}
	}

	// Row physical order를 제거한 canonical ContentKey order입니다.
	TArray<const FCFContentRecord*> SortedRecords;
	for (const FCFContentRecord& Record : Workbook.Records)
	{
		SortedRecords.Add(&Record);
	}
	SortedRecords.Sort([](const FCFContentRecord& Left, const FCFContentRecord& Right)
	{
		return Left.Key.ToStableString() < Right.Key.ToStableString();
	});

	CFContentCorePrivate::AppendUInt32(CanonicalBytes, static_cast<uint32>(SortedRecords.Num()));
	for (const FCFContentRecord* Record : SortedRecords)
	{
		if (Record != nullptr)
		{
			CFContentCorePrivate::AppendRecord(CanonicalBytes, *Record);
		}
	}

	return CFDACommonPrimitives::HashCanonicalBytes(
		CanonicalBytes,
		OutSemanticHash,
		OutError);
}

bool FCFContentSchemaMigration::BuildPreview(
	const FCFContentWorkbookModel& SourceWorkbook,
	const FCFContentMigrationRule& Rule,
	FCFContentMigrationPreview& OutPreview,
	FString& OutError)
{
	OutPreview = FCFContentMigrationPreview();
	OutError.Reset();

	if (Rule.FromRevision <= 0
		|| Rule.ToRevision <= Rule.FromRevision
		|| SourceWorkbook.SchemaRevision != Rule.FromRevision)
	{
		OutError = TEXT("Migration revision 계약이 source workbook과 일치하지 않습니다.");
		return false;
	}

	// Source schema/content validation diagnostics입니다.
	TArray<FCFContentValidationIssue> SourceIssues;
	if (!FCFContentSchemaValidator::ValidateWorkbook(SourceWorkbook, SourceIssues))
	{
		OutError = TEXT("Migration source workbook이 canonical validation을 통과하지 못했습니다.");
		return false;
	}

	if (!FCFContentSemanticHasher::BuildWorkbookSemanticHash(
		SourceWorkbook,
		OutPreview.SourceSemanticHash,
		OutError))
	{
		return false;
	}

	OutPreview.MigratedModel = SourceWorkbook;
	OutPreview.MigratedModel.SchemaRevision = Rule.ToRevision;

	// Deterministic migration 적용 순서입니다.
	TArray<FString> OldColumnIds;
	Rule.ColumnRenames.GetKeys(OldColumnIds);
	OldColumnIds.Sort();

	for (const FString& OldColumnId : OldColumnIds)
	{
		// Old ColumnId가 mapping되는 new stable ColumnId입니다.
		const FString* NewColumnIdPtr = Rule.ColumnRenames.Find(OldColumnId);
		if (NewColumnIdPtr == nullptr)
		{
			continue;
		}
		// Validated new stable ColumnId입니다.
		const FString& NewColumnId = *NewColumnIdPtr;
		if (!CFIsStableContentId(OldColumnId)
			|| !CFIsStableContentId(NewColumnId)
			|| OldColumnId.Equals(NewColumnId, ESearchCase::CaseSensitive))
		{
			OutError = FString::Printf(TEXT("Invalid migration ColumnId mapping: %s -> %s"), *OldColumnId, *NewColumnId);
			return false;
		}

		for (FCFContentSheetDescriptor& Sheet : OutPreview.MigratedModel.Sheets)
		{
			// Existing new ColumnId collision 여부입니다.
			const bool bNewColumnExists = Sheet.Fields.ContainsByPredicate([&NewColumnId](const FCFContentFieldDescriptor& Field)
			{
				return Field.ColumnId.Equals(NewColumnId, ESearchCase::CaseSensitive);
			});
			// Rename할 old descriptor pointer입니다.
			FCFContentFieldDescriptor* OldDescriptor = Sheet.Fields.FindByPredicate([&OldColumnId](const FCFContentFieldDescriptor& Field)
			{
				return Field.ColumnId.Equals(OldColumnId, ESearchCase::CaseSensitive);
			});

			if (OldDescriptor != nullptr)
			{
				if (bNewColumnExists)
				{
					OutError = FString::Printf(TEXT("Migration descriptor collision: %s -> %s"), *OldColumnId, *NewColumnId);
					return false;
				}
				OldDescriptor->ColumnId = NewColumnId;
			}
		}

		for (FCFContentRecord& Record : OutPreview.MigratedModel.Records)
		{
			if (FCFContentValue* OldValue = Record.Fields.Find(OldColumnId))
			{
				if (Record.Fields.Contains(NewColumnId))
				{
					OutError = FString::Printf(TEXT("Migration record field collision: %s -> %s"), *OldColumnId, *NewColumnId);
					return false;
				}
				// Renamed scalar value copy입니다.
				const FCFContentValue ValueCopy = *OldValue;
				Record.Fields.Remove(OldColumnId);
				Record.Fields.Add(NewColumnId, ValueCopy);
			}

			for (TPair<FString, FCFContentCollection>& CollectionPair : Record.Collections)
			{
				for (FCFContentCollectionItem& Item : CollectionPair.Value.Items)
				{
					if (FCFContentValue* OldValue = Item.Fields.Find(OldColumnId))
					{
						if (Item.Fields.Contains(NewColumnId))
						{
							OutError = FString::Printf(TEXT("Migration child field collision: %s -> %s"), *OldColumnId, *NewColumnId);
							return false;
						}
						// Renamed child value copy입니다.
						const FCFContentValue ValueCopy = *OldValue;
						Item.Fields.Remove(OldColumnId);
						Item.Fields.Add(NewColumnId, ValueCopy);
					}
				}
			}
		}

		OutPreview.Summary.Add(FString::Printf(TEXT("ColumnId %s -> %s"), *OldColumnId, *NewColumnId));
	}

	// Migrated target validation diagnostics입니다.
	TArray<FCFContentValidationIssue> TargetIssues;
	if (!FCFContentSchemaValidator::ValidateWorkbook(OutPreview.MigratedModel, TargetIssues))
	{
		OutError = TargetIssues.Num() > 0
			? FString::Printf(TEXT("Migrated workbook validation 실패: %s (%s)"), *TargetIssues[0].Code, *TargetIssues[0].Path)
			: TEXT("Migrated workbook validation 실패.");
		return false;
	}

	if (!FCFContentSemanticHasher::BuildWorkbookSemanticHash(
		OutPreview.MigratedModel,
		OutPreview.TargetSemanticHash,
		OutError))
	{
		return false;
	}

	return true;
}
