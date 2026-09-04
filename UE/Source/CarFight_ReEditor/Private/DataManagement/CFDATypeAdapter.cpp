// Copyright (c) CarFight. All Rights Reserved.
// File: CFDATypeAdapter.cpp
// Version: v1.1.0
// Date: 2026-09-03
// Description: CF-FQ-045 DAM-P0-02C loaded UObject identity/validation typed adapter 구현입니다.
// Changelog:
// - v1.1.0: operational evaluation failure와 DA Health를 분리하고 Registry duplicate namespace authority + typed canonical identity를 사용.
// - v1.0.2: VehicleFitting NativeDataValidation이 IsDataValid와 동일하게 BuildFittingSnapshot Error/Warning까지 반영하도록 교정.
// - v1.0.1: UPrimaryDataAsset 선언 owner를 Engine/DataAsset.h로 교정.
// - v1.0.0: FName/FGuid/PrimaryAssetId identity와 current Native/Custom validator 14종을 read-only로 연결.
// Migration:
// - Runtime/Content/Asset mutation 없음. adapter는 이미 load된 UObject만 읽으며 Save/Reference query를 수행하지 않습니다.

#include "DataManagement/CFDATypeAdapter.h"
#include "DataManagement/CFDATypeRegistry.h"

#include "CFAmmoData.h"
#include "CFEquipmentPresetData.h"
#include "CFInventoryItemData.h"
#include "CFRuntimeTestCatalogData.h"
#include "CFTargetSelectData.h"
#include "CFVDAValidator.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseData.h"
#include "CFVehicleFittingData.h"
#include "CFVehicleSensorData.h"
#include "CFWeaponData.h"
#include "Engine/DataAsset.h"
#include "UI/CFHUDLayoutData.h"
#include "UI/CFUIDensityData.h"
#include "UI/CFUIStyleData.h"
#include "UObject/UnrealType.h"

namespace CFDATypeAdapterPrivate
{
	// Detail diagnostics가 비정상적으로 커지는 것을 막는 P0 bounded message 상한입니다.
	constexpr int32 MaxDiagnosticMessages = 64;

	// bounded 결과에 사람이 읽는 진단 한 줄을 추가합니다.
	void AddMessage(
		const FString& Message,
		FCFDALoadedAssetResult& InOutResult)
	{
		if (!Message.IsEmpty() && InOutResult.Messages.Num() < MaxDiagnosticMessages)
		{
			InOutResult.Messages.Add(Message);
		}
	}

	// bool validator 실패를 사용자용 generic 오류 한 줄로 변환합니다.
	void AddGenericValidationError(
		const FCFDASemanticDescriptor& Descriptor,
		FCFDALoadedAssetResult& InOutResult)
	{
		AddMessage(
			FString::Printf(
				TEXT("%s 검증 계약 '%s'을 통과하지 못했습니다."),
				*Descriptor.TypeDisplayName,
				*Descriptor.ValidationSourceName),
			InOutResult);
	}
}

