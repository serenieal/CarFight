// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-19
// Description: CF-FQ-032 VehiclePanel P2 Source Art를 Production HUD Asset으로 가져오는 Editor-only commandlet 계약
// Scope: P2 Texture import, UE 5.8 UI Texture 규격 검증, DA_CFHUDVisual_Default Vehicle 슬롯 연결, SpeedGauge/ArmorBodyMap/VehiclePanel Build·Validate·Save만 수행합니다.
// Changelog:
// - v1.1.0: P2 Texture import에 UI Group + UserInterface2D(RGBA) + NoMipmaps + sRGB 계약을 강제하고 저장 전 fail-closed 검증하도록 강화.
// - v1.0.0: deterministic P2 import/build commandlet 최초 추가.
// Migration:
// - Runtime CarFight_Re와 Gameplay/Presenter 계약은 변경하지 않습니다.
// - Radar/Target/Weapon Visual 슬롯은 수정하지 않습니다.
// - SourceArt/UI/HUD/P2의 동일 이름 PNG를 재실행 시 같은 Texture2D에 교체 import할 수 있습니다.
// - v1.1.0부터 기존 P2 Texture도 commandlet 재실행 시 현재 UI Texture 규격으로 교정됩니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFHUDArtP2Cmdlet.generated.h"

/**
 * VehiclePanel P2 HUD Source Art를 정확한 Production Asset 경로로 import하고 연결하는 Editor-only commandlet입니다.
 */
UCLASS()
class CARFIGHT_REEDITOR_API UCFHUDArtP2Commandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// [v1.0.0] Headless Editor commandlet 기본 실행 속성을 준비합니다.
	UCFHUDArtP2Commandlet();

	// [v1.0.0] P2 Source Art import → DataAsset 연결 → Production Widget 재생성 → 저장 전체 파이프라인을 실행합니다.
	virtual int32 Main(const FString& Params) override;
};
