// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleImportService.cpp
// Version: v1.2.0
// Date: 2026-08-18
// Description: DAUTH-P0-08G~P0-11 Existing Definition Import / Legacy Pin / Adoption / Raw Preservation Core 구현입니다.
// Scope: Snapshot-only import analysis와 Recipe-only ownership mutation primitive를 제공하며 Target Definition은 수정하지 않습니다.
// Changelog:
// - v1.2.0: Frozen 24.59 Preserve Raw As Legacy Pin을 existing Registry ownership/ManageState 규칙으로 처리하는 공용 primitive 추가.
// - v1.1.0: persistent Recipe mutation 없이 exact Import summary를 만드는 PreviewDefinitionImport를 추가.
// - v1.0.0: Section 22.20~22.21 lossless import, hidden Mount serialized partition, semantic candidate copy, group/field adoption 최초 구현.
// Migration:
// - Movement raw 값에서 Driving Feel/Profile을 역산하지 않습니다.
// - Definition Apply/UI/CSV/Runtime schema mutation을 포함하지 않습니다.

#include "DataAuthoring/CFVehicleImportService.h"

#include "CFVehicleData.h"
#include "DataAuthoring/CFVehicleFieldCodec.h"
#include "DataAuthoring/CFVehicleFieldRegistry.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleResolver.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "ScopedTransaction.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

namespace CFVehicleImportPrivate
{
	/** Import semantic candidate를 persistent Recipe에 쓰기 전 local value로 구성합니다. */
	struct FSemanticCandidateState
	{
		// Current visual asset references와 wheel socket binding intent입니다.
		FCFVehicleAssetIntent AssetIntent;

		// Current Base/Gross mass를 direct semantic explicit value로 보존합니다.
		FCFVehicleMassIntent MassIntent;

		// Current MaxHealth를 direct semantic explicit value로 보존합니다.
		FCFVehicleDurabilityIntent DurabilityIntent;

		// Current stable-ID Hardpoint identity/category/socket semantic rows입니다.
		TArray<FCFHardpointIntent> HardpointIntents;

		// Current stable-ID Mount active semantic rows입니다.
		TArray<FCFMountIntent> MountIntents;

		// Current Default Defense/Destroyed FX와 socket semantic input입니다.
		FCFVehicleDefaultIntent DefaultDataIntent;

		// Current DriveState runtime override gate를 direct semantic mode로 보존합니다.
		ECFVehicleDriveStateMode DriveStateMode = ECFVehicleDriveStateMode::ProjectDefault;

		// 실제 direct semantic copy에 사용한 Current Definition leaf 수입니다.
		int32 CopiedFieldCount = 0;
	};

	// Exact Stable Field Path가 Registry wildcard descriptor와 같은 leaf를 가리키는지 검사합니다.
	bool DoesPathMatchDescriptor(
		const FCFVehicleFieldPath& ExactPath,
		const FCFVehicleFieldDescriptor& Descriptor)
	{
		const FCFVehicleFieldPath& Pattern = Descriptor.StablePathPattern;
		if (ExactPath.CollectionPropertyName != Pattern.CollectionPropertyName
			|| ExactPath.SelectorKeyPropertyName != Pattern.SelectorKeyPropertyName
			|| ExactPath.PropertyChain != Pattern.PropertyChain)
		{
			return false;
		}

		if (Pattern.CollectionPropertyName.IsNone())
		{
			return ExactPath.SelectorKeyValue.IsNone();
		}
		return !ExactPath.SelectorKeyValue.IsNone();
	}

	// Exact Stable Field Path를 소유하는 Frozen Registry descriptor를 찾습니다.
	const FCFVehicleFieldDescriptor* FindDescriptor(const FCFVehicleFieldPath& ExactPath)
	{
		return FCFVehicleFieldRegistry::GetDescriptors().FindByPredicate([&ExactPath](const FCFVehicleFieldDescriptor& Descriptor)
		{
			return DoesPathMatchDescriptor(ExactPath, Descriptor);
		});
	}

	// Definition Snapshot에서 exact canonical path entry를 찾습니다.
	const FCFVehicleFieldEntry* FindDefinitionEntry(
		const FCFVehicleDefinitionSnapshot& Definition,
		const FString& CanonicalPath)
	{
		return Definition.SortedFields.FindByPredicate([&CanonicalPath](const FCFVehicleFieldEntry& Entry)
		{
			return Entry.FieldPath.ToCanonicalString(true) == CanonicalPath;
		});
	}

	// Current Definition의 same-type field value를 semantic target USTRUCT property로 checked import합니다.
	bool ImportSameTypeField(
		const FCFVehicleFieldEntry& SourceEntry,
		const UScriptStruct& TargetStruct,
		void* TargetStructAddress,
		const FName TargetPropertyName,
		FString& OutError)
	{
		// Semantic target property입니다.
		const FProperty* TargetProperty = FindFProperty<FProperty>(&TargetStruct, TargetPropertyName);
		if (!TargetProperty || !TargetStructAddress)
		{
			OutError = FString::Printf(TEXT("Semantic candidate target property를 찾을 수 없습니다: %s.%s"), *TargetStruct.GetName(), *TargetPropertyName.ToString());
			return false;
		}

		// Semantic target property의 mutable value storage입니다.
		void* TargetValueAddress = TargetProperty->ContainerPtrToValuePtr<void>(TargetStructAddress);
		if (!FCFVehicleFieldCodec::ImportValue(*TargetProperty, TargetValueAddress, nullptr, SourceEntry.Value, OutError))
		{
			OutError = FString::Printf(TEXT("Semantic candidate typed copy 실패: %s -> %s.%s / %s"), *SourceEntry.FieldPath.ToCanonicalString(true), *TargetStruct.GetName(), *TargetPropertyName.ToString(), *OutError);
			return false;
		}
		return true;
	}