// 이미 load된 UObject 한 건의 stable identity와 typed validation을 계산합니다.
FCFDALoadedAssetResult FCFDATypeAdapter::EvaluateLoadedAsset(
	const FCFDAAssetRecord& AssetRecord,
	UObject* LoadedObject,
	const FCFDATypeRegistry& TypeRegistry,
	const uint64 InventoryGeneration)
{
	FCFDALoadedAssetResult Result;
	Result.InventoryGeneration = InventoryGeneration;
	Result.ObjectPath = AssetRecord.ObjectPath;
	Result.ClassPath = AssetRecord.ClassPath;
	Result.CoverageState = AssetRecord.CoverageState;

	const FCFDAResolvedSemantic ResolvedSemantic =
		TypeRegistry.ResolveSemantic(AssetRecord.ClassPath, AssetRecord.ClassPath);
	Result.CoverageState = ResolvedSemantic.CoverageState;

	if (!ResolvedSemantic.HasAuthoritativePolicy())
	{
		Result.EvaluationState = ECFDAEvaluationState::PolicyUnavailable;
		Result.HealthState = ECFDAHealthState::NotValidated;
		Result.StableIdState = ECFDAStableIdState::NotResolved;
		Result.DuplicateState = ECFDADuplicateState::NotAnalyzed;
		CFDATypeAdapterPrivate::AddMessage(
			TEXT("Typed Semantic descriptor가 등록되지 않아 identity/validation policy를 실행하지 않았습니다."),
			Result);
		return Result;
	}

	if (!LoadedObject)
	{
		Result.EvaluationState = ECFDAEvaluationState::LoadFailed;
		Result.HealthState = ECFDAHealthState::NotValidated;
		CFDATypeAdapterPrivate::AddMessage(TEXT("DataAsset UObject를 load하지 못했습니다."), Result);
		return Result;
	}

	Result.bAssetLoaded = true;

	const FString LoadedClassPath = LoadedObject->GetClass()->GetClassPathName().ToString();
	if (LoadedClassPath != AssetRecord.ClassPath)
	{
		Result.EvaluationState = ECFDAEvaluationState::ClassMismatch;
		Result.HealthState = ECFDAHealthState::NotValidated;
		CFDATypeAdapterPrivate::AddMessage(
			FString::Printf(
				TEXT("Asset Registry class와 loaded UObject class가 다릅니다. Metadata=%s Loaded=%s"),
				*AssetRecord.ClassPath,
				*LoadedClassPath),
			Result);
		return Result;
	}

	Result.EvaluationState = ECFDAEvaluationState::Succeeded;
	if (!ResolveStableIdentity(LoadedObject, ResolvedSemantic.Descriptor, Result))
	{
		Result.EvaluationState = ECFDAEvaluationState::AdapterUnavailable;
		Result.HealthState = ECFDAHealthState::NotValidated;
		return Result;
	}

	if (!RunTypedValidation(LoadedObject, ResolvedSemantic.Descriptor, Result))
	{
		Result.EvaluationState = ECFDAEvaluationState::AdapterUnavailable;
		Result.HealthState = ECFDAHealthState::NotValidated;
	}
	return Result;
}

