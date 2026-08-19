// Copyright (c) CarFight. All Rights Reserved.
// File: CFPerformanceProfile.h
// Version: v1.0.0
// Changelog: v1.0.0 DAUTH-P0-08A Editor-only Performance Profile 최초 구현.
// Migration: IsEditorOnly()=true로 Authoring Asset을 cook 대상에서 명시적으로 제외합니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAuthoring/CFVehicleProfileTypes.h"
#include "CFPerformanceProfile.generated.h"

/** Performance Domain의 Editor-only shared Authoring Profile입니다. */
UCLASS(BlueprintType)
class CARFIGHT_REEDITOR_API UCFPerformanceProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	// Cook/save 판단에서 이 Authoring Asset을 명시적으로 Editor-only로 분류합니다.
	virtual bool IsEditorOnly() const override { return true; }

	// Shared Profile의 표시/진단 metadata입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profile", meta=(DisplayName="프로파일 정보"))
	FCFVehicleProfileMeta Meta;

	// Performance Domain의 typed balance payload입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profile", meta=(DisplayName="성능 데이터"))
	FCFPerformanceProfileData Data;
};
