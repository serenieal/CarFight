// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleMaterializer.cpp
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08F transient Definition Materializer와 기존 UCFVDAValidator bridge 구현입니다.
// Scope: Stable-ID array reconstruction, checked leaf import, resolved projection readback/hash, Definition Validation을 제공합니다.
// Changelog:
// - v1.0.0: Frozen Section 22.32/22.38 R15 Materializer / Validation Foundation 최초 구현.
// Migration:
// - Runtime UCFVehicleData schema, 기존 UCFVDAValidator, Content Asset, Apply 경로는 변경하지 않습니다.
// - Legacy Serialized leaf는 SortedResolvedFields에 존재할 때만 import하며 새 Authoring 입력으로 생성하지 않습니다.

#include "DataAuthoring/CFVehicleMaterializer.h"

#include "CFVDAValidator.h"
#include "CFVehicleData.h"
#include "DataAuthoring/CFVehicleFieldCodec.h"
#include "DataAuthoring/CFVehicleFieldRegistry.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

namespace CFVehicleMaterializerPrivate
{
	/** Stable-ID collection 하나의 transient storage와 selector→index mapping입니다. */
	struct FStableCollectionPlan
	{
		// UCFVehicleData의 top-level TArray property입니다.
		const FArrayProperty* ArrayProperty = nullptr;

		// TArray element struct property입니다.
		const FStructProperty* InnerStructProperty = nullptr;

		// Element identity를 저장하는 FName stable selector property입니다.
		const FNameProperty* SelectorProperty = nullptr;

		// Transient validation candidate에 생성한 deterministic selector 순서입니다.
		TArray<FName> OrderedSelectors;

		// Exact selector에서 physical transient array index를 찾는 mapping입니다.
		TMap<FName, int32> SelectorToIndex;
	};

	// Exact resolved field path가 collection element identity leaf인지 판정합니다.
	bool IsIdentityField(const FCFVehicleFieldPath& FieldPath)
	{
		return !FieldPath.CollectionPropertyName.IsNone()
			&& !FieldPath.SelectorKeyPropertyName.IsNone()
			&& FieldPath.PropertyChain.Num() == 1
			&& FieldPath.PropertyChain[0] == FieldPath.SelectorKeyPropertyName;
	}

	// Resolver output이 canonical exact path 오름차순이며 duplicate가 없는지 fail-closed 검사합니다.
	bool ValidateResolvedFieldOrdering(
		const TArray<FCFVehicleResolvedField>& SortedResolvedFields,
		FString& OutError)
	{
		// 직전 field의 canonical exact path입니다.
		FString PreviousPath;
		for (const FCFVehicleResolvedField& ResolvedField : SortedResolvedFields)
		{
			// 현재 field의 canonical exact path입니다.
			const FString CurrentPath = ResolvedField.FieldPath.ToCanonicalString(true);
			if (CurrentPath.IsEmpty())
			{
				OutError = TEXT("Resolved field에 비어 있는 Stable Field Path가 있습니다.");
				return false;
			}

			if (!PreviousPath.IsEmpty() && !(PreviousPath < CurrentPath))
			{
				OutError = FString::Printf(
					TEXT("SortedResolvedFields가 strict canonical order가 아니거나 duplicate입니다. Previous=%s Current=%s"),
					*PreviousPath,
					*CurrentPath);
				return false;
			}
			PreviousPath = CurrentPath;
		}

		OutError.Reset();
		return true;
	}

