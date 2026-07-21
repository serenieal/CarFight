// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-07-14
// Description: CarFight 차량 피해 DataAsset 구현
// Scope: DamageData 기본값과 직접 피해 정책을 포함한 디버그 요약 생성을 제공합니다.
// Changelog:
// - v1.1.0: 자기 피해 허용 정책을 DamageData 요약 문자열에 추가.
// - v1.0.0: DamageData 최소 필드와 디버그 요약 생성을 추가.
// Migration:
// - BaseDamage만 최소 VehicleHealthComp 직접 피해에 사용하고 장갑, 모듈, 범위 피해와 물리 충격은 후속으로 유지한다.

#include "CFDamageData.h"

// [v1.0.0] 기본 피해 데이터 값을 초기화합니다.
UCFDamageData::UCFDamageData()
{
}

// [v1.0.0] 디버그 패널과 로그에 표시할 피해 데이터 요약 문자열을 생성합니다.
FString UCFDamageData::BuildDamageSummary() const
{
	// [v1.0.0] 피해 타입 enum 값을 표시용 문자열로 변환한 값입니다.
	const FString DamageTypeText = UEnum::GetValueAsString(DamageType);

	// [v1.0.0] 범위 피해 사용 가능 여부를 표시할 문자열입니다.
	const FString RadialDamageText = CanUseRadialDamage() ? TEXT("Ready") : TEXT("DisabledOrIncomplete");

	// [v1.0.0] 범위 피해 설정 스위치를 표시할 문자열입니다.
	const FString UseRadialDamageText = bUseRadialDamage ? TEXT("Yes") : TEXT("No");

	// [v1.1.0] 자기 피해 허용 정책을 표시할 문자열입니다.
	const FString CanDamageSelfText = bCanDamageSelf ? TEXT("Yes") : TEXT("No");

	return FString::Printf(
		TEXT("DamageData: Id=%s, Type=%s, Base=%.1f, SelfDamage=%s, Pen=%.1f, UseRadial=%s, RadialGate=%s, Radius=%.1f, InnerRadius=%.1f, Explosion=%.1f, MinScale=%.2f, ModuleScale=%.2f, Impulse=%.1f"),
		*DamageId.ToString(),
		*DamageTypeText,
		BaseDamage,
		*CanDamageSelfText,
		ArmorPenetration,
		*UseRadialDamageText,
		*RadialDamageText,
		ExplosionRadius,
		ExplosionInnerRadius,
		ExplosionDamage,
		MinExplosionDamageScale,
		ModuleDamageScale,
		ImpulseStrength);
}

// [v1.0.0] 폭발 / 범위 피해를 사용할 수 있는 설정인지 반환합니다.
bool UCFDamageData::CanUseRadialDamage() const
{
	return bUseRadialDamage && ExplosionRadius > 0.0f && ExplosionDamage > 0.0f;
}
