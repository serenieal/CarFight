// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.8.0
// Date: 2026-08-01
// Description: CarFight 차량 무기 DataAsset과 피팅 무기 질량 구현
// Scope: 차량 장착 프로파일 호환성, 피팅 질량, 런처 발사 패턴·Release 설정과 선택 대상 사용 정책 요약을 제공합니다.
// Changelog:
// - v1.8.0: CF-FQ-034 FIT-P0-02 유효 무기 질량 Getter와 통합 요약 출력을 추가.
// - v1.7.0: 런처 Release 유효값 보정, 전용 요약과 WeaponData 통합 요약을 추가.
// - v1.6.0: 런처 발사 패턴 유효값 보정, 전용 요약과 WeaponData 통합 요약을 추가.
// - v1.5.0: TS-P0-07 TargetUsePolicy 필터 개수를 무기 디버그 요약에 추가.
// - v1.4.0: WeaponData 직접 DamageData 요약을 제거하고 ProjectileData 단일 피해 소유 구조로 정리.
// - v1.3.0: DefaultDamageData 직접 참조를 디버그 요약에 포함.
// - v1.2.0: 분당 발사속도 기반 발사 간격 환산과 기존 CooldownSeconds 저장값 마이그레이션을 추가.
// - v1.1.0: DefaultProjectileData 직접 참조를 디버그 요약에 포함.
// - v1.0.0: WeaponData 최소 필드, 장착 타입/크기 호환성 검사, 디버그 요약 생성을 추가.
// Migration:
// - 기존 WeaponData는 WeaponMassKg=0 기본값으로 기존 발사와 차량 주행 결과를 유지한다.
// - 기존 WeaponData는 SingleCycle / 1발 기본값으로 기존 발사 결과를 유지하며 Ripple·Salvo 실행은 아직 시작하지 않는다.
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

