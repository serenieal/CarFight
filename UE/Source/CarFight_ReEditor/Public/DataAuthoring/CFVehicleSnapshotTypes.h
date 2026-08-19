// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleSnapshotTypes.h
// Version: v1.1.0
// Date: 2026-08-17
// Description: DAUTH-P0-08C/D Immutable Snapshot Foundation의 public value-copy 계약입니다.
// Scope: Recipe, 5 Profile, Registry-expanded Definition, Asset Snapshot을 제공합니다.
// Changelog:
// - v1.1.0: DAUTH-P0-08D Chassis Socket / Wheel Bounds Asset Snapshot 계약 추가.
// - v1.0.0: Section 22.13/22.27 Snapshot/Fingerprint public contract 최초 구현.
// Migration:
// - Snapshot은 Editor-only Authoring 계산 입력이며 Runtime UCFVehicleData schema를 변경하지 않습니다.
// - Resolver / Apply / UI / CSV 계약은 이 파일에서 구현하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleAuthoringTypes.h"
#include "DataAuthoring/CFVehicleProfileTypes.h"
#include "CFVehicleSnapshotTypes.generated.h"

/** Registry descriptor가 실제 Definition element까지 확장된 exact field/value entry입니다. */
USTRUCT()
struct FCFVehicleFieldEntry
{
	GENERATED_BODY()

	// Scalar 또는 Stable-ID selector가 확정된 exact Definition field path입니다.
	UPROPERTY()
	FCFVehicleFieldPath FieldPath;

	// Reflection codec으로 읽은 type signature 포함 canonical field value입니다.
	UPROPERTY()
	FCFVehicleFieldValue Value;
};

/** Persistent Recipe UObject를 다시 읽지 않고 Resolve에 사용할 immutable-style value copy입니다. */
USTRUCT()
struct FCFVehicleRecipeSnapshot
{
	GENERATED_BODY()

	// Snapshot을 만든 Recipe record의 diagnostic identity입니다.
	UPROPERTY()
	FGuid RecipeId;

	// Target UObject hard reference 대신 보존하는 soft object path입니다.
	UPROPERTY()
	FSoftObjectPath TargetVehicleDataPath;

	// Profile 추천/표시용 semantic archetype identity의 value copy입니다.
	UPROPERTY()
	FName VehicleArchetypeId = NAME_None;

	// Chassis/Wheel asset과 wheel socket binding intent의 value copy입니다.
	UPROPERTY()
	FCFVehicleAssetIntent AssetIntent;

	// 5개 typed Profile binding의 value copy입니다.
	UPROPERTY()
	FCFVehicleProfileBindings ProfileBindings;

	// Frozen 4축 Driving Feel intent의 value copy입니다.
	UPROPERTY()
	FCFVehicleFeelIntent DrivingFeelIntent;

	// Base/Gross mass semantic intent의 value copy입니다.
	UPROPERTY()
	FCFVehicleMassIntent MassIntent;

	// Vehicle MaxHealth semantic intent의 value copy입니다.
	UPROPERTY()
	FCFVehicleDurabilityIntent DurabilityIntent;

	// Stable-ID Hardpoint semantic intent의 ordered value copy입니다.
	UPROPERTY()
	TArray<FCFHardpointIntent> HardpointIntents;

	// Stable-ID Mount semantic intent의 ordered value copy입니다.
	UPROPERTY()
	TArray<FCFMountIntent> MountIntents;

	// Defense/Destroyed FX semantic intent의 value copy입니다.
	UPROPERTY()
	FCFVehicleDefaultIntent DefaultDataIntent;

	// WheelVisual semantic policy의 value copy입니다.
	UPROPERTY()
	FCFWheelVisualIntent WheelVisualIntent;

	// ProjectDefault / VehicleSpecific DriveState mode의 value copy입니다.
	UPROPERTY()
	ECFVehicleDriveStateMode DriveStateMode = ECFVehicleDriveStateMode::ProjectDefault;

	// Accepted measurement decision/fingerprint의 value copy입니다.
	UPROPERTY()
	FCFVehicleAssetAdoption AssetAdoption;

