// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.13.0
// Date: 2026-08-19
// Description: CarFight 차량 무기 DataAsset·피팅 질량·Charge·Heat P0 입력과 CF-FQ-008 정적 DataValidation 구현
// Scope: 차량 장착 프로파일 호환성, 피팅 질량, 런처 발사 패턴·Release 설정, 탄약·Charge·Heat 정적 설정과 선택 대상 사용 정책 요약을 제공합니다.
// Changelog:
// - v1.13.0: UI-P0-06 WeaponCharge explicit opt-in 판정과 정적 범위 검증을 추가. all-zero/incomplete zero 조합은 Disabled 호환 상태로 허용.
// - v1.12.0: UI-P0-06 Heat Runtime 활성 조건과 HeatDissipationPerSecond 정적 검증을 추가. 불완전한 기존 Heat 설정은 Disabled 호환 상태로 허용.
// - v1.11.0: CF-FQ-008 WD-P0-01 WeaponData 정적 계약 검증과 Unreal Data Validation 구현. Runtime fallback과 기존 에셋 값은 변경하지 않음.
// - v1.10.0: CF-FQ-031 AMMO-P0-01 탄창·초기 장전·발사당 탄약·Reload 유효값과 유한탄 호환 판정·요약을 추가.
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
// - v1.13.0 WeaponCharge는 Maximum/PerShot/Recovery가 양수이고 Initial이 0~Maximum 범위일 때만 활성화한다. 기존 all-zero Asset은 Disabled이며 발사를 제한하지 않는다.
// - v1.12.0 HeatPerShot, MaxHeat, HeatDissipationPerSecond가 모두 양수일 때만 Heat Runtime을 활성화한다. 일부 값만 0인 기존/예약 설정은 Invalid로 만들지 않는다.
// - v1.11.0 DataValidation은 음수·비유한 값, 식별/장착 모순과 finite-ammo 정적 불완전성만 Invalid로 보고한다. 기존 안전 fallback 값은 허용한다.
// - 기존 WeaponData는 bUseInfiniteAmmoForDebug=true와 DefaultAmmoData=None 기본값으로 신규 탄약 Runtime이 발사를 제한하지 않는다.
// - 기존 WeaponData는 WeaponMassKg=0 기본값으로 기존 발사와 차량 주행 결과를 유지한다.
// - 기존 WeaponData는 SingleCycle / 1발 기본값으로 기존 발사 결과를 유지하며 Ripple·Salvo 실행은 아직 시작하지 않는다.
// - 기존 에셋의 CooldownSeconds 값은 로드 시 FireRatePerMinute로 환산한다.
// - 기존 FireOrigin / Dummy HitScan 흐름은 유지하고, ProjectileData가 비어 있으면 선택 데이터 미지정 상태로만 표시한다.
// - HitScan / Laser 피해도 WeaponData가 아니라 가상 ProjectileData의 DefaultDamageData에서 해석한다.

#include "CFWeaponData.h"

#include "CFAmmoData.h"
#include "CFProjectileData.h"

#define LOCTEXT_NAMESPACE "CFWeaponData"

// [v1.10.0] 기존 MagazineSize를 신규 Runtime의 MagazineCapacity 의미로 안전하게 보정해 반환합니다.
int32 UCFWeaponData::GetEffectiveMagazineCapacity() const
{
	return FMath::Max(MagazineSize, 0);
}

// [v1.10.0] 출격 초기 장전량을 유효 탄창 용량 범위로 보정해 반환합니다.
int32 UCFWeaponData::GetEffectiveInitialLoadedAmmoCount() const
{
	return FMath::Clamp(InitialLoadedAmmoCount, 0, GetEffectiveMagazineCapacity());
}

// [v1.10.0] 한 번의 정상 발사에 필요한 탄약 단위를 최소 1로 보정해 반환합니다.
int32 UCFWeaponData::GetEffectiveAmmoUnitsPerShot() const
{
	return FMath::Max(AmmoUnitsPerShot, 1);
}

