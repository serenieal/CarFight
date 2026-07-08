// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-07-03
// Description: CarFight 장비 프리셋 DataAsset 구현
// Scope: 터렛 마운트와 무기 조합의 완성 여부, 장착 호환성, 디버그 요약을 제공합니다.
// Changelog:
// - v1.1.0: MountProfile legacy 직접 fallback 제거에 맞춰 내부 참조 미지정은 Missing 상태로 유지.
// - v1.0.0: 최소 EquipmentPresetData 기본값, 장착 호환성 검사, 디버그 요약 생성을 추가.
// Migration:
// - MountProfile.DefaultEquipmentPresetData가 있으면 TurretMountData / WeaponData의 단일 소스로 사용한다.
// - 프리셋 내부 참조가 비어 있으면 해당 장비 데이터는 Missing 상태가 되며 MountProfile 직접 fallback은 없다.

#include "CFEquipmentPresetData.h"

#include "CFTurretMountData.h"
#include "CFWeaponData.h"

// [v1.0.0] 기본 장비 프리셋 값을 초기화합니다.
UCFEquipmentPresetData::UCFEquipmentPresetData()
	: DisplayName(NSLOCTEXT("CarFight", "ProtoRoofCannonKitDisplayName", "Prototype Roof Cannon Kit"))
{
}

// [v1.0.0] 터렛 마운트와 무기 데이터가 모두 지정되어 있는지 반환합니다.
bool UCFEquipmentPresetData::HasCompleteEquipmentData() const
{
	return DefaultTurretMountData != nullptr && DefaultWeaponData != nullptr;
}

// [v1.0.0] 지정한 장착 타입과 크기 제한에서 이 장비 프리셋을 사용할 수 있는지 반환합니다.
bool UCFEquipmentPresetData::CanUseOnMount(const ECFVehicleMountType InMountType, const ECFVehicleWeaponSize InMountSizeLimit) const
{
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

	return bMountTypeCompatible && bWeaponSizeCompatible && bWeaponDataCompatible;
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

	// [v1.0.0] 프리셋 내부 필수 조합이 모두 지정되어 있는지 표시할 문자열입니다.
	const FString CompleteText = HasCompleteEquipmentData() ? TEXT("Yes") : TEXT("No");

	return FString::Printf(
		TEXT("EquipmentPresetData: Id=%s, Name=%s, RequiredMount=%s, RequiredSize=%s, TurretMount=%s, Weapon=%s, Complete=%s"),
		*EquipmentId.ToString(),
		*DisplayName.ToString(),
		*RequiredMountTypeText,
		*RequiredWeaponSizeText,
		*TurretMountDataText,
		*WeaponDataText,
		*CompleteText);
}
