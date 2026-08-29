// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFUIVisualMigrateCmdlet.h
// Version: v1.0.1
// Date: 2026-08-25
// Description: CF-FQ-039 WBP_CFArmorSector layout-preserving visual migration 전용 Editor Commandlet 계약입니다.
// Changelog:
// - v1.0.1: 이미 Image_DirectionIcon이 저장된 Asset도 Bridge의 UE 5.8 GUID metadata repair를 거친 뒤 clean compile/postcondition 검증하고, 실제 repair가 있을 때만 다시 저장하도록 보강.
// - v1.0.0: WBP_CFArmorSector 한 Asset에 Image_DirectionIcon만 additive 추가하고 compile/postcondition/save하는 one-shot Editor-only commandlet을 추가.
// Migration:
// - 구형 CFHUDArtP2Commandlet의 Texture import/HUDVisualData binding 경로를 재사용하지 않습니다.
// - SourceArt, DA_CFHUDVisual_Default, WBP_CFArmorBodyMap과 다른 Production Widget은 수정·저장하지 않습니다.
// - 이미 Image_DirectionIcon이 존재하면 clean compile을 다시 검증하고 GUID metadata repair가 없는 경우 package를 다시 저장하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFUIVisualMigrateCmdlet.generated.h"

/**
 * CF-FQ-039 Armor modular 구조를 기존 WBP_CFArmorSector Designer Tree에 안전하게 적용하는 Editor-only Commandlet입니다.
 */
UCLASS()
class CARFIGHT_REEDITOR_API UCFUIVisualMigrateCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor commandlet 실행 속성을 준비합니다.
	UCFUIVisualMigrateCommandlet();

	// WBP_CFArmorSector에 Direction Icon 슬롯만 additive migration하고 검증된 경우 해당 package 하나만 저장합니다.
	virtual int32 Main(const FString& Params) override;
};
