// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-02
// Description: CarFight 차량 피해 DataAsset 구현
// Scope: DamageData 기본값과 디버그 요약 생성을 제공합니다.
// Changelog:
// - v1.0.0: DamageData 최소 필드와 디버그 요약 생성을 추가.
// Migration:
// - 실제 HP 차감 / 모듈 손상 / 장갑 계산은 후속 Damage Runtime으로 분리하고, 이 단계에서는 값 저장과 Debug 확인만 수행한다.

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

	return FString::Printf(
		TEXT("DamageData: Id=%s, Type=%s, Base=%.1f, Pen=%.1f, UseRadial=%s, RadialGate=%s, Radius=%.1f, InnerRadius=%.1f, Explosion=%.1f, MinScale=%.2f, ModuleScale=%.2f, Impulse=%.1f"),
		*DamageId.ToString(),
		*DamageTypeText,
		BaseDamage,
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
