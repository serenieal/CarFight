// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleSnapshotBuilder.cpp
// Version: v1.3.0
// Date: 2026-08-26
// Description: Recipe/Profile/Definition immutable Snapshot과 Builder-private ownership metadata를 포함한 deterministic fingerprint/hash 구현입니다.
// Changelog:
// - v1.3.0: CF-FQ-040 VB-P0-05에서 Profile Meta.OwnerRecipeId를 snapshot source metadata에 value-copy. Profile payload fingerprint에는 포함하지 않아 owner stale precondition과 payload stale precondition을 분리.
// - v1.2.0: DAUTH-P0-08G prospective Adoption Preview용 Recipe Snapshot fingerprint 공용 entry point 추가.
// - v1.1.0: DAUTH-P0-08F에서 Resolver/R15 readback이 Definition Snapshot과 동일 hash authority를 사용하도록 공용 entry point 추가.
// - v1.0.0: DAUTH-P0-08C Section 22.13/22.27 Snapshot Foundation 최초 구현.
// Migration:
// - Runtime UCFVehicleData, Content Asset, Apply 경로를 수정하지 않습니다.
// - 기존 Recipe/Definition hash format revision과 canonical serialization은 변경하지 않습니다.

#include "DataAuthoring/CFVehicleSnapshotBuilder.h"

#include "CFVehicleData.h"
#include "Containers/StringConv.h"
#include "DataAuthoring/CFDriveStateProfile.h"
#include "DataAuthoring/CFDrivetrainProfile.h"
#include "DataAuthoring/CFHandlingProfile.h"
#include "DataAuthoring/CFPerformanceProfile.h"
#include "DataAuthoring/CFVehicleBaseProfile.h"
#include "DataAuthoring/CFVehicleFieldCodec.h"
#include "DataAuthoring/CFVehicleFieldRegistry.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "Misc/SecureHash.h"
#include "UObject/StructOnScope.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

namespace CFVehicleSnapshotPrivate
{
	// Snapshot fingerprint/hash canonical payload format revision입니다.
	static constexpr int32 SnapshotHashFormatRevision = 1;

	// UTF-8 canonical payload를 lowercase MD5 hexadecimal digest로 변환합니다.
	FString HashUtf8Payload(const FString& Payload)
	{
		// TCHAR 표현과 분리된 canonical UTF-8 payload입니다.
		const FTCHARToUTF8 Utf8Payload(*Payload);
		// UTF-8 bytes의 deterministic digest를 계산할 MD5 state입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());
		// MD5 128-bit 결과 bytes입니다.
		uint8 Digest[16];
		Md5.Final(Digest);

		// Platform-independent lowercase hexadecimal 결과 문자열입니다.
		FString HexResult;
		HexResult.Reserve(32);
		// 한 nibble을 lowercase hex로 바꾸는 고정 문자표입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			HexResult.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			HexResult.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return HexResult;
	}

