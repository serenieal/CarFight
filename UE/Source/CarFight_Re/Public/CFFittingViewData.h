// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-15
// Description: CF-FQ-034 FIT-P0-06 차량 피팅 읽기 전용 ViewData·Debug 계약
// Scope: 검증 완료 FCFVehicleFittingSnapshot을 Blueprint/UI가 안전하게 표시할 Mount·Defense·Ammo·Mass·Mobility·Issue ViewData로 투영합니다.
// Changelog:
// - v1.0.0: Fitting ViewData row, PhysicalMassOnly Mobility Preview와 순수 Builder 계약을 최초 추가.
// Migration:
// - Snapshot의 호환성, 질량 Breakdown, Validation 결과를 다시 계산하지 않고 그대로 투영합니다.
// - VehicleData에 정식 플레이어 표시 이름 필드가 없으므로 P0 VehicleDisplayName은 Asset 이름 fallback이며 bVehicleDisplayNameUsesAssetName으로 명시합니다.
// - 추가 Mobility Adapter는 현재 비활성입니다. 축 Scale 1.0은 추가 보정 없음이며 실제 Chaos 질량 효과는 별도로 계속 적용됩니다.
// - Blueprint/UMG는 이 ViewData를 표시만 하고 Mount 호환, Ammo 질량, 총중량 공식을 그래프에 복제하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFFittingTypes.h"
#include "CFFittingViewData.generated.h"

class UCFAmmoData;
class UCFEquipmentPresetData;
class UCFTurretMountData;
class UCFVehicleData;
class UCFVehicleDefenseData;
class UCFWeaponData;

/** Fitting ViewData가 표시하는 질량 Breakdown 행 종류입니다. */
UENUM(BlueprintType)
enum class ECFFittingMassRowType : uint8
{
	BaseVehicle UMETA(DisplayName="기준 차량 질량 (Base Vehicle)"),
	Equipment UMETA(DisplayName="장비 질량 (Equipment)"),
	Ammo UMETA(DisplayName="탄약 질량 (Ammo)"),
	Defense UMETA(DisplayName="방어 질량 (Defense)"),
	Payload UMETA(DisplayName="탑재 질량 (Payload)"),
	TotalVehicle UMETA(DisplayName="차량 총중량 (Total Vehicle)"),
	MaximumGross UMETA(DisplayName="최대 허용 총중량 (Maximum Gross)")
};

/** Fitting Mobility Preview가 어떤 효과를 표시하는지 구분합니다. */
UENUM(BlueprintType)
enum class ECFFittingMobilityPreviewMode : uint8
{
	Unavailable UMETA(DisplayName="미사용 가능 (Unavailable)"),
	PhysicalMassOnly UMETA(DisplayName="실제 물리 질량만 사용 (Physical Mass Only)"),
	AdditionalAdapterApplied UMETA(DisplayName="추가 Mobility Adapter 적용 (Additional Adapter Applied)")
};

/** 한 MountProfile의 최종 선택을 UI가 표시할 읽기 전용 행입니다. */
USTRUCT(BlueprintType)
struct FCFFittingMountViewRow
{
	GENERATED_BODY()

	// [v1.0.0] 최종 해석된 장착 프로파일 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mount", meta=(DisplayName="장착 프로파일 ID (MountProfileId)", ToolTip="Snapshot.ResolvedMounts에서 그대로 투영한 최종 MountProfileId입니다."))
	FName MountProfileId = NAME_None;

	// [v1.0.0] 최종 장비가 배치될 하드포인트 위치 슬롯 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mount", meta=(DisplayName="위치 슬롯 ID (LocationSlotId)", ToolTip="Snapshot.ResolvedMounts에서 그대로 투영한 최종 하드포인트 위치 슬롯 ID입니다."))
	FName LocationSlotId = NAME_None;

	// [v1.0.0] 최종 해석된 장착 타입입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mount", meta=(DisplayName="장착 타입 (MountType)", ToolTip="Snapshot이 이미 검증한 최종 MountType입니다. UI가 호환성을 다시 계산하지 않습니다."))
	ECFVehicleMountType MountType = ECFVehicleMountType::None;

