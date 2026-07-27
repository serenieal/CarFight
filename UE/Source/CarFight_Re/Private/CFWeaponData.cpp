// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.5.0
// Date: 2026-07-24
// Description: CarFight 차량 무기 DataAsset 구현
// Scope: 차량 장착 프로파일 호환성과 선택 대상 사용 정책 요약을 제공합니다.
// Changelog:
// - v1.5.0: TS-P0-07 TargetUsePolicy 필터 개수를 무기 디버그 요약에 추가.
// - v1.4.0: WeaponData 직접 DamageData 요약을 제거하고 ProjectileData 단일 피해 소유 구조로 정리.
// - v1.3.0: DefaultDamageData 직접 참조를 디버그 요약에 포함.
// - v1.2.0: 분당 발사속도 기반 발사 간격 환산과 기존 CooldownSeconds 저장값 마이그레이션을 추가.
// - v1.1.0: DefaultProjectileData 직접 참조를 디버그 요약에 포함.
// - v1.0.0: WeaponData 최소 필드, 장착 타입/크기 호환성 검사, 디버그 요약 생성을 추가.
// Migration:
// - 기존 에셋의 CooldownSeconds 값은 로드 시 FireRatePerMinute로 환산한다.
// - 기존 FireOrigin / Dummy HitScan 흐름은 유지하고, ProjectileData가 비어 있으면 선택 데이터 미지정 상태로만 표시한다.
// - HitScan / Laser 피해도 WeaponData가 아니라 가상 ProjectileData의 DefaultDamageData에서 해석한다.

#include "CFWeaponData.h"

#include "CFProjectileData.h"

// [v1.0.0] 기본 무기 데이터 값을 초기화합니다.
UCFWeaponData::UCFWeaponData()
{
	CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
}

// [v1.2.0] 기존 CooldownSeconds 저장값을 FireRatePerMinute로 변환합니다.
void UCFWeaponData::PostLoad()
{
	Super::PostLoad();

	MigrateLegacyCooldownSeconds();
}

// [v1.2.0] 저장된 기존 CooldownSeconds 값을 FireRatePerMinute로 1회 변환합니다.
void UCFWeaponData::MigrateLegacyCooldownSeconds()
{
	if (bMigratedCooldownSecondsToFireRate || CooldownSeconds <= 0.0f)
	{
		return;
	}

	// [v1.2.0] 기존 초 단위 쿨다운을 분당 발사속도로 환산한 값입니다.
	const float MigratedFireRatePerMinute = 60.0f / CooldownSeconds;

	FireRatePerMinute = FMath::Max(MigratedFireRatePerMinute, 0.0f);
	CooldownSeconds = 0.0f;
	bMigratedCooldownSecondsToFireRate = true;
}

// [v1.0.0] 지정한 장착 타입을 이 무기가 지원하는지 반환합니다.
bool UCFWeaponData::SupportsMountType(const ECFVehicleMountType InMountType) const
{
	if (InMountType == ECFVehicleMountType::None)
	{
		return false;
	}

	return CompatibleMountTypes.Contains(InMountType);
}

// [v1.0.0] 지정한 장착 크기 제한 안에 이 무기가 들어가는지 반환합니다.
bool UCFWeaponData::SupportsWeaponSize(const ECFVehicleWeaponSize InMountSizeLimit) const
{
	if (WeaponSize == ECFVehicleWeaponSize::None || InMountSizeLimit == ECFVehicleWeaponSize::None)
	{
		return false;
	}

	// [v1.0.0] 이 무기의 크기 enum 값을 정수로 환산한 값입니다.
	const uint8 WeaponSizeValue = static_cast<uint8>(WeaponSize);

	// [v1.0.0] 장착 프로파일의 크기 제한 enum 값을 정수로 환산한 값입니다.
	const uint8 MountSizeLimitValue = static_cast<uint8>(InMountSizeLimit);

	return WeaponSizeValue <= MountSizeLimitValue;
}

// [v1.0.0] 지정한 장착 타입과 크기 제한에서 이 무기를 사용할 수 있는지 반환합니다.
bool UCFWeaponData::CanUseOnMount(const ECFVehicleMountType InMountType, const ECFVehicleWeaponSize InMountSizeLimit) const
{
	return SupportsMountType(InMountType) && SupportsWeaponSize(InMountSizeLimit);
}

// [v1.0.0] 디버그 패널에 표시할 무기 데이터 요약 문자열을 생성합니다.
FString UCFWeaponData::BuildWeaponSummary() const
{
	// [v1.0.0] 발사 모드 enum 값을 표시용 문자열로 변환한 값입니다.
	const FString FireModeText = UEnum::GetValueAsString(FireMode);

	// [v1.0.0] 무기 크기 enum 값을 표시용 문자열로 변환한 값입니다.
	const FString WeaponSizeText = UEnum::GetValueAsString(WeaponSize);

	// [v1.1.0] 직접 참조된 ProjectileData가 있으면 ProjectileId를 우선 표시하고, 없으면 기존 ID 필드를 표시합니다.
	const FString ProjectileDataText = DefaultProjectileData ? DefaultProjectileData->ProjectileId.ToString() : ProjectileDataId.ToString();

	// [v1.1.0] 직접 참조된 ProjectileData 에셋 이름입니다.
	const FString ProjectileAssetText = DefaultProjectileData ? DefaultProjectileData->GetName() : TEXT("MissingOptional");

	// [v1.2.0] 디버그 요약에 표시할 유효 분당 발사속도입니다.
	const float EffectiveFireRatePerMinute = GetEffectiveFireRatePerMinute();

	// [v1.2.0] 런타임 검증에 사용할 환산 발사 간격입니다.
	const float FireIntervalSeconds = GetFireIntervalSeconds();

	return FString::Printf(
		TEXT("WeaponData: Id=%s, Size=%s, FireMode=%s, FireRate=%.1fRPM, Interval=%.2fs, Range=%.1f, TargetPolicy=Categories:%d/Relations:%d/RequiredTags:%d/ExcludedTags:%d/TrackStates:%d, Projectile=%s, ProjectileAsset=%s, LegacyDamageProfile=%s, LegacyBaseDamage=%.1f"),
		*WeaponId.ToString(),
		*WeaponSizeText,
		*FireModeText,
		EffectiveFireRatePerMinute,
		FireIntervalSeconds,
		MaxRange,
		TargetUsePolicy.AllowedCategories.Num(),
		TargetUsePolicy.AllowedRelations.Num(),
		TargetUsePolicy.RequiredAttributeTags.Num(),
		TargetUsePolicy.ExcludedAttributeTags.Num(),
		TargetUsePolicy.AllowedTrackStates.Num(),
		*ProjectileDataText,
		*ProjectileAssetText,
		*DamageProfileId.ToString(),
		BaseDamage);
}

// [v1.2.0] 음수를 제거한 유효 분당 발사속도를 반환합니다.
float UCFWeaponData::GetEffectiveFireRatePerMinute() const
{
	return FMath::Max(FireRatePerMinute, 0.0f);
}

// [v1.2.0] 분당 발사속도를 실제 발사 간격 초로 환산합니다.
float UCFWeaponData::GetFireIntervalSeconds() const
{
	// [v1.2.0] 음수를 제거한 유효 분당 발사속도입니다.
	const float EffectiveFireRatePerMinute = GetEffectiveFireRatePerMinute();
	if (EffectiveFireRatePerMinute <= 0.0f)
	{
		return 0.0f;
	}

	return 60.0f / EffectiveFireRatePerMinute;
}
