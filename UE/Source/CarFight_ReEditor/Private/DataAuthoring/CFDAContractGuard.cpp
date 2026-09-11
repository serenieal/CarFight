// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAContractGuard.cpp
// Version: v1.6.0
// Date: 2026-09-10
// Description: CF-FQ-050 Missile compatibility facade와 CF-FQ-051 per-TypeKey DACE common 구현입니다.
// Changelog:
// - v1.6.0: CF-FQ-053 VDR-P0-01에서 nested USTRUCT authored field를 Type-neutral하게 재귀 관측하는 additive Reflection seam을 추가했습니다. 기존 direct/Missile facade 의미는 유지합니다.
// - v1.5.0: AmmoData SourceShape bootstrap을 위해 int32 authored property의 stable Reflection kind `Int`를 추가했습니다. 기존 Missile descriptor/signature에는 영향이 없습니다.
// - v1.4.0: DAO-P0-04 correction에서 provider-parameterized descriptor/revision/history/canonical Staging seam, Array element JSON observation과 scalar SoftObject target-class Reflection을 추가했습니다. 기존 Missile facade/accepted history는 보존합니다.
// - v1.3.0: canonical Product Staging exact3 read-only strict parse, Resolution/Evidence validation, Staging/Product Pending machine state와 accepted append/Current promotion gate를 추가했습니다.
// - v1.2.1: Source/Mapping-only change의 safe 여부 자동 추론을 제거하고 explicit NoMigration structural gate로 전환했습니다. no-delta stale declaration을 차단하고 accepted snapshot four component signature의 canonical SHA-256 integrity를 fail-closed했습니다.
// - v1.2.0: Combined candidate contract signature, CurrentChangeDeclaration binding, Schema/Adapter revision bump guard, safe native refactor explicit NoMigration와 accepted snapshot chain/monotonic validation을 추가.
// - v1.1.1: Array/Set/Map Reflection의 element/key/value를 recursive stable type token으로 ReflectedTypePath에 포함해 outer container shape가 같아도 inner type drift를 fail-closed 검출하도록 보강.
// - v1.1.0: Native Reflection Source observation, Source/Adapter exact comparison, mapping 양방향 coverage, serializer JSON shape, fingerprint token, materializer-extractor semantic roundtrip validator를 추가.
// - v1.0.1: UE FString 기본 ordering에 의존하지 않고 descriptor canonical row를 case-sensitive ordinal 비교로 명시 정렬해 accepted signature가 incidental case-insensitive ordering에 종속되지 않도록 교정.
// - v1.0.0: SourceShape/AdapterShape/SourceAdapterMapping/SemanticContract exact descriptor와 SHA-256 signature 계산, bootstrap baseline validation을 최초 추가.
// Migration:
// - Descriptor는 CF-FQ-050 developer guard authority이며 기존 CF-FQ-049 parser/serializer/materializer implementation을 generic writer로 교체하지 않습니다.
// - Reflection/coverage 검증은 read-only memory state만 사용하며 Product Sync/Apply/Save 또는 canonical Staging write를 호출하지 않습니다.
// - v1.1.1부터 새 container authored property를 descriptor에 추가할 때 ReflectedTypePath는 Reflection이 생성하는 recursive Array<...>/Set<...>/Map<...,...> token과 exact 일치해야 합니다.
// - v1.2.0부터 contract 변화는 latest accepted SnapshotId와 Guard 계산 CandidateContractSignature에 결속된 explicit declaration이 필요합니다. 현재 baseline이 accepted latest와 같으면 declaration은 nullptr로 유지합니다.
// - v1.2.1부터 NoMigration은 Source/Mapping-only category를 Guard가 자동 safe 판정한 결과가 아니라 개발자가 명시한 safe 판단입니다. accepted history의 four component signature는 lowercase canonical sha256 형식이어야 합니다.
// - v1.3.0부터 canonical Product Staging은 disk read + production strict parser만 사용합니다. Guard는 파일/UObject/package를 수정하거나 SyncProduct/ApplyReviewed/SavePackage를 호출하지 않습니다.
// - v1.4.0 common seam은 accepted snapshot을 생성/append하지 않습니다. AmmoData bootstrap은 후속 구현이며 Missile accepted chain과 Product exact3는 그대로 유지합니다.
// - v1.5.0부터 FIntProperty는 `Int` stable token으로 Reflection descriptor에 기록합니다. Missile accepted baseline을 rebaseline하지 않습니다.
// - v1.6.0부터 recursive Reflection seam은 nested FStructProperty만 dot-path로 펼치고 Array/Set/Map element 및 Object/SoftObject reference는 따라가지 않습니다. 기존 direct helper와 Missile facade 결과는 변경하지 않습니다.

#include "CFDAContractGuard.h"

#include "CFDAMissileProvider.h"
#include "CFDATypeDispatch.h"
#include "CFMissileGuidePresetData.h"
#include "CFMissileGuideTypes.h"
#include "Containers/StringConv.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/UnrealType.h"

#define UI CF_OPENSSL_UI
THIRD_PARTY_INCLUDES_START
#include <openssl/evp.h>
THIRD_PARTY_INCLUDES_END
#undef UI

namespace CFDAContractGuardPrivate
{
	// Pilot Source root class path입니다.
	static constexpr TCHAR SourceRootClassPath[] = TEXT("/Script/CarFight_Re.CFMissileGuidePresetData");
	// Nested Guidance config struct path입니다.
	static constexpr TCHAR GuideConfigStructPath[] = TEXT("/Script/CarFight_Re.CFMissileGuideConfig");
	// CarFight main_game root를 `<main_game>/UE/` ProjectDir의 부모로 resolve합니다.
	FString GetMainGameRoot()
	{
		// CarFight .uproject가 위치한 `<main_game>/UE/` directory입니다.
		const FString UnrealProjectRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
		// canonical Staging root가 위치한 `<main_game>/` directory입니다.
		FString MainGameRoot = FPaths::ConvertRelativePathToFull(FPaths::Combine(UnrealProjectRoot, TEXT("..")));
		FPaths::NormalizeDirectoryName(MainGameRoot);
		return MainGameRoot;
	}

	// Nested Guidance config 한 field의 descriptor seed입니다.
	struct FConfigFieldSpec
	{
		// Source/JSON field name입니다.
		const TCHAR* Name = TEXT("");
		// Source property kind입니다.
		const TCHAR* PropertyKind = TEXT("");
		// Enum 등 reflected type path입니다.
		const TCHAR* ReflectedTypePath = TEXT("");
		// JSON value kind입니다.
		const TCHAR* JsonValueKind = TEXT("");
		// Adapter representation kind입니다.
		const TCHAR* RepresentationKind = TEXT("");
		// Numeric field만 가지는 range semantic policy입니다.
		const TCHAR* RangePolicy = TEXT("");
	};

	// Serializer JSON tree에서 실제 관측한 field kind입니다.
	struct FObservedJsonField
	{
		// String/Number/Boolean/Object/Null actual JSON kind입니다.
		FString JsonValueKind;
		// JSON null value인지 나타냅니다.
		bool bNull = false;
	};

