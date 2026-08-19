// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-18
// Description: CarFight 장비 프리셋 DataAsset
// Scope: 차량 MountProfile이 우선 참조할 무장 패키지 또는 Scanner Utility 설정과 Player-facing 장비 표시 이름을 제공합니다.
// Changelog:
// - v1.3.0: UI-P0-06 감사에서 Fitting ViewData가 이미 소비하던 DisplayName을 Player-facing 장비 표시 이름 계약으로 명확화. 빈 이름은 공개 이름 없음이며 EquipmentId/WeaponId/AssetName으로 대체하지 않음.
// - v1.2.0: CF-FQ-037 SCAN-P0-01 기존 VehicleSensorData를 Utility Scanner payload로 추가하고 mount-aware 완성도 검증을 도입.
// - v1.1.0: MountProfile legacy 직접 fallback 제거에 맞춰 비어 있는 내부 참조는 Missing 상태로 처리한다고 명시.
// - v1.0.0: TurretMountData와 WeaponData의 유효 조합을 묶는 최소 EquipmentPresetData를 추가.
// Migration:
// - FCFVehicleMountProfile.DefaultEquipmentPresetData에 이 DataAsset을 연결한다.
// - 기존 전투 장비는 DefaultTurretMountData + DefaultWeaponData를 유지하고 DefaultSensorData는 비운다.
// - Utility Scanner는 기존 UCFVehicleSensorData를 DefaultSensorData로 연결하고 RequiredWeaponSize=None을 사용한다.
// - 무장 payload와 Scanner payload를 한 프리셋에 동시에 지정하지 않는다.
// - ProjectileData와 DamageData는 기존처럼 WeaponData.DefaultProjectileData / ProjectileData.DefaultDamageData 경로를 유지한다.
// - DisplayName은 Fitting과 전투 HUD가 공유하는 Player-facing 사람 읽기용 이름이다. 빈 값은 공개 이름 미지정이며 내부 EquipmentId, WeaponId 또는 AssetName으로 자동 대체하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleWeaponTypes.h"
#include "Engine/DataAsset.h"
#include "CFEquipmentPresetData.generated.h"

class UCFTurretMountData;
class UCFVehicleSensorData;
class UCFWeaponData;

/**
  * 무장 패키지 또는 Utility Scanner 패키지를 하나의 장비 프리셋으로 표현하는 데이터입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFEquipmentPresetData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] 기본 장비 프리셋 값을 초기화합니다.
	UCFEquipmentPresetData();

				// [v1.2.0] 무장 패키지 또는 Scanner 패키지 중 하나의 payload가 모호하지 않게 완성됐는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|EquipmentPresetData", meta=(DisplayName="장비 데이터 완성 여부 (Has Complete Equipment Data)", ToolTip="TurretMountData+WeaponData 무장 패키지 또는 SensorData Scanner 패키지 중 하나만 완성되어 있는지 반환합니다."))
	bool HasCompleteEquipmentData() const;

	// [v1.2.0] 실제 MountType에서 요구하는 장비 payload가 정확히 완성됐는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|EquipmentPresetData", meta=(DisplayName="장착 타입 기준 장비 완성 여부 (Has Complete Equipment Data For Mount)", ToolTip="Utility는 SensorData만, 기존 전투 Mount는 TurretMountData+WeaponData만 사용하도록 장착 타입 기준 완성도를 검사합니다."))
	bool HasCompleteEquipmentDataForMount(ECFVehicleMountType InMountType) const;

	// [v1.0.0] 지정한 장착 타입과 크기 제한에서 이 장비 프리셋을 사용할 수 있는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|EquipmentPresetData", meta=(DisplayName="장착 가능 여부 (Can Use On Mount)", ToolTip="장비 프리셋의 payload 완성도, 요구 장착 타입, 요구 크기와 무기 또는 SensorConfig 호환성을 함께 검사합니다."))
	bool CanUseOnMount(ECFVehicleMountType InMountType, ECFVehicleWeaponSize InMountSizeLimit) const;

	// [v1.0.0] 디버그 패널에 표시할 장비 프리셋 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|EquipmentPresetData", meta=(DisplayName="장비 프리셋 요약 생성 (Build Equipment Summary)", ToolTip="디버그 패널과 로그에 표시할 장비 프리셋 핵심 데이터 요약 문자열을 생성합니다."))
	FString BuildEquipmentSummary() const;

	// [v1.0.0] 장비 프리셋을 식별하는 안정적인 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|EquipmentPresetData|Identity", meta=(DisplayName="장비 프리셋 ID (EquipmentId)", ToolTip="전투 로그, 디버그, 저장 데이터에서 이 장비 조합을 식별할 이름입니다. 예: Proto_RoofCannonKit"))
	FName EquipmentId = TEXT("Proto_RoofCannonKit");

		// [v1.3.0] 에디터·Fitting·전투 HUD가 공유하는 Player-facing 장비 프리셋 표시 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|EquipmentPresetData|Identity", meta=(DisplayName="표시 이름 (DisplayName)", ToolTip="플레이어 UI와 에디터에서 사람이 읽을 장비 프리셋 이름입니다. 비어 있으면 공개 이름 미지정이며 EquipmentId, WeaponId 또는 Asset 이름으로 자동 대체하지 않습니다."))
	FText DisplayName;

	// [v1.0.0] 이 장비 프리셋이 요구하는 장착 타입입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|EquipmentPresetData|Mount", meta=(DisplayName="요구 장착 타입 (RequiredMountType)", ToolTip="이 장비 프리셋을 사용할 수 있는 차량 장착 타입입니다. None이면 프리셋 자체의 타입 제한은 생략합니다."))
	ECFVehicleMountType RequiredMountType = ECFVehicleMountType::Turret;

	// [v1.0.0] 이 장비 프리셋이 요구하는 최소 장착 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|EquipmentPresetData|Mount", meta=(DisplayName="요구 무기 크기 (RequiredWeaponSize)", ToolTip="이 장비 프리셋이 요구하는 무기 크기입니다. MountProfile의 SizeLimit보다 작거나 같아야 합니다."))
	ECFVehicleWeaponSize RequiredWeaponSize = ECFVehicleWeaponSize::Large;

	// [v1.1.0] 이 장비 프리셋이 사용할 터렛 마운트 DataAsset입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|EquipmentPresetData|Refs", meta=(DisplayName="기본 터렛 마운트 데이터 (DefaultTurretMountData)", ToolTip="이 장비 조합에서 사용할 터렛 마운트 DataAsset입니다. 비어 있으면 터렛 시각 / 조준 추적은 Missing 상태가 됩니다."))
	TObjectPtr<UCFTurretMountData> DefaultTurretMountData = nullptr;

				// [v1.1.0] 이 장비 프리셋이 사용할 무기 DataAsset입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|EquipmentPresetData|Refs", meta=(DisplayName="기본 무기 데이터 (DefaultWeaponData)", ToolTip="이 장비 조합에서 사용할 무기 DataAsset입니다. 비어 있으면 WeaponData / ProjectileData 해석은 Missing 상태가 됩니다."))
	TObjectPtr<UCFWeaponData> DefaultWeaponData = nullptr;

	// [v1.2.0] Utility Scanner 장비 프리셋이 사용할 기존 VehicleSensorData입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|EquipmentPresetData|Refs", meta=(DisplayName="기본 센서 데이터 (DefaultSensorData)", ToolTip="Utility Scanner 프리셋에서 사용할 VehicleSensorData입니다. Scanner가 아닌 무장 프리셋에서는 비워 둡니다."))
	TObjectPtr<UCFVehicleSensorData> DefaultSensorData = nullptr;
};
