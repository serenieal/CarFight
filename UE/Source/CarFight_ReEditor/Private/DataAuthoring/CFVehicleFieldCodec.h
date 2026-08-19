// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleFieldCodec.h
// Version: v1.1.0
// Date: 2026-08-17
// Description: VehicleData leaf 값을 Unreal Reflection canonical text로 변환하는 Editor-only codec입니다.
// Scope: Type signature, Export/Import, deterministic value hash를 제공합니다.
// Changelog:
// - v1.1.0: Import 뒤 trailing text를 차단하고 hash payload를 UTF-8 bytes 기준으로 고정.
// - v1.0.0: DAUTH-P0-08A generic Field Value codec 최초 구현.
// Migration:
// - UI/AI가 CanonicalValueText를 직접 편집하는 API로 사용하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Containers/StringConv.h"
#include "DataAuthoring/CFVehicleAuthoringTypes.h"
#include "Misc/SecureHash.h"
#include "UObject/UnrealType.h"

/** FProperty와 FCFVehicleFieldValue 사이의 checked canonical 변환을 담당합니다. */
class FCFVehicleFieldCodec
{
public:
	// Property 종류와 Struct/Enum/Object/Class constraint를 포함한 type signature를 만듭니다.
	static FString BuildTypeSignature(const FProperty& Property)
	{
		if (const FSoftClassProperty* SoftClassProperty = CastField<FSoftClassProperty>(&Property))
		{
			return FString::Printf(TEXT("SoftClass:%s"), *GetPathNameSafe(SoftClassProperty->MetaClass));
		}

		if (const FClassProperty* ClassProperty = CastField<FClassProperty>(&Property))
		{
			return FString::Printf(TEXT("Class:%s"), *GetPathNameSafe(ClassProperty->MetaClass));
		}

		if (const FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(&Property))
		{
			return FString::Printf(TEXT("SoftObject:%s"), *GetPathNameSafe(SoftObjectProperty->PropertyClass));
		}

		if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(&Property))
		{
			return FString::Printf(TEXT("Object:%s"), *GetPathNameSafe(ObjectProperty->PropertyClass));
		}

		if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(&Property))
		{
			return FString::Printf(TEXT("Enum:%s"), *GetPathNameSafe(EnumProperty->GetEnum()));
		}

		if (const FByteProperty* ByteProperty = CastField<FByteProperty>(&Property); ByteProperty && ByteProperty->Enum)
		{
			return FString::Printf(TEXT("Enum:%s"), *GetPathNameSafe(ByteProperty->Enum));
		}

		if (const FStructProperty* StructProperty = CastField<FStructProperty>(&Property))
		{
			return FString::Printf(TEXT("Struct:%s"), *GetPathNameSafe(StructProperty->Struct));
		}

		return Property.GetClass()->GetName();
	}

	// Property value를 type signature와 canonical text로 export합니다.
	static bool ExportValue(const FProperty& Property, const void* ValueAddress, FCFVehicleFieldValue& OutValue, FString& OutError)
	{
		if (!ValueAddress)
		{
			OutError = TEXT("ValueAddress가 null입니다.");
			return false;
		}

		OutValue.PropertyTypeSignature = BuildTypeSignature(Property);
		OutValue.CanonicalValueText.Reset();
		Property.ExportTextItem_Direct(OutValue.CanonicalValueText, ValueAddress, nullptr, nullptr, PPF_None);
		OutError.Reset();
		return true;
	}

	// Canonical value의 type signature를 검사한 뒤 Property storage에 import합니다.
	static bool ImportValue(const FProperty& Property, void* ValueAddress, UObject* OwnerObject, const FCFVehicleFieldValue& InValue, FString& OutError)
	{
		if (!ValueAddress)
		{
			OutError = TEXT("ValueAddress가 null입니다.");
			return false;
		}

		// 실제 Property에서 다시 계산한 현재 type signature입니다.
		const FString CurrentTypeSignature = BuildTypeSignature(Property);
		if (CurrentTypeSignature != InValue.PropertyTypeSignature)
		{
			OutError = FString::Printf(
				TEXT("Field type signature mismatch. Expected=%s Actual=%s"),
				*CurrentTypeSignature,
				*InValue.PropertyTypeSignature);
			return false;
		}

		// Unreal Reflection parser가 성공하면 canonical text 뒤의 위치를 반환합니다.
		const TCHAR* ImportResult = Property.ImportText_Direct(
			*InValue.CanonicalValueText,
			ValueAddress,
			OwnerObject,
			PPF_None);
				if (!ImportResult)
		{
			OutError = FString::Printf(TEXT("Canonical value import에 실패했습니다: %s"), *InValue.CanonicalValueText);
			return false;
		}

		// Property parser가 소비하지 않고 남긴 trailing text입니다.
		const FString RemainingText = FString(ImportResult).TrimStartAndEnd();
		if (!RemainingText.IsEmpty())
		{
			OutError = FString::Printf(TEXT("Canonical value 뒤에 해석되지 않은 text가 남았습니다: %s"), *RemainingText);
			return false;
		}

		OutError.Reset();
		return true;
	}

		// Type signature와 canonical text를 UTF-8 bytes로 고정해 deterministic field value hash를 만듭니다.
	static FString HashValue(const FCFVehicleFieldValue& Value)
	{
		// Type과 value 사이 충돌을 막기 위한 canonical hash payload입니다.
		const FString HashPayload = Value.PropertyTypeSignature + TEXT("\n") + Value.CanonicalValueText;
		// TCHAR 플랫폼 표현과 분리된 canonical UTF-8 payload입니다.
		const FTCHARToUTF8 Utf8Payload(*HashPayload);
		// UTF-8 bytes의 MD5 digest를 계산할 상태입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());
		// MD5 128-bit 결과 bytes입니다.
		uint8 Digest[16];
		Md5.Final(Digest);

		// Platform-independent lowercase hexadecimal 결과 문자열입니다.
		FString HexResult;
		HexResult.Reserve(32);
		// 한 nibble을 lowercase hex로 변환할 고정 문자표입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			HexResult.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			HexResult.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return HexResult;
	}
};
