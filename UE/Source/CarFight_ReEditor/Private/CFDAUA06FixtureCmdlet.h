// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFDAUA06FixtureCmdlet.h
// Version: v1.0.0
// Date: 2026-08-21
// Description: DAUTH-P0-12 UA-06 Explicit Apply / Undo USER Acceptance용 Performance test fixture 생성 commandlet 계약입니다.
// Changelog:
// - v1.0.0: DA_TestSedan 현재 Performance baseline과 동일 결과를 내는 test-only Performance Profile을 deterministic하게 생성하는 Editor-only commandlet 최초 추가.
// Migration:
// - Production Recipe/Profile/VehicleData를 수정하지 않습니다.
// - 생성 Profile은 UA-06 Source edit→Diff→Apply→Undo 검증 전용이며 production balance 기준으로 사용하지 않습니다.
// - Recipe binding은 이 commandlet이 수행하지 않으며 USER Acceptance 중 reviewed Workspace 경로로만 연결합니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFDAUA06FixtureCmdlet.generated.h"

/**
 * UA-06 Explicit Apply / Undo에서 정확히 한 Performance field diff를 만들기 위한 test-only Profile 생성 commandlet입니다.
 */
UCLASS()
class CARFIGHT_REEDITOR_API UCFDAUA06FixtureCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor commandlet 실행 속성을 준비합니다.
	UCFDAUA06FixtureCommandlet();

	// DA_TestSedan 현재 Performance baseline과 결과가 같은 UA-06 Performance Profile을 생성·검증·저장합니다.
	virtual int32 Main(const FString& Params) override;
};
