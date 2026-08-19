// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-16
// Description: CarFight 차량 피팅 선택·검증·결정론적 Snapshot 공용 타입
// Scope: 출격 전 장착·Scanner·탄약·방어 선택, 검증 상태·오류와 결정론적 질량 Snapshot의 데이터 계약을 제공합니다.
// Changelog:
// - v1.3.0: CF-FQ-037 SCAN-P0-01 해석된 VehicleSensorData와 단일 Scanner Source 검증 코드를 Snapshot 계약에 추가.
// - v1.2.0: CF-FQ-031 AMMO-P0-07 FCFAmmoSortieLoad를 Snapshot에 보존하고 실제 AmmoMassKg 계산·Runtime 초기화 원본으로 사용하도록 계약 확장.
// - v1.1.0: CF-FQ-034 FIT-P0-03 누락 선택 문제 코드, 정책 빈 장착 소스와 해석된 방어 선택 방식을 추가.
// - v1.0.0: CF-FQ-034 FIT-P0-02 공용 피팅 enum과 구조체를 최초 추가.
// Migration:
// - FIT-P0-03 Snapshot 해석은 Pawn 없이 수행하며 기존 VehicleData, WeaponData, DefenseData와 차량 런타임 동작을 변경하지 않는다.
// - AMMO-P0-07부터 Snapshot의 InitialSortieAmmoLoads가 실제 출격 탄약 수량·질량·VehicleAmmoComp 초기화의 단일 원본이다.
// - MaximumLoadableAmmoCount는 검증 상한일 뿐 현재 출격 수량으로 자동 대입하지 않는다.
// - Chaos Vehicle 질량 적용과 PhysicsAsset 검증은 기존 FIT-P0-05 경로를 유지한다.

#pragma once

#include "CoreMinimal.h"
#include "CFAmmoTypes.h"
#include "CFVehicleWeaponTypes.h"
#include "CFFittingTypes.generated.h"

class UCFEquipmentPresetData;
class UCFTurretMountData;
class UCFVehicleData;
class UCFVehicleDefenseData;
class UCFVehicleSensorData;
class UCFWeaponData;

/**
 * 피팅 검증의 현재 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFFittingValidationState : uint8
{
	NotEvaluated UMETA(DisplayName="미평가 (Not Evaluated)"),
	Valid UMETA(DisplayName="유효 (Valid)"),
	ValidWithWarnings UMETA(DisplayName="경고 포함 유효 (Valid With Warnings)"),
	Invalid UMETA(DisplayName="무효 (Invalid)")
};

/**
 * 피팅 검증 항목의 심각도입니다.
 */
UENUM(BlueprintType)
enum class ECFFittingIssueSeverity : uint8
{
	Info UMETA(DisplayName="정보 (Info)"),
	Warning UMETA(DisplayName="경고 (Warning)"),
	Error UMETA(DisplayName="오류 (Error)")
};

/**
 * 피팅 검증에서 사용할 안정적인 문제 코드입니다.
 */
UENUM(BlueprintType)
enum class ECFFittingIssueCode : uint8
{
	None UMETA(DisplayName="없음 (None)"),
				MissingFittingData UMETA(DisplayName="피팅 데이터 없음 (Missing Fitting Data)"),
	MissingFittingId UMETA(DisplayName="피팅 ID 없음 (Missing Fitting ID)"),
	MissingVehicleData UMETA(DisplayName="차량 데이터 없음 (Missing Vehicle Data)"),
	VehicleDataMismatch UMETA(DisplayName="차량 데이터 불일치 (Vehicle Data Mismatch)"),
	DuplicateMountSelection UMETA(DisplayName="장착 선택 중복 (Duplicate Mount Selection)"),
	MissingMountSelection UMETA(DisplayName="장착 선택 없음 (Missing Mount Selection)"),
	UnknownMountProfile UMETA(DisplayName="알 수 없는 장착 프로파일 (Unknown Mount Profile)"),
	MissingHardpointSlot UMETA(DisplayName="하드포인트 슬롯 없음 (Missing Hardpoint Slot)"),
				MissingEquipmentPreset UMETA(DisplayName="장비 프리셋 없음 (Missing Equipment Preset)"),
	IncompleteEquipmentPreset UMETA(DisplayName="장비 프리셋 불완전 (Incomplete Equipment Preset)"),
	InvalidSensorData UMETA(DisplayName="센서 데이터 무효 (Invalid Sensor Data)"),
	MultipleSensorSources UMETA(DisplayName="복수 센서 Source (Multiple Sensor Sources)"),
	MountTypeMismatch UMETA(DisplayName="장착 타입 불일치 (Mount Type Mismatch)"),
	WeaponSizeExceeded UMETA(DisplayName="무기 크기 초과 (Weapon Size Exceeded)"),
	WeaponMountIncompatible UMETA(DisplayName="무기 장착 비호환 (Weapon Mount Incompatible)"),
	MissingDefenseData UMETA(DisplayName="방어 데이터 없음 (Missing Defense Data)"),
	InvalidAmmoSelection UMETA(DisplayName="탄약 선택 무효 (Invalid Ammo Selection)"),
	AmmoCountExceeded UMETA(DisplayName="탄약 수량 초과 (Ammo Count Exceeded)"),
	InvalidMassValue UMETA(DisplayName="질량 값 무효 (Invalid Mass Value)"),
	GrossMassExceeded UMETA(DisplayName="허용 총중량 초과 (Gross Mass Exceeded)"),
	MissingMassSource UMETA(DisplayName="질량 소스 없음 (Missing Mass Source)"),
	RuntimeMassMismatch UMETA(DisplayName="런타임 질량 불일치 (Runtime Mass Mismatch)")
};