	// [v1.0.0] 최종 해석에 사용된 무기 크기 제한입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mount", meta=(DisplayName="무기 크기 제한 (SizeLimit)", ToolTip="Snapshot이 이미 검증한 최종 SizeLimit입니다."))
	ECFVehicleWeaponSize SizeLimit = ECFVehicleWeaponSize::None;

	// [v1.0.0] 최종 선택된 장비 프리셋 DataAsset입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mount", meta=(DisplayName="장비 프리셋 데이터 (EquipmentPresetData)", ToolTip="차량 기본값 또는 피팅 Override에서 최종 해석된 EquipmentPresetData입니다."))
	TObjectPtr<UCFEquipmentPresetData> EquipmentPresetData = nullptr;

	// [v1.0.0] 장비 프리셋이 제공하는 사람 읽기용 표시 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mount", meta=(DisplayName="장비 표시 이름 (EquipmentDisplayName)", ToolTip="EquipmentPresetData.DisplayName을 그대로 표시합니다. 빈 장착이면 비어 있습니다."))
	FText EquipmentDisplayName;

	// [v1.0.0] 최종 해석된 터렛 마운트 DataAsset입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mount", meta=(DisplayName="터렛 마운트 데이터 (TurretMountData)", ToolTip="Snapshot.ResolvedMounts의 TurretMountData 참조입니다."))
	TObjectPtr<UCFTurretMountData> TurretMountData = nullptr;

	// [v1.0.0] 최종 해석된 무기 DataAsset입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mount", meta=(DisplayName="무기 데이터 (WeaponData)", ToolTip="Snapshot.ResolvedMounts의 WeaponData 참조입니다."))
	TObjectPtr<UCFWeaponData> WeaponData = nullptr;

	// [v1.0.0] 최종 선택이 차량 기본값·피팅 Override·빈 장착 중 어디에서 왔는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mount", meta=(DisplayName="선택 소스 (SelectionSource)", ToolTip="Snapshot이 보존한 최종 장비 선택 소스입니다."))
	ECFFittingSelectionSource SelectionSource = ECFFittingSelectionSource::None;

	// [v1.0.0] 최종 터렛 마운트의 Snapshot 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mount", meta=(Units="kg", DisplayName="터렛 마운트 질량 kg (TurretMountMassKg)", ToolTip="Snapshot이 이미 계산한 TurretMountMassKg를 그대로 투영합니다."))
	float TurretMountMassKg = 0.0f;

	// [v1.0.0] 최종 무기의 Snapshot 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mount", meta=(Units="kg", DisplayName="무기 질량 kg (WeaponMassKg)", ToolTip="Snapshot이 이미 계산한 WeaponMassKg를 그대로 투영합니다."))
	float WeaponMassKg = 0.0f;
};

/** 최종 차량 방어 선택을 UI가 표시할 읽기 전용 행입니다. */
USTRUCT(BlueprintType)
struct FCFFittingDefenseViewRow
{
	GENERATED_BODY()

	// [v1.0.0] 차량 기본 방어·명시적 None·Override 중 Snapshot이 최종 사용한 선택 방식입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Defense", meta=(DisplayName="방어 선택 방식 (SelectionMode)", ToolTip="Snapshot.ResolvedDefenseSelectionMode를 그대로 투영합니다."))
	ECFDefenseSelectionMode SelectionMode = ECFDefenseSelectionMode::UseVehicleDefault;

	// [v1.0.0] 최종 해석된 VehicleDefenseData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Defense", meta=(DisplayName="방어 데이터 (DefenseData)", ToolTip="Snapshot.ResolvedDefenseData를 그대로 투영합니다. ExplicitNone이면 비어 있습니다."))
	TObjectPtr<UCFVehicleDefenseData> DefenseData = nullptr;

	// [v1.0.0] 최종 DefenseData의 안정 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Defense", meta=(DisplayName="방어 ID (DefenseId)", ToolTip="Resolved DefenseData가 있으면 DefenseId를 표시하고 없으면 None입니다."))
	FName DefenseId = NAME_None;

