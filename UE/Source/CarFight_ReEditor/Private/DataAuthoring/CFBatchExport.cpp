// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchExport.cpp
// Version: v1.1.0
// Date: 2026-08-18
// Description: DAUTH-P0-08J~P0-11 canonical UTF-8 CSV + immutable .cfbatch.json export baseline 구현입니다.
// Scope: Recipe/Profile projection, deterministic export와 immutable baseline 기준 reviewed one-cell CSV reconstruction을 제공합니다.
// Changelog:
// - v1.1.0: normal Shared Profile Editor가 existing B2 pipeline을 재사용하도록 exact one-cell edited CSV builder 추가.
// - v1.0.1: UE 5.8 FNumericProperty에 없는 IsUnsigned 의존을 제거하고 concrete unsigned property class 판정으로 교정.
// - v1.0.0: Section 26.26~26.30 export foundation 최초 구현.
// Migration:
// - Disk file write, CSV import, formula evaluation, 3-way merge, Recipe/Profile commit, Definition Apply를 수행하지 않습니다.

#include "DataAuthoring/CFBatchExport.h"

#include "CFVehicleData.h"
#include "Containers/StringConv.h"
#include "DataAuthoring/CFBatchColumnRegistry.h"
#include "DataAuthoring/CFDriveStateProfile.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Misc/SecureHash.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonWriter.h"
#include "UObject/UnrealType.h"

namespace CFBatchExportPrivate
{
	/** Reflection property와 actual value address를 함께 반환하는 read-only view입니다. */
	struct FReflectedValueView
	{
		// Resolved leaf property입니다.
		const FProperty* Property = nullptr;

		// Property의 actual value storage address입니다.
		const void* ValueAddress = nullptr;
	};

	// UTF-8 canonical payload를 lowercase MD5 hexadecimal digest로 변환합니다.
	FString HashUtf8Payload(const FString& Payload)
	{
		// TCHAR payload의 canonical UTF-8 view입니다.
		const FTCHARToUTF8 Utf8Payload(*Payload);
		// Deterministic MD5 state입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());
		// 128-bit MD5 digest bytes입니다.
		uint8 Digest[16];
		Md5.Final(Digest);

		// Lowercase hexadecimal digest입니다.
		FString Result;
		Result.Reserve(32);
		// Stable lowercase hexadecimal table입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			Result.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			Result.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return Result;
	}