/**
 * 피팅에서 선택하지 않은 장착 프로파일을 해석하는 정책입니다.
 */
UENUM(BlueprintType)
enum class ECFMissingMountPolicy : uint8
{
	UseVehicleDefault UMETA(DisplayName="차량 기본 장비 사용 (Use Vehicle Default)"),
	TreatAsEmpty UMETA(DisplayName="빈 장착으로 처리 (Treat As Empty)"),
	TreatAsError UMETA(DisplayName="오류로 처리 (Treat As Error)")
};

/**
 * 피팅에서 차량 방어 데이터를 선택하는 방식입니다.
 */
UENUM(BlueprintType)
enum class ECFDefenseSelectionMode : uint8
{
	UseVehicleDefault UMETA(DisplayName="차량 기본 방어 사용 (Use Vehicle Default)"),
	ExplicitNone UMETA(DisplayName="방어 없음 명시 (Explicit None)"),
	Override UMETA(DisplayName="피팅 방어로 덮어쓰기 (Override)")
};

/**
 * 최종 장착 선택이 어느 데이터에서 해석됐는지 나타냅니다.
 */
UENUM(BlueprintType)
enum class ECFFittingSelectionSource : uint8
{
	None UMETA(DisplayName="없음 (None)"),
				VehicleDefault UMETA(DisplayName="차량 기본값 (Vehicle Default)"),
	FittingOverride UMETA(DisplayName="피팅 덮어쓰기 (Fitting Override)"),
	ExplicitEmpty UMETA(DisplayName="명시적 빈 장착 (Explicit Empty)"),
	MissingPolicyEmpty UMETA(DisplayName="누락 정책 빈 장착 (Missing Policy Empty)")
};

/**
 * UI와 런타임이 공유할 피팅 검증 문제 한 건입니다.
 */
USTRUCT(BlueprintType)
struct FCFFittingValidationIssue
{
	GENERATED_BODY()

	// [v1.0.0] 이 검증 문제의 심각도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Validation", meta=(DisplayName="문제 심각도 (Severity)", ToolTip="정보, 경고, 오류 중 이 피팅 검증 문제의 심각도입니다."))
	ECFFittingIssueSeverity Severity = ECFFittingIssueSeverity::Info;

	// [v1.0.0] 코드와 UI가 안정적으로 분기할 검증 문제 코드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Validation", meta=(DisplayName="문제 코드 (IssueCode)", ToolTip="피팅 검증 실패 원인을 코드와 UI가 안정적으로 식별할 enum 값입니다."))
	ECFFittingIssueCode IssueCode = ECFFittingIssueCode::None;

	// [v1.0.0] 사용자와 개발자가 읽을 수 있는 검증 문제 설명입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Validation", meta=(DisplayName="문제 설명 (Message)", ToolTip="피팅을 적용할 수 없거나 경고가 발생한 구체적인 이유입니다."))
	FText Message;

	// [v1.0.0] 특정 장착 프로파일과 관련된 문제일 때 해당 프로파일 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Validation", meta=(DisplayName="관련 장착 프로파일 ID (MountProfileId)", ToolTip="특정 장착 선택에서 발생한 문제이면 해당 MountProfileId를 저장합니다. 전역 문제이면 None입니다."))
	FName MountProfileId = NAME_None;
};