	// Runtime hard object reference canonical text를 같은 class constraint의 Recipe soft reference에만 변환합니다.
	bool ImportObjectAsSoftReference(
		const FCFVehicleFieldEntry& SourceEntry,
		const UScriptStruct& TargetStruct,
		void* TargetStructAddress,
		const FName TargetPropertyName,
		FString& OutError)
	{
		// Recipe semantic target soft object property입니다.
		const FSoftObjectProperty* TargetProperty = FindFProperty<FSoftObjectProperty>(&TargetStruct, TargetPropertyName);
		if (!TargetProperty || !TargetStructAddress)
		{
			OutError = FString::Printf(TEXT("Semantic soft reference target property를 찾을 수 없습니다: %s.%s"), *TargetStruct.GetName(), *TargetPropertyName.ToString());
			return false;
		}

		// Target soft property의 current type signature입니다.
		const FString TargetSignature = FCFVehicleFieldCodec::BuildTypeSignature(*TargetProperty);
		// Runtime hard object source에서 기대하는 same-class signature입니다.
		FString ExpectedSourceSignature = TargetSignature;
		if (!ExpectedSourceSignature.RemoveFromStart(TEXT("SoftObject:")))
		{
			OutError = FString::Printf(TEXT("Target이 SoftObject signature가 아닙니다: %s"), *TargetSignature);
			return false;
		}
		ExpectedSourceSignature = TEXT("Object:") + ExpectedSourceSignature;
		if (SourceEntry.Value.PropertyTypeSignature != ExpectedSourceSignature)
		{
			OutError = FString::Printf(TEXT("Hard→Soft reference class constraint mismatch. Expected=%s Actual=%s"), *ExpectedSourceSignature, *SourceEntry.Value.PropertyTypeSignature);
			return false;
		}

		// Recipe soft reference의 mutable value storage입니다.
		void* TargetValueAddress = TargetProperty->ContainerPtrToValuePtr<void>(TargetStructAddress);
		// Runtime object ExportText를 same-class soft reference parser로 읽은 뒤 남는 text 위치입니다.
		const TCHAR* ImportResult = TargetProperty->ImportText_Direct(*SourceEntry.Value.CanonicalValueText, TargetValueAddress, nullptr, PPF_None);
		if (!ImportResult)
		{
			OutError = FString::Printf(TEXT("Hard→Soft reference semantic copy에 실패했습니다: %s"), *SourceEntry.Value.CanonicalValueText);
			return false;
		}

		// Soft reference parser가 소비하지 않은 trailing text입니다.
		const FString RemainingText = FString(ImportResult).TrimStartAndEnd();
		if (!RemainingText.IsEmpty())
		{
			OutError = FString::Printf(TEXT("Hard→Soft reference 뒤에 해석되지 않은 text가 남았습니다: %s"), *RemainingText);
			return false;
		}
		return true;
	}

	// Object canonical value가 explicit None인지 검사합니다.
	bool IsNoneObjectValue(const FCFVehicleFieldEntry& Entry)
	{
		return Entry.Value.CanonicalValueText.TrimStartAndEnd().Equals(TEXT("None"), ESearchCase::IgnoreCase);
	}

	// Required scalar Definition entry를 찾고 없으면 import를 fail-closed합니다.
	const FCFVehicleFieldEntry* RequireEntry(
		const FCFVehicleDefinitionSnapshot& Definition,
		const FString& CanonicalPath,
		FString& OutError)
	{
		// Requested Current Definition field entry입니다.
		const FCFVehicleFieldEntry* Entry = FindDefinitionEntry(Definition, CanonicalPath);
		if (!Entry)
		{
			OutError = FString::Printf(TEXT("Existing Definition import에 필요한 field가 없습니다: %s"), *CanonicalPath);
		}
		return Entry;
	}

	// Current Definition의 direct semantic scalar/reference fields를 local candidate state로 복사합니다.
	bool CopyScalarSemanticCandidates(
		const FCFVehicleDefinitionSnapshot& Definition,
		FSemanticCandidateState& InOutState,
		FString& OutError)
	{
		static const TCHAR* VisualSourcePaths[] =
		{
			TEXT("VehicleVisualConfig.ChassisMesh"), TEXT("VehicleVisualConfig.WheelMeshFL"),
			TEXT("VehicleVisualConfig.WheelMeshFR"), TEXT("VehicleVisualConfig.WheelMeshRL"), TEXT("VehicleVisualConfig.WheelMeshRR")
		};
		static const FName VisualTargetProperties[] =
		{
			TEXT("ChassisMesh"), TEXT("WheelMeshFL"), TEXT("WheelMeshFR"), TEXT("WheelMeshRL"), TEXT("WheelMeshRR")
		};
		for (int32 FieldIndex = 0; FieldIndex < UE_ARRAY_COUNT(VisualSourcePaths); ++FieldIndex)
		{
			// Current visual object reference entry입니다.
			const FCFVehicleFieldEntry* Entry = RequireEntry(Definition, VisualSourcePaths[FieldIndex], OutError);
			if (!Entry || !ImportObjectAsSoftReference(*Entry, *FCFVehicleAssetIntent::StaticStruct(), &InOutState.AssetIntent, VisualTargetProperties[FieldIndex], OutError))
			{
				return false;
			}
			++InOutState.CopiedFieldCount;
		}

		static const TCHAR* SocketSourcePaths[] =
		{
			TEXT("VehicleLayoutConfig.BodyWheelSocketFL"), TEXT("VehicleLayoutConfig.BodyWheelSocketFR"),
			TEXT("VehicleLayoutConfig.BodyWheelSocketRL"), TEXT("VehicleLayoutConfig.BodyWheelSocketRR")
		};
		static const FName SocketTargetProperties[] =
		{
			TEXT("BodyWheelSocketFL"), TEXT("BodyWheelSocketFR"), TEXT("BodyWheelSocketRL"), TEXT("BodyWheelSocketRR")
		};
		for (int32 FieldIndex = 0; FieldIndex < UE_ARRAY_COUNT(SocketSourcePaths); ++FieldIndex)
		{
			// Current wheel socket binding entry입니다.
			const FCFVehicleFieldEntry* Entry = RequireEntry(Definition, SocketSourcePaths[FieldIndex], OutError);
			if (!Entry || !ImportSameTypeField(*Entry, *FCFVehicleAssetIntent::StaticStruct(), &InOutState.AssetIntent, SocketTargetProperties[FieldIndex], OutError))
			{
				return false;
			}
			++InOutState.CopiedFieldCount;
		}

		// Current Base mass semantic value입니다.
		const FCFVehicleFieldEntry* BaseMassEntry = RequireEntry(Definition, TEXT("BaseVehicleMassKg"), OutError);
		// Current Gross mass semantic value입니다.
		const FCFVehicleFieldEntry* GrossMassEntry = RequireEntry(Definition, TEXT("MaximumGrossMassKg"), OutError);
		if (!BaseMassEntry || !GrossMassEntry
			|| !ImportSameTypeField(*BaseMassEntry, *FCFVehicleMassIntent::StaticStruct(), &InOutState.MassIntent, TEXT("ExplicitBaseMassKg"), OutError)
			|| !ImportSameTypeField(*GrossMassEntry, *FCFVehicleMassIntent::StaticStruct(), &InOutState.MassIntent, TEXT("ExplicitGrossMassKg"), OutError))
		{
			return false;
		}
		InOutState.MassIntent.BaseMassMode = ECFAuthoringInputMode::ExplicitValue;
		InOutState.MassIntent.GrossMassMode = ECFAuthoringInputMode::ExplicitValue;
		InOutState.CopiedFieldCount += 2;

		// Current MaxHealth direct semantic value입니다.
		const FCFVehicleFieldEntry* MaxHealthEntry = RequireEntry(Definition, TEXT("VehicleDurabilityConfig.MaxHealth"), OutError);
		if (!MaxHealthEntry || !ImportSameTypeField(*MaxHealthEntry, *FCFVehicleDurabilityIntent::StaticStruct(), &InOutState.DurabilityIntent, TEXT("ExplicitMaxHealth"), OutError))
		{
			return false;
		}
		InOutState.DurabilityIntent.MaxHealthMode = ECFAuthoringInputMode::ExplicitValue;
		++InOutState.CopiedFieldCount;

		// Current Default Defense object reference입니다.
		const FCFVehicleFieldEntry* DefenseEntry = RequireEntry(Definition, TEXT("DefaultDefenseData"), OutError);
		// Current Destroyed FX object reference입니다.
		const FCFVehicleFieldEntry* DestroyedFxEntry = RequireEntry(Definition, TEXT("DefaultDestroyedFxData"), OutError);
		if (!DefenseEntry || !DestroyedFxEntry
			|| !ImportObjectAsSoftReference(*DefenseEntry, *FCFVehicleDefaultIntent::StaticStruct(), &InOutState.DefaultDataIntent, TEXT("DefaultDefenseData"), OutError)
			|| !ImportObjectAsSoftReference(*DestroyedFxEntry, *FCFVehicleDefaultIntent::StaticStruct(), &InOutState.DefaultDataIntent, TEXT("DefaultDestroyedFxData"), OutError))
		{
			return false;
		}
		InOutState.DefaultDataIntent.DefenseMode = IsNoneObjectValue(*DefenseEntry) ? ECFAssetIntentMode::ExplicitNone : ECFAssetIntentMode::ExplicitAsset;
		InOutState.DefaultDataIntent.DestroyedFxMode = IsNoneObjectValue(*DestroyedFxEntry) ? ECFAssetIntentMode::ExplicitNone : ECFAssetIntentMode::ExplicitAsset;
		InOutState.CopiedFieldCount += 2;

		// Current Destroyed FX socket direct semantic input입니다.
		const FCFVehicleFieldEntry* DestroyedSocketEntry = RequireEntry(Definition, TEXT("DestroyedFxSocketName"), OutError);
		if (!DestroyedSocketEntry || !ImportSameTypeField(*DestroyedSocketEntry, *FCFVehicleDefaultIntent::StaticStruct(), &InOutState.DefaultDataIntent, TEXT("DestroyedFxSocketName"), OutError))
		{
			return false;
		}
		++InOutState.CopiedFieldCount;

		// Current DriveState runtime gate를 direct semantic mode로 읽을 temporary config입니다.
		FCFVehicleDriveStateConfig DriveStateConfig;
		// Current DriveState gate exact field입니다.
		const FCFVehicleFieldEntry* DriveStateGateEntry = RequireEntry(Definition, TEXT("DriveStateConfig.bUseDriveStateOverrides"), OutError);
		if (!DriveStateGateEntry || !ImportSameTypeField(*DriveStateGateEntry, *FCFVehicleDriveStateConfig::StaticStruct(), &DriveStateConfig, TEXT("bUseDriveStateOverrides"), OutError))
		{
			return false;
		}
		InOutState.DriveStateMode = DriveStateConfig.bUseDriveStateOverrides ? ECFVehicleDriveStateMode::VehicleSpecific : ECFVehicleDriveStateMode::ProjectDefault;
		++InOutState.CopiedFieldCount;
		return true;
	}