	// Current nested config exact26 field seed를 반환합니다.
	const TArray<FConfigFieldSpec>& GetConfigFieldSpecs()
	{
		// Source/parser/serializer/fingerprint current exact26을 표현하는 immutable seed입니다.
		static const TArray<FConfigFieldSpec> Specs =
		{
			{TEXT("bUseGuidance"), TEXT("Bool"), TEXT(""), TEXT("Boolean"), TEXT("Bool"), TEXT("")},
			{TEXT("GuideMode"), TEXT("Enum"), TEXT("/Script/CarFight_Re.ECFMissileGuideMode"), TEXT("String"), TEXT("EnumNameToken"), TEXT("")},
			{TEXT("LostTargetPolicy"), TEXT("Enum"), TEXT("/Script/CarFight_Re.ECFMissileLostTargetPolicy"), TEXT("String"), TEXT("EnumNameToken"), TEXT("")},
			{TEXT("NavigationConstant"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..10")},
			{TEXT("MaximumTurnRateDegPerSec"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..720")},
			{TEXT("MaximumLateralAccelerationCmPerSecSq"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..1000000")},
			{TEXT("GuidanceResponseTimeSeconds"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0.001..10")},
			{TEXT("MinimumGuidanceSpeedCmPerSec"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..1000000")},
			{TEXT("SeekerFieldOfViewDeg"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..360")},
			{TEXT("LockBreakAngleDeg"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..180")},
			{TEXT("TargetLostGraceTimeSeconds"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..30")},
			{TEXT("SeekerModel"), TEXT("Enum"), TEXT("/Script/CarFight_Re.ECFMissileSeekerModel"), TEXT("String"), TEXT("EnumNameToken"), TEXT("")},
			{TEXT("TargetObservationMode"), TEXT("Enum"), TEXT("/Script/CarFight_Re.ECFMissileTargetObservationMode"), TEXT("String"), TEXT("EnumNameToken"), TEXT("")},
			{TEXT("GuidanceLaw"), TEXT("Enum"), TEXT("/Script/CarFight_Re.ECFMissileGuidanceLaw"), TEXT("String"), TEXT("EnumNameToken"), TEXT("")},
			{TEXT("GuidanceActivationMode"), TEXT("Enum"), TEXT("/Script/CarFight_Re.ECFMissileGuidanceActivationMode"), TEXT("String"), TEXT("EnumNameToken"), TEXT("")},
			{TEXT("GuidanceActivationDelaySeconds"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..30")},
			{TEXT("GuidanceActivationDistanceCm"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..1000000")},
			{TEXT("LeadTimeSeconds"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..10")},
			{TEXT("MaxLeadDistanceCm"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..1000000")},
			{TEXT("ReacquisitionMode"), TEXT("Enum"), TEXT("/Script/CarFight_Re.ECFMissileReacquisitionMode"), TEXT("String"), TEXT("EnumNameToken"), TEXT("")},
			{TEXT("TargetObservationIntervalSeconds"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0.001..10")},
			{TEXT("TargetVelocityEstimateResponseTimeSeconds"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0.001..10")},
			{TEXT("AcquisitionConeHalfAngleDeg"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..180")},
			{TEXT("TrackingConeHalfAngleDeg"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..180")},
			{TEXT("ReacquisitionConeHalfAngleDeg"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..180")},
			{TEXT("ReacquisitionTimeSeconds"), TEXT("Float"), TEXT(""), TEXT("Number"), TEXT("Float"), TEXT("0..30")}
		};
		return Specs;
	}

	// Developer guard result에 한 fail-closed issue를 추가합니다.
	void AddIssue(FCFDAContractGuardResult& InOutResult, const ECFDAContractIssueCode Code, const FString& FieldPath, const FString& Message)
	{
		// 새 developer-only issue입니다.
		FCFDAContractGuardIssue Issue;
		Issue.Code = Code;
		Issue.FieldPath = FieldPath;
		Issue.Message = Message;
		InOutResult.Issues.Add(MoveTemp(Issue));
		InOutResult.bPassed = false;
	}

	// Accepted contract component가 lowercase canonical sha256:<64hex> 형식인지 확인합니다.
	bool IsCanonicalSha256Signature(const FString& Signature)
	{
		if (Signature.Len() != 71 || !Signature.StartsWith(TEXT("sha256:"), ESearchCase::CaseSensitive))
		{
			return false;
		}
		// SHA-256 digest hex 영역을 검사하는 문자 index입니다.
		for (int32 CharacterIndex = 7; CharacterIndex < Signature.Len(); ++CharacterIndex)
		{
			// 현재 검사하는 digest 문자입니다.
			const TCHAR Character = Signature[CharacterIndex];
			// 0~9 decimal digit 여부입니다.
			const bool bDecimalDigit = Character >= static_cast<TCHAR>('0') && Character <= static_cast<TCHAR>('9');
			// a~f lowercase hex 여부입니다.
			const bool bLowercaseHex = Character >= static_cast<TCHAR>('a') && Character <= static_cast<TCHAR>('f');
			if (!bDecimalDigit && !bLowercaseHex)
			{
				return false;
			}
		}
		return true;
	}

	// Accepted snapshot 한 component signature의 필수 canonical integrity를 fail-closed 검증합니다.
	void ValidateAcceptedComponentSignature(FCFDAContractGuardResult& InOutResult, const FString& SnapshotId, const FString& FieldName, const FString& Signature)
	{
		if (!IsCanonicalSha256Signature(Signature))
		{
			AddIssue(InOutResult, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, FieldName, FString::Printf(TEXT("Accepted snapshot %s의 %s가 canonical sha256:<64 lowercase hex> 형식이 아닙니다."), *SnapshotId, *FieldName));
		}
	}

	// 다른 validator 결과의 issue를 target 결과에 보존합니다.
	void AppendIssues(FCFDAContractGuardResult& InOutTarget, const FCFDAContractGuardResult& Source)
	{
		for (const FCFDAContractGuardIssue& Issue : Source.Issues)
		{
			InOutTarget.Issues.Add(Issue);
		}
		InOutTarget.bPassed = InOutTarget.Issues.IsEmpty();
	}

	// Case-sensitive ordinal deterministic ordering을 적용합니다.
	void SortRowsOrdinal(TArray<FString>& Rows)
	{
		Rows.Sort([](const FString& Left, const FString& Right)
		{
			return Left.Compare(Right, ESearchCase::CaseSensitive) < 0;
		});
	}

	// 두 FString set이 exact same member set인지 확인합니다.
	bool AreStringSetsEqual(const TSet<FString>& Left, const TSet<FString>& Right)
	{
		if (Left.Num() != Right.Num()) return false;
		for (const FString& Value : Left)
		{
			if (!Right.Contains(Value)) return false;
		}
		return true;
	}

	// Source descriptor 한 행을 deterministic canonical text로 변환합니다.
	FString BuildSourceRow(const FCFDASourceFieldDescriptor& Descriptor)
	{
		return FString::Printf(TEXT("%s\t%s\t%s\t%s\t%s\t%s"), *Descriptor.SourceRootClassPath, *Descriptor.SourcePropertyPath, *Descriptor.PropertyKind, *Descriptor.ReflectedTypePath, *Descriptor.ContainerKind, *Descriptor.NestedStructPath);
	}

	// Adapter descriptor 한 행을 deterministic canonical text로 변환합니다.
	FString BuildAdapterRow(const FCFDAAdapterFieldDescriptor& Descriptor)
	{
		return FString::Printf(TEXT("%s\t%s\t%s\t%s\t%s\t%s"), *Descriptor.AdapterJsonPath, *Descriptor.JsonValueKind, *Descriptor.PresencePolicy, *Descriptor.NullPolicy, *Descriptor.RepresentationKind, *Descriptor.SemanticRole);
	}

	// UTF-8 canonical text를 SHA-256 protocol fingerprint로 변환합니다.
	bool HashCanonicalText(const FString& CanonicalText, FString& OutSignature, FString& OutError)
	{
		// Canonical UTF-8 bytes입니다.
		FTCHARToUTF8 CanonicalUtf8(*CanonicalText);
		// OpenSSL EVP가 채울 SHA-256 결과입니다.
		FSHA256Signature Signature;
		// OpenSSL EVP가 반환하는 digest byte 수입니다.
		unsigned int DigestLength = 0;
		// Empty text에서도 non-null input pointer를 유지하는 sentinel입니다.
		const uint8 EmptyInputByte = 0;
		// 실제 hash input pointer입니다.
		const uint8* HashData = CanonicalUtf8.Length() > 0 ? reinterpret_cast<const uint8*>(CanonicalUtf8.Get()) : &EmptyInputByte;
		// Portable SHA-256 실행 결과입니다.
		const int32 DigestResult = EVP_Digest(HashData, static_cast<size_t>(CanonicalUtf8.Length()), Signature.Signature, &DigestLength, EVP_sha256(), nullptr);
		if (DigestResult != 1 || DigestLength != UE_ARRAY_COUNT(Signature.Signature))
		{
			OutSignature.Reset();
			OutError = TEXT("DACE contract SHA-256 signature 생성에 실패했습니다.");
			return false;
		}
		OutSignature = TEXT("sha256:") + Signature.ToString().ToLower();
		OutError.Reset();
		return true;
	}

	// Canonical row 배열을 sort/join/hash합니다.
	bool HashRows(TArray<FString> Rows, FString& OutSignature, FString& OutError)
	{
		SortRowsOrdinal(Rows);
		// Stable case-sensitive ordinal row order로 결합한 canonical descriptor text입니다.
		const FString CanonicalText = FString::Join(Rows, TEXT("\n"));
		return HashCanonicalText(CanonicalText, OutSignature, OutError);
	}

	// Reflection property의 stable property kind를 반환합니다.
	FString GetPropertyKind(const FProperty& Property)
	{
		if (CastField<const FBoolProperty>(&Property) != nullptr) return TEXT("Bool");
		if (CastField<const FFloatProperty>(&Property) != nullptr) return TEXT("Float");
		if (CastField<const FDoubleProperty>(&Property) != nullptr) return TEXT("Double");
		if (CastField<const FIntProperty>(&Property) != nullptr) return TEXT("Int");
		if (CastField<const FEnumProperty>(&Property) != nullptr) return TEXT("Enum");
		if (const FByteProperty* ByteProperty = CastField<const FByteProperty>(&Property)) return ByteProperty->Enum != nullptr ? TEXT("Enum") : TEXT("Byte");
		if (CastField<const FNameProperty>(&Property) != nullptr) return TEXT("Name");
		if (CastField<const FTextProperty>(&Property) != nullptr) return TEXT("Text");
		if (CastField<const FStructProperty>(&Property) != nullptr) return TEXT("Struct");
		if (CastField<const FSoftObjectProperty>(&Property) != nullptr) return TEXT("SoftObject");
		if (CastField<const FObjectPropertyBase>(&Property) != nullptr) return TEXT("Object");
		if (CastField<const FArrayProperty>(&Property) != nullptr) return TEXT("Array");
		if (CastField<const FSetProperty>(&Property) != nullptr) return TEXT("Set");
		if (CastField<const FMapProperty>(&Property) != nullptr) return TEXT("Map");
		return Property.GetClass()->GetName();
	}

	// Container 내부까지 재귀적으로 포함하는 stable reflected property type token을 생성합니다.
	FString BuildReflectedPropertyTypeToken(const FProperty& Property)
	{
		if (const FArrayProperty* ArrayProperty = CastField<const FArrayProperty>(&Property))
		{
			// Array element property입니다.
			const FProperty* InnerProperty = ArrayProperty->Inner;
			return InnerProperty != nullptr
				? FString::Printf(TEXT("Array<%s>"), *BuildReflectedPropertyTypeToken(*InnerProperty))
				: TEXT("Array<?>");
		}
		if (const FSetProperty* SetProperty = CastField<const FSetProperty>(&Property))
		{
			// Set element property입니다.
			const FProperty* ElementProperty = SetProperty->ElementProp;
			return ElementProperty != nullptr
				? FString::Printf(TEXT("Set<%s>"), *BuildReflectedPropertyTypeToken(*ElementProperty))
				: TEXT("Set<?>");
		}
		if (const FMapProperty* MapProperty = CastField<const FMapProperty>(&Property))
		{
			// Map key property입니다.
			const FProperty* KeyProperty = MapProperty->KeyProp;
			// Map value property입니다.
			const FProperty* ValueProperty = MapProperty->ValueProp;
			const FString KeyToken = KeyProperty != nullptr ? BuildReflectedPropertyTypeToken(*KeyProperty) : TEXT("?");
			const FString ValueToken = ValueProperty != nullptr ? BuildReflectedPropertyTypeToken(*ValueProperty) : TEXT("?");
			return FString::Printf(TEXT("Map<%s,%s>"), *KeyToken, *ValueToken);
		}
		if (const FEnumProperty* EnumProperty = CastField<const FEnumProperty>(&Property))
		{
			return EnumProperty->GetEnum() != nullptr
				? FString(TEXT("Enum:")) + EnumProperty->GetEnum()->GetPathName()
				: TEXT("Enum:?");
		}
		if (const FByteProperty* ByteProperty = CastField<const FByteProperty>(&Property))
		{
			return ByteProperty->Enum != nullptr
				? FString(TEXT("Enum:")) + ByteProperty->Enum->GetPathName()
				: TEXT("Byte");
		}
		if (const FStructProperty* StructProperty = CastField<const FStructProperty>(&Property))
		{
			return StructProperty->Struct != nullptr
				? FString(TEXT("Struct:")) + StructProperty->Struct->GetPathName()
				: TEXT("Struct:?");
		}
		if (const FSoftObjectProperty* SoftObjectProperty = CastField<const FSoftObjectProperty>(&Property))
		{
			return SoftObjectProperty->PropertyClass != nullptr
				? FString(TEXT("SoftObject:")) + SoftObjectProperty->PropertyClass->GetPathName()
				: TEXT("SoftObject:?");
		}
		if (const FObjectPropertyBase* ObjectProperty = CastField<const FObjectPropertyBase>(&Property))
		{
			return ObjectProperty->PropertyClass != nullptr
				? FString(TEXT("Object:")) + ObjectProperty->PropertyClass->GetPathName()
				: TEXT("Object:?");
		}
		return GetPropertyKind(Property);
	}

	// Reflection property의 canonical enum/struct path 또는 recursive container inner type token을 반환합니다.
	FString GetReflectedTypePath(const FProperty& Property)
	{
		if (CastField<const FArrayProperty>(&Property) != nullptr
			|| CastField<const FSetProperty>(&Property) != nullptr
			|| CastField<const FMapProperty>(&Property) != nullptr)
		{
			return BuildReflectedPropertyTypeToken(Property);
		}
		if (const FEnumProperty* EnumProperty = CastField<const FEnumProperty>(&Property))
		{
			return EnumProperty->GetEnum() != nullptr ? EnumProperty->GetEnum()->GetPathName() : FString();
		}
		if (const FByteProperty* ByteProperty = CastField<const FByteProperty>(&Property))
		{
			return ByteProperty->Enum != nullptr ? ByteProperty->Enum->GetPathName() : FString();
		}
		if (const FStructProperty* StructProperty = CastField<const FStructProperty>(&Property))
		{
			return StructProperty->Struct != nullptr ? StructProperty->Struct->GetPathName() : FString();
		}
		if (const FObjectPropertyBase* ObjectProperty = CastField<const FObjectPropertyBase>(&Property))
		{
			return ObjectProperty->PropertyClass != nullptr ? ObjectProperty->PropertyClass->GetPathName() : FString();
		}
		return FString();
	}

	// Reflection property의 stable container shape를 반환합니다.
	FString GetContainerKind(const FProperty& Property)
	{
		if (CastField<const FArrayProperty>(&Property) != nullptr) return TEXT("Array");
		if (CastField<const FSetProperty>(&Property) != nullptr) return TEXT("Set");
		if (CastField<const FMapProperty>(&Property) != nullptr) return TEXT("Map");
		if (CastField<const FStructProperty>(&Property) != nullptr) return TEXT("Struct");
		return TEXT("Scalar");
	}

	// Reflection property를 Source structural descriptor 한 행으로 변환합니다.
	FCFDASourceFieldDescriptor BuildReflectedSourceDescriptor(const FProperty& Property, const FString& RootClassPath, const FString& SourcePropertyPath, const FString& OwningNestedStructPath)
	{
		// 반환할 reflected Source field descriptor입니다.
		FCFDASourceFieldDescriptor Descriptor;
		Descriptor.SourceRootClassPath = RootClassPath;
		Descriptor.SourcePropertyPath = SourcePropertyPath;
		Descriptor.PropertyKind = GetPropertyKind(Property);
		Descriptor.ReflectedTypePath = GetReflectedTypePath(Property);
		Descriptor.ContainerKind = GetContainerKind(Property);
		Descriptor.NestedStructPath = OwningNestedStructPath;
		if (OwningNestedStructPath.IsEmpty())
		{
			if (const FStructProperty* StructProperty = CastField<const FStructProperty>(&Property))
			{
				Descriptor.NestedStructPath = StructProperty->Struct != nullptr ? StructProperty->Struct->GetPathName() : FString();
			}
		}
		return Descriptor;
	}

	// 한 UStruct가 직접 소유한 CPF_Edit property를 관측하고 nested FStructProperty만 dot-path로 재귀 전개합니다.
	void CollectRecursiveReflectedSourceDescriptors(
		const UStruct* OwningStruct,
		const FString& RootClassPath,
		const FString& ParentPropertyPath,
		const FString& OwningNestedStructPath,
		TArray<FCFDASourceFieldDescriptor>& OutDescriptors)
	{
		if (OwningStruct == nullptr)
		{
			return;
		}

		for (TFieldIterator<FProperty> PropertyIterator(OwningStruct); PropertyIterator; ++PropertyIterator)
		{
			// 현재 UStruct가 직접 소유한 authored reflected property입니다.
			const FProperty* Property = *PropertyIterator;
			if (Property == nullptr || Property->GetOwnerStruct() != OwningStruct || !Property->HasAnyPropertyFlags(CPF_Edit))
			{
				continue;
			}

			// Root class부터 이어지는 canonical dot-path입니다.
			const FString PropertyPath = ParentPropertyPath.IsEmpty()
				? Property->GetName()
				: ParentPropertyPath + TEXT(".") + Property->GetName();
			// 실제 nested USTRUCT container인지 확인합니다.
			const FStructProperty* StructProperty = CastField<const FStructProperty>(Property);
			// 현재 descriptor row가 사용할 nested struct path입니다.
			const FString DescriptorNestedStructPath = StructProperty != nullptr && StructProperty->Struct != nullptr
				? StructProperty->Struct->GetPathName()
				: OwningNestedStructPath;
			OutDescriptors.Add(BuildReflectedSourceDescriptor(*Property, RootClassPath, PropertyPath, DescriptorNestedStructPath));

			if (StructProperty == nullptr || StructProperty->Struct == nullptr)
			{
				continue;
			}

			// Nested USTRUCT 자체가 직접 소유한 authored member만 재귀 관측합니다.
			CollectRecursiveReflectedSourceDescriptors(
				StructProperty->Struct,
				RootClassPath,
				PropertyPath,
				StructProperty->Struct->GetPathName(),
				OutDescriptors);
		}
	}

	// JSON value의 actual physical kind를 반환합니다.
	FString GetObservedJsonKind(const TSharedPtr<FJsonValue>& Value)
	{
		if (!Value.IsValid()) return TEXT("Invalid");
		switch (Value->Type)
		{
		case EJson::String: return TEXT("String");
		case EJson::Number: return TEXT("Number");
		case EJson::Boolean: return TEXT("Boolean");
		case EJson::Object: return TEXT("Object");
		case EJson::Null: return TEXT("Null");
		case EJson::Array: return TEXT("Array");
		default: return TEXT("Unknown");
		}
	}

	// JSON object tree의 모든 field path/kind를 재귀적으로 관측합니다.
	void CollectObservedJsonFields(const TSharedPtr<FJsonObject>& Object, const FString& ParentPath, TMap<FString, FObservedJsonField>& OutFields)
	{
		if (!Object.IsValid()) return;
		for (const auto& Pair : Object->Values)
		{
			// Shared JSON key를 owned field name으로 변환합니다.
			const FString FieldName(Pair.Key.ToView());
			// Root부터 이어진 canonical JSON path입니다.
			const FString FieldPath = ParentPath.IsEmpty() ? FieldName : ParentPath + TEXT(".") + FieldName;
			// 현재 JSON field의 actual observation입니다.
			FObservedJsonField ObservedField;
			ObservedField.JsonValueKind = GetObservedJsonKind(Pair.Value);
			ObservedField.bNull = Pair.Value.IsValid() && Pair.Value->Type == EJson::Null;
			OutFields.Add(FieldPath, MoveTemp(ObservedField));
			if (Pair.Value.IsValid() && Pair.Value->Type == EJson::Object)
			{
				CollectObservedJsonFields(Pair.Value->AsObject(), FieldPath, OutFields);
			}
			else if (Pair.Value.IsValid() && Pair.Value->Type == EJson::Array)
			{
				// Non-empty Array의 element physical kind를 synthesized `[]` path로 관측합니다.
				const TArray<TSharedPtr<FJsonValue>>& ArrayValues = Pair.Value->AsArray();
				if (!ArrayValues.IsEmpty())
				{
					// 첫 element에서 시작하는 homogeneous element kind입니다.
					const FString FirstElementKind = GetObservedJsonKind(ArrayValues[0]);
					// Array element가 모두 같은 physical JSON kind인지 나타냅니다.
					bool bHomogeneousElementKind = true;
					for (const TSharedPtr<FJsonValue>& ArrayValue : ArrayValues)
					{
						if (!GetObservedJsonKind(ArrayValue).Equals(FirstElementKind, ESearchCase::CaseSensitive))
						{
							bHomogeneousElementKind = false;
							break;
						}
					}
					// Synthesized element-level observation입니다.
					FObservedJsonField ElementObservedField;
					ElementObservedField.JsonValueKind = bHomogeneousElementKind ? FirstElementKind : TEXT("Mixed");
					ElementObservedField.bNull = bHomogeneousElementKind && FirstElementKind.Equals(TEXT("Null"), ESearchCase::CaseSensitive);
					OutFields.Add(FieldPath + TEXT("[]"), MoveTemp(ElementObservedField));
				}
			}
		}
	}

	// Expected Adapter kind/null policy가 observed JSON field와 호환되는지 확인합니다.
	bool DoesObservedJsonMatch(const FCFDAAdapterFieldDescriptor& Expected, const FObservedJsonField& Observed)
	{
		if (Observed.bNull)
		{
			return Expected.NullPolicy.Equals(TEXT("Nullable"), ESearchCase::CaseSensitive)
				&& Expected.JsonValueKind.Equals(TEXT("StringOrNull"), ESearchCase::CaseSensitive);
		}
		if (Expected.JsonValueKind.Equals(TEXT("StringOrNull"), ESearchCase::CaseSensitive))
		{
			return Observed.JsonValueKind.Equals(TEXT("String"), ESearchCase::CaseSensitive);
		}
		return Expected.JsonValueKind.Equals(Observed.JsonValueKind, ESearchCase::CaseSensitive);
	}
}

// Current MissileGuidePreset Source structural descriptor를 deterministic order로 반환합니다.
const TArray<FCFDASourceFieldDescriptor>& FCFDAContractGuard::GetSourceShapeDescriptor()
{
	// Current Source exact4 top-level + nested config exact26 descriptor입니다.
	static const TArray<FCFDASourceFieldDescriptor> Descriptors = []()
	{
		// Lambda가 구성할 immutable descriptor 배열입니다.
		TArray<FCFDASourceFieldDescriptor> Result;
		Result.Reserve(30);
		Result.Add({CFDAContractGuardPrivate::SourceRootClassPath, TEXT("PresetId"), TEXT("Name"), TEXT(""), TEXT("Scalar"), TEXT("")});
		Result.Add({CFDAContractGuardPrivate::SourceRootClassPath, TEXT("PresetDisplayName"), TEXT("Text"), TEXT(""), TEXT("Scalar"), TEXT("")});
		Result.Add({CFDAContractGuardPrivate::SourceRootClassPath, TEXT("PresetDescription"), TEXT("Text"), TEXT(""), TEXT("Scalar"), TEXT("")});
		Result.Add({CFDAContractGuardPrivate::SourceRootClassPath, TEXT("MissileGuideConfig"), TEXT("Struct"), CFDAContractGuardPrivate::GuideConfigStructPath, TEXT("Struct"), CFDAContractGuardPrivate::GuideConfigStructPath});
		// Nested config exact26 leaf descriptor를 추가합니다.
		for (const CFDAContractGuardPrivate::FConfigFieldSpec& Spec : CFDAContractGuardPrivate::GetConfigFieldSpecs())
		{
			Result.Add({CFDAContractGuardPrivate::SourceRootClassPath, FString(TEXT("MissileGuideConfig.")) + Spec.Name, Spec.PropertyKind, Spec.ReflectedTypePath, TEXT("Scalar"), CFDAContractGuardPrivate::GuideConfigStructPath});
		}
		return Result;
	}();
	return Descriptors;
}

// Current MissileGuidePreset Adapter physical shape descriptor를 deterministic order로 반환합니다.
const TArray<FCFDAAdapterFieldDescriptor>& FCFDAContractGuard::GetAdapterShapeDescriptor()
{
	// Strict root/payload/FText/config exact physical JSON descriptor입니다.
	static const TArray<FCFDAAdapterFieldDescriptor> Descriptors = []()
	{
		// Lambda가 구성할 immutable descriptor 배열입니다.
		TArray<FCFDAAdapterFieldDescriptor> Result;
		Result.Reserve(42);
		Result.Add({TEXT("SchemaId"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("SchemaId"), TEXT("Metadata")});
		Result.Add({TEXT("SchemaRevision"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("IntegerRevision"), TEXT("Metadata")});
		Result.Add({TEXT("AdapterContractRevision"), TEXT("Number"), TEXT("Required"), TEXT("NonNull"), TEXT("IntegerRevision"), TEXT("Metadata")});
		Result.Add({TEXT("DataAssetTypeClassPath"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("ClassPath"), TEXT("Metadata")});
		Result.Add({TEXT("StableLogicalId"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("NameToken"), TEXT("Identity")});
		Result.Add({TEXT("TargetObjectPath"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("ObjectPath"), TEXT("Metadata")});
		Result.Add({TEXT("BaseSemanticFingerprint"), TEXT("StringOrNull"), TEXT("Required"), TEXT("Nullable"), TEXT("Sha256OrNull"), TEXT("Metadata")});
		Result.Add({TEXT("Payload"), TEXT("Object"), TEXT("Required"), TEXT("NonNull"), TEXT("WholeRecordObject"), TEXT("PayloadContainer")});
		Result.Add({TEXT("Payload.PresetId"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("NameToken"), TEXT("Payload")});
		Result.Add({TEXT("Payload.PresetDisplayName"), TEXT("Object"), TEXT("Required"), TEXT("NonNull"), TEXT("LiteralFTextObject"), TEXT("PayloadContainer")});
		Result.Add({TEXT("Payload.PresetDisplayName.Kind"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("LiteralKind"), TEXT("RepresentationConstant")});
		Result.Add({TEXT("Payload.PresetDisplayName.Text"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("LiteralText"), TEXT("Payload")});
		Result.Add({TEXT("Payload.PresetDescription"), TEXT("Object"), TEXT("Required"), TEXT("NonNull"), TEXT("LiteralFTextObject"), TEXT("PayloadContainer")});
		Result.Add({TEXT("Payload.PresetDescription.Kind"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("LiteralKind"), TEXT("RepresentationConstant")});
		Result.Add({TEXT("Payload.PresetDescription.Text"), TEXT("String"), TEXT("Required"), TEXT("NonNull"), TEXT("LiteralText"), TEXT("Payload")});
		Result.Add({TEXT("Payload.MissileGuideConfig"), TEXT("Object"), TEXT("Required"), TEXT("NonNull"), TEXT("WholeRecordObject"), TEXT("PayloadContainer")});
		// Nested config exact26 physical field descriptor를 추가합니다.
		for (const CFDAContractGuardPrivate::FConfigFieldSpec& Spec : CFDAContractGuardPrivate::GetConfigFieldSpecs())
		{
			Result.Add({FString(TEXT("Payload.MissileGuideConfig.")) + Spec.Name, Spec.JsonValueKind, TEXT("Required"), TEXT("NonNull"), Spec.RepresentationKind, TEXT("Payload")});
		}
		return Result;
	}();
	return Descriptors;
}

// Current MissileGuidePreset explicit Source↔Adapter mapping descriptor를 deterministic order로 반환합니다.
const TArray<FCFDASourceAdapterMappingDescriptor>& FCFDAContractGuard::GetSourceAdapterMappingDescriptor()
{
	// Source authored leaf, adapter metadata와 Literal constant를 구분하는 mapping descriptor입니다.
	static const TArray<FCFDASourceAdapterMappingDescriptor> Descriptors = []()
	{
		// Lambda가 구성할 immutable mapping 배열입니다.
		TArray<FCFDASourceAdapterMappingDescriptor> Result;
		Result.Reserve(38);
		Result.Add({TEXT(""), TEXT("SchemaId"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")});
		Result.Add({TEXT(""), TEXT("SchemaRevision"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")});
		Result.Add({TEXT(""), TEXT("AdapterContractRevision"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")});
		Result.Add({TEXT(""), TEXT("DataAssetTypeClassPath"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")});
		Result.Add({TEXT(""), TEXT("TargetObjectPath"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")});
		Result.Add({TEXT(""), TEXT("BaseSemanticFingerprint"), TEXT("Metadata"), TEXT("AdapterOnlyMetadata")});
		Result.Add({TEXT("PresetId"), TEXT("StableLogicalId"), TEXT("NameToken"), TEXT("SourceToAdapter")});
		Result.Add({TEXT("PresetId"), TEXT("Payload.PresetId"), TEXT("NameToken"), TEXT("SourceToAdapter")});
		Result.Add({TEXT("PresetDisplayName"), TEXT("Payload.PresetDisplayName.Kind"), TEXT("LiteralKind"), TEXT("AdapterOnlyConstant")});
		Result.Add({TEXT("PresetDisplayName"), TEXT("Payload.PresetDisplayName.Text"), TEXT("LiteralText"), TEXT("SourceToAdapter")});
		Result.Add({TEXT("PresetDescription"), TEXT("Payload.PresetDescription.Kind"), TEXT("LiteralKind"), TEXT("AdapterOnlyConstant")});
		Result.Add({TEXT("PresetDescription"), TEXT("Payload.PresetDescription.Text"), TEXT("LiteralText"), TEXT("SourceToAdapter")});
		// Nested config exact26 one-to-one mapping을 추가합니다.
		for (const CFDAContractGuardPrivate::FConfigFieldSpec& Spec : CFDAContractGuardPrivate::GetConfigFieldSpecs())
		{
			Result.Add({FString(TEXT("MissileGuideConfig.")) + Spec.Name, FString(TEXT("Payload.MissileGuideConfig.")) + Spec.Name, Spec.RepresentationKind, TEXT("SourceToAdapter")});
		}
		return Result;
	}();
	return Descriptors;
}

// Current MissileGuidePreset explicit semantic contract descriptor를 deterministic order로 반환합니다.
const TArray<FCFDASemanticRuleDescriptor>& FCFDAContractGuard::GetSemanticContractDescriptor()
{
	// Reflection으로 추론할 수 없는 current adapter semantic policy descriptor입니다.
	static const TArray<FCFDASemanticRuleDescriptor> Descriptors = []()
	{
		// Lambda가 구성할 immutable semantic rule 배열입니다.
		TArray<FCFDASemanticRuleDescriptor> Result;
		Result.Reserve(26);
		Result.Add({TEXT("FText.LiteralRepresentation"), TEXT("SourceStringOnly;Kind=Literal;PackageOnlyNamespaceKeyIgnored;StringTableOrAuthoredNamespaceRejected")});
		Result.Add({TEXT("FName.CasePolicy"), TEXT("SemanticComparisonCaseInsensitive;FingerprintLowercase")});
		Result.Add({TEXT("Enum.TokenPolicy"), TEXT("ExactReflectedSourceToken;CaseSensitive")});
		Result.Add({TEXT("Numeric.FinitePolicy"), TEXT("AllAuthoredFloatsMustBeFinite;NoClampOnParseOrFingerprint")});
		Result.Add({TEXT("Identity.StableLogicalId"), TEXT("StableLogicalIdEqualsPayload.PresetIdByFNameSemantics")});
		Result.Add({TEXT("Fingerprint.Inclusion"), TEXT("SchemaId;SchemaRevision;AdapterContractRevision;DataAssetTypeClassPath;PayloadWritableSemanticLeaves")});
		Result.Add({TEXT("Fingerprint.FloatPolicy"), TEXT("IEEE754Float32;NegativeZeroCanonicalizedToPositiveZero")});
		Result.Add({TEXT("Resolver.TargetPathPolicy"), TEXT("ExactGameObjectPath;PackageLeafEqualsObjectName")});
		// Numeric config field의 explicit frozen range policy를 추가합니다.
		for (const CFDAContractGuardPrivate::FConfigFieldSpec& Spec : CFDAContractGuardPrivate::GetConfigFieldSpecs())
		{
			if (Spec.RangePolicy[0] != TEXT('\0'))
			{
				Result.Add({FString(TEXT("Range.MissileGuideConfig.")) + Spec.Name, Spec.RangePolicy});
			}
		}
		return Result;
	}();
	return Descriptors;
}

// Current production semantic fingerprint가 emit해야 하는 exact token descriptor를 반환합니다.
const TArray<FCFDAFingerprintTokenDescriptor>& FCFDAContractGuard::GetFingerprintTokenDescriptor()
{
	// Metadata exact4 + payload top-level exact3 + config exact26 fingerprint token 계약입니다.
	static const TArray<FCFDAFingerprintTokenDescriptor> Descriptors = []()
	{
		// Lambda가 구성할 immutable fingerprint token descriptor입니다.
		TArray<FCFDAFingerprintTokenDescriptor> Result;
		Result.Reserve(33);
		Result.Add({TEXT("SchemaId"), TEXT(""), TEXT("Metadata")});
		Result.Add({TEXT("SchemaRevision"), TEXT(""), TEXT("Metadata")});
		Result.Add({TEXT("AdapterContractRevision"), TEXT(""), TEXT("Metadata")});
		Result.Add({TEXT("DataAssetTypeClassPath"), TEXT(""), TEXT("Metadata")});
		Result.Add({TEXT("Payload.PresetId"), TEXT("PresetId"), TEXT("PayloadSemanticLeaf")});
		Result.Add({TEXT("Payload.PresetDisplayName"), TEXT("PresetDisplayName"), TEXT("PayloadSemanticLeaf")});
		Result.Add({TEXT("Payload.PresetDescription"), TEXT("PresetDescription"), TEXT("PayloadSemanticLeaf")});
		for (const CFDAContractGuardPrivate::FConfigFieldSpec& Spec : CFDAContractGuardPrivate::GetConfigFieldSpecs())
		{
			Result.Add({FString(TEXT("Config.")) + Spec.Name, FString(TEXT("MissileGuideConfig.")) + Spec.Name, TEXT("PayloadSemanticLeaf")});
		}
		return Result;
	}();
	return Descriptors;
}

// Current four component descriptor signatures를 계산합니다.
bool FCFDAContractGuard::BuildCurrentSignatures(FCFDAContractSignatures& OutSignatures, FString& OutError)
{
	return BuildSignaturesFromDescriptors(
		GetSourceShapeDescriptor(),
		GetAdapterShapeDescriptor(),
		GetSourceAdapterMappingDescriptor(),
		GetSemanticContractDescriptor(),
		OutSignatures,
		OutError);
}

// 임의 exact TypeKey provider가 소유하는 four descriptor 집합을 동일 canonical algorithm으로 계산합니다.
bool FCFDAContractGuard::BuildSignaturesFromDescriptors(
	const TArray<FCFDASourceFieldDescriptor>& SourceDescriptors,
	const TArray<FCFDAAdapterFieldDescriptor>& AdapterDescriptors,
	const TArray<FCFDASourceAdapterMappingDescriptor>& MappingDescriptors,
	const TArray<FCFDASemanticRuleDescriptor>& SemanticDescriptors,
	FCFDAContractSignatures& OutSignatures,
	FString& OutError)
{
	OutSignatures = FCFDAContractSignatures();
	// Source canonical rows입니다.
	TArray<FString> SourceRows;
	for (const FCFDASourceFieldDescriptor& Descriptor : SourceDescriptors)
	{
		SourceRows.Add(CFDAContractGuardPrivate::BuildSourceRow(Descriptor));
	}
	if (!CFDAContractGuardPrivate::HashRows(MoveTemp(SourceRows), OutSignatures.SourceShapeSignature, OutError)) return false;
	// Adapter canonical rows입니다.
	TArray<FString> AdapterRows;
	for (const FCFDAAdapterFieldDescriptor& Descriptor : AdapterDescriptors)
	{
		AdapterRows.Add(CFDAContractGuardPrivate::BuildAdapterRow(Descriptor));
	}
	if (!CFDAContractGuardPrivate::HashRows(MoveTemp(AdapterRows), OutSignatures.AdapterShapeSignature, OutError)) return false;
	// Mapping canonical rows입니다.
	TArray<FString> MappingRows;
	for (const FCFDASourceAdapterMappingDescriptor& Descriptor : MappingDescriptors)
	{
		MappingRows.Add(FString::Printf(TEXT("%s\t%s\t%s\t%s"), *Descriptor.SourcePropertyPath, *Descriptor.AdapterJsonPath, *Descriptor.RepresentationKind, *Descriptor.MappingRole));
	}
	if (!CFDAContractGuardPrivate::HashRows(MoveTemp(MappingRows), OutSignatures.SourceAdapterMappingSignature, OutError)) return false;
	// Semantic canonical rows입니다.
	TArray<FString> SemanticRows;
	for (const FCFDASemanticRuleDescriptor& Descriptor : SemanticDescriptors)
	{
		SemanticRows.Add(Descriptor.PolicyKey + TEXT("\t") + Descriptor.PolicyValue);
	}
	if (!CFDAContractGuardPrivate::HashRows(MoveTemp(SemanticRows), OutSignatures.SemanticContractSignature, OutError)) return false;
	OutError.Reset();
	return true;
}

// Four component signatures를 fixed-key order의 candidate contract signature 하나로 결합합니다.
bool FCFDAContractGuard::BuildContractBundleSignature(const FCFDAContractSignatures& Signatures, FString& OutSignature, FString& OutError)
{
	// Four component signatures를 의미가 고정된 key order로 결합한 canonical text입니다.
	const FString CanonicalText = FString::Printf(
		TEXT("SourceShapeSignature\t%s\n")
		TEXT("AdapterShapeSignature\t%s\n")
		TEXT("SourceAdapterMappingSignature\t%s\n")
		TEXT("SemanticContractSignature\t%s"),
		*Signatures.SourceShapeSignature,
		*Signatures.AdapterShapeSignature,
		*Signatures.SourceAdapterMappingSignature,
		*Signatures.SemanticContractSignature);
	return CFDAContractGuardPrivate::HashCanonicalText(CanonicalText, OutSignature, OutError);
}

// 임의 native DataAsset class가 직접 소유한 CPF_Edit property shape를 TypeKey-neutral하게 관측합니다.
bool FCFDAContractGuard::BuildDirectReflectedSourceShapeDescriptor(
	const UClass& SourceClass,
	TArray<FCFDASourceFieldDescriptor>& OutDescriptors,
	FString& OutError)
{
	OutDescriptors.Reset();
	// Reflection이 보고한 exact root class path입니다.
	const FString RootClassPath = SourceClass.GetClassPathName().ToString();
	for (TFieldIterator<FProperty> PropertyIterator(&SourceClass); PropertyIterator; ++PropertyIterator)
	{
		// 현재 direct class가 실제 소유한 authored reflected property입니다.
		const FProperty* Property = *PropertyIterator;
		if (Property == nullptr || Property->GetOwnerStruct() != &SourceClass || !Property->HasAnyPropertyFlags(CPF_Edit))
		{
			continue;
		}
		OutDescriptors.Add(CFDAContractGuardPrivate::BuildReflectedSourceDescriptor(*Property, RootClassPath, Property->GetName(), FString()));
	}
	if (OutDescriptors.IsEmpty())
	{
		OutError = FString::Printf(TEXT("%s direct authored Reflection 결과가 비었습니다."), *RootClassPath);
		return false;
	}
	OutError.Reset();
	return true;
}

// 임의 native DataAsset class의 direct authored property와 nested USTRUCT CPF_Edit member를 Type-neutral하게 재귀 관측합니다.
bool FCFDAContractGuard::BuildRecursiveReflectedSourceShapeDescriptor(
	const UClass& SourceClass,
	TArray<FCFDASourceFieldDescriptor>& OutDescriptors,
	FString& OutError)
{
	OutDescriptors.Reset();
	// Reflection이 보고한 exact root class path입니다.
	const FString RootClassPath = SourceClass.GetClassPathName().ToString();
	CFDAContractGuardPrivate::CollectRecursiveReflectedSourceDescriptors(
		&SourceClass,
		RootClassPath,
		FString(),
		FString(),
		OutDescriptors);
	if (OutDescriptors.IsEmpty())
	{
		OutError = FString::Printf(TEXT("%s recursive authored Reflection 결과가 비었습니다."), *RootClassPath);
		return false;
	}
	OutError.Reset();
	return true;
}

// Native Reflection에서 current Staging-owned Source shape를 직접 관측합니다.
bool FCFDAContractGuard::BuildReflectedSourceShapeDescriptor(TArray<FCFDASourceFieldDescriptor>& OutDescriptors, FString& OutError)
{
	OutDescriptors.Reset();
	// Current Pilot native source class입니다.
	UClass* SourceClass = UCFMissileGuidePresetData::StaticClass();
	if (SourceClass == nullptr)
	{
		OutError = TEXT("CFMissileGuidePresetData native class Reflection을 찾을 수 없습니다.");
		return false;
	}
	// Reflection이 보고한 actual class path입니다.
	const FString RootClassPath = SourceClass->GetClassPathName().ToString();
	for (TFieldIterator<FProperty> PropertyIterator(SourceClass); PropertyIterator; ++PropertyIterator)
	{
		// 현재 direct class가 실제 소유한 reflected property입니다.
		const FProperty* Property = *PropertyIterator;
		if (Property == nullptr || Property->GetOwnerStruct() != SourceClass || !Property->HasAnyPropertyFlags(CPF_Edit))
		{
			continue;
		}
		// Current direct authored property path입니다.
		const FString PropertyPath = Property->GetName();
		OutDescriptors.Add(CFDAContractGuardPrivate::BuildReflectedSourceDescriptor(*Property, RootClassPath, PropertyPath, FString()));
		if (!PropertyPath.Equals(TEXT("MissileGuideConfig"), ESearchCase::CaseSensitive))
		{
			continue;
		}
		// Current MissileGuideConfig reflected struct property입니다.
		const FStructProperty* GuideConfigProperty = CastField<const FStructProperty>(Property);
		if (GuideConfigProperty == nullptr || GuideConfigProperty->Struct == nullptr)
		{
			continue;
		}
		// Actual nested struct path입니다.
		const FString NestedStructPath = GuideConfigProperty->Struct->GetPathName();
		for (TFieldIterator<FProperty> NestedPropertyIterator(GuideConfigProperty->Struct); NestedPropertyIterator; ++NestedPropertyIterator)
		{
			// Current GuideConfig struct가 직접 소유한 authored reflected property입니다.
			const FProperty* NestedProperty = *NestedPropertyIterator;
			if (NestedProperty == nullptr || NestedProperty->GetOwnerStruct() != GuideConfigProperty->Struct || !NestedProperty->HasAnyPropertyFlags(CPF_Edit))
			{
				continue;
			}
			// Root property부터 이어진 nested authored path입니다.
			const FString NestedPropertyPath = PropertyPath + TEXT(".") + NestedProperty->GetName();
			OutDescriptors.Add(CFDAContractGuardPrivate::BuildReflectedSourceDescriptor(*NestedProperty, RootClassPath, NestedPropertyPath, NestedStructPath));
		}
	}
	if (OutDescriptors.IsEmpty())
	{
		OutError = TEXT("CFMissileGuidePresetData Staging-owned authored Reflection 결과가 비었습니다.");
		return false;
	}
	OutError.Reset();
	return true;
}

// Expected Source descriptor와 observed Reflection/fixture descriptor를 exact 비교합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateSourceShapeCoverage(const TArray<FCFDASourceFieldDescriptor>& ExpectedDescriptors, const TArray<FCFDASourceFieldDescriptor>& ObservedDescriptors)
{
	// 반환할 fail-closed validation 결과입니다.
	FCFDAContractGuardResult Result;
	// Expected canonical rows입니다.
	TArray<FString> ExpectedRows;
	ExpectedRows.Reserve(ExpectedDescriptors.Num());
	for (const FCFDASourceFieldDescriptor& Descriptor : ExpectedDescriptors) ExpectedRows.Add(CFDAContractGuardPrivate::BuildSourceRow(Descriptor));
	// Observed canonical rows입니다.
	TArray<FString> ObservedRows;
	ObservedRows.Reserve(ObservedDescriptors.Num());
	for (const FCFDASourceFieldDescriptor& Descriptor : ObservedDescriptors) ObservedRows.Add(CFDAContractGuardPrivate::BuildSourceRow(Descriptor));
	CFDAContractGuardPrivate::SortRowsOrdinal(ExpectedRows);
	CFDAContractGuardPrivate::SortRowsOrdinal(ObservedRows);
	if (ExpectedRows != ObservedRows)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::SourceAuthoringContractDrift, TEXT("SourceShape"), FString::Printf(TEXT("Source descriptor와 native/fixture Reflection shape가 다릅니다. Expected=%d Observed=%d"), ExpectedRows.Num(), ObservedRows.Num()));
	}
	return Result;
}

// Expected Adapter descriptor와 observed/fixture Adapter descriptor를 exact 비교합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateAdapterShapeCoverage(const TArray<FCFDAAdapterFieldDescriptor>& ExpectedDescriptors, const TArray<FCFDAAdapterFieldDescriptor>& ObservedDescriptors)
{
	// 반환할 fail-closed validation 결과입니다.
	FCFDAContractGuardResult Result;
	// Expected canonical Adapter rows입니다.
	TArray<FString> ExpectedRows;
	ExpectedRows.Reserve(ExpectedDescriptors.Num());
	for (const FCFDAAdapterFieldDescriptor& Descriptor : ExpectedDescriptors) ExpectedRows.Add(CFDAContractGuardPrivate::BuildAdapterRow(Descriptor));
	// Observed canonical Adapter rows입니다.
	TArray<FString> ObservedRows;
	ObservedRows.Reserve(ObservedDescriptors.Num());
	for (const FCFDAAdapterFieldDescriptor& Descriptor : ObservedDescriptors) ObservedRows.Add(CFDAContractGuardPrivate::BuildAdapterRow(Descriptor));
	CFDAContractGuardPrivate::SortRowsOrdinal(ExpectedRows);
	CFDAContractGuardPrivate::SortRowsOrdinal(ObservedRows);
	if (ExpectedRows != ObservedRows)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AdapterShapeDrift, TEXT("AdapterShape"), FString::Printf(TEXT("Adapter physical descriptor shape가 accepted/current expectation과 다릅니다. Expected=%d Observed=%d"), ExpectedRows.Num(), ObservedRows.Num()));
	}
	return Result;
}

// Source leaf set, Adapter terminal field set과 mapping 양쪽 coverage가 exact인지 검증합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateSourceAdapterMappingCoverage(const TArray<FCFDASourceFieldDescriptor>& SourceDescriptors, const TArray<FCFDAAdapterFieldDescriptor>& AdapterDescriptors, const TArray<FCFDASourceAdapterMappingDescriptor>& MappingDescriptors)
{
	// 반환할 fail-closed validation 결과입니다.
	FCFDAContractGuardResult Result;
	// Staging-owned Source semantic leaf set입니다. Struct container node 자체는 leaf가 아닙니다.
	TSet<FString> SourceLeafPaths;
	for (const FCFDASourceFieldDescriptor& Descriptor : SourceDescriptors)
	{
		if (!Descriptor.PropertyKind.Equals(TEXT("Struct"), ESearchCase::CaseSensitive)) SourceLeafPaths.Add(Descriptor.SourcePropertyPath);
	}
	// SourceToAdapter mapping이 실제로 덮는 Source leaf set입니다.
	TSet<FString> MappedSourceLeafPaths;
	// PayloadContainer를 제외한 exact Adapter terminal field set입니다.
	TSet<FString> AdapterTerminalPaths;
	for (const FCFDAAdapterFieldDescriptor& Descriptor : AdapterDescriptors)
	{
		if (!Descriptor.SemanticRole.Equals(TEXT("PayloadContainer"), ESearchCase::CaseSensitive)) AdapterTerminalPaths.Add(Descriptor.AdapterJsonPath);
	}
	// Mapping이 실제로 덮는 Adapter terminal field set입니다.
	TSet<FString> MappedAdapterTerminalPaths;
	for (const FCFDASourceAdapterMappingDescriptor& Descriptor : MappingDescriptors)
	{
		MappedAdapterTerminalPaths.Add(Descriptor.AdapterJsonPath);
		if (Descriptor.MappingRole.Equals(TEXT("SourceToAdapter"), ESearchCase::CaseSensitive)) MappedSourceLeafPaths.Add(Descriptor.SourcePropertyPath);
	}
	if (!CFDAContractGuardPrivate::AreStringSetsEqual(SourceLeafPaths, MappedSourceLeafPaths))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::SourceAdapterMappingDrift, TEXT("Mapping.SourceSide"), FString::Printf(TEXT("Source authored leaf mapping coverage가 exact가 아닙니다. SourceLeaves=%d Mapped=%d"), SourceLeafPaths.Num(), MappedSourceLeafPaths.Num()));
	}
	if (!CFDAContractGuardPrivate::AreStringSetsEqual(AdapterTerminalPaths, MappedAdapterTerminalPaths))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::SourceAdapterMappingDrift, TEXT("Mapping.AdapterSide"), FString::Printf(TEXT("Adapter terminal mapping coverage가 exact가 아닙니다. AdapterFields=%d Mapped=%d"), AdapterTerminalPaths.Num(), MappedAdapterTerminalPaths.Num()));
	}
	if (MappedAdapterTerminalPaths.Num() != MappingDescriptors.Num())
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::SourceAdapterMappingDrift, TEXT("Mapping.AdapterSide"), TEXT("둘 이상의 mapping row가 같은 AdapterJsonPath를 소유합니다."));
	}
	return Result;
}

// Production serializer 또는 memory fixture JSON physical shape가 Adapter descriptor와 exact인지 검증합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateSerializedAdapterCoverage(const FString& JsonText)
{
	return ValidateSerializedAdapterCoverageAgainstDescriptor(JsonText, GetAdapterShapeDescriptor());
}

// 임의 exact TypeKey Adapter descriptor를 대상으로 JSON physical shape를 검증하며 Array element `[]` observation을 포함합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateSerializedAdapterCoverageAgainstDescriptor(
	const FString& JsonText,
	const TArray<FCFDAAdapterFieldDescriptor>& AdapterDescriptors)
{
	// 반환할 fail-closed validation 결과입니다.
	FCFDAContractGuardResult Result;
	// JSON parser가 만들 root object입니다.
	TSharedPtr<FJsonObject> RootObject;
	// Memory-only JSON reader입니다.
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::SerializerCoverageMismatch, TEXT("$"), TEXT("Serializer coverage 검사 대상이 유효한 JSON object가 아닙니다."));
		return Result;
	}
	// Actual serialized JSON path/kind observation입니다.
	TMap<FString, CFDAContractGuardPrivate::FObservedJsonField> ObservedFields;
	CFDAContractGuardPrivate::CollectObservedJsonFields(RootObject, FString(), ObservedFields);
	// Descriptor path로 expected field를 빠르게 찾기 위한 map입니다.
	TMap<FString, const FCFDAAdapterFieldDescriptor*> ExpectedByPath;
	for (const FCFDAAdapterFieldDescriptor& Descriptor : AdapterDescriptors) ExpectedByPath.Add(Descriptor.AdapterJsonPath, &Descriptor);
	for (const FCFDAAdapterFieldDescriptor& Expected : AdapterDescriptors)
	{
		// Expected path에 대응하는 actual serialized field입니다.
		const CFDAContractGuardPrivate::FObservedJsonField* Observed = ObservedFields.Find(Expected.AdapterJsonPath);
		if (Observed == nullptr)
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::SerializerCoverageMismatch, Expected.AdapterJsonPath, TEXT("Adapter descriptor의 required field가 serialized JSON에 없습니다."));
			continue;
		}
		if (!CFDAContractGuardPrivate::DoesObservedJsonMatch(Expected, *Observed))
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::SerializerCoverageMismatch, Expected.AdapterJsonPath, FString::Printf(TEXT("Serialized JSON kind/null shape가 Adapter descriptor와 다릅니다. Expected=%s Actual=%s"), *Expected.JsonValueKind, *Observed->JsonValueKind));
		}
	}
	for (const TPair<FString, CFDAContractGuardPrivate::FObservedJsonField>& ObservedPair : ObservedFields)
	{
		if (!ExpectedByPath.Contains(ObservedPair.Key))
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::SerializerCoverageMismatch, ObservedPair.Key, TEXT("Adapter descriptor에 없는 field가 serialized JSON에 존재합니다."));
		}
	}
	return Result;
}

// Production fingerprint가 emit한 token label sequence가 fingerprint descriptor와 exact인지 검증합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateFingerprintCoverage(const TArray<FString>& ObservedTokenLabels)
{
	// 반환할 fail-closed validation 결과입니다.
	FCFDAContractGuardResult Result;
	// Descriptor에서 투영한 expected exact token label sequence입니다.
	TArray<FString> ExpectedTokenLabels;
	ExpectedTokenLabels.Reserve(GetFingerprintTokenDescriptor().Num());
	for (const FCFDAFingerprintTokenDescriptor& Descriptor : GetFingerprintTokenDescriptor()) ExpectedTokenLabels.Add(Descriptor.TokenLabel);
	if (ExpectedTokenLabels.Num() != ObservedTokenLabels.Num())
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::FingerprintCoverageMismatch, TEXT("Fingerprint.TokenLabels"), FString::Printf(TEXT("Fingerprint token count가 descriptor와 다릅니다. Expected=%d Observed=%d"), ExpectedTokenLabels.Num(), ObservedTokenLabels.Num()));
		return Result;
	}
	for (int32 TokenIndex = 0; TokenIndex < ExpectedTokenLabels.Num(); ++TokenIndex)
	{
		if (!ExpectedTokenLabels[TokenIndex].Equals(ObservedTokenLabels[TokenIndex], ESearchCase::CaseSensitive))
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::FingerprintCoverageMismatch, FString::Printf(TEXT("Fingerprint.Token[%d]"), TokenIndex), FString::Printf(TEXT("Fingerprint token label/order가 descriptor와 다릅니다. Expected=%s Observed=%s"), *ExpectedTokenLabels[TokenIndex], *ObservedTokenLabels[TokenIndex]));
			break;
		}
	}
	return Result;
}

// Materializer→extractor readback payload가 expected payload의 full semantic fingerprint를 보존했는지 검증합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateMaterializerRoundTripCoverage(const FCFDAMissilePresetPayload& ExpectedPayload, const FCFDAMissilePresetPayload& ReadbackPayload)
{
	// 반환할 fail-closed validation 결과입니다.
	FCFDAContractGuardResult Result;
	// Expected payload semantic fingerprint입니다.
	FString ExpectedFingerprint;
	// Expected fingerprint 생성 오류입니다.
	FString ExpectedError;
	if (!FCFDAStagingService::BuildSemanticFingerprint(ExpectedPayload, ExpectedFingerprint, ExpectedError))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::MaterializerCoverageMismatch, TEXT("Materializer.ExpectedPayload"), ExpectedError);
		return Result;
	}
	// Production extractor readback semantic fingerprint입니다.
	FString ReadbackFingerprint;
	// Readback fingerprint 생성 오류입니다.
	FString ReadbackError;
	if (!FCFDAStagingService::BuildSemanticFingerprint(ReadbackPayload, ReadbackFingerprint, ReadbackError))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::MaterializerCoverageMismatch, TEXT("Materializer.ReadbackPayload"), ReadbackError);
		return Result;
	}
	if (!ExpectedFingerprint.Equals(ReadbackFingerprint, ESearchCase::CaseSensitive))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::MaterializerCoverageMismatch, TEXT("Materializer.SemanticFingerprint"), TEXT("Production materializer→extractor readback이 expected full semantic payload를 보존하지 못했습니다."));
	}
	return Result;
}

// Current native Reflection과 Source/Adapter/mapping descriptor 관계를 production-side structural gate로 검증합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateCurrentStructuralCoverage()
{
	// 반환할 aggregated structural guard 결과입니다.
	FCFDAContractGuardResult Result;
	// Current native Reflection observation입니다.
	TArray<FCFDASourceFieldDescriptor> ReflectedSourceDescriptors;
	// Reflection observation 실패 원인입니다.
	FString ReflectionError;
	if (!BuildReflectedSourceShapeDescriptor(ReflectedSourceDescriptors, ReflectionError))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::SourceAuthoringContractDrift, TEXT("SourceReflection"), ReflectionError);
		return Result;
	}
	// Descriptor와 actual Reflection exact 비교 결과입니다.
	const FCFDAContractGuardResult SourceResult = ValidateSourceShapeCoverage(GetSourceShapeDescriptor(), ReflectedSourceDescriptors);
	CFDAContractGuardPrivate::AppendIssues(Result, SourceResult);
	// Source/Adapter/mapping 양방향 exact coverage 결과입니다.
	const FCFDAContractGuardResult MappingResult = ValidateSourceAdapterMappingCoverage(GetSourceShapeDescriptor(), GetAdapterShapeDescriptor(), GetSourceAdapterMappingDescriptor());
	CFDAContractGuardPrivate::AppendIssues(Result, MappingResult);
	return Result;
}

// Guard result에 requested developer issue code가 존재하는지 확인합니다.
bool FCFDAContractGuard::HasIssueCode(const FCFDAContractGuardResult& Result, const ECFDAContractIssueCode Code)
{
	for (const FCFDAContractGuardIssue& Issue : Result.Issues)
	{
		if (Issue.Code == Code) return true;
	}
	return false;
}

// 아직 accepted되지 않은 current change declaration을 반환합니다. Current contract가 latest accepted baseline과 같으므로 현재는 declaration이 없습니다.
const FCFDACurrentChangeDeclaration* FCFDAContractGuard::GetCurrentChangeDeclaration()
{
	return nullptr;
}

// Latest accepted snapshot 대비 candidate revision/signature와 explicit migration declaration을 fail-closed 검증합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateRevisionGuard(
	const FCFDAAcceptedContractSnapshot& BaseSnapshot,
	const FCFDAContractSignatures& CandidateSignatures,
	const int32 CandidateSchemaRevision,
	const int32 CandidateAdapterContractRevision,
	const FCFDACurrentChangeDeclaration* ChangeDeclaration)
{
	// 반환할 fail-closed revision guard 결과입니다.
	FCFDAContractGuardResult Result;
	// Native Source structural signature 변화 여부입니다.
	const bool bSourceChanged = !BaseSnapshot.SourceShapeSignature.Equals(CandidateSignatures.SourceShapeSignature, ESearchCase::CaseSensitive);
	// Adapter physical shape signature 변화 여부입니다.
	const bool bAdapterChanged = !BaseSnapshot.AdapterShapeSignature.Equals(CandidateSignatures.AdapterShapeSignature, ESearchCase::CaseSensitive);
	// Source↔Adapter mapping signature 변화 여부입니다.
	const bool bMappingChanged = !BaseSnapshot.SourceAdapterMappingSignature.Equals(CandidateSignatures.SourceAdapterMappingSignature, ESearchCase::CaseSensitive);
	// Explicit semantic contract signature 변화 여부입니다.
	const bool bSemanticChanged = !BaseSnapshot.SemanticContractSignature.Equals(CandidateSignatures.SemanticContractSignature, ESearchCase::CaseSensitive);
	// Four contract signatures 중 하나라도 바뀌었는지 나타냅니다.
	const bool bAnyContractChanged = bSourceChanged || bAdapterChanged || bMappingChanged || bSemanticChanged;
	// Schema revision이 accepted baseline보다 전진했는지 나타냅니다.
	const bool bSchemaRevisionAdvanced = CandidateSchemaRevision > BaseSnapshot.SchemaRevision;
	// Adapter semantic revision이 accepted baseline보다 전진했는지 나타냅니다.
	const bool bAdapterRevisionAdvanced = CandidateAdapterContractRevision > BaseSnapshot.AdapterContractRevision;
	// Revision metadata 자체가 accepted baseline과 다른지 나타냅니다.
	const bool bAnyRevisionChanged = CandidateSchemaRevision != BaseSnapshot.SchemaRevision || CandidateAdapterContractRevision != BaseSnapshot.AdapterContractRevision;

	if (CandidateSchemaRevision < BaseSnapshot.SchemaRevision)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, TEXT("SchemaRevision"), TEXT("Candidate SchemaRevision이 latest accepted revision보다 감소했습니다."));
	}
	if (CandidateAdapterContractRevision < BaseSnapshot.AdapterContractRevision)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, TEXT("AdapterContractRevision"), TEXT("Candidate AdapterContractRevision이 latest accepted revision보다 감소했습니다."));
	}
	if (bAdapterChanged && !bSchemaRevisionAdvanced)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::SchemaRevisionBumpRequired, TEXT("AdapterShapeSignature"), TEXT("Adapter physical shape가 변했지만 SchemaRevision이 latest accepted revision보다 전진하지 않았습니다."));
	}
	if (bSemanticChanged && !bAdapterRevisionAdvanced)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AdapterRevisionBumpRequired, TEXT("SemanticContractSignature"), TEXT("Declared semantic contract가 변했지만 AdapterContractRevision이 latest accepted revision보다 전진하지 않았습니다."));
	}

	if (!bAnyContractChanged && !bAnyRevisionChanged)
	{
		if (ChangeDeclaration != nullptr)
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::MigrationImpactUndeclared, TEXT("CurrentChangeDeclaration"), TEXT("Latest accepted baseline과 동일한 no-delta 상태에는 CurrentChangeDeclaration이 남아 있을 수 없습니다."));
		}
		return Result;
	}
	if (ChangeDeclaration == nullptr)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::MigrationImpactUndeclared, TEXT("CurrentChangeDeclaration"), TEXT("Contract 또는 revision 변화가 있지만 CurrentChangeDeclaration이 없습니다."));
		return Result;
	}
	if (!ChangeDeclaration->BaseSnapshotId.Equals(BaseSnapshot.SnapshotId, ESearchCase::CaseSensitive))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, TEXT("CurrentChangeDeclaration.BaseSnapshotId"), TEXT("CurrentChangeDeclaration이 latest accepted snapshot이 아닌 stale base를 가리킵니다."));
	}

	// Guard가 actual candidate four-signature bundle에서 계산한 binding signature입니다.
	FString ActualCandidateContractSignature;
	// Candidate contract signature 계산 실패 사유입니다.
	FString ContractSignatureError;
	if (!BuildContractBundleSignature(CandidateSignatures, ActualCandidateContractSignature, ContractSignatureError))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::MigrationImpactUndeclared, TEXT("CurrentChangeDeclaration.CandidateContractSignature"), ContractSignatureError);
	}
	else if (!ChangeDeclaration->CandidateContractSignature.Equals(ActualCandidateContractSignature, ESearchCase::CaseSensitive))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::MigrationImpactUndeclared, TEXT("CurrentChangeDeclaration.CandidateContractSignature"), TEXT("Declared CandidateContractSignature가 actual current contract signature bundle과 다릅니다."));
	}
	if (!ChangeDeclaration->bImpactDeclared)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::MigrationImpactUndeclared, TEXT("CurrentChangeDeclaration.Impact"), TEXT("Contract 변화의 migration impact가 explicit하게 선언되지 않았습니다."));
		return Result;
	}

	// Impact가 Staging migration을 포함하는지 나타냅니다.
	const bool bImpactIncludesStagingMigration =
		ChangeDeclaration->Impact == ECFDAContractMigrationImpact::StagingMigrationRequired
		|| ChangeDeclaration->Impact == ECFDAContractMigrationImpact::StagingAndProductMigrationReviewRequired;
	// Adapter/semantic 변화 또는 revision 전진으로 Staging revision migration이 필요한지 나타냅니다.
	const bool bStagingMigrationRequired = bAdapterChanged || bSemanticChanged || bSchemaRevisionAdvanced || bAdapterRevisionAdvanced;
	if (bStagingMigrationRequired && !bImpactIncludesStagingMigration)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::MigrationImpactUndeclared, TEXT("CurrentChangeDeclaration.Impact"), TEXT("Adapter/semantic contract 변화 또는 revision 전진에는 최소 StagingMigrationRequired impact가 필요합니다."));
	}

	// Explicit NoMigration을 구조적으로 허용할 수 있는 Source/Mapping-only candidate인지 나타냅니다.
	const bool bNoMigrationStructurallyAllowed = (bSourceChanged || bMappingChanged)
		&& !bAdapterChanged
		&& !bSemanticChanged
		&& !bAnyRevisionChanged;
	if (ChangeDeclaration->Impact == ECFDAContractMigrationImpact::NoMigration && !bNoMigrationStructurallyAllowed)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::MigrationImpactUndeclared, TEXT("CurrentChangeDeclaration.Impact"), TEXT("Explicit NoMigration은 Adapter/semantic/revision 변화가 없는 Source/Mapping-only candidate에서만 선언할 수 있습니다."));
	}
	return Result;
}

// Base accepted identity를 exact TypeKey provider에 먼저 결속한 뒤 provider revision으로 revision guard를 실행합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateRevisionGuardForProvider(
	const FCFDATypeProvider& Provider,
	const FCFDAAcceptedContractSnapshot& BaseSnapshot,
	const FCFDAContractSignatures& CandidateSignatures,
	const FCFDACurrentChangeDeclaration* ChangeDeclaration)
{
	// Provider identity binding과 기존 revision algorithm 결과를 합칠 fail-closed 결과입니다.
	FCFDAContractGuardResult Result;
	if (Provider.DaceReadiness != ECFDADaceReadiness::ContractReady)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, TEXT("Provider.DaceReadiness"), TEXT("Revision Guard는 DACE ContractReady provider에서만 허용됩니다."));
		return Result;
	}
	if (!BaseSnapshot.SchemaId.Equals(Provider.TypeKey.SchemaId, ESearchCase::CaseSensitive)
		|| !BaseSnapshot.DataAssetTypeClassPath.Equals(Provider.TypeKey.DataAssetTypeClassPath, ESearchCase::CaseSensitive))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, TEXT("Provider.TypeKey"), TEXT("Accepted base snapshot identity가 selected exact TypeKey provider와 다릅니다."));
		return Result;
	}
	// Selected provider가 소유하는 current revision을 사용하는 공용 revision guard 결과입니다.
	const FCFDAContractGuardResult RevisionResult = ValidateRevisionGuard(
		BaseSnapshot,
		CandidateSignatures,
		Provider.SchemaRevision,
		Provider.AdapterContractRevision,
		ChangeDeclaration);
	CFDAContractGuardPrivate::AppendIssues(Result, RevisionResult);
	return Result;
}