	// Delimiter 충돌 없이 canonical payload 조각을 추가하기 위해 label/문자수/value를 기록합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("\n");
	}

	// UStruct가 직접 선언한 reflected property를 이름순으로 수집합니다.
	TArray<const FProperty*> GetSortedDirectProperties(const UStruct& Struct)
	{
		// Canonical property ordering에 사용할 reflected property 목록입니다.
		TArray<const FProperty*> Properties;
		for (TFieldIterator<FProperty> PropertyIt(&Struct); PropertyIt; ++PropertyIt)
		{
			// 상위 struct/class에서 상속된 property는 현재 payload의 직접 field가 아닙니다.
			const FProperty* Property = *PropertyIt;
			if (Property && Property->GetOwnerStruct() == &Struct)
			{
				Properties.Add(Property);
			}
		}

		Properties.Sort([](const FProperty& Left, const FProperty& Right)
		{
			return Left.GetName() < Right.GetName();
		});
		return Properties;
	}

	// Reflected property 하나를 container 순서와 무관한 canonical text로 재귀 직렬화합니다.
	bool AppendCanonicalProperty(
		const FProperty& Property,
		const void* ValueAddress,
		FString& OutPayload,
		FString& OutError);

	// UScriptStruct value를 property 이름순 canonical text로 직렬화합니다.
	bool AppendCanonicalStruct(
		const UScriptStruct& Struct,
		const void* StructValueAddress,
		FString& OutPayload,
		FString& OutError)
	{
		if (!StructValueAddress)
		{
			OutError = FString::Printf(TEXT("Struct value address가 null입니다: %s"), *Struct.GetPathName());
			return false;
		}

		AppendToken(OutPayload, TEXT("StructType"), Struct.GetPathName());
		for (const FProperty* Property : GetSortedDirectProperties(Struct))
		{
			if (!Property)
			{
				continue;
			}

			AppendToken(OutPayload, TEXT("PropertyName"), Property->GetName());
			// 현재 struct instance 안에서 property value가 시작하는 주소입니다.
			const void* PropertyValueAddress = Property->ContainerPtrToValuePtr<void>(StructValueAddress);
			if (!AppendCanonicalProperty(*Property, PropertyValueAddress, OutPayload, OutError))
			{
				return false;
			}
		}
		return true;
	}

	// Reflected property 하나를 container 순서와 무관한 canonical text로 재귀 직렬화합니다.
	bool AppendCanonicalProperty(
		const FProperty& Property,
		const void* ValueAddress,
		FString& OutPayload,
		FString& OutError)
	{
		if (!ValueAddress)
		{
			OutError = FString::Printf(TEXT("Property value address가 null입니다: %s"), *Property.GetName());
			return false;
		}

		if (const FStructProperty* StructProperty = CastField<FStructProperty>(&Property))
		{
			if (!StructProperty->Struct)
			{
				OutError = FString::Printf(TEXT("Struct property type이 없습니다: %s"), *Property.GetName());
				return false;
			}
			return AppendCanonicalStruct(*StructProperty->Struct, ValueAddress, OutPayload, OutError);
		}

		if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(&Property))
		{
			if (!ArrayProperty->Inner)
			{
				OutError = FString::Printf(TEXT("Array inner property가 없습니다: %s"), *Property.GetName());
				return false;
			}

			// Array semantic order는 Recipe intent의 일부이므로 현재 logical order를 보존합니다.
			FScriptArrayHelper ArrayHelper(ArrayProperty, const_cast<void*>(ValueAddress));
			AppendToken(OutPayload, TEXT("ArrayCount"), FString::FromInt(ArrayHelper.Num()));
			for (int32 ElementIndex = 0; ElementIndex < ArrayHelper.Num(); ++ElementIndex)
			{
				AppendToken(OutPayload, TEXT("ArrayIndex"), FString::FromInt(ElementIndex));
				if (!AppendCanonicalProperty(*ArrayProperty->Inner, ArrayHelper.GetRawPtr(ElementIndex), OutPayload, OutError))
				{
					return false;
				}
			}
			return true;
		}

		if (const FSetProperty* SetProperty = CastField<FSetProperty>(&Property))
		{
			if (!SetProperty->ElementProp)
			{
				OutError = FString::Printf(TEXT("Set element property가 없습니다: %s"), *Property.GetName());
				return false;
			}

			// TSet iteration order를 fingerprint에 노출하지 않기 위해 element canonical text를 별도 수집합니다.
			FScriptSetHelper SetHelper(SetProperty, const_cast<void*>(ValueAddress));
			TArray<FString> ElementPayloads;
			for (int32 SparseIndex = 0; SparseIndex < SetHelper.GetMaxIndex(); ++SparseIndex)
			{
				if (!SetHelper.IsValidIndex(SparseIndex))
				{
					continue;
				}

				// Set element 하나의 독립 canonical payload입니다.
				FString ElementPayload;
				if (!AppendCanonicalProperty(*SetProperty->ElementProp, SetHelper.GetElementPtr(SparseIndex), ElementPayload, OutError))
				{
					return false;
				}
				ElementPayloads.Add(MoveTemp(ElementPayload));
			}
			ElementPayloads.Sort();
			AppendToken(OutPayload, TEXT("SetCount"), FString::FromInt(ElementPayloads.Num()));
			for (const FString& ElementPayload : ElementPayloads)
			{
				AppendToken(OutPayload, TEXT("SetElement"), ElementPayload);
			}
			return true;
		}

		if (const FMapProperty* MapProperty = CastField<FMapProperty>(&Property))
		{
			if (!MapProperty->KeyProp || !MapProperty->ValueProp)
			{
				OutError = FString::Printf(TEXT("Map key/value property가 없습니다: %s"), *Property.GetName());
				return false;
			}

			// TMap iteration order를 fingerprint에 노출하지 않기 위한 key/value pair canonical payload 목록입니다.
			FScriptMapHelper MapHelper(MapProperty, const_cast<void*>(ValueAddress));
			TArray<FString> PairPayloads;
			for (int32 SparseIndex = 0; SparseIndex < MapHelper.GetMaxIndex(); ++SparseIndex)
			{
				if (!MapHelper.IsValidIndex(SparseIndex))
				{
					continue;
				}

				// Map pair 하나의 독립 canonical payload입니다.
				FString PairPayload;
				if (!AppendCanonicalProperty(*MapProperty->KeyProp, MapHelper.GetKeyPtr(SparseIndex), PairPayload, OutError)
					|| !AppendCanonicalProperty(*MapProperty->ValueProp, MapHelper.GetValuePtr(SparseIndex), PairPayload, OutError))
				{
					return false;
				}
				PairPayloads.Add(MoveTemp(PairPayload));
			}
			PairPayloads.Sort();
			AppendToken(OutPayload, TEXT("MapCount"), FString::FromInt(PairPayloads.Num()));
			for (const FString& PairPayload : PairPayloads)
			{
				AppendToken(OutPayload, TEXT("MapPair"), PairPayload);
			}
			return true;
		}

		// Primitive/Name/Enum/Object/Class/Soft reference는 P0-08B Field Codec과 동일한 Unreal Export 규칙을 사용합니다.
		FCFVehicleFieldValue EncodedValue;
		if (!FCFVehicleFieldCodec::ExportValue(Property, ValueAddress, EncodedValue, OutError))
		{
			return false;
		}
		AppendToken(OutPayload, TEXT("Type"), EncodedValue.PropertyTypeSignature);
		AppendToken(OutPayload, TEXT("Value"), EncodedValue.CanonicalValueText);
		return true;
	}

	// Typed Profile payload 하나만 canonical hash하고 Meta display text/revision은 제외합니다.
	bool BuildProfilePayloadFingerprint(
		const UScriptStruct& PayloadStruct,
		const void* PayloadAddress,
		FString& OutFingerprint,
		FString& OutError)
	{
		// Profile hash format과 typed payload를 결합할 canonical input입니다.
		FString CanonicalPayload;
		AppendToken(CanonicalPayload, TEXT("Kind"), TEXT("VehicleProfilePayload"));
		AppendToken(CanonicalPayload, TEXT("HashFormatRevision"), FString::FromInt(SnapshotHashFormatRevision));
		if (!AppendCanonicalStruct(PayloadStruct, PayloadAddress, CanonicalPayload, OutError))
		{
			return false;
		}
		OutFingerprint = HashUtf8Payload(CanonicalPayload);
		return true;
	}

	// Override/Legacy Pin array를 storage order가 아닌 Stable Field Path 순으로 canonical 직렬화합니다.
	void AppendOverrideSet(
		FString& OutPayload,
		const TCHAR* Label,
		const TArray<FCFVehicleFieldOverride>& Overrides)
	{
		// Override order 자체는 Resolver 의미가 아니므로 canonical row 문자열을 정렬합니다.
		TArray<FString> CanonicalRows;
		CanonicalRows.Reserve(Overrides.Num());
		for (const FCFVehicleFieldOverride& Override : Overrides)
		{
			// Reason은 diagnostics metadata이므로 effective authoring fingerprint에서 제외합니다.
			FString Row;
			AppendToken(Row, TEXT("Path"), Override.FieldPath.ToCanonicalString(true));
			AppendToken(Row, TEXT("Type"), Override.OverrideValue.PropertyTypeSignature);
			AppendToken(Row, TEXT("Value"), Override.OverrideValue.CanonicalValueText);
			CanonicalRows.Add(MoveTemp(Row));
		}
		CanonicalRows.Sort();

		AppendToken(OutPayload, Label, FString::FromInt(CanonicalRows.Num()));
		for (const FString& Row : CanonicalRows)
		{
			AppendToken(OutPayload, TEXT("OverrideRow"), Row);
		}
	}

	// Recipe Snapshot에서 Section 22.27 Authoring Intent만 hash하고 AppliedState/diagnostic revision을 제외합니다.
	bool BuildRecipeFingerprint(
		const FCFVehicleRecipeSnapshot& Snapshot,
		FString& OutFingerprint,
		FString& OutError)
	{
		// Recipe hash format과 Frozen semantic input set을 결합할 canonical payload입니다.
		FString CanonicalPayload;
		AppendToken(CanonicalPayload, TEXT("Kind"), TEXT("VehicleRecipeIntent"));
		AppendToken(CanonicalPayload, TEXT("HashFormatRevision"), FString::FromInt(SnapshotHashFormatRevision));

		// 동일 helper를 재사용하면서 각 semantic block 사이에 명시 label boundary를 둡니다.
		auto AppendStructBlock = [&CanonicalPayload, &OutError](const TCHAR* Label, const UScriptStruct& Struct, const void* ValueAddress) -> bool
		{
			AppendToken(CanonicalPayload, TEXT("Block"), Label);
			return AppendCanonicalStruct(Struct, ValueAddress, CanonicalPayload, OutError);
		};

		if (!AppendStructBlock(TEXT("AssetIntent"), *FCFVehicleAssetIntent::StaticStruct(), &Snapshot.AssetIntent)
			|| !AppendStructBlock(TEXT("ProfileBindings"), *FCFVehicleProfileBindings::StaticStruct(), &Snapshot.ProfileBindings)
			|| !AppendStructBlock(TEXT("DrivingFeelIntent"), *FCFVehicleFeelIntent::StaticStruct(), &Snapshot.DrivingFeelIntent)
			|| !AppendStructBlock(TEXT("MassIntent"), *FCFVehicleMassIntent::StaticStruct(), &Snapshot.MassIntent)
			|| !AppendStructBlock(TEXT("DurabilityIntent"), *FCFVehicleDurabilityIntent::StaticStruct(), &Snapshot.DurabilityIntent))
		{
			return false;
		}

		AppendToken(CanonicalPayload, TEXT("HardpointCount"), FString::FromInt(Snapshot.HardpointIntents.Num()));
		for (const FCFHardpointIntent& HardpointIntent : Snapshot.HardpointIntents)
		{
			if (!AppendStructBlock(TEXT("HardpointIntent"), *FCFHardpointIntent::StaticStruct(), &HardpointIntent))
			{
				return false;
			}
		}

		AppendToken(CanonicalPayload, TEXT("MountCount"), FString::FromInt(Snapshot.MountIntents.Num()));
		for (const FCFMountIntent& MountIntent : Snapshot.MountIntents)
		{
			if (!AppendStructBlock(TEXT("MountIntent"), *FCFMountIntent::StaticStruct(), &MountIntent))
			{
				return false;
			}
		}

		if (!AppendStructBlock(TEXT("DefaultDataIntent"), *FCFVehicleDefaultIntent::StaticStruct(), &Snapshot.DefaultDataIntent)
			|| !AppendStructBlock(TEXT("WheelVisualIntent"), *FCFWheelVisualIntent::StaticStruct(), &Snapshot.WheelVisualIntent))
		{
			return false;
		}
		AppendToken(CanonicalPayload, TEXT("DriveStateMode"), FString::FromInt(static_cast<uint8>(Snapshot.DriveStateMode)));
		if (!AppendStructBlock(TEXT("AssetAdoption"), *FCFVehicleAssetAdoption::StaticStruct(), &Snapshot.AssetAdoption))
		{
			return false;
		}

		// Initial import baseline hash/ManageState 자체보다 실제 Legacy Pin/serialized ownership과 Adoption group이 Resolver에 영향을 줍니다.
		AppendOverrideSet(CanonicalPayload, TEXT("LegacyPinnedFields"), Snapshot.ImportState.LegacyPinnedFields);
		AppendOverrideSet(CanonicalPayload, TEXT("LegacySerializedFields"), Snapshot.ImportState.LegacySerializedFields);
		// TSet iteration order를 제거하기 위한 sorted Adoption Group 숫자 목록입니다.
		TArray<uint8> AdoptedGroups;
		AdoptedGroups.Reserve(Snapshot.ImportState.AdoptedGroups.Num());
		for (const ECFVehicleAdoptGroup Group : Snapshot.ImportState.AdoptedGroups)
		{
			AdoptedGroups.Add(static_cast<uint8>(Group));
		}
		AdoptedGroups.Sort();
		AppendToken(CanonicalPayload, TEXT("AdoptedGroupCount"), FString::FromInt(AdoptedGroups.Num()));
		for (const uint8 GroupValue : AdoptedGroups)
		{
			AppendToken(CanonicalPayload, TEXT("AdoptedGroup"), FString::FromInt(GroupValue));
		}

		AppendOverrideSet(CanonicalPayload, TEXT("AdvancedOverrides"), Snapshot.AdvancedOverrides);
		OutFingerprint = HashUtf8Payload(CanonicalPayload);
		return true;
	}

	// UStruct root 또는 nested struct에서 PropertyChain leaf와 실제 value 주소를 찾습니다.
	bool ResolvePropertyChain(
		const UStruct& RootStruct,
		const void* RootContainer,
		const TArray<FName>& PropertyChain,
		const FProperty*& OutLeafProperty,
		const void*& OutLeafValueAddress,
		FString& OutError)
	{
		if (!RootContainer || PropertyChain.IsEmpty())
		{
			OutError = TEXT("PropertyChain root 또는 chain이 비어 있습니다.");
			return false;
		}

		// 현재 property를 찾을 reflected struct/class입니다.
		const UStruct* CurrentStruct = &RootStruct;
		// 현재 property의 ContainerPtrToValuePtr 기준 주소입니다.
		const void* CurrentContainer = RootContainer;
		for (int32 ChainIndex = 0; ChainIndex < PropertyChain.Num(); ++ChainIndex)
		{
			// 현재 chain segment에 대응하는 reflected property입니다.
			const FProperty* Property = FindFProperty<FProperty>(CurrentStruct, PropertyChain[ChainIndex]);
			if (!Property)
			{
				OutError = FString::Printf(TEXT("Snapshot property를 찾을 수 없습니다: %s.%s"), *CurrentStruct->GetPathName(), *PropertyChain[ChainIndex].ToString());
				return false;
			}

			// 현재 container instance에서 property value가 시작하는 주소입니다.
			const void* PropertyValueAddress = Property->ContainerPtrToValuePtr<void>(CurrentContainer);
			if (ChainIndex == PropertyChain.Num() - 1)
			{
				OutLeafProperty = Property;
				OutLeafValueAddress = PropertyValueAddress;
				return true;
			}

			// Leaf 이전 segment는 nested USTRUCT만 허용합니다.
			const FStructProperty* StructProperty = CastField<FStructProperty>(Property);
			if (!StructProperty || !StructProperty->Struct)
			{
				OutError = FString::Printf(TEXT("Snapshot path 중간 segment가 Struct가 아닙니다: %s"), *Property->GetName());
				return false;
			}
			CurrentStruct = StructProperty->Struct;
			CurrentContainer = PropertyValueAddress;
		}

		OutError = TEXT("PropertyChain resolution이 leaf 없이 종료되었습니다.");
		return false;
	}

	// Exact path/value를 codec으로 읽어 Definition Snapshot entry에 추가합니다.
	bool AppendDefinitionEntry(
		const FCFVehicleFieldPath& ExactPath,
		const FProperty& LeafProperty,
		const void* LeafValueAddress,
		TArray<FCFVehicleFieldEntry>& OutEntries,
		FString& OutError)
	{
		// Registry-expanded exact Definition field entry입니다.
		FCFVehicleFieldEntry Entry;
		Entry.FieldPath = ExactPath;
		if (!FCFVehicleFieldCodec::ExportValue(LeafProperty, LeafValueAddress, Entry.Value, OutError))
		{
			return false;
		}
		OutEntries.Add(MoveTemp(Entry));
		return true;
	}

	// Stable selector path로 정렬하고 duplicate exact path를 차단합니다.
	bool FinalizeDefinitionEntries(
		TArray<FCFVehicleFieldEntry>& InOutEntries,
		FString& OutError)
	{
		InOutEntries.Sort([](const FCFVehicleFieldEntry& Left, const FCFVehicleFieldEntry& Right)
		{
			return Left.FieldPath.ToCanonicalString(true) < Right.FieldPath.ToCanonicalString(true);
		});

		// 이전 row의 canonical path와 비교해 duplicate stable selector collision을 검출합니다.
		FString PreviousPath;
		for (const FCFVehicleFieldEntry& Entry : InOutEntries)
		{
			// Missing selector는 Project Default pattern에서 wildcard로 canonicalize합니다.
			const FString CurrentPath = Entry.FieldPath.ToCanonicalString(true);
			if (!PreviousPath.IsEmpty() && PreviousPath == CurrentPath)
			{
				OutError = FString::Printf(TEXT("Definition Snapshot stable path가 중복됩니다: %s"), *CurrentPath);
				return false;
			}
			PreviousPath = CurrentPath;
		}
		return true;
	}

	// Sorted exact/pattern field set의 path/type/value를 deterministic Definition hash로 만듭니다.
	FString BuildDefinitionHash(const TArray<FCFVehicleFieldEntry>& SortedEntries)
	{
		// Definition hash format과 모든 canonical field entry를 결합할 payload입니다.
		FString CanonicalPayload;
		AppendToken(CanonicalPayload, TEXT("Kind"), TEXT("VehicleDefinitionFields"));
		AppendToken(CanonicalPayload, TEXT("HashFormatRevision"), FString::FromInt(SnapshotHashFormatRevision));
		AppendToken(CanonicalPayload, TEXT("FieldCount"), FString::FromInt(SortedEntries.Num()));
		for (const FCFVehicleFieldEntry& Entry : SortedEntries)
		{
			AppendToken(CanonicalPayload, TEXT("Path"), Entry.FieldPath.ToCanonicalString(true));
			AppendToken(CanonicalPayload, TEXT("Type"), Entry.Value.PropertyTypeSignature);
			AppendToken(CanonicalPayload, TEXT("Value"), Entry.Value.CanonicalValueText);
		}
		return HashUtf8Payload(CanonicalPayload);
	}

	// Profile UObject 하나의 source metadata와 typed payload를 Snapshot pair로 복사합니다.
	template <typename ProfileType, typename ProfileDataType>
	bool CopyProfileSnapshot(
		const ProfileType* Profile,
		FCFVehicleProfileSource& OutSource,
		ProfileDataType& OutData,
		FString& OutError)
	{
		OutSource = FCFVehicleProfileSource();
		OutData = ProfileDataType();
		if (!Profile)
		{
			return true;
		}

		OutSource.SourceObjectPath = FSoftObjectPath(Profile->GetPathName());
		OutSource.OwnerRecipeId = Profile->Meta.OwnerRecipeId;
		OutSource.AuthoringRevision = Profile->Meta.AuthoringRevision;
		OutData = Profile->Data;
		if (!BuildProfilePayloadFingerprint(*ProfileDataType::StaticStruct(), &OutData, OutSource.ProfileFingerprint, OutError))
		{
			return false;
		}
		return true;
	}
}