	// Current Definition Hardpoint exact selector rows를 stable ID order로 semantic intent에 복사합니다.
	bool CopyHardpointCandidates(
		const FCFVehicleDefinitionSnapshot& Definition,
		FSemanticCandidateState& InOutState,
		FString& OutError)
	{
		// Current Definition에서 발견한 exact Hardpoint stable IDs입니다.
		TArray<FName> SelectorValues;
		for (const FCFVehicleFieldEntry& Entry : Definition.SortedFields)
		{
			const FCFVehicleFieldDescriptor* Descriptor = FindDescriptor(Entry.FieldPath);
			if (Descriptor && Descriptor->bIdentityField && Entry.FieldPath.CollectionPropertyName == TEXT("HardpointSlots"))
			{
				SelectorValues.AddUnique(Entry.FieldPath.SelectorKeyValue);
			}
		}
		SelectorValues.Sort([](const FName Left, const FName Right) { return Left.LexicalLess(Right); });

		InOutState.HardpointIntents.Reset();
		for (const FName SelectorValue : SelectorValues)
		{
			// Current selector의 direct semantic Hardpoint intent입니다.
			FCFHardpointIntent& Intent = InOutState.HardpointIntents.AddDefaulted_GetRef();
			Intent.LocationSlotId = SelectorValue;

			// Current Hardpoint Category entry입니다.
			const FCFVehicleFieldEntry* CategoryEntry = RequireEntry(Definition, FString::Printf(TEXT("HardpointSlots[LocationSlotId=%s].LocationCategory"), *SelectorValue.ToString()), OutError);
			// Current Hardpoint Socket binding entry입니다.
			const FCFVehicleFieldEntry* SocketEntry = RequireEntry(Definition, FString::Printf(TEXT("HardpointSlots[LocationSlotId=%s].SocketName"), *SelectorValue.ToString()), OutError);
			if (!CategoryEntry || !SocketEntry
				|| !ImportSameTypeField(*CategoryEntry, *FCFHardpointIntent::StaticStruct(), &Intent, TEXT("LocationCategory"), OutError)
				|| !ImportSameTypeField(*SocketEntry, *FCFHardpointIntent::StaticStruct(), &Intent, TEXT("SocketName"), OutError))
			{
				return false;
			}
			InOutState.CopiedFieldCount += 3;
		}
		return true;
	}