// Accepted snapshot history의 four component signature integrity, record signature, unique id, previous chain과 revision monotonicity를 검증합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateAcceptedSnapshotChain(const TArray<FCFDAAcceptedContractSnapshot>& Snapshots)
{
	// 반환할 fail-closed accepted history validation 결과입니다.
	FCFDAContractGuardResult Result;
	if (Snapshots.IsEmpty())
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, TEXT("AcceptedSnapshots"), TEXT("Accepted snapshot history가 비어 있습니다."));
		return Result;
	}
	// 중복 SnapshotId를 차단하기 위한 observed identity set입니다.
	TSet<FString> ObservedSnapshotIds;
	for (int32 SnapshotIndex = 0; SnapshotIndex < Snapshots.Num(); ++SnapshotIndex)
	{
		// 현재 검증할 accepted snapshot record입니다.
		const FCFDAAcceptedContractSnapshot& Snapshot = Snapshots[SnapshotIndex];
		if (Snapshot.SnapshotId.IsEmpty() || ObservedSnapshotIds.Contains(Snapshot.SnapshotId))
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, TEXT("SnapshotId"), TEXT("Accepted snapshot identity가 비어 있거나 중복되었습니다."));
		}
		ObservedSnapshotIds.Add(Snapshot.SnapshotId);
		CFDAContractGuardPrivate::ValidateAcceptedComponentSignature(Result, Snapshot.SnapshotId, TEXT("SourceShapeSignature"), Snapshot.SourceShapeSignature);
		CFDAContractGuardPrivate::ValidateAcceptedComponentSignature(Result, Snapshot.SnapshotId, TEXT("AdapterShapeSignature"), Snapshot.AdapterShapeSignature);
		CFDAContractGuardPrivate::ValidateAcceptedComponentSignature(Result, Snapshot.SnapshotId, TEXT("SourceAdapterMappingSignature"), Snapshot.SourceAdapterMappingSignature);
		CFDAContractGuardPrivate::ValidateAcceptedComponentSignature(Result, Snapshot.SnapshotId, TEXT("SemanticContractSignature"), Snapshot.SemanticContractSignature);
		// Accepted record가 Pending/invalid Resolution/Evidence 상태를 포함하지 않는지 검증한 결과입니다.
		const FCFDAContractGuardResult MigrationResolutionResult = ValidateMigrationResolution(Snapshot.MigrationImpact, Snapshot.MigrationResolution, Snapshot.MigrationEvidenceId, true);
		CFDAContractGuardPrivate::AppendIssues(Result, MigrationResolutionResult);
		// Stored record에서 재계산한 deterministic snapshot signature입니다.
		FString RecomputedSnapshotSignature;
		// Snapshot signature 계산 실패 사유입니다.
		FString SnapshotSignatureError;
		if (!BuildSnapshotSignature(Snapshot, RecomputedSnapshotSignature, SnapshotSignatureError)
			|| !RecomputedSnapshotSignature.Equals(Snapshot.SnapshotSignature, ESearchCase::CaseSensitive))
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, Snapshot.SnapshotId, SnapshotSignatureError.IsEmpty() ? TEXT("Accepted snapshot record signature가 stored signature와 다릅니다.") : SnapshotSignatureError);
		}
		if (SnapshotIndex == 0)
		{
			if (!Snapshot.PreviousSnapshotSignature.IsEmpty())
			{
				CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, TEXT("PreviousSnapshotSignature"), TEXT("Bootstrap accepted snapshot의 PreviousSnapshotSignature는 empty여야 합니다."));
			}
			continue;
		}

		// 바로 이전 accepted snapshot record입니다.
		const FCFDAAcceptedContractSnapshot& PreviousSnapshot = Snapshots[SnapshotIndex - 1];
		if (!Snapshot.PreviousSnapshotSignature.Equals(PreviousSnapshot.SnapshotSignature, ESearchCase::CaseSensitive))
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, Snapshot.SnapshotId, TEXT("Accepted snapshot PreviousSnapshotSignature가 바로 이전 record signature와 연결되지 않습니다."));
		}
		if (!Snapshot.SchemaId.Equals(PreviousSnapshot.SchemaId, ESearchCase::CaseSensitive)
			|| !Snapshot.DataAssetTypeClassPath.Equals(PreviousSnapshot.DataAssetTypeClassPath, ESearchCase::CaseSensitive))
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, Snapshot.SnapshotId, TEXT("한 accepted history chain 안에서 SchemaId 또는 DataAssetTypeClassPath identity가 바뀌었습니다."));
		}
		if (Snapshot.SchemaRevision < PreviousSnapshot.SchemaRevision || Snapshot.AdapterContractRevision < PreviousSnapshot.AdapterContractRevision)
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, Snapshot.SnapshotId, TEXT("Accepted snapshot revision은 이전 record보다 감소할 수 없습니다."));
		}

		// Accepted record의 four component candidate signatures입니다.
		FCFDAContractSignatures CandidateSignatures;
		CandidateSignatures.SourceShapeSignature = Snapshot.SourceShapeSignature;
		CandidateSignatures.AdapterShapeSignature = Snapshot.AdapterShapeSignature;
		CandidateSignatures.SourceAdapterMappingSignature = Snapshot.SourceAdapterMappingSignature;
		CandidateSignatures.SemanticContractSignature = Snapshot.SemanticContractSignature;
		// Accepted record가 소유하는 migration declaration을 revision guard 입력으로 투영합니다.
		FCFDACurrentChangeDeclaration AcceptedDeclaration;
		AcceptedDeclaration.BaseSnapshotId = PreviousSnapshot.SnapshotId;
		AcceptedDeclaration.bImpactDeclared = true;
		AcceptedDeclaration.Impact = Snapshot.MigrationImpact;
		AcceptedDeclaration.Resolution = Snapshot.MigrationResolution;
		AcceptedDeclaration.MigrationEvidenceId = Snapshot.MigrationEvidenceId;
		// Accepted candidate contract bundle signature입니다.
		FString CandidateContractSignature;
		// Candidate contract bundle signature 계산 실패 사유입니다.
		FString CandidateContractSignatureError;
		if (BuildContractBundleSignature(CandidateSignatures, CandidateContractSignature, CandidateContractSignatureError))
		{
			AcceptedDeclaration.CandidateContractSignature = CandidateContractSignature;
		}
		else
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, Snapshot.SnapshotId, CandidateContractSignatureError);
		}
		// Adjacent accepted records 사이의 revision/impact semantic consistency 결과입니다.
		const FCFDAContractGuardResult RevisionResult = ValidateRevisionGuard(PreviousSnapshot, CandidateSignatures, Snapshot.SchemaRevision, Snapshot.AdapterContractRevision, &AcceptedDeclaration);
		CFDAContractGuardPrivate::AppendIssues(Result, RevisionResult);
	}
	return Result;
}

