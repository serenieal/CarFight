// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleRecipeData.h
// Version: v1.7.0
// Date: 2026-09-02
// Description: Vehicle Authoring Intent + Builder Transmission/Engine Curve provenance receipt의 persistent Editor-only Recipe DataAsset입니다.
// Scope: Target binding, Asset/Profile/Feel/Mass/Hardpoint/Mount/Default/Override/Import/Applied authoring truth를 보관합니다.
// Changelog:
// - v1.7.0: P0-07 UAT에서 USER Driving PASS를 host-local config에만 두면 완료 차량이 재기동 후 되감기는 문제를 막기 위해 Target path/hash 기반 persistent non-semantic BuilderDrivingAcceptanceReceipt를 additive 추가.
// - v1.6.0: CF-FQ-043 Guided Hardpoint 계획 lifecycle을 CompanionMode와 분리하기 위해 BuilderHardpointPlanMode 4-state persistent Builder metadata를 additive 추가. 기존 Recipe default는 LegacyCompatible.
// - v1.5.0: ESH-02 vehicle-specific Engine Curve review disposition, evidence/method binding, deterministic proposal hash receipt metadata를 additive 추가.
// - v1.4.0: Guided 신규 차량의 vehicle-specific Transmission 완료 Gate를 위해 persistent BuilderTransmissionPolicy와 field-level Transmission review receipt를 추가. 기존 Asset 기본값은 LegacyCompatible로 보존.
// - v1.3.0: Editor restart 뒤 Final Review provenance를 정확히 복원할 수 있도록 BuilderCommitReceipt에 canonical ConsumedClaimIds 목록을 non-semantic metadata로 추가.
// - v1.2.0: Builder-private 4 Profile commit과 accepted Evidence/Claim set을 persistent하게 연결하는 non-semantic BuilderCommitReceipt를 추가.
// - v1.1.0: Recipe 복제 시 원본 Guid를 상속하지 않고 새 RecipeId를 발급하도록 PostDuplicate 계약을 추가.
// - v1.0.0: DAUTH-P0-08A Frozen Recipe schema 최초 구현.
// Migration:
// - Runtime UCFVehicleData에 Recipe reference나 provenance를 추가하지 않습니다.
// - BuilderCommitReceipt는 Recipe semantic fingerprint에서 제외되는 provenance metadata이며 Profile/Evidence가 바뀌면 Final Review에서 stale로 판정합니다.
// - BuilderTransmissionPolicy의 serialized default는 LegacyCompatible입니다. 기존 Recipe/Legacy 차량에 vehicle-specific Transmission을 자동 강제하지 않습니다.
// - BuilderHardpointPlanMode의 serialized default도 LegacyCompatible입니다. FQ-043 이전 Recipe는 explicit Hardpoint 계획 Gate를 자동 강제하지 않습니다.
// - BuilderDrivingAcceptanceReceipt는 Recipe semantic fingerprint/Runtime에 포함되지 않는 USER acceptance provenance입니다. same Target DefinitionHash에서는 benchmark RunId가 바뀌어도 유지되고 Target identity/hash가 바뀌면 stale입니다.
// - ESH-02 이전 receipt는 EngineCurveProposalHash가 비어 있고 EngineCurveReview가 BaselineInherited 기본값이므로 기존 차량에 vehicle-specific Engine Curve를 자동 강제하지 않습니다.
// - IsEditorOnly()=true로 Recipe package의 Never-Cook 의도를 명시합니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAuthoring/CFVehicleAuthoringTypes.h"
#include "CFVehicleRecipeData.generated.h"

/** Builder Final Review에서 Legacy compatibility와 Guided 신규 차량의 vehicle-specific Transmission 의무를 구분합니다. */
UENUM(BlueprintType)
enum class ECFBuilderTransmissionPolicy : uint8
{
	LegacyCompatible,
	VehicleSpecificRequired
};

/** Guided Hardpoint 계획의 persistent Builder workflow mode입니다. Resolver semantic fingerprint에는 포함하지 않습니다. */
UENUM(BlueprintType)
enum class ECFBuilderHardpointPlanMode : uint8
{
	LegacyCompatible,
	Unspecified,
	NoHardpoints,
	UseHardpoints
};

/** Transmission component가 어떤 근거로 현재 값을 갖는지 표시합니다. */
UENUM(BlueprintType)
enum class ECFBuilderTransmissionDisposition : uint8
{
	EvidenceDirect,
	EvidenceDerived,
	BaselineInherited,
	GameBias
};