// Persistent Recipe를 Resolve-safe value copy와 deterministic Recipe fingerprint로 변환합니다.
bool FCFVehicleSnapshotBuilder::BuildRecipeSnapshot(
	const UCFVehicleRecipeData& Recipe,
	FCFVehicleRecipeSnapshot& OutSnapshot,
	FString& OutError)
{
	OutSnapshot = FCFVehicleRecipeSnapshot();
	OutSnapshot.RecipeId = Recipe.RecipeId;
	OutSnapshot.TargetVehicleDataPath = Recipe.TargetVehicleData.ToSoftObjectPath();
	OutSnapshot.VehicleArchetypeId = Recipe.VehicleArchetypeId;
	OutSnapshot.AssetIntent = Recipe.AssetIntent;
	OutSnapshot.ProfileBindings = Recipe.ProfileBindings;
	OutSnapshot.DrivingFeelIntent = Recipe.DrivingFeelIntent;
	OutSnapshot.MassIntent = Recipe.MassIntent;
	OutSnapshot.DurabilityIntent = Recipe.DurabilityIntent;
	OutSnapshot.HardpointIntents = Recipe.HardpointIntents;
	OutSnapshot.MountIntents = Recipe.MountIntents;
	OutSnapshot.DefaultDataIntent = Recipe.DefaultDataIntent;
	OutSnapshot.WheelVisualIntent = Recipe.WheelVisualIntent;
	OutSnapshot.DriveStateMode = Recipe.DriveStateMode;
	OutSnapshot.AssetAdoption = Recipe.AssetAdoption;
	OutSnapshot.AdvancedOverrides = Recipe.AdvancedOverrides;
	OutSnapshot.ImportState = Recipe.ImportState;
	OutSnapshot.AppliedState = Recipe.AppliedState;
	OutSnapshot.AuthoringRevision = Recipe.AuthoringRevision;

	if (!CFVehicleSnapshotPrivate::BuildRecipeFingerprint(OutSnapshot, OutSnapshot.RecipeFingerprint, OutError))
	{
		return false;
	}
	OutError.Reset();
	return true;
}