// Accepted history 전체를 provider exact TypeKey + provider-local history namespace에 결속해 cross-TypeKey chain 오염을 차단합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateAcceptedSnapshotChainForProvider(
	const FCFDATypeProvider& Provider,
	const TArray<FCFDAAcceptedContractSnapshot>& Snapshots)
{
	// 공용 chain integrity 결과입니다.
	FCFDAContractGuardResult Result = ValidateAcceptedSnapshotChain(Snapshots);
	if (Provider.DaceReadiness != ECFDADaceReadiness::ContractReady)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, TEXT("Provider.DaceReadiness"), TEXT("Accepted DACE history validation은 ContractReady provider에서만 허용됩니다."));
		return Result;
	}
	if (Provider.DaceAcceptedHistoryNamespace.IsEmpty())
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, TEXT("Provider.DaceAcceptedHistoryNamespace"), TEXT("Provider-local DACE history namespace가 비어 있습니다."));
		return Result;
	}
	// Provider namespace 아래 snapshot identity prefix입니다.
	const FString SnapshotIdPrefix = Provider.DaceAcceptedHistoryNamespace + TEXT("-");
	for (const FCFDAAcceptedContractSnapshot& Snapshot : Snapshots)
	{
		if (!Snapshot.SchemaId.Equals(Provider.TypeKey.SchemaId, ESearchCase::CaseSensitive)
			|| !Snapshot.DataAssetTypeClassPath.Equals(Provider.TypeKey.DataAssetTypeClassPath, ESearchCase::CaseSensitive))
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, Snapshot.SnapshotId, TEXT("Provider-local accepted history에 다른 exact TypeKey record가 섞였습니다."));
		}
		if (!Snapshot.SnapshotId.StartsWith(SnapshotIdPrefix, ESearchCase::CaseSensitive))
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, Snapshot.SnapshotId, TEXT("Accepted SnapshotId가 selected provider의 DACE history namespace에 속하지 않습니다."));
		}
	}
	return Result;
}

