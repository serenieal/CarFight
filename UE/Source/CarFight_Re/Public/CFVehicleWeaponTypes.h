// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.8.0
// Date: 2026-07-03
// Description: CarFight 차량 전투 장착 프로파일과 발사 원점 공용 타입
// Scope: 하드포인트 장착 타입, 무기 크기, 장착 프로파일, 터렛 상태, 발사 원점 계산 결과를 제공합니다.
// Changelog:
// - v1.8.0: MountProfile의 DefaultWeaponData / DefaultTurretMountData legacy 직접 슬롯을 삭제하고 EquipmentPresetData 전용 경로로 전환.
// - v1.7.0: MountProfile에 기본 EquipmentPresetData 참조를 추가하고 WeaponData / TurretMountData 직접 참조를 legacy fallback으로 명시.
// - v1.6.0: DataAsset 재저장 확인 후 MountProfile의 legacy Min/Max Yaw/Pitch 필드를 실제 삭제.
// - v1.5.0: MountProfile 각도 제한 4개를 legacy 직렬화 호환값으로 전환하고 런타임 / 에디터 노출에서 제거.
// - v1.4.0: MountProfile inline 터렛 시각 / 기계값 fallback을 런타임 제거 단계로 전환하고 에디터 편집 노출을 중단.
// - v1.3.0: MountProfile에 기본 TurretMountData 참조를 추가하고 inline 터렛 시각 필드를 fallback으로 명시.
// - v1.2.0: MountProfile에 P0 터렛 시각 장착용 Yaw / Pitch 메쉬와 Pitch 피벗 소켓 설정을 추가.
// - v1.1.0: MountProfile에 선택 기본 WeaponData 참조를 추가해 차량 장착 슬롯과 무기 데이터를 연결.
// - v1.0.0: P0 Top_01 터렛 FireOrigin 검증을 위한 최소 전투 장착 타입을 추가.
// Migration:
// - 기존 HardpointSlots는 유지하고, MountProfile은 LocationSlotRef로 HardpointSlots를 참조한다.
// - DefaultEquipmentPresetData가 지정되면 TurretMountData / WeaponData의 단일 소스로 사용한다.
// - DefaultEquipmentPresetData가 비어 있거나 프리셋 내부 참조가 비어 있으면 해당 장비 데이터는 Missing 상태로 표시한다.
// - MountProfile의 DefaultWeaponData / DefaultTurretMountData 직접 참조 슬롯은 삭제했으며 fallback으로 사용하지 않는다.
// - WeaponData가 비어 있어도 기존 FireOrigin 계산은 계속 fallback 없이 동작한다.
// - TurretMountData가 비어 있으면 터렛 시각 / 회전 기계값은 Missing 상태로 건너뛰고 MountProfile inline fallback은 사용하지 않는다.
// - MountProfile의 Min/Max Yaw/Pitch는 DataAsset 재저장 확인 뒤 삭제했으며 런타임 터렛 제한에 사용하지 않는다.
// - 터렛 회전 제한은 EquipmentPresetData 내부 TurretMountData의 Min/Max Yaw/Pitch만 사용한다.
// - 남은 legacy inline 터렛 시각 / 기계 필드는 기존 DataAsset 직렬화 호환을 위해 잠시 보존하지만 에디터 편집 대상에서 제외한다.
// - ProjectileData / DamageData는 각각 전용 DataAsset 기준이며 MountProfile에는 직접 추가하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleWeaponTypes.generated.h"

class UCFEquipmentPresetData;
class UStaticMesh;

