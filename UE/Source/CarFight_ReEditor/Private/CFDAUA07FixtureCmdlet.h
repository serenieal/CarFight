// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFDAUA07FixtureCmdlet.h
// Version: v1.0.0
// Date: 2026-08-20
// Description: DAUTH-P0-12 UA-07/08 Driving Feel USER Acceptance용 test-only Profile fixture 생성 commandlet 계약입니다.
// Changelog:
// - v1.0.0: /Game/Test/CarFightDataAuthoring에 Handling/Performance Profile 2개를 deterministic하게 준비하는 Editor-only commandlet 최초 추가.
// Migration:
// - Production Recipe/Profile/VehicleData를 수정하지 않습니다.
// - 생성 Profile은 USER Acceptance 전용이며 production balance 기준으로 사용하지 않습니다.
// - Feel response는 기존 Resolver Automation fixture를 재사용하고 direct baseline은 실행 시점 DA_TestSedan에서 복사합니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFDAUA07FixtureCmdlet.generated.h"

/**
 * UA-07/08에서 4축 Driving Feel 인과관계를 검증할 test-only Profile 2개를 생성하는 Editor-only commandlet입니다.
 */
UCLASS()
class CARFIGHT_REEDITOR_API UCFDAUA07FixtureCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor commandlet 실행 속성을 준비합니다.
	UCFDAUA07FixtureCommandlet();

	// DA_TestSedan baseline을 읽고 UA-07 Handling/Performance Profile을 생성·검증·저장합니다.
	virtual int32 Main(const FString& Params) override;
};