// Production accepted history와 actual current descriptor/service revision을 함께 검증합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateCurrentRevisionGuard()
{
	// Current Missile compatibility facade가 선택한 exact provider입니다.
	const FCFDATypeProvider& Provider = CFDAMissileProvider::GetProvider().Descriptor;
	// Accepted history 자체를 provider TypeKey/history namespace에 결속한 validation 결과입니다.
	FCFDAContractGuardResult Result = ValidateAcceptedSnapshotChainForProvider(Provider, GetAcceptedSnapshots());
	// Production accepted history입니다.
	const TArray<FCFDAAcceptedContractSnapshot>& Snapshots = GetAcceptedSnapshots();
	if (Snapshots.IsEmpty())
	{
		return Result;
	}
	// Current descriptor에서 계산한 actual four component signatures입니다.
	FCFDAContractSignatures CurrentSignatures;
	// Current signature 계산 failure 사유입니다.
	FString CurrentSignatureError;
	if (!BuildCurrentSignatures(CurrentSignatures, CurrentSignatureError))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, TEXT("CurrentContractSignature"), CurrentSignatureError);
		return Result;
	}
	// Latest accepted baseline snapshot입니다.
	const FCFDAAcceptedContractSnapshot& LatestSnapshot = Snapshots.Last();
	if (!LatestSnapshot.SchemaId.Equals(Provider.TypeKey.SchemaId, ESearchCase::CaseSensitive)
		|| !LatestSnapshot.DataAssetTypeClassPath.Equals(Provider.TypeKey.DataAssetTypeClassPath, ESearchCase::CaseSensitive))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, TEXT("CurrentContractIdentity"), TEXT("Current SchemaId/DataAssetTypeClassPath가 latest accepted history identity와 다릅니다."));
	}
	// Latest accepted baseline 대비 selected provider revision/signature/declaration 결과입니다.
	const FCFDAContractGuardResult RevisionResult = ValidateRevisionGuardForProvider(
		Provider,
		LatestSnapshot,
		CurrentSignatures,
		GetCurrentChangeDeclaration());
	CFDAContractGuardPrivate::AppendIssues(Result, RevisionResult);
	return Result;
}