	// [v1.0.0] 최종 방어 패키지가 Snapshot 총중량에 기여한 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Defense", meta=(Units="kg", DisplayName="방어 질량 kg (DefenseMassKg)", ToolTip="Snapshot.DefenseMassKg를 그대로 투영합니다."))
	float DefenseMassKg = 0.0f;
};

/** 한 출격 탄종 선택을 UI가 표시할 읽기 전용 행입니다. */
USTRUCT(BlueprintType)
struct FCFFittingAmmoViewRow
{
	GENERATED_BODY()

	// [v1.0.0] 이번 출격에 실제 적재되는 AmmoData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Ammo", meta=(DisplayName="탄약 데이터 (AmmoData)", ToolTip="Snapshot.InitialSortieAmmoLoads의 AmmoData를 그대로 투영합니다."))
	TObjectPtr<UCFAmmoData> AmmoData = nullptr;

	// [v1.0.0] AmmoData의 안정 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Ammo", meta=(DisplayName="탄약 ID (AmmoId)", ToolTip="AmmoData.AmmoId를 표시합니다. 최대 적재량이나 Runtime 현재 수량으로 대체하지 않습니다."))
	FName AmmoId = NAME_None;

	// [v1.0.0] AmmoData가 제공하는 플레이어용 표시 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Ammo", meta=(DisplayName="탄약 표시 이름 (AmmoDisplayName)", ToolTip="AmmoData.AmmoDisplayName을 그대로 표시합니다."))
	FText AmmoDisplayName;

	// [v1.0.0] 이번 출격 시작 시 실제 장전+예비 전체 탄약 수량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Ammo", meta=(DisplayName="출격 시작 탄약량 (InitialSortieAmmoCount)", ToolTip="Snapshot.InitialSortieAmmoLoads의 실제 출격 수량입니다. MaximumLoadableAmmoCount가 아닙니다."))
	int32 InitialSortieAmmoCount = 0;

	// [v1.0.0] AmmoData가 제공하는 유효 한 단위 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Ammo", meta=(Units="kg", DisplayName="탄약 단위 질량 kg (UnitMassKg)", ToolTip="표시 참고용 AmmoData 유효 단위 질량입니다. 전체 AmmoMassKg는 Snapshot 값을 사용합니다."))
	float UnitMassKg = 0.0f;
};

/** Snapshot 질량 Breakdown의 한 값을 UI가 표시할 읽기 전용 행입니다. */
USTRUCT(BlueprintType)
struct FCFFittingMassViewRow
{
	GENERATED_BODY()

	// [v1.0.0] 이 질량 행이 기준 차량·장비·탄약·방어·탑재·총중량·최대 총중량 중 무엇인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mass", meta=(DisplayName="질량 행 종류 (MassRowType)", ToolTip="Snapshot에서 그대로 투영한 질량 Breakdown 값의 종류입니다."))
	ECFFittingMassRowType MassRowType = ECFFittingMassRowType::BaseVehicle;

	// [v1.0.0] Snapshot이 이미 계산한 해당 질량 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mass", meta=(Units="kg", DisplayName="질량 kg (MassKg)", ToolTip="ViewData Builder가 Snapshot 값을 재계산하지 않고 그대로 복사한 질량입니다."))
	float MassKg = 0.0f;
};

/** 실제 질량 효과와 선택적 추가 Mobility Adapter 상태를 UI가 표시할 Preview입니다. */
USTRUCT(BlueprintType)
struct FCFFittingMobilityViewData
{
	GENERATED_BODY()

	// [v1.0.0] Mobility Preview가 실제 물리 질량만 사용하는지 추가 Adapter까지 적용하는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mobility", meta=(DisplayName="Mobility Preview 모드 (PreviewMode)", ToolTip="PhysicalMassOnly이면 실제 Chaos 질량이 1차 효과이며 추가 가속·제동·조향 Scalar는 적용하지 않습니다."))
	ECFFittingMobilityPreviewMode PreviewMode = ECFFittingMobilityPreviewMode::Unavailable;

	// [v1.0.0] TotalVehicleMassKg를 BaseVehicleMassKg로 나눈 실제 질량 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mobility", meta=(DisplayName="질량 비율 (MassRatio)", ToolTip="실제 물리 질량 Preview 입력인 TotalVehicleMassKg / BaseVehicleMassKg입니다. Base 질량이 유효하지 않으면 PreviewMode가 Unavailable입니다."))
	float MassRatio = 0.0f;

