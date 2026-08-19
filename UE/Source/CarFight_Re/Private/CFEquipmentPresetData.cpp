// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-16
// Description: CarFight 장비 프리셋 DataAsset 구현
// Scope: 무장 패키지와 Utility Scanner 패키지의 완성 여부, 장착 호환성, 디버그 요약을 제공합니다.
// Changelog:
// - v1.2.0: CF-FQ-037 SCAN-P0-01 VehicleSensorData Scanner payload, mount-aware 완성도와 SensorConfig 호환 검증을 추가.
// - v1.1.0: MountProfile legacy 직접 fallback 제거에 맞춰 내부 참조 미지정은 Missing 상태로 유지.
// - v1.0.0: 최소 EquipmentPresetData 기본값, 장착 호환성 검사, 디버그 요약 생성을 추가.
// Migration:
// - 기존 전투 장비는 DefaultTurretMountData + DefaultWeaponData 조합을 그대로 사용합니다.
// - Utility Scanner는 DefaultSensorData만 사용하고 RequiredWeaponSize=None으로 설정합니다.
// - 한 프리셋에 무장 payload와 Scanner payload를 동시에 지정하면 모호한 계약으로 거부합니다.
// - SensorData의 실제 Runtime 적용과 hot reapply는 SCAN-P0-02 이후가 소유합니다.

#include "CFEquipmentPresetData.h"

#include "CFTurretMountData.h"
#include "CFVehicleSensorData.h"
#include "CFWeaponData.h"

// [v1.0.0] 기본 장비 프리셋 값을 초기화합니다.
UCFEquipmentPresetData::UCFEquipmentPresetData()
	: DisplayName(NSLOCTEXT("CarFight", "ProtoRoofCannonKitDisplayName", "Prototype Roof Cannon Kit"))
{
}

// [v1.2.0] 무장 패키지 또는 Scanner 패키지 중 하나의 payload가 모호하지 않게 완성됐는지 반환합니다.
bool UCFEquipmentPresetData::HasCompleteEquipmentData() const
{
	// [v1.2.0] 기존 전투 장비가 TurretMountData와 WeaponData만 함께 가지는 완성 상태입니다.
	const bool bHasCompleteWeaponPackage = DefaultTurretMountData != nullptr
		&& DefaultWeaponData != nullptr
		&& DefaultSensorData == nullptr;

	// [v1.2.0] Utility Scanner가 SensorData만 가지는 완성 상태입니다.
	const bool bHasCompleteScannerPackage = DefaultSensorData != nullptr
		&& DefaultTurretMountData == nullptr
		&& DefaultWeaponData == nullptr;

	return bHasCompleteWeaponPackage || bHasCompleteScannerPackage;
}

// [v1.2.0] 실제 MountType에서 요구하는 장비 payload가 정확히 완성됐는지 반환합니다.
bool UCFEquipmentPresetData::HasCompleteEquipmentDataForMount(const ECFVehicleMountType InMountType) const
{
	if (InMountType == ECFVehicleMountType::Utility)
	{
		return DefaultSensorData != nullptr
			&& DefaultTurretMountData == nullptr
			&& DefaultWeaponData == nullptr
			&& RequiredWeaponSize == ECFVehicleWeaponSize::None;
	}

	if (InMountType == ECFVehicleMountType::None)
	{
		return false;
	}

	return DefaultTurretMountData != nullptr
		&& DefaultWeaponData != nullptr
		&& DefaultSensorData == nullptr;
}