// descriptor의 Identity Policy/Resolver에 따라 stable identity를 read-only로 해석합니다.
bool FCFDATypeAdapter::ResolveStableIdentity(
	UObject* LoadedObject,
	const FCFDASemanticDescriptor& Descriptor,
	FCFDALoadedAssetResult& InOutResult)
{
	if (Descriptor.IdentityPolicy == ECFDAIdentityPolicy::NotApplicable)
	{
		InOutResult.StableIdState = ECFDAStableIdState::NotApplicable;
		InOutResult.StableId.Reset();
		InOutResult.DuplicateNamespace.Reset();
		InOutResult.CanonicalStableIdentity = FCFDACanonicalStableIdentity();
		InOutResult.DuplicateState = ECFDADuplicateState::NotApplicable;
		return true;
	}

	auto ApplyMissingIdentity = [&Descriptor, &InOutResult]()
	{
		InOutResult.StableId.Reset();
		InOutResult.DuplicateNamespace.Reset();
		InOutResult.CanonicalStableIdentity = FCFDACanonicalStableIdentity();
		InOutResult.DuplicateState = ECFDADuplicateState::NotAnalyzed;
		if (Descriptor.IdentityPolicy == ECFDAIdentityPolicy::Required)
		{
			InOutResult.StableIdState = ECFDAStableIdState::MissingRequired;
			RaiseHealthState(ECFDAHealthState::Error, InOutResult);
			CFDATypeAdapterPrivate::AddMessage(
				FString::Printf(
					TEXT("필수 Stable ID '%s'가 비어 있습니다."),
					*Descriptor.IdentitySourceName),
				InOutResult);
		}
		else
		{
			InOutResult.StableIdState = ECFDAStableIdState::NotResolved;
		}
	};

	const ECFDADuplicateNamespacePolicy NamespacePolicy =
		FCFDATypeRegistry::ResolveDuplicateNamespacePolicy(Descriptor);

	switch (Descriptor.IdentityResolverKind)
	{
	case ECFDAIdentityResolverKind::ExplicitFName:
	{
		if (NamespacePolicy != ECFDADuplicateNamespacePolicy::ExactClassPath)
		{
			CFDATypeAdapterPrivate::AddMessage(TEXT("FName identity의 duplicate namespace policy가 ExactClassPath가 아닙니다."), InOutResult);
			return false;
		}

		const FNameProperty* NameProperty =
			FindFProperty<FNameProperty>(LoadedObject->GetClass(), FName(*Descriptor.IdentitySourceName));
		if (!NameProperty)
		{
			InOutResult.StableIdState = ECFDAStableIdState::NotResolved;
			CFDATypeAdapterPrivate::AddMessage(
				FString::Printf(TEXT("Identity adapter가 FName source '%s'를 찾지 못했습니다."), *Descriptor.IdentitySourceName),
				InOutResult);
			return false;
		}

		const FName IdentityValue = NameProperty->GetPropertyValue_InContainer(LoadedObject);
		if (IdentityValue.IsNone())
		{
			ApplyMissingIdentity();
			return true;
		}

		InOutResult.StableIdState = ECFDAStableIdState::Resolved;
		InOutResult.StableId = IdentityValue.ToString();
		InOutResult.DuplicateNamespace = Descriptor.ClassPath;
		InOutResult.CanonicalStableIdentity.Namespace = Descriptor.ClassPath;
		InOutResult.CanonicalStableIdentity.ValueKind = ECFDAStableIdentityValueKind::Name;
		InOutResult.CanonicalStableIdentity.NameValue = IdentityValue;
		return true;
	}

	case ECFDAIdentityResolverKind::ExplicitGuid:
	{
		if (NamespacePolicy != ECFDADuplicateNamespacePolicy::ExactClassPath)
		{
			CFDATypeAdapterPrivate::AddMessage(TEXT("FGuid identity의 duplicate namespace policy가 ExactClassPath가 아닙니다."), InOutResult);
			return false;
		}

		const FStructProperty* GuidProperty =
			FindFProperty<FStructProperty>(LoadedObject->GetClass(), FName(*Descriptor.IdentitySourceName));
		if (!GuidProperty || GuidProperty->Struct != TBaseStructure<FGuid>::Get())
		{
			InOutResult.StableIdState = ECFDAStableIdState::NotResolved;
			CFDATypeAdapterPrivate::AddMessage(
				FString::Printf(TEXT("Identity adapter가 FGuid source '%s'를 찾지 못했습니다."), *Descriptor.IdentitySourceName),
				InOutResult);
			return false;
		}

		const FGuid* IdentityValue = GuidProperty->ContainerPtrToValuePtr<FGuid>(LoadedObject);
		if (!IdentityValue || !IdentityValue->IsValid())
		{
			ApplyMissingIdentity();
			return true;
		}

		InOutResult.StableIdState = ECFDAStableIdState::Resolved;
		InOutResult.StableId = IdentityValue->ToString();
		InOutResult.DuplicateNamespace = Descriptor.ClassPath;
		InOutResult.CanonicalStableIdentity.Namespace = Descriptor.ClassPath;
		InOutResult.CanonicalStableIdentity.ValueKind = ECFDAStableIdentityValueKind::Guid;
		InOutResult.CanonicalStableIdentity.GuidValue = *IdentityValue;
		return true;
	}

	case ECFDAIdentityResolverKind::PrimaryAssetId:
	{
		if (NamespacePolicy != ECFDADuplicateNamespacePolicy::PrimaryAssetType)
		{
			CFDATypeAdapterPrivate::AddMessage(TEXT("PrimaryAssetId identity의 duplicate namespace policy가 PrimaryAssetType이 아닙니다."), InOutResult);
			return false;
		}

		const UPrimaryDataAsset* PrimaryDataAsset = Cast<UPrimaryDataAsset>(LoadedObject);
		if (!PrimaryDataAsset)
		{
			InOutResult.StableIdState = ECFDAStableIdState::NotResolved;
			CFDATypeAdapterPrivate::AddMessage(TEXT("Identity adapter가 UPrimaryDataAsset 계약을 찾지 못했습니다."), InOutResult);
			return false;
		}

		const FPrimaryAssetId IdentityValue = PrimaryDataAsset->GetPrimaryAssetId();
		if (!IdentityValue.IsValid())
		{
			ApplyMissingIdentity();
			return true;
		}

		InOutResult.StableIdState = ECFDAStableIdState::Resolved;
		InOutResult.StableId = IdentityValue.ToString();
		InOutResult.DuplicateNamespace = IdentityValue.PrimaryAssetType.ToString();
		InOutResult.CanonicalStableIdentity.Namespace = InOutResult.DuplicateNamespace;
		InOutResult.CanonicalStableIdentity.ValueKind = ECFDAStableIdentityValueKind::Name;
		InOutResult.CanonicalStableIdentity.NameValue = IdentityValue.PrimaryAssetName;
		return true;
	}

	default:
		InOutResult.StableIdState = ECFDAStableIdState::NotResolved;
		CFDATypeAdapterPrivate::AddMessage(TEXT("등록된 Identity Policy에 대응하는 resolver가 없습니다."), InOutResult);
		return false;
	}
}