	// Delimiter collision 없이 canonical hash token을 추가합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("\n");
	}

	// Dataset Kind를 stable technical token으로 변환합니다.
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

	// Profile Domain을 stable technical token으로 변환합니다.
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

	// Value type을 manifest용 stable technical token으로 변환합니다.
	FString ValueTypeToString(const ECFBatchValueType ValueType)
	{
		switch (ValueType)
		{
		case ECFBatchValueType::String:
			return TEXT("String");
		case ECFBatchValueType::Integer:
			return TEXT("Integer");
		case ECFBatchValueType::Number:
			return TEXT("Number");
		case ECFBatchValueType::Boolean:
			return TEXT("Boolean");
		case ECFBatchValueType::CanonicalText:
			return TEXT("CanonicalText");
		default:
			return TEXT("Unknown");
		}
	}

	// Column access를 manifest용 stable technical token으로 변환합니다.
	FString AccessToString(const ECFBatchColumnAccess Access)
	{
		return Access == ECFBatchColumnAccess::Editable ? TEXT("Editable") : TEXT("ReadOnly");
	}

	// Authoring owner를 manifest용 stable technical token으로 변환합니다.
	FString OwnerToString(const ECFBatchAuthoringOwner Owner)
	{
		switch (Owner)
		{
		case ECFBatchAuthoringOwner::Metadata:
			return TEXT("Metadata");
		case ECFBatchAuthoringOwner::ResolvedFieldRegistry:
			return TEXT("ResolvedFieldRegistry");
		case ECFBatchAuthoringOwner::Recipe:
			return TEXT("Recipe");
		case ECFBatchAuthoringOwner::Profile:
			return TEXT("Profile");
		default:
			return TEXT("Unknown");
		}
	}

	// Typed mutation kind를 manifest용 stable technical token으로 변환합니다.
	FString MutationToString(const ECFBatchMutationKind MutationKind)
	{
		switch (MutationKind)
		{
		case ECFBatchMutationKind::None:
			return TEXT("None");
		case ECFBatchMutationKind::RecipeDrivingFeelAxis:
			return TEXT("RecipeDrivingFeelAxis");
		case ECFBatchMutationKind::RecipeMassExplicitValue:
			return TEXT("RecipeMassExplicitValue");
		case ECFBatchMutationKind::RecipeDurabilityExplicitValue:
			return TEXT("RecipeDurabilityExplicitValue");
		case ECFBatchMutationKind::ProfileNumericLeaf:
			return TEXT("ProfileNumericLeaf");
		default:
			return TEXT("Unknown");
		}
	}

	// Blank policy를 manifest용 stable technical token으로 변환합니다.
	FString BlankPolicyToString(const ECFBatchBlankPolicy BlankPolicy)
	{
		return BlankPolicy == ECFBatchBlankPolicy::NoChange ? TEXT("NoChange") : TEXT("NotApplicable");
	}

	// Dotted reflected property path를 read-only leaf property/address로 resolve합니다.
	bool ResolvePropertyPath(
		const void* RootContainer,
		const UStruct& RootStruct,
		const FString& PropertyPath,
		FReflectedValueView& OutView,
		FString& OutError)
	{
		OutView = FReflectedValueView();
		// Dot으로 분리한 stable reflection property names입니다.
		TArray<FString> PropertyParts;
		PropertyPath.ParseIntoArray(PropertyParts, TEXT("."), true);
		if (PropertyParts.IsEmpty())
		{
			OutError = TEXT("Reflection property path가 비어 있습니다.");
			return false;
		}

		// 현재 property를 찾을 reflected struct입니다.
		const UStruct* CurrentStruct = &RootStruct;
		// 현재 property storage의 outer container address입니다.
		const void* CurrentContainer = RootContainer;
		for (int32 PartIndex = 0; PartIndex < PropertyParts.Num(); ++PartIndex)
		{
			// 현재 path segment가 가리키는 reflected property입니다.
			const FProperty* Property = FindFProperty<FProperty>(CurrentStruct, FName(*PropertyParts[PartIndex]));
			if (!Property)
			{
				OutError = FString::Printf(TEXT("Reflection property를 찾지 못했습니다: %s"), *PropertyPath);
				return false;
			}
			// 현재 property의 actual value storage입니다.
			const void* ValueAddress = Property->ContainerPtrToValuePtr<void>(CurrentContainer);
			if (PartIndex == PropertyParts.Num() - 1)
			{
				OutView.Property = Property;
				OutView.ValueAddress = ValueAddress;
				OutError.Reset();
				return true;
			}

			// 중간 segment는 nested struct여야 합니다.
			const FStructProperty* StructProperty = CastField<FStructProperty>(Property);
			if (!StructProperty || !StructProperty->Struct)
			{
				OutError = FString::Printf(TEXT("Reflection path 중간 segment가 struct가 아닙니다: %s"), *PropertyPath);
				return false;
			}
			CurrentStruct = StructProperty->Struct;
			CurrentContainer = ValueAddress;
		}

		OutError = FString::Printf(TEXT("Reflection property path resolve가 terminal value 없이 끝났습니다: %s"), *PropertyPath);
		return false;
	}

	// Plain integer/float property를 invariant canonical numeric text로 읽습니다.
	bool ReadCanonicalNumeric(
		const FReflectedValueView& ValueView,
		FString& OutCanonicalValue,
		FString& OutError)
	{
		if (!ValueView.Property || !ValueView.ValueAddress || CastField<FBoolProperty>(ValueView.Property))
		{
			OutError = TEXT("Batch numeric value가 plain scalar numeric property가 아닙니다.");
			return false;
		}
		// Numeric scalar property입니다.
		const FNumericProperty* NumericProperty = CastField<FNumericProperty>(ValueView.Property);
		if (!NumericProperty || CastField<FEnumProperty>(ValueView.Property))
		{
			OutError = FString::Printf(TEXT("Batch editable property가 numeric leaf가 아닙니다: %s"), *ValueView.Property->GetName());
			return false;
		}
		if (const FByteProperty* ByteProperty = CastField<FByteProperty>(ValueView.Property))
		{
			if (ByteProperty->Enum)
			{
				OutError = FString::Printf(TEXT("Enum-backed byte property는 Batch numeric editable이 아닙니다: %s"), *ValueView.Property->GetName());
				return false;
			}
		}

				if (NumericProperty->IsInteger())
		{
			// UE 5.8에는 FNumericProperty::IsUnsigned가 없으므로 concrete reflected integer class로 판정합니다.
			const bool bUnsignedInteger = CastField<FByteProperty>(ValueView.Property) != nullptr
				|| CastField<FUInt16Property>(ValueView.Property) != nullptr
				|| CastField<FUInt32Property>(ValueView.Property) != nullptr
				|| CastField<FUInt64Property>(ValueView.Property) != nullptr;
			if (bUnsignedInteger)
			{
				// Unsigned integer exact value입니다.
				const uint64 IntegerValue = NumericProperty->GetUnsignedIntPropertyValue(ValueView.ValueAddress);
				OutCanonicalValue = FString::Printf(TEXT("%llu"), static_cast<unsigned long long>(IntegerValue));
			}
			else
			{
				// Signed integer exact value입니다.
				const int64 IntegerValue = NumericProperty->GetSignedIntPropertyValue(ValueView.ValueAddress);
				OutCanonicalValue = FString::Printf(TEXT("%lld"), static_cast<long long>(IntegerValue));
			}
			OutError.Reset();
			return true;
		}

		if (const FFloatProperty* FloatProperty = CastField<FFloatProperty>(ValueView.Property))
		{
			// Source float의 exact authored value입니다.
			const float FloatValue = FloatProperty->GetPropertyValue(ValueView.ValueAddress);
			if (!FMath::IsFinite(FloatValue))
			{
				OutError = FString::Printf(TEXT("NaN/Infinity는 canonical CSV numeric value로 export할 수 없습니다: %s"), *ValueView.Property->GetName());
				return false;
			}
			OutCanonicalValue = FString::Printf(TEXT("%.9g"), FloatValue == 0.0f ? 0.0 : static_cast<double>(FloatValue));
			OutCanonicalValue.ReplaceInline(TEXT(","), TEXT("."), ESearchCase::CaseSensitive);
			OutError.Reset();
			return true;
		}
		if (const FDoubleProperty* DoubleProperty = CastField<FDoubleProperty>(ValueView.Property))
		{
			// Source double의 exact authored value입니다.
			const double DoubleValue = DoubleProperty->GetPropertyValue(ValueView.ValueAddress);
			if (!FMath::IsFinite(DoubleValue))
			{
				OutError = FString::Printf(TEXT("NaN/Infinity는 canonical CSV numeric value로 export할 수 없습니다: %s"), *ValueView.Property->GetName());
				return false;
			}
			OutCanonicalValue = FString::Printf(TEXT("%.17g"), DoubleValue == 0.0 ? 0.0 : DoubleValue);
			OutCanonicalValue.ReplaceInline(TEXT(","), TEXT("."), ESearchCase::CaseSensitive);
			OutError.Reset();
			return true;
		}

		OutError = FString::Printf(TEXT("지원하지 않는 numeric property type입니다: %s"), *ValueView.Property->GetClass()->GetName());
		return false;
	}

	// Enum property의 current symbolic name을 source-mode ownership token으로 읽습니다.
	bool ReadEnumName(
		const void* RootContainer,
		const UStruct& RootStruct,
		const FString& PropertyPath,
		FString& OutEnumName,
		FString& OutError)
	{
		// Enum leaf property/address입니다.
		FReflectedValueView ValueView;
		if (!ResolvePropertyPath(RootContainer, RootStruct, PropertyPath, ValueView, OutError))
		{
			return false;
		}
		if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(ValueView.Property))
		{
			// Enum underlying numeric storage입니다.
			const FNumericProperty* UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
			// Current enum integer value입니다.
			const int64 EnumValue = UnderlyingProperty->GetSignedIntPropertyValue(ValueView.ValueAddress);
			OutEnumName = EnumProperty->GetEnum()->GetNameStringByValue(EnumValue);
			OutError.Reset();
			return !OutEnumName.IsEmpty();
		}
		if (const FByteProperty* ByteProperty = CastField<FByteProperty>(ValueView.Property))
		{
			if (ByteProperty->Enum)
			{
				// Legacy enum-backed byte current value입니다.
				const uint8 EnumValue = ByteProperty->GetPropertyValue(ValueView.ValueAddress);
				OutEnumName = ByteProperty->Enum->GetNameStringByValue(EnumValue);
				OutError.Reset();
				return !OutEnumName.IsEmpty();
			}
		}
		OutError = FString::Printf(TEXT("Edit condition target이 Enum property가 아닙니다: %s"), *PropertyPath);
		return false;
	}

	// Profile Domain과 UObject class가 exact typed schema로 일치하는지 검사합니다.
	bool IsExpectedProfileClass(const UObject& ProfileObject, const ECFVehicleProfileDomain Domain)
	{
		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			return ProfileObject.IsA(UCFVehicleBaseProfile::StaticClass());
		case ECFVehicleProfileDomain::Drivetrain:
			return ProfileObject.IsA(UCFDrivetrainProfile::StaticClass());
		case ECFVehicleProfileDomain::Handling:
			return ProfileObject.IsA(UCFHandlingProfile::StaticClass());
		case ECFVehicleProfileDomain::Performance:
			return ProfileObject.IsA(UCFPerformanceProfile::StaticClass());
		case ECFVehicleProfileDomain::DriveState:
			return ProfileObject.IsA(UCFDriveStateProfile::StaticClass());
		default:
			return false;
		}
	}

	// Existing SnapshotBuilder를 이용해 selected Profile의 resolver payload fingerprint를 읽습니다.
	bool BuildProfileFingerprint(
		const UObject& ProfileObject,
		const ECFVehicleProfileDomain Domain,
		FString& OutFingerprint,
		FString& OutError)
	{
		// VehicleBase selected Profile pointer입니다.
		const UCFVehicleBaseProfile* BaseProfile = Domain == ECFVehicleProfileDomain::VehicleBase ? Cast<UCFVehicleBaseProfile>(&ProfileObject) : nullptr;
		// Drivetrain selected Profile pointer입니다.
		const UCFDrivetrainProfile* DrivetrainProfile = Domain == ECFVehicleProfileDomain::Drivetrain ? Cast<UCFDrivetrainProfile>(&ProfileObject) : nullptr;
		// Handling selected Profile pointer입니다.
		const UCFHandlingProfile* HandlingProfile = Domain == ECFVehicleProfileDomain::Handling ? Cast<UCFHandlingProfile>(&ProfileObject) : nullptr;
		// Performance selected Profile pointer입니다.
		const UCFPerformanceProfile* PerformanceProfile = Domain == ECFVehicleProfileDomain::Performance ? Cast<UCFPerformanceProfile>(&ProfileObject) : nullptr;
		// DriveState selected Profile pointer입니다.
		const UCFDriveStateProfile* DriveStateProfile = Domain == ECFVehicleProfileDomain::DriveState ? Cast<UCFDriveStateProfile>(&ProfileObject) : nullptr;
		// Existing SnapshotBuilder output입니다.
		FCFVehicleProfileSnapshotSet ProfileSnapshots;
		if (!FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(
			BaseProfile,
			DrivetrainProfile,
			HandlingProfile,
			PerformanceProfile,
			DriveStateProfile,
			ProfileSnapshots,
			OutError))
		{
			return false;
		}

		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			OutFingerprint = ProfileSnapshots.BaseSource.ProfileFingerprint;
			break;
		case ECFVehicleProfileDomain::Drivetrain:
			OutFingerprint = ProfileSnapshots.DrivetrainSource.ProfileFingerprint;
			break;
		case ECFVehicleProfileDomain::Handling:
			OutFingerprint = ProfileSnapshots.HandlingSource.ProfileFingerprint;
			break;
		case ECFVehicleProfileDomain::Performance:
			OutFingerprint = ProfileSnapshots.PerformanceSource.ProfileFingerprint;
			break;
		case ECFVehicleProfileDomain::DriveState:
			OutFingerprint = ProfileSnapshots.DriveStateSource.ProfileFingerprint;
			break;
		default:
			OutError = TEXT("지원하지 않는 Profile Domain입니다.");
			return false;
		}
		return !OutFingerprint.IsEmpty();
	}

	// Recipe binding과 optional target identity를 확인하고 current UCFVehicleData를 반환합니다.
	const UCFVehicleData* ResolveRecipeTarget(
		const UCFVehicleRecipeData& Recipe,
		const UCFVehicleData* OptionalTargetVehicleData,
		FString& OutError)
	{
		// Recipe가 소유하는 canonical Target soft object path입니다.
		const FSoftObjectPath RecipeTargetPath = Recipe.TargetVehicleData.ToSoftObjectPath();
		if (!RecipeTargetPath.IsValid())
		{
			OutError = TEXT("RecipeNumericEdit export에는 Recipe TargetVehicleData binding이 필요합니다.");
			return nullptr;
		}
		if (OptionalTargetVehicleData)
		{
			// Caller-provided exact Target identity입니다.
			const FSoftObjectPath OptionalTargetPath(OptionalTargetVehicleData);
			if (OptionalTargetPath != RecipeTargetPath)
			{
				OutError = TEXT("Recipe Target binding과 explicit export Target identity가 다릅니다.");
				return nullptr;
			}
			OutError.Reset();
			return OptionalTargetVehicleData;
		}

		// 이미 load된 target 또는 soft path에서 load한 object입니다.
		UObject* LoadedObject = RecipeTargetPath.ResolveObject();
		if (!LoadedObject)
		{
			LoadedObject = RecipeTargetPath.TryLoad();
		}
		// Exact runtime canonical target입니다.
		const UCFVehicleData* TargetVehicleData = Cast<UCFVehicleData>(LoadedObject);
		if (!TargetVehicleData)
		{
			OutError = FString::Printf(TEXT("Recipe Target VehicleData를 resolve할 수 없습니다: %s"), *RecipeTargetPath.ToString());
			return nullptr;
		}
		OutError.Reset();
		return TargetVehicleData;
	}

	// Baseline cell 배열에서 exact stable ColumnId를 찾습니다.
	const FCFBatchBaselineCell* FindCell(const TArray<FCFBatchBaselineCell>& Cells, const FString& ColumnId)
	{
		return Cells.FindByPredicate([&ColumnId](const FCFBatchBaselineCell& Cell)
		{
			return Cell.ColumnId == ColumnId;
		});
	}

	// Standard CSV escaping 규칙으로 한 cell을 직렬화합니다.
	FString EscapeCsvCell(const FString& CellValue)
	{
		// comma/quote/newline이 있으면 standard quote escaping이 필요합니다.
		const bool bNeedsQuotes = CellValue.Contains(TEXT(","))
			|| CellValue.Contains(TEXT("\""))
			|| CellValue.Contains(TEXT("\r"))
			|| CellValue.Contains(TEXT("\n"));
		if (!bNeedsQuotes)
		{
			return CellValue;
		}
		// 내부 quote를 두 개의 quote로 escape한 payload입니다.
		FString EscapedValue = CellValue.Replace(TEXT("\""), TEXT("\"\""), ESearchCase::CaseSensitive);
		return TEXT("\"") + EscapedValue + TEXT("\"");
	}

	// FString을 BOM 없는 exact UTF-8 bytes로 변환합니다.
	void BuildUtf8Bytes(const FString& Text, TArray<uint8>& OutBytes)
	{
		OutBytes.Reset();
		// TCHAR text의 UTF-8 transcoded view입니다.
		const FTCHARToUTF8 Utf8Text(*Text);
		OutBytes.Append(reinterpret_cast<const uint8*>(Utf8Text.Get()), Utf8Text.Length());
	}

	// Descriptor의 hash/manifest relevant technical fields가 exact 같은지 비교합니다.
	bool AreDescriptorsEquivalent(const FCFBatchColumnDescriptor& Left, const FCFBatchColumnDescriptor& Right)
	{
		return Left.ColumnId == Right.ColumnId
			&& Left.DatasetKind == Right.DatasetKind
			&& Left.ProfileDomain == Right.ProfileDomain
			&& Left.ValueType == Right.ValueType
			&& Left.Unit == Right.Unit
			&& Left.Access == Right.Access
			&& Left.AuthoringOwner == Right.AuthoringOwner
			&& Left.TypedMutationKind == Right.TypedMutationKind
			&& Left.PropertyOrSemanticTarget == Right.PropertyOrSemanticTarget
			&& Left.BlankPolicy == Right.BlankPolicy
			&& Left.ValidationMetadata == Right.ValidationMetadata
			&& Left.EditConditionTarget == Right.EditConditionTarget
			&& Left.RequiredEditConditionValue == Right.RequiredEditConditionValue
			&& Left.bReservedMetadata == Right.bReservedMetadata;
	}

	// Stable manifest JSON에 editable technical descriptor 하나를 기록합니다.
	void WriteDescriptorJson(
		TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>& Writer,
		const FCFBatchColumnDescriptor& Descriptor)
	{
		Writer.WriteObjectStart();
		Writer.WriteValue(TEXT("ColumnId"), Descriptor.ColumnId);
		Writer.WriteValue(TEXT("ValueType"), ValueTypeToString(Descriptor.ValueType));
		Writer.WriteValue(TEXT("Unit"), Descriptor.Unit);
		Writer.WriteValue(TEXT("Access"), AccessToString(Descriptor.Access));
		Writer.WriteValue(TEXT("AuthoringOwner"), OwnerToString(Descriptor.AuthoringOwner));
		Writer.WriteValue(TEXT("TypedMutationKind"), MutationToString(Descriptor.TypedMutationKind));
		Writer.WriteValue(TEXT("PropertyOrSemanticTarget"), Descriptor.PropertyOrSemanticTarget);
		Writer.WriteValue(TEXT("BlankPolicy"), BlankPolicyToString(Descriptor.BlankPolicy));
		Writer.WriteValue(TEXT("ValidationMetadata"), Descriptor.ValidationMetadata);
		Writer.WriteValue(TEXT("EditConditionTarget"), Descriptor.EditConditionTarget);
		Writer.WriteValue(TEXT("RequiredEditConditionValue"), Descriptor.RequiredEditConditionValue);
		Writer.WriteObjectEnd();
	}

	// Fixed field order의 deterministic .cfbatch.json text를 만듭니다.
	bool BuildManifestJson(const FCFBatchManifest& Manifest, FString& OutJson, FString& OutError)
	{
		OutJson.Reset();
		// Whitespace 없는 deterministic JSON writer입니다.
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutJson);
		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("BatchExportId"), Manifest.BatchExportId.ToString(EGuidFormats::DigitsWithHyphensLower));
		Writer->WriteValue(TEXT("DatasetKind"), DatasetToString(Manifest.DatasetKind));
		Writer->WriteValue(TEXT("ProfileDomain"), DomainToString(Manifest.ProfileDomain));
		Writer->WriteValue(TEXT("SchemaId"), Manifest.SchemaId);
		Writer->WriteValue(TEXT("SchemaRevision"), Manifest.SchemaRevision);
		Writer->WriteValue(TEXT("ResolverContractRevision"), Manifest.ResolverContractRevision);

		Writer->WriteArrayStart(TEXT("CsvColumnIds"));
		for (const FString& ColumnId : Manifest.CsvColumnIds)
		{
			Writer->WriteValue(ColumnId);
		}
		Writer->WriteArrayEnd();

		Writer->WriteArrayStart(TEXT("EditableColumnDescriptors"));
		for (const FCFBatchColumnDescriptor& Descriptor : Manifest.EditableColumns)
		{
			WriteDescriptorJson(*Writer, Descriptor);
		}
		Writer->WriteArrayEnd();

		Writer->WriteArrayStart(TEXT("Rows"));
		for (const FCFBatchManifestRow& Row : Manifest.Rows)
		{
			Writer->WriteObjectStart();
			Writer->WriteValue(TEXT("RowId"), Row.RowId);
			Writer->WriteValue(TEXT("TargetPath"), Row.TargetPath);
			Writer->WriteValue(TEXT("RecipePath"), Row.RecipePath);
			Writer->WriteValue(TEXT("ProfilePath"), Row.ProfilePath);
			Writer->WriteValue(TEXT("ProfileDomain"), DomainToString(Row.ProfileDomain));
			Writer->WriteValue(TEXT("ObjectFingerprint"), Row.ObjectFingerprint);
			Writer->WriteValue(TEXT("TargetDefinitionHash"), Row.TargetDefinitionHash);
			Writer->WriteArrayStart(TEXT("BaselineCells"));
			for (const FCFBatchBaselineCell& Cell : Row.BaselineCells)
			{
				Writer->WriteObjectStart();
				Writer->WriteValue(TEXT("ColumnId"), Cell.ColumnId);
				Writer->WriteValue(TEXT("CanonicalValue"), Cell.CanonicalValue);
				Writer->WriteValue(TEXT("EditableAtExport"), Cell.bEditableAtExport);
				Writer->WriteValue(TEXT("OwnershipSourceMode"), Cell.OwnershipSourceMode);
				Writer->WriteObjectEnd();
			}
			Writer->WriteArrayEnd();
			Writer->WriteObjectEnd();
		}
		Writer->WriteArrayEnd();
		Writer->WriteValue(TEXT("ExportSetHash"), Manifest.ExportSetHash);
		Writer->WriteObjectEnd();
		if (!Writer->Close())
		{
			OutError = TEXT(".cfbatch.json writer close에 실패했습니다.");
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Reserved metadata ColumnId의 row-specific canonical value를 반환합니다.
	FString GetReservedCellValue(
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
}

// Persistent Recipe/Target을 수정하지 않고 RecipeNumericEdit 한 row를 current schema에서 projection합니다.
bool FCFBatchExportService::BuildRecipeNumericRow(
	const UCFVehicleRecipeData& Recipe,
	const UCFVehicleData* OptionalTargetVehicleData,
	FCFBatchExportRow& OutRow,
	TArray<FString>& OutErrors)
{
	OutRow = FCFBatchExportRow();
	OutErrors.Reset();

	// Existing Recipe Snapshot/fingerprint authority입니다.
	FCFVehicleRecipeSnapshot RecipeSnapshot;
	// Snapshot/target/Reflection failure diagnostic입니다.
	FString Error;
	if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(Recipe, RecipeSnapshot, Error))
	{
		OutErrors.Add(Error);
		return false;
	}
	// Recipe binding이 가리키는 current Target VehicleData입니다.
	const UCFVehicleData* TargetVehicleData = CFBatchExportPrivate::ResolveRecipeTarget(Recipe, OptionalTargetVehicleData, Error);
	if (!TargetVehicleData)
	{
		OutErrors.Add(Error);
		return false;
	}
	// Existing full Definition Snapshot/hash authority입니다.
	FCFVehicleDefinitionSnapshot TargetSnapshot;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*TargetVehicleData, TargetSnapshot, Error))
	{
		OutErrors.Add(Error);
		return false;
	}

	// Current RecipeNumericEdit schema projection입니다.
	TArray<FCFBatchColumnDescriptor> Columns;
	if (!FCFBatchColumnRegistry::GetDatasetColumns(
		ECFBatchDatasetKind::RecipeNumericEdit,
		ECFVehicleProfileDomain::None,
		Columns,
		OutErrors))
	{
		return false;
	}

	OutRow.RowId = Recipe.RecipeId.ToString(EGuidFormats::Digits);
	OutRow.TargetPath = FSoftObjectPath(TargetVehicleData).ToString();
	OutRow.RecipePath = FSoftObjectPath(&Recipe).ToString();
	OutRow.ObjectFingerprint = RecipeSnapshot.RecipeFingerprint;
	OutRow.TargetDefinitionHash = TargetSnapshot.DefinitionHash;

	for (const FCFBatchColumnDescriptor& Descriptor : Columns)
	{
		if (Descriptor.bReservedMetadata)
		{
			continue;
		}
		// Recipe semantic numeric leaf의 reflected property/address입니다.
		CFBatchExportPrivate::FReflectedValueView ValueView;
		if (!CFBatchExportPrivate::ResolvePropertyPath(
			&Recipe,
			*Recipe.GetClass(),
			Descriptor.PropertyOrSemanticTarget,
			ValueView,
			Error))
		{
			OutErrors.Add(Error);
			continue;
		}
		// Export baseline의 invariant canonical numeric value입니다.
		FString CanonicalValue;
		if (!CFBatchExportPrivate::ReadCanonicalNumeric(ValueView, CanonicalValue, Error))
		{
			OutErrors.Add(Error);
			continue;
		}

		// Source mode 조건이 없는 Driving Feel은 항상 Recipe semantic input으로 editable입니다.
		bool bEditableAtExport = true;
		// Manifest baseline ownership/source mode입니다.
		FString OwnershipSourceMode = TEXT("RecipeExplicitSemanticInput");
		if (!Descriptor.EditConditionTarget.IsEmpty())
		{
			if (!CFBatchExportPrivate::ReadEnumName(
				&Recipe,
				*Recipe.GetClass(),
				Descriptor.EditConditionTarget,
				OwnershipSourceMode,
				Error))
			{
				OutErrors.Add(Error);
				continue;
			}
			bEditableAtExport = OwnershipSourceMode == Descriptor.RequiredEditConditionValue;
		}

		// Manifest가 보존할 exact baseline cell입니다.
		FCFBatchBaselineCell& Cell = OutRow.BaselineCells.AddDefaulted_GetRef();
		Cell.ColumnId = Descriptor.ColumnId;
		Cell.CanonicalValue = CanonicalValue;
		Cell.bEditableAtExport = bEditableAtExport;
		Cell.OwnershipSourceMode = OwnershipSourceMode;
	}

	OutRow.BaselineCells.Sort([](const FCFBatchBaselineCell& Left, const FCFBatchBaselineCell& Right)
	{
		return Left.ColumnId < Right.ColumnId;
	});
	return OutErrors.IsEmpty();
}