// 한 Staging JSON을 current production strict parser/revision 계약으로 read-only 검증합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateStagingJsonCompatibility(const FString& JsonText, const FString& StagingRelativePath)
{
	return ValidateStagingJsonCompatibilityForProvider(CFDAMissileProvider::GetProvider(), JsonText, StagingRelativePath);
}

// selected exact TypeKey provider의 production parser/revision 계약으로 한 Staging JSON을 read-only 검증합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateStagingJsonCompatibilityForProvider(
	const FCFDATypeProviderEntry& ProviderEntry,
	const FString& JsonText,
	const FString& StagingRelativePath)
{
	// 반환할 Staging compatibility 결과입니다.
	FCFDAContractGuardResult Result;
	if (ProviderEntry.Operations.ParseCommonCandidate == nullptr)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::StagingMigrationPending, StagingRelativePath, TEXT("Selected provider에 production ParseCommonCandidate operation이 없습니다."));
		return Result;
	}
	// Provider typed parser를 통과한 payload-free current envelope입니다.
	FCFDACommonEnvelope Envelope;
	// Production parser가 반환한 typed/shared issues입니다.
	TArray<FCFDAStagingIssue> ParseIssues;
	if (!ProviderEntry.Operations.ParseCommonCandidate(JsonText, StagingRelativePath, Envelope, ParseIssues))
	{
		// 가장 구체적인 production parser field path입니다.
		const FString FieldPath = ParseIssues.IsEmpty() ? StagingRelativePath : ParseIssues[0].FieldPath;
		// 가장 구체적인 production parser 설명입니다.
		const FString DetailMessage = ParseIssues.IsEmpty() ? TEXT("Production provider parser가 canonical Staging JSON을 거부했습니다.") : ParseIssues[0].Message;
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::StagingMigrationPending, FieldPath, FString::Printf(TEXT("Canonical Staging이 selected provider current parser/revision과 호환되지 않습니다: %s"), *DetailMessage));
		return Result;
	}
	// Parsed envelope가 selected provider exact TypeKey/revision/root 계약과 일치하는지 확인합니다.
	FString ProviderContractError;
	if (!CFDATypeDispatch::ValidateProviderContract(Envelope, ProviderEntry.Descriptor, ProviderContractError))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::StagingMigrationPending, StagingRelativePath, ProviderContractError);
	}
	return Result;
}

