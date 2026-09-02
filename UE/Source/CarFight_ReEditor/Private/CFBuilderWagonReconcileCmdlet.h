// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderWagonReconcileCmdlet.h
// Version: v1.0.0
// Date: 2026-09-01
// Description: CF-FQ-040 actual Wagon의 WSA USER-approved SocketScale truth와 Vehicle Builder AssetAdoption을 post-closure 정합화하는 bounded one-shot commandlet입니다.
// Changelog:
// - v1.0.0: Wagon chassis stale 0.63 Socket Scale을 USER-approved 0.80으로 제한 복구하고, existing R2 Measurement Preview/OwnershipWrite Commit으로 Radius/Width 4건 adoption을 fresh 재채택하는 exact reconciliation을 추가.
// Migration:
// - Wheel Size Authority P0 기능을 다시 열거나 일반 migration을 수행하지 않습니다.
// - exact Wagon Chassis + Recipe 두 package만 저장하며 DA_Vehicle_Wagon Target은 read-only verification만 수행합니다.
// - current Socket Scale은 0.63 또는 이미 0.80인 경우만 허용하고 다른 값은 USER intent drift로 fail-closed합니다.
// - Target의 USER PASS 상태(0.80 WheelAnchor / 약 40cm Radius / 약 25cm Width)가 일치하지 않으면 아무 것도 저장하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFBuilderWagonReconcileCmdlet.generated.h"

/** Actual Wagon post-WSA source/adoption drift만 exact current truth에 맞추는 Editor-only one-shot commandlet입니다. */
UCLASS()
class CARFIGHT_REEDITOR_API UCFBuilderWagonReconcileCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor commandlet 실행 속성을 준비합니다.
	UCFBuilderWagonReconcileCommandlet();

	// USER-approved 0.80 Socket truth를 Chassis와 Recipe adoption에 재정합화하고 Target mutation 없이 Resolver Success를 검증합니다.
	virtual int32 Main(const FString& Params) override;
};
