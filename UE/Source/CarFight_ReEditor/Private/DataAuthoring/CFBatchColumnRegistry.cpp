// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchColumnRegistry.cpp
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08J Batch Column Registry projection 구현입니다.
// Scope: Existing Recipe/Profile reflection과 FCFVehicleFieldRegistry 117 descriptor를 stable CSV column metadata로 투영합니다.
// Changelog:
// - v1.0.0: Reserved metadata, Recipe numeric allowlist, 5 Profile numeric leaf projection, Resolved 117 projection 최초 구현.
// Migration:
// - numeric 값/default/source precedence를 이 Registry에 복제하지 않습니다.
// - Runtime UCFVehicleData는 read-only Reflection source로만 사용합니다.

#include "DataAuthoring/CFBatchColumnRegistry.h"

#include "CFVehicleData.h"
#include "DataAuthoring/CFVehicleAuthoringTypes.h"
#include "DataAuthoring/CFVehicleFieldRegistry.h"
#include "DataAuthoring/CFVehicleProfileTypes.h"
#include "UObject/UnrealType.h"

namespace CFBatchColumnRegistryPrivate
{
	/** Dataset column 목록에 추가할 descriptor seed입니다. */
	struct FColumnSeed
	{
		// Stable technical ColumnId입니다.
		const TCHAR* ColumnId;

		// 사용자 표시 label입니다.
		const TCHAR* DisplayLabel;

		// Canonical value type입니다.
		ECFBatchValueType ValueType;

		// ReadOnly / Editable access입니다.
		ECFBatchColumnAccess Access;

		// Authoring owner입니다.
		ECFBatchAuthoringOwner Owner;

		// Typed mutation kind입니다.
		ECFBatchMutationKind MutationKind;

		// Reflection property 또는 semantic target입니다.
		const TCHAR* Target;

		// Reserved metadata 여부입니다.
		bool bReserved;
	};

	// Stable technical Profile Domain token을 반환합니다.
	FString GetDomainToken(const ECFVehicleProfileDomain Domain)
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

	// Dataset Kind stable technical token을 반환합니다.
	FString GetDatasetToken(const ECFBatchDatasetKind DatasetKind)
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

	// Reflection property를 canonical Batch value type으로 분류합니다.
	ECFBatchValueType GetValueType(const FProperty& Property)
	{
		if (CastField<FBoolProperty>(&Property))
		{
			return ECFBatchValueType::Boolean;
		}
		if (const FNumericProperty* NumericProperty = CastField<FNumericProperty>(&Property))
		{
			return NumericProperty->IsInteger()
				? ECFBatchValueType::Integer
				: ECFBatchValueType::Number;
		}
		if (CastField<FStrProperty>(&Property) || CastField<FNameProperty>(&Property) || CastField<FTextProperty>(&Property))
		{
			return ECFBatchValueType::String;
		}
				return ECFBatchValueType::CanonicalText;
	}

	// Enum/Bool을 배제한 plain scalar integer/float numeric property인지 검사합니다.
	bool IsPlainNumericProperty(const FProperty& Property)
	{
		if (CastField<FBoolProperty>(&Property) || CastField<FEnumProperty>(&Property))
		{
			return false;
		}
		if (const FByteProperty* ByteProperty = CastField<FByteProperty>(&Property))
		{
			if (ByteProperty->Enum)
			{
				return false;
			}
		}
		return CastField<FNumericProperty>(&Property) != nullptr;
	}

	// Property metadata의 unit을 numeric cell과 분리된 descriptor text로 반환합니다.
	FString GetUnit(const FProperty& Property)
	{
		return Property.HasMetaData(TEXT("Units"))
			? Property.GetMetaData(TEXT("Units"))
			: FString();
	}

