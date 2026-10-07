// Copyright (c) CarFight. All Rights Reserved.
// File: CFProdTargetBindings.h
// Version: v1.0.0
// Date: 2026-10-06
// Description: CCAS Production target contract를 existing typed Weapon/Equipment durable backend callback에 결속하는 concrete adapter입니다.
// Changelog:
// - v1.0.0: exact target binding registry와 Turret/Damage/Ammo/Projectile/Weapon/EquipmentPreset backend binding helpers를 추가.
// Migration:
// - Product orchestrator는 payload를 해석하지 않습니다. 실제 Create/Update/readback은 기존 Weapon Guide/Equipment Builder backend가 계속 소유합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFProdProvisioning.h"
#include "EquipmentAuthoring/CFEquipmentBuilderTypes.h"
#include "WeaponAuthoring/CFWeaponGuideTypes.h"

/** One Production target에 결속된 existing typed backend callback 집합입니다. */
struct FCFProdTargetBinding
{
	// Binding이 exact match해야 하는 reviewed Product target contract입니다.
	FCFProdProvisionTarget Target;

	// Existing typed backend의 persisted fingerprint read callback입니다.
	TFunction<bool(bool& bOutExists, FString& OutFingerprint, FString& OutError)> ReadPersistedFingerprint;

	// Existing typed backend의 reviewed durable apply + post-readback callback입니다.
	TFunction<bool(FString& OutPersistedFingerprint, FString& OutError)> ApplyReviewedTarget;
};

/** Product orchestrator가 사용하는 concrete Production typed target adapter입니다. */
class FCFProdBoundTargetAdapter final : public ICFProdTypedTargetAdapter
{
public:
	// Exact target binding 하나를 중복 없이 등록합니다.
	bool RegisterBinding(
		const FCFProdTargetBinding& Binding,
		FString& OutError);

	// 등록된 exact ContentKey 수를 반환합니다.
	int32 GetBindingCount() const { return BindingsByStableKey.Num(); }

	// Exact target contract와 결속된 existing backend에서 current persisted fingerprint를 읽습니다.
	virtual bool ReadPersistedFingerprint(
		const FCFProdProvisionTarget& Target,
		bool& bOutExists,
		FString& OutFingerprint,
		FString& OutError) const override;

	// Exact target contract와 결속된 existing backend에 reviewed durable apply를 위임합니다.
	virtual bool ApplyReviewedTarget(
		const FCFProdProvisionTarget& Target,
		FString& OutPersistedFingerprint,
		FString& OutError) override;

private:
	// ContentKey stable string -> exact backend binding입니다.
	TMap<FString, FCFProdTargetBinding> BindingsByStableKey;
};

/** Frozen Production typed target를 기존 Weapon Guide / Equipment Builder backend에 연결합니다. */
class FCFProdTargetBindings
{
public:
	// TurretMount Production target을 existing Weapon Guide durable backend에 결속합니다.
	static bool BindTurret(
		FCFProdBoundTargetAdapter& Adapter,
		const FCFProdProvisionTarget& Target,
		const FCFWeaponGuideDraft& Draft,
		FString& OutError);

	// Damage Production target을 existing Damage reviewed provider에 결속합니다.
	static bool BindDamage(
		FCFProdBoundTargetAdapter& Adapter,
		const FCFProdProvisionTarget& Target,
		const FCFWeaponGuideDraft& Draft,
		FString& OutError);

	// Ammo Production target을 existing Ammo reviewed provider에 결속합니다.
	static bool BindAmmo(
		FCFProdBoundTargetAdapter& Adapter,
		const FCFProdProvisionTarget& Target,
		const FCFWeaponGuideDraft& Draft,
		FString& OutError);

	// Projectile Production target을 existing Weapon Guide durable backend에 결속합니다.
	static bool BindProjectile(
		FCFProdBoundTargetAdapter& Adapter,
		const FCFProdProvisionTarget& Target,
		const FCFWeaponGuideDraft& Draft,
		const FString& DamageObjectPath,
		FString& OutError);

	// Weapon Production target을 existing Weapon Guide durable backend에 결속합니다.
	static bool BindWeapon(
		FCFProdBoundTargetAdapter& Adapter,
		const FCFProdProvisionTarget& Target,
		const FCFWeaponGuideDraft& Draft,
		const FString& ProjectileObjectPath,
		const FString& AmmoObjectPath,
		FName ResolvedAmmoTypeId,
		FString& OutError);

	// EquipmentPreset Production target을 existing Equipment Builder reviewed durable path에 결속합니다.
	static bool BindEquipmentPreset(
		FCFProdBoundTargetAdapter& Adapter,
		const FCFProdProvisionTarget& Target,
		const FCFEquipmentPresetDraft& Draft,
		FString& OutError);
};
