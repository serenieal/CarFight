// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderWagonPhysicsCmdlet.h
// Version: v1.0.1
// Date: 2026-09-01
// Description: CF-FQ-040 actual Wagon PhysicsDraft v2 USER-reviewed Step 5 Profile commit을 existing Builder VM/facade로 실행하는 bounded one-shot commandlet입니다.
// Changelog:
// - v1.0.1: generated UObject constructor symbol과 일치하도록 explicit default commandlet constructor 구현 누락을 교정.
// - v1.0.0: Reference 재승인, PhysicsDraft v2 load, mutation0 preview, AuthoringWrite commit, 4 private Profile+Recipe 5-package save/rollback, Target/Evidence invariant 검증을 최초 구현.
// Migration:
// - raw Profile/VehicleData write를 만들지 않고 FCFVehicleBuilderVM의 existing Step 1/5 경로를 그대로 재사용합니다.
// - Target VehicleData는 read-only invariant 검증만 수행하며 Step 7 전에는 저장하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFBuilderWagonPhysicsCmdlet.generated.h"

/** Actual Wagon Step 5 Physics Proposal을 exact current Evidence/Recipe/Profile authority에 commit하는 Editor-only commandlet입니다. */
UCLASS()
class CARFIGHT_REEDITOR_API UCFBuilderWagonPhysicsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor commandlet 실행 속성을 준비합니다.
	UCFBuilderWagonPhysicsCommandlet();

	// Current Reference를 USER-reviewed token으로 재승인한 뒤 PhysicsDraft v2를 Preview/Commit하고 5개 authoring package만 저장합니다.
	virtual int32 Main(const FString& Params) override;
};
