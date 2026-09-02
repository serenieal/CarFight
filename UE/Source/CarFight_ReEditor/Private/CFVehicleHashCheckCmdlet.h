// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleHashCheckCmdlet.h
// Version: v1.0.0
// Date: 2026-09-02
// Description: saved UCFVehicleData의 current semantic DefinitionHash를 공용 Snapshot authority로 검증하는 Editor-only read-only commandlet입니다.
// Changelog:
// - v1.0.0: VehicleData object path + expected DefinitionHash를 입력받아 FCFVehicleSnapshotBuilder current hash와 exact 비교하고 mismatch를 fail-closed.
// Migration:
// - Product Asset/Recipe/Profile/Map/Config를 수정하거나 저장하지 않습니다.
// - expected hash를 제공한 benchmark/evidence wrapper의 stale-target 실행을 차단하는 preflight 전용입니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFVehicleHashCheckCmdlet.generated.h"

/** saved VehicleData의 semantic DefinitionHash가 caller expected identity와 일치하는지 read-only 검증합니다. */
UCLASS()
class CARFIGHT_REEDITOR_API UCFVehicleHashCheckCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor read-only commandlet 실행 속성을 준비합니다.
	UCFVehicleHashCheckCommandlet();

	// VehicleData를 load하고 current DefinitionHash를 계산해 expected hash와 exact 비교합니다.
	virtual int32 Main(const FString& Params) override;
};