	// Clamp metadata를 stable validation text로 projection합니다.
	FString GetValidationMetadata(const FProperty& Property)
	{
		// Validation metadata를 stable key=value token으로 누적할 문자열입니다.
		TArray<FString> Tokens;
		if (Property.HasMetaData(TEXT("ClampMin")))
		{
			Tokens.Add(TEXT("ClampMin=") + Property.GetMetaData(TEXT("ClampMin")));
		}
		if (Property.HasMetaData(TEXT("ClampMax")))
		{
			Tokens.Add(TEXT("ClampMax=") + Property.GetMetaData(TEXT("ClampMax")));
		}
		return FString::Join(Tokens, TEXT(";"));
	}

	// Seed 하나를 concrete descriptor로 추가합니다.
	void AddSeedDescriptor(
		const FColumnSeed& Seed,
		const ECFBatchDatasetKind DatasetKind,
		const ECFVehicleProfileDomain ProfileDomain,
		TArray<FCFBatchColumnDescriptor>& OutColumns)
	{
		// Seed에서 생성할 Batch column descriptor입니다.
		FCFBatchColumnDescriptor Descriptor;
		Descriptor.ColumnId = Seed.ColumnId;
		Descriptor.DatasetKind = DatasetKind;
		Descriptor.ProfileDomain = ProfileDomain;
		Descriptor.DisplayLabel = FText::FromString(Seed.DisplayLabel);
		Descriptor.ValueType = Seed.ValueType;
		Descriptor.Access = Seed.Access;
		Descriptor.AuthoringOwner = Seed.Owner;
		Descriptor.TypedMutationKind = Seed.MutationKind;
		Descriptor.PropertyOrSemanticTarget = Seed.Target;
		Descriptor.BlankPolicy = Seed.Access == ECFBatchColumnAccess::Editable
			? ECFBatchBlankPolicy::NoChange
			: ECFBatchBlankPolicy::NotApplicable;
		Descriptor.bReservedMetadata = Seed.bReserved;
		OutColumns.Add(MoveTemp(Descriptor));
	}

