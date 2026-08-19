// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAssetReader.cpp
// Version: v1.0.0
// Date: 2026-08-17
// Description: Frozen Section 22.13/22.27 Asset Snapshot Reader와 resolver-relevant fingerprint 구현입니다.
// Changelog:
// - v1.0.0: Chassis requested socket extraction, Wheel local bounds extraction, deterministic asset fingerprint 구현.
// Migration:
// - 기존 UCFVehicleData::CaptureLayoutFromChassisSockets() direct mutation 경로는 변경하지 않습니다.
// - Reader는 UObject/Asset을 읽기만 하며 Resolver/Apply/Save를 수행하지 않습니다.

#include "DataAuthoring/CFVehicleAssetReader.h"

#include "Containers/StringConv.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "Misc/SecureHash.h"

namespace CFVehicleAssetReaderPrivate
{
	// Asset Snapshot canonical payload format revision입니다.
	static constexpr int32 AssetSnapshotHashFormatRevision = 1;

	// Recipe에서 Wheel socket 이름이 비어 있을 때 기존 Layout Capture와 동일하게 사용할 프로젝트 기본 이름입니다.
	FName ResolveWheelSocketName(const FName ConfiguredSocketName, const TCHAR* DefaultSocketName)
	{
		return ConfiguredSocketName.IsNone() ? FName(DefaultSocketName) : ConfiguredSocketName;
	}

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