// 이미 resolve된 5개 typed Profile UObject를 source+payload Snapshot Set으로 복사합니다.
bool FCFVehicleSnapshotBuilder::BuildProfileSnapshotSet(
	const UCFVehicleBaseProfile* BaseProfile,
	const UCFDrivetrainProfile* DrivetrainProfile,
	const UCFHandlingProfile* HandlingProfile,
	const UCFPerformanceProfile* PerformanceProfile,
	const UCFDriveStateProfile* DriveStateProfile,
	FCFVehicleProfileSnapshotSet& OutSnapshot,
	FString& OutError)
{
	OutSnapshot = FCFVehicleProfileSnapshotSet();
	if (!CFVehicleSnapshotPrivate::CopyProfileSnapshot(BaseProfile, OutSnapshot.BaseSource, OutSnapshot.BaseData, OutError)
		|| !CFVehicleSnapshotPrivate::CopyProfileSnapshot(DrivetrainProfile, OutSnapshot.DrivetrainSource, OutSnapshot.DrivetrainData, OutError)
		|| !CFVehicleSnapshotPrivate::CopyProfileSnapshot(HandlingProfile, OutSnapshot.HandlingSource, OutSnapshot.HandlingData, OutError)
		|| !CFVehicleSnapshotPrivate::CopyProfileSnapshot(PerformanceProfile, OutSnapshot.PerformanceSource, OutSnapshot.PerformanceData, OutError)
		|| !CFVehicleSnapshotPrivate::CopyProfileSnapshot(DriveStateProfile, OutSnapshot.DriveStateSource, OutSnapshot.DriveStateData, OutError))
	{
		return false;
	}
	OutError.Reset();
	return true;
}

