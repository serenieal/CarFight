// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-18
// Description: CF-FQ-032 UI-P0-06 Player-facing Weapon Selection의 내부 Runtime 입력 타입
// Scope: Applied Fitting Snapshot의 실제 weapon-bearing mount를 고정 순서로 WeaponComp에 전달하되 내부 MountProfileId를 UI 의미나 표시 이름으로 노출하지 않습니다.
// Changelog:
// - v1.0.0: 내부 MountProfileId, 실제 EquipmentPresetData, 실제 WeaponData를 묶는 선택 Runtime 항목을 추가.
// Migration:
// - InternalMountProfileId는 Ammo/FireOrigin 등 기존 Runtime identity와 연결하기 위한 내부 키일 뿐 Player-facing WeaponGroup/이름으로 표시하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFWeaponSelectTypes.generated.h"

class UCFEquipmentPresetData;
class UCFWeaponData;

/**
 * Applied Fitting Snapshot의 한 weapon-bearing mount를 WeaponComp 선택 Runtime에 전달하는 내부 항목입니다.
 */
USTRUCT()
struct CARFIGHT_RE_API FCFWeaponSelectRuntimeItem
{
	GENERATED_BODY()

	// [v1.0.0] 기존 FireOrigin·Ammo Runtime과 연결할 실제 내부 MountProfileId입니다. Player-facing 이름/그룹으로 사용하지 않습니다.
	UPROPERTY()
	FName InternalMountProfileId = NAME_None;

	// [v1.0.0] 현재 피팅이 이 mount에 실제로 해석한 EquipmentPresetData입니다.
	UPROPERTY()
	TObjectPtr<UCFEquipmentPresetData> EquipmentPresetData = nullptr;

	// [v1.0.0] 현재 피팅이 이 mount에 실제로 해석한 WeaponData입니다.
	UPROPERTY()
	TObjectPtr<UCFWeaponData> WeaponData = nullptr;
};