	// Delimiter 충돌 없이 canonical payload 조각을 label/문자수/value 형태로 추가합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("\n");
	}

	// signed zero를 하나로 정규화한 floating-point canonical text를 반환합니다.
	FString ToCanonicalNumber(const double Value)
	{
		// +0/-0 표현 차이를 제거한 canonical numeric value입니다.
		const double NormalizedValue = Value == 0.0 ? 0.0 : Value;
		return FString::Printf(TEXT("%.17g"), NormalizedValue);
	}

	// FVector를 X/Y/Z 고정 순서 canonical payload로 추가합니다.
	void AppendVector(FString& OutPayload, const TCHAR* Label, const FVector& Value)
	{
		AppendToken(OutPayload, Label, FString::Printf(
			TEXT("%s,%s,%s"),
			*ToCanonicalNumber(Value.X),
			*ToCanonicalNumber(Value.Y),
			*ToCanonicalNumber(Value.Z)));
	}

	// FRotator를 Pitch/Yaw/Roll 고정 순서 canonical payload로 추가합니다.
	void AppendRotator(FString& OutPayload, const TCHAR* Label, const FRotator& Value)
	{
		AppendToken(OutPayload, Label, FString::Printf(
			TEXT("%s,%s,%s"),
			*ToCanonicalNumber(Value.Pitch),
			*ToCanonicalNumber(Value.Yaw),
			*ToCanonicalNumber(Value.Roll)));
	}

	// Recipe와 Hardpoint intent에서 Chassis에 실제 요청할 socket 이름을 dedupe/sort하여 수집합니다.
	TArray<FName> CollectRequestedSocketNames(const FCFVehicleRecipeSnapshot& RecipeSnapshot)
	{
		// 중복 socket binding을 하나의 Chassis fact로 합칠 unique name 집합입니다.
		TSet<FName> UniqueSocketNames;
		UniqueSocketNames.Add(ResolveWheelSocketName(RecipeSnapshot.AssetIntent.BodyWheelSocketFL, TEXT("Wheel_Anchor_FL")));
		UniqueSocketNames.Add(ResolveWheelSocketName(RecipeSnapshot.AssetIntent.BodyWheelSocketFR, TEXT("Wheel_Anchor_FR")));
		UniqueSocketNames.Add(ResolveWheelSocketName(RecipeSnapshot.AssetIntent.BodyWheelSocketRL, TEXT("Wheel_Anchor_RL")));
		UniqueSocketNames.Add(ResolveWheelSocketName(RecipeSnapshot.AssetIntent.BodyWheelSocketRR, TEXT("Wheel_Anchor_RR")));

		for (const FCFHardpointIntent& HardpointIntent : RecipeSnapshot.HardpointIntents)
		{
			if (!HardpointIntent.SocketName.IsNone())
			{
				UniqueSocketNames.Add(HardpointIntent.SocketName);
			}
		}

		// Fingerprint와 lookup 결과가 container iteration order에 의존하지 않게 할 sorted socket name 목록입니다.
		TArray<FName> SortedSocketNames = UniqueSocketNames.Array();
		SortedSocketNames.Sort(FNameLexicalLess());
		return SortedSocketNames;
	}

	// Chassis StaticMesh에서 요청한 socket 하나를 immutable fact로 복사합니다.
	FCFVehicleSocketSnapshot ReadSocketFact(const UStaticMesh* ChassisMesh, const FName SocketName)
	{
		// 요청한 socket name과 found state를 보존할 결과 fact입니다.
		FCFVehicleSocketSnapshot SocketSnapshot;
		SocketSnapshot.SocketName = SocketName;
		if (!ChassisMesh)
		{
			return SocketSnapshot;
		}

		// Chassis StaticMesh에서 찾은 socket object입니다.
		const UStaticMeshSocket* MeshSocket = ChassisMesh->FindSocket(SocketName);
		if (!MeshSocket)
		{
			return SocketSnapshot;
		}

		SocketSnapshot.bFound = true;
		SocketSnapshot.RelativeLocation = MeshSocket->RelativeLocation;
		SocketSnapshot.RelativeRotation = MeshSocket->RelativeRotation;
		SocketSnapshot.RelativeScale = MeshSocket->RelativeScale;
		return SocketSnapshot;
	}

	// Chassis identity와 실제 요청한 socket transform facts만 사용해 Layout fingerprint를 계산합니다.
	FString BuildChassisLayoutFingerprint(const FCFVehicleAssetSnapshot& Snapshot)
	{
		// Chassis layout resolver가 소비할 facts의 canonical payload입니다.
		FString CanonicalPayload;
		AppendToken(CanonicalPayload, TEXT("Kind"), TEXT("VehicleChassisLayout"));
		AppendToken(CanonicalPayload, TEXT("HashFormatRevision"), FString::FromInt(AssetSnapshotHashFormatRevision));
		AppendToken(CanonicalPayload, TEXT("ObjectPath"), Snapshot.ChassisObjectPath.ToString());
		AppendToken(CanonicalPayload, TEXT("AssetLoaded"), Snapshot.bChassisLoaded ? TEXT("1") : TEXT("0"));
		AppendToken(CanonicalPayload, TEXT("SocketCount"), FString::FromInt(Snapshot.ChassisSockets.Num()));
		for (const FCFVehicleSocketSnapshot& SocketSnapshot : Snapshot.ChassisSockets)
		{
			AppendToken(CanonicalPayload, TEXT("SocketName"), SocketSnapshot.SocketName.ToString());
			AppendToken(CanonicalPayload, TEXT("Found"), SocketSnapshot.bFound ? TEXT("1") : TEXT("0"));
			if (SocketSnapshot.bFound)
			{
				AppendVector(CanonicalPayload, TEXT("Location"), SocketSnapshot.RelativeLocation);
				AppendRotator(CanonicalPayload, TEXT("Rotation"), SocketSnapshot.RelativeRotation);
				AppendVector(CanonicalPayload, TEXT("Scale"), SocketSnapshot.RelativeScale);
			}
		}
		return HashUtf8Payload(CanonicalPayload);
	}

	// Wheel object identity와 local bounds facts만 사용해 measurement fingerprint를 계산합니다.
	FString BuildWheelMeasureFingerprint(const FCFVehicleWheelAssetSnapshot& Snapshot)
	{
		// Wheel measurement resolver가 소비할 facts의 canonical payload입니다.
		FString CanonicalPayload;
		AppendToken(CanonicalPayload, TEXT("Kind"), TEXT("VehicleWheelMeasure"));
		AppendToken(CanonicalPayload, TEXT("HashFormatRevision"), FString::FromInt(AssetSnapshotHashFormatRevision));
		AppendToken(CanonicalPayload, TEXT("ObjectPath"), Snapshot.ObjectPath.ToString());
		AppendToken(CanonicalPayload, TEXT("AssetLoaded"), Snapshot.bAssetLoaded ? TEXT("1") : TEXT("0"));
		if (Snapshot.bAssetLoaded)
		{
			AppendVector(CanonicalPayload, TEXT("BoundsOrigin"), Snapshot.BoundsOrigin);
			AppendVector(CanonicalPayload, TEXT("BoundsExtent"), Snapshot.BoundsExtent);
		}
		return HashUtf8Payload(CanonicalPayload);
	}

	// Soft Wheel ref 하나를 load/read하여 object path, local bounds와 measurement fingerprint를 만듭니다.
	bool ReadWheelSnapshot(
		const TSoftObjectPtr<UStaticMesh>& WheelMeshReference,
		FCFVehicleWheelAssetSnapshot& OutWheelSnapshot,
		FString& OutError)
	{
		OutWheelSnapshot = FCFVehicleWheelAssetSnapshot();
		OutWheelSnapshot.ObjectPath = WheelMeshReference.ToSoftObjectPath();
		if (!OutWheelSnapshot.ObjectPath.IsValid())
		{
			OutWheelSnapshot.MeasureFingerprint = BuildWheelMeasureFingerprint(OutWheelSnapshot);
			return true;
		}

		// Bounds를 읽을 실제 Wheel StaticMesh입니다.
		UStaticMesh* WheelMesh = WheelMeshReference.LoadSynchronous();
		if (!WheelMesh)
		{
			OutError = FString::Printf(TEXT("Wheel StaticMesh를 resolve할 수 없습니다: %s"), *OutWheelSnapshot.ObjectPath.ToString());
			return false;
		}

		// StaticMesh asset local bounds입니다.
		const FBoxSphereBounds WheelBounds = WheelMesh->GetBounds();
		OutWheelSnapshot.bAssetLoaded = true;
		OutWheelSnapshot.BoundsOrigin = WheelBounds.Origin;
		OutWheelSnapshot.BoundsExtent = WheelBounds.BoxExtent;
		OutWheelSnapshot.MeasureFingerprint = BuildWheelMeasureFingerprint(OutWheelSnapshot);
		return true;
	}
}