	// Current Definition Mount active six leaf를 stable ID order로 semantic intent에 복사하고 hidden ten leaf는 복사하지 않습니다.
	bool CopyMountCandidates(
		const FCFVehicleDefinitionSnapshot& Definition,
		FSemanticCandidateState& InOutState,
		FString& OutError)
	{
		// Current Definition에서 발견한 exact Mount stable IDs입니다.
		TArray<FName> SelectorValues;
		for (const FCFVehicleFieldEntry& Entry : Definition.SortedFields)
		{
			const FCFVehicleFieldDescriptor* Descriptor = FindDescriptor(Entry.FieldPath);
			if (Descriptor && Descriptor->bIdentityField && Entry.FieldPath.CollectionPropertyName == TEXT("MountProfiles"))
			{
				SelectorValues.AddUnique(Entry.FieldPath.SelectorKeyValue);
			}
		}
		SelectorValues.Sort([](const FName Left, const FName Right) { return Left.LexicalLess(Right); });

		InOutState.MountIntents.Reset();
		for (const FName SelectorValue : SelectorValues)
		{
			// Current selector의 active semantic Mount intent입니다.
			FCFMountIntent& Intent = InOutState.MountIntents.AddDefaulted_GetRef();
			Intent.MountProfileId = SelectorValue;

			// Current Mount LocationSlotRef entry입니다.
			const FCFVehicleFieldEntry* SlotEntry = RequireEntry(Definition, FString::Printf(TEXT("MountProfiles[MountProfileId=%s].LocationSlotRef"), *SelectorValue.ToString()), OutError);
			// Current MountType entry입니다.
			const FCFVehicleFieldEntry* TypeEntry = RequireEntry(Definition, FString::Printf(TEXT("MountProfiles[MountProfileId=%s].MountType"), *SelectorValue.ToString()), OutError);
			// Current Mount SizeLimit entry입니다.
			const FCFVehicleFieldEntry* SizeEntry = RequireEntry(Definition, FString::Printf(TEXT("MountProfiles[MountProfileId=%s].SizeLimit"), *SelectorValue.ToString()), OutError);
			// Current Mount DefaultEquipmentPresetData entry입니다.
			const FCFVehicleFieldEntry* PresetEntry = RequireEntry(Definition, FString::Printf(TEXT("MountProfiles[MountProfileId=%s].DefaultEquipmentPresetData"), *SelectorValue.ToString()), OutError);
			// Current Mount bExposedModule entry입니다.
			const FCFVehicleFieldEntry* ExposedEntry = RequireEntry(Definition, FString::Printf(TEXT("MountProfiles[MountProfileId=%s].bExposedModule"), *SelectorValue.ToString()), OutError);
			if (!SlotEntry || !TypeEntry || !SizeEntry || !PresetEntry || !ExposedEntry
				|| !ImportSameTypeField(*SlotEntry, *FCFMountIntent::StaticStruct(), &Intent, TEXT("LocationSlotRef"), OutError)
				|| !ImportSameTypeField(*TypeEntry, *FCFMountIntent::StaticStruct(), &Intent, TEXT("MountType"), OutError)
				|| !ImportSameTypeField(*SizeEntry, *FCFMountIntent::StaticStruct(), &Intent, TEXT("SizeLimit"), OutError)
				|| !ImportObjectAsSoftReference(*PresetEntry, *FCFMountIntent::StaticStruct(), &Intent, TEXT("DefaultEquipmentPresetData"), OutError)
				|| !ImportSameTypeField(*ExposedEntry, *FCFMountIntent::StaticStruct(), &Intent, TEXT("bExposedModule"), OutError))
			{
				return false;
			}
			InOutState.CopiedFieldCount += 6;
		}
		return true;
	}

	// Current Definition Snapshot 전체를 normal Legacy Pin과 hidden serialized passthrough로 deterministic partition합니다.
	bool BuildImportState(
		const FCFVehicleDefinitionSnapshot& Definition,
		FCFVehicleImportState& OutImportState,
		FString& OutError)
	{
		OutImportState = FCFVehicleImportState();
		OutImportState.ManageState = ECFVehicleManageState::LegacyImported;
		OutImportState.ImportedDefinitionHash = Definition.DefinitionHash;
		OutImportState.AdoptedGroups.Reset();

		for (const FCFVehicleFieldEntry& Entry : Definition.SortedFields)
		{
			const FCFVehicleFieldDescriptor* Descriptor = FindDescriptor(Entry.FieldPath);
			if (!Descriptor)
			{
				OutError = FString::Printf(TEXT("Current Definition field를 Frozen Registry에서 찾을 수 없습니다: %s"), *Entry.FieldPath.ToCanonicalString(true));
				return false;
			}

			// Existing exact field를 lossless baseline으로 보존할 override row입니다.
			FCFVehicleFieldOverride Override;
			Override.FieldPath = Entry.FieldPath;
			Override.OverrideValue = Entry.Value;
			Override.Reason = Descriptor->bLegacySerialized ? TEXT("Existing Definition hidden serialized compatibility baseline") : TEXT("Existing Definition imported pinned baseline");
			if (Descriptor->bLegacySerialized)
			{
				OutImportState.LegacySerializedFields.Add(MoveTemp(Override));
			}
			else
			{
				OutImportState.LegacyPinnedFields.Add(MoveTemp(Override));
			}
		}
		return true;
	}

	// Recipe Snapshot의 normal Legacy Pin count로 migration ManageState를 갱신합니다.
	void UpdateManageState(FCFVehicleImportState& ImportState)
	{
		if (ImportState.LegacyPinnedFields.IsEmpty())
		{
			ImportState.ManageState = ECFVehicleManageState::Managed;
		}
		else if (!ImportState.AdoptedGroups.IsEmpty())
		{
			ImportState.ManageState = ECFVehicleManageState::PartiallyManaged;
		}
		else
		{
			ImportState.ManageState = ECFVehicleManageState::LegacyImported;
		}
	}

		// Reviewed raw field/value를 Legacy Pin으로 deterministic upsert하고 해당 group의 완전 Adoption marker를 해제합니다.
	bool UpsertRawPins(
		FCFVehicleImportState& ImportState,
		const TArray<FCFVehicleFieldOverride>& RawPins,
		FString& OutError)
	{
		if (RawPins.IsEmpty())
		{
			OutError = TEXT("Preserve Raw 대상 field가 없습니다.");
			return false;
		}

		for (const FCFVehicleFieldOverride& RawPin : RawPins)
		{
			// Reviewed exact path를 소유하는 Frozen Registry descriptor입니다.
			const FCFVehicleFieldDescriptor* Descriptor = FindDescriptor(RawPin.FieldPath);
			if (!Descriptor || Descriptor->bLegacySerialized)
			{
				OutError = FString::Printf(TEXT("Preserve Raw가 허용되지 않는 field입니다: %s"), *RawPin.FieldPath.ToCanonicalString(true));
				return false;
			}
			if (RawPin.OverrideValue.PropertyTypeSignature.IsEmpty())
			{
				OutError = FString::Printf(TEXT("Preserve Raw typed value가 비어 있습니다: %s"), *RawPin.FieldPath.ToCanonicalString(true));
				return false;
			}

			// Same exact path의 기존 Legacy Pin index입니다.
			const FString CanonicalPath = RawPin.FieldPath.ToCanonicalString(true);
			const int32 ExistingPinIndex = ImportState.LegacyPinnedFields.IndexOfByPredicate([&CanonicalPath](const FCFVehicleFieldOverride& ExistingPin)
			{
				return ExistingPin.FieldPath.ToCanonicalString(true) == CanonicalPath;
			});
			if (ExistingPinIndex != INDEX_NONE)
			{
				ImportState.LegacyPinnedFields[ExistingPinIndex] = RawPin;
			}
			else
			{
				ImportState.LegacyPinnedFields.Add(RawPin);
			}

			ImportState.AdoptedGroups.Remove(Descriptor->AdoptionGroup);
		}

		ImportState.LegacyPinnedFields.Sort([](const FCFVehicleFieldOverride& Left, const FCFVehicleFieldOverride& Right)
		{
			return Left.FieldPath.ToCanonicalString(true) < Right.FieldPath.ToCanonicalString(true);
		});
		UpdateManageState(ImportState);
		OutError.Reset();
		return true;
	}