	// [v1.0.0] 현재 추가 가속 Mobility Adapter 배율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mobility", meta=(DisplayName="추가 가속 배율 (AccelerationScale)", ToolTip="P0에서는 추가 Mobility Adapter가 비활성이므로 1.0입니다. 실제 Chaos 질량에 의한 가속 변화와 별개입니다."))
	float AccelerationScale = 1.0f;

	// [v1.0.0] 현재 추가 제동 Mobility Adapter 배율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mobility", meta=(DisplayName="추가 제동 배율 (BrakingScale)", ToolTip="P0에서는 추가 Mobility Adapter가 비활성이므로 1.0입니다. 실제 Chaos 질량에 의한 제동 변화와 별개입니다."))
	float BrakingScale = 1.0f;

	// [v1.0.0] 현재 추가 조향 반응 Mobility Adapter 배율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mobility", meta=(DisplayName="추가 조향 반응 배율 (SteeringResponseScale)", ToolTip="P0에서는 추가 Mobility Adapter가 비활성이므로 1.0입니다. 실제 Chaos 질량에 의한 주행 변화와 별개입니다."))
	float SteeringResponseScale = 1.0f;

	// [v1.0.0] 별도 데이터 기반 Mobility Adapter가 실제 적용 중인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mobility", meta=(DisplayName="추가 Mobility Adapter 적용됨 (bAdditionalAdapterApplied)", ToolTip="현재 P0에서는 False입니다. True가 되기 전까지 축 Scale 1.0은 추가 보정 없음이라는 뜻입니다."))
	bool bAdditionalAdapterApplied = false;
};

/** Fitting Snapshot 전체를 Blueprint/UI가 안전하게 표시할 읽기 전용 ViewData입니다. */
USTRUCT(BlueprintType)
struct FCFVehicleFittingViewData
{
	GENERATED_BODY()

	// [v1.0.0] 이 ViewData의 기준 VehicleData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData", meta=(DisplayName="차량 데이터 (VehicleData)", ToolTip="Snapshot.VehicleData를 그대로 투영합니다."))
	TObjectPtr<UCFVehicleData> VehicleData = nullptr;

	// [v1.0.0] P0 Debug에서 표시할 차량 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData", meta=(DisplayName="차량 표시 이름 (VehicleDisplayName)", ToolTip="VehicleData에 정식 플레이어 표시 이름이 아직 없어 P0에서는 Asset 이름을 사용합니다. bVehicleDisplayNameUsesAssetName으로 fallback 여부를 확인할 수 있습니다."))
	FText VehicleDisplayName;

	// [v1.0.0] VehicleDisplayName이 정식 사용자 이름이 아니라 VehicleData Asset 이름 fallback인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData", meta=(DisplayName="차량 이름 Asset Fallback 사용 (bVehicleDisplayNameUsesAssetName)", ToolTip="True이면 VehicleDisplayName은 현재 VehicleData Asset 이름을 P0 Debug용으로 표시한 값입니다."))
	bool bVehicleDisplayNameUsesAssetName = false;

	// [v1.0.0] ViewData가 표시하는 피팅 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData", meta=(DisplayName="피팅 ID (FittingId)", ToolTip="Snapshot.FittingId를 그대로 투영합니다."))
	FName FittingId = NAME_None;

	// [v1.0.0] 현재 Snapshot의 검증 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData", meta=(DisplayName="검증 상태 (ValidationState)", ToolTip="Snapshot.ValidationState를 그대로 투영합니다."))
	ECFFittingValidationState ValidationState = ECFFittingValidationState::NotEvaluated;

	// [v1.0.0] 현재 Snapshot을 런타임에 적용할 수 있는지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData", meta=(DisplayName="피팅 적용 가능 (bCanApply)", ToolTip="Snapshot.IsValid() 결과입니다. UI는 자체 Validation 공식을 만들지 않습니다."))
	bool bCanApply = false;

