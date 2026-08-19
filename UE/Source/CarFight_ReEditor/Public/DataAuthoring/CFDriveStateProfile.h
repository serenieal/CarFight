// Copyright (c) CarFight. All Rights Reserved.
// File: CFDriveStateProfile.h
// Version: v1.0.0
// Changelog: v1.0.0 DAUTH-P0-08A Editor-only DriveState Profile 최초 구현.
// Migration: Runtime gate bUseDriveStateOverrides는 저장하지 않고 IsEditorOnly()=true를 사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAuthoring/CFVehicleProfileTypes.h"
#include "CFDriveStateProfile.generated.h"

/** DriveState Domain의 Editor-only shared Authoring Profile입니다. */
UCLASS(BlueprintType)
class CARFIGHT_REEDITOR_API UCFDriveStateProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	// Cook/save 판단에서 이 Authoring Asset을 명시적으로 Editor-only로 분류합니다.
	virtual bool IsEditorOnly() const override { return true; }

	// Shared Profile의 표시/진단 metadata입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profile", meta=(DisplayName="프로파일 정보"))
	FCFVehicleProfileMeta Meta;

	// Runtime gate를 제외한 DriveState behavior 14개 field입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Profile", meta=(DisplayName="DriveState 데이터"))
	FCFDriveStateProfileData Data;
};