	// Registry가 선언한 Stable-ID collection 이름을 lexical order로 수집합니다.
	TArray<FName> GetStableCollectionNames()
	{
		// 중복 collection 이름을 제거할 집합입니다.
		TSet<FName> CollectionNameSet;
		for (const FCFVehicleFieldDescriptor& Descriptor : FCFVehicleFieldRegistry::GetDescriptors())
		{
			if (!Descriptor.StablePathPattern.CollectionPropertyName.IsNone())
			{
				CollectionNameSet.Add(Descriptor.StablePathPattern.CollectionPropertyName);
			}
		}

		// Deterministic materialization 순서로 변환할 collection 이름 목록입니다.
		TArray<FName> CollectionNames;
		CollectionNames.Reserve(CollectionNameSet.Num());
		for (const FName CollectionName : CollectionNameSet)
		{
			CollectionNames.Add(CollectionName);
		}
		CollectionNames.Sort([](const FName Left, const FName Right)
		{
			return Left.LexicalLess(Right);
		});
		return CollectionNames;
	}

	// Collection/selector에 대응하는 identity leaf가 Resolver output에 실제 존재하는지 확인합니다.
	bool HasIdentityResolvedField(
		const TArray<FCFVehicleResolvedField>& SortedResolvedFields,
		const FName CollectionPropertyName,
		const FName SelectorKeyPropertyName,
		const FName SelectorValue)
	{
		return SortedResolvedFields.ContainsByPredicate(
			[CollectionPropertyName, SelectorKeyPropertyName, SelectorValue](const FCFVehicleResolvedField& ResolvedField)
			{
				return ResolvedField.FieldPath.CollectionPropertyName == CollectionPropertyName
					&& ResolvedField.FieldPath.SelectorKeyPropertyName == SelectorKeyPropertyName
					&& ResolvedField.FieldPath.SelectorKeyValue == SelectorValue
					&& IsIdentityField(ResolvedField.FieldPath);
			});
	}