// [v1.8.0] 음수나 비유한 값을 제거한 피팅용 유효 무기 질량을 반환합니다.
float UCFWeaponData::GetEffectiveWeaponMassKg() const
{
	return FMath::IsFinite(WeaponMassKg) ? FMath::Max(WeaponMassKg, 0.0f) : 0.0f;
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

	// [v1.6.0] 디버그 요약에 포함할 안전한 런처 발사 패턴 설정입니다.
	const FCFLauncherFirePatternConfig EffectiveLauncherConfig = GetEffectiveLauncherFirePatternConfig();

		// [v1.6.0] 런처 발사 패턴 enum 값을 표시용 문자열로 변환한 값입니다.
	const FString LauncherPatternText = UEnum::GetValueAsString(EffectiveLauncherConfig.FirePattern);

	// [v1.7.0] 현재 무기와 ProjectileData 기준으로 보정한 런처 Release 설정입니다.
	const FCFLauncherReleaseConfig EffectiveReleaseConfig = GetEffectiveLauncherReleaseConfig();

	// [v1.7.0] 런처 Release Mode를 표시용 문자열로 변환한 값입니다.
	const FString LauncherReleaseModeText = UEnum::GetValueAsString(EffectiveReleaseConfig.ReleaseMode);

	return FString::Printf(
				TEXT("WeaponData: Id=%s, Size=%s, Mass=%.1fkg, FireMode=%s, FireRate=%.1fRPM, Interval=%.2fs, LauncherPattern=%s, LauncherProjectiles=%d, RippleInterval=%.3fs, SalvoSimultaneous=%d, ReleaseMode=%s, EjectionSpeed=%.1f, CarrierRatio=%.2f, Clearance=%.1f, Range=%.1f, TargetPolicy=Categories:%d/Relations:%d/RequiredTags:%d/ExcludedTags:%d/TrackStates:%d, Projectile=%s, ProjectileAsset=%s, LegacyDamageProfile=%s, LegacyBaseDamage=%.1f"),
		*WeaponId.ToString(),
		*WeaponSizeText,
		GetEffectiveWeaponMassKg(),
		*FireModeText,
		EffectiveFireRatePerMinute,
						FireIntervalSeconds,
		*LauncherPatternText,
		EffectiveLauncherConfig.GetEffectiveProjectileCount(),
		EffectiveLauncherConfig.GetEffectiveInterMuzzleDelaySeconds(),
				EffectiveLauncherConfig.GetEffectiveMaximumSimultaneousLaunchCount(),
		*LauncherReleaseModeText,
		EffectiveReleaseConfig.EjectionSpeed,
		EffectiveReleaseConfig.CarrierVelocityRatio,
		EffectiveReleaseConfig.LauncherClearanceTraceDistanceCm,
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

// [v1.6.0] 음수·0·패턴 비적용 값을 안전하게 보정한 런처 발사 패턴 설정을 반환합니다.
FCFLauncherFirePatternConfig UCFWeaponData::GetEffectiveLauncherFirePatternConfig() const
{
	FCFLauncherFirePatternConfig EffectiveConfig = LauncherFirePatternConfig;
	EffectiveConfig.ProjectileCountPerTrigger = EffectiveConfig.GetEffectiveProjectileCount();
	EffectiveConfig.InterMuzzleDelaySeconds = EffectiveConfig.GetEffectiveInterMuzzleDelaySeconds();
	EffectiveConfig.MaximumSimultaneousLaunchCount = EffectiveConfig.GetEffectiveMaximumSimultaneousLaunchCount();
	return EffectiveConfig;
}

// [v1.6.0] 현재 런처 발사 패턴의 유효 수량·간격·실패·쿨다운 정책을 한 줄로 반환합니다.
FString UCFWeaponData::BuildLauncherFirePatternSummary() const
{
	// [v1.6.0] 원본 DataAsset 값을 안전한 런타임 값으로 보정한 발사 패턴 설정입니다.
	const FCFLauncherFirePatternConfig EffectiveConfig = GetEffectiveLauncherFirePatternConfig();

	return FString::Printf(
		TEXT("LauncherFirePattern: Pattern=%s, Projectiles=%d, RippleInterval=%.3fs, SalvoSimultaneous=%d, FailurePolicy=%s, CooldownStart=%s"),
		*UEnum::GetValueAsString(EffectiveConfig.FirePattern),
		EffectiveConfig.GetEffectiveProjectileCount(),
		EffectiveConfig.GetEffectiveInterMuzzleDelaySeconds(),
		EffectiveConfig.GetEffectiveMaximumSimultaneousLaunchCount(),
		*UEnum::GetValueAsString(EffectiveConfig.SequenceFailurePolicy),
		*UEnum::GetValueAsString(EffectiveConfig.CooldownStartPolicy));
}

// [v1.7.0] 현재 무기·ProjectileData 기준으로 안전하게 보정된 런처 Release 설정을 반환합니다.
FCFLauncherReleaseConfig UCFWeaponData::GetEffectiveLauncherReleaseConfig() const
{
	FCFLauncherReleaseConfig EffectiveConfig = LauncherReleaseConfig;

	// [v1.7.0] HitScan은 Projectile 분리 단계가 없으므로 저장된 비Direct 값을 런타임에 적용하지 않습니다.
	if (FireMode != ECFWeaponFireMode::Projectile)
	{
		EffectiveConfig.ReleaseMode = ECFProjectileReleaseMode::Direct;
		EffectiveConfig.CarrierVelocityRatio = 0.0f;
	}

	// [v1.7.0] EjectionSpeed가 비어 있을 때 사용할 ProjectileData 초기 속력입니다.
	const float FallbackInitialSpeed = DefaultProjectileData
		? FMath::Max(DefaultProjectileData->InitialSpeed, 1.0f)
		: 1.0f;

	EffectiveConfig.LocalEjectionDirection = EffectiveConfig.GetEffectiveLocalEjectionDirection();
	EffectiveConfig.EjectionSpeed = EffectiveConfig.GetEffectiveReleaseSpeed(FallbackInitialSpeed);
	EffectiveConfig.CarrierVelocityRatio = EffectiveConfig.GetEffectiveCarrierVelocityRatio();
	EffectiveConfig.LauncherClearanceTraceDistanceCm = EffectiveConfig.GetEffectiveLauncherClearanceTraceDistanceCm();
	return EffectiveConfig;
}

// [v1.7.0] 현재 런처 Release 모드와 사출·차량 속도 상속 설정을 한 줄로 반환합니다.
FString UCFWeaponData::BuildLauncherReleaseSummary() const
{
	const FCFLauncherReleaseConfig EffectiveConfig = GetEffectiveLauncherReleaseConfig();
	return FString::Printf(
		TEXT("LauncherRelease: Mode=%s, LocalDirection=(%.3f, %.3f, %.3f), Speed=%.1fcm/s, CarrierRatio=%.2f, Clearance=%.1fcm"),
		*UEnum::GetValueAsString(EffectiveConfig.ReleaseMode),
		EffectiveConfig.LocalEjectionDirection.X,
		EffectiveConfig.LocalEjectionDirection.Y,
		EffectiveConfig.LocalEjectionDirection.Z,
		EffectiveConfig.EjectionSpeed,
		EffectiveConfig.CarrierVelocityRatio,
		EffectiveConfig.LauncherClearanceTraceDistanceCm);
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