// Current UCFVehicleData를 Registry descriptor로 실제 Stable-ID element까지 확장한 exact Snapshot으로 읽습니다.
bool FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(
	const UCFVehicleData& Definition,
	FCFVehicleDefinitionSnapshot& OutSnapshot,
	FString& OutError)
{
	OutSnapshot = FCFVehicleDefinitionSnapshot();

	for (const FCFVehicleFieldDescriptor& Descriptor : FCFVehicleFieldRegistry::GetDescriptors())
	{
		const FCFVehicleFieldPath& PathPattern = Descriptor.StablePathPattern;
		if (PathPattern.CollectionPropertyName.IsNone())
		{
			// Scalar/nested descriptor가 가리키는 실제 leaf property입니다.
			const FProperty* LeafProperty = nullptr;
			// Scalar/nested descriptor가 가리키는 실제 value 주소입니다.
			const void* LeafValueAddress = nullptr;
			if (!CFVehicleSnapshotPrivate::ResolvePropertyChain(*Definition.GetClass(), &Definition, PathPattern.PropertyChain, LeafProperty, LeafValueAddress, OutError)
				|| !LeafProperty
				|| !CFVehicleSnapshotPrivate::AppendDefinitionEntry(PathPattern, *LeafProperty, LeafValueAddress, OutSnapshot.SortedFields, OutError))
			{
				return false;
			}
			continue;
		}

		// Stable-ID collection을 소유하는 UCFVehicleData top-level array property입니다.
		const FArrayProperty* ArrayProperty = FindFProperty<FArrayProperty>(Definition.GetClass(), PathPattern.CollectionPropertyName);
		const FStructProperty* InnerStructProperty = ArrayProperty ? CastField<FStructProperty>(ArrayProperty->Inner) : nullptr;
		if (!ArrayProperty || !InnerStructProperty || !InnerStructProperty->Struct)
		{
			OutError = FString::Printf(TEXT("Definition Snapshot collection schema가 유효하지 않습니다: %s"), *PathPattern.CollectionPropertyName.ToString());
			return false;
		}

		// Collection element identity를 읽을 FName stable selector property입니다.
		const FNameProperty* SelectorProperty = FindFProperty<FNameProperty>(InnerStructProperty->Struct, PathPattern.SelectorKeyPropertyName);
		if (!SelectorProperty)
		{
			OutError = FString::Printf(TEXT("Definition Snapshot selector가 FName property가 아닙니다: %s.%s"), *PathPattern.CollectionPropertyName.ToString(), *PathPattern.SelectorKeyPropertyName.ToString());
			return false;
		}

		// 실제 Definition instance 안의 array storage입니다.
		const void* ArrayValueAddress = ArrayProperty->ContainerPtrToValuePtr<void>(&Definition);
		FScriptArrayHelper ArrayHelper(ArrayProperty, const_cast<void*>(ArrayValueAddress));
		for (int32 ElementIndex = 0; ElementIndex < ArrayHelper.Num(); ++ElementIndex)
		{
			// 현재 Stable-ID array element의 struct storage입니다.
			const void* ElementAddress = ArrayHelper.GetRawPtr(ElementIndex);
			// Array index 대신 exact Stable-ID path에 사용할 selector value입니다.
			const FName SelectorValue = SelectorProperty->GetPropertyValue_InContainer(ElementAddress);
			// Registry wildcard pattern에 실제 selector value를 넣은 concrete path입니다.
			FCFVehicleFieldPath ExactPath = PathPattern;
			ExactPath.SelectorKeyValue = SelectorValue;

			// Element struct 내부에서 descriptor leaf를 찾습니다.
			const FProperty* LeafProperty = nullptr;
			// Element struct 내부의 실제 leaf value 주소입니다.
			const void* LeafValueAddress = nullptr;
			if (!CFVehicleSnapshotPrivate::ResolvePropertyChain(*InnerStructProperty->Struct, ElementAddress, PathPattern.PropertyChain, LeafProperty, LeafValueAddress, OutError)
				|| !LeafProperty
				|| !CFVehicleSnapshotPrivate::AppendDefinitionEntry(ExactPath, *LeafProperty, LeafValueAddress, OutSnapshot.SortedFields, OutError))
			{
				return false;
			}
		}
	}

	if (!CFVehicleSnapshotPrivate::FinalizeDefinitionEntries(OutSnapshot.SortedFields, OutError))
	{
		return false;
	}
	OutSnapshot.DefinitionHash = CFVehicleSnapshotPrivate::BuildDefinitionHash(OutSnapshot.SortedFields);
	OutError.Reset();
	return true;
}