	// Stable-ID arrays를 비우고 Resolver exact selector 집합만 lexical order로 transient candidate에 생성합니다.
	bool PrepareStableCollections(
		UCFVehicleData& Candidate,
		const TArray<FCFVehicleResolvedField>& SortedResolvedFields,
		TMap<FName, FStableCollectionPlan>& OutCollectionPlans,
		FString& OutError)
	{
		OutCollectionPlans.Reset();

		// Registry에 존재하는 Stable-ID collection 이름 목록입니다.
		const TArray<FName> CollectionNames = GetStableCollectionNames();
		for (const FName CollectionName : CollectionNames)
		{
			// Candidate top-level Stable-ID array property입니다.
			const FArrayProperty* ArrayProperty = FindFProperty<FArrayProperty>(Candidate.GetClass(), CollectionName);
			// Stable-ID array element struct schema입니다.
			const FStructProperty* InnerStructProperty = ArrayProperty ? CastField<FStructProperty>(ArrayProperty->Inner) : nullptr;
			if (!ArrayProperty || !InnerStructProperty || !InnerStructProperty->Struct)
			{
				OutError = FString::Printf(TEXT("Stable-ID collection schema가 유효하지 않습니다: %s"), *CollectionName.ToString());
				return false;
			}

			// 이 collection이 사용하는 stable selector property 이름입니다.
			FName SelectorKeyPropertyName = NAME_None;
			// Resolver output에 등장한 exact selector 집합입니다.
			TSet<FName> SelectorSet;
			for (const FCFVehicleResolvedField& ResolvedField : SortedResolvedFields)
			{
				if (ResolvedField.FieldPath.CollectionPropertyName != CollectionName)
				{
					continue;
				}

				if (ResolvedField.FieldPath.SelectorKeyPropertyName.IsNone() || ResolvedField.FieldPath.SelectorKeyValue.IsNone())
				{
					OutError = FString::Printf(TEXT("Materializer array field에 exact stable selector가 없습니다: %s"), *ResolvedField.FieldPath.ToCanonicalString(true));
					return false;
				}

				if (SelectorKeyPropertyName.IsNone())
				{
					SelectorKeyPropertyName = ResolvedField.FieldPath.SelectorKeyPropertyName;
				}
				else if (SelectorKeyPropertyName != ResolvedField.FieldPath.SelectorKeyPropertyName)
				{
					OutError = FString::Printf(TEXT("같은 collection에 서로 다른 selector key가 사용됐습니다: %s"), *CollectionName.ToString());
					return false;
				}
				SelectorSet.Add(ResolvedField.FieldPath.SelectorKeyValue);
			}

			// Candidate가 C++ default/PostInit에서 가진 요소가 있더라도 resolved selector 집합만 남기기 위한 array storage입니다.
			void* ArrayValueAddress = ArrayProperty->ContainerPtrToValuePtr<void>(&Candidate);
			// Stable-ID array를 reflection으로 구성할 helper입니다.
			FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayValueAddress);
			ArrayHelper.EmptyValues();

			// 이 collection의 transient construction plan입니다.
			FStableCollectionPlan CollectionPlan;
			CollectionPlan.ArrayProperty = ArrayProperty;
			CollectionPlan.InnerStructProperty = InnerStructProperty;

			if (SelectorSet.IsEmpty())
			{
				OutCollectionPlans.Add(CollectionName, MoveTemp(CollectionPlan));
				continue;
			}

			// Element identity field의 reflected FName property입니다.
			const FNameProperty* SelectorProperty = FindFProperty<FNameProperty>(InnerStructProperty->Struct, SelectorKeyPropertyName);
			if (!SelectorProperty)
			{
				OutError = FString::Printf(
					TEXT("Stable selector가 FName property가 아닙니다: %s.%s"),
					*CollectionName.ToString(),
					*SelectorKeyPropertyName.ToString());
				return false;
			}
			CollectionPlan.SelectorProperty = SelectorProperty;

			CollectionPlan.OrderedSelectors.Reserve(SelectorSet.Num());
			for (const FName SelectorValue : SelectorSet)
			{
				CollectionPlan.OrderedSelectors.Add(SelectorValue);
			}
			CollectionPlan.OrderedSelectors.Sort([](const FName Left, const FName Right)
			{
				return Left.LexicalLess(Right);
			});

			for (const FName SelectorValue : CollectionPlan.OrderedSelectors)
			{
				if (!HasIdentityResolvedField(SortedResolvedFields, CollectionName, SelectorKeyPropertyName, SelectorValue))
				{
					OutError = FString::Printf(
						TEXT("Stable-ID element에 identity resolved leaf가 없습니다: %s[%s=%s]"),
						*CollectionName.ToString(),
						*SelectorKeyPropertyName.ToString(),
						*SelectorValue.ToString());
					return false;
				}
			}

			ArrayHelper.AddValues(CollectionPlan.OrderedSelectors.Num());
			for (int32 SelectorIndex = 0; SelectorIndex < CollectionPlan.OrderedSelectors.Num(); ++SelectorIndex)
			{
				// 현재 physical array index에 대응하는 stable selector입니다.
				const FName SelectorValue = CollectionPlan.OrderedSelectors[SelectorIndex];
				// 새로 default-constructed 된 element storage입니다.
				void* ElementAddress = ArrayHelper.GetRawPtr(SelectorIndex);
				// Leaf import 전에도 stable identity가 확정되도록 selector value storage입니다.
				void* SelectorValueAddress = SelectorProperty->ContainerPtrToValuePtr<void>(ElementAddress);
				SelectorProperty->SetPropertyValue(SelectorValueAddress, SelectorValue);
				CollectionPlan.SelectorToIndex.Add(SelectorValue, SelectorIndex);
			}

			OutCollectionPlans.Add(CollectionName, MoveTemp(CollectionPlan));
		}