/**
 * 차량 하드포인트 장착 방식입니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleMountType : uint8
{
	None UMETA(DisplayName="None"),
	Fixed UMETA(DisplayName="Fixed"),
	Gimbal UMETA(DisplayName="Gimbal"),
	Turret UMETA(DisplayName="Turret"),
	Launcher UMETA(DisplayName="Launcher"),
	Utility UMETA(DisplayName="Utility")
};

/**
 * 차량 무기 장착 크기 제한입니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleWeaponSize : uint8
{
	None UMETA(DisplayName="None"),
	Small UMETA(DisplayName="Small"),
	Medium UMETA(DisplayName="Medium"),
	Large UMETA(DisplayName="Large")
};

/**
 * 차량 위치 슬롯에서 어떤 장착 규칙을 사용할지 설명하는 데이터입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleMountProfile
{
	GENERATED_BODY()

	// [v1.0.0] 장착 프로파일을 식별하는 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Mount", meta=(DisplayName="장착 프로파일 ID (MountProfileId)", ToolTip="전투 장착 규칙을 식별하는 이름입니다. 예: RoofTurret_MediumOrLarge"))
	FName MountProfileId = TEXT("RoofTurret_MediumOrLarge");

	// [v1.0.0] 이 장착 프로파일이 사용할 차량 하드포인트 위치 슬롯입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Mount", meta=(DisplayName="위치 슬롯 참조 (LocationSlotRef)", ToolTip="이 장착 프로파일이 참조할 차량 하드포인트 위치 슬롯 ID입니다. 예: Top_01"))
	FName LocationSlotRef = TEXT("Top_01");

	// [v1.0.0] 이 슬롯이 사용하는 장착 방식입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Mount", meta=(DisplayName="장착 타입 (MountType)", ToolTip="Fixed, Gimbal, Turret, Launcher, Utility 중 이 프로파일의 전투 규칙 타입입니다."))
	ECFVehicleMountType MountType = ECFVehicleMountType::Turret;

	// [v1.0.0] 이 슬롯에 장착 가능한 최대 무기 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Mount", meta=(DisplayName="무기 크기 제한 (SizeLimit)", ToolTip="이 장착 프로파일이 허용하는 최대 무기 크기입니다."))
	ECFVehicleWeaponSize SizeLimit = ECFVehicleWeaponSize::Large;

	// [v1.8.0] 이 장착 프로파일에서 기본으로 사용할 장비 프리셋 DataAsset입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Mount", meta=(DisplayName="기본 장비 프리셋 데이터 (DefaultEquipmentPresetData)", ToolTip="이 장착 프로파일에 기본으로 연결할 장비 프리셋 DataAsset입니다. TurretMountData와 WeaponData의 단일 해석 소스로 사용합니다."))
	TObjectPtr<UCFEquipmentPresetData> DefaultEquipmentPresetData = nullptr;

	// [v1.4.0] 기존 DataAsset 직렬화 호환만을 위해 남긴 legacy 터렛 하부 메쉬입니다.
	UPROPERTY()
	TObjectPtr<UStaticMesh> TurretYawMesh = nullptr;

	// [v1.4.0] 기존 DataAsset 직렬화 호환만을 위해 남긴 legacy 터렛 상부 또는 포신 메쉬입니다.
	UPROPERTY()
	TObjectPtr<UStaticMesh> TurretPitchMesh = nullptr;

	// [v1.4.0] 기존 DataAsset 직렬화 호환만을 위해 남긴 legacy Yaw 메쉬 상대 Transform입니다.
	UPROPERTY()
	FTransform TurretYawRelativeTransform = FTransform::Identity;

	// [v1.4.0] 기존 DataAsset 직렬화 호환만을 위해 남긴 legacy Pitch 회전 기준 소켓 이름입니다.
	UPROPERTY()
	FName PitchPivotSocketName = TEXT("PitchPivot");

	// [v1.4.0] 기존 DataAsset 직렬화 호환만을 위해 남긴 legacy Pitch 메쉬 상대 Transform입니다.
	UPROPERTY()
	FTransform TurretPitchRelativeTransform = FTransform::Identity;

	// [v1.4.0] 기존 DataAsset 직렬화 호환만을 위해 남긴 legacy 좌우 회전 속도 값입니다.
	UPROPERTY()
	float YawTurnRateDegPerSec = 35.0f;

	// [v1.4.0] 기존 DataAsset 직렬화 호환만을 위해 남긴 legacy 상하 회전 속도 값입니다.
	UPROPERTY()
	float PitchTurnRateDegPerSec = 20.0f;

	// [v1.4.0] 기존 DataAsset 직렬화 호환만을 위해 남긴 legacy 안정화 허용 오차입니다.
	UPROPERTY()
	float StabilizationToleranceDeg = 2.0f;

	// [v1.4.0] 기존 DataAsset 직렬화 호환만을 위해 남긴 legacy 안정화 유지 시간입니다.
	UPROPERTY()
	float AimSettleTimeSeconds = 0.35f;

	// [v1.4.0] 기존 DataAsset 직렬화 호환만을 위해 남긴 legacy 장착 중량 값입니다.
	UPROPERTY()
	float MountWeightKg = 350.0f;

	// [v1.0.0] 외부 노출 장비로 보고 후속 Damage 후보에 포함할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Mount", meta=(DisplayName="노출 모듈 여부 (bExposedModule)", ToolTip="True이면 이 장착 프로파일을 외부 노출 장비로 보고 후속 Damage / Module 후보에 포함합니다."))
	bool bExposedModule = true;
};

/**
 * 차량 터렛의 현재 조준 추적 상태입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleTurretState
{
	GENERATED_BODY()

	// [v1.0.0] 현재 터렛 좌우 각도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Turret", meta=(DisplayName="현재 Yaw 각도 (CurrentYawDeg)", ToolTip="현재 터렛 좌우 회전 각도입니다."))
	float CurrentYawDeg = 0.0f;

	// [v1.0.0] 현재 포신 상하 각도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Turret", meta=(DisplayName="현재 Pitch 각도 (CurrentPitchDeg)", ToolTip="현재 포신 상하 회전 각도입니다."))
	float CurrentPitchDeg = 0.0f;

	// [v1.0.0] 목표 터렛 좌우 각도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Turret", meta=(DisplayName="목표 Yaw 각도 (TargetYawDeg)", ToolTip="플레이어 조준점 또는 목표 기준으로 계산된 목표 좌우 각도입니다."))
	float TargetYawDeg = 0.0f;

	// [v1.0.0] 목표 포신 상하 각도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Turret", meta=(DisplayName="목표 Pitch 각도 (TargetPitchDeg)", ToolTip="플레이어 조준점 또는 목표 기준으로 계산된 목표 상하 각도입니다."))
	float TargetPitchDeg = 0.0f;

	// [v1.0.0] 터렛이 목표 각도에 충분히 도달했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Turret", meta=(DisplayName="터렛 안정화 여부 (bTurretSettled)", ToolTip="터렛이 목표 각도에 충분히 도달해 발사 가능한 상태로 볼 수 있는지 여부입니다."))
	bool bTurretSettled = false;
};

/**
 * 실제 Fire가 사용할 발사 원점 계산 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleFireOrigin
{
	GENERATED_BODY()

	// [v1.0.0] 발사 원점 계산이 성공했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Fire", meta=(DisplayName="발사 원점 해결 여부 (bResolved)", ToolTip="하드포인트와 장착 프로파일에서 실제 발사 원점을 계산했는지 여부입니다."))
	bool bResolved = false;

	// [v1.0.0] 발사 원점을 만든 장착 프로파일 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Fire", meta=(DisplayName="장착 프로파일 ID (MountProfileId)", ToolTip="발사 원점 계산에 사용한 장착 프로파일 ID입니다."))
	FName MountProfileId = NAME_None;

	// [v1.0.0] 발사 원점을 만든 위치 슬롯 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Fire", meta=(DisplayName="위치 슬롯 ID (LocationSlotId)", ToolTip="발사 원점 계산에 사용한 차량 하드포인트 위치 슬롯 ID입니다."))
	FName LocationSlotId = NAME_None;

	// [v1.0.0] 발사 원점을 만든 장착 타입입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Fire", meta=(DisplayName="장착 타입 (MountType)", ToolTip="발사 원점 계산에 사용한 장착 타입입니다."))
	ECFVehicleMountType MountType = ECFVehicleMountType::None;

	// [v1.0.0] 월드 기준 실제 발사 시작 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Fire", meta=(DisplayName="월드 발사 위치 (WorldFireLocation)", ToolTip="월드 기준 실제 발사 시작 위치입니다."))
	FVector WorldFireLocation = FVector::ZeroVector;

	// [v1.0.0] 월드 기준 실제 발사 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Vehicle Weapon|Fire", meta=(DisplayName="월드 발사 방향 (WorldFireDirection)", ToolTip="월드 기준 실제 발사 방향입니다."))
	FVector WorldFireDirection = FVector::ForwardVector;
};