	// Movement derived gate Pin은 모든 underlying movement ownership group Pin이 해제된 뒤에만 자동 해제합니다.
	void ReleaseMovementDerivedGateIfReady(
		FCFVehicleImportState& ImportState,
		TArray<FCFVehicleFieldPath>* OutRemovedPaths)
	{
		auto IsMovementOwnershipGroup = [](const ECFVehicleAdoptGroup Group)
		{
			return Group == ECFVehicleAdoptGroup::Drivetrain
				|| Group == ECFVehicleAdoptGroup::Handling
				|| Group == ECFVehicleAdoptGroup::Performance
				|| Group == ECFVehicleAdoptGroup::WheelGeometry
				|| Group == ECFVehicleAdoptGroup::TechnicalHandling;
		};

		// 아직 남은 movement ownership Pin이 하나라도 있는지 여부입니다.
		const bool bHasMovementOwnershipPins = ImportState.LegacyPinnedFields.ContainsByPredicate([&IsMovementOwnershipGroup](const FCFVehicleFieldOverride& Override)
		{
			const FCFVehicleFieldDescriptor* Descriptor = FindDescriptor(Override.FieldPath);
			return Descriptor && IsMovementOwnershipGroup(Descriptor->AdoptionGroup);
		});
		if (bHasMovementOwnershipPins)
		{
			return;
		}

		for (int32 PinIndex = ImportState.LegacyPinnedFields.Num() - 1; PinIndex >= 0; --PinIndex)
		{
			const FCFVehicleFieldOverride& Pin = ImportState.LegacyPinnedFields[PinIndex];
			const FCFVehicleFieldDescriptor* Descriptor = FindDescriptor(Pin.FieldPath);
			if (Descriptor && Descriptor->AdoptionGroup == ECFVehicleAdoptGroup::DerivedState
				&& Pin.FieldPath.ToCanonicalString(true) == TEXT("VehicleMovementConfig.bUseMovementOverrides"))
			{
				if (OutRemovedPaths)
				{
					OutRemovedPaths->Add(Pin.FieldPath);
				}
				ImportState.LegacyPinnedFields.RemoveAt(PinIndex);
			}
		}
	}

	// Group preview용으로 exact group Legacy Pins를 virtual Recipe Snapshot에서 제거합니다.
	bool RemoveGroupPins(
		FCFVehicleRecipeSnapshot& ProspectiveRecipe,
		const ECFVehicleAdoptGroup AdoptionGroup,
		TArray<FCFVehicleFieldPath>& OutDirectSelectedPaths,
		TArray<FCFVehicleFieldPath>& OutAllRemovedPaths)
	{
		for (int32 PinIndex = ProspectiveRecipe.ImportState.LegacyPinnedFields.Num() - 1; PinIndex >= 0; --PinIndex)
		{
			const FCFVehicleFieldOverride& Pin = ProspectiveRecipe.ImportState.LegacyPinnedFields[PinIndex];
			const FCFVehicleFieldDescriptor* Descriptor = FindDescriptor(Pin.FieldPath);
			if (Descriptor && Descriptor->AdoptionGroup == AdoptionGroup)
			{
				OutDirectSelectedPaths.Add(Pin.FieldPath);
				OutAllRemovedPaths.Add(Pin.FieldPath);
				ProspectiveRecipe.ImportState.LegacyPinnedFields.RemoveAt(PinIndex);
			}
		}
		OutDirectSelectedPaths.Sort([](const FCFVehicleFieldPath& Left, const FCFVehicleFieldPath& Right)
		{
			return Left.ToCanonicalString(true) < Right.ToCanonicalString(true);
		});
		OutAllRemovedPaths = OutDirectSelectedPaths;
		ReleaseMovementDerivedGateIfReady(ProspectiveRecipe.ImportState, &OutAllRemovedPaths);
		OutAllRemovedPaths.Sort([](const FCFVehicleFieldPath& Left, const FCFVehicleFieldPath& Right)
		{
			return Left.ToCanonicalString(true) < Right.ToCanonicalString(true);
		});
		return !OutDirectSelectedPaths.IsEmpty();
	}

	// Exact field preview용으로 Legacy Pin 하나를 virtual Recipe Snapshot에서 제거합니다.
	bool RemoveFieldPin(
		FCFVehicleRecipeSnapshot& ProspectiveRecipe,
		const FCFVehicleFieldPath& FieldPath,
		TArray<FCFVehicleFieldPath>& OutDirectSelectedPaths,
		TArray<FCFVehicleFieldPath>& OutAllRemovedPaths)
	{
		const FString TargetPath = FieldPath.ToCanonicalString(true);
		for (int32 PinIndex = 0; PinIndex < ProspectiveRecipe.ImportState.LegacyPinnedFields.Num(); ++PinIndex)
		{
			if (ProspectiveRecipe.ImportState.LegacyPinnedFields[PinIndex].FieldPath.ToCanonicalString(true) == TargetPath)
			{
				OutDirectSelectedPaths.Add(FieldPath);
				OutAllRemovedPaths.Add(FieldPath);
				ProspectiveRecipe.ImportState.LegacyPinnedFields.RemoveAt(PinIndex);
				ReleaseMovementDerivedGateIfReady(ProspectiveRecipe.ImportState, &OutAllRemovedPaths);
				OutAllRemovedPaths.Sort([](const FCFVehicleFieldPath& Left, const FCFVehicleFieldPath& Right)
				{
					return Left.ToCanonicalString(true) < Right.ToCanonicalString(true);
				});
				return true;
			}
		}
		return false;
	}

