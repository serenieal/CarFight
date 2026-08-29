// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFWSAMigrateCmdlet.h
// Version: v1.1.1
// Date: 2026-08-28
// Description: WSA-P0-05 Wagon 전용 Legacy Migration을 기존 typed Data Authoring 경계로 실행하는 bounded Editor commandlet입니다.
// Changelog:
// - v1.1.1: Step 2~4 exact deferred blocker에서는 Recipe-only WSA truth 저장 + Target Step 7 defer를 허용하고, 그 외에는 기존 fail-closed/Apply 계약을 유지하는 현재 실행 경계를 문서화.
// - v1.0.2: canonical Wheel의 실제 bounds 근사 오차를 공용 Wheel Size compatibility tolerance로 판정하도록 preflight 계약을 교정.
// - v1.0.1: 두 Wagon package Save 중간 실패 시 pair rollback을 수행하는 실행 계약을 반영.
// - v1.0.0: Wagon exact preflight, typed Asset/SocketScale intent commit, Socket-derived Wheel measurement 4건 채택, normal Definition Apply, exact 2-package Save를 추가.
// Migration:
// - 이 commandlet은 DA_Recipe_Wagon / DA_Vehicle_Wagon만 대상으로 하며 Sedan/SUV/test/legacy VehicleData를 수정하지 않습니다.
// - Raw UObject property writer를 만들지 않고 FCFVehicleAuthoringVM의 기존 Preview/Approval/Commit/Apply 경계만 사용합니다.
// - WSA 자체 검증은 항상 fail-closed입니다. exact Wagon Step 2~4 blocker signature에서는 Recipe WSA intent/adoption만 저장하고 Target Definition Apply는 Builder Step 7까지 defer합니다.
// - Preview가 Apply-ready인 경우에만 normal Definition Apply 뒤 Target+Recipe 두 package를 저장합니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFWSAMigrateCmdlet.generated.h"

/** WSA-P0-05에서 USER 승인된 Wagon SocketScale migration만 수행하는 Editor-only commandlet입니다. */
UCLASS()
class CARFIGHT_REEDITOR_API UCFWSAMigrateCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor commandlet 실행 속성을 준비합니다.
	UCFWSAMigrateCommandlet();

	// Wagon exact current truth를 검증하고 typed migration 뒤 Step 2~4면 Recipe-only 저장, Apply-ready면 Definition Apply + 두 패키지 저장을 수행합니다.
	virtual int32 Main(const FString& Params) override;
};
