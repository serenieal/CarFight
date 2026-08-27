// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleResolver.h
// Version: v1.1.0
// Date: 2026-08-26
// Description: DAUTH-P0-08E Pure Vehicle Resolver + CF-FQ-040 VB-P0-05 typed Transmission/Reference wheel profile contract입니다.
// Scope: Immutable snapshots만 받아 Frozen R0~R16 순서의 deterministic preview를 생성합니다.
// Changelog:
// - v1.1.0: VB-P0-05 Profile opt-in Transmission/Reference wheel mapping과 positive reverse-ratio validation을 포함해 ResolverContractRevision을 2로 전진.
// - v1.0.0: ResolverContractRevision=1과 Resolve entry point 최초 구현.
// Migration:
// - ResolverContractRevision=2 이전 preview/approval은 stale로 처리하고 새 preview를 생성합니다.
// - Live UObject/Asset/Slate 재조회, Target mutation, Apply/UI/CSV를 포함하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleResolverTypes.h"

/** Frozen Vehicle Authoring snapshots를 deterministic field candidate/result로 변환하는 Pure Resolver입니다. */
class FCFVehicleResolver
{
public:
	// VB-P0-05 typed Transmission/Reference wheel mapping을 포함한 current semantic contract revision입니다.
	static constexpr int32 CurrentResolverContractRevision = 2;

	// Immutable request 하나를 Frozen R0~R16 순서로 resolve합니다.
	static bool Resolve(const FCFVehicleResolveRequest& Request, FCFVehicleResolveResult& OutResult);
};