	// Exceptional Advanced Leaf Override의 value copy입니다.
	UPROPERTY()
	TArray<FCFVehicleFieldOverride> AdvancedOverrides;

	// Legacy Pin / Serialized Passthrough / Adoption baseline의 value copy입니다.
	UPROPERTY()
	FCFVehicleImportState ImportState;

	// Stale/Drift 비교에 필요한 마지막 Apply baseline의 value copy입니다.
	UPROPERTY()
	FCFVehicleAppliedState AppliedState;

	// 사람이 읽는 diagnostic sequence이며 fingerprint authority는 아닙니다.
	UPROPERTY()
	int32 AuthoringRevision = 0;

	// Section 22.27 Authoring Intent canonical payload만 hash한 deterministic fingerprint입니다.
	UPROPERTY()
	FString RecipeFingerprint;
};

/** Shared Profile UObject의 identity/revision과 resolver payload fingerprint를 분리한 source record입니다. */
USTRUCT()
struct FCFVehicleProfileSource
{
	GENERATED_BODY()

	// Profile Asset identity로 사용하는 soft object path입니다.
	UPROPERTY()
	FSoftObjectPath SourceObjectPath;

	// 사람이 읽는 Profile edit diagnostic revision입니다.
	UPROPERTY()
	int32 AuthoringRevision = 0;

	// DisplayName/Description을 제외한 typed resolver payload fingerprint입니다.
	UPROPERTY()
	FString ProfileFingerprint;

	// 실제 Profile source가 Snapshot에 포함되어 있는지 반환합니다.
	bool IsPresent() const
	{
		return SourceObjectPath.IsValid();
	}
};

/** Frozen 5개 Profile Domain을 UObject 없이 Pure Resolver에 넘길 typed Snapshot Set입니다. */
USTRUCT()
struct FCFVehicleProfileSnapshotSet
{
	GENERATED_BODY()

	// Vehicle Base Profile source identity/revision/fingerprint입니다.
	UPROPERTY()
	FCFVehicleProfileSource BaseSource;

	// Vehicle Base Profile typed resolver payload입니다.
	UPROPERTY()
	FCFVehicleBaseProfileData BaseData;

	// Drivetrain Profile source identity/revision/fingerprint입니다.
	UPROPERTY()
	FCFVehicleProfileSource DrivetrainSource;

	// Drivetrain Profile typed resolver payload입니다.
	UPROPERTY()
	FCFDrivetrainProfileData DrivetrainData;

	// Handling Profile source identity/revision/fingerprint입니다.
	UPROPERTY()
	FCFVehicleProfileSource HandlingSource;

	// Handling Profile typed resolver payload입니다.
	UPROPERTY()
	FCFHandlingProfileData HandlingData;

	// Performance Profile source identity/revision/fingerprint입니다.
	UPROPERTY()
	FCFVehicleProfileSource PerformanceSource;

	// Performance Profile typed resolver payload입니다.
	UPROPERTY()
	FCFPerformanceProfileData PerformanceData;

	// DriveState Profile source identity/revision/fingerprint입니다. ProjectDefault mode에서는 비어 있을 수 있습니다.
	UPROPERTY()
	FCFVehicleProfileSource DriveStateSource;

	// Runtime gate를 제외한 DriveState behavior 14개 typed payload입니다.
	UPROPERTY()
	FCFDriveStateProfileData DriveStateData;
};

/** Current Target 또는 Project Compatibility Default를 같은 codec/order/hash로 표현하는 raw Definition Snapshot입니다. */
USTRUCT()
struct FCFVehicleDefinitionSnapshot
{
	GENERATED_BODY()

	// Canonical Stable Field Path 오름차순으로 정렬된 Registry-expanded field entry입니다.
	UPROPERTY()
	TArray<FCFVehicleFieldEntry> SortedFields;

	// SortedFields의 path/type/value canonical payload deterministic hash입니다.
	UPROPERTY()
	FString DefinitionHash;
};