	// Prospective Resolve에서 선택 Pin 각각이 Legacy source가 아닌 새 effective winner를 얻었는지 검사합니다.
	bool ValidateProspectiveWinners(
		const FCFVehicleResolveResult& ProspectiveResult,
		const TArray<FCFVehicleFieldPath>& DirectSelectedPaths,
		FString& OutBlockReason)
	{
		for (const FCFVehicleFieldPath& SelectedPath : DirectSelectedPaths)
		{
			const FString CanonicalPath = SelectedPath.ToCanonicalString(true);
			// Prospective exact field의 Source Trace입니다.
			const FCFVehicleSourceTrace* Trace = ProspectiveResult.PreviewSourceTrace.FindByPredicate([&CanonicalPath](const FCFVehicleSourceTrace& Candidate)
			{
				return Candidate.FieldPath.ToCanonicalString(true) == CanonicalPath;
			});
			if (!Trace || !Trace->Layers.IsValidIndex(Trace->EffectiveLayerIndex))
			{
				OutBlockReason = FString::Printf(TEXT("Adoption 뒤 새 Source가 없는 field입니다: %s"), *CanonicalPath);
				return false;
			}

			// Selected Pin을 제외한 prospective effective Source 종류입니다.
			const ECFVehicleSourceType SourceType = Trace->Layers[Trace->EffectiveLayerIndex].SourceType;
			if (SourceType == ECFVehicleSourceType::LegacyImportedPinnedBaseline || SourceType == ECFVehicleSourceType::LegacySerializedPassthrough)
			{
				OutBlockReason = FString::Printf(TEXT("Adoption 뒤에도 Legacy Source가 effective한 field입니다: %s"), *CanonicalPath);
				return false;
			}
		}
		OutBlockReason.Reset();
		return true;
	}

	// Current/Prospective immutable requests를 Resolve하고 preview commit eligibility를 계산합니다.
	bool ResolveAdoptionPreview(
		const FCFVehicleResolveRequest& CurrentRequest,
		FCFVehicleResolveRequest& ProspectiveRequest,
		const TArray<FCFVehicleFieldPath>& DirectSelectedPaths,
		FCFVehicleAdoptionPreview& OutPreview,
		FString& OutError)
	{
		if (!FCFVehicleResolver::Resolve(CurrentRequest, OutPreview.CurrentResolveResult))
		{
			OutError = TEXT("현재 Legacy Pin 유지 Resolve에서 internal Error가 발생했습니다.");
			return false;
		}

		if (!FCFVehicleSnapshotBuilder::BuildRecipeFingerprintFromSnapshot(ProspectiveRequest.Recipe, ProspectiveRequest.Recipe.RecipeFingerprint, OutError))
		{
			return false;
		}
		OutPreview.ProspectiveRecipeFingerprint = ProspectiveRequest.Recipe.RecipeFingerprint;

		if (!FCFVehicleResolver::Resolve(ProspectiveRequest, OutPreview.ProspectiveResolveResult))
		{
			OutPreview.BlockReason = TEXT("Prospective Adoption Resolve에서 internal Error가 발생했습니다.");
			OutPreview.bCanCommit = false;
			OutError.Reset();
			return true;
		}

		if (!ValidateProspectiveWinners(OutPreview.ProspectiveResolveResult, DirectSelectedPaths, OutPreview.BlockReason))
		{
			OutPreview.bCanCommit = false;
			OutError.Reset();
			return true;
		}

		OutPreview.bCanCommit = true;
		OutPreview.BlockReason.Reset();
		OutError.Reset();
		return true;
	}

	// Import/preview 입력 Recipe Snapshot fingerprint가 canonical semantic payload와 일치하는지 검사합니다.
	bool ValidateRequestRecipeFingerprint(
		const FCFVehicleRecipeSnapshot& RecipeSnapshot,
		FString& OutCanonicalFingerprint,
		FString& OutError)
	{
		if (!FCFVehicleSnapshotBuilder::BuildRecipeFingerprintFromSnapshot(RecipeSnapshot, OutCanonicalFingerprint, OutError))
		{
			return false;
		}
		if (RecipeSnapshot.RecipeFingerprint != OutCanonicalFingerprint)
		{
			OutError = FString::Printf(TEXT("Adoption request RecipeFingerprint가 snapshot semantic payload와 일치하지 않습니다. Request=%s Canonical=%s"), *RecipeSnapshot.RecipeFingerprint, *OutCanonicalFingerprint);
			return false;
		}
		return true;
	}
}

// Persistent Recipe를 수정하지 않고 exact Legacy partition/direct semantic candidate count를 계산합니다.
bool FCFVehicleImportService::PreviewDefinitionImport(
	const FCFVehicleDefinitionSnapshot& CurrentDefinition,
	FCFVehicleImportResult& OutResult,
	FString& OutError)
{
	OutResult = FCFVehicleImportResult();
	if (CurrentDefinition.DefinitionHash.IsEmpty())
	{
		OutError = TEXT("Existing Definition Snapshot hash가 비어 있습니다.");
		return false;
	}

	// Current Definition field set에서 다시 계산한 canonical hash입니다.
	FString RecomputedDefinitionHash;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionHashFromFields(CurrentDefinition.SortedFields, RecomputedDefinitionHash, OutError))
	{
		return false;
	}
	if (RecomputedDefinitionHash != CurrentDefinition.DefinitionHash)
	{
		OutError = FString::Printf(TEXT("Existing Definition Snapshot hash가 field set과 일치하지 않습니다. Snapshot=%s Recomputed=%s"), *CurrentDefinition.DefinitionHash, *RecomputedDefinitionHash);
		return false;
	}

	// Persistent Recipe에 쓰지 않는 exact import migration state입니다.
	FCFVehicleImportState PreviewImportState;
	if (!CFVehicleImportPrivate::BuildImportState(CurrentDefinition, PreviewImportState, OutError))
	{
		return false;
	}

	// Persistent Recipe 없이 direct semantic candidate copy 규칙만 실행하는 local state입니다.
	CFVehicleImportPrivate::FSemanticCandidateState SemanticState;
	if (!CFVehicleImportPrivate::CopyScalarSemanticCandidates(CurrentDefinition, SemanticState, OutError)
		|| !CFVehicleImportPrivate::CopyHardpointCandidates(CurrentDefinition, SemanticState, OutError)
		|| !CFVehicleImportPrivate::CopyMountCandidates(CurrentDefinition, SemanticState, OutError))
	{
		return false;
	}

	OutResult.ImportedDefinitionHash = CurrentDefinition.DefinitionHash;
	OutResult.LegacyPinnedFieldCount = PreviewImportState.LegacyPinnedFields.Num();
	OutResult.LegacySerializedFieldCount = PreviewImportState.LegacySerializedFields.Num();
	OutResult.SemanticCandidateFieldCount = SemanticState.CopiedFieldCount;
	OutError.Reset();
	return true;
}

