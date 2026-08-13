// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-13
// Description: CF-FQ-031 탄종 정적 DataAsset 구현
// Scope: 탄약 질량·적재 한도 유효값과 Debug 요약을 제공합니다.
// Changelog:
// - v1.0.0: AMMO-P0-01 UCFAmmoData 유효값과 요약 구현.
// Migration:
// - 인게임 현재 탄수는 이 DataAsset에서 계산하지 않고 UCFVehicleAmmoComp Runtime이 소유합니다.

#include "CFAmmoData.h"

// [v1.0.0] 음수나 비유한 값을 제거한 탄약 한 단위 질량을 kg으로 반환합니다.
float UCFAmmoData::GetEffectiveUnitMassKg() const
{
	return FMath::IsFinite(UnitMassKg) ? FMath::Max(UnitMassKg, 0.0f) : 0.0f;
}

// [v1.0.0] 음수 적재 한도를 0으로 보정한 피팅 최대 적재량을 반환합니다.
int32 UCFAmmoData::GetEffectiveMaximumLoadableAmmoCount() const
{
	return FMath::Max(MaximumLoadableAmmoCount, 0);
}

// [v1.0.0] Runtime 식별에 필요한 AmmoId가 유효한지 반환합니다.
bool UCFAmmoData::IsAmmoDataValid() const
{
	return !AmmoId.IsNone();
}

// [v1.0.0] 로그와 Debug에서 사용할 탄종 정적 설정 요약을 반환합니다.
FString UCFAmmoData::BuildAmmoSummary() const
{
	return FString::Printf(
		TEXT("AmmoData: Id=%s, Family=%s, UnitMass=%.3fkg, MaximumLoadable=%d, Resupply=%s"),
		*AmmoId.ToString(),
		*AmmoFamilyId.ToString(),
		GetEffectiveUnitMassKg(),
		GetEffectiveMaximumLoadableAmmoCount(),
		bCanBeResupplied ? TEXT("Yes") : TEXT("No"));
}
