// Copyright (c) CarFight. All Rights Reserved.
// File: CFWeaponGuideBackend.h
// Version: v1.0.0
// Date: 2026-10-06
// Description: CF-FQ-055 Weapon Guide의 existing typed durable backend를 CCAS Production Provisioning이 재사용하기 위한 Editor-private seam입니다.
// Changelog:
// - v1.0.0: Turret/Damage/Ammo/Projectile/Weapon desired fingerprint, persisted readback, Create/Update durable apply seam을 추가.
// Migration:
// - 새로운 package writer를 만들지 않습니다. 실제 mutation은 기존 CFDADurableCore 및 Damage/Ammo reviewed provider가 계속 소유합니다.

#pragma once

#include "CoreMinimal.h"
#include "WeaponAuthoring/CFWeaponGuideTypes.h"

namespace CFWeaponGuideBackend
{
	// TurretMount Draft + logical ID의 desired semantic fingerprint를 계산합니다.
	bool BuildTurretDesiredFingerprint(
		const FCFWeaponGuideDraft& Draft,
		FName TurretMountId,
		FString& OutFingerprint,
		FString& OutError);

	// Damage Draft + logical ID의 desired semantic fingerprint를 계산합니다.
	bool BuildDamageDesiredFingerprint(
		const FCFWeaponGuideDraft& Draft,
		FName DamageId,
		FString& OutFingerprint,
		FString& OutError);

	// Ammo Draft + logical ID의 desired semantic fingerprint를 계산합니다.
	bool BuildAmmoDesiredFingerprint(
		const FCFWeaponGuideDraft& Draft,
		FName AmmoId,
		FString& OutFingerprint,
		FString& OutError);

	// Projectile Draft + resolved Damage reference의 desired semantic fingerprint를 계산합니다.
	bool BuildProjectileDesiredFingerprint(
		const FCFWeaponGuideDraft& Draft,
		FName ProjectileId,
		const FSoftObjectPath& DamagePath,
		FString& OutFingerprint,
		FString& OutError);

	// Weapon Draft + resolved child references의 desired semantic fingerprint를 계산합니다.
	bool BuildWeaponDesiredFingerprint(
		const FCFWeaponGuideDraft& Draft,
		FName WeaponId,
		const FSoftObjectPath& ProjectilePath,
		const FSoftObjectPath& AmmoPath,
		FName ResolvedAmmoTypeId,
		FString& OutFingerprint,
		FString& OutError);

	// Persisted TurretMountData의 exact semantic fingerprint를 읽습니다.
	bool ReadTurretFingerprint(
		const FString& TargetObjectPath,
		bool& bOutExists,
		FString& OutFingerprint,
		FString& OutError);

	// Persisted DamageData의 exact semantic fingerprint를 읽습니다.
	bool ReadDamageFingerprint(
		const FString& TargetObjectPath,
		bool& bOutExists,
		FString& OutFingerprint,
		FString& OutError);

	// Persisted AmmoData의 exact semantic fingerprint를 읽습니다.
	bool ReadAmmoFingerprint(
		const FString& TargetObjectPath,
		bool& bOutExists,
		FString& OutFingerprint,
		FString& OutError);

	// Persisted ProjectileData의 exact semantic fingerprint를 읽습니다.
	bool ReadProjectileFingerprint(
		const FString& TargetObjectPath,
		bool& bOutExists,
		FString& OutFingerprint,
		FString& OutError);

	// Persisted WeaponData의 exact semantic fingerprint를 읽습니다.
	bool ReadWeaponFingerprint(
		const FString& TargetObjectPath,
		bool& bOutExists,
		FString& OutFingerprint,
		FString& OutError);

	// Existing Weapon Guide typed backend로 TurretMountData Create/Update를 durable 적용합니다.
	bool ApplyTurret(
		const FCFWeaponGuideDraft& Draft,
		FName TurretMountId,
		const FString& TargetObjectPath,
		const FString& ExpectedDesiredFingerprint,
		FString& OutPersistedFingerprint,
		FString& OutError);

	// Existing Damage provider로 DamageData Create/Update를 durable 적용합니다.
	bool ApplyDamage(
		const FCFWeaponGuideDraft& Draft,
		FName DamageId,
		const FString& TargetObjectPath,
		const FString& ExpectedDesiredFingerprint,
		FString& OutPersistedFingerprint,
		FString& OutError);

	// Existing Ammo provider로 AmmoData Create/Update를 durable 적용합니다.
	bool ApplyAmmo(
		const FCFWeaponGuideDraft& Draft,
		FName AmmoId,
		const FString& TargetObjectPath,
		const FString& ExpectedDesiredFingerprint,
		FString& OutPersistedFingerprint,
		FString& OutError);

	// Existing Weapon Guide typed backend로 ProjectileData Create/Update를 durable 적용합니다.
	bool ApplyProjectile(
		const FCFWeaponGuideDraft& Draft,
		FName ProjectileId,
		const FSoftObjectPath& DamagePath,
		const FString& TargetObjectPath,
		const FString& ExpectedDesiredFingerprint,
		FString& OutPersistedFingerprint,
		FString& OutError);

	// Existing Weapon Guide typed backend로 WeaponData Create/Update를 durable 적용합니다.
	bool ApplyWeapon(
		const FCFWeaponGuideDraft& Draft,
		FName WeaponId,
		const FSoftObjectPath& ProjectilePath,
		const FSoftObjectPath& AmmoPath,
		FName ResolvedAmmoTypeId,
		const FString& TargetObjectPath,
		const FString& ExpectedDesiredFingerprint,
		FString& OutPersistedFingerprint,
		FString& OutError);
}
