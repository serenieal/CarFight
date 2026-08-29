// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleRecipeData.h
// Version: v1.3.0
// Date: 2026-08-27
// Description: Vehicle Authoring Intent + Builder profile provenance receipt의 persistent Editor-only Recipe DataAsset입니다.
// Scope: Target binding, Asset/Profile/Feel/Mass/Hardpoint/Mount/Default/Override/Import/Applied authoring truth를 보관합니다.
// Changelog:
// - v1.3.0: Editor restart 뒤 Final Review provenance를 정확히 복원할 수 있도록 BuilderCommitReceipt에 canonical ConsumedClaimIds 목록을 non-semantic metadata로 추가.
// - v1.2.0: Builder-private 4 Profile commit과 accepted Evidence/Claim set을 persistent하게 연결하는 non-semantic BuilderCommitReceipt를 추가.
// - v1.1.0: Recipe 복제 시 원본 Guid를 상속하지 않고 새 RecipeId를 발급하도록 PostDuplicate 계약을 추가.
// - v1.0.0: DAUTH-P0-08A Frozen Recipe schema 최초 구현.
// Migration:
// - Runtime UCFVehicleData에 Recipe reference나 provenance를 추가하지 않습니다.
// - BuilderCommitReceipt는 Recipe semantic fingerprint에서 제외되는 provenance metadata이며 Profile/Evidence가 바뀌면 Final Review에서 stale로 판정합니다.
// - IsEditorOnly()=true로 Recipe package의 Never-Cook 의도를 명시합니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAuthoring/CFVehicleAuthoringTypes.h"
#include "CFVehicleRecipeData.generated.h"

/** Builder-private 4 Profile에 실제 commit된 accepted Evidence proposal을 증명하는 persistent non-semantic receipt입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleBuilderCommitReceipt
{
	GENERATED_BODY()

	// Accepted Builder Profile commit의 exact proposal hash입니다. 비어 있으면 receipt가 없습니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FString ProposalHash;

	// Accepted proposal이 사용한 exact Reference Evidence path입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FSoftObjectPath EvidencePath;

	// Accepted proposal이 사용한 persistent Evidence identity입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FGuid EvidenceId;

	// Accepted proposal이 사용한 semantic Evidence fingerprint입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FString EvidenceFingerprint;

	// Accepted proposal이 실제 소비한 canonical Claim ID를 deterministic lexical order로 보존한 resume provenance 목록입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	TArray<FName> ConsumedClaimIds;

	// Accepted proposal이 실제 소비한 canonical Claim ID set의 order-independent hash입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FString ConsumedClaimIdsHash;

	// Commit 직후 VehicleBase Profile payload fingerprint입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FString VehicleBaseFingerprint;

	// Commit 직후 Drivetrain Profile payload fingerprint입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FString DrivetrainFingerprint;

	// Commit 직후 Handling Profile payload fingerprint입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FString HandlingFingerprint;

	// Commit 직후 Performance Profile payload fingerprint입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FString PerformanceFingerprint;

	// Accepted complete Profile payload의 prospective resolved Definition hash입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FString ProspectiveResolvedDefinitionHash;

	// Accepted proposal이 사용한 Resolver contract revision입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	int32 ResolverContractRevision = 0;

	// Receipt가 실제 accepted proposal을 나타내는 최소 identity를 갖는지 반환합니다.
	bool IsValid() const
	{
		return !ProposalHash.IsEmpty() && EvidenceId.IsValid() && !EvidenceFingerprint.IsEmpty();
	}
};

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

	// Builder-private 4 Profile에 실제 commit된 Evidence/Claim provenance receipt입니다. Recipe semantic fingerprint에는 포함되지 않습니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder", meta=(DisplayName="Builder Commit Receipt"))
	FCFVehicleBuilderCommitReceipt BuilderCommitReceipt;

	// 사용자/서비스 Recipe edit의 diagnostic sequence입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Identity", meta=(DisplayName="Authoring Revision"))
	int32 AuthoringRevision = 0;
};