// Current Definition Snapshot 전체를 Legacy Pin으로 보존하고 direct semantic candidate만 Recipe에 복사합니다.
bool FCFVehicleImportService::ImportDefinitionSnapshot(
	const FCFVehicleDefinitionSnapshot& CurrentDefinition,
	UCFVehicleRecipeData& Recipe,
	FCFVehicleImportResult& OutResult,
	FString& OutError)
{
	OutResult = FCFVehicleImportResult();
	if (CurrentDefinition.DefinitionHash.IsEmpty())
	{
		OutError = TEXT("Existing Definition Snapshot hash가 비어 있습니다.");
		return false;
	}

	// Current Definition field set에서 다시 계산한 canonical hash입니다.
	FString RecomputedDefinitionHash;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionHashFromFields(CurrentDefinition.SortedFields, RecomputedDefinitionHash, OutError))
	{
		return false;
	}
	if (RecomputedDefinitionHash != CurrentDefinition.DefinitionHash)
	{
		OutError = FString::Printf(TEXT("Existing Definition Snapshot hash가 field set과 일치하지 않습니다. Snapshot=%s Recomputed=%s"), *CurrentDefinition.DefinitionHash, *RecomputedDefinitionHash);
		return false;
	}

	if (!Recipe.ImportState.ImportedDefinitionHash.IsEmpty()
		|| !Recipe.ImportState.LegacyPinnedFields.IsEmpty()
		|| !Recipe.ImportState.LegacySerializedFields.IsEmpty())
	{
		OutError = TEXT("이미 Existing Definition import baseline이 있는 Recipe에는 Foundation Import를 덮어쓸 수 없습니다.");
		return false;
	}

	// Recipe에 적용하기 전 완전히 구성할 new migration state입니다.
	FCFVehicleImportState NewImportState;
	if (!CFVehicleImportPrivate::BuildImportState(CurrentDefinition, NewImportState, OutError))
	{
		return false;
	}

	// Recipe의 direct semantic block을 현재 값으로 시작하되 Profile/DrivingFeel/Advanced ownership은 건드리지 않는 local state입니다.
	CFVehicleImportPrivate::FSemanticCandidateState SemanticState;
	SemanticState.AssetIntent = Recipe.AssetIntent;
	SemanticState.MassIntent = Recipe.MassIntent;
	SemanticState.DurabilityIntent = Recipe.DurabilityIntent;
	SemanticState.DefaultDataIntent = Recipe.DefaultDataIntent;
	SemanticState.DriveStateMode = Recipe.DriveStateMode;
	if (!CFVehicleImportPrivate::CopyScalarSemanticCandidates(CurrentDefinition, SemanticState, OutError)
		|| !CFVehicleImportPrivate::CopyHardpointCandidates(CurrentDefinition, SemanticState, OutError)
		|| !CFVehicleImportPrivate::CopyMountCandidates(CurrentDefinition, SemanticState, OutError))
	{
		return false;
	}

	// Recipe-only Initial Import를 Undo 가능하게 묶는 transaction입니다.
	FScopedTransaction ImportTransaction(NSLOCTEXT("CarFightDataAuthoring", "ImportExistingVehicleDefinition", "기존 차량 Definition 가져오기"));
	Recipe.Modify();
	Recipe.AssetIntent = SemanticState.AssetIntent;
	Recipe.MassIntent = SemanticState.MassIntent;
	Recipe.DurabilityIntent = SemanticState.DurabilityIntent;
	Recipe.HardpointIntents = MoveTemp(SemanticState.HardpointIntents);
	Recipe.MountIntents = MoveTemp(SemanticState.MountIntents);
	Recipe.DefaultDataIntent = SemanticState.DefaultDataIntent;
	Recipe.DriveStateMode = SemanticState.DriveStateMode;
	Recipe.ImportState = MoveTemp(NewImportState);
	++Recipe.AuthoringRevision;
	Recipe.MarkPackageDirty();

	OutResult.ImportedDefinitionHash = CurrentDefinition.DefinitionHash;
	OutResult.LegacyPinnedFieldCount = Recipe.ImportState.LegacyPinnedFields.Num();
	OutResult.LegacySerializedFieldCount = Recipe.ImportState.LegacySerializedFields.Num();
	OutResult.SemanticCandidateFieldCount = SemanticState.CopiedFieldCount;
	OutError.Reset();
	return true;
}

// 선택 user-facing group의 Legacy Pin만 가상 제거해 Source/Diff/Validation preview를 계산합니다.
bool FCFVehicleImportService::PreviewGroupAdoption(
	const FCFVehicleResolveRequest& CurrentRequest,
	const ECFVehicleAdoptGroup AdoptionGroup,
	FCFVehicleAdoptionPreview& OutPreview,
	FString& OutError)
{
	OutPreview = FCFVehicleAdoptionPreview();
	OutPreview.Scope = ECFVehicleAdoptionScope::Group;
	OutPreview.AdoptionGroup = AdoptionGroup;
	if (AdoptionGroup == ECFVehicleAdoptGroup::DerivedState || AdoptionGroup == ECFVehicleAdoptGroup::LegacyTechnical)
	{
		OutError = TEXT("DerivedState와 LegacyTechnical은 normal group Adoption 대상이 아닙니다.");
		return false;
	}

	// Current request semantic payload에서 다시 계산한 baseline fingerprint입니다.
	FString BaselineFingerprint;
	if (!CFVehicleImportPrivate::ValidateRequestRecipeFingerprint(CurrentRequest.Recipe, BaselineFingerprint, OutError))
	{
		return false;
	}
	OutPreview.BaselineRecipeFingerprint = BaselineFingerprint;

	// Persistent Recipe를 수정하지 않는 prospective immutable request copy입니다.
	FCFVehicleResolveRequest ProspectiveRequest = CurrentRequest;
	// 실제 선택 group에 직접 속한 Pin paths입니다.
	TArray<FCFVehicleFieldPath> DirectSelectedPaths;
	if (!CFVehicleImportPrivate::RemoveGroupPins(ProspectiveRequest.Recipe, AdoptionGroup, DirectSelectedPaths, OutPreview.LegacyPinPathsToRemove))
	{
		OutError = TEXT("선택 Adoption Group에 제거할 Legacy Pin이 없습니다.");
		return false;
	}
	ProspectiveRequest.Recipe.ImportState.AdoptedGroups.Add(AdoptionGroup);
	CFVehicleImportPrivate::UpdateManageState(ProspectiveRequest.Recipe.ImportState);
	return CFVehicleImportPrivate::ResolveAdoptionPreview(CurrentRequest, ProspectiveRequest, DirectSelectedPaths, OutPreview, OutError);
}