		OutError.Reset();
		return true;
	}

	// Exact Stable Field Path가 가리키는 writable leaf property와 candidate storage 주소를 찾습니다.
	bool ResolveWritableLeaf(
		UCFVehicleData& Candidate,
		const FCFVehicleFieldPath& FieldPath,
		const TMap<FName, FStableCollectionPlan>& CollectionPlans,
		const FProperty*& OutLeafProperty,
		void*& OutLeafValueAddress,
		FString& OutError)
	{
		OutLeafProperty = nullptr;
		OutLeafValueAddress = nullptr;

		// Property traversal을 시작할 reflected struct/class입니다.
		const UStruct* CurrentStruct = Candidate.GetClass();
		// CurrentStruct property가 위치한 실제 mutable container address입니다.
		void* CurrentContainer = &Candidate;

		if (!FieldPath.CollectionPropertyName.IsNone())
		{
			// Prebuilt stable collection construction plan입니다.
			const FStableCollectionPlan* CollectionPlan = CollectionPlans.Find(FieldPath.CollectionPropertyName);
			if (!CollectionPlan || !CollectionPlan->ArrayProperty || !CollectionPlan->InnerStructProperty)
			{
				OutError = FString::Printf(TEXT("Materializer collection plan을 찾을 수 없습니다: %s"), *FieldPath.CollectionPropertyName.ToString());
				return false;
			}

			// Exact selector가 배치된 transient physical array index입니다.
			const int32* ElementIndex = CollectionPlan->SelectorToIndex.Find(FieldPath.SelectorKeyValue);
			if (!ElementIndex)
			{
				OutError = FString::Printf(TEXT("Materializer selector element를 찾을 수 없습니다: %s"), *FieldPath.ToCanonicalString(true));
				return false;
			}

			// Candidate의 current collection array storage입니다.
			void* ArrayValueAddress = CollectionPlan->ArrayProperty->ContainerPtrToValuePtr<void>(&Candidate);
			// Exact element storage를 찾을 reflection helper입니다.
			FScriptArrayHelper ArrayHelper(CollectionPlan->ArrayProperty, ArrayValueAddress);
			if (!ArrayHelper.IsValidIndex(*ElementIndex))
			{
				OutError = FString::Printf(TEXT("Materializer selector index가 유효하지 않습니다: %s"), *FieldPath.ToCanonicalString(true));
				return false;
			}

			CurrentStruct = CollectionPlan->InnerStructProperty->Struct;
			CurrentContainer = ArrayHelper.GetRawPtr(*ElementIndex);
		}

		if (FieldPath.PropertyChain.IsEmpty())
		{
			OutError = FString::Printf(TEXT("Materializer leaf property chain이 비어 있습니다: %s"), *FieldPath.ToCanonicalString(true));
			return false;
		}

		for (int32 ChainIndex = 0; ChainIndex < FieldPath.PropertyChain.Num(); ++ChainIndex)
		{
			// 현재 traversal segment의 reflected property입니다.
			const FProperty* Property = FindFProperty<FProperty>(CurrentStruct, FieldPath.PropertyChain[ChainIndex]);
			if (!Property)
			{
				OutError = FString::Printf(TEXT("Materializer property를 찾을 수 없습니다: %s"), *FieldPath.ToCanonicalString(true));
				return false;
			}

			// 현재 property의 실제 mutable storage address입니다.
			void* PropertyValueAddress = Property->ContainerPtrToValuePtr<void>(CurrentContainer);
			if (ChainIndex == FieldPath.PropertyChain.Num() - 1)
			{
				OutLeafProperty = Property;
				OutLeafValueAddress = PropertyValueAddress;
				OutError.Reset();
				return true;
			}

			// Nested property chain의 다음 container schema입니다.
			const FStructProperty* StructProperty = CastField<FStructProperty>(Property);
			if (!StructProperty || !StructProperty->Struct)
			{
				OutError = FString::Printf(TEXT("Materializer nested path 중간 property가 struct가 아닙니다: %s"), *FieldPath.ToCanonicalString(true));
				return false;
			}
			CurrentStruct = StructProperty->Struct;
			CurrentContainer = PropertyValueAddress;
		}

		OutError = FString::Printf(TEXT("Materializer leaf resolve가 완료되지 않았습니다: %s"), *FieldPath.ToCanonicalString(true));
		return false;
	}

	// Sorted resolved field를 Field Codec checked import로 transient candidate에 적용합니다.
	bool ImportResolvedFields(
		UCFVehicleData& Candidate,
		const TArray<FCFVehicleResolvedField>& SortedResolvedFields,
		const TMap<FName, FStableCollectionPlan>& CollectionPlans,
		FString& OutError)
	{
		for (const FCFVehicleResolvedField& ResolvedField : SortedResolvedFields)
		{
			// Exact path가 가리키는 target reflected leaf property입니다.
			const FProperty* LeafProperty = nullptr;
			// Exact path가 가리키는 transient candidate mutable value storage입니다.
			void* LeafValueAddress = nullptr;
			if (!ResolveWritableLeaf(Candidate, ResolvedField.FieldPath, CollectionPlans, LeafProperty, LeafValueAddress, OutError) || !LeafProperty)
			{
				return false;
			}

			// Type signature와 trailing text까지 검사하는 Field Codec import 실패 이유입니다.
			FString ImportError;
			if (!FCFVehicleFieldCodec::ImportValue(*LeafProperty, LeafValueAddress, &Candidate, ResolvedField.Value, ImportError))
			{
				OutError = FString::Printf(
					TEXT("Resolved field import에 실패했습니다: %s / %s"),
					*ResolvedField.FieldPath.ToCanonicalString(true),
					*ImportError);
				return false;
			}
		}

		OutError.Reset();
		return true;
	}

	// Materialization 뒤 physical element identity가 requested selector와 일치하는지 검증합니다.
	bool VerifyStableSelectors(
		UCFVehicleData& Candidate,
		const TMap<FName, FStableCollectionPlan>& CollectionPlans,
		FString& OutError)
	{
		for (const TPair<FName, FStableCollectionPlan>& CollectionPair : CollectionPlans)
		{
			// 현재 검증 중인 collection 이름입니다.
			const FName CollectionName = CollectionPair.Key;
			// 현재 collection construction plan입니다.
			const FStableCollectionPlan& CollectionPlan = CollectionPair.Value;
			if (!CollectionPlan.ArrayProperty || !CollectionPlan.SelectorProperty)
			{
				continue;
			}

			// Candidate의 current collection array storage입니다.
			void* ArrayValueAddress = CollectionPlan.ArrayProperty->ContainerPtrToValuePtr<void>(&Candidate);
			// Stable selector physical identity를 읽을 helper입니다.
			FScriptArrayHelper ArrayHelper(CollectionPlan.ArrayProperty, ArrayValueAddress);
			if (ArrayHelper.Num() != CollectionPlan.OrderedSelectors.Num())
			{
				OutError = FString::Printf(TEXT("Stable-ID collection element 수가 construction plan과 다릅니다: %s"), *CollectionName.ToString());
				return false;
			}

			for (int32 ElementIndex = 0; ElementIndex < CollectionPlan.OrderedSelectors.Num(); ++ElementIndex)
			{
				// Plan이 기대하는 exact stable selector입니다.
				const FName ExpectedSelector = CollectionPlan.OrderedSelectors[ElementIndex];
				// Materialized element storage입니다.
				const void* ElementAddress = ArrayHelper.GetRawPtr(ElementIndex);
				// Identity leaf import 이후 실제 selector 값입니다.
				const FName ActualSelector = CollectionPlan.SelectorProperty->GetPropertyValue_InContainer(ElementAddress);
				if (ActualSelector != ExpectedSelector)
				{
					OutError = FString::Printf(
						TEXT("Stable selector identity가 materialization 중 변경됐습니다: %s Expected=%s Actual=%s"),
						*CollectionName.ToString(),
						*ExpectedSelector.ToString(),
						*ActualSelector.ToString());
					return false;
				}
			}
		}

		OutError.Reset();
		return true;
	}

	// Materialized candidate에서 Resolver가 실제 소유한 exact leaf set만 다시 export해 hash readback projection을 만듭니다.
	bool BuildResolvedReadbackSnapshot(
		UCFVehicleData& Candidate,
		const TArray<FCFVehicleResolvedField>& SortedResolvedFields,
		const TMap<FName, FStableCollectionPlan>& CollectionPlans,
		FCFVehicleDefinitionSnapshot& OutSnapshot,
		FString& OutError)
	{
		OutSnapshot = FCFVehicleDefinitionSnapshot();
		OutSnapshot.SortedFields.Reserve(SortedResolvedFields.Num());

		for (const FCFVehicleResolvedField& ResolvedField : SortedResolvedFields)
		{
			// Readback exact path가 가리키는 reflected target property입니다.
			const FProperty* LeafProperty = nullptr;
			// Readback exact path가 가리키는 candidate value storage입니다.
			void* LeafValueAddress = nullptr;
			if (!ResolveWritableLeaf(Candidate, ResolvedField.FieldPath, CollectionPlans, LeafProperty, LeafValueAddress, OutError) || !LeafProperty)
			{
				return false;
			}

			// Materialized target property 기준으로 다시 canonical export한 value입니다.
			FCFVehicleFieldValue ReadbackValue;
			if (!FCFVehicleFieldCodec::ExportValue(*LeafProperty, LeafValueAddress, ReadbackValue, OutError))
			{
				return false;
			}

			// Resolver-owned projection에 추가할 exact field entry입니다.
			FCFVehicleFieldEntry& ReadbackEntry = OutSnapshot.SortedFields.AddDefaulted_GetRef();
			ReadbackEntry.FieldPath = ResolvedField.FieldPath;
			ReadbackEntry.Value = MoveTemp(ReadbackValue);
		}

		if (!FCFVehicleSnapshotBuilder::BuildDefinitionHashFromFields(OutSnapshot.SortedFields, OutSnapshot.DefinitionHash, OutError))
		{
			return false;
		}

		OutError.Reset();
		return true;
	}

	// 기존 UCFVDAValidator severity를 Authoring DefinitionValidation severity로 그대로 의미 보존해 변환합니다.
	ECFVehicleValidationSeverity ConvertValidationSeverity(const ECFVDASeverity Severity)
	{
		switch (Severity)
		{
		case ECFVDASeverity::Info:
			return ECFVehicleValidationSeverity::Info;
		case ECFVDASeverity::Warning:
			return ECFVehicleValidationSeverity::Warning;
		case ECFVDASeverity::Blocked:
			return ECFVehicleValidationSeverity::Blocked;
		case ECFVDASeverity::Error:
			return ECFVehicleValidationSeverity::Error;
		case ECFVDASeverity::Pass:
		default:
			return ECFVehicleValidationSeverity::Info;
		}
	}

	// Existing Validator report를 immutable Authoring DefinitionValidation rows로 변환합니다.
	void ConvertValidationReport(
		const FCFVDAValidationReport& ValidationReport,
		FCFVehicleMaterializationResult& OutResult)
	{
		OutResult.DefinitionValidation.Reset();
		OutResult.bHasBlockingValidation = false;

		for (const FCFVDAValidationItem& ValidationItem : ValidationReport.Items)
		{
			if (ValidationItem.Severity == ECFVDASeverity::Pass)
			{
				continue;
			}

			// Authoring validation issue로 보존할 mapped severity입니다.
			const ECFVehicleValidationSeverity MappedSeverity = ConvertValidationSeverity(ValidationItem.Severity);
			// GroupName을 기준으로 기존 Validator taxonomy를 손실 없이 식별할 stable issue code입니다.
			const FName IssueCode(*FString::Printf(TEXT("Definition.%s"), *ValidationItem.GroupName.ToString()));
			// 기존 Validator 사용자 메시지입니다.
			FString IssueMessage = ValidationItem.Message.ToString();
			// 기존 Validator가 제공한 권장 조치입니다.
			const FString RecommendedAction = ValidationItem.RecommendedAction.ToString();
			if (!RecommendedAction.IsEmpty())
			{
				IssueMessage += FString::Printf(TEXT(" | 권장 조치: %s"), *RecommendedAction);
			}

			// Public Resolver result에 저장할 Definition validation issue입니다.
			FCFVehicleValidationIssue& Issue = OutResult.DefinitionValidation.AddDefaulted_GetRef();
			Issue.Severity = MappedSeverity;
			Issue.IssueCode = IssueCode;
			Issue.ValidatorFieldPath = ValidationItem.FieldPath;
			Issue.Message = MoveTemp(IssueMessage);

			if (MappedSeverity == ECFVehicleValidationSeverity::Error || MappedSeverity == ECFVehicleValidationSeverity::Blocked)
			{
				OutResult.bHasBlockingValidation = true;
			}
		}
	}

	// Known stable collection의 physical selector 순서를 public materialization evidence에 복사합니다.
	void CopyCollectionOrders(
		const TMap<FName, FStableCollectionPlan>& CollectionPlans,
		FCFVehicleMaterializationResult& OutResult)
	{
		OutResult.HardpointOrder.Reset();
		OutResult.MountOrder.Reset();

		if (const FStableCollectionPlan* HardpointPlan = CollectionPlans.Find(TEXT("HardpointSlots")))
		{
			OutResult.HardpointOrder = HardpointPlan->OrderedSelectors;
		}
		if (const FStableCollectionPlan* MountPlan = CollectionPlans.Find(TEXT("MountProfiles")))
		{
			OutResult.MountOrder = MountPlan->OrderedSelectors;
		}
	}
}