// [v1.10.0] 기존 ReloadTimeSeconds를 0 이상의 유한한 재장전 시간으로 보정해 반환합니다.
float UCFWeaponData::GetEffectiveReloadTimeSeconds() const
{
	return FMath::IsFinite(ReloadTimeSeconds) ? FMath::Max(ReloadTimeSeconds, 0.0f) : 0.0f;
}

// [v1.10.0] 이 WeaponData가 명시적인 유한 탄약 Runtime을 사용하도록 설정됐는지 반환합니다.
bool UCFWeaponData::UsesFiniteAmmoRuntime() const
{
	return !bUseInfiniteAmmoForDebug
		&& IsValid(DefaultAmmoData)
		&& DefaultAmmoData->IsAmmoDataValid()
		&& GetEffectiveMagazineCapacity() > 0;
}

// [v1.13.0] 네 explicit Charge 입력이 유효해 실제 무기 내부 Charge Runtime을 사용할지 반환합니다.
bool UCFWeaponData::UsesWeaponChargeRuntime() const
{
	return FMath::IsFinite(MaximumWeaponCharge)
		&& FMath::IsFinite(InitialWeaponCharge)
		&& FMath::IsFinite(WeaponChargePerShot)
		&& FMath::IsFinite(WeaponChargeRecoveryPerSecond)
		&& MaximumWeaponCharge > KINDA_SMALL_NUMBER
		&& InitialWeaponCharge >= 0.0f
		&& InitialWeaponCharge <= MaximumWeaponCharge + KINDA_SMALL_NUMBER
		&& WeaponChargePerShot > KINDA_SMALL_NUMBER
		&& WeaponChargePerShot <= MaximumWeaponCharge + KINDA_SMALL_NUMBER
		&& WeaponChargeRecoveryPerSecond > KINDA_SMALL_NUMBER;
}

// [v1.12.0] 세 explicit Heat 입력이 모두 유효해 실제 무기 Heat Runtime을 사용할지 반환합니다.
bool UCFWeaponData::UsesWeaponHeatRuntime() const
{
	return FMath::IsFinite(HeatPerShot)
		&& FMath::IsFinite(MaxHeat)
		&& FMath::IsFinite(HeatDissipationPerSecond)
		&& HeatPerShot > KINDA_SMALL_NUMBER
		&& MaxHeat > KINDA_SMALL_NUMBER
		&& HeatDissipationPerSecond > KINDA_SMALL_NUMBER;
}

// [v1.10.0] 로그와 Debug에서 사용할 탄약 정적 설정 요약을 반환합니다.
FString UCFWeaponData::BuildAmmoConfigSummary() const
{
	return FString::Printf(
		TEXT("WeaponAmmoConfig: Mode=%s, Ammo=%s, Magazine=%d, InitialLoaded=%d, UnitsPerShot=%d, Reload=%.2fs, ReloadMode=%s, AutoReload=%s, PartialReload=%s, PartialSequence=%s"),
		UsesFiniteAmmoRuntime() ? TEXT("Finite") : TEXT("InfiniteCompatibility"),
		DefaultAmmoData ? *DefaultAmmoData->AmmoId.ToString() : TEXT("None"),
		GetEffectiveMagazineCapacity(),
		GetEffectiveInitialLoadedAmmoCount(),
		GetEffectiveAmmoUnitsPerShot(),
		GetEffectiveReloadTimeSeconds(),
		*UEnum::GetValueAsString(ReloadMode),
		bAutoReloadWhenEmpty ? TEXT("Yes") : TEXT("No"),
		bAllowPartialReload ? TEXT("Yes") : TEXT("No"),
		bAllowPartialSequence ? TEXT("Yes") : TEXT("No"));
}

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