// [v1.0.0] 지정한 장착 타입과 크기 제한에서 이 장비 프리셋을 사용할 수 있는지 반환합니다.
bool UCFEquipmentPresetData::CanUseOnMount(const ECFVehicleMountType InMountType, const ECFVehicleWeaponSize InMountSizeLimit) const
{
	if (!HasCompleteEquipmentDataForMount(InMountType))
	{
		return false;
	}

	// [v1.0.0] 프리셋의 장착 타입 요구사항을 통과했는지 여부입니다.
	const bool bMountTypeCompatible = (RequiredMountType == ECFVehicleMountType::None) || (RequiredMountType == InMountType);

	// [v1.0.0] 프리셋의 장착 크기 요구사항을 통과했는지 여부입니다.
	bool bWeaponSizeCompatible = RequiredWeaponSize == ECFVehicleWeaponSize::None;

	if (!bWeaponSizeCompatible && InMountSizeLimit != ECFVehicleWeaponSize::None)
	{
		// [v1.0.0] 프리셋 요구 크기 enum 값을 정수로 환산한 값입니다.
		const uint8 RequiredWeaponSizeValue = static_cast<uint8>(RequiredWeaponSize);

		// [v1.0.0] 장착 프로파일 크기 제한 enum 값을 정수로 환산한 값입니다.
		const uint8 MountSizeLimitValue = static_cast<uint8>(InMountSizeLimit);

		bWeaponSizeCompatible = RequiredWeaponSizeValue <= MountSizeLimitValue;
	}

	// [v1.0.0] 연결된 WeaponData 자체가 장착 타입과 크기 제한을 통과했는지 여부입니다.
	const bool bWeaponDataCompatible = !DefaultWeaponData || DefaultWeaponData->CanUseOnMount(InMountType, InMountSizeLimit);

	// [v1.2.0] 연결된 Scanner SensorData가 실제 SensorConfig 계약을 통과했는지 여부입니다.
	const bool bSensorDataCompatible = !DefaultSensorData || DefaultSensorData->IsSensorConfigValid();

	return bMountTypeCompatible
		&& bWeaponSizeCompatible
		&& bWeaponDataCompatible
		&& bSensorDataCompatible;
}

// [v1.0.0] 디버그 패널에 표시할 장비 프리셋 요약 문자열을 생성합니다.
FString UCFEquipmentPresetData::BuildEquipmentSummary() const
{
	// [v1.0.0] 요구 장착 타입 enum 값을 표시용 문자열로 변환한 값입니다.
	const FString RequiredMountTypeText = UEnum::GetValueAsString(RequiredMountType);

	// [v1.0.0] 요구 무기 크기 enum 값을 표시용 문자열로 변환한 값입니다.
	const FString RequiredWeaponSizeText = UEnum::GetValueAsString(RequiredWeaponSize);

	// [v1.0.0] 터렛 마운트 데이터 ID 또는 미지정 표시 문자열입니다.
	const FString TurretMountDataText = DefaultTurretMountData ? DefaultTurretMountData->TurretMountId.ToString() : TEXT("MissingOptional");

	// [v1.0.0] 무기 데이터 ID 또는 미지정 표시 문자열입니다.
	const FString WeaponDataText = DefaultWeaponData ? DefaultWeaponData->WeaponId.ToString() : TEXT("MissingOptional");

	// [v1.2.0] Scanner SensorData 에셋 이름 또는 미지정 표시 문자열입니다.
	const FString SensorDataText = DefaultSensorData ? DefaultSensorData->GetName() : TEXT("MissingOptional");

	// [v1.2.0] 프리셋 내부 payload가 무장 또는 Scanner 중 하나로 완성됐는지 표시할 문자열입니다.
	const FString CompleteText = HasCompleteEquipmentData() ? TEXT("Yes") : TEXT("No");

	return FString::Printf(
		TEXT("EquipmentPresetData: Id=%s, Name=%s, RequiredMount=%s, RequiredSize=%s, TurretMount=%s, Weapon=%s, Sensor=%s, Complete=%s"),
		*EquipmentId.ToString(),
		*DisplayName.ToString(),
		*RequiredMountTypeText,
		*RequiredWeaponSizeText,
		*TurretMountDataText,
		*WeaponDataText,
		*SensorDataText,
		*CompleteText);
}