// Persistent Profile을 수정하지 않고 ProfileNumericEdit 한 row를 current typed schema에서 projection합니다.
bool FCFBatchExportService::BuildProfileNumericRow(
	const UObject& ProfileObject,
	const ECFVehicleProfileDomain ProfileDomain,
	FCFBatchExportRow& OutRow,
	TArray<FString>& OutErrors)
{
	OutRow = FCFBatchExportRow();
	OutErrors.Reset();
	if (!CFBatchExportPrivate::IsExpectedProfileClass(ProfileObject, ProfileDomain))
	{
		OutErrors.Add(TEXT("ProfileNumericEdit Domain과 Profile UObject class가 일치하지 않습니다."));
		return false;
	}

	// Existing SnapshotBuilder가 계산한 typed resolver payload fingerprint입니다.
	FString ProfileFingerprint;
	// Reflection/snapshot failure diagnostic입니다.
	FString Error;
	if (!CFBatchExportPrivate::BuildProfileFingerprint(ProfileObject, ProfileDomain, ProfileFingerprint, Error))
	{
		OutErrors.Add(Error);
		return false;
	}
	// Current ProfileNumericEdit exact Domain schema projection입니다.
	TArray<FCFBatchColumnDescriptor> Columns;
	if (!FCFBatchColumnRegistry::GetDatasetColumns(
		ECFBatchDatasetKind::ProfileNumericEdit,
		ProfileDomain,
		Columns,
		OutErrors))
	{
		return false;
	}

	OutRow.RowId = FSoftObjectPath(&ProfileObject).ToString();
	OutRow.ProfilePath = FSoftObjectPath(&ProfileObject).ToString();
	OutRow.ProfileDomain = ProfileDomain;
	OutRow.ObjectFingerprint = ProfileFingerprint;

	for (const FCFBatchColumnDescriptor& Descriptor : Columns)
	{
		if (Descriptor.bReservedMetadata)
		{
			continue;
		}
		// Typed Profile Data nested numeric leaf property/address입니다.
		CFBatchExportPrivate::FReflectedValueView ValueView;
		if (!CFBatchExportPrivate::ResolvePropertyPath(
			&ProfileObject,
			*ProfileObject.GetClass(),
			Descriptor.PropertyOrSemanticTarget,
			ValueView,
			Error))
		{
			OutErrors.Add(Error);
			continue;
		}
		// Export baseline invariant canonical numeric value입니다.
		FString CanonicalValue;
		if (!CFBatchExportPrivate::ReadCanonicalNumeric(ValueView, CanonicalValue, Error))
		{
			OutErrors.Add(Error);
			continue;
		}

		// Typed Profile scalar numeric leaf는 해당 exact Domain owner로 Batch editable입니다.
		FCFBatchBaselineCell& Cell = OutRow.BaselineCells.AddDefaulted_GetRef();
		Cell.ColumnId = Descriptor.ColumnId;
		Cell.CanonicalValue = CanonicalValue;
		Cell.bEditableAtExport = true;
		Cell.OwnershipSourceMode = TEXT("Profile.") + CFBatchExportPrivate::DomainToString(ProfileDomain);
	}

	OutRow.BaselineCells.Sort([](const FCFBatchBaselineCell& Left, const FCFBatchBaselineCell& Right)
	{
		return Left.ColumnId < Right.ColumnId;
	});
	return OutErrors.IsEmpty();
}

