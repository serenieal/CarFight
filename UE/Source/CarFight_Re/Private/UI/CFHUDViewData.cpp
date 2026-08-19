// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-19
// Description: CF-FQ-032 UI-P0-06 Weapon Resource Channel additive projection + actual Charge/Heat channel
// Scope: 실제 Weapon HUD 필드를 공통 Resource Channel ViewData로 투영하며 raw 배열 순서를 Production Visual 순서로 사용하지 않습니다.
// Changelog:
// - v1.2.0: 실제 WeaponCharge Runtime ViewData가 Known/KnownZero일 때 Percent Charge 채널을 추가하고 충전 부족을 bBlocksFire로 전달. VehicleBattery는 계속 생성하지 않음.
// - v1.1.0: 실제 Weapon Heat Runtime ViewData가 Known/KnownZero일 때 Percent Heat 채널을 추가하고 Overheated를 bBlocksFire로 전달. Battery·Charge는 계속 생성하지 않음.
// - v1.0.0: Ammo, ReserveAmmo, Cooldown, Reload, LauncherSequence의 기존 실제 ViewData를 공통 채널로 투영. VehicleBattery, WeaponCharge, Heat는 실제 Runtime 부재로 생성하지 않음.
// Migration:
// - 기존 FCFWeaponHUDData 개별 필드는 현재 호환 계약으로 유지하며 ResourceChannels는 additive semantic projection입니다.
// - WeaponCharge 채널은 실제 WeaponChargeAvailability가 있을 때만 생성하며 VehicleBattery/정적 설정에서 현재값을 추정하지 않습니다.
// - Heat 채널은 실제 HeatAvailability가 있을 때만 생성하며 정적 WeaponData에서 현재값을 추정하지 않습니다.
// - VehicleBattery 채널은 실제 Gameplay Runtime Provider와 값 계약이 추가되기 전 생성하지 않습니다.

#include "UI/CFHUDViewData.h"

