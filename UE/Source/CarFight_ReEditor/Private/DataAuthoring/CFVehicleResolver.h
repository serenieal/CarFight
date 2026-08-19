// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleResolver.h
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08E Pure Vehicle Resolver entry contract입니다.
// Scope: Immutable snapshots만 받아 Frozen R0~R16 순서의 deterministic preview를 생성합니다.
// Changelog:
// - v1.0.0: ResolverContractRevision=1과 Resolve entry point 최초 구현.
// Migration:
// - Live UObject/Asset/Slate 재조회, Target mutation, Apply/UI/CSV를 포함하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleResolverTypes.h"

/** Frozen Vehicle Authoring snapshots를 deterministic field candidate/result로 변환하는 Pure Resolver입니다. */
class FCFVehicleResolver
{
public:
	// P0 최초 Frozen semantic contract revision입니다.
	static constexpr int32 CurrentResolverContractRevision = 1;

	// Immutable request 하나를 Frozen R0~R16 순서로 resolve합니다.
	static bool Resolve(const FCFVehicleResolveRequest& Request, FCFVehicleResolveResult& OutResult);
};