// Immutable manifest baseline은 바꾸지 않고 exact one editable cell만 caller value로 교체한 canonical CSV를 재구성합니다.
bool FCFBatchExportService::BuildEditedCellCsv(
	const FCFBatchExportArtifact& Artifact,
	const FString& RowId,
	const FString& ColumnId,
	const FString& CanonicalValue,
	FString& OutCsvText,
	TArray<FString>& OutErrors)
{
	OutCsvText.Reset();
	OutErrors.Reset();

	// Current projected registry columns입니다.
	TArray<FCFBatchColumnDescriptor> Columns;
	if (!FCFBatchColumnRegistry::GetDatasetColumns(Artifact.Manifest.DatasetKind, Artifact.Manifest.ProfileDomain, Columns, OutErrors))
	{
		return false;
	}
	if (Artifact.Manifest.CsvColumnIds.Num() != Columns.Num())
	{
		OutErrors.Add(TEXT("Edited CSV builder의 manifest header가 current Registry와 다릅니다."));
		return false;
	}
	for (int32 ColumnIndex = 0; ColumnIndex < Columns.Num(); ++ColumnIndex)
	{
		if (Artifact.Manifest.CsvColumnIds[ColumnIndex] != Columns[ColumnIndex].ColumnId)
		{
			OutErrors.Add(TEXT("Edited CSV builder의 canonical ColumnId 순서가 current Registry와 다릅니다."));
			return false;
		}
	}

	// Caller가 바꾸려는 exact editable Registry column입니다.
	const FCFBatchColumnDescriptor* EditedDescriptor = Columns.FindByPredicate([&ColumnId](const FCFBatchColumnDescriptor& Descriptor)
	{
		return Descriptor.ColumnId == ColumnId;
	});
	if (!EditedDescriptor || EditedDescriptor->bReservedMetadata || EditedDescriptor->Access != ECFBatchColumnAccess::Editable)
	{
		OutErrors.Add(FString::Printf(TEXT("Edited CSV 대상 column이 current editable allowlist가 아닙니다: %s"), *ColumnId));
		return false;
	}

	// Caller가 바꾸려는 exact manifest row입니다.
	const FCFBatchManifestRow* EditedRow = Artifact.Manifest.Rows.FindByPredicate([&RowId](const FCFBatchManifestRow& Row)
	{
		return Row.RowId == RowId;
	});
	if (!EditedRow)
	{
		OutErrors.Add(FString::Printf(TEXT("Edited CSV 대상 RowId가 manifest에 없습니다: %s"), *RowId));
		return false;
	}
	// Export 시점 exact baseline cell입니다.
	const FCFBatchBaselineCell* EditedBaselineCell = CFBatchExportPrivate::FindCell(EditedRow->BaselineCells, ColumnId);
	if (!EditedBaselineCell || !EditedBaselineCell->bEditableAtExport)
	{
		OutErrors.Add(FString::Printf(TEXT("Edited CSV 대상 cell은 export 시점에 editable하지 않았습니다: Row=%s Column=%s"), *RowId, *ColumnId));
		return false;
	}

	// Stable ColumnId header입니다.
	TArray<FString> HeaderCells;
	HeaderCells.Reserve(Columns.Num());
	for (const FCFBatchColumnDescriptor& Descriptor : Columns)
	{
		HeaderCells.Add(CFBatchExportPrivate::EscapeCsvCell(Descriptor.ColumnId));
	}
	OutCsvText = FString::Join(HeaderCells, TEXT(",")) + TEXT("\n");

	for (const FCFBatchManifestRow& Row : Artifact.Manifest.Rows)
	{
		// Canonical column order의 one row입니다.
		TArray<FString> CsvCells;
		CsvCells.Reserve(Columns.Num());
		for (const FCFBatchColumnDescriptor& Descriptor : Columns)
		{
			// Reserved metadata 또는 immutable baseline/edit one-cell value입니다.
			FString CellValue;
			if (Descriptor.bReservedMetadata)
			{
				CellValue = CFBatchExportPrivate::GetReservedCellValue(Descriptor, Artifact.Manifest, Row);
			}
			else
			{
				// Exact immutable baseline cell입니다.
				const FCFBatchBaselineCell* BaselineCell = CFBatchExportPrivate::FindCell(Row.BaselineCells, Descriptor.ColumnId);
				if (!BaselineCell)
				{
					OutErrors.Add(FString::Printf(TEXT("Edited CSV serialize 중 baseline cell이 없습니다: Row=%s Column=%s"), *Row.RowId, *Descriptor.ColumnId));
					return false;
				}
				if (Row.RowId == RowId && Descriptor.ColumnId == ColumnId)
				{
					CellValue = CanonicalValue;
				}
				else
				{
					CellValue = Descriptor.Access == ECFBatchColumnAccess::Editable && !BaselineCell->bEditableAtExport
						? FString()
						: BaselineCell->CanonicalValue;
				}
			}
			CsvCells.Add(CFBatchExportPrivate::EscapeCsvCell(CellValue));
		}
		OutCsvText += FString::Join(CsvCells, TEXT(",")) + TEXT("\n");
	}
	return true;
}