// SortedResolvedFields를 transient UCFVehicleData로 복원하고 Validator/readback evidence를 value-copy로 반환합니다.
bool FCFVehicleMaterializer::MaterializeAndValidate(
	const TArray<FCFVehicleResolvedField>& SortedResolvedFields,
	FCFVehicleMaterializationResult& OutResult,
	FString& OutError)
{
	OutResult = FCFVehicleMaterializationResult();
	if (!CFVehicleMaterializerPrivate::ValidateResolvedFieldOrdering(SortedResolvedFields, OutError))
	{
		return false;
	}

	// Materializer lifetime 동안 GC로부터 transient candidate를 보호하는 strong reference입니다.
	TStrongObjectPtr<UCFVehicleData> CandidateHolder(NewObject<UCFVehicleData>(GetTransientPackage(), NAME_None, RF_Transient));
	// 실제 transient UCFVehicleData candidate pointer입니다.
	UCFVehicleData* Candidate = CandidateHolder.Get();
	if (!Candidate)
	{
		OutError = TEXT("Transient UCFVehicleData candidate를 생성할 수 없습니다.");
		return false;
	}

	// Stable-ID array의 selector 집합과 transient physical index mapping입니다.
	TMap<FName, CFVehicleMaterializerPrivate::FStableCollectionPlan> CollectionPlans;
	if (!CFVehicleMaterializerPrivate::PrepareStableCollections(*Candidate, SortedResolvedFields, CollectionPlans, OutError)
		|| !CFVehicleMaterializerPrivate::ImportResolvedFields(*Candidate, SortedResolvedFields, CollectionPlans, OutError)
		|| !CFVehicleMaterializerPrivate::VerifyStableSelectors(*Candidate, CollectionPlans, OutError)
		|| !CFVehicleMaterializerPrivate::BuildResolvedReadbackSnapshot(*Candidate, SortedResolvedFields, CollectionPlans, OutResult.ResolvedReadbackSnapshot, OutError))
	{
		return false;
	}

	CFVehicleMaterializerPrivate::CopyCollectionOrders(CollectionPlans, OutResult);

	// Frozen Section 22.38에 따라 SourceVehicleData=nullptr로 실행하는 기존 Runtime Definition validator report입니다.
	const FCFVDAValidationReport ValidationReport = UCFVDAValidator::ValidateVehicleData(Candidate, nullptr);
	CFVehicleMaterializerPrivate::ConvertValidationReport(ValidationReport, OutResult);

	OutError.Reset();
	return true;
}