// canonical Product MissileGuidePreset Staging exact3을 disk에서 read-only로 읽어 current strict parser/revision과 검증합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateCurrentCanonicalStagingCompatibility()
{
	return ValidateCanonicalStagingCompatibilityForProvider(CFDAMissileProvider::GetProvider());
}

// provider가 explicit 선언한 canonical target set을 read-only 검증합니다. 선언된 empty set은 valid exact0입니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateCanonicalStagingCompatibilityForProvider(const FCFDATypeProviderEntry& ProviderEntry)
{
	// Provider-local canonical target set 전체 결과입니다.
	FCFDAContractGuardResult Result;
	if (!ProviderEntry.Descriptor.bDaceCanonicalStagingTargetSetDeclared)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::StagingMigrationPending, TEXT("Provider.DaceCanonicalStagingTargetSet"), TEXT("Selected provider가 DACE canonical Staging target set을 explicit하게 선언하지 않았습니다."));
		return Result;
	}
	for (const FString& StagingRelativePath : ProviderEntry.Descriptor.DaceCanonicalStagingRelativePaths)
	{
		// Project checkout 안의 read-only physical JSON 경로입니다.
		const FString PhysicalPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(CFDAContractGuardPrivate::GetMainGameRoot(), StagingRelativePath));
		// Disk에서 읽은 canonical JSON text입니다.
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *PhysicalPath))
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::StagingMigrationPending, StagingRelativePath, TEXT("Provider-owned canonical Product Staging JSON을 read-only로 읽을 수 없습니다."));
			continue;
		}
		// Selected provider production parser/revision compatibility 결과입니다.
		const FCFDAContractGuardResult CompatibilityResult = ValidateStagingJsonCompatibilityForProvider(ProviderEntry, JsonText, StagingRelativePath);
		CFDAContractGuardPrivate::AppendIssues(Result, CompatibilityResult);
	}
	return Result;
}

