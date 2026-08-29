// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehSilSrcCmdlet.h
// Version: v1.0.0
// Date: 2026-08-25
// Description: CF-FQ-039 Vehicle-specific silhouette의 ChassisMesh-derived Review Source Candidate를 생성하는 Editor-only Commandlet 계약입니다.
// Changelog:
// - v1.0.0: Sedan/SUV exact ChassisMesh LOD0을 top-down left-facing RGBA silhouette PNG로 투영하는 bounded source generator를 추가.
// Migration:
// - Production Texture import, DA_CFHUDVisual_Default.VehicleSilhouettes binding, UMG/Asset save는 수행하지 않습니다.
// - 출력은 SourceArt/UI/HUD/VT의 VT05 Review Source Candidate 2종으로 제한합니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFVehSilSrcCmdlet.generated.h"

/**
 * CF-FQ-039의 exact ChassisMesh에서 Vehicle-specific silhouette Review Source Candidate를 생성하는 Editor-only Commandlet입니다.
 */
UCLASS()
class CARFIGHT_REEDITOR_API UCFVehSilSrcCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor commandlet 실행 속성을 준비합니다.
	UCFVehSilSrcCommandlet();

	// Sedan/SUV exact ChassisMesh를 읽어 SourceArt PNG 2종만 생성합니다.
	virtual int32 Main(const FString& Params) override;
};