	// [v1.0.0] Snapshot 순서를 보존한 최종 MountProfile 표시 행입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData", meta=(DisplayName="장착 표시 행 (MountRows)", ToolTip="Snapshot.ResolvedMounts 순서를 그대로 보존한 최종 장착 표시 목록입니다."))
	TArray<FCFFittingMountViewRow> MountRows;

	// [v1.0.0] 최종 Defense 선택 표시 행입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData", meta=(DisplayName="방어 표시 행 (DefenseRow)", ToolTip="Snapshot의 최종 방어 선택과 DefenseMassKg를 표시합니다."))
	FCFFittingDefenseViewRow DefenseRow;

	// [v1.0.0] Snapshot 순서를 보존한 실제 출격 Ammo 선택 표시 행입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData", meta=(DisplayName="탄약 표시 행 (AmmoRows)", ToolTip="Snapshot.InitialSortieAmmoLoads의 실제 출격 탄약 선택을 순서 그대로 표시합니다."))
	TArray<FCFFittingAmmoViewRow> AmmoRows;

	// [v1.0.0] Snapshot 질량 Breakdown을 고정 순서로 표시할 행 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData", meta=(DisplayName="질량 표시 행 (MassRows)", ToolTip="Base, Equipment, Ammo, Defense, Payload, Total, MaximumGross 순서로 Snapshot 질량 값을 그대로 표시합니다."))
	TArray<FCFFittingMassViewRow> MassRows;

	// [v1.0.0] 최종 차량 총중량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mass", meta=(Units="kg", DisplayName="차량 총중량 kg (TotalVehicleMassKg)", ToolTip="Snapshot.TotalVehicleMassKg를 그대로 투영합니다."))
	float TotalVehicleMassKg = 0.0f;

	// [v1.0.0] 차량 플랫폼 최대 허용 총중량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mass", meta=(Units="kg", DisplayName="최대 허용 총중량 kg (MaximumGrossMassKg)", ToolTip="Snapshot.MaximumGrossMassKg를 그대로 투영합니다."))
	float MaximumGrossMassKg = 0.0f;

	// [v1.0.0] 최대 탑재 질량 대비 현재 Payload 사용률입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mass", meta=(DisplayName="탑재 질량 사용률 (PayloadUsageRatio)", ToolTip="Snapshot.PayloadUsageRatio를 그대로 투영합니다."))
	float PayloadUsageRatio = 0.0f;

	// [v1.0.0] 최대 허용 총중량 대비 현재 총중량 사용률입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Mass", meta=(DisplayName="총중량 사용률 (GrossMassUsageRatio)", ToolTip="Snapshot.GrossMassUsageRatio를 그대로 투영합니다."))
	float GrossMassUsageRatio = 0.0f;

	// [v1.0.0] 실제 물리 질량과 추가 Mobility Adapter 상태를 설명하는 Preview입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData", meta=(DisplayName="Mobility Preview (MobilityPreview)", ToolTip="실제 질량 비율과 추가 Mobility Adapter 활성 여부를 표시합니다. P0는 PhysicalMassOnly입니다."))
	FCFFittingMobilityViewData MobilityPreview;

	// [v1.0.0] Snapshot에서 수집된 검증 문제 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData", meta=(DisplayName="문제 표시 행 (IssueRows)", ToolTip="Snapshot.ValidationIssues를 순서와 내용을 바꾸지 않고 그대로 복사합니다."))
	TArray<FCFFittingValidationIssue> IssueRows;

	// [v1.0.0] 로그와 최소 Debug Panel에서 사용할 bounded 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|ViewData|Debug", meta=(DisplayName="Debug 요약 (DebugSummary)", ToolTip="FittingId, Validation, Mount/Ammo/Issue 수와 질량 정보를 한 줄로 요약한 개발용 문자열입니다."))
	FString DebugSummary;
};

/** 결정론적 Fitting Snapshot을 읽기 전용 ViewData로 투영하는 순수 Builder입니다. */
struct CARFIGHT_RE_API FCFFittingViewBuilder
{
	// [v1.0.0] Snapshot을 변경하지 않고 Blueprint/UI용 ViewData를 결정론적으로 생성합니다.
	static FCFVehicleFittingViewData Build(const FCFVehicleFittingSnapshot& FittingSnapshot);
};
