// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderWagonPolicyCmdlet.h
// Version: v1.0.0
// Date: 2026-09-01
// Description: CF-FQ-040 actual Wagon Recipe의 BuilderTransmissionPolicy를 VehicleSpecificRequired로 one-time 승격하는 bounded Editor commandlet입니다.
// Changelog:
// - v1.0.0: exact Wagon Recipe LegacyCompatible→VehicleSpecificRequired 전환, AuthoringRevision +1, Target/Profile fingerprint invariant, Receipt untouched 계약을 최초 구현.
// Migration:
// - actual Wagon Recipe 한 package만 저장합니다.
// - BuilderCommitReceipt는 변경하지 않아 기존 LegacyCompatible receipt가 새 정책에서 의도적으로 stale 처리되도록 합니다.
// - Drivetrain Profile/Transmission 수치/Target VehicleData는 변경하거나 저장하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFBuilderWagonPolicyCmdlet.generated.h"

/** Actual Wagon의 vehicle-specific Transmission completion policy만 one-time 승격하는 Editor-only commandlet입니다. */
UCLASS()
class CARFIGHT_REEDITOR_API UCFBuilderWagonPolicyCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor commandlet 실행 속성을 준비합니다.
	UCFBuilderWagonPolicyCommandlet();

	// LegacyCompatible actual Wagon Recipe를 VehicleSpecificRequired로 전환하고 non-policy authority가 불변인지 검증합니다.
	virtual int32 Main(const FString& Params) override;
};
