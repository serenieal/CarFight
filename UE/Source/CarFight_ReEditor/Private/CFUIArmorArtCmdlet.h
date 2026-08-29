// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFUIArmorArtCmdlet.h
// Version: v1.1.0
// Date: 2026-08-25
// Description: CF-FQ-039 modular Armor Production Texture를 slice별로 안전하게 import/binding하는 Editor Commandlet 계약입니다.
// Changelog:
// - v1.1.0: 기존 2-icon one-shot 경로를 보존하면서 -PlateOnly 모드로 common Armor Plate 1종만 신규 Import/Bind하는 후속 slice를 추가.
// - v1.0.0: Source-bound Arrow/Chevron2 두 PNG만 신규 Texture2D로 Import하고 HUDVisualData의 두 modular icon field에 연결하는 one-shot commandlet을 추가.
// Migration:
// - 기존 CFHUDArtP2Commandlet, legacy 6-direction Plate, WBP_CFArmorSector 배치/회전은 수정하지 않습니다.
// - 각 slice의 신규 목적 Texture가 이미 존재하면 overwrite하지 않고 fail-closed합니다.
// - -PlateOnly는 기존 Arrow/Chevron2 Texture와 DataAsset binding을 검증만 하고 수정하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFUIArmorArtCmdlet.generated.h"

/**
 * CF-FQ-039의 Source-bound modular Armor Texture를 승인된 slice 단위로 자산화하는 Editor-only Commandlet입니다.
 */
UCLASS()
class CARFIGHT_REEDITOR_API UCFUIArmorArtCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor commandlet 실행 속성을 준비합니다.
	UCFUIArmorArtCommandlet();

	// 기본 2-Icon one-shot 또는 -PlateOnly common Plate one-shot을 실행합니다.
	virtual int32 Main(const FString& Params) override;
};