	// Dataset 공용 identity/schema metadata column을 추가합니다.
	void AppendCommonReservedColumns(
		const ECFBatchDatasetKind DatasetKind,
		const ECFVehicleProfileDomain ProfileDomain,
		TArray<FCFBatchColumnDescriptor>& OutColumns)
	{
		// 모든 CSV row의 stable row identity입니다.
		static const FColumnSeed CommonSeeds[] =
		{
			{TEXT("__cf_row_id"), TEXT("행 ID"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::Metadata, ECFBatchMutationKind::None, TEXT("Metadata.RowId"), true},
			{TEXT("__cf_schema_id"), TEXT("스키마 ID"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::Metadata, ECFBatchMutationKind::None, TEXT("Metadata.SchemaId"), true},
			{TEXT("__cf_schema_revision"), TEXT("스키마 Revision"), ECFBatchValueType::Integer, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::Metadata, ECFBatchMutationKind::None, TEXT("Metadata.SchemaRevision"), true}
		};
		for (const FColumnSeed& Seed : CommonSeeds)
		{
			AddSeedDescriptor(Seed, DatasetKind, ProfileDomain, OutColumns);
		}
	}

	// Recipe/Vehicle dataset reserved identity/fingerprint columns을 추가합니다.
	void AppendRecipeReservedColumns(
		const ECFBatchDatasetKind DatasetKind,
		TArray<FCFBatchColumnDescriptor>& OutColumns)
	{
		AppendCommonReservedColumns(DatasetKind, ECFVehicleProfileDomain::None, OutColumns);
		// Section 26.25 Recipe row의 remaining reserved metadata입니다.
		static const FColumnSeed RecipeSeeds[] =
		{
			{TEXT("__cf_target"), TEXT("대상 VehicleData"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::Metadata, ECFBatchMutationKind::None, TEXT("Metadata.TargetPath"), true},
			{TEXT("__cf_recipe"), TEXT("Recipe"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::Metadata, ECFBatchMutationKind::None, TEXT("Metadata.RecipePath"), true},
			{TEXT("__cf_export_recipe_fingerprint"), TEXT("Export Recipe Fingerprint"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::Metadata, ECFBatchMutationKind::None, TEXT("Metadata.RecipeFingerprint"), true},
			{TEXT("__cf_export_target_hash"), TEXT("Export Target Hash"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::Metadata, ECFBatchMutationKind::None, TEXT("Metadata.TargetDefinitionHash"), true}
		};
		for (const FColumnSeed& Seed : RecipeSeeds)
		{
			AddSeedDescriptor(Seed, DatasetKind, ECFVehicleProfileDomain::None, OutColumns);
		}
	}

	// Profile dataset Section 26.25 reserved identity/fingerprint columns을 추가합니다.
	void AppendProfileReservedColumns(
		const ECFVehicleProfileDomain ProfileDomain,
		TArray<FCFBatchColumnDescriptor>& OutColumns)
	{
		AppendCommonReservedColumns(ECFBatchDatasetKind::ProfileNumericEdit, ProfileDomain, OutColumns);
		// Profile edit row의 remaining reserved metadata입니다.
		static const FColumnSeed ProfileSeeds[] =
		{
			{TEXT("__cf_profile"), TEXT("Profile"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::Metadata, ECFBatchMutationKind::None, TEXT("Metadata.ProfilePath"), true},
			{TEXT("__cf_profile_domain"), TEXT("Profile Domain"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::Metadata, ECFBatchMutationKind::None, TEXT("Metadata.ProfileDomain"), true},
			{TEXT("__cf_export_profile_fingerprint"), TEXT("Export Profile Fingerprint"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::Metadata, ECFBatchMutationKind::None, TEXT("Metadata.ProfileFingerprint"), true}
		};
		for (const FColumnSeed& Seed : ProfileSeeds)
		{
			AddSeedDescriptor(Seed, ECFBatchDatasetKind::ProfileNumericEdit, ProfileDomain, OutColumns);
		}
	}

	// Recipe USTRUCT 하나의 direct numeric semantic leaf를 descriptor로 projection합니다.
	void AppendRecipeNumericStruct(
		const UScriptStruct& Struct,
		const FString& ColumnPrefix,
		const FString& PropertyPrefix,
		const ECFBatchMutationKind MutationKind,
		TArray<FCFBatchColumnDescriptor>& OutColumns)
	{
		for (TFieldIterator<FProperty> PropertyIt(&Struct); PropertyIt; ++PropertyIt)
		{
			// 이 USTRUCT가 직접 선언한 property만 projection합니다.
			FProperty* Property = *PropertyIt;
			if (!Property || Property->GetOwnerStruct() != &Struct || CastField<FBoolProperty>(Property))
			{
				continue;
			}
						// Recipe P0 allowlist에는 enum/bool을 제외한 scalar integer/float numeric leaf만 들어갑니다.
			if (!IsPlainNumericProperty(*Property))
			{
				continue;
			}

			// Reflection schema에서 projection할 Recipe numeric descriptor입니다.
			FCFBatchColumnDescriptor Descriptor;
			Descriptor.ColumnId = ColumnPrefix + TEXT(".") + Property->GetName();
			Descriptor.DatasetKind = ECFBatchDatasetKind::RecipeNumericEdit;
			Descriptor.DisplayLabel = Property->GetDisplayNameText();
			Descriptor.ValueType = GetValueType(*Property);
			Descriptor.Unit = GetUnit(*Property);
			Descriptor.Access = ECFBatchColumnAccess::Editable;
			Descriptor.AuthoringOwner = ECFBatchAuthoringOwner::Recipe;
			Descriptor.TypedMutationKind = MutationKind;
			Descriptor.PropertyOrSemanticTarget = PropertyPrefix + TEXT(".") + Property->GetName();
			Descriptor.BlankPolicy = ECFBatchBlankPolicy::NoChange;
			Descriptor.ValidationMetadata = GetValidationMetadata(*Property);
			OutColumns.Add(MoveTemp(Descriptor));
		}
	}

	// Recipe Mass/Durability value descriptor에 source-mode ownership guard를 projection합니다.
	void ApplyRecipeEditConditions(TArray<FCFBatchColumnDescriptor>& InOutColumns)
	{
		for (FCFBatchColumnDescriptor& Descriptor : InOutColumns)
		{
			if (Descriptor.ColumnId == TEXT("Recipe.Mass.ExplicitBaseMassKg"))
			{
				Descriptor.EditConditionTarget = TEXT("MassIntent.BaseMassMode");
				Descriptor.RequiredEditConditionValue = TEXT("ExplicitValue");
			}
			else if (Descriptor.ColumnId == TEXT("Recipe.Mass.ExplicitGrossMassKg"))
			{
				Descriptor.EditConditionTarget = TEXT("MassIntent.GrossMassMode");
				Descriptor.RequiredEditConditionValue = TEXT("ExplicitValue");
			}
			else if (Descriptor.ColumnId == TEXT("Recipe.Durability.ExplicitMaxHealth"))
			{
				Descriptor.EditConditionTarget = TEXT("DurabilityIntent.MaxHealthMode");
				Descriptor.RequiredEditConditionValue = TEXT("ExplicitValue");
			}
		}
	}

	// Profile Domain의 typed payload UStruct를 반환합니다.
	const UScriptStruct* GetProfileDataStruct(const ECFVehicleProfileDomain Domain)
	{
		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			return FCFVehicleBaseProfileData::StaticStruct();
		case ECFVehicleProfileDomain::Drivetrain:
			return FCFDrivetrainProfileData::StaticStruct();
		case ECFVehicleProfileDomain::Handling:
			return FCFHandlingProfileData::StaticStruct();
		case ECFVehicleProfileDomain::Performance:
			return FCFPerformanceProfileData::StaticStruct();
		case ECFVehicleProfileDomain::DriveState:
			return FCFDriveStateProfileData::StaticStruct();
		default:
			return nullptr;
		}
	}

	// Profile typed payload의 nested independent scalar numeric leaf를 재귀 projection합니다.
	void AppendProfileNumericLeaves(
		const UStruct& Struct,
		const ECFVehicleProfileDomain Domain,
		const FString& RelativePrefix,
		TArray<FCFBatchColumnDescriptor>& OutColumns)
	{
		for (TFieldIterator<FProperty> PropertyIt(&Struct); PropertyIt; ++PropertyIt)
		{
			// 현재 typed payload struct가 직접 선언한 property만 처리합니다.
			FProperty* Property = *PropertyIt;
			if (!Property || Property->GetOwnerStruct() != &Struct)
			{
				continue;
			}

			// Data 아래 stable property target path입니다.
			const FString RelativePath = RelativePrefix.IsEmpty()
				? Property->GetName()
				: RelativePrefix + TEXT(".") + Property->GetName();

			if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				// P0 Profile schema의 plain composition struct 내부 numeric leaf만 flatten합니다.
				if (StructProperty->Struct == FCFFeelResponse::StaticStruct()
					|| StructProperty->Struct == FCFMassScaleRule::StaticStruct())
				{
					AppendProfileNumericLeaves(*StructProperty->Struct, Domain, RelativePath, OutColumns);
				}
				continue;
			}

			if (CastField<FBoolProperty>(Property))
			{
				continue;
			}
						// Enum/Object/Class/Array가 아닌 scalar numeric leaf만 Profile batch editable입니다.
			if (!IsPlainNumericProperty(*Property))
			{
				continue;
			}

			// Typed Profile schema에서 projection한 editable numeric descriptor입니다.
			FCFBatchColumnDescriptor Descriptor;
			Descriptor.ColumnId = TEXT("Profile.") + GetDomainToken(Domain) + TEXT(".") + RelativePath;
			Descriptor.DatasetKind = ECFBatchDatasetKind::ProfileNumericEdit;
			Descriptor.ProfileDomain = Domain;
			Descriptor.DisplayLabel = Property->GetDisplayNameText();
			Descriptor.ValueType = GetValueType(*Property);
			Descriptor.Unit = GetUnit(*Property);
			Descriptor.Access = ECFBatchColumnAccess::Editable;
			Descriptor.AuthoringOwner = ECFBatchAuthoringOwner::Profile;
			Descriptor.TypedMutationKind = ECFBatchMutationKind::ProfileNumericLeaf;
			Descriptor.PropertyOrSemanticTarget = TEXT("Data.") + RelativePath;
			Descriptor.BlankPolicy = ECFBatchBlankPolicy::NoChange;
			Descriptor.ValidationMetadata = GetValidationMetadata(*Property);
			OutColumns.Add(MoveTemp(Descriptor));
		}
	}

	// Stable Field Registry pattern이 가리키는 actual reflected leaf property를 찾습니다.
	const FProperty* ResolveDefinitionPatternProperty(const FCFVehicleFieldPath& StablePathPattern)
	{
		// Scalar path 또는 array element struct를 순회할 current reflected struct입니다.
		const UStruct* CurrentStruct = UCFVehicleData::StaticClass();
		if (!StablePathPattern.CollectionPropertyName.IsNone())
		{
			// UCFVehicleData top-level stable array property입니다.
			const FArrayProperty* ArrayProperty = FindFProperty<FArrayProperty>(CurrentStruct, StablePathPattern.CollectionPropertyName);
			if (!ArrayProperty)
			{
				return nullptr;
			}
			// Stable array element struct property입니다.
			const FStructProperty* InnerStruct = CastField<FStructProperty>(ArrayProperty->Inner);
			if (!InnerStruct || !InnerStruct->Struct)
			{
				return nullptr;
			}
			CurrentStruct = InnerStruct->Struct;
		}

		// Leaf까지 이어지는 reflected property입니다.
		const FProperty* LastProperty = nullptr;
		for (const FName PropertyName : StablePathPattern.PropertyChain)
		{
			LastProperty = FindFProperty<FProperty>(CurrentStruct, PropertyName);
			if (!LastProperty)
			{
				return nullptr;
			}
			if (const FStructProperty* StructProperty = CastField<FStructProperty>(LastProperty))
			{
				CurrentStruct = StructProperty->Struct;
			}
		}
		return LastProperty;
	}

	// Read-only long-form ResolvedFieldReport columns을 추가합니다.
	void AppendResolvedReportColumns(TArray<FCFBatchColumnDescriptor>& OutColumns)
	{
		AppendRecipeReservedColumns(ECFBatchDatasetKind::ResolvedFieldReport, OutColumns);
		// Section 26.8 long-form report fields입니다.
		static const FColumnSeed ReportSeeds[] =
		{
			{TEXT("Resolved.StableFieldPath"), TEXT("Stable Field Path"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::ResolvedFieldRegistry, ECFBatchMutationKind::None, TEXT("Resolved.FieldPath"), false},
			{TEXT("Resolved.CurrentTargetValue"), TEXT("현재 Target 값"), ECFBatchValueType::CanonicalText, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::ResolvedFieldRegistry, ECFBatchMutationKind::None, TEXT("Resolved.CurrentTargetValue"), false},
			{TEXT("Resolved.ResolvedValue"), TEXT("Resolve 값"), ECFBatchValueType::CanonicalText, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::ResolvedFieldRegistry, ECFBatchMutationKind::None, TEXT("Resolved.ResolvedValue"), false},
			{TEXT("Resolved.EffectiveSourceType"), TEXT("유효 Source Type"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::ResolvedFieldRegistry, ECFBatchMutationKind::None, TEXT("Resolved.SourceType"), false},
			{TEXT("Resolved.EffectiveSourceId"), TEXT("유효 Source ID"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::ResolvedFieldRegistry, ECFBatchMutationKind::None, TEXT("Resolved.SourceId"), false},
			{TEXT("Resolved.DiffStatus"), TEXT("Diff 상태"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::ResolvedFieldRegistry, ECFBatchMutationKind::None, TEXT("Resolved.DiffStatus"), false},
			{TEXT("Resolved.ValidationStatus"), TEXT("Validation 상태"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::ResolvedFieldRegistry, ECFBatchMutationKind::None, TEXT("Resolved.ValidationStatus"), false}
		};
		for (const FColumnSeed& Seed : ReportSeeds)
		{
			AddSeedDescriptor(Seed, ECFBatchDatasetKind::ResolvedFieldReport, ECFVehicleProfileDomain::None, OutColumns);
		}
	}

	// Read-only VehicleSummaryReport의 bounded foundation columns을 추가합니다.
	void AppendVehicleSummaryColumns(TArray<FCFBatchColumnDescriptor>& OutColumns)
	{
		AppendRecipeReservedColumns(ECFBatchDatasetKind::VehicleSummaryReport, OutColumns);
		// Summary가 새 source truth가 되지 않도록 context/read-only 값만 둡니다.
		static const FColumnSeed SummarySeeds[] =
		{
			{TEXT("Summary.ManagementState"), TEXT("관리 상태"), ECFBatchValueType::String, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::Recipe, ECFBatchMutationKind::None, TEXT("Recipe.ImportState.ManageState"), false},
			{TEXT("Summary.AdvancedOverrideCount"), TEXT("고급 Override 수"), ECFBatchValueType::Integer, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::Recipe, ECFBatchMutationKind::None, TEXT("Recipe.AdvancedOverrides.Num"), false},
			{TEXT("Summary.LegacyPinCount"), TEXT("Legacy Pin 수"), ECFBatchValueType::Integer, ECFBatchColumnAccess::ReadOnly, ECFBatchAuthoringOwner::Recipe, ECFBatchMutationKind::None, TEXT("Recipe.ImportState.LegacyPinnedFields.Num"), false}
		};
		for (const FColumnSeed& Seed : SummarySeeds)
		{
			AddSeedDescriptor(Seed, ECFBatchDatasetKind::VehicleSummaryReport, ECFVehicleProfileDomain::None, OutColumns);
		}
	}
}

// Dataset/Profile Domain의 canonical CSV column descriptor를 stable ColumnId 순서로 반환합니다.
bool FCFBatchColumnRegistry::GetDatasetColumns(
	const ECFBatchDatasetKind DatasetKind,
	const ECFVehicleProfileDomain ProfileDomain,
	TArray<FCFBatchColumnDescriptor>& OutColumns,
	TArray<FString>& OutErrors)
{
	OutColumns.Reset();
	OutErrors.Reset();

	switch (DatasetKind)
	{
	case ECFBatchDatasetKind::VehicleSummaryReport:
		CFBatchColumnRegistryPrivate::AppendVehicleSummaryColumns(OutColumns);
		break;

	case ECFBatchDatasetKind::ResolvedFieldReport:
		CFBatchColumnRegistryPrivate::AppendResolvedReportColumns(OutColumns);
		break;

	case ECFBatchDatasetKind::RecipeNumericEdit:
		CFBatchColumnRegistryPrivate::AppendRecipeReservedColumns(DatasetKind, OutColumns);
		CFBatchColumnRegistryPrivate::AppendRecipeNumericStruct(
			*FCFVehicleFeelIntent::StaticStruct(),
			TEXT("Recipe.DrivingFeel"),
			TEXT("DrivingFeelIntent"),
			ECFBatchMutationKind::RecipeDrivingFeelAxis,
			OutColumns);
		CFBatchColumnRegistryPrivate::AppendRecipeNumericStruct(
			*FCFVehicleMassIntent::StaticStruct(),
			TEXT("Recipe.Mass"),
			TEXT("MassIntent"),
			ECFBatchMutationKind::RecipeMassExplicitValue,
			OutColumns);
		CFBatchColumnRegistryPrivate::AppendRecipeNumericStruct(
			*FCFVehicleDurabilityIntent::StaticStruct(),
			TEXT("Recipe.Durability"),
			TEXT("DurabilityIntent"),
			ECFBatchMutationKind::RecipeDurabilityExplicitValue,
			OutColumns);
		CFBatchColumnRegistryPrivate::ApplyRecipeEditConditions(OutColumns);
		break;

	case ECFBatchDatasetKind::ProfileNumericEdit:
	{
		// ProfileNumericEdit는 한 file에 exact one Domain만 허용합니다.
		const UScriptStruct* ProfileDataStruct = CFBatchColumnRegistryPrivate::GetProfileDataStruct(ProfileDomain);
		if (!ProfileDataStruct)
		{
			OutErrors.Add(TEXT("ProfileNumericEdit에는 VehicleBase/Drivetrain/Handling/Performance/DriveState exact Domain이 필요합니다."));
			return false;
		}
		CFBatchColumnRegistryPrivate::AppendProfileReservedColumns(ProfileDomain, OutColumns);
		CFBatchColumnRegistryPrivate::AppendProfileNumericLeaves(*ProfileDataStruct, ProfileDomain, FString(), OutColumns);
		break;
	}

	default:
		OutErrors.Add(TEXT("지원하지 않는 Batch Dataset Kind입니다."));
		return false;
	}

	OutColumns.Sort([](const FCFBatchColumnDescriptor& Left, const FCFBatchColumnDescriptor& Right)
	{
		return Left.ColumnId < Right.ColumnId;
	});
	return ValidateDescriptorSet(OutColumns, OutErrors);
}

// Existing FCFVehicleFieldRegistry 117 descriptors를 read-only report metadata로 projection합니다.
bool FCFBatchColumnRegistry::GetResolvedFieldProjections(
	TArray<FCFBatchResolvedProjection>& OutProjections,
	TArray<FString>& OutErrors)
{
	OutProjections.Reset();
	OutErrors.Reset();

	// Frozen 117 Field Registry authority입니다.
	const TArray<FCFVehicleFieldDescriptor>& FieldDescriptors = FCFVehicleFieldRegistry::GetDescriptors();
	OutProjections.Reserve(FieldDescriptors.Num());
	for (const FCFVehicleFieldDescriptor& FieldDescriptor : FieldDescriptors)
	{
		// Existing Registry path가 가리키는 reflected Runtime leaf입니다.
		const FProperty* Property = CFBatchColumnRegistryPrivate::ResolveDefinitionPatternProperty(FieldDescriptor.StablePathPattern);
		if (!Property)
		{
			OutErrors.Add(FString::Printf(TEXT("Resolved projection property를 찾지 못했습니다: %s"), *FieldDescriptor.GetCanonicalPattern()));
			continue;
		}

		// Field Registry descriptor에서 read-only report metadata로 projection할 row입니다.
		FCFBatchResolvedProjection Projection;
		Projection.StableFieldPattern = FieldDescriptor.GetCanonicalPattern();
		Projection.ValueType = CFBatchColumnRegistryPrivate::GetValueType(*Property);
		Projection.Unit = CFBatchColumnRegistryPrivate::GetUnit(*Property);
		Projection.PrimaryProfileDomain = FieldDescriptor.PrimaryProfileDomain;
		Projection.ResolveRule = FieldDescriptor.ResolveRule;
		Projection.AdoptionGroup = FieldDescriptor.AdoptionGroup;
		Projection.bIdentityField = FieldDescriptor.bIdentityField;
		Projection.bLegacySerialized = FieldDescriptor.bLegacySerialized;
		OutProjections.Add(MoveTemp(Projection));
	}

	OutProjections.Sort([](const FCFBatchResolvedProjection& Left, const FCFBatchResolvedProjection& Right)
	{
		return Left.StableFieldPattern < Right.StableFieldPattern;
	});

	if (OutProjections.Num() != FCFVehicleFieldRegistry::ExpectedLeafPatternCount)
	{
		OutErrors.Add(FString::Printf(
			TEXT("Resolved report projection count mismatch. Expected=%d Actual=%d"),
			FCFVehicleFieldRegistry::ExpectedLeafPatternCount,
			OutProjections.Num()));
	}
	return OutErrors.IsEmpty();
}

// Dataset/Profile Domain의 stable schema id를 반환합니다.
FString FCFBatchColumnRegistry::GetSchemaId(
	const ECFBatchDatasetKind DatasetKind,
	const ECFVehicleProfileDomain ProfileDomain)
{
	const FString DatasetToken = CFBatchColumnRegistryPrivate::GetDatasetToken(DatasetKind);
	if (DatasetKind == ECFBatchDatasetKind::ProfileNumericEdit)
	{
		return TEXT("CarFight.Batch.") + DatasetToken + TEXT(".") + CFBatchColumnRegistryPrivate::GetDomainToken(ProfileDomain);
	}
	return TEXT("CarFight.Batch.") + DatasetToken;
}

// __cf_ importer-owned reserved technical prefix인지 검사합니다.
bool FCFBatchColumnRegistry::IsReservedColumnId(const FString& ColumnId)
{
	return ColumnId.StartsWith(TEXT("__cf_"), ESearchCase::CaseSensitive);
}

// Duplicate/reserved collision/editability invariant를 검사합니다.
bool FCFBatchColumnRegistry::ValidateDescriptorSet(
	const TArray<FCFBatchColumnDescriptor>& Columns,
	TArray<FString>& OutErrors)
{
	OutErrors.Reset();
	// Duplicate stable ColumnId를 탐지할 set입니다.
	TSet<FString> SeenColumnIds;
	for (const FCFBatchColumnDescriptor& Descriptor : Columns)
	{
		if (Descriptor.ColumnId.TrimStartAndEnd().IsEmpty())
		{
			OutErrors.Add(TEXT("Batch ColumnId가 비어 있습니다."));
			continue;
		}
		if (SeenColumnIds.Contains(Descriptor.ColumnId))
		{
			OutErrors.Add(FString::Printf(TEXT("Duplicate Batch ColumnId: %s"), *Descriptor.ColumnId));
			continue;
		}
		SeenColumnIds.Add(Descriptor.ColumnId);

		// __cf_ prefix는 importer-owned metadata 외 descriptor가 사용할 수 없습니다.
		const bool bHasReservedPrefix = IsReservedColumnId(Descriptor.ColumnId);
		if (bHasReservedPrefix != Descriptor.bReservedMetadata)
		{
			OutErrors.Add(FString::Printf(TEXT("Reserved __cf_ prefix ownership mismatch: %s"), *Descriptor.ColumnId));
		}
		if (Descriptor.bReservedMetadata)
		{
			if (Descriptor.Access != ECFBatchColumnAccess::ReadOnly
				|| Descriptor.AuthoringOwner != ECFBatchAuthoringOwner::Metadata
				|| Descriptor.TypedMutationKind != ECFBatchMutationKind::None)
			{
				OutErrors.Add(FString::Printf(TEXT("Reserved metadata column은 read-only Metadata/None이어야 합니다: %s"), *Descriptor.ColumnId));
			}
		}

		if (Descriptor.Access == ECFBatchColumnAccess::Editable)
		{
			if (Descriptor.bReservedMetadata)
			{
				OutErrors.Add(FString::Printf(TEXT("Reserved metadata column은 editable일 수 없습니다: %s"), *Descriptor.ColumnId));
			}
			if (Descriptor.TypedMutationKind == ECFBatchMutationKind::None)
			{
				OutErrors.Add(FString::Printf(TEXT("Editable column에는 typed mutation metadata가 필요합니다: %s"), *Descriptor.ColumnId));
			}
			if (Descriptor.BlankPolicy != ECFBatchBlankPolicy::NoChange)
			{
				OutErrors.Add(FString::Printf(TEXT("Editable blank policy는 NoChange여야 합니다: %s"), *Descriptor.ColumnId));
			}
		}
		else if (Descriptor.TypedMutationKind != ECFBatchMutationKind::None)
		{
			OutErrors.Add(FString::Printf(TEXT("Read-only column에는 mutation kind를 둘 수 없습니다: %s"), *Descriptor.ColumnId));
		}
	}
	return OutErrors.IsEmpty();
}
