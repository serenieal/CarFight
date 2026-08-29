// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehSilProdCmdlet.h
// Version: v1.0.0
// Date: 2026-08-25
// Description: CF-FQ-039 승인된 Vehicle-specific silhouette Source를 Production Texture와 VehicleData catalog로 자산화하는 Editor-only Commandlet 계약입니다.
// Changelog:
// - v1.0.0: VT05 Sedan/SUV PNG 2종을 Production Texture로 Import하고 DA_CFHUDVisual_Default.VehicleSilhouettes 3개 exact mapping을 저장하는 idempotent one-shot 경로를 추가.
// Migration:
// - 기존 VehicleSilhouette fallback, ArmorCommonPlate/Direction Icon, WBP/Designer Layout은 수정하지 않습니다.
// - 기존 Production Texture/catalog가 예상 계약과 정확히 일치하면 validation-only PASS하며, 다른 기존 값은 overwrite하지 않고 fail-closed합니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFVehSilProdCmdlet.generated.h"

/**
 * CF-FQ-039 승인 Vehicle-specific silhouette Source를 Production Texture와 VehicleData identity catalog로 연결하는 Editor-only Commandlet입니다.
 */
UCLASS()
class CARFIGHT_REEDITOR_API UCFVehSilProdCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor commandlet 실행 속성을 준비합니다.
	UCFVehSilProdCommandlet();

	// Sedan/SUV Source 2종을 Production Texture로 보장하고 VehicleData 3종 catalog를 exact mapping으로 저장합니다.
	virtual int32 Main(const FString& Params) override;
};