// Registry schema와 row baseline으로 canonical CSV/UTF-8/manifest/hash를 deterministic하게 생성합니다.
bool FCFBatchExportService::BuildExport(
	const FCFBatchExportRequest& Request,
	FCFBatchExportArtifact& OutArtifact,
	TArray<FString>& OutErrors)
{
	OutArtifact = FCFBatchExportArtifact();
	OutErrors.Reset();
	if (!Request.BatchExportId.IsValid())
	{
		OutErrors.Add(TEXT("BatchExportId는 exporter가 임의 생성하지 않으며 valid explicit GUID가 필요합니다."));
		return false;
	}
	if (Request.Rows.IsEmpty())
	{
		OutErrors.Add(TEXT("Canonical Batch export에는 최소 한 row가 필요합니다."));
		return false;
	}

	// Current projection Registry의 canonical dataset columns입니다.
	TArray<FCFBatchColumnDescriptor> Columns;
	if (!FCFBatchColumnRegistry::GetDatasetColumns(Request.DatasetKind, Request.ProfileDomain, Columns, OutErrors))
	{
		return false;
	}

	// Request row order와 독립적인 canonical RowId order copy입니다.
	TArray<FCFBatchExportRow> SortedRows = Request.Rows;
	SortedRows.Sort([](const FCFBatchExportRow& Left, const FCFBatchExportRow& Right)
	{
		return Left.RowId < Right.RowId;
	});

	// Duplicate RowId 탐지 set입니다.
	TSet<FString> SeenRowIds;
	for (const FCFBatchExportRow& Row : SortedRows)
	{
		if (Row.RowId.IsEmpty())
		{
			OutErrors.Add(TEXT("Batch export row에는 non-empty stable RowId가 필요합니다."));
			continue;
		}
		if (SeenRowIds.Contains(Row.RowId))
		{
			OutErrors.Add(FString::Printf(TEXT("Duplicate Batch RowId: %s"), *Row.RowId));
			continue;
		}
		SeenRowIds.Add(Row.RowId);
		if (Request.DatasetKind == ECFBatchDatasetKind::ProfileNumericEdit && Row.ProfileDomain != Request.ProfileDomain)
		{
			OutErrors.Add(FString::Printf(TEXT("Profile row Domain이 file Dataset Domain과 다릅니다: %s"), *Row.RowId));
		}
	}
	if (!OutErrors.IsEmpty())
	{
		return false;
	}

	// Current registry 기반 immutable-style manifest입니다.
	FCFBatchManifest Manifest;
	Manifest.BatchExportId = Request.BatchExportId;
	Manifest.DatasetKind = Request.DatasetKind;
	Manifest.ProfileDomain = Request.ProfileDomain;
	Manifest.SchemaId = FCFBatchColumnRegistry::GetSchemaId(Request.DatasetKind, Request.ProfileDomain);
	Manifest.SchemaRevision = FCFBatchColumnRegistry::GetSchemaRevision();
	Manifest.ResolverContractRevision = FCFVehicleResolver::CurrentResolverContractRevision;
	for (const FCFBatchColumnDescriptor& Descriptor : Columns)
	{
		Manifest.CsvColumnIds.Add(Descriptor.ColumnId);
		if (Descriptor.Access == ECFBatchColumnAccess::Editable && !Descriptor.bReservedMetadata)
		{
			Manifest.EditableColumns.Add(Descriptor);
		}
	}

	for (const FCFBatchExportRow& SourceRow : SortedRows)
	{
		// Manifest에 보존할 one row baseline입니다.
		FCFBatchManifestRow ManifestRow;
		ManifestRow.RowId = SourceRow.RowId;
		ManifestRow.TargetPath = SourceRow.TargetPath;
		ManifestRow.RecipePath = SourceRow.RecipePath;
		ManifestRow.ProfilePath = SourceRow.ProfilePath;
		ManifestRow.ProfileDomain = SourceRow.ProfileDomain;
		ManifestRow.ObjectFingerprint = SourceRow.ObjectFingerprint;
		ManifestRow.TargetDefinitionHash = SourceRow.TargetDefinitionHash;
		ManifestRow.BaselineCells = SourceRow.BaselineCells;
		ManifestRow.BaselineCells.Sort([](const FCFBatchBaselineCell& Left, const FCFBatchBaselineCell& Right)
		{
			return Left.ColumnId < Right.ColumnId;
		});

		// Duplicate/unknown/missing non-reserved cell을 fail-closed 검사합니다.
		TSet<FString> SeenCellIds;
		for (const FCFBatchBaselineCell& Cell : ManifestRow.BaselineCells)
		{
			if (SeenCellIds.Contains(Cell.ColumnId))
			{
				OutErrors.Add(FString::Printf(TEXT("Duplicate baseline ColumnId: Row=%s Column=%s"), *ManifestRow.RowId, *Cell.ColumnId));
				continue;
			}
			SeenCellIds.Add(Cell.ColumnId);
			// Registry column 존재 여부입니다.
			const FCFBatchColumnDescriptor* Descriptor = Columns.FindByPredicate([&Cell](const FCFBatchColumnDescriptor& Candidate)
			{
				return Candidate.ColumnId == Cell.ColumnId;
			});
			if (!Descriptor || Descriptor->bReservedMetadata)
			{
				OutErrors.Add(FString::Printf(TEXT("Unknown/reserved baseline ColumnId: Row=%s Column=%s"), *ManifestRow.RowId, *Cell.ColumnId));
			}
		}
		for (const FCFBatchColumnDescriptor& Descriptor : Columns)
		{
			if (!Descriptor.bReservedMetadata && !SeenCellIds.Contains(Descriptor.ColumnId))
			{
				OutErrors.Add(FString::Printf(TEXT("Missing baseline cell: Row=%s Column=%s"), *ManifestRow.RowId, *Descriptor.ColumnId));
			}
		}
		Manifest.Rows.Add(MoveTemp(ManifestRow));
	}
	if (!OutErrors.IsEmpty())
	{
		return false;
	}

	Manifest.ExportSetHash = BuildExportSetHash(Manifest);
	OutArtifact.Manifest = Manifest;

	// Stable ColumnId header입니다.
	TArray<FString> HeaderCells;
	HeaderCells.Reserve(Columns.Num());
	for (const FCFBatchColumnDescriptor& Descriptor : Columns)
	{
		HeaderCells.Add(CFBatchExportPrivate::EscapeCsvCell(Descriptor.ColumnId));
	}
	OutArtifact.CanonicalCsvText = FString::Join(HeaderCells, TEXT(",")) + TEXT("\n");

	for (const FCFBatchManifestRow& Row : Manifest.Rows)
	{
		// Canonical column order에 맞춘 one CSV row cells입니다.
		TArray<FString> CsvCells;
		CsvCells.Reserve(Columns.Num());
		for (const FCFBatchColumnDescriptor& Descriptor : Columns)
		{
			// Reserved metadata 또는 baseline canonical numeric/string value입니다.
			FString CellValue;
			if (Descriptor.bReservedMetadata)
			{
				CellValue = CFBatchExportPrivate::GetReservedCellValue(Descriptor, Manifest, Row);
			}
			else
			{
				// Exact baseline cell입니다.
				const FCFBatchBaselineCell* Cell = CFBatchExportPrivate::FindCell(Row.BaselineCells, Descriptor.ColumnId);
				if (!Cell)
				{
					OutErrors.Add(FString::Printf(TEXT("CSV serialize 중 baseline cell이 사라졌습니다: Row=%s Column=%s"), *Row.RowId, *Descriptor.ColumnId));
					return false;
				}
				// Source mode가 Spreadsheet edit를 허용하지 않으면 CSV cell은 blank/unavailable로 export합니다.
				CellValue = Descriptor.Access == ECFBatchColumnAccess::Editable && !Cell->bEditableAtExport
					? FString()
					: Cell->CanonicalValue;
			}
			CsvCells.Add(CFBatchExportPrivate::EscapeCsvCell(CellValue));
		}
		OutArtifact.CanonicalCsvText += FString::Join(CsvCells, TEXT(",")) + TEXT("\n");
	}
	CFBatchExportPrivate::BuildUtf8Bytes(OutArtifact.CanonicalCsvText, OutArtifact.CsvUtf8Bytes);

	// Fixed-order immutable baseline JSON입니다.
	FString JsonError;
	if (!CFBatchExportPrivate::BuildManifestJson(Manifest, OutArtifact.ManifestJsonText, JsonError))
	{
		OutErrors.Add(JsonError);
		return false;
	}
	CFBatchExportPrivate::BuildUtf8Bytes(OutArtifact.ManifestJsonText, OutArtifact.ManifestUtf8Bytes);
	return true;
}