// Source Trace에서 선택한 exact Legacy Pin 하나만 가상 제거하는 advanced preview를 계산합니다.
bool FCFVehicleImportService::PreviewFieldAdoption(
	const FCFVehicleResolveRequest& CurrentRequest,
	const FCFVehicleFieldPath& FieldPath,
	FCFVehicleAdoptionPreview& OutPreview,
	FString& OutError)
{
	OutPreview = FCFVehicleAdoptionPreview();
	OutPreview.Scope = ECFVehicleAdoptionScope::Field;
	OutPreview.AdoptionFieldPath = FieldPath;

	// Field-level Adoption 제한을 판정할 Frozen Registry descriptor입니다.
	const FCFVehicleFieldDescriptor* Descriptor = CFVehicleImportPrivate::FindDescriptor(FieldPath);
	if (!Descriptor)
	{
		OutError = TEXT("Field-level Adoption 대상이 Frozen Registry에 없습니다.");
		return false;
	}
	if (Descriptor->bIdentityField || Descriptor->bLegacySerialized || Descriptor->AdoptionGroup == ECFVehicleAdoptGroup::DerivedState)
	{
		OutError = TEXT("Identity, DerivedState 또는 Legacy Serialized field는 field-level Adoption 대상이 아닙니다.");
		return false;
	}

	// Current request semantic payload에서 다시 계산한 baseline fingerprint입니다.
	FString BaselineFingerprint;
	if (!CFVehicleImportPrivate::ValidateRequestRecipeFingerprint(CurrentRequest.Recipe, BaselineFingerprint, OutError))
	{
		return false;
	}
	OutPreview.BaselineRecipeFingerprint = BaselineFingerprint;

	// Persistent Recipe를 수정하지 않는 prospective immutable request copy입니다.
	FCFVehicleResolveRequest ProspectiveRequest = CurrentRequest;
	// 실제 선택 exact Pin path 하나입니다.
	TArray<FCFVehicleFieldPath> DirectSelectedPaths;
	if (!CFVehicleImportPrivate::RemoveFieldPin(ProspectiveRequest.Recipe, FieldPath, DirectSelectedPaths, OutPreview.LegacyPinPathsToRemove))
	{
		OutError = TEXT("선택 field가 현재 Recipe의 LegacyPinnedFields에 없습니다.");
		return false;
	}
	CFVehicleImportPrivate::UpdateManageState(ProspectiveRequest.Recipe.ImportState);
	return CFVehicleImportPrivate::ResolveAdoptionPreview(CurrentRequest, ProspectiveRequest, DirectSelectedPaths, OutPreview, OutError);
}

// Reviewed raw field/value를 transient Recipe Snapshot의 Legacy Pin ownership으로 upsert합니다.
bool FCFVehicleImportService::ApplyRawPinsToSnapshot(
	FCFVehicleRecipeSnapshot& RecipeSnapshot,
	const TArray<FCFVehicleFieldOverride>& RawPins,
	FString& OutError)
{
	return CFVehicleImportPrivate::UpsertRawPins(RecipeSnapshot.ImportState, RawPins, OutError);
}

// Reviewed raw field/value를 persistent Recipe의 Legacy Pin ownership으로 upsert합니다. Transaction/revision은 caller가 소유합니다.
bool FCFVehicleImportService::ApplyRawPins(
	UCFVehicleRecipeData& Recipe,
	const TArray<FCFVehicleFieldOverride>& RawPins,
	FString& OutError)
{
	return CFVehicleImportPrivate::UpsertRawPins(Recipe.ImportState, RawPins, OutError);
}

// Fresh Preview fingerprint를 검증한 뒤 Legacy Pin 제거/AdoptedGroups/revision만 Recipe에 transaction으로 반영합니다.
bool FCFVehicleImportService::CommitAdoption(
	UCFVehicleRecipeData& Recipe,
	const FCFVehicleAdoptionPreview& ApprovedPreview,
	FString& OutError)
{
	if (!ApprovedPreview.bCanCommit || ApprovedPreview.LegacyPinPathsToRemove.IsEmpty())
	{
		OutError = TEXT("승인 가능한 Adoption Preview가 아닙니다.");
		return false;
	}

	// Commit 직전 persistent Recipe의 fresh immutable Snapshot입니다.
	FCFVehicleRecipeSnapshot CurrentRecipeSnapshot;
	if (!FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(Recipe, CurrentRecipeSnapshot, OutError))
	{
		return false;
	}
	if (CurrentRecipeSnapshot.RecipeFingerprint != ApprovedPreview.BaselineRecipeFingerprint)
	{
		OutError = TEXT("Adoption Preview 이후 Recipe semantic payload가 변경되어 preview가 stale입니다. Fresh Preview가 필요합니다.");
		return false;
	}

	// Preview가 제거 대상으로 고정한 exact canonical paths입니다.
	TSet<FString> RemovalPaths;
	for (const FCFVehicleFieldPath& FieldPath : ApprovedPreview.LegacyPinPathsToRemove)
	{
		RemovalPaths.Add(FieldPath.ToCanonicalString(true));
	}

	for (const FString& RemovalPath : RemovalPaths)
	{
		if (!Recipe.ImportState.LegacyPinnedFields.ContainsByPredicate([&RemovalPath](const FCFVehicleFieldOverride& Pin)
		{
			return Pin.FieldPath.ToCanonicalString(true) == RemovalPath;
		}))
		{
			OutError = FString::Printf(TEXT("Adoption commit 대상 Legacy Pin이 현재 Recipe에서 사라졌습니다: %s"), *RemovalPath);
			return false;
		}
	}

	// Recipe-only ownership 전환을 Undo 가능하게 묶는 transaction입니다.
	FScopedTransaction AdoptionTransaction(NSLOCTEXT("CarFightDataAuthoring", "AdoptVehicleAuthoringSource", "차량 Authoring Source 채택"));
	Recipe.Modify();
	Recipe.ImportState.LegacyPinnedFields.RemoveAll([&RemovalPaths](const FCFVehicleFieldOverride& Pin)
	{
		return RemovalPaths.Contains(Pin.FieldPath.ToCanonicalString(true));
	});
	if (ApprovedPreview.Scope == ECFVehicleAdoptionScope::Group)
	{
		Recipe.ImportState.AdoptedGroups.Add(ApprovedPreview.AdoptionGroup);
	}
	CFVehicleImportPrivate::UpdateManageState(Recipe.ImportState);
	++Recipe.AuthoringRevision;
	Recipe.MarkPackageDirty();
	OutError.Reset();
	return true;
}