/** 하나의 Transmission semantic field에 대한 reviewed provenance입니다. */
USTRUCT(BlueprintType)
struct FCFBuilderTransmissionComponentReview
{
	GENERATED_BODY()

	// ForwardGearRatios 같은 stable Transmission semantic key입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Transmission")
	FName SemanticKey = NAME_None;

	// FACT/DERIVED/Baseline/GAME_BIAS 중 current 값의 작성 근거입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Transmission")
	ECFBuilderTransmissionDisposition Disposition = ECFBuilderTransmissionDisposition::BaselineInherited;

	// FACT/DERIVED/GAME_BIAS 입력으로 실제 소비한 canonical Evidence Claim ID입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Transmission")
	TArray<FName> EvidenceClaimIds;

	// GAME_BIAS가 actual value Unknown을 보존하면서 사용한 Unknown Fact ID입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Transmission")
	TArray<FName> UnknownFactIds;

	// DERIVED/GAME_BIAS 계산 방법의 stable ID입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Transmission")
	FName MethodId = NAME_None;

	// 계산 방법의 deterministic revision입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Transmission")
	int32 MethodRevision = 0;

	// 계산 방법이 소비한 deterministic parameter key/value입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Transmission")
	TMap<FName, FString> MethodParameters;
};

/** Engine Curve proposal이 어떤 근거로 현재 curve shape를 갖는지 표시합니다. */
UENUM(BlueprintType)
enum class ECFBuilderEngineCurveDisposition : uint8
{
	EvidenceDirect,
	EvidenceDerived,
	BaselineInherited,
	GameBias
};

/** 한 complete vehicle-specific Engine Curve proposal의 reviewed provenance입니다. */
USTRUCT(BlueprintType)
struct FCFBuilderEngineCurveReview
{
	GENERATED_BODY()

	// Engine Curve review schema revision입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Engine Curve")
	int32 SchemaRevision = 1;

	// FACT/DERIVED/Baseline/GAME_BIAS 중 current curve shape의 작성 근거입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Engine Curve")
	ECFBuilderEngineCurveDisposition Disposition = ECFBuilderEngineCurveDisposition::BaselineInherited;

	// Engine Curve 작성에 실제 소비한 canonical Evidence Claim ID입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Engine Curve")
	TArray<FName> EvidenceClaimIds;

	// GAME_BIAS가 실제 curve 미공개 상태를 보존하면서 사용한 Unknown Fact ID입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Engine Curve")
	TArray<FName> UnknownFactIds;

	// DERIVED/GAME_BIAS curve 생성 방법의 stable ID입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Engine Curve")
	FName MethodId = NAME_None;

	// Curve 생성 방법의 deterministic revision입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Engine Curve")
	int32 MethodRevision = 0;

	// Curve 생성 방법이 소비한 deterministic parameter key/value입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Engine Curve")
	TMap<FName, FString> MethodParameters;

	// BaselineInherited가 아닌 실제 reviewed Engine Curve proposal인지 반환합니다.
	bool IsVehicleSpecificReview() const
	{
		return Disposition != ECFBuilderEngineCurveDisposition::BaselineInherited;
	}
};

/** 한 complete Drivetrain Transmission proposal의 field-level reviewed provenance입니다. */
USTRUCT(BlueprintType)
struct FCFBuilderTransmissionReview
{
	GENERATED_BODY()

	// Transmission review schema revision입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Transmission")
	int32 SchemaRevision = 1;

	// Evidence가 알려준 forward gear count입니다. 0이면 Unknown이며 proposed gear count 자체가 GAME_BIAS일 수 있습니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Transmission")
	int32 ExpectedForwardGearCount = 0;

	// Transmission semantic별 reviewed provenance 목록입니다.
	UPROPERTY(EditAnywhere, Category="CarFight|Data Authoring|Builder Transmission")
	TArray<FCFBuilderTransmissionComponentReview> Components;