/**
 * 한 장착 프로파일에 적용할 플레이어 피팅 선택입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleMountSelection
{
	GENERATED_BODY()

	// [v1.0.0] 이 선택이 덮어쓸 VehicleData 장착 프로파일 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Mount", meta=(DisplayName="장착 프로파일 ID (MountProfileId)", ToolTip="VehicleData.MountProfiles에서 이 선택을 적용할 장착 프로파일 ID입니다."))
	FName MountProfileId = NAME_None;

	// [v1.0.0] 활성 장착에서 사용할 장비 프리셋 DataAsset입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Mount", meta=(EditCondition="bEnabled", DisplayName="장비 프리셋 데이터 (EquipmentPresetData)", ToolTip="이 장착 프로파일에 적용할 EquipmentPresetData입니다. 활성 선택에서 비어 있으면 검증 오류입니다."))
	TObjectPtr<UCFEquipmentPresetData> EquipmentPresetData = nullptr;

	// [v1.0.0] False이면 해당 장착 프로파일을 명시적으로 빈 장착으로 해석합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Mount", meta=(DisplayName="장착 사용 (bEnabled)", ToolTip="True이면 선택한 장비 프리셋을 사용합니다. False이면 누락 선택과 구분되는 명시적 빈 장착입니다."))
	bool bEnabled = true;
};

/**
 * 피팅에서 사용할 차량 방어 선택입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDefenseSelection
{
	GENERATED_BODY()

	// [v1.0.0] 차량 기본 방어, 명시적 방어 없음, 피팅 방어 덮어쓰기 중 선택 방식입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Defense", meta=(DisplayName="방어 선택 방식 (SelectionMode)", ToolTip="차량 기본 방어를 사용할지, 방어 없음을 명시할지, 별도 VehicleDefenseData로 덮어쓸지 결정합니다."))
	ECFDefenseSelectionMode SelectionMode = ECFDefenseSelectionMode::UseVehicleDefault;

	// [v1.0.0] Override 방식에서 사용할 차량 방어 DataAsset입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Defense", meta=(EditCondition="SelectionMode == ECFDefenseSelectionMode::Override", EditConditionHides, DisplayName="방어 데이터 (DefenseData)", ToolTip="방어 선택 방식이 Override일 때 적용할 VehicleDefenseData입니다. Override에서 비어 있으면 검증 오류입니다."))
	TObjectPtr<UCFVehicleDefenseData> DefenseData = nullptr;
};

/**
 * 검증을 통과한 한 장착 프로파일의 해석 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFResolvedFittingMount
{
	GENERATED_BODY()

	// [v1.0.0] 최종 해석된 장착 프로파일 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="장착 프로파일 ID (MountProfileId)", ToolTip="최종 피팅 스냅샷에서 해석된 MountProfileId입니다."))
	FName MountProfileId = NAME_None;

	// [v1.0.0] 최종 해석된 차량 하드포인트 위치 슬롯 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="위치 슬롯 ID (LocationSlotId)", ToolTip="최종 피팅 스냅샷에서 장비를 배치할 하드포인트 위치 슬롯 ID입니다."))
	FName LocationSlotId = NAME_None;

	// [v1.0.0] 최종 해석된 장착 타입입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="장착 타입 (MountType)", ToolTip="최종 피팅 스냅샷에서 검증을 통과한 장착 타입입니다."))
	ECFVehicleMountType MountType = ECFVehicleMountType::None;

	// [v1.0.0] 최종 해석된 장착 크기 제한입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="무기 크기 제한 (SizeLimit)", ToolTip="최종 피팅 스냅샷에서 검증에 사용한 최대 무기 크기입니다."))
	ECFVehicleWeaponSize SizeLimit = ECFVehicleWeaponSize::None;

	// [v1.0.0] 최종 해석된 장비 프리셋 DataAsset입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="장비 프리셋 데이터 (EquipmentPresetData)", ToolTip="차량 기본값 또는 피팅 덮어쓰기에서 최종 선택된 EquipmentPresetData입니다."))
	TObjectPtr<UCFEquipmentPresetData> EquipmentPresetData = nullptr;

	// [v1.0.0] 장비 프리셋에서 최종 해석된 터렛 마운트 DataAsset입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="터렛 마운트 데이터 (TurretMountData)", ToolTip="최종 장비 프리셋에서 해석된 TurretMountData입니다."))
	TObjectPtr<UCFTurretMountData> TurretMountData = nullptr;

				// [v1.0.0] 장비 프리셋에서 최종 해석된 무기 DataAsset입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="무기 데이터 (WeaponData)", ToolTip="최종 장비 프리셋에서 해석된 WeaponData입니다."))
	TObjectPtr<UCFWeaponData> WeaponData = nullptr;

	// [v1.3.0] Scanner Utility 프리셋에서 최종 해석된 Sensor 설정 DataAsset입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="센서 데이터 (SensorData)", ToolTip="Utility Scanner 장비 프리셋에서 해석된 VehicleSensorData입니다. Scanner가 아닌 장착에서는 비어 있습니다."))
	TObjectPtr<UCFVehicleSensorData> SensorData = nullptr;

	// [v1.0.0] 이 최종 선택이 차량 기본값인지 피팅 덮어쓰기인지 나타냅니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="선택 소스 (SelectionSource)", ToolTip="이 장착 결과가 차량 기본 장비, 피팅 덮어쓰기 또는 명시적 빈 장착 중 어디에서 해석됐는지 나타냅니다."))
	ECFFittingSelectionSource SelectionSource = ECFFittingSelectionSource::None;

	// [v1.0.0] 최종 터렛 마운트가 기여하는 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(Units="kg", DisplayName="터렛 마운트 질량 kg (TurretMountMassKg)", ToolTip="최종 TurretMountData가 피팅 총중량에 기여하는 질량입니다."))
	float TurretMountMassKg = 0.0f;

	// [v1.0.0] 최종 무기가 기여하는 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(Units="kg", DisplayName="무기 질량 kg (WeaponMassKg)", ToolTip="최종 WeaponData가 피팅 총중량에 기여하는 질량입니다."))
	float WeaponMassKg = 0.0f;
};

/**
 * 검증을 통과한 피팅의 결정론적 해석 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleFittingSnapshot
{
	GENERATED_BODY()

	// [v1.0.0] 이 스냅샷을 생성한 피팅 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="피팅 ID (FittingId)", ToolTip="이 결정론적 스냅샷을 생성한 VehicleFittingData의 ID입니다."))
	FName FittingId = NAME_None;

	// [v1.0.0] 이 스냅샷의 기준 차량 플랫폼 DataAsset입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="차량 데이터 (VehicleData)", ToolTip="이 피팅 스냅샷이 해석된 기준 VehicleData입니다."))
	TObjectPtr<UCFVehicleData> VehicleData = nullptr;

	// [v1.0.0] 이 스냅샷의 최종 검증 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="검증 상태 (ValidationState)", ToolTip="현재 스냅샷이 미평가, 유효, 경고 포함 유효, 무효 중 어느 상태인지 나타냅니다."))
	ECFFittingValidationState ValidationState = ECFFittingValidationState::NotEvaluated;

	// [v1.0.0] 스냅샷을 생성하는 동안 수집된 검증 문제 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="검증 문제 목록 (ValidationIssues)", ToolTip="피팅 해석 중 수집한 정보, 경고와 오류 목록입니다."))
	TArray<FCFFittingValidationIssue> ValidationIssues;

						// [v1.0.0] 각 장착 프로파일에서 최종 해석된 장비 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="해석된 장착 목록 (ResolvedMounts)", ToolTip="차량 기본값과 피팅 선택을 모두 해석한 최종 장착 결과입니다."))
	TArray<FCFResolvedFittingMount> ResolvedMounts;

	// [v1.3.0] 유효한 Snapshot에서 단 하나로 해석된 Scanner SensorData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="해석된 센서 데이터 (ResolvedSensorData)", ToolTip="장착 결과 전체에서 단 하나로 해석된 VehicleSensorData입니다. Scanner Source가 없거나 둘 이상이면 비어 있습니다."))
	TObjectPtr<UCFVehicleSensorData> ResolvedSensorData = nullptr;

	// [v1.2.0] 실제 이번 출격에 싣는 탄종별 장전+예비 전체 탄약량 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="출격 탄약 적재 목록 (InitialSortieAmmoLoads)", ToolTip="VehicleFittingData에서 검증된 실제 출격 탄약 목록입니다. VehicleAmmoComp 초기화와 AmmoMassKg 계산의 원본이며 피팅 최대 적재량을 현재 수량으로 대체하지 않습니다."))
	TArray<FCFAmmoSortieLoad> InitialSortieAmmoLoads;

	// [v1.1.0] 최종 Snapshot이 사용한 방어 선택 방식입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="해석된 방어 선택 방식 (ResolvedDefenseSelectionMode)", ToolTip="차량 기본 방어, 명시적 방어 없음 또는 피팅 방어 덮어쓰기 중 최종 Snapshot이 해석한 방식을 보존합니다."))
	ECFDefenseSelectionMode ResolvedDefenseSelectionMode = ECFDefenseSelectionMode::UseVehicleDefault;

	// [v1.0.0] 최종 해석된 차량 방어 DataAsset입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot", meta=(DisplayName="해석된 방어 데이터 (ResolvedDefenseData)", ToolTip="차량 기본값, 명시적 None 또는 피팅 Override를 해석한 최종 VehicleDefenseData입니다."))
	TObjectPtr<UCFVehicleDefenseData> ResolvedDefenseData = nullptr;

	// [v1.0.0] 차량 플랫폼 자체의 기준 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot|Mass", meta=(Units="kg", DisplayName="기준 차량 질량 kg (BaseVehicleMassKg)", ToolTip="장비, 탄약과 방어 질량을 더하기 전 VehicleData의 기준 차량 질량입니다."))
	float BaseVehicleMassKg = 0.0f;

	// [v1.0.0] 모든 터렛 마운트와 무기 질량의 합입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot|Mass", meta=(Units="kg", DisplayName="장비 질량 kg (EquipmentMassKg)", ToolTip="해석된 모든 터렛 마운트와 무기 질량을 합산한 값입니다."))
	float EquipmentMassKg = 0.0f;

				// [v1.2.0] 명시적 출격 탄약 전체가 기여하는 실제 총질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot|Mass", meta=(Units="kg", DisplayName="탄약 질량 kg (AmmoMassKg)", ToolTip="InitialSortieAmmoLoads의 실제 출격 수량 × AmmoData.UnitMassKg를 합산한 질량입니다. MaximumLoadableAmmoCount는 질량 계산에 사용하지 않습니다."))
	float AmmoMassKg = 0.0f;

	// [v1.0.0] 최종 방어 패키지가 기여하는 질량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot|Mass", meta=(Units="kg", DisplayName="방어 질량 kg (DefenseMassKg)", ToolTip="최종 VehicleDefenseData가 피팅 총중량에 기여하는 질량입니다."))
	float DefenseMassKg = 0.0f;

	// [v1.0.0] 장비, 탄약과 방어 질량의 합입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot|Mass", meta=(Units="kg", DisplayName="탑재 질량 kg (PayloadMassKg)", ToolTip="EquipmentMassKg, AmmoMassKg와 DefenseMassKg를 합산한 탑재 질량입니다."))
	float PayloadMassKg = 0.0f;

	// [v1.0.0] 기준 차량과 모든 탑재 질량을 합한 최종 총중량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot|Mass", meta=(Units="kg", DisplayName="차량 총중량 kg (TotalVehicleMassKg)", ToolTip="BaseVehicleMassKg와 PayloadMassKg를 합산한 최종 차량 총중량입니다."))
	float TotalVehicleMassKg = 0.0f;

	// [v1.0.0] VehicleData가 허용하는 최대 차량 총중량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot|Mass", meta=(Units="kg", DisplayName="최대 허용 총중량 kg (MaximumGrossMassKg)", ToolTip="이 차량 플랫폼이 피팅 적용을 허용하는 최대 총중량입니다."))
	float MaximumGrossMassKg = 0.0f;

	// [v1.0.0] 파생된 최대 탑재 질량에서 현재 탑재 질량이 차지하는 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot|Mass", meta=(DisplayName="탑재 질량 사용률 (PayloadUsageRatio)", ToolTip="PayloadMassKg를 MaximumGrossMassKg - BaseVehicleMassKg로 나눈 비율입니다."))
	float PayloadUsageRatio = 0.0f;

	// [v1.0.0] 최대 허용 총중량에서 현재 차량 총중량이 차지하는 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Fitting|Snapshot|Mass", meta=(DisplayName="총중량 사용률 (GrossMassUsageRatio)", ToolTip="TotalVehicleMassKg를 MaximumGrossMassKg로 나눈 비율입니다."))
	float GrossMassUsageRatio = 0.0f;

	// [v1.0.0] 오류 없이 런타임 적용 가능한 스냅샷인지 반환합니다.
	bool IsValid() const
	{
		return ValidationState == ECFFittingValidationState::Valid
			|| ValidationState == ECFFittingValidationState::ValidWithWarnings;
	}
};