// UObject를 다시 읽지 않고 Recipe Snapshot semantic payload의 deterministic fingerprint를 다시 계산합니다.
bool FCFVehicleSnapshotBuilder::BuildRecipeFingerprintFromSnapshot(
	const FCFVehicleRecipeSnapshot& RecipeSnapshot,
	FString& OutRecipeFingerprint,
	FString& OutError)
{
	if (!CFVehicleSnapshotPrivate::BuildRecipeFingerprint(RecipeSnapshot, OutRecipeFingerprint, OutError))
	{
		OutRecipeFingerprint.Reset();
		return false;
	}

	OutError.Reset();
	return true;
}

// Stable field entry set을 path 순으로 정규화한 뒤 Snapshot과 Resolver가 공유하는 deterministic Definition hash를 만듭니다.
bool FCFVehicleSnapshotBuilder::BuildDefinitionHashFromFields(
	const TArray<FCFVehicleFieldEntry>& FieldEntries,
	FString& OutDefinitionHash,
	FString& OutError)
{
	// 호출자 storage 순서와 무관하게 canonical path 정렬을 수행할 working copy입니다.
	TArray<FCFVehicleFieldEntry> SortedEntries = FieldEntries;
	if (!CFVehicleSnapshotPrivate::FinalizeDefinitionEntries(SortedEntries, OutError))
	{
		OutDefinitionHash.Reset();
		return false;
	}

	OutDefinitionHash = CFVehicleSnapshotPrivate::BuildDefinitionHash(SortedEntries);
	OutError.Reset();
	return true;
}