// [v1.0.0] 기존 실제 Weapon 필드를 공통 ResourceChannels 배열로 다시 투영합니다.
void FCFWeaponHUDData::RebuildResourceChannelsFromCurrentFields()
{
	ResourceChannels.Reset();

	// [v1.0.0] finite Ammo Runtime이 실제 현재값을 제공하는지 나타냅니다.
	const bool bAmmoAvailable = AmmoAvailability == ECFUIViewAvailability::Known
		|| AmmoAvailability == ECFUIViewAvailability::KnownZero;
	if (bAmmoAvailable)
	{
		// [v1.0.0] 실제 Loaded / MagazineCapacity를 전달할 Ammo 공통 채널입니다.
		FCFWeaponResourceHUDData AmmoChannel;
		AmmoChannel.ChannelType = ECFWeaponResourceChannelType::Ammo;
		AmmoChannel.DisplayMode = ECFWeaponResourceDisplayMode::CountPair;
		AmmoChannel.Availability = AmmoAvailability;
		AmmoChannel.bHasCurrentValue = true;
		AmmoChannel.CurrentValue = static_cast<float>(FMath::Max(LoadedAmmoCount, 0));
		AmmoChannel.bHasMaximumValue = true;
		AmmoChannel.MaximumValue = static_cast<float>(FMath::Max(MagazineCapacity, 0));
		AmmoChannel.bHasNormalizedValue = MagazineCapacity > 0;
		AmmoChannel.NormalizedValue = AmmoChannel.bHasNormalizedValue
			? FMath::Clamp(AmmoChannel.CurrentValue / AmmoChannel.MaximumValue, 0.0f, 1.0f)
			: 0.0f;
		AmmoChannel.bIsActive = true;
		AmmoChannel.bIsVisible = true;
		AmmoChannel.bBlocksFire = CurrentOnboardAmmoCount <= 0;
		ResourceChannels.Add(AmmoChannel);

		// [v1.0.0] 같은 AmmoId의 실제 차량 공유 예비량을 전달할 Reserve 공통 채널입니다.
		FCFWeaponResourceHUDData ReserveAmmoChannel;
		ReserveAmmoChannel.ChannelType = ECFWeaponResourceChannelType::ReserveAmmo;
		ReserveAmmoChannel.DisplayMode = ECFWeaponResourceDisplayMode::Count;
		ReserveAmmoChannel.Availability = ReserveAmmoCount > 0
			? ECFUIViewAvailability::Known
			: ECFUIViewAvailability::KnownZero;
		ReserveAmmoChannel.bHasCurrentValue = true;
		ReserveAmmoChannel.CurrentValue = static_cast<float>(FMath::Max(ReserveAmmoCount, 0));
		ReserveAmmoChannel.bIsActive = true;
		ReserveAmmoChannel.bIsVisible = true;
				ResourceChannels.Add(ReserveAmmoChannel);
	}

		// [v1.2.0] 실제 WeaponCharge Runtime이 현재값을 제공하는지 나타냅니다.
	const bool bWeaponChargeAvailable = MaximumWeaponCharge > KINDA_SMALL_NUMBER
		&& (WeaponChargeAvailability == ECFUIViewAvailability::Known
			|| WeaponChargeAvailability == ECFUIViewAvailability::KnownZero);
	if (bWeaponChargeAvailable)
	{
		// [v1.2.0] 실제 per-weapon Charge와 충전 부족 차단 상태를 전달하는 공통 Percent 채널입니다.
		FCFWeaponResourceHUDData WeaponChargeChannel;
		WeaponChargeChannel.ChannelType = ECFWeaponResourceChannelType::WeaponCharge;
		WeaponChargeChannel.DisplayMode = ECFWeaponResourceDisplayMode::Percent;
		WeaponChargeChannel.Availability = WeaponChargeAvailability;
		WeaponChargeChannel.bHasCurrentValue = true;
		WeaponChargeChannel.CurrentValue = FMath::Clamp(CurrentWeaponCharge, 0.0f, MaximumWeaponCharge);
		WeaponChargeChannel.bHasMaximumValue = true;
		WeaponChargeChannel.MaximumValue = FMath::Max(MaximumWeaponCharge, 0.0f);
		WeaponChargeChannel.bHasNormalizedValue = true;
		WeaponChargeChannel.NormalizedValue = FMath::Clamp(WeaponChargeRatio, 0.0f, 1.0f);
		WeaponChargeChannel.bIsActive = WeaponChargeChannel.CurrentValue + KINDA_SMALL_NUMBER < WeaponChargeChannel.MaximumValue;
		WeaponChargeChannel.bIsVisible = true;
		WeaponChargeChannel.bBlocksFire = bWeaponChargeInsufficient;
		ResourceChannels.Add(WeaponChargeChannel);
	}

	// [v1.1.0] 실제 Weapon Heat Runtime이 현재값을 제공하는지 나타냅니다.
	const bool bHeatAvailable = MaximumHeat > KINDA_SMALL_NUMBER
		&& (HeatAvailability == ECFUIViewAvailability::Known
			|| HeatAvailability == ECFUIViewAvailability::KnownZero);
	if (bHeatAvailable)
	{
		// [v1.1.0] 실제 per-weapon Heat와 과열 차단 상태를 전달하는 공통 Percent 채널입니다.
		FCFWeaponResourceHUDData HeatChannel;
		HeatChannel.ChannelType = ECFWeaponResourceChannelType::Heat;
		HeatChannel.DisplayMode = ECFWeaponResourceDisplayMode::Percent;
		HeatChannel.Availability = HeatAvailability;
		HeatChannel.bHasCurrentValue = true;
		HeatChannel.CurrentValue = FMath::Clamp(CurrentHeat, 0.0f, MaximumHeat);
		HeatChannel.bHasMaximumValue = true;
		HeatChannel.MaximumValue = FMath::Max(MaximumHeat, 0.0f);
		HeatChannel.bHasNormalizedValue = true;
		HeatChannel.NormalizedValue = FMath::Clamp(HeatRatio, 0.0f, 1.0f);
		HeatChannel.bIsActive = HeatChannel.CurrentValue > KINDA_SMALL_NUMBER;
		HeatChannel.bIsVisible = true;
		HeatChannel.bBlocksFire = bWeaponOverheated;
		ResourceChannels.Add(HeatChannel);
	}

	// [v1.0.0] 실제 Weapon Runtime이 유효한 발사 간격을 제공하는지 나타냅니다.
	const bool bCooldownAvailable = CooldownDurationSeconds > KINDA_SMALL_NUMBER
		&& (CooldownAvailability == ECFUIViewAvailability::Known
			|| CooldownAvailability == ECFUIViewAvailability::KnownZero);
	if (bCooldownAvailable)
	{
		// [v1.0.0] 실제 Weapon/Volley Cooldown의 남은 시간과 진행률을 전달할 공통 채널입니다.
		FCFWeaponResourceHUDData CooldownChannel;
		CooldownChannel.ChannelType = ECFWeaponResourceChannelType::Cooldown;
		CooldownChannel.DisplayMode = ECFWeaponResourceDisplayMode::TimeRemaining;
		CooldownChannel.Availability = CooldownAvailability;
		CooldownChannel.bHasCurrentValue = true;
		CooldownChannel.CurrentValue = FMath::Max(CooldownDurationSeconds - RemainingCooldownSeconds, 0.0f);
		CooldownChannel.bHasMaximumValue = true;
		CooldownChannel.MaximumValue = FMath::Max(CooldownDurationSeconds, 0.0f);
		CooldownChannel.bHasNormalizedValue = CooldownChannel.MaximumValue > KINDA_SMALL_NUMBER;
		CooldownChannel.NormalizedValue = CooldownChannel.bHasNormalizedValue
			? FMath::Clamp(CooldownChannel.CurrentValue / CooldownChannel.MaximumValue, 0.0f, 1.0f)
			: 0.0f;
		CooldownChannel.bHasRemainingTimeSeconds = true;
		CooldownChannel.RemainingTimeSeconds = FMath::Max(RemainingCooldownSeconds, 0.0f);
		CooldownChannel.bIsActive = CooldownChannel.RemainingTimeSeconds > KINDA_SMALL_NUMBER;
		CooldownChannel.bIsVisible = CooldownChannel.bIsActive;
		CooldownChannel.bBlocksFire = CooldownChannel.bIsActive;
		ResourceChannels.Add(CooldownChannel);
	}

	// [v1.0.0] 실제 Ammo Runtime이 현재 Reloading 상태인지 나타냅니다.
	const bool bReloadActive = bAmmoAvailable && ReloadState == ECFWeaponReloadState::Reloading;
	if (bAmmoAvailable)
	{
		// [v1.0.0] 실제 Ammo Reload 상태와 남은 시간을 전달할 공통 채널입니다.
		FCFWeaponResourceHUDData ReloadChannel;
		ReloadChannel.ChannelType = ECFWeaponResourceChannelType::Reload;
		ReloadChannel.DisplayMode = ECFWeaponResourceDisplayMode::TimeRemaining;
		ReloadChannel.Availability = bReloadActive
			? ECFUIViewAvailability::Known
			: ECFUIViewAvailability::KnownZero;
		ReloadChannel.bHasCurrentValue = ReloadDurationSeconds > KINDA_SMALL_NUMBER;
		ReloadChannel.CurrentValue = ReloadChannel.bHasCurrentValue
			? FMath::Max(ReloadDurationSeconds - RemainingReloadTimeSeconds, 0.0f)
			: 0.0f;
		ReloadChannel.bHasMaximumValue = ReloadDurationSeconds > KINDA_SMALL_NUMBER;
		ReloadChannel.MaximumValue = FMath::Max(ReloadDurationSeconds, 0.0f);
		ReloadChannel.bHasNormalizedValue = ReloadChannel.bHasMaximumValue;
		ReloadChannel.NormalizedValue = ReloadChannel.bHasNormalizedValue
			? FMath::Clamp(ReloadChannel.CurrentValue / ReloadChannel.MaximumValue, 0.0f, 1.0f)
			: 0.0f;
		ReloadChannel.bHasRemainingTimeSeconds = bReloadActive;
		ReloadChannel.RemainingTimeSeconds = bReloadActive ? FMath::Max(RemainingReloadTimeSeconds, 0.0f) : 0.0f;
		ReloadChannel.bIsActive = bReloadActive;
		ReloadChannel.bIsVisible = bReloadActive;
		ReloadChannel.bBlocksFire = bReloadActive
			&& bWeaponActionLocked
			&& WeaponActionLockReason == ECFWeaponActionLockReason::Reloading;
		ResourceChannels.Add(ReloadChannel);
	}

	// [v1.0.0] Launcher Runtime 채널 자체가 실제로 존재하는지 나타냅니다.
	const bool bLauncherAvailable = LauncherAvailability == ECFUIViewAvailability::Known
		|| LauncherAvailability == ECFUIViewAvailability::KnownZero;
	if (bLauncherAvailable)
	{
		// [v1.0.0] 실제 Launcher Sequence 승인 수와 전체 수를 전달할 공통 채널입니다.
		FCFWeaponResourceHUDData LauncherSequenceChannel;
		LauncherSequenceChannel.ChannelType = ECFWeaponResourceChannelType::LauncherSequence;
		LauncherSequenceChannel.DisplayMode = ECFWeaponResourceDisplayMode::Sequence;
		LauncherSequenceChannel.Availability = LauncherAvailability;
		LauncherSequenceChannel.bHasCurrentValue = LauncherTotalProjectileCount > 0;
		LauncherSequenceChannel.CurrentValue = static_cast<float>(FMath::Max(LauncherAcceptedProjectileCount, 0));
		LauncherSequenceChannel.bHasMaximumValue = LauncherTotalProjectileCount > 0;
		LauncherSequenceChannel.MaximumValue = static_cast<float>(FMath::Max(LauncherTotalProjectileCount, 0));
		LauncherSequenceChannel.bHasNormalizedValue = LauncherSequenceChannel.bHasMaximumValue;
		LauncherSequenceChannel.NormalizedValue = LauncherSequenceChannel.bHasNormalizedValue
			? FMath::Clamp(LauncherSequenceChannel.CurrentValue / LauncherSequenceChannel.MaximumValue, 0.0f, 1.0f)
			: 0.0f;
		LauncherSequenceChannel.bIsActive = bLauncherSequenceActive;
		LauncherSequenceChannel.bIsVisible = bLauncherSequenceActive;
		LauncherSequenceChannel.bBlocksFire = bLauncherSequenceActive
			&& bWeaponActionLocked
			&& WeaponActionLockReason == ECFWeaponActionLockReason::LauncherSequenceActive;
		ResourceChannels.Add(LauncherSequenceChannel);
	}
}