// Manifest semantic baseline을 다시 hash/검사해 손상 또는 descriptor mismatch를 탐지합니다.
bool FCFBatchExportService::ValidateManifestBaseline(
	const FCFBatchManifest& Manifest,
	TArray<FString>& OutErrors)
{
	OutErrors.Reset();
	if (!Manifest.BatchExportId.IsValid())
	{
		OutErrors.Add(TEXT("Manifest BatchExportId가 invalid입니다."));
	}
	// Current projected registry columns입니다.
	TArray<FCFBatchColumnDescriptor> CurrentColumns;
	TArray<FString> RegistryErrors;
	if (!FCFBatchColumnRegistry::GetDatasetColumns(Manifest.DatasetKind, Manifest.ProfileDomain, CurrentColumns, RegistryErrors))
	{
		OutErrors.Append(RegistryErrors);
		return false;
	}
	if (Manifest.SchemaId != FCFBatchColumnRegistry::GetSchemaId(Manifest.DatasetKind, Manifest.ProfileDomain)
		|| Manifest.SchemaRevision != FCFBatchColumnRegistry::GetSchemaRevision())
	{
		OutErrors.Add(TEXT("Manifest schema id/revision이 current Batch Column Registry와 일치하지 않습니다."));
	}
	if (Manifest.ResolverContractRevision != FCFVehicleResolver::CurrentResolverContractRevision)
	{
		OutErrors.Add(TEXT("Manifest ResolverContractRevision이 current Frozen Resolver contract와 일치하지 않습니다."));
	}

	// Current canonical CSV header ids입니다.
	TArray<FString> CurrentColumnIds;
	// Current editable technical descriptors입니다.
	TArray<FCFBatchColumnDescriptor> CurrentEditableColumns;
	for (const FCFBatchColumnDescriptor& Descriptor : CurrentColumns)
	{
		CurrentColumnIds.Add(Descriptor.ColumnId);
		if (Descriptor.Access == ECFBatchColumnAccess::Editable && !Descriptor.bReservedMetadata)
		{
			CurrentEditableColumns.Add(Descriptor);
		}
	}
	if (Manifest.CsvColumnIds != CurrentColumnIds)
	{
		OutErrors.Add(TEXT("Manifest CsvColumnIds가 current canonical Registry header와 일치하지 않습니다."));
	}
	if (Manifest.EditableColumns.Num() != CurrentEditableColumns.Num())
	{
		OutErrors.Add(TEXT("Manifest editable descriptor count가 current Registry와 일치하지 않습니다."));
	}
	else
	{
		for (int32 ColumnIndex = 0; ColumnIndex < CurrentEditableColumns.Num(); ++ColumnIndex)
		{
			if (!CFBatchExportPrivate::AreDescriptorsEquivalent(Manifest.EditableColumns[ColumnIndex], CurrentEditableColumns[ColumnIndex]))
			{
				OutErrors.Add(FString::Printf(TEXT("Manifest editable descriptor가 current Registry와 다릅니다: %s"), *CurrentEditableColumns[ColumnIndex].ColumnId));
			}
		}
	}

	// Duplicate row identity를 검증할 set입니다.
	TSet<FString> SeenRowIds;
	for (const FCFBatchManifestRow& Row : Manifest.Rows)
	{
		if (Row.RowId.IsEmpty() || SeenRowIds.Contains(Row.RowId))
		{
			OutErrors.Add(FString::Printf(TEXT("Manifest row identity가 비었거나 중복입니다: %s"), *Row.RowId));
			continue;
		}
		SeenRowIds.Add(Row.RowId);
		if (Manifest.DatasetKind == ECFBatchDatasetKind::ProfileNumericEdit && Row.ProfileDomain != Manifest.ProfileDomain)
		{
			OutErrors.Add(FString::Printf(TEXT("Manifest Profile row Domain mismatch: %s"), *Row.RowId));
		}

		// Row baseline cell identity 검증 set입니다.
		TSet<FString> SeenCellIds;
		for (const FCFBatchBaselineCell& Cell : Row.BaselineCells)
		{
			if (SeenCellIds.Contains(Cell.ColumnId))
			{
				OutErrors.Add(FString::Printf(TEXT("Manifest baseline duplicate ColumnId: %s / %s"), *Row.RowId, *Cell.ColumnId));
				continue;
			}
			SeenCellIds.Add(Cell.ColumnId);
		}
		for (const FCFBatchColumnDescriptor& Descriptor : CurrentColumns)
		{
			if (!Descriptor.bReservedMetadata && !SeenCellIds.Contains(Descriptor.ColumnId))
			{
				OutErrors.Add(FString::Printf(TEXT("Manifest baseline missing ColumnId: %s / %s"), *Row.RowId, *Descriptor.ColumnId));
			}
		}
		for (const FCFBatchBaselineCell& Cell : Row.BaselineCells)
		{
			const bool bKnownNonReservedColumn = CurrentColumns.ContainsByPredicate([&Cell](const FCFBatchColumnDescriptor& Descriptor)
			{
				return !Descriptor.bReservedMetadata && Descriptor.ColumnId == Cell.ColumnId;
			});
			if (!bKnownNonReservedColumn)
			{
				OutErrors.Add(FString::Printf(TEXT("Manifest baseline unknown ColumnId: %s / %s"), *Row.RowId, *Cell.ColumnId));
			}
		}
	}

	if (Manifest.ExportSetHash.IsEmpty() || Manifest.ExportSetHash != BuildExportSetHash(Manifest))
	{
		OutErrors.Add(TEXT("Manifest ExportSetHash가 semantic baseline과 일치하지 않습니다."));
	}
	return OutErrors.IsEmpty();
}

