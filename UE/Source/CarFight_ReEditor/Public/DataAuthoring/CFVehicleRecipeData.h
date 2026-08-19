// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleRecipeData.h
// Version: v1.1.0
// Date: 2026-08-17
// Description: Vehicle Authoring Intent의 persistent Editor-only Recipe DataAsset입니다.
// Scope: Target binding, Asset/Profile/Feel/Mass/Hardpoint/Mount/Default/Override/Import/Applied authoring truth를 보관합니다.
// Changelog:
// - v1.1.0: Recipe 복제 시 원본 Guid를 상속하지 않고 새 RecipeId를 발급하도록 PostDuplicate 계약을 추가.
// - v1.0.0: DAUTH-P0-08A Frozen Recipe schema 최초 구현.
// Migration:
// - Runtime UCFVehicleData에 Recipe reference나 provenance를 추가하지 않습니다.
// - IsEditorOnly()=true로 Recipe package의 Never-Cook 의도를 명시합니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAuthoring/CFVehicleAuthoringTypes.h"
#include "CFVehicleRecipeData.generated.h"

/** P0 Vehicle Authoring Intent의 persistent Editor-only SSOT입니다. */
UCLASS(BlueprintType)
class CARFIGHT_REEDITOR_API UCFVehicleRecipeData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 새 Recipe object에 독립 Authoring identity를 부여합니다.
	UCFVehicleRecipeData()
		: RecipeId(FGuid::NewGuid())
	{
	}

		// Cook/save 판단에서 이 Authoring Asset을 명시적으로 Editor-only로 분류합니다.
	virtual bool IsEditorOnly() const override { return true; }

	// Recipe Asset 복제 시 PIE 복제를 제외하고 새 persistent Recipe identity를 부여합니다.
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

	// Recipe record 자체의 persistent Authoring identity입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Identity", meta=(DisplayName="Recipe ID"))
	FGuid RecipeId;

	// 이 Recipe가 resolve/apply할 Runtime Canonical VehicleData target입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Identity", meta=(DisplayName="대상 VehicleData"))
	TSoftObjectPtr<UCFVehicleData> TargetVehicleData;

	// Profile 추천/표시에서 사용할 semantic vehicle archetype identity입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Identity", meta=(DisplayName="차량 Archetype ID"))
	FName VehicleArchetypeId = NAME_None;

	// Chassis/Wheel asset과 wheel socket binding intent입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Recipe", meta=(DisplayName="차량 자산 Intent"))
	FCFVehicleAssetIntent AssetIntent;

	// 5개 flat Profile binding입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Recipe", meta=(DisplayName="프로파일 Binding"))
	FCFVehicleProfileBindings ProfileBindings;

	// Frozen 4축 Driving Feel semantic intent입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Recipe", meta=(DisplayName="주행감 Intent"))
	FCFVehicleFeelIntent DrivingFeelIntent;

	// Base/Gross mass의 Profile/Explicit intent입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Recipe", meta=(DisplayName="질량 Intent"))
	FCFVehicleMassIntent MassIntent;

	// Vehicle MaxHealth의 Profile/Explicit intent입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Recipe", meta=(DisplayName="내구도 Intent"))
	FCFVehicleDurabilityIntent DurabilityIntent;

	// Stable-ID Hardpoint semantic intent 목록입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Recipe", meta=(DisplayName="하드포인트 Intent"))
	TArray<FCFHardpointIntent> HardpointIntents;

	// Stable-ID Mount semantic intent 목록입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Recipe", meta=(DisplayName="장착 Intent"))
	TArray<FCFMountIntent> MountIntents;

	// Defense/Destroyed FX의 Profile/Explicit/None intent입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Recipe", meta=(DisplayName="기본 데이터 Intent"))
	FCFVehicleDefaultIntent DefaultDataIntent;

	// WheelVisual raw flag를 대신하는 semantic policy입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Recipe", meta=(DisplayName="휠 시각 Intent"))
	FCFWheelVisualIntent WheelVisualIntent;

	// Project Default와 Vehicle-specific DriveState Profile을 구분합니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Recipe", meta=(DisplayName="DriveState 모드"))
	ECFVehicleDriveStateMode DriveStateMode = ECFVehicleDriveStateMode::ProjectDefault;

	// Wheel geometry measurement를 명시적으로 채택한 상태입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Advanced", meta=(DisplayName="자산 측정 채택"))
	FCFVehicleAssetAdoption AssetAdoption;

	// Registry에서 허용된 exceptional leaf override 목록입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Advanced", meta=(DisplayName="고급 필드 Override"))
	TArray<FCFVehicleFieldOverride> AdvancedOverrides;

	// Existing Definition import / Legacy Pin / Adoption 상태입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Migration", meta=(DisplayName="가져오기 상태"))
	FCFVehicleImportState ImportState;

	// 마지막 successful Apply의 deterministic provenance baseline입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Applied", meta=(DisplayName="마지막 적용 상태"))
	FCFVehicleAppliedState AppliedState;

	// 사용자/서비스 Recipe edit의 diagnostic sequence입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Identity", meta=(DisplayName="Authoring Revision"))
	int32 AuthoringRevision = 0;
};