// UCFVehicleData C++ defaults와 default-constructed array element를 사용해 exact 117-pattern compatibility baseline을 만듭니다.
bool FCFVehicleSnapshotBuilder::BuildProjectCompatibilityDefaultSnapshot(
	FCFVehicleDefinitionSnapshot& OutSnapshot,
	FString& OutError)
{
	OutSnapshot = FCFVehicleDefinitionSnapshot();
	// Runtime schema를 수정하지 않고 C++ member initializer 값을 읽을 UCFVehicleData CDO입니다.
	const UCFVehicleData* DefaultDefinition = GetDefault<UCFVehicleData>();
	if (!DefaultDefinition)
	{
		OutError = TEXT("UCFVehicleData CDO를 가져올 수 없습니다.");
		return false;
	}

	for (const FCFVehicleFieldDescriptor& Descriptor : FCFVehicleFieldRegistry::GetDescriptors())
	{
		const FCFVehicleFieldPath& PathPattern = Descriptor.StablePathPattern;
		if (PathPattern.CollectionPropertyName.IsNone())
		{
			// CDO scalar/nested descriptor가 가리키는 reflected leaf property입니다.
			const FProperty* LeafProperty = nullptr;
			// CDO scalar/nested descriptor가 가리키는 default value 주소입니다.
			const void* LeafValueAddress = nullptr;
			if (!CFVehicleSnapshotPrivate::ResolvePropertyChain(*DefaultDefinition->GetClass(), DefaultDefinition, PathPattern.PropertyChain, LeafProperty, LeafValueAddress, OutError)
				|| !LeafProperty
				|| !CFVehicleSnapshotPrivate::AppendDefinitionEntry(PathPattern, *LeafProperty, LeafValueAddress, OutSnapshot.SortedFields, OutError))
			{
				return false;
			}
			continue;
		}

		// Project Default에서 empty array여도 element C++ defaults를 읽기 위한 top-level array schema입니다.
		const FArrayProperty* ArrayProperty = FindFProperty<FArrayProperty>(DefaultDefinition->GetClass(), PathPattern.CollectionPropertyName);
		const FStructProperty* InnerStructProperty = ArrayProperty ? CastField<FStructProperty>(ArrayProperty->Inner) : nullptr;
		if (!ArrayProperty || !InnerStructProperty || !InnerStructProperty->Struct)
		{
			OutError = FString::Printf(TEXT("Project Default collection schema가 유효하지 않습니다: %s"), *PathPattern.CollectionPropertyName.ToString());
			return false;
		}

		// 실제 CDO array를 변경하지 않고 USTRUCT C++ default initializer만 가진 transient stack storage입니다.
		FStructOnScope DefaultElement(InnerStructProperty->Struct);
		const void* DefaultElementAddress = DefaultElement.GetStructMemory();
		// Default element 내부 descriptor leaf property입니다.
		const FProperty* LeafProperty = nullptr;
		// Default element 내부 descriptor leaf C++ default value 주소입니다.
		const void* LeafValueAddress = nullptr;
		if (!CFVehicleSnapshotPrivate::ResolvePropertyChain(*InnerStructProperty->Struct, DefaultElementAddress, PathPattern.PropertyChain, LeafProperty, LeafValueAddress, OutError)
			|| !LeafProperty
			|| !CFVehicleSnapshotPrivate::AppendDefinitionEntry(PathPattern, *LeafProperty, LeafValueAddress, OutSnapshot.SortedFields, OutError))
		{
			return false;
		}
	}

	if (!CFVehicleSnapshotPrivate::FinalizeDefinitionEntries(OutSnapshot.SortedFields, OutError))
	{
		return false;
	}
	if (OutSnapshot.SortedFields.Num() != FCFVehicleFieldRegistry::ExpectedLeafPatternCount)
	{
		OutError = FString::Printf(
			TEXT("Project Compatibility Default Snapshot count mismatch. Expected=%d Actual=%d"),
			FCFVehicleFieldRegistry::ExpectedLeafPatternCount,
			OutSnapshot.SortedFields.Num());
		return false;
	}

	OutSnapshot.DefinitionHash = CFVehicleSnapshotPrivate::BuildDefinitionHash(OutSnapshot.SortedFields);
	OutError.Reset();
	return true;
}
