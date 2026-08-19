// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleMaterializer.h
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08F transient UCFVehicleData Definition Materializer / Validation 계약입니다.
// Scope: SortedResolvedFields를 transient candidate로 복원하고 readback snapshot과 기존 UCFVDAValidator 결과를 value-copy로 반환합니다.
// Changelog:
// - v1.0.0: Stable-ID Hardpoint/Mount 재구성, FieldCodec ImportValue, resolved projection readback, UCFVDAValidator bridge 최초 구현.
// Migration:
// - 생성하는 UCFVehicleData는 GetTransientPackage() 소유 RF_Transient object이며 Content Asset을 저장하거나 수정하지 않습니다.
// - transient validation 배열 순서는 stable selector lexical order이며 실제 Apply의 persisted array order 정책을 대체하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFVehicleResolverTypes.h"

/** R15 transient materialization 결과 중 UObject lifetime 밖으로 보존할 value-copy evidence입니다. */
struct FCFVehicleMaterializationResult
{
	// Materialized candidate에서 Resolver-owned field만 다시 읽은 deterministic projection Snapshot입니다.
	FCFVehicleDefinitionSnapshot ResolvedReadbackSnapshot;

	// 기존 UCFVDAValidator 결과를 Authoring validation contract로 변환한 issues입니다.
	TArray<FCFVehicleValidationIssue> DefinitionValidation;

	// Transient candidate에 실제 생성한 Hardpoint stable selector 순서입니다.
	TArray<FName> HardpointOrder;

	// Transient candidate에 실제 생성한 Mount stable selector 순서입니다.
	TArray<FName> MountOrder;

	// Definition Error/Blocked가 있어 Apply eligibility를 막아야 하는지 여부입니다.
	bool bHasBlockingValidation = false;
};

/** Resolver leaf set을 transient Runtime schema candidate로 복원하는 Editor-only Materializer입니다. */
class FCFVehicleMaterializer
{
public:
	// SortedResolvedFields를 transient UCFVehicleData로 복원하고 Validator/readback evidence를 value-copy로 반환합니다.
	static bool MaterializeAndValidate(
		const TArray<FCFVehicleResolvedField>& SortedResolvedFields,
		FCFVehicleMaterializationResult& OutResult,
		FString& OutError);
};