// ExportSetHash authority를 manifest semantic data에서 deterministic하게 계산합니다.
FString FCFBatchExportService::BuildExportSetHash(const FCFBatchManifest& Manifest)
{
	// Hash input에서 row/descriptor order를 canonicalize한 local copies입니다.
	TArray<FCFBatchColumnDescriptor> EditableColumns = Manifest.EditableColumns;
	EditableColumns.Sort([](const FCFBatchColumnDescriptor& Left, const FCFBatchColumnDescriptor& Right)
	{
		return Left.ColumnId < Right.ColumnId;
	});
	// Hash input에서 row order를 canonicalize한 local copy입니다.
	TArray<FCFBatchManifestRow> Rows = Manifest.Rows;
	Rows.Sort([](const FCFBatchManifestRow& Left, const FCFBatchManifestRow& Right)
	{
		return Left.RowId < Right.RowId;
	});
	for (FCFBatchManifestRow& Row : Rows)
	{
		Row.BaselineCells.Sort([](const FCFBatchBaselineCell& Left, const FCFBatchBaselineCell& Right)
		{
			return Left.ColumnId < Right.ColumnId;
		});
	}

	// Immutable export-baseline semantic payload입니다.
	FString Payload;
	CFBatchExportPrivate::AppendToken(Payload, TEXT("Kind"), TEXT("CarFightBatchExportSet"));
	CFBatchExportPrivate::AppendToken(Payload, TEXT("FormatRevision"), TEXT("1"));
	CFBatchExportPrivate::AppendToken(Payload, TEXT("BatchExportId"), Manifest.BatchExportId.ToString(EGuidFormats::Digits));
	CFBatchExportPrivate::AppendToken(Payload, TEXT("DatasetKind"), CFBatchExportPrivate::DatasetToString(Manifest.DatasetKind));
	CFBatchExportPrivate::AppendToken(Payload, TEXT("ProfileDomain"), CFBatchExportPrivate::DomainToString(Manifest.ProfileDomain));
	CFBatchExportPrivate::AppendToken(Payload, TEXT("SchemaId"), Manifest.SchemaId);
	CFBatchExportPrivate::AppendToken(Payload, TEXT("SchemaRevision"), FString::FromInt(Manifest.SchemaRevision));
	CFBatchExportPrivate::AppendToken(Payload, TEXT("ResolverContractRevision"), FString::FromInt(Manifest.ResolverContractRevision));
	CFBatchExportPrivate::AppendToken(Payload, TEXT("CsvColumnCount"), FString::FromInt(Manifest.CsvColumnIds.Num()));
	for (const FString& ColumnId : Manifest.CsvColumnIds)
	{
		CFBatchExportPrivate::AppendToken(Payload, TEXT("CsvColumn"), ColumnId);
	}
	CFBatchExportPrivate::AppendToken(Payload, TEXT("EditableColumnCount"), FString::FromInt(EditableColumns.Num()));
	for (const FCFBatchColumnDescriptor& Descriptor : EditableColumns)
	{
		CFBatchExportPrivate::AppendToken(Payload, TEXT("ColumnId"), Descriptor.ColumnId);
		CFBatchExportPrivate::AppendToken(Payload, TEXT("ValueType"), CFBatchExportPrivate::ValueTypeToString(Descriptor.ValueType));
		CFBatchExportPrivate::AppendToken(Payload, TEXT("Unit"), Descriptor.Unit);
		CFBatchExportPrivate::AppendToken(Payload, TEXT("Access"), CFBatchExportPrivate::AccessToString(Descriptor.Access));
		CFBatchExportPrivate::AppendToken(Payload, TEXT("Owner"), CFBatchExportPrivate::OwnerToString(Descriptor.AuthoringOwner));
		CFBatchExportPrivate::AppendToken(Payload, TEXT("Mutation"), CFBatchExportPrivate::MutationToString(Descriptor.TypedMutationKind));
		CFBatchExportPrivate::AppendToken(Payload, TEXT("Target"), Descriptor.PropertyOrSemanticTarget);
		CFBatchExportPrivate::AppendToken(Payload, TEXT("BlankPolicy"), CFBatchExportPrivate::BlankPolicyToString(Descriptor.BlankPolicy));
		CFBatchExportPrivate::AppendToken(Payload, TEXT("Validation"), Descriptor.ValidationMetadata);
		CFBatchExportPrivate::AppendToken(Payload, TEXT("EditCondition"), Descriptor.EditConditionTarget);
		CFBatchExportPrivate::AppendToken(Payload, TEXT("RequiredCondition"), Descriptor.RequiredEditConditionValue);
	}
	CFBatchExportPrivate::AppendToken(Payload, TEXT("RowCount"), FString::FromInt(Rows.Num()));
	for (const FCFBatchManifestRow& Row : Rows)
	{
		CFBatchExportPrivate::AppendToken(Payload, TEXT("RowId"), Row.RowId);
		CFBatchExportPrivate::AppendToken(Payload, TEXT("TargetPath"), Row.TargetPath);
		CFBatchExportPrivate::AppendToken(Payload, TEXT("RecipePath"), Row.RecipePath);
		CFBatchExportPrivate::AppendToken(Payload, TEXT("ProfilePath"), Row.ProfilePath);
		CFBatchExportPrivate::AppendToken(Payload, TEXT("RowProfileDomain"), CFBatchExportPrivate::DomainToString(Row.ProfileDomain));
		CFBatchExportPrivate::AppendToken(Payload, TEXT("ObjectFingerprint"), Row.ObjectFingerprint);
		CFBatchExportPrivate::AppendToken(Payload, TEXT("TargetDefinitionHash"), Row.TargetDefinitionHash);
		CFBatchExportPrivate::AppendToken(Payload, TEXT("BaselineCellCount"), FString::FromInt(Row.BaselineCells.Num()));
		for (const FCFBatchBaselineCell& Cell : Row.BaselineCells)
		{
			CFBatchExportPrivate::AppendToken(Payload, TEXT("BaselineColumn"), Cell.ColumnId);
			CFBatchExportPrivate::AppendToken(Payload, TEXT("BaselineValue"), Cell.CanonicalValue);
			CFBatchExportPrivate::AppendToken(Payload, TEXT("EditableAtExport"), Cell.bEditableAtExport ? TEXT("1") : TEXT("0"));
			CFBatchExportPrivate::AppendToken(Payload, TEXT("OwnershipSourceMode"), Cell.OwnershipSourceMode);
		}
	}
	return CFBatchExportPrivate::HashUtf8Payload(Payload);
}