// Recipe Snapshot이 요청한 Chassis socket facts와 4개 Wheel bounds를 resolver-safe value copy로 읽습니다.
bool FCFVehicleAssetReader::BuildAssetSnapshot(
	const FCFVehicleRecipeSnapshot& RecipeSnapshot,
	FCFVehicleAssetSnapshot& OutSnapshot,
	FString& OutError)
{
	OutSnapshot = FCFVehicleAssetSnapshot();
	OutError.Reset();

	OutSnapshot.ChassisObjectPath = RecipeSnapshot.AssetIntent.ChassisMesh.ToSoftObjectPath();
	// Chassis Socket fact를 읽을 실제 StaticMesh입니다. Empty ref는 Setup Incomplete 상태로 허용합니다.
	UStaticMesh* ChassisMesh = nullptr;
	if (OutSnapshot.ChassisObjectPath.IsValid())
	{
		ChassisMesh = RecipeSnapshot.AssetIntent.ChassisMesh.LoadSynchronous();
		if (!ChassisMesh)
		{
			OutError = FString::Printf(TEXT("Chassis StaticMesh를 resolve할 수 없습니다: %s"), *OutSnapshot.ChassisObjectPath.ToString());
			return false;
		}
		OutSnapshot.bChassisLoaded = true;
	}

	// Recipe binding에서 실제 요청한 canonical socket name 목록입니다.
	const TArray<FName> RequestedSocketNames = CFVehicleAssetReaderPrivate::CollectRequestedSocketNames(RecipeSnapshot);
	OutSnapshot.ChassisSockets.Reserve(RequestedSocketNames.Num());
	for (const FName SocketName : RequestedSocketNames)
	{
		OutSnapshot.ChassisSockets.Add(CFVehicleAssetReaderPrivate::ReadSocketFact(ChassisMesh, SocketName));
	}
	OutSnapshot.ChassisLayoutFingerprint = CFVehicleAssetReaderPrivate::BuildChassisLayoutFingerprint(OutSnapshot);

	if (!CFVehicleAssetReaderPrivate::ReadWheelSnapshot(RecipeSnapshot.AssetIntent.WheelMeshFL, OutSnapshot.WheelFL, OutError)
		|| !CFVehicleAssetReaderPrivate::ReadWheelSnapshot(RecipeSnapshot.AssetIntent.WheelMeshFR, OutSnapshot.WheelFR, OutError)
		|| !CFVehicleAssetReaderPrivate::ReadWheelSnapshot(RecipeSnapshot.AssetIntent.WheelMeshRL, OutSnapshot.WheelRL, OutError)
		|| !CFVehicleAssetReaderPrivate::ReadWheelSnapshot(RecipeSnapshot.AssetIntent.WheelMeshRR, OutSnapshot.WheelRR, OutError))
	{
		return false;
	}

	OutError.Reset();
	return true;
}