// Declared impact의 Resolution/Evidence와 canonical Staging 호환성을 migration pending semantics로 검증합니다.
FCFDAContractGuardResult FCFDAContractGuard::ValidateMigrationResolution(
	const ECFDAContractMigrationImpact Impact,
	const ECFDAContractMigrationResolution Resolution,
	const FString& MigrationEvidenceId,
	const bool bCanonicalStagingCompatible)
{
	// 반환할 fail-closed migration resolution 결과입니다.
	FCFDAContractGuardResult Result;
	// Impact가 Staging migration을 포함하는지 나타냅니다.
	const bool bRequiresStagingMigration = Impact == ECFDAContractMigrationImpact::StagingMigrationRequired
		|| Impact == ECFDAContractMigrationImpact::StagingAndProductMigrationReviewRequired;
	// Impact가 Product migration review를 포함하는지 나타냅니다.
	const bool bRequiresProductReview = Impact == ECFDAContractMigrationImpact::ProductMigrationReviewRequired
		|| Impact == ECFDAContractMigrationImpact::StagingAndProductMigrationReviewRequired;

	if (Impact == ECFDAContractMigrationImpact::NoMigration)
	{
		if (Resolution != ECFDAContractMigrationResolution::NotRequired || !MigrationEvidenceId.IsEmpty())
		{
			CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::MigrationImpactUndeclared, TEXT("MigrationResolution"), TEXT("NoMigration은 Resolution=NotRequired와 empty MigrationEvidenceId여야 합니다."));
		}
		return Result;
	}

	if (Resolution == ECFDAContractMigrationResolution::NotRequired)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::MigrationImpactUndeclared, TEXT("MigrationResolution"), TEXT("Migration/review가 필요한 impact는 Resolution=NotRequired를 사용할 수 없습니다."));
	}
	if (Resolution == ECFDAContractMigrationResolution::Resolved && MigrationEvidenceId.IsEmpty())
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::MigrationImpactUndeclared, TEXT("MigrationEvidenceId"), TEXT("MigrationResolution=Resolved이면 non-empty MigrationEvidenceId가 필요합니다."));
	}
	if (bRequiresStagingMigration && (Resolution != ECFDAContractMigrationResolution::Resolved || !bCanonicalStagingCompatible))
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::StagingMigrationPending, TEXT("MigrationResolution"), TEXT("Staging migration impact가 아직 Resolved+evidence 상태가 아니거나 canonical Staging이 current revision/strict parser와 호환되지 않습니다."));
	}
	if (bRequiresProductReview && Resolution != ECFDAContractMigrationResolution::Resolved)
	{
		CFDAContractGuardPrivate::AddIssue(Result, ECFDAContractIssueCode::ProductMigrationReviewPending, TEXT("MigrationResolution"), TEXT("Product migration review impact가 아직 Resolved 상태가 아닙니다."));
	}
	return Result;
}

// CurrentChangeDeclaration과 canonical Staging 결과를 accepted append/Current promotion machine gate로 평가합니다.
FCFDAMigrationGateResult FCFDAContractGuard::EvaluateMigrationGate(const FCFDACurrentChangeDeclaration* ChangeDeclaration, const FCFDAContractGuardResult& CanonicalStagingResult)
{
	// 반환할 append/promotion gate 상태입니다.
	FCFDAMigrationGateResult GateResult;
	GateResult.Validation = CanonicalStagingResult;
	if (ChangeDeclaration != nullptr)
	{
		if (!ChangeDeclaration->bImpactDeclared)
		{
			CFDAContractGuardPrivate::AddIssue(GateResult.Validation, ECFDAContractIssueCode::MigrationImpactUndeclared, TEXT("CurrentChangeDeclaration.Impact"), TEXT("Migration gate에 explicit impact declaration이 없습니다."));
		}
		else
		{
			// Canonical exact3 전체가 current strict parser/revision과 호환되는지 나타냅니다.
			const bool bCanonicalStagingCompatible = CanonicalStagingResult.bPassed;
			// Declared Resolution/Evidence와 Pending semantics 검증 결과입니다.
			const FCFDAContractGuardResult ResolutionResult = ValidateMigrationResolution(ChangeDeclaration->Impact, ChangeDeclaration->Resolution, ChangeDeclaration->MigrationEvidenceId, bCanonicalStagingCompatible);
			CFDAContractGuardPrivate::AppendIssues(GateResult.Validation, ResolutionResult);
		}
	}
	// Accepted append는 모든 prerequisite가 PASS하고 실제 current change declaration이 존재할 때만 허용합니다.
	GateResult.bAcceptedSnapshotAppendAllowed = GateResult.Validation.bPassed && ChangeDeclaration != nullptr;
	// Current System promotion은 no-delta accepted baseline에서도 prerequisite가 모두 PASS하면 허용할 수 있습니다.
	GateResult.bCurrentSystemPromotionAllowed = GateResult.Validation.bPassed;
	return GateResult;
}

// Selected exact TypeKey provider의 DACE readiness를 먼저 확인한 뒤 migration gate를 평가합니다.
FCFDAMigrationGateResult FCFDAContractGuard::EvaluateMigrationGateForProvider(
	const FCFDATypeProviderEntry& ProviderEntry,
	const FCFDACurrentChangeDeclaration* ChangeDeclaration,
	const FCFDAContractGuardResult& CanonicalStagingResult)
{
	// 기존 declaration/canonical compatibility algorithm 결과입니다.
	FCFDAMigrationGateResult GateResult = EvaluateMigrationGate(ChangeDeclaration, CanonicalStagingResult);
	if (ProviderEntry.Descriptor.DaceReadiness != ECFDADaceReadiness::ContractReady)
	{
		CFDAContractGuardPrivate::AddIssue(GateResult.Validation, ECFDAContractIssueCode::AcceptedSnapshotChainInvalid, TEXT("Provider.DaceReadiness"), TEXT("DACE ContractNotReady provider는 accepted append 또는 Current promotion gate를 통과할 수 없습니다."));
		GateResult.bAcceptedSnapshotAppendAllowed = false;
		GateResult.bCurrentSystemPromotionAllowed = false;
	}
	return GateResult;
}

// Production revision guard + canonical exact3 + current migration declaration을 종합한 current Missile compatibility operational gate를 평가합니다.
FCFDAMigrationGateResult FCFDAContractGuard::EvaluateCurrentMigrationGate()
{
	// Current Missile exact provider입니다.
	const FCFDATypeProviderEntry& ProviderEntry = CFDAMissileProvider::GetProvider();
	// Current provider-owned canonical exact3 read-only compatibility 결과입니다.
	const FCFDAContractGuardResult CanonicalStagingResult = ValidateCanonicalStagingCompatibilityForProvider(ProviderEntry);
	// Provider readiness + Declaration/Resolution/Evidence 기반 migration gate입니다.
	FCFDAMigrationGateResult GateResult = EvaluateMigrationGateForProvider(ProviderEntry, GetCurrentChangeDeclaration(), CanonicalStagingResult);
	// Accepted history + actual current revision/signature/declaration 결과입니다.
	const FCFDAContractGuardResult RevisionResult = ValidateCurrentRevisionGuard();
	CFDAContractGuardPrivate::AppendIssues(GateResult.Validation, RevisionResult);
	if (!GateResult.Validation.bPassed)
	{
		GateResult.bAcceptedSnapshotAppendAllowed = false;
		GateResult.bCurrentSystemPromotionAllowed = false;
	}
	return GateResult;
}

// Bootstrap accepted snapshot의 frozen first record와 전체 append-only chain을 검증합니다.
bool FCFDAContractGuard::ValidateBootstrapAcceptedSnapshot(FString& OutError)
{
	// Production-owned accepted snapshot history입니다.
	const TArray<FCFDAAcceptedContractSnapshot>& Snapshots = GetAcceptedSnapshots();
	if (Snapshots.IsEmpty())
	{
		OutError = TEXT("DACE accepted snapshot history에 bootstrap record가 없습니다.");
		return false;
	}
	// Historical first bootstrap accepted snapshot입니다.
	const FCFDAAcceptedContractSnapshot& Bootstrap = Snapshots[0];
	if (!Bootstrap.SnapshotId.Equals(TEXT("DACE-MissileGuidePreset-S1-A2-Bootstrap"), ESearchCase::CaseSensitive)
		|| !Bootstrap.SchemaId.Equals(TEXT("CarFight.DataAsset.MissileGuidePreset"), ESearchCase::CaseSensitive)
		|| Bootstrap.SchemaRevision != 1
		|| Bootstrap.AdapterContractRevision != 2
		|| !Bootstrap.DataAssetTypeClassPath.Equals(TEXT("/Script/CarFight_Re.CFMissileGuidePresetData"), ESearchCase::CaseSensitive)
		|| !Bootstrap.PreviousSnapshotSignature.IsEmpty())
	{
		OutError = TEXT("Historical bootstrap accepted snapshot identity/revision baseline이 변경되었습니다.");
		return false;
	}
	// 전체 append-only accepted history chain 검증 결과입니다.
	const FCFDAContractGuardResult ChainResult = ValidateAcceptedSnapshotChain(Snapshots);
	if (!ChainResult.bPassed)
	{
		OutError = ChainResult.Issues.IsEmpty() ? TEXT("Accepted snapshot chain validation에 실패했습니다.") : ChainResult.Issues[0].Message;
		return false;
	}
	OutError.Reset();
	return true;
}