// descriptor의 Validation Policy에 따라 existing validator를 read-only로 실행합니다.
bool FCFDATypeAdapter::RunTypedValidation(
	UObject* LoadedObject,
	const FCFDASemanticDescriptor& Descriptor,
	FCFDALoadedAssetResult& InOutResult)
{
	if (Descriptor.ValidationPolicy == ECFDAValidationPolicy::None)
	{
		RaiseHealthState(ECFDAHealthState::NotApplicable, InOutResult);
		return true;
	}

	// 기존 public validation contract에서 수집한 오류 목록입니다.
	TArray<FText> ValidationErrors;

	if (Descriptor.ValidationPolicy == ECFDAValidationPolicy::NativeDataValidation)
	{
		bool bValidationPassed = false;
		bool bAdapterMatched = true;

		if (Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFEquipmentItemData")
			|| Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFDefenseItemData"))
		{
			const UCFInventoryItemData* ItemData = Cast<UCFInventoryItemData>(LoadedObject);
			bAdapterMatched = ItemData != nullptr;
			bValidationPassed = ItemData && ItemData->ValidateItemDefinitionContract(ValidationErrors);
		}
		else if (Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFVehicleDefenseData"))
		{
			const UCFVehicleDefenseData* DefenseData = Cast<UCFVehicleDefenseData>(LoadedObject);
			bAdapterMatched = DefenseData != nullptr;
			bValidationPassed = DefenseData && DefenseData->ValidateDefenseDataContract(ValidationErrors);
		}
		else if (Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFVehicleFittingData"))
		{
			const UCFVehicleFittingData* FittingData = Cast<UCFVehicleFittingData>(LoadedObject);
			bAdapterMatched = FittingData != nullptr;
			bValidationPassed = FittingData && FittingData->ValidateFittingDataContract(ValidationErrors);

			if (FittingData && bValidationPassed)
			{
				// IsDataValid()가 기본 계약 뒤에 수행하는 결정론적 Fitting Snapshot 검증 결과입니다.
				const FCFVehicleFittingSnapshot Snapshot = FittingData->BuildFittingSnapshot();

				// Snapshot Warning/Error를 loaded Health와 diagnostics에 같은 의미로 투영합니다.
				for (const FCFFittingValidationIssue& ValidationIssue : Snapshot.ValidationIssues)
				{
					if (ValidationIssue.Severity == ECFFittingIssueSeverity::Error)
					{
						RaiseHealthState(ECFDAHealthState::Error, InOutResult);
						CFDATypeAdapterPrivate::AddMessage(ValidationIssue.Message.ToString(), InOutResult);
					}
					else if (ValidationIssue.Severity == ECFFittingIssueSeverity::Warning)
					{
						RaiseHealthState(ECFDAHealthState::Warning, InOutResult);
						CFDATypeAdapterPrivate::AddMessage(ValidationIssue.Message.ToString(), InOutResult);
					}
				}

				bValidationPassed = Snapshot.IsValid();
			}
		}
		else if (Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFVehicleSensorData"))
		{
			const UCFVehicleSensorData* SensorData = Cast<UCFVehicleSensorData>(LoadedObject);
			bAdapterMatched = SensorData != nullptr;
			bValidationPassed = SensorData && SensorData->ValidateSensorDataContract(ValidationErrors);
		}
		else if (Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFWeaponData"))
		{
			const UCFWeaponData* WeaponData = Cast<UCFWeaponData>(LoadedObject);
			bAdapterMatched = WeaponData != nullptr;
			bValidationPassed = WeaponData && WeaponData->ValidateWeaponDataContract(ValidationErrors);
		}
		else
		{
			bAdapterMatched = false;
		}

		if (!bAdapterMatched)
		{
			CFDATypeAdapterPrivate::AddMessage(
				FString::Printf(
					TEXT("NativeDataValidation adapter가 등록되지 않았습니다: %s"),
					*Descriptor.ClassPath),
				InOutResult);
			return false;
		}

		if (bValidationPassed)
		{
			// 이미 Snapshot Warning이 올라간 경우에는 OK가 Warning을 낮추지 않습니다.
			RaiseHealthState(ECFDAHealthState::OK, InOutResult);
		}
		else
		{
			RaiseHealthState(ECFDAHealthState::Error, InOutResult);
			AppendTextMessages(ValidationErrors, InOutResult);
			if (ValidationErrors.IsEmpty())
			{
				CFDATypeAdapterPrivate::AddGenericValidationError(Descriptor, InOutResult);
			}
		}
		return true;
	}

	if (Descriptor.ValidationPolicy == ECFDAValidationPolicy::CustomContract)
	{
		if (const UCFAmmoData* AmmoData = Cast<UCFAmmoData>(LoadedObject);
			Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFAmmoData") && AmmoData)
		{
			if (AmmoData->IsAmmoDataValid())
			{
				RaiseHealthState(ECFDAHealthState::OK, InOutResult);
			}
			else
			{
				RaiseHealthState(ECFDAHealthState::Error, InOutResult);
				CFDATypeAdapterPrivate::AddGenericValidationError(Descriptor, InOutResult);
			}
			return true;
		}

		if (const UCFEquipmentPresetData* EquipmentData = Cast<UCFEquipmentPresetData>(LoadedObject);
			Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFEquipmentPresetData") && EquipmentData)
		{
			if (EquipmentData->HasCompleteEquipmentData())
			{
				RaiseHealthState(ECFDAHealthState::OK, InOutResult);
			}
			else
			{
				RaiseHealthState(ECFDAHealthState::Error, InOutResult);
				CFDATypeAdapterPrivate::AddGenericValidationError(Descriptor, InOutResult);
			}
			return true;
		}

		if (const UCFRuntimeTestCatalogData* CatalogData = Cast<UCFRuntimeTestCatalogData>(LoadedObject);
			Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFRuntimeTestCatalogData") && CatalogData)
		{
			if (CatalogData->ValidateRuntimeTestCatalog(ValidationErrors))
			{
				RaiseHealthState(ECFDAHealthState::OK, InOutResult);
			}
			else
			{
				RaiseHealthState(ECFDAHealthState::Error, InOutResult);
				AppendTextMessages(ValidationErrors, InOutResult);
			}
			return true;
		}

		if (const UCFTargetSelectData* TargetSelectData = Cast<UCFTargetSelectData>(LoadedObject);
			Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFTargetSelectData") && TargetSelectData)
		{
			if (TargetSelectData->IsTargetSelectConfigValid())
			{
				RaiseHealthState(ECFDAHealthState::OK, InOutResult);
			}
			else
			{
				RaiseHealthState(ECFDAHealthState::Error, InOutResult);
				CFDATypeAdapterPrivate::AddGenericValidationError(Descriptor, InOutResult);
			}
			return true;
		}

		if (UCFVehicleData* VehicleData = Cast<UCFVehicleData>(LoadedObject);
			Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFVehicleData") && VehicleData)
		{
			// existing public read-only VehicleData validator report입니다.
			const FCFVDAValidationReport Report =
				UCFVDAValidator::ValidateVehicleData(VehicleData, nullptr);

			for (const FCFVDAValidationItem& Item : Report.Items)
			{
				if (Item.Severity != ECFVDASeverity::Pass)
				{
					CFDATypeAdapterPrivate::AddMessage(Item.Message.ToString(), InOutResult);
				}
			}

			if (Report.ErrorCount > 0)
			{
				RaiseHealthState(ECFDAHealthState::Error, InOutResult);
			}
			else if (Report.WarningCount > 0 || Report.BlockedCount > 0)
			{
				RaiseHealthState(ECFDAHealthState::Warning, InOutResult);
			}
			else
			{
				RaiseHealthState(ECFDAHealthState::OK, InOutResult);
			}
			return true;
		}

		if (const UCFHUDLayoutData* LayoutData = Cast<UCFHUDLayoutData>(LoadedObject);
			Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFHUDLayoutData") && LayoutData)
		{
			// existing Layout validator의 failure reason입니다.
			FString FailureReason;
			if (LayoutData->ValidateLayoutData(FailureReason))
			{
				RaiseHealthState(ECFDAHealthState::OK, InOutResult);
			}
			else
			{
				RaiseHealthState(ECFDAHealthState::Error, InOutResult);
				CFDATypeAdapterPrivate::AddMessage(FailureReason, InOutResult);
			}
			return true;
		}

		if (const UCFUIDensityData* DensityData = Cast<UCFUIDensityData>(LoadedObject);
			Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFUIDensityData") && DensityData)
		{
			// existing Density validator의 failure reason입니다.
			FString FailureReason;
			if (DensityData->ValidateDensityData(FailureReason))
			{
				RaiseHealthState(ECFDAHealthState::OK, InOutResult);
			}
			else
			{
				RaiseHealthState(ECFDAHealthState::Error, InOutResult);
				CFDATypeAdapterPrivate::AddMessage(FailureReason, InOutResult);
			}
			return true;
		}

		if (const UCFUIStyleData* StyleData = Cast<UCFUIStyleData>(LoadedObject);
			Descriptor.ClassPath == TEXT("/Script/CarFight_Re.CFUIStyleData") && StyleData)
		{
			// existing Style validator의 failure reason입니다.
			FString FailureReason;
			if (StyleData->ValidateStyleData(FailureReason))
			{
				RaiseHealthState(ECFDAHealthState::OK, InOutResult);
			}
			else
			{
				RaiseHealthState(ECFDAHealthState::Error, InOutResult);
				CFDATypeAdapterPrivate::AddMessage(FailureReason, InOutResult);
			}
			return true;
		}

		CFDATypeAdapterPrivate::AddMessage(
			FString::Printf(
				TEXT("CustomContract adapter가 등록되지 않았습니다: %s"),
				*Descriptor.ClassPath),
			InOutResult);
		return false;
	}

	CFDATypeAdapterPrivate::AddMessage(TEXT("알 수 없는 Validation Policy입니다."), InOutResult);
	return false;
}

// Health 우선순위를 낮추지 않으면서 새 상태를 반영합니다.
void FCFDATypeAdapter::RaiseHealthState(
	const ECFDAHealthState NewHealthState,
	FCFDALoadedAssetResult& InOutResult)
{
	auto HealthRank = [](const ECFDAHealthState State)
	{
		switch (State)
		{
		case ECFDAHealthState::Error: return 4;
		case ECFDAHealthState::Warning: return 3;
		case ECFDAHealthState::OK: return 2;
		case ECFDAHealthState::NotApplicable: return 1;
		case ECFDAHealthState::NotValidated:
		default:
			return 0;
		}
	};

	if (HealthRank(NewHealthState) > HealthRank(InOutResult.HealthState))
	{
		InOutResult.HealthState = NewHealthState;
	}
}

// FText validation error 목록을 문자열 메시지로 추가합니다.
void FCFDATypeAdapter::AppendTextMessages(
	const TArray<FText>& ValidationMessages,
	FCFDALoadedAssetResult& InOutResult)
{
	for (const FText& ValidationMessage : ValidationMessages)
	{
		CFDATypeAdapterPrivate::AddMessage(
			ValidationMessage.ToString(),
			InOutResult);
	}
}