	// 실제 reviewed Transmission metadata가 존재하는지 반환합니다.
	bool IsPresent() const
	{
		return !Components.IsEmpty();
	}
};

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

	// Accepted Profile proposal 당시 Recipe의 exact Builder Transmission policy입니다. 정책 raw drift를 Final Review에서 탐지합니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	ECFBuilderTransmissionPolicy TransmissionPolicy = ECFBuilderTransmissionPolicy::LegacyCompatible;

	// Accepted vehicle-specific Transmission review + Drivetrain payload의 deterministic hash입니다. LegacyCompatible에서는 비어 있을 수 있습니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FString TransmissionProposalHash;

	// Editor restart 뒤에도 field-level Transmission provenance를 fresh 재검증할 persistent review metadata입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FCFBuilderTransmissionReview TransmissionReview;

	// Accepted vehicle-specific Engine Curve review + Performance payload의 deterministic hash입니다. BaselineInherited/legacy에서는 비어 있을 수 있습니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FString EngineCurveProposalHash;

	// Editor restart 뒤에도 Engine Curve provenance를 fresh 재검증할 persistent review metadata입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	FCFBuilderEngineCurveReview EngineCurveReview;

	// Accepted proposal이 사용한 Resolver contract revision입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Receipt")
	int32 ResolverContractRevision = 0;

	// Receipt가 실제 accepted proposal을 나타내는 최소 identity를 갖는지 반환합니다.
	bool IsValid() const
	{
		return !ProposalHash.IsEmpty() && EvidenceId.IsValid() && !EvidenceFingerprint.IsEmpty();
	}
};

/** USER가 실제 주행 후 PASS한 exact Vehicle Definition을 persistent하게 증명하는 non-semantic receipt입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleBuilderDrivingAcceptanceReceipt
{
	GENERATED_BODY()

	// USER가 PASS한 exact Target VehicleData path입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Driving Receipt")
	FSoftObjectPath TargetVehicleDataPath;

	// USER가 실제 주행해 PASS한 exact Target DefinitionHash입니다. Step 8 acceptance validity의 authority입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Driving Receipt")
	FString TargetDefinitionHash;

	// PASS 당시 current technical benchmark RunId입니다. 진단/추적용이며 same DefinitionHash에서 새 RunId가 생겨도 USER PASS 자체는 무효화하지 않습니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder Driving Receipt")
	FString AcceptedBenchmarkRunId;

	// 최소 persistent identity가 존재하는지 반환합니다.
	bool IsValid() const
	{
		return TargetVehicleDataPath.IsValid() && !TargetDefinitionHash.IsEmpty();
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

	// Guided Builder가 이 Recipe의 vehicle-specific Transmission 완료를 Final Review에서 강제할지 결정합니다. 기존 Recipe 기본값은 LegacyCompatible입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder", meta=(DisplayName="Builder 변속기 정책", ToolTip="LegacyCompatible은 기존 차량 호환 동작을 보존합니다. VehicleSpecificRequired는 Guided Builder 신규 차량이 Final Review 전에 차량별 Transmission Proposal을 완료하도록 강제합니다."))
	ECFBuilderTransmissionPolicy BuilderTransmissionPolicy = ECFBuilderTransmissionPolicy::LegacyCompatible;

	// Guided Builder의 Hardpoint 계획 의도를 persistent하게 보존합니다. 기존 Recipe 기본값은 LegacyCompatible이며 Resolver semantic fingerprint에는 포함되지 않습니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder", meta=(DisplayName="Builder 하드포인트 계획", ToolTip="LegacyCompatible은 기존 차량의 선택적 Hardpoint/Mount 동작을 보존합니다. Unspecified는 신규 Guided 차량이 아직 장착점 사용 여부를 결정하지 않은 상태입니다. NoHardpoints는 장착점 0개를 명시한 상태이고 UseHardpoints는 장착 위치를 작성하는 상태입니다."))
	ECFBuilderHardpointPlanMode BuilderHardpointPlanMode = ECFBuilderHardpointPlanMode::LegacyCompatible;

	// Builder-private 4 Profile에 실제 commit된 Evidence/Claim provenance receipt입니다. Recipe semantic fingerprint에는 포함되지 않습니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder", meta=(DisplayName="Builder Commit Receipt"))
	FCFVehicleBuilderCommitReceipt BuilderCommitReceipt;

	// USER가 실제 주행 후 PASS한 exact Target Definition을 보존하는 non-semantic durable receipt입니다. Runtime/Resolver fingerprint에는 포함되지 않습니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Builder", meta=(DisplayName="Builder Driving Acceptance Receipt"))
	FCFVehicleBuilderDrivingAcceptanceReceipt BuilderDrivingAcceptanceReceipt;

	// 사용자/서비스 Recipe edit의 diagnostic sequence입니다.
	UPROPERTY(VisibleAnywhere, Category="CarFight|Data Authoring|Identity", meta=(DisplayName="Authoring Revision"))
	int32 AuthoringRevision = 0;
};