// [v1.11.0] DataValidation과 Automation이 공유할 WeaponData 정적 계약 오류 목록을 생성합니다.
bool UCFWeaponData::ValidateWeaponDataContract(TArray<FText>& OutValidationErrors) const
{
	OutValidationErrors.Reset();

	if (WeaponId.IsNone())
	{
		OutValidationErrors.Add(LOCTEXT("MissingWeaponId", "WeaponId는 None일 수 없습니다."));
	}

	if (WeaponSize == ECFVehicleWeaponSize::None)
	{
		OutValidationErrors.Add(LOCTEXT("MissingWeaponSize", "WeaponSize는 None일 수 없습니다."));
	}

	if (CompatibleMountTypes.IsEmpty())
	{
		OutValidationErrors.Add(LOCTEXT("EmptyCompatibleMountTypes", "CompatibleMountTypes에는 최소 하나의 유효한 장착 타입이 필요합니다."));
	}

	// [v1.11.0] 중복 장착 타입을 검출하기 위해 이미 확인한 유효 장착 타입 집합입니다.
	TSet<ECFVehicleMountType> SeenMountTypes;
	for (const ECFVehicleMountType MountType : CompatibleMountTypes)
	{
		if (MountType == ECFVehicleMountType::None)
		{
			OutValidationErrors.Add(LOCTEXT("InvalidCompatibleMountTypeNone", "CompatibleMountTypes에는 None을 넣을 수 없습니다."));
			continue;
		}

		if (SeenMountTypes.Contains(MountType))
		{
			OutValidationErrors.Add(LOCTEXT("DuplicateCompatibleMountType", "CompatibleMountTypes에는 같은 장착 타입을 중복으로 넣을 수 없습니다."));
			continue;
		}

		SeenMountTypes.Add(MountType);
	}

	if (!FMath::IsFinite(WeaponMassKg) || WeaponMassKg < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidWeaponMass", "WeaponMassKg는 유한한 0 이상의 값이어야 합니다."));
	}

	if (!FMath::IsFinite(FireRatePerMinute) || FireRatePerMinute < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidFireRate", "FireRatePerMinute는 유한한 0 이상의 값이어야 합니다. 0은 쿨다운 없는 현재 fallback으로 허용됩니다."));
	}

	if (!FMath::IsFinite(MaxRange) || MaxRange < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidMaxRange", "MaxRange는 유한한 0 이상의 값이어야 합니다. 0은 현재 사거리 fallback으로 허용됩니다."));
	}

	if (!FMath::IsFinite(SpreadDeg) || SpreadDeg < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidSpread", "SpreadDeg는 유한한 0 이상의 값이어야 합니다."));
	}

	if (MagazineSize < 0)
	{
		OutValidationErrors.Add(LOCTEXT("NegativeMagazineSize", "MagazineSize는 0 이상이어야 합니다."));
	}

	if (!FMath::IsFinite(ReloadTimeSeconds) || ReloadTimeSeconds < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidReloadTime", "ReloadTimeSeconds는 유한한 0 이상의 값이어야 합니다."));
	}

	if (InitialLoadedAmmoCount < 0)
	{
		OutValidationErrors.Add(LOCTEXT("NegativeInitialLoadedAmmoCount", "InitialLoadedAmmoCount는 0 이상이어야 합니다."));
	}
	else if (MagazineSize >= 0 && InitialLoadedAmmoCount > MagazineSize)
	{
		OutValidationErrors.Add(LOCTEXT("InitialLoadedExceedsMagazine", "InitialLoadedAmmoCount는 MagazineSize를 넘을 수 없습니다."));
	}

	if (AmmoUnitsPerShot < 1)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidAmmoUnitsPerShot", "AmmoUnitsPerShot는 최소 1이어야 합니다."));
	}

	// [v1.11.0] 명시된 DefaultAmmoData가 안정 AmmoId를 가진 유효 정적 탄종인지 여부입니다.
	const bool bHasValidDefaultAmmoData = IsValid(DefaultAmmoData) && DefaultAmmoData->IsAmmoDataValid();
	if (DefaultAmmoData && !bHasValidDefaultAmmoData)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidDefaultAmmoData", "DefaultAmmoData가 지정되면 유효한 AmmoId를 가진 AmmoData여야 합니다."));
	}

	if (!bUseInfiniteAmmoForDebug)
	{
		if (!bHasValidDefaultAmmoData)
		{
			OutValidationErrors.Add(LOCTEXT("FiniteAmmoMissingDefaultAmmo", "유한 탄약 Runtime을 사용하려면 유효한 DefaultAmmoData가 필요합니다."));
		}

		if (MagazineSize <= 0)
		{
			OutValidationErrors.Add(LOCTEXT("FiniteAmmoMissingMagazine", "유한 탄약 Runtime을 사용하려면 MagazineSize가 0보다 커야 합니다."));
		}
	}

		if (!FMath::IsFinite(MaximumWeaponCharge) || MaximumWeaponCharge < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidMaximumWeaponCharge", "MaximumWeaponCharge는 유한한 0 이상의 값이어야 합니다."));
	}

	if (!FMath::IsFinite(InitialWeaponCharge) || InitialWeaponCharge < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidInitialWeaponCharge", "InitialWeaponCharge는 유한한 0 이상의 값이어야 합니다."));
	}
	else if (FMath::IsFinite(MaximumWeaponCharge)
		&& MaximumWeaponCharge >= 0.0f
		&& InitialWeaponCharge > MaximumWeaponCharge + KINDA_SMALL_NUMBER)
	{
		OutValidationErrors.Add(LOCTEXT("InitialWeaponChargeExceedsMaximum", "InitialWeaponCharge는 MaximumWeaponCharge를 넘을 수 없습니다."));
	}

	if (!FMath::IsFinite(WeaponChargePerShot) || WeaponChargePerShot < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidWeaponChargePerShot", "WeaponChargePerShot은 유한한 0 이상의 값이어야 합니다."));
	}
	else if (FMath::IsFinite(MaximumWeaponCharge)
		&& MaximumWeaponCharge >= 0.0f
		&& WeaponChargePerShot > MaximumWeaponCharge + KINDA_SMALL_NUMBER)
	{
		OutValidationErrors.Add(LOCTEXT("WeaponChargePerShotExceedsMaximum", "WeaponChargePerShot은 MaximumWeaponCharge를 넘을 수 없습니다."));
	}

	if (!FMath::IsFinite(WeaponChargeRecoveryPerSecond) || WeaponChargeRecoveryPerSecond < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidWeaponChargeRecovery", "WeaponChargeRecoveryPerSecond는 유한한 0 이상의 값이어야 합니다."));
	}

	if (!FMath::IsFinite(HeatPerShot) || HeatPerShot < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidHeatPerShot", "HeatPerShot은 유한한 0 이상의 값이어야 합니다."));
	}

	if (!FMath::IsFinite(MaxHeat) || MaxHeat < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidMaxHeat", "MaxHeat는 유한한 0 이상의 값이어야 합니다."));
	}

	if (!FMath::IsFinite(HeatDissipationPerSecond) || HeatDissipationPerSecond < 0.0f)
	{
		OutValidationErrors.Add(LOCTEXT("InvalidHeatDissipationPerSecond", "HeatDissipationPerSecond는 유한한 0 이상의 값이어야 합니다."));
	}

	return OutValidationErrors.IsEmpty();
}

#if WITH_EDITOR
// [v1.11.0] Unreal Data Validation에서 잘못된 WeaponData 정적 설정을 보고합니다.
EDataValidationResult UCFWeaponData::IsDataValid(FDataValidationContext& Context) const
{
	// [v1.11.0] 상위 PrimaryDataAsset이 반환한 기본 검증 결과입니다.
	EDataValidationResult ValidationResult = Super::IsDataValid(Context);

	// [v1.11.0] WeaponData 정적 계약에서 발견된 오류 목록입니다.
	TArray<FText> ValidationErrors;
	if (!ValidateWeaponDataContract(ValidationErrors))
	{
		for (const FText& ValidationError : ValidationErrors)
		{
			Context.AddError(ValidationError);
		}

		return EDataValidationResult::Invalid;
	}

	if (ValidationResult == EDataValidationResult::NotValidated)
	{
		ValidationResult = EDataValidationResult::Valid;
	}

	return ValidationResult;
}
#endif

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

#undef LOCTEXT_NAMESPACE