/** Chassis StaticMesh에서 요청한 socket 하나의 resolver-relevant relative transform 사실입니다. */
USTRUCT()
struct FCFVehicleSocketSnapshot
{
	GENERATED_BODY()

	// Recipe의 Wheel/Hardpoint binding이 요청한 Chassis socket 이름입니다.
	UPROPERTY()
	FName SocketName = NAME_None;

	// 현재 Chassis StaticMesh에서 이 socket을 실제로 찾았는지 여부입니다.
	UPROPERTY()
	bool bFound = false;

	// 찾은 socket의 StaticMesh-local relative location입니다.
	UPROPERTY()
	FVector RelativeLocation = FVector::ZeroVector;

	// 찾은 socket의 StaticMesh-local relative rotation입니다.
	UPROPERTY()
	FRotator RelativeRotation = FRotator::ZeroRotator;

	// 찾은 socket의 StaticMesh-local relative scale입니다.
	UPROPERTY()
	FVector RelativeScale = FVector::OneVector;
};

/** Wheel StaticMesh 하나의 object identity와 local bounds measurement 사실입니다. */
USTRUCT()
struct FCFVehicleWheelAssetSnapshot
{
	GENERATED_BODY()

	// Recipe가 선택한 Wheel StaticMesh의 soft object path입니다.
	UPROPERTY()
	FSoftObjectPath ObjectPath;

	// non-empty soft path가 실제 UStaticMesh로 resolve되었는지 여부입니다.
	UPROPERTY()
	bool bAssetLoaded = false;

	// UStaticMesh local bounds origin입니다.
	UPROPERTY()
	FVector BoundsOrigin = FVector::ZeroVector;

	// UStaticMesh local bounds box extent입니다.
	UPROPERTY()
	FVector BoundsExtent = FVector::ZeroVector;

	// Object Path + Bounds Origin/Extent만 hash한 resolver-relevant measurement fingerprint입니다.
	UPROPERTY()
	FString MeasureFingerprint;
};

/** Pure Resolver가 live StaticMesh를 다시 읽지 않도록 필요한 asset 사실만 value-copy한 Snapshot입니다. */
USTRUCT()
struct FCFVehicleAssetSnapshot
{
	GENERATED_BODY()

	// Recipe가 선택한 Chassis StaticMesh의 soft object path입니다.
	UPROPERTY()
	FSoftObjectPath ChassisObjectPath;

	// non-empty Chassis soft path가 실제 UStaticMesh로 resolve되었는지 여부입니다.
	UPROPERTY()
	bool bChassisLoaded = false;

	// Wheel/Hardpoint binding에서 요청한 socket을 이름순으로 dedupe한 relative transform 사실입니다.
	UPROPERTY()
	TArray<FCFVehicleSocketSnapshot> ChassisSockets;

	// Chassis Object Path + requested socket names/found state/relative transforms의 deterministic fingerprint입니다.
	UPROPERTY()
	FString ChassisLayoutFingerprint;

	// 앞왼쪽 Wheel StaticMesh measurement snapshot입니다.
	UPROPERTY()
	FCFVehicleWheelAssetSnapshot WheelFL;

	// 앞오른쪽 Wheel StaticMesh measurement snapshot입니다.
	UPROPERTY()
	FCFVehicleWheelAssetSnapshot WheelFR;

	// 뒤왼쪽 Wheel StaticMesh measurement snapshot입니다.
	UPROPERTY()
	FCFVehicleWheelAssetSnapshot WheelRL;

	// 뒤오른쪽 Wheel StaticMesh measurement snapshot입니다.
	UPROPERTY()
	FCFVehicleWheelAssetSnapshot WheelRR;

	// 이름으로 Chassis socket fact를 찾습니다.
	const FCFVehicleSocketSnapshot* FindChassisSocket(const FName SocketName) const
	{
		return ChassisSockets.FindByPredicate([SocketName](const FCFVehicleSocketSnapshot& SocketSnapshot)
		{
			return SocketSnapshot.SocketName == SocketName;
		});
	}
};
