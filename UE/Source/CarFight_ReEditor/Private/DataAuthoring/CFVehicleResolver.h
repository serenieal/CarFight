// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleResolver.h
// Version: v1.6.0
// Date: 2026-09-28
// Description: Pure Vehicle Resolver에 VCFX ReferenceMaxSpeed / Camera Presentation typed field contract를 추가합니다.
// Scope: Immutable snapshots만 받아 Frozen R0~R16 순서의 deterministic preview를 생성합니다.
// Changelog:
// - v1.6.0: ReferenceMaxSpeedKmh + CameraPresentationDataOverride 137-leaf 계약을 포함해 ResolverContractRevision을 7로 전진.
// - v1.5.0: DefaultSensorData 135번째 field와 Project Basic Sensor / Recipe Sensorless semantic을 포함해 ResolverContractRevision을 6으로 전진.
// - v1.4.0: ESH-01 bUseEngineTorqueCurve + atomic EngineTorqueCurve 2개 field와 Profile opt-in validation을 포함해 ResolverContractRevision을 5로 전진.
// - v1.3.0: WSA-P0-02 SocketScaleFromChassis의 Radius/Width derivation과 narrow size fingerprint semantic을 포함해 ResolverContractRevision을 4로 전진.
// - v1.2.0: WSA-P0-01 WheelAnchor RelativeScale 4개 + bUseWheelSocketScale 1개로 Resolver field/hash schema가 127→132로 확장되어 ResolverContractRevision을 3으로 전진.
// - v1.1.0: VB-P0-05 Profile opt-in Transmission/Reference wheel mapping과 positive reverse-ratio validation을 포함해 ResolverContractRevision을 2로 전진.
// - v1.0.0: ResolverContractRevision=1과 Resolve entry point 최초 구현.
// Migration:
// - ResolverContractRevision=6 이전 preview/approval/Builder receipt는 DefaultSensorData Project Basic/Sensorless semantic을 포함하지 않으므로 stale로 처리하고 새 preview를 생성합니다.
// - ResolverContractRevision=5 이전 preview/approval/Builder receipt는 vehicle-specific Engine TorqueCurve semantic을 포함하지 않으므로 stale로 처리하고 새 preview를 생성합니다.
// - ResolverContractRevision=4는 WSA-P0-02 SocketScaleFromChassis derived physics historical revision입니다.
// - ResolverContractRevision=4 이전 preview/approval/Builder receipt는 SocketScaleFromChassis derived physics semantic을 포함하지 않으므로 stale로 처리하고 새 preview를 생성합니다.
// - ResolverContractRevision=3은 WSA-P0-01 132-leaf schema-only historical revision입니다.
// - ResolverContractRevision=2는 VB-P0-05 127-leaf Transmission/Reference wheel 계약의 historical revision입니다.
// - Live UObject/Asset/Slate 재조회, Target mutation, Apply/UI/CSV를 포함하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleResolverTypes.h"

/** Frozen Vehicle Authoring snapshots를 deterministic field candidate/result로 변환하는 Pure Resolver입니다. */
class FCFVehicleResolver
{
public:
	// Camera Driving FX typed field semantic까지 포함한 current Resolver contract revision입니다.
	static constexpr int32 CurrentResolverContractRevision = 7;

	// Immutable request 하나를 Frozen R0~R16 순서로 resolve합니다.
	static bool Resolve(const FCFVehicleResolveRequest& Request, FCFVehicleResolveResult& OutResult);
};
