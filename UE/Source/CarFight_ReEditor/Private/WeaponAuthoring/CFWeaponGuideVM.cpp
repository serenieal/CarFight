// Copyright (c) CarFight. All Rights Reserved.
// File: CFWeaponGuideVM.cpp
// Version: v1.4.1
// Date: 2026-09-19
// Description: CF-FQ-055 Weapon Equipment Authoring Guide의 durable exact5 graph, completion semantic integrity와 navigation-only Equipment Builder handoff 구현입니다.
// Changelog:
// - v1.4.1: WEA-P0-04 Mid-review P1 교정으로 P0-03 Bundle fingerprint를 보존한 별도 CompletionSemanticFingerprint와 persisted exact5 semantic revalidation을 추가했습니다.
// - v1.4.0: WEA-P0-04 completion Bundle integrity 재검증, USER completion summary와 context injection 없는 Equipment Builder navigation contract를 추가했습니다.
// - v1.3.0: WEA-P0-03 Damage/Ammo existing provider durable Create, exact5 fresh preflight, session partial-durable recovery와 persisted reference graph readback을 추가했습니다.
// - v1.2.1: WEA-P0-02 신규 Damage/Ammo Draft가 포함된 durable entry는 다른 incomplete Step validation보다 먼저 P0-03 boundary로 fail-visible하도록 순서를 교정했습니다.
// - v1.2.0: WEA-P0-02 신규 Damage/Ammo Draft를 existing typed provider의 ReviewedMutationReady payload/fingerprint/serializer/parser 계약에 mutation0으로 연결하고 P0-03 durable boundary를 명시했습니다.
// - v1.1.0: fixed exact8 navigation을 Stable StepId + VisibleSteps로 교체하고 Launcher/Propulsion/MissileFlight/Guidance/Heat/Charge conditional validation, existing reference consistency, Projectile Capability durable roundtrip을 추가.
// - v1.0.0: exact8 Wizard navigation, existing child clean reuse, Turret/Projectile/Weapon Create-only durable sequencing, partial durable recovery와 deterministic result bundle을 구현.
// Migration:
// - 기존 Product child asset은 update/save하지 않습니다. 신규 child만 exact Create하며 EquipmentPreset 저장은 기존 Equipment Builder에 남깁니다.
// - WEA-P0-03부터 신규 Damage/Ammo는 별도 writer 없이 existing provider의 ReviewedMutationReady callback과 shared durable core를 재사용합니다.
// - 중간 durable 성공 뒤 downstream 실패 시 자동 rollback/delete하지 않고 confirmed session path를 same-session retry에서 재검증·재사용합니다.

#include "WeaponAuthoring/CFWeaponGuideVM.h"

#include "CFAmmoData.h"
#include "CFDamageData.h"
#include "CFProjectileActor.h"
#include "CFProjectileData.h"
#include "CFTurretMountData.h"
#include "CFWeaponData.h"
#include "DataAuthoring/CFDAAmmoProvider.h"
#include "DataAuthoring/CFDACommonPrimitives.h"
#include "DataAuthoring/CFDADamageProvider.h"
#include "DataAuthoring/CFDADurableCore.h"
#include "DataAuthoring/CFDATypeDispatch.h"
#include "DataAuthoring/CFDAStagingApply.h"

#include "Engine/StaticMesh.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	// Weapon Guide Product child asset의 고정 루트입니다.
	const FString WeaponGuideProductRoot(TEXT("/Game/CarFight/Weapons/Data"));

	// 신규 TurretMountData를 durable core로 round-trip하기 위한 local typed payload입니다.
	struct FCFWeaponGuideTurretPayload
	{
		// TurretMount logical ID입니다.
		FName TurretMountId = NAME_None;

		// Base mesh exact object path입니다.
		FSoftObjectPath TurretBaseMeshPath;

		// Yaw mesh exact object path입니다.
		FSoftObjectPath TurretYawMeshPath;

		// Pitch mesh exact object path입니다.
		FSoftObjectPath TurretPitchMeshPath;

		// Yaw pivot socket 이름입니다.
		FName YawPivotSocketName = NAME_None;

		// Pitch pivot socket 이름입니다.
		FName PitchPivotSocketName = NAME_None;

		// Legacy primary muzzle socket 이름입니다.
		FName PrimaryMuzzleSocketName = NAME_None;

		// 명시적 가변 muzzle socket 목록입니다.
		TArray<FName> MuzzleSocketNames;

		// 모든 muzzle 존재를 강제할지 여부입니다.
		bool bRequireAllMuzzles = false;

		// Yaw 최소각입니다.
		float MinYawDeg = 0.0f;

		// Yaw 최대각입니다.
		float MaxYawDeg = 0.0f;

		// Pitch 최소각입니다.
		float MinPitchDeg = 0.0f;

		// Pitch 최대각입니다.
		float MaxPitchDeg = 0.0f;

		// Yaw 회전 속도입니다.
		float YawTurnRateDegPerSec = 0.0f;

		// Pitch 회전 속도입니다.
		float PitchTurnRateDegPerSec = 0.0f;

		// 피팅에 반영할 질량입니다.
		float TurretMountWeightKg = 0.0f;
	};

	// 신규 ProjectileData를 durable core로 round-trip하기 위한 local typed payload입니다.
	struct FCFWeaponGuideProjectilePayload
	{
		// Projectile logical ID입니다.
		FName ProjectileId = NAME_None;

		// Projectile Actor class path입니다.
		FSoftClassPath ProjectileActorClassPath;

		// Projectile visual mesh path입니다.
		FSoftObjectPath ProjectileStaticMeshPath;

		// Projectile 초기 속도입니다.
		float InitialSpeed = 0.0f;

		// Projectile 수명입니다.
		float LifeTimeSeconds = 0.0f;

		// 중력 사용 여부입니다.
		bool bAffectedByGravity = false;

		// 중력 배율입니다.
		float GravityScale = 0.0f;

		// 충돌 반경입니다.
		float CollisionRadius = 0.0f;

		// Sweep collision 사용 여부입니다.
		bool bUseSweepCollision = true;

		// 자체 추진 Rocket/Missile 설정입니다.
		FCFProjectilePropulsionConfig PropulsionConfig;

		// 미사일 비행 상태 설정입니다.
		FCFMissileFlightConfig MissileFlightConfig;

		// 미사일 유도 설정입니다.
		FCFMissileGuideConfig MissileGuideConfig;

		// DamageData exact path입니다.
		FSoftObjectPath DamageDataPath;
	};

	// 신규 WeaponData를 durable core로 round-trip하기 위한 local typed payload입니다.
	struct FCFWeaponGuideWeaponPayload
	{
		// Weapon logical ID입니다.
		FName WeaponId = NAME_None;

		// Weapon size입니다.
		ECFVehicleWeaponSize WeaponSize = ECFVehicleWeaponSize::None;

		// 허용할 exact mount type입니다.
		ECFVehicleMountType CompatibleMountType = ECFVehicleMountType::None;

		// 무기 질량입니다.
		float WeaponMassKg = 0.0f;

		// 발사 처리 방식입니다.
		ECFWeaponFireMode FireMode = ECFWeaponFireMode::Projectile;

		// 분당 발사 수입니다.
		float FireRatePerMinute = 0.0f;

		// 최대 사거리입니다.
		float MaxRange = 0.0f;

		// 탄퍼짐 각도입니다.
		float SpreadDeg = 0.0f;

		// 탄창 용량입니다.
		int32 MagazineSize = 0;

		// 초기 장전량입니다.
		int32 InitialLoadedAmmoCount = 0;

		// 1회 발사 소비 탄약 단위입니다.
		int32 AmmoUnitsPerShot = 1;

		// 재장전 시간입니다.
		float ReloadTimeSeconds = 0.0f;

		// 유한 탄약 사용 여부입니다.
		bool bUseFiniteAmmo = false;

		// AmmoData exact path입니다.
		FSoftObjectPath AmmoDataPath;

		// compatibility fallback 탄종 ID입니다.
		FName AmmoTypeId = NAME_None;

		// ProjectileData exact path입니다.
		FSoftObjectPath ProjectileDataPath;

		// Launcher pattern 설정입니다.
		FCFLauncherFirePatternConfig LauncherFirePatternConfig;

		// Launcher release 설정입니다.
		FCFLauncherReleaseConfig LauncherReleaseConfig;

		// Heat 선택 기능 사용 여부입니다.
		bool bUseHeat = false;

		// 한 발당 열량입니다.
		float HeatPerShot = 0.0f;

		// 최대 열량입니다.
		float MaxHeat = 0.0f;

		// 초당 냉각량입니다.
		float HeatDissipationPerSecond = 0.0f;

		// Charge 선택 기능 사용 여부입니다.
		bool bUseCharge = false;

		// 최대 Charge입니다.
		float MaximumWeaponCharge = 0.0f;

		// 초기 Charge입니다.
		float InitialWeaponCharge = 0.0f;

		// 한 발당 Charge 소비량입니다.
		float WeaponChargePerShot = 0.0f;

		// 초당 Charge 회복량입니다.
		float WeaponChargeRecoveryPerSecond = 0.0f;
	};

	// Object pointer를 nullable canonical soft path로 변환합니다.
	FSoftObjectPath BuildObjectPath(const UObject* Object)
	{
		return Object != nullptr ? FSoftObjectPath(Object) : FSoftObjectPath();
	}

	// Turret payload의 deterministic semantic fingerprint를 생성합니다.
	bool BuildTurretFingerprint(const FCFWeaponGuideTurretPayload& Payload, FString& OutFingerprint, FString& OutError)
	{
		// Semantic token을 누적할 canonical byte stream입니다.
		TArray<uint8> Bytes;
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("TurretMountId"), CFDACommonPrimitives::CanonicalNameText(Payload.TurretMountId));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("BaseMesh"), Payload.TurretBaseMeshPath.ToString());
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("YawMesh"), Payload.TurretYawMeshPath.ToString());
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("PitchMesh"), Payload.TurretPitchMeshPath.ToString());
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("YawPivot"), CFDACommonPrimitives::CanonicalNameText(Payload.YawPivotSocketName));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("PitchPivot"), CFDACommonPrimitives::CanonicalNameText(Payload.PitchPivotSocketName));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("PrimaryMuzzle"), CFDACommonPrimitives::CanonicalNameText(Payload.PrimaryMuzzleSocketName));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("MuzzleCount"), LexToString(Payload.MuzzleSocketNames.Num()));
		for (const FName MuzzleSocketName : Payload.MuzzleSocketNames)
		{
			CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("Muzzle"), CFDACommonPrimitives::CanonicalNameText(MuzzleSocketName));
		}
		CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("RequireAllMuzzles"), Payload.bRequireAllMuzzles);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("MinYaw"), Payload.MinYawDeg);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("MaxYaw"), Payload.MaxYawDeg);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("MinPitch"), Payload.MinPitchDeg);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("MaxPitch"), Payload.MaxPitchDeg);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("YawRate"), Payload.YawTurnRateDegPerSec);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("PitchRate"), Payload.PitchTurnRateDegPerSec);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("WeightKg"), Payload.TurretMountWeightKg);
		return CFDACommonPrimitives::HashCanonicalBytes(Bytes, OutFingerprint, OutError);
	}

	// Turret payload를 exact DataAsset에 materialize합니다.
	void MaterializeTurretPayload(UCFTurretMountData& Asset, const FCFWeaponGuideTurretPayload& Payload)
	{
		Asset.TurretMountId = Payload.TurretMountId;
		Asset.TurretBaseMesh = Cast<UStaticMesh>(Payload.TurretBaseMeshPath.TryLoad());
		Asset.TurretYawMesh = Cast<UStaticMesh>(Payload.TurretYawMeshPath.TryLoad());
		Asset.TurretPitchMesh = Cast<UStaticMesh>(Payload.TurretPitchMeshPath.TryLoad());
		Asset.YawPivotSocketName = Payload.YawPivotSocketName;
		Asset.PitchPivotSocketName = Payload.PitchPivotSocketName;
		Asset.MuzzleSocketName = Payload.PrimaryMuzzleSocketName;
		Asset.MuzzleSocketNames = Payload.MuzzleSocketNames;
		Asset.bRequireAllMuzzles = Payload.bRequireAllMuzzles;
		Asset.MinYawDeg = Payload.MinYawDeg;
		Asset.MaxYawDeg = Payload.MaxYawDeg;
		Asset.MinPitchDeg = Payload.MinPitchDeg;
		Asset.MaxPitchDeg = Payload.MaxPitchDeg;
		Asset.YawTurnRateDegPerSec = Payload.YawTurnRateDegPerSec;
		Asset.PitchTurnRateDegPerSec = Payload.PitchTurnRateDegPerSec;
		Asset.TurretMountWeightKg = Payload.TurretMountWeightKg;
	}

	// Turret DataAsset에서 Guide-owned payload를 lossless 추출합니다.
	bool ExtractTurretPayload(const UCFTurretMountData& Asset, FCFWeaponGuideTurretPayload& OutPayload, TArray<FCFDAStagingIssue>& OutIssues)
	{
		OutPayload.TurretMountId = Asset.TurretMountId;
		OutPayload.TurretBaseMeshPath = BuildObjectPath(Asset.TurretBaseMesh.Get());
		OutPayload.TurretYawMeshPath = BuildObjectPath(Asset.TurretYawMesh.Get());
		OutPayload.TurretPitchMeshPath = BuildObjectPath(Asset.TurretPitchMesh.Get());
		OutPayload.YawPivotSocketName = Asset.YawPivotSocketName;
		OutPayload.PitchPivotSocketName = Asset.PitchPivotSocketName;
		OutPayload.PrimaryMuzzleSocketName = Asset.MuzzleSocketName;
		OutPayload.MuzzleSocketNames = Asset.MuzzleSocketNames;
		OutPayload.bRequireAllMuzzles = Asset.bRequireAllMuzzles;
		OutPayload.MinYawDeg = Asset.MinYawDeg;
		OutPayload.MaxYawDeg = Asset.MaxYawDeg;
		OutPayload.MinPitchDeg = Asset.MinPitchDeg;
		OutPayload.MaxPitchDeg = Asset.MaxPitchDeg;
		OutPayload.YawTurnRateDegPerSec = Asset.YawTurnRateDegPerSec;
		OutPayload.PitchTurnRateDegPerSec = Asset.PitchTurnRateDegPerSec;
		OutPayload.TurretMountWeightKg = Asset.TurretMountWeightKg;
		OutIssues.Reset();
		return true;
	}

	// Projectile payload의 deterministic semantic fingerprint를 생성합니다.
	bool BuildProjectileFingerprint(const FCFWeaponGuideProjectilePayload& Payload, FString& OutFingerprint, FString& OutError)
	{
		// Semantic token을 누적할 canonical byte stream입니다.
		TArray<uint8> Bytes;
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("ProjectileId"), CFDACommonPrimitives::CanonicalNameText(Payload.ProjectileId));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("ActorClass"), Payload.ProjectileActorClassPath.ToString());
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("Mesh"), Payload.ProjectileStaticMeshPath.ToString());
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("InitialSpeed"), Payload.InitialSpeed);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("LifeTime"), Payload.LifeTimeSeconds);
		CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("Gravity"), Payload.bAffectedByGravity);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("GravityScale"), Payload.GravityScale);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("CollisionRadius"), Payload.CollisionRadius);
		CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("Sweep"), Payload.bUseSweepCollision);
		CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("UsePropulsion"), Payload.PropulsionConfig.bUsePropulsion);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("IgnitionDelay"), Payload.PropulsionConfig.IgnitionDelaySeconds);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("BurnDuration"), Payload.PropulsionConfig.BurnDurationSeconds);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("ThrustAcceleration"), Payload.PropulsionConfig.ThrustAccelerationCmPerSecSq);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("MaximumPropelledSpeed"), Payload.PropulsionConfig.MaximumPropelledSpeed);
		CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("UseMissileFlight"), Payload.MissileFlightConfig.bUseMissileFlight);
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("AttackProfile"), LexToString(static_cast<int32>(Payload.MissileFlightConfig.AttackProfile)));
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("MinimumClearanceTime"), Payload.MissileFlightConfig.MinimumClearanceTimeSeconds);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("MinimumClearanceDistance"), Payload.MissileFlightConfig.MinimumClearanceDistanceCm);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("TransitionDuration"), Payload.MissileFlightConfig.TransitionDurationSeconds);
		CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("UseTerminalPhase"), Payload.MissileFlightConfig.bUseTerminalPhase);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("TerminalPhaseDistance"), Payload.MissileFlightConfig.TerminalPhaseStartDistanceCm);
		CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("UseGuidance"), Payload.MissileGuideConfig.bUseGuidance);
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("GuideMode"), LexToString(static_cast<int32>(Payload.MissileGuideConfig.GuideMode)));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("LostTargetPolicy"), LexToString(static_cast<int32>(Payload.MissileGuideConfig.LostTargetPolicy)));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("GuidanceLaw"), LexToString(static_cast<int32>(Payload.MissileGuideConfig.GuidanceLaw)));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("GuidanceActivationMode"), LexToString(static_cast<int32>(Payload.MissileGuideConfig.GuidanceActivationMode)));
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("MaximumTurnRate"), Payload.MissileGuideConfig.MaximumTurnRateDegPerSec);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("MaximumLateralAcceleration"), Payload.MissileGuideConfig.MaximumLateralAccelerationCmPerSecSq);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("GuidanceResponseTime"), Payload.MissileGuideConfig.GuidanceResponseTimeSeconds);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("MinimumGuidanceSpeed"), Payload.MissileGuideConfig.MinimumGuidanceSpeedCmPerSec);
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("Damage"), Payload.DamageDataPath.ToString());
		return CFDACommonPrimitives::HashCanonicalBytes(Bytes, OutFingerprint, OutError);
	}

	// Projectile payload를 exact DataAsset에 materialize합니다.
	void MaterializeProjectilePayload(UCFProjectileData& Asset, const FCFWeaponGuideProjectilePayload& Payload)
	{
		Asset.ProjectileId = Payload.ProjectileId;
		Asset.ProjectileActorClass = Payload.ProjectileActorClassPath.IsNull()
			? nullptr
			: Payload.ProjectileActorClassPath.TryLoadClass<ACFProjectileActor>();
		Asset.ProjectileStaticMesh = Cast<UStaticMesh>(Payload.ProjectileStaticMeshPath.TryLoad());
		Asset.InitialSpeed = Payload.InitialSpeed;
		Asset.LifeTimeSeconds = Payload.LifeTimeSeconds;
		Asset.bAffectedByGravity = Payload.bAffectedByGravity;
		Asset.GravityScale = Payload.GravityScale;
		Asset.CollisionRadius = Payload.CollisionRadius;
		Asset.bUseSweepCollision = Payload.bUseSweepCollision;
		Asset.PropulsionConfig = Payload.PropulsionConfig;
		Asset.MissileFlightConfig = Payload.MissileFlightConfig;
		Asset.MissileGuideConfig = Payload.MissileGuideConfig;
		Asset.DefaultDamageData = Cast<UCFDamageData>(Payload.DamageDataPath.TryLoad());
		Asset.DamageProfileId = Asset.DefaultDamageData ? Asset.DefaultDamageData->DamageId : NAME_None;
	}

	// Projectile DataAsset에서 Guide-owned payload를 lossless 추출합니다.
	bool ExtractProjectilePayload(const UCFProjectileData& Asset, FCFWeaponGuideProjectilePayload& OutPayload, TArray<FCFDAStagingIssue>& OutIssues)
	{
		OutPayload.ProjectileId = Asset.ProjectileId;
		OutPayload.ProjectileActorClassPath = Asset.ProjectileActorClass ? FSoftClassPath(Asset.ProjectileActorClass.Get()) : FSoftClassPath();
		OutPayload.ProjectileStaticMeshPath = BuildObjectPath(Asset.ProjectileStaticMesh.Get());
		OutPayload.InitialSpeed = Asset.InitialSpeed;
		OutPayload.LifeTimeSeconds = Asset.LifeTimeSeconds;
		OutPayload.bAffectedByGravity = Asset.bAffectedByGravity;
		OutPayload.GravityScale = Asset.GravityScale;
		OutPayload.CollisionRadius = Asset.CollisionRadius;
		OutPayload.bUseSweepCollision = Asset.bUseSweepCollision;
		OutPayload.PropulsionConfig = Asset.PropulsionConfig;
		OutPayload.MissileFlightConfig = Asset.MissileFlightConfig;
		OutPayload.MissileGuideConfig = Asset.MissileGuideConfig;
		OutPayload.DamageDataPath = BuildObjectPath(Asset.DefaultDamageData.Get());
		OutIssues.Reset();
		return true;
	}

	// Weapon payload의 deterministic semantic fingerprint를 생성합니다.
	bool BuildWeaponFingerprint(const FCFWeaponGuideWeaponPayload& Payload, FString& OutFingerprint, FString& OutError)
	{
		// Semantic token을 누적할 canonical byte stream입니다.
		TArray<uint8> Bytes;
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("WeaponId"), CFDACommonPrimitives::CanonicalNameText(Payload.WeaponId));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("WeaponSize"), LexToString(static_cast<int32>(Payload.WeaponSize)));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("MountType"), LexToString(static_cast<int32>(Payload.CompatibleMountType)));
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("Mass"), Payload.WeaponMassKg);
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("FireMode"), LexToString(static_cast<int32>(Payload.FireMode)));
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("RPM"), Payload.FireRatePerMinute);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("Range"), Payload.MaxRange);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("Spread"), Payload.SpreadDeg);
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("Magazine"), LexToString(Payload.MagazineSize));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("InitialLoaded"), LexToString(Payload.InitialLoadedAmmoCount));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("AmmoUnitsPerShot"), LexToString(Payload.AmmoUnitsPerShot));
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("ReloadTime"), Payload.ReloadTimeSeconds);
		CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("FiniteAmmo"), Payload.bUseFiniteAmmo);
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("AmmoData"), Payload.AmmoDataPath.ToString());
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("AmmoTypeId"), CFDACommonPrimitives::CanonicalNameText(Payload.AmmoTypeId));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("Projectile"), Payload.ProjectileDataPath.ToString());
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("LauncherPattern"), LexToString(static_cast<int32>(Payload.LauncherFirePatternConfig.FirePattern)));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("LauncherCount"), LexToString(Payload.LauncherFirePatternConfig.ProjectileCountPerTrigger));
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("LauncherDelay"), Payload.LauncherFirePatternConfig.InterMuzzleDelaySeconds);
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("LauncherSimultaneous"), LexToString(Payload.LauncherFirePatternConfig.MaximumSimultaneousLaunchCount));
		CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("ReleaseMode"), LexToString(static_cast<int32>(Payload.LauncherReleaseConfig.ReleaseMode)));
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("EjectionSpeed"), Payload.LauncherReleaseConfig.EjectionSpeed);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("CarrierVelocityRatio"), Payload.LauncherReleaseConfig.CarrierVelocityRatio);
		CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("UseHeat"), Payload.bUseHeat);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("HeatPerShot"), Payload.HeatPerShot);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("MaxHeat"), Payload.MaxHeat);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("HeatDissipation"), Payload.HeatDissipationPerSecond);
		CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("UseCharge"), Payload.bUseCharge);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("MaximumCharge"), Payload.MaximumWeaponCharge);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("InitialCharge"), Payload.InitialWeaponCharge);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("ChargePerShot"), Payload.WeaponChargePerShot);
		CFDACommonPrimitives::AppendFloatToken(Bytes, TEXT("ChargeRecovery"), Payload.WeaponChargeRecoveryPerSecond);
		return CFDACommonPrimitives::HashCanonicalBytes(Bytes, OutFingerprint, OutError);
	}

	// Weapon payload를 exact DataAsset에 materialize합니다.
	void MaterializeWeaponPayload(UCFWeaponData& Asset, const FCFWeaponGuideWeaponPayload& Payload)
	{
		Asset.WeaponId = Payload.WeaponId;
		Asset.WeaponSize = Payload.WeaponSize;
		Asset.CompatibleMountTypes.Reset();
		Asset.CompatibleMountTypes.Add(Payload.CompatibleMountType);
		Asset.WeaponMassKg = Payload.WeaponMassKg;
		Asset.FireMode = Payload.FireMode;
		Asset.FireRatePerMinute = Payload.FireRatePerMinute;
		Asset.MaxRange = Payload.MaxRange;
		Asset.SpreadDeg = Payload.SpreadDeg;
		Asset.MagazineSize = Payload.MagazineSize;
		Asset.InitialLoadedAmmoCount = Payload.InitialLoadedAmmoCount;
		Asset.AmmoUnitsPerShot = Payload.AmmoUnitsPerShot;
		Asset.ReloadTimeSeconds = Payload.ReloadTimeSeconds;
		Asset.bUseInfiniteAmmoForDebug = !Payload.bUseFiniteAmmo;
		Asset.DefaultAmmoData = Cast<UCFAmmoData>(Payload.AmmoDataPath.TryLoad());
		Asset.AmmoTypeId = Payload.AmmoTypeId;
		Asset.DefaultProjectileData = Cast<UCFProjectileData>(Payload.ProjectileDataPath.TryLoad());
		Asset.ProjectileDataId = Asset.DefaultProjectileData ? Asset.DefaultProjectileData->ProjectileId : NAME_None;
		Asset.LauncherFirePatternConfig = Payload.LauncherFirePatternConfig;
		Asset.LauncherReleaseConfig = Payload.LauncherReleaseConfig;
		Asset.HeatPerShot = Payload.bUseHeat ? Payload.HeatPerShot : 0.0f;
		Asset.MaxHeat = Payload.bUseHeat ? Payload.MaxHeat : 0.0f;
		Asset.HeatDissipationPerSecond = Payload.bUseHeat ? Payload.HeatDissipationPerSecond : 0.0f;
		Asset.MaximumWeaponCharge = Payload.bUseCharge ? Payload.MaximumWeaponCharge : 0.0f;
		Asset.InitialWeaponCharge = Payload.bUseCharge ? Payload.InitialWeaponCharge : 0.0f;
		Asset.WeaponChargePerShot = Payload.bUseCharge ? Payload.WeaponChargePerShot : 0.0f;
		Asset.WeaponChargeRecoveryPerSecond = Payload.bUseCharge ? Payload.WeaponChargeRecoveryPerSecond : 0.0f;
	}

	// Weapon DataAsset에서 Guide-owned payload를 lossless 추출합니다.
	bool ExtractWeaponPayload(const UCFWeaponData& Asset, FCFWeaponGuideWeaponPayload& OutPayload, TArray<FCFDAStagingIssue>& OutIssues)
	{
		OutPayload.WeaponId = Asset.WeaponId;
		OutPayload.WeaponSize = Asset.WeaponSize;
		OutPayload.CompatibleMountType = Asset.CompatibleMountTypes.Num() == 1 ? Asset.CompatibleMountTypes[0] : ECFVehicleMountType::None;
		OutPayload.WeaponMassKg = Asset.WeaponMassKg;
		OutPayload.FireMode = Asset.FireMode;
		OutPayload.FireRatePerMinute = Asset.FireRatePerMinute;
		OutPayload.MaxRange = Asset.MaxRange;
		OutPayload.SpreadDeg = Asset.SpreadDeg;
		OutPayload.MagazineSize = Asset.MagazineSize;
		OutPayload.InitialLoadedAmmoCount = Asset.InitialLoadedAmmoCount;
		OutPayload.AmmoUnitsPerShot = Asset.AmmoUnitsPerShot;
		OutPayload.ReloadTimeSeconds = Asset.ReloadTimeSeconds;
		OutPayload.bUseFiniteAmmo = !Asset.bUseInfiniteAmmoForDebug;
		OutPayload.AmmoDataPath = BuildObjectPath(Asset.DefaultAmmoData.Get());
		OutPayload.AmmoTypeId = Asset.AmmoTypeId;
		OutPayload.ProjectileDataPath = BuildObjectPath(Asset.DefaultProjectileData.Get());
		OutPayload.LauncherFirePatternConfig = Asset.LauncherFirePatternConfig;
		OutPayload.LauncherReleaseConfig = Asset.LauncherReleaseConfig;
		OutPayload.bUseHeat = Asset.HeatPerShot > 0.0f || Asset.MaxHeat > 0.0f || Asset.HeatDissipationPerSecond > 0.0f;
		OutPayload.HeatPerShot = Asset.HeatPerShot;
		OutPayload.MaxHeat = Asset.MaxHeat;
		OutPayload.HeatDissipationPerSecond = Asset.HeatDissipationPerSecond;
		OutPayload.bUseCharge = Asset.MaximumWeaponCharge > 0.0f || Asset.InitialWeaponCharge > 0.0f || Asset.WeaponChargePerShot > 0.0f || Asset.WeaponChargeRecoveryPerSecond > 0.0f;
		OutPayload.MaximumWeaponCharge = Asset.MaximumWeaponCharge;
		OutPayload.InitialWeaponCharge = Asset.InitialWeaponCharge;
		OutPayload.WeaponChargePerShot = Asset.WeaponChargePerShot;
		OutPayload.WeaponChargeRecoveryPerSecond = Asset.WeaponChargeRecoveryPerSecond;
		OutIssues.Reset();
		return true;
	}

	// Create-only durable core가 요구하는 common Preview row를 구성합니다.
	FCFDACommonPreviewRow BuildCreateRow(
		const FString& SchemaId,
		const UClass* DataAssetClass,
		const FName StableLogicalId,
		const FString& TargetObjectPath,
		const FString& SemanticFingerprint)
	{
		// Exact Create candidate row입니다.
		FCFDACommonPreviewRow Row;
		Row.Kind = ECFDAStagingPreviewKind::Create;
		Row.Envelope.SchemaId = SchemaId;
		Row.Envelope.SchemaRevision = 1;
		Row.Envelope.AdapterContractRevision = 1;
		Row.Envelope.DataAssetTypeClassPath = DataAssetClass ? DataAssetClass->GetPathName() : FString();
		Row.Envelope.StableLogicalId = StableLogicalId;
		Row.Envelope.TargetObjectPath = TargetObjectPath;
		Row.Envelope.StagingRelativePath = FString::Printf(TEXT("WeaponGuide://%s"), *StableLogicalId.ToString());
		Row.Envelope.StagingSemanticFingerprint = SemanticFingerprint;
		Row.Envelope.PlannedOperation = ECFDAStagingPreviewKind::Create;
		return Row;
	}

	// Current Draft와 resolved reference로 Guide-local Turret payload를 구성합니다.
	FCFWeaponGuideTurretPayload BuildGuideTurretPayload(const FCFWeaponGuideDraft& Draft, const FName TurretId)
	{
		// Guide-local Turret exact payload입니다.
		FCFWeaponGuideTurretPayload Payload;
		Payload.TurretMountId = TurretId;
		Payload.TurretBaseMeshPath = Draft.Turret.TurretBaseMeshPath;
		Payload.TurretYawMeshPath = Draft.Turret.TurretYawMeshPath;
		Payload.TurretPitchMeshPath = Draft.Turret.TurretPitchMeshPath;
		Payload.YawPivotSocketName = Draft.Turret.YawPivotSocketName;
		Payload.PitchPivotSocketName = Draft.Turret.PitchPivotSocketName;
		Payload.PrimaryMuzzleSocketName = Draft.Turret.PrimaryMuzzleSocketName;
		Payload.MuzzleSocketNames = Draft.Turret.AdditionalMuzzleSocketNames;
		if (!Payload.PrimaryMuzzleSocketName.IsNone() && !Payload.MuzzleSocketNames.Contains(Payload.PrimaryMuzzleSocketName))
		{
			Payload.MuzzleSocketNames.Insert(Payload.PrimaryMuzzleSocketName, 0);
		}
		Payload.bRequireAllMuzzles = Draft.Turret.bRequireAllMuzzles;
		Payload.MinYawDeg = Draft.Turret.MinYawDeg;
		Payload.MaxYawDeg = Draft.Turret.MaxYawDeg;
		Payload.MinPitchDeg = Draft.Turret.MinPitchDeg;
		Payload.MaxPitchDeg = Draft.Turret.MaxPitchDeg;
		Payload.YawTurnRateDegPerSec = Draft.Turret.YawTurnRateDegPerSec;
		Payload.PitchTurnRateDegPerSec = Draft.Turret.PitchTurnRateDegPerSec;
		Payload.TurretMountWeightKg = Draft.Turret.TurretMountWeightKg;
		return Payload;
	}

	// Current Draft와 resolved Damage reference로 Guide-local Projectile payload를 구성합니다.
	FCFWeaponGuideProjectilePayload BuildGuideProjectilePayload(
		const FCFWeaponGuideDraft& Draft,
		const FName ProjectileId,
		const FSoftObjectPath& DamagePath)
	{
		// Guide-local Projectile exact payload입니다.
		FCFWeaponGuideProjectilePayload Payload;
		Payload.ProjectileId = ProjectileId;
		Payload.ProjectileActorClassPath = Draft.Projectile.ProjectileActorClassPath;
		Payload.ProjectileStaticMeshPath = Draft.Projectile.ProjectileStaticMeshPath;
		Payload.InitialSpeed = Draft.Projectile.InitialSpeed;
		Payload.LifeTimeSeconds = Draft.Projectile.LifeTimeSeconds;
		Payload.bAffectedByGravity = Draft.Projectile.bAffectedByGravity;
		Payload.GravityScale = Draft.Projectile.GravityScale;
		Payload.CollisionRadius = Draft.Projectile.CollisionRadius;
		Payload.bUseSweepCollision = Draft.Projectile.bUseSweepCollision;
		Payload.PropulsionConfig = Draft.Projectile.PropulsionConfig;
		Payload.MissileFlightConfig = Draft.Projectile.MissileFlightConfig;
		Payload.MissileGuideConfig = Draft.Projectile.MissileGuideConfig;
		Payload.DamageDataPath = DamagePath;
		return Payload;
	}

	// Current Draft와 resolved Projectile/Ammo reference로 Guide-local Weapon payload를 구성합니다.
	FCFWeaponGuideWeaponPayload BuildGuideWeaponPayload(
		const FCFWeaponGuideDraft& Draft,
		const FName WeaponId,
		const FSoftObjectPath& ProjectilePath,
		const FSoftObjectPath& AmmoPath,
		const FName ResolvedAmmoTypeId)
	{
		// Guide-local Weapon exact payload입니다.
		FCFWeaponGuideWeaponPayload Payload;
		Payload.WeaponId = WeaponId;
		Payload.WeaponSize = Draft.RequiredWeaponSize;
		Payload.CompatibleMountType = Draft.RequiredMountType;
		Payload.WeaponMassKg = Draft.Weapon.WeaponMassKg;
		Payload.FireMode = Draft.Weapon.FireMode;
		Payload.FireRatePerMinute = Draft.Weapon.FireRatePerMinute;
		Payload.MaxRange = Draft.Weapon.MaxRange;
		Payload.SpreadDeg = Draft.Weapon.SpreadDeg;
		Payload.MagazineSize = Draft.Weapon.MagazineSize;
		Payload.InitialLoadedAmmoCount = Draft.Weapon.InitialLoadedAmmoCount;
		Payload.AmmoUnitsPerShot = Draft.Weapon.AmmoUnitsPerShot;
		Payload.ReloadTimeSeconds = Draft.Weapon.ReloadTimeSeconds;
		Payload.bUseFiniteAmmo = Draft.Weapon.bUseFiniteAmmo;
		Payload.AmmoDataPath = Draft.Weapon.bUseFiniteAmmo ? AmmoPath : FSoftObjectPath();
		Payload.AmmoTypeId = ResolvedAmmoTypeId;
		Payload.ProjectileDataPath = ProjectilePath;
		Payload.LauncherFirePatternConfig = Draft.Weapon.LauncherFirePatternConfig;
		Payload.LauncherReleaseConfig = Draft.Weapon.LauncherReleaseConfig;
		Payload.bUseHeat = Draft.Weapon.bUseHeat;
		Payload.HeatPerShot = Draft.Weapon.HeatPerShot;
		Payload.MaxHeat = Draft.Weapon.MaxHeat;
		Payload.HeatDissipationPerSecond = Draft.Weapon.HeatDissipationPerSecond;
		Payload.bUseCharge = Draft.Weapon.bUseCharge;
		Payload.MaximumWeaponCharge = Draft.Weapon.MaximumWeaponCharge;
		Payload.InitialWeaponCharge = Draft.Weapon.InitialWeaponCharge;
		Payload.WeaponChargePerShot = Draft.Weapon.WeaponChargePerShot;
		Payload.WeaponChargeRecoveryPerSecond = Draft.Weapon.WeaponChargeRecoveryPerSecond;
		return Payload;
	}

	// Guide Damage Draft를 existing provider exact12 typed payload로 변환합니다.
	FCFDADamagePayload BuildDamageProviderPayload(const FCFWeaponGuideDraft& Draft, const FName DamageId)
	{
		// Existing Damage provider가 소유하는 exact12 payload입니다.
		FCFDADamagePayload Payload;
		Payload.DamageId = DamageId;
		Payload.DamageType = Draft.Damage.DamageType;
		Payload.BaseDamage = Draft.Damage.BaseDamage;
		Payload.bCanDamageSelf = Draft.Damage.bCanDamageSelf;
		Payload.ArmorPenetration = Draft.Damage.ArmorPenetration;
		Payload.bUseRadialDamage = Draft.Damage.bUseRadialDamage;
		Payload.ExplosionRadius = Draft.Damage.ExplosionRadius;
		Payload.ExplosionInnerRadius = Draft.Damage.ExplosionInnerRadius;
		Payload.ExplosionDamage = Draft.Damage.ExplosionDamage;
		Payload.MinExplosionDamageScale = Draft.Damage.MinExplosionDamageScale;
		Payload.ModuleDamageScale = Draft.Damage.ModuleDamageScale;
		Payload.ImpulseStrength = Draft.Damage.ImpulseStrength;
		return Payload;
	}

	// Guide Ammo Draft를 existing provider exact8 typed payload로 변환합니다.
	FCFDAAmmoPayload BuildAmmoProviderPayload(const FCFWeaponGuideDraft& Draft, const FName AmmoId)
	{
		// Existing Ammo provider가 소유하는 exact8 payload입니다.
		FCFDAAmmoPayload Payload;
		Payload.AmmoId = AmmoId;
		Payload.AmmoDisplayName.Text = Draft.Ammo.AmmoDisplayName;
		Payload.AmmoFamilyId = Draft.Ammo.AmmoFamilyId;
		Payload.UnitMassKg = Draft.Ammo.UnitMassKg;
		Payload.AmmoTags = Draft.Ammo.AmmoTags;
		Payload.AmmoIcon = Draft.Ammo.AmmoIconPath;
		Payload.MaximumLoadableAmmoCount = Draft.Ammo.MaximumLoadableAmmoCount;
		Payload.bCanBeResupplied = Draft.Ammo.bCanBeResupplied;
		return Payload;
	}

	// Existing Damage provider에서 memory-only candidate를 fresh Create Preview로 분류합니다.
	bool BuildDamageProviderCreateCandidate(
		const FCFWeaponGuideDraft& Draft,
		const FName DamageId,
		const FString& TargetObjectPath,
		FString& OutJsonText,
		FCFDACommonPreviewRow& OutPreviewRow,
		FString& OutError)
	{
		// Current production Damage provider authority입니다.
		const FCFDADamageTypeProvider& Provider = CFDADamageProvider::GetProvider();
		if (!CFDATypeDispatch::ValidateProviderMutationReady(Provider, OutError))
		{
			return false;
		}

		// Guide Draft에서 변환한 exact12 desired payload입니다.
		const FCFDADamagePayload Payload = BuildDamageProviderPayload(Draft, DamageId);
		if (!CFDADamageProviderImpl::SerializeStagingJson(Payload, TargetObjectPath, FString(), OutJsonText, OutError))
		{
			return false;
		}

		// Memory-only JSON을 provider-local parser/reference validation까지 통과시킨 common envelope입니다.
		FCFDACommonEnvelope Envelope;
		// Provider parser diagnostics입니다.
		TArray<FCFDAStagingIssue> ParseIssues;
		if (!CFDADamageProviderImpl::ParseCommonCandidate(OutJsonText, FString(), Envelope, ParseIssues))
		{
			OutError = ParseIssues.IsEmpty() ? TEXT("Damage provider fresh candidate parse에 실패했습니다.") : ParseIssues[0].Message;
			return false;
		}

		// Exact current Damage target/identity truth입니다.
		FCFDACommonCurrentState CurrentState;
		// Current-state resolver diagnostics입니다.
		TArray<FCFDAStagingIssue> CurrentIssues;
		if (!CFDADamageProviderImpl::ResolveCommonCurrentState(Envelope, CurrentState, CurrentIssues))
		{
			OutError = CurrentIssues.IsEmpty() ? TEXT("Damage provider current-state resolver에 실패했습니다.") : CurrentIssues[0].Message;
			return false;
		}

		OutPreviewRow = CFDATypeDispatch::BuildCommonPreview(Envelope, CurrentState);
		if (OutPreviewRow.Kind != ECFDAStagingPreviewKind::Create)
		{
			OutError = OutPreviewRow.Issues.IsEmpty()
				? TEXT("Damage provider fresh Preview가 exact Create로 분류되지 않았습니다.")
				: OutPreviewRow.Issues[0].Message;
			return false;
		}
		OutError.Reset();
		return true;
	}

	// Existing Ammo provider에서 memory-only candidate를 fresh Create Preview로 분류합니다.
	bool BuildAmmoProviderCreateCandidate(
		const FCFWeaponGuideDraft& Draft,
		const FName AmmoId,
		const FString& TargetObjectPath,
		FString& OutJsonText,
		FCFDACommonPreviewRow& OutPreviewRow,
		FString& OutError)
	{
		// Current production Ammo provider authority입니다.
		const FCFDAAmmoTypeProvider& Provider = CFDAAmmoProvider::GetProvider();
		if (!CFDATypeDispatch::ValidateProviderMutationReady(Provider, OutError))
		{
			return false;
		}

		// Guide Draft에서 변환한 exact8 desired payload입니다.
		const FCFDAAmmoPayload Payload = BuildAmmoProviderPayload(Draft, AmmoId);
		if (!CFDAAmmoProviderImpl::SerializeStagingJson(Payload, TargetObjectPath, FString(), OutJsonText, OutError))
		{
			return false;
		}

		// Memory-only JSON을 provider-local parser/reference validation까지 통과시킨 common envelope입니다.
		FCFDACommonEnvelope Envelope;
		// Provider parser diagnostics입니다.
		TArray<FCFDAStagingIssue> ParseIssues;
		if (!CFDAAmmoProviderImpl::ParseCommonCandidate(OutJsonText, FString(), Envelope, ParseIssues))
		{
			OutError = ParseIssues.IsEmpty() ? TEXT("Ammo provider fresh candidate parse에 실패했습니다.") : ParseIssues[0].Message;
			return false;
		}

		// Exact current Ammo target/identity truth입니다.
		FCFDACommonCurrentState CurrentState;
		// Current-state resolver diagnostics입니다.
		TArray<FCFDAStagingIssue> CurrentIssues;
		if (!CFDAAmmoProviderImpl::ResolveCommonCurrentState(Envelope, CurrentState, CurrentIssues))
		{
			OutError = CurrentIssues.IsEmpty() ? TEXT("Ammo provider current-state resolver에 실패했습니다.") : CurrentIssues[0].Message;
			return false;
		}

		OutPreviewRow = CFDATypeDispatch::BuildCommonPreview(Envelope, CurrentState);
		if (OutPreviewRow.Kind != ECFDAStagingPreviewKind::Create)
		{
			OutError = OutPreviewRow.Issues.IsEmpty()
				? TEXT("Ammo provider fresh Preview가 exact Create로 분류되지 않았습니다.")
				: OutPreviewRow.Issues[0].Message;
			return false;
		}
		OutError.Reset();
		return true;
	}
}

// 새 빈 무장 제작 세션으로 모든 transient state를 초기화합니다.
void FCFWeaponGuideVM::Reset()
{
	CurrentStep = ECFWeaponGuideStep::IdentityTemplate;
	Draft = FCFWeaponGuideDraft();

	// Guided Draft의 일반 Projectile 신규 작성에서 사용할 CarFight 기본 Projectile Actor class입니다.
	Draft.Projectile.ProjectileActorClassPath = FSoftClassPath(ACFProjectileActor::StaticClass());

	ResultBundle = FCFWeaponGuideResultBundle();
	CompletionSemanticFingerprint.Reset();
	SessionCreatedDamagePath.Reset();
	SessionCreatedAmmoPath.Reset();
	SessionCreatedTurretPath.Reset();
	SessionCreatedProjectilePath.Reset();
	SessionCreatedWeaponPath.Reset();
#if WITH_DEV_AUTOMATION_TESTS
	TestDurableInterruptionStep.Reset();
#endif
	LastStatusMessage = TEXT("새 무장 제작 초안을 시작했습니다. 먼저 에셋 이름, 표시 이름과 제작 Template을 확인하세요.");
}

// 현재 Draft/Capability에서 사용자에게 보이는 Step projection을 반환합니다.
TArray<ECFWeaponGuideStep> FCFWeaponGuideVM::GetVisibleSteps() const
{
	// current Draft에서 계산한 visible semantic Step 목록입니다.
	TArray<ECFWeaponGuideStep> VisibleSteps;
	BuildVisibleSteps(VisibleSteps);
	return VisibleSteps;
}

// 지정한 Stable StepId가 현재 projection에 보이는지 반환합니다.
bool FCFWeaponGuideVM::IsStepVisible(const ECFWeaponGuideStep Step) const
{
	// current visible semantic Step 목록입니다.
	TArray<ECFWeaponGuideStep> VisibleSteps;
	BuildVisibleSteps(VisibleSteps);
	return VisibleSteps.Contains(Step);
}

// current Draft에서 visible semantic StepId를 deterministic 순서로 계산합니다.
void FCFWeaponGuideVM::BuildVisibleSteps(TArray<ECFWeaponGuideStep>& OutSteps) const
{
	OutSteps.Reset();
	OutSteps.Add(ECFWeaponGuideStep::IdentityTemplate);
	OutSteps.Add(ECFWeaponGuideStep::MountCompatibility);
	OutSteps.Add(ECFWeaponGuideStep::MountGeometry);
	OutSteps.Add(ECFWeaponGuideStep::FireBehavior);

	if (Draft.Weapon.FireMode == ECFWeaponFireMode::Projectile)
	{
		OutSteps.Add(ECFWeaponGuideStep::Projectile);
	}

	OutSteps.Add(ECFWeaponGuideStep::Damage);

	if (Draft.Weapon.bUseFiniteAmmo)
	{
		OutSteps.Add(ECFWeaponGuideStep::Ammo);
	}

	if (Draft.Weapon.bUseLauncher)
	{
		OutSteps.Add(ECFWeaponGuideStep::Launcher);
	}

	if (Draft.Weapon.FireMode == ECFWeaponFireMode::Projectile && Draft.Projectile.PropulsionConfig.bUsePropulsion)
	{
		OutSteps.Add(ECFWeaponGuideStep::Propulsion);
	}

	if (Draft.Weapon.FireMode == ECFWeaponFireMode::Projectile && Draft.Projectile.MissileFlightConfig.bUseMissileFlight)
	{
		OutSteps.Add(ECFWeaponGuideStep::MissileFlight);
	}

	if (Draft.Weapon.FireMode == ECFWeaponFireMode::Projectile && Draft.Projectile.MissileGuideConfig.bUseGuidance)
	{
		OutSteps.Add(ECFWeaponGuideStep::Guidance);
	}

	if (Draft.Weapon.bUseHeat)
	{
		OutSteps.Add(ECFWeaponGuideStep::Heat);
	}

	if (Draft.Weapon.bUseCharge)
	{
		OutSteps.Add(ECFWeaponGuideStep::Charge);
	}

	OutSteps.Add(ECFWeaponGuideStep::ReviewCreate);
	OutSteps.Add(ECFWeaponGuideStep::Complete);
}

// current Step이 projection에서 사라졌을 때 nearest visible Step으로 수렴시킵니다.
void FCFWeaponGuideVM::NormalizeCurrentStepToVisible()
{
	// topology 변경 후 current visible semantic Step 목록입니다.
	TArray<ECFWeaponGuideStep> VisibleSteps;
	BuildVisibleSteps(VisibleSteps);
	if (VisibleSteps.IsEmpty() || VisibleSteps.Contains(CurrentStep))
	{
		return;
	}

	// 제거된 Step보다 semantic 순서가 앞서거나 같은 마지막 visible Step입니다.
	ECFWeaponGuideStep NearestStep = VisibleSteps[0];
	for (const ECFWeaponGuideStep CandidateStep : VisibleSteps)
	{
		if (static_cast<uint8>(CandidateStep) > static_cast<uint8>(CurrentStep))
		{
			break;
		}
		NearestStep = CandidateStep;
	}

	CurrentStep = NearestStep;
}

// 사용자 기본 이름을 정규화하고 모든 신규 child target/ID 파생 상태를 갱신합니다.
void FCFWeaponGuideVM::SetBaseAssetName(const FString& BaseAssetName)
{
	// 부분 durable Create 뒤 identity 변경을 막기 위한 생성 흔적 존재 여부입니다.
	const bool bHasDurableChild = !SessionCreatedDamagePath.IsNull() || !SessionCreatedAmmoPath.IsNull() || !SessionCreatedTurretPath.IsNull() || !SessionCreatedProjectilePath.IsNull() || !SessionCreatedWeaponPath.IsNull();
	if (bHasDurableChild)
	{
		LastStatusMessage = TEXT("일부 child가 이미 durable 저장되었습니다. 같은 세션에서는 Base Asset Name을 바꿀 수 없습니다. 현재 이름으로 재시도하거나 새 세션을 시작하세요.");
		return;
	}

	// 앞뒤 공백을 제거한 사용자 기본 이름입니다.
	FString TrimmedName = BaseAssetName;
	TrimmedName.TrimStartAndEndInline();
	if (Draft.BaseAssetName == TrimmedName)
	{
		return;
	}

	Draft.BaseAssetName = MoveTemp(TrimmedName);
	ResultBundle = FCFWeaponGuideResultBundle();
	LastStatusMessage = Draft.BaseAssetName.IsEmpty()
		? TEXT("에셋 이름을 입력하세요.")
		: FString::Printf(TEXT("'%s' 기준으로 child ID와 저장 경로를 자동 계산했습니다."), *Draft.BaseAssetName);
}

// 게임 표시 이름 제안값만 transient draft에 반영합니다.
void FCFWeaponGuideVM::SetSuggestedDisplayName(const FString& SuggestedDisplayName)
{
	Draft.SuggestedDisplayName = SuggestedDisplayName;
	Draft.SuggestedDisplayName.TrimStartAndEndInline();
	ResultBundle = FCFWeaponGuideResultBundle();
}

// 제작 Template을 선택하고 일반적인 Capability 기본값을 제안합니다.
void FCFWeaponGuideVM::SetTemplate(const ECFWeaponGuideTemplate Template)
{
	// 부분 durable Create 뒤 토폴로지 변경을 막기 위한 생성 흔적 존재 여부입니다.
	const bool bHasDurableChild = !SessionCreatedDamagePath.IsNull() || !SessionCreatedAmmoPath.IsNull() || !SessionCreatedTurretPath.IsNull() || !SessionCreatedProjectilePath.IsNull() || !SessionCreatedWeaponPath.IsNull();
	if (bHasDurableChild)
	{
		LastStatusMessage = TEXT("일부 child가 이미 durable 저장되었습니다. 같은 세션에서는 Template을 바꿀 수 없습니다.");
		return;
	}

	Draft.Template = Template;
	if (Template == ECFWeaponGuideTemplate::DirectFireCannon)
	{
		Draft.Weapon.FireMode = ECFWeaponFireMode::Projectile;
		Draft.Weapon.bUseFiniteAmmo = false;
		Draft.Weapon.bUseLauncher = false;
		Draft.Projectile.PropulsionConfig.bUsePropulsion = false;
		Draft.Projectile.MissileFlightConfig.bUseMissileFlight = false;
		Draft.Projectile.MissileGuideConfig.bUseGuidance = false;
		Draft.Weapon.bUseHeat = false;
		Draft.Weapon.bUseCharge = false;
	}
	else if (Template == ECFWeaponGuideTemplate::AutomaticGun)
	{
		Draft.Weapon.FireMode = ECFWeaponFireMode::Projectile;
		Draft.Weapon.bUseFiniteAmmo = true;
		Draft.Weapon.bUseLauncher = false;
		Draft.Projectile.PropulsionConfig.bUsePropulsion = false;
		Draft.Projectile.MissileFlightConfig.bUseMissileFlight = false;
		Draft.Projectile.MissileGuideConfig.bUseGuidance = false;
		Draft.Weapon.bUseHeat = false;
		Draft.Weapon.bUseCharge = false;
	}
	else if (Template == ECFWeaponGuideTemplate::RocketLauncher)
	{
		Draft.Weapon.FireMode = ECFWeaponFireMode::Projectile;
		Draft.Weapon.bUseLauncher = true;
		Draft.Projectile.PropulsionConfig.bUsePropulsion = true;
		Draft.Projectile.MissileFlightConfig.bUseMissileFlight = false;
		Draft.Projectile.MissileGuideConfig.bUseGuidance = false;
	}
	else if (Template == ECFWeaponGuideTemplate::GuidedMissileLauncher)
	{
		Draft.Weapon.FireMode = ECFWeaponFireMode::Projectile;
		Draft.Weapon.bUseLauncher = true;
		Draft.Projectile.PropulsionConfig.bUsePropulsion = true;
		Draft.Projectile.MissileFlightConfig.bUseMissileFlight = true;
		Draft.Projectile.MissileGuideConfig.bUseGuidance = true;
		Draft.Projectile.MissileGuideConfig.GuideMode = ECFMissileGuideMode::TargetActor;
	}

	ResultBundle = FCFWeaponGuideResultBundle();
	NormalizeCurrentStepToVisible();
	LastStatusMessage = TEXT("선택한 Template의 일반적인 Capability 기본값을 적용했습니다. 각 Step에서 실제 값을 검토하세요.");
}

// Capability나 FireMode 등 visible topology를 바꾸는 Draft 수정 뒤 current Step을 안전한 visible Step에 수렴시킵니다.
void FCFWeaponGuideVM::NotifyDraftTopologyChanged()
{
	ResultBundle = FCFWeaponGuideResultBundle();
	NormalizeCurrentStepToVisible();
}

// RequiredMountType을 변경하고 신규 Weapon의 compatible mount 의미와 동기화합니다.
void FCFWeaponGuideVM::SetRequiredMountType(const ECFVehicleMountType RequiredMountType)
{
	Draft.RequiredMountType = RequiredMountType;
	ResultBundle = FCFWeaponGuideResultBundle();
}

// RequiredWeaponSize를 변경하고 신규 WeaponData의 WeaponSize 의미와 동기화합니다.
void FCFWeaponGuideVM::SetRequiredWeaponSize(const ECFVehicleWeaponSize RequiredWeaponSize)
{
	Draft.RequiredWeaponSize = RequiredWeaponSize;
	ResultBundle = FCFWeaponGuideResultBundle();
}

// 기존 TurretMountData exact object path를 read-only reuse 대상으로 선택합니다.
bool FCFWeaponGuideVM::SetExistingTurretPath(const FSoftObjectPath& ObjectPath, FString& OutError)
{
	if (!ValidateExistingChild(ObjectPath, UCFTurretMountData::StaticClass(), TEXT("TurretMountData"), OutError))
	{
		return false;
	}
	Draft.Turret.Mode = ECFWeaponGuideChildMode::ReuseExisting;
	Draft.Turret.ExistingAssetPath = ObjectPath;
	ResultBundle = FCFWeaponGuideResultBundle();
	LastStatusMessage = TEXT("기존 장착/포탑 데이터를 read-only 재사용 대상으로 선택했습니다.");
	return true;
}

// 기존 WeaponData exact object path를 read-only reuse 대상으로 선택합니다.
bool FCFWeaponGuideVM::SetExistingWeaponPath(const FSoftObjectPath& ObjectPath, FString& OutError)
{
	if (!ValidateExistingChild(ObjectPath, UCFWeaponData::StaticClass(), TEXT("WeaponData"), OutError))
	{
		return false;
	}
	// 선택한 기존 WeaponData입니다.
	UCFWeaponData* WeaponData = Cast<UCFWeaponData>(ObjectPath.TryLoad());
	if (WeaponData == nullptr)
	{
		OutError = TEXT("기존 WeaponData load에 실패했습니다.");
		return false;
	}

	Draft.Weapon.Mode = ECFWeaponGuideChildMode::ReuseExisting;
	Draft.Weapon.ExistingAssetPath = ObjectPath;
	Draft.Weapon.FireMode = WeaponData->FireMode;
	Draft.Weapon.bUseFiniteAmmo = !WeaponData->bUseInfiniteAmmoForDebug;
	Draft.Weapon.AmmoTypeId = WeaponData->AmmoTypeId;
	Draft.Weapon.LauncherFirePatternConfig = WeaponData->LauncherFirePatternConfig;
	Draft.Weapon.LauncherReleaseConfig = WeaponData->LauncherReleaseConfig;
	Draft.Weapon.bUseHeat = WeaponData->HeatPerShot > 0.0f || WeaponData->MaxHeat > 0.0f || WeaponData->HeatDissipationPerSecond > 0.0f;
	Draft.Weapon.HeatPerShot = WeaponData->HeatPerShot;
	Draft.Weapon.MaxHeat = WeaponData->MaxHeat;
	Draft.Weapon.HeatDissipationPerSecond = WeaponData->HeatDissipationPerSecond;
	Draft.Weapon.bUseCharge = WeaponData->MaximumWeaponCharge > 0.0f || WeaponData->InitialWeaponCharge > 0.0f || WeaponData->WeaponChargePerShot > 0.0f || WeaponData->WeaponChargeRecoveryPerSecond > 0.0f;
	Draft.Weapon.MaximumWeaponCharge = WeaponData->MaximumWeaponCharge;
	Draft.Weapon.InitialWeaponCharge = WeaponData->InitialWeaponCharge;
	Draft.Weapon.WeaponChargePerShot = WeaponData->WeaponChargePerShot;
	Draft.Weapon.WeaponChargeRecoveryPerSecond = WeaponData->WeaponChargeRecoveryPerSecond;
	Draft.Ammo.Mode = ECFWeaponGuideChildMode::ReuseExisting;
	Draft.Ammo.ExistingAssetPath = BuildObjectPath(WeaponData->DefaultAmmoData.Get());
	if (WeaponData->DefaultProjectileData)
	{
		Draft.Projectile.Mode = ECFWeaponGuideChildMode::ReuseExisting;
		Draft.Projectile.ExistingAssetPath = FSoftObjectPath(WeaponData->DefaultProjectileData.Get());
		Draft.Projectile.PropulsionConfig = WeaponData->DefaultProjectileData->PropulsionConfig;
		Draft.Projectile.MissileFlightConfig = WeaponData->DefaultProjectileData->MissileFlightConfig;
		Draft.Projectile.MissileGuideConfig = WeaponData->DefaultProjectileData->MissileGuideConfig;
		Draft.Damage.Mode = ECFWeaponGuideChildMode::ReuseExisting;
		Draft.Damage.ExistingAssetPath = BuildObjectPath(WeaponData->DefaultProjectileData->DefaultDamageData.Get());
	}
	ResultBundle = FCFWeaponGuideResultBundle();
	LastStatusMessage = TEXT("기존 WeaponData와 연결 Projectile/Ammo/Damage reference를 read-only 재사용 상태로 불러왔습니다.");
	OutError.Reset();
	return true;
}

// 기존 ProjectileData exact object path를 read-only reuse 대상으로 선택합니다.
bool FCFWeaponGuideVM::SetExistingProjectilePath(const FSoftObjectPath& ObjectPath, FString& OutError)
{
	if (!ValidateExistingChild(ObjectPath, UCFProjectileData::StaticClass(), TEXT("ProjectileData"), OutError))
	{
		return false;
	}
	// 선택한 기존 ProjectileData입니다.
	UCFProjectileData* ProjectileData = Cast<UCFProjectileData>(ObjectPath.TryLoad());
	if (ProjectileData == nullptr)
	{
		OutError = TEXT("기존 ProjectileData load에 실패했습니다.");
		return false;
	}
	Draft.Projectile.Mode = ECFWeaponGuideChildMode::ReuseExisting;
	Draft.Projectile.ExistingAssetPath = ObjectPath;
	Draft.Projectile.PropulsionConfig = ProjectileData->PropulsionConfig;
	Draft.Projectile.MissileFlightConfig = ProjectileData->MissileFlightConfig;
	Draft.Projectile.MissileGuideConfig = ProjectileData->MissileGuideConfig;
	Draft.Damage.Mode = ECFWeaponGuideChildMode::ReuseExisting;
	Draft.Damage.ExistingAssetPath = BuildObjectPath(ProjectileData->DefaultDamageData.Get());
	ResultBundle = FCFWeaponGuideResultBundle();
	LastStatusMessage = TEXT("기존 ProjectileData와 연결 DamageData를 read-only 재사용 상태로 불러왔습니다.");
	OutError.Reset();
	return true;
}

// 기존 DamageData exact object path를 Projectile damage source로 선택합니다.
bool FCFWeaponGuideVM::SetDamageDataPath(const FSoftObjectPath& ObjectPath, FString& OutError)
{
	if (!ValidateExistingChild(ObjectPath, UCFDamageData::StaticClass(), TEXT("DamageData"), OutError))
	{
		return false;
	}
	Draft.Damage.Mode = ECFWeaponGuideChildMode::ReuseExisting;
	Draft.Damage.ExistingAssetPath = ObjectPath;
	ResultBundle = FCFWeaponGuideResultBundle();
	LastStatusMessage = TEXT("DamageData를 선택했습니다. Weapon Guide는 이 기존 자산을 수정하거나 저장하지 않습니다.");
	return true;
}

// 기존 AmmoData exact object path를 finite ammo source로 선택합니다.
bool FCFWeaponGuideVM::SetAmmoDataPath(const FSoftObjectPath& ObjectPath, FString& OutError)
{
	if (!ValidateExistingChild(ObjectPath, UCFAmmoData::StaticClass(), TEXT("AmmoData"), OutError))
	{
		return false;
	}
	// 선택한 기존 AmmoData입니다.
	UCFAmmoData* AmmoData = Cast<UCFAmmoData>(ObjectPath.TryLoad());
	if (AmmoData == nullptr || !AmmoData->IsAmmoDataValid())
	{
		OutError = TEXT("선택한 AmmoData가 유효한 AmmoId를 가지지 않습니다.");
		return false;
	}
	Draft.Ammo.Mode = ECFWeaponGuideChildMode::ReuseExisting;
	Draft.Ammo.ExistingAssetPath = ObjectPath;
	Draft.Weapon.AmmoTypeId = AmmoData->AmmoId;
	ResultBundle = FCFWeaponGuideResultBundle();
	LastStatusMessage = TEXT("AmmoData를 선택했습니다. Weapon Guide는 기존 Ammo durable authority를 그대로 재사용합니다.");
	OutError.Reset();
	return true;
}

// 이전 visible Step으로 이동할 수 있는지 반환합니다.
bool FCFWeaponGuideVM::CanMovePrevious() const
{
	// current visible semantic Step 목록입니다.
	TArray<ECFWeaponGuideStep> VisibleSteps;
	BuildVisibleSteps(VisibleSteps);
	return VisibleSteps.IndexOfByKey(CurrentStep) > 0;
}

// 현재 단계 validation을 통과해 다음 visible Step으로 이동할 수 있는지 반환합니다.
bool FCFWeaponGuideVM::CanMoveNext(FString& OutError) const
{
	// current visible semantic Step 목록입니다.
	TArray<ECFWeaponGuideStep> VisibleSteps;
	BuildVisibleSteps(VisibleSteps);
	// current Step의 visible index입니다.
	const int32 CurrentIndex = VisibleSteps.IndexOfByKey(CurrentStep);
	if (CurrentIndex == INDEX_NONE || CurrentIndex >= VisibleSteps.Num() - 1)
	{
		OutError = TEXT("이미 마지막 단계이거나 현재 단계가 visible projection에 없습니다.");
		return false;
	}

	// 현재 단계 validation 결과입니다.
	TArray<FCFWeaponGuideIssue> Issues;
	ValidateStep(CurrentStep, Issues);
	if (HasBlockingIssue(Issues))
	{
		OutError = Issues[0].Message;
		return false;
	}

	OutError.Reset();
	return true;
}

// 이전 visible Step으로 이동합니다.
bool FCFWeaponGuideVM::MovePrevious()
{
	// current visible semantic Step 목록입니다.
	TArray<ECFWeaponGuideStep> VisibleSteps;
	BuildVisibleSteps(VisibleSteps);
	// current Step의 visible index입니다.
	const int32 CurrentIndex = VisibleSteps.IndexOfByKey(CurrentStep);
	if (CurrentIndex <= 0)
	{
		return false;
	}

	CurrentStep = VisibleSteps[CurrentIndex - 1];
	LastStatusMessage = FString::Printf(TEXT("%s 단계로 돌아왔습니다."), *GetStepDisplayName(CurrentStep));
	return true;
}

// 현재 단계 validation을 통과한 뒤 다음 visible Step으로 이동합니다.
bool FCFWeaponGuideVM::MoveNext(FString& OutError)
{
	if (!CanMoveNext(OutError))
	{
		LastStatusMessage = OutError;
		return false;
	}

	// current visible semantic Step 목록입니다.
	TArray<ECFWeaponGuideStep> VisibleSteps;
	BuildVisibleSteps(VisibleSteps);
	// current Step의 visible index입니다.
	const int32 CurrentIndex = VisibleSteps.IndexOfByKey(CurrentStep);
	if (CurrentIndex == INDEX_NONE || CurrentIndex >= VisibleSteps.Num() - 1)
	{
		OutError = TEXT("다음 visible Step을 찾을 수 없습니다.");
		LastStatusMessage = OutError;
		return false;
	}

	CurrentStep = VisibleSteps[CurrentIndex + 1];
	LastStatusMessage = FString::Printf(TEXT("%s 단계로 이동했습니다."), *GetStepDisplayName(CurrentStep));
	OutError.Reset();
	return true;
}

// 지정한 Stable StepId에 필요한 fail-closed validation을 수행합니다.
bool FCFWeaponGuideVM::ValidateStep(const ECFWeaponGuideStep Step, TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	OutIssues.Reset();

	if (Step == ECFWeaponGuideStep::IdentityTemplate)
	{
		// 기본 이름 validation 실패 상세입니다.
		FString NameError;
		if (!ValidateBaseAssetName(NameError))
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, Step, NameError);
		}
		if (Draft.SuggestedDisplayName.IsEmpty())
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, Step, TEXT("게임 표시 이름을 입력하세요. 이 값은 완료 후 EquipmentPreset DisplayName 초안으로 전달됩니다."));
		}
	}
	else if (Step == ECFWeaponGuideStep::MountCompatibility)
	{
		if (Draft.RequiredMountType == ECFVehicleMountType::None || Draft.RequiredMountType == ECFVehicleMountType::Utility)
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, Step, TEXT("무장 장착 타입은 None/Utility가 될 수 없습니다."));
		}
		if (Draft.RequiredWeaponSize == ECFVehicleWeaponSize::None)
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, Step, TEXT("무기 크기를 선택하세요."));
		}
	}
	else if (Step == ECFWeaponGuideStep::MountGeometry)
	{
		ValidateTurretDraft(OutIssues);
	}
	else if (Step == ECFWeaponGuideStep::FireBehavior)
	{
		ValidateWeaponDraft(OutIssues);
	}
	else if (Step == ECFWeaponGuideStep::Projectile)
	{
		ValidateProjectileDraft(OutIssues);
	}
	else if (Step == ECFWeaponGuideStep::Damage)
	{
		ValidateDamageDraft(OutIssues);
	}
	else if (Step == ECFWeaponGuideStep::Ammo)
	{
		ValidateAmmoDraft(OutIssues);
	}
	else if (Step == ECFWeaponGuideStep::Launcher)
	{
		ValidateLauncherDraft(OutIssues);
	}
	else if (Step == ECFWeaponGuideStep::Propulsion)
	{
		ValidatePropulsionDraft(OutIssues);
	}
	else if (Step == ECFWeaponGuideStep::MissileFlight)
	{
		ValidateMissileFlightDraft(OutIssues);
	}
	else if (Step == ECFWeaponGuideStep::Guidance)
	{
		ValidateGuidanceDraft(OutIssues);
	}
	else if (Step == ECFWeaponGuideStep::Heat)
	{
		ValidateHeatDraft(OutIssues);
	}
	else if (Step == ECFWeaponGuideStep::Charge)
	{
		ValidateChargeDraft(OutIssues);
	}
	else if (Step == ECFWeaponGuideStep::ReviewCreate || Step == ECFWeaponGuideStep::Complete)
	{
		ValidateAll(OutIssues);
	}

	return !HasBlockingIssue(OutIssues);
}

// 완료 전에 exact 전체 draft와 child reference graph를 fresh 검증합니다.
bool FCFWeaponGuideVM::ValidateAll(TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	OutIssues.Reset();

	// 항상 필요한 공통 Step과 HitScan virtual projectile 내부 계약까지 검사할 Stable Step 목록입니다.
	const ECFWeaponGuideStep RequiredSteps[] =
	{
		ECFWeaponGuideStep::IdentityTemplate,
		ECFWeaponGuideStep::MountCompatibility,
		ECFWeaponGuideStep::MountGeometry,
		ECFWeaponGuideStep::FireBehavior,
		ECFWeaponGuideStep::Projectile,
		ECFWeaponGuideStep::Damage,
		ECFWeaponGuideStep::Ammo,
		ECFWeaponGuideStep::Launcher,
		ECFWeaponGuideStep::Propulsion,
		ECFWeaponGuideStep::MissileFlight,
		ECFWeaponGuideStep::Guidance,
		ECFWeaponGuideStep::Heat,
		ECFWeaponGuideStep::Charge
	};

	for (const ECFWeaponGuideStep Step : RequiredSteps)
	{
		// 한 semantic Step의 validation 진단입니다.
		TArray<FCFWeaponGuideIssue> StepIssues;
		ValidateStep(Step, StepIssues);
		OutIssues.Append(StepIssues);
	}

	ValidateExistingReferenceConsistency(OutIssues);
	return !HasBlockingIssue(OutIssues);
}

// 현재 validation을 사용자용 여러 줄 문자열로 구성합니다.
FString FCFWeaponGuideVM::BuildValidationSummary() const
{
	// 전체 validation 진단입니다.
	TArray<FCFWeaponGuideIssue> Issues;
	ValidateAll(Issues);
	if (Issues.IsEmpty())
	{
		return TEXT("검증 PASS — 현재 무장 구성에서 blocking issue가 없습니다.");
	}
	// 사용자용 진단 행 목록입니다.
	TArray<FString> Lines;
	for (const FCFWeaponGuideIssue& Issue : Issues)
	{
		// Severity prefix입니다.
		const TCHAR* Prefix = Issue.Severity == ECFWeaponGuideIssueSeverity::Error
			? TEXT("[오류]")
			: (Issue.Severity == ECFWeaponGuideIssueSeverity::Warning ? TEXT("[경고]") : TEXT("[정보]"));
		Lines.Add(FString::Printf(TEXT("%s %s: %s"), Prefix, *GetStepDisplayName(Issue.Step), *Issue.Message));
	}
	return FString::Join(Lines, TEXT("\n"));
}

// 신규 Damage Draft를 existing CFDADamageProvider의 fresh Create Preview까지 mutation0 검토합니다.
bool FCFWeaponGuideVM::BuildDamageProviderPreview(FString& OutSummary, FString& OutError) const
{
	if (Draft.Damage.Mode != ECFWeaponGuideChildMode::CreateNew)
	{
		OutError = TEXT("DamageData가 신규 생성 모드가 아닙니다.");
		OutSummary.Reset();
		return false;
	}
	if (!SessionCreatedDamagePath.IsNull())
	{
		OutSummary = FString::Printf(
			TEXT("신규 DamageData provider durable 확인됨\nProvider: CFDADamageProvider\n대상: %s\n상태: DurableApplied / same-session recovery 재사용"),
			*SessionCreatedDamagePath.ToString());
		OutError.Reset();
		return true;
	}

	// Memory-only provider source JSON입니다.
	FString ProviderJson;
	// Fresh current truth를 반영한 exact Create Preview입니다.
	FCFDACommonPreviewRow PreviewRow;
	if (!BuildDamageProviderCreateCandidate(
		Draft,
		GetDerivedDamageId(),
		GetDerivedDamageObjectPath(),
		ProviderJson,
		PreviewRow,
		OutError))
	{
		OutSummary.Reset();
		return false;
	}

	OutSummary = FString::Printf(
		TEXT("신규 DamageData provider 검토 PASS\nProvider: CFDADamageProvider\n대상: %s\nSemantic Fingerprint: %s\n상태: ReviewedMutationReady / fresh Create / 아직 저장하지 않음"),
		*GetDerivedDamageObjectPath(),
		*PreviewRow.Envelope.StagingSemanticFingerprint);
	OutError.Reset();
	return true;
}

// 신규 Ammo Draft를 existing CFDAAmmoProvider의 fresh Create Preview까지 mutation0 검토합니다.
bool FCFWeaponGuideVM::BuildAmmoProviderPreview(FString& OutSummary, FString& OutError) const
{
	if (Draft.Ammo.Mode != ECFWeaponGuideChildMode::CreateNew)
	{
		OutError = TEXT("AmmoData가 신규 생성 모드가 아닙니다.");
		OutSummary.Reset();
		return false;
	}
	if (!SessionCreatedAmmoPath.IsNull())
	{
		OutSummary = FString::Printf(
			TEXT("신규 AmmoData provider durable 확인됨\nProvider: CFDAAmmoProvider\n대상: %s\n상태: DurableApplied / same-session recovery 재사용"),
			*SessionCreatedAmmoPath.ToString());
		OutError.Reset();
		return true;
	}

	// Memory-only provider source JSON입니다.
	FString ProviderJson;
	// Fresh current truth를 반영한 exact Create Preview입니다.
	FCFDACommonPreviewRow PreviewRow;
	if (!BuildAmmoProviderCreateCandidate(
		Draft,
		GetDerivedAmmoId(),
		GetDerivedAmmoObjectPath(),
		ProviderJson,
		PreviewRow,
		OutError))
	{
		OutSummary.Reset();
		return false;
	}

	OutSummary = FString::Printf(
		TEXT("신규 AmmoData provider 검토 PASS\nProvider: CFDAAmmoProvider\n대상: %s\nSemantic Fingerprint: %s\n상태: ReviewedMutationReady / fresh Create / 아직 저장하지 않음"),
		*GetDerivedAmmoObjectPath(),
		*PreviewRow.Envelope.StagingSemanticFingerprint);
	OutError.Reset();
	return true;
}

// 현재 draft에서 provider exact2 + Guide-local exact3 child를 순차 durable 적용하고 완료 Bundle을 확정합니다.
bool FCFWeaponGuideVM::CreateChildrenAndBuildResult(FString& OutError)
{
	// 모든 visible/virtual Step과 existing reference consistency를 mutation 전에 다시 검증합니다.
	TArray<FCFWeaponGuideIssue> Issues;
	if (!ValidateAll(Issues))
	{
		OutError = Issues.IsEmpty() ? TEXT("전체 validation에 실패했습니다.") : Issues[0].Message;
		LastStatusMessage = OutError;
		return false;
	}

	// 아직 생성되지 않은 exact target과 same-session confirmed child를 한 번 더 fresh preflight합니다.
	if (!ValidateFreshDurablePreflight(OutError))
	{
		LastStatusMessage = OutError;
		return false;
	}

	// 최종 DamageData path입니다.
	FSoftObjectPath DamagePath = Draft.Damage.Mode == ECFWeaponGuideChildMode::ReuseExisting
		? Draft.Damage.ExistingAssetPath
		: SessionCreatedDamagePath;
	if (Draft.Damage.Mode == ECFWeaponGuideChildMode::CreateNew && DamagePath.IsNull())
	{
		if (!CreateDamageAsset(DamagePath, OutError))
		{
			LastStatusMessage = TEXT("DamageData durable Create에 실패했습니다. mutation 결과가 불확실한 경우 새 child를 자동 삭제하지 않습니다. ") + OutError;
			return false;
		}
		SessionCreatedDamagePath = DamagePath;
#if WITH_DEV_AUTOMATION_TESTS
		if (TestDurableInterruptionStep.IsSet() && TestDurableInterruptionStep.GetValue() == ECFWeaponGuideStep::Damage)
		{
			OutError = TEXT("Automation: confirmed Damage durable 직후 의도적으로 중단했습니다.");
			LastStatusMessage = OutError;
			return false;
		}
#endif
	}

	// 최종 finite AmmoData path입니다. 무한 탄약 구성에서는 비워 둡니다.
	FSoftObjectPath AmmoPath;
	if (Draft.Weapon.bUseFiniteAmmo)
	{
		AmmoPath = Draft.Ammo.Mode == ECFWeaponGuideChildMode::ReuseExisting
			? Draft.Ammo.ExistingAssetPath
			: SessionCreatedAmmoPath;
		if (Draft.Ammo.Mode == ECFWeaponGuideChildMode::CreateNew && AmmoPath.IsNull())
		{
			if (!CreateAmmoAsset(AmmoPath, OutError))
			{
				LastStatusMessage = TEXT("DamageData는 이미 저장됐을 수 있지만 AmmoData durable Create에 실패했습니다. confirmed child는 same-session retry에서 재사용합니다. ") + OutError;
				return false;
			}
			SessionCreatedAmmoPath = AmmoPath;
#if WITH_DEV_AUTOMATION_TESTS
			if (TestDurableInterruptionStep.IsSet() && TestDurableInterruptionStep.GetValue() == ECFWeaponGuideStep::Ammo)
			{
				OutError = TEXT("Automation: confirmed Damage/Ammo durable 직후 의도적으로 중단했습니다.");
				LastStatusMessage = OutError;
				return false;
			}
#endif
		}
	}

	// 최종 TurretMountData path입니다.
	FSoftObjectPath TurretPath = Draft.Turret.Mode == ECFWeaponGuideChildMode::ReuseExisting
		? Draft.Turret.ExistingAssetPath
		: SessionCreatedTurretPath;
	if (Draft.Turret.Mode == ECFWeaponGuideChildMode::CreateNew && TurretPath.IsNull())
	{
		if (!CreateTurretAsset(TurretPath, OutError))
		{
			LastStatusMessage = TEXT("앞선 Damage/Ammo는 이미 저장됐을 수 있지만 TurretMountData 생성에 실패했습니다. confirmed child는 same-session retry에서 재사용합니다. ") + OutError;
			return false;
		}
		SessionCreatedTurretPath = TurretPath;
#if WITH_DEV_AUTOMATION_TESTS
		if (TestDurableInterruptionStep.IsSet() && TestDurableInterruptionStep.GetValue() == ECFWeaponGuideStep::MountGeometry)
		{
			OutError = TEXT("Automation: confirmed Damage/Ammo/Turret durable 직후 의도적으로 중단했습니다.");
			LastStatusMessage = OutError;
			return false;
		}
#endif
	}

	// 최종 ProjectileData path입니다.
	FSoftObjectPath ProjectilePath = Draft.Projectile.Mode == ECFWeaponGuideChildMode::ReuseExisting
		? Draft.Projectile.ExistingAssetPath
		: SessionCreatedProjectilePath;
	if (Draft.Projectile.Mode == ECFWeaponGuideChildMode::CreateNew && ProjectilePath.IsNull())
	{
		if (!CreateProjectileAsset(DamagePath, ProjectilePath, OutError))
		{
			LastStatusMessage = TEXT("앞선 child는 이미 저장됐을 수 있지만 ProjectileData 생성에 실패했습니다. confirmed child는 same-session retry에서 재사용합니다. ") + OutError;
			return false;
		}
		SessionCreatedProjectilePath = ProjectilePath;
	}

	// 최종 WeaponData path입니다.
	FSoftObjectPath WeaponPath = Draft.Weapon.Mode == ECFWeaponGuideChildMode::ReuseExisting
		? Draft.Weapon.ExistingAssetPath
		: SessionCreatedWeaponPath;
	if (Draft.Weapon.Mode == ECFWeaponGuideChildMode::CreateNew && WeaponPath.IsNull())
	{
		if (!CreateWeaponAsset(ProjectilePath, AmmoPath, WeaponPath, OutError))
		{
			LastStatusMessage = TEXT("앞선 child는 이미 저장됐을 수 있지만 WeaponData 생성에 실패했습니다. confirmed child는 same-session retry에서 재사용합니다. ") + OutError;
			return false;
		}
		SessionCreatedWeaponPath = WeaponPath;
	}

	// 모든 child 저장 완료 뒤 disk-backed exact reference graph를 다시 읽어 검증합니다.
	if (!ValidateDurableReferenceGraph(TurretPath, DamagePath, AmmoPath, ProjectilePath, WeaponPath, OutError))
	{
		LastStatusMessage = TEXT("durable child는 저장됐지만 최종 reference graph readback을 확정하지 못했습니다. 자동 rollback/delete하지 않습니다. ") + OutError;
		return false;
	}

	// Exact graph readback까지 통과한 뒤에만 완료 Bundle을 확정합니다.
	FCFWeaponGuideResultBundle NewBundle;
	if (!BuildResultBundle(NewBundle, OutError))
	{
		LastStatusMessage = OutError;
		return false;
	}

	// P0-03 Bundle fingerprint는 그대로 두고 P0-04 USER summary/handoff 의미를 별도 semantic fingerprint로 고정합니다.
	FString NewCompletionSemanticFingerprint;
	if (!BuildCompletionSemanticFingerprint(NewBundle, NewCompletionSemanticFingerprint, OutError))
	{
		LastStatusMessage = TEXT("durable graph는 확정됐지만 P0-04 completion semantic snapshot을 고정하지 못했습니다. ") + OutError;
		return false;
	}
	ResultBundle = MoveTemp(NewBundle);
	CompletionSemanticFingerprint = MoveTemp(NewCompletionSemanticFingerprint);
	CurrentStep = ECFWeaponGuideStep::Complete;
	LastStatusMessage = TEXT("무장 child exact5 durable Create/reuse와 reference graph readback을 확정했습니다. EquipmentPreset은 생성하거나 저장하지 않았습니다.");
	OutError.Reset();
	return true;
}

// Product 신규 TurretMountData object path를 반환합니다.
FString FCFWeaponGuideVM::GetDerivedTurretObjectPath() const
{
	return FString::Printf(TEXT("%s/TurretMounts/DA_%s_Mount.DA_%s_Mount"), *GetAssetRoot(), *Draft.BaseAssetName, *Draft.BaseAssetName);
}

// Product 신규 WeaponData object path를 반환합니다.
FString FCFWeaponGuideVM::GetDerivedWeaponObjectPath() const
{
	return FString::Printf(TEXT("%s/WeaponDefs/DA_%s_Weapon.DA_%s_Weapon"), *GetAssetRoot(), *Draft.BaseAssetName, *Draft.BaseAssetName);
}

// Product 신규 ProjectileData object path를 반환합니다.
FString FCFWeaponGuideVM::GetDerivedProjectileObjectPath() const
{
	return FString::Printf(TEXT("%s/ProjectileDefs/DA_%s_Projectile.DA_%s_Projectile"), *GetAssetRoot(), *Draft.BaseAssetName, *Draft.BaseAssetName);
}

// Product 신규 DamageData object path를 반환합니다.
FString FCFWeaponGuideVM::GetDerivedDamageObjectPath() const
{
	return FString::Printf(TEXT("%s/DamageDefs/DA_%s_Damage.DA_%s_Damage"), *GetAssetRoot(), *Draft.BaseAssetName, *Draft.BaseAssetName);
}

// Product 신규 AmmoData object path를 반환합니다.
FString FCFWeaponGuideVM::GetDerivedAmmoObjectPath() const
{
	return FString::Printf(TEXT("%s/AmmoDefs/DA_%s_Ammo.DA_%s_Ammo"), *GetAssetRoot(), *Draft.BaseAssetName, *Draft.BaseAssetName);
}

// 신규 TurretMountData의 자동 logical ID를 반환합니다.
FName FCFWeaponGuideVM::GetDerivedTurretId() const
{
	return Draft.BaseAssetName.IsEmpty() ? NAME_None : FName(*FString::Printf(TEXT("%s_Mount"), *Draft.BaseAssetName));
}

// 신규 WeaponData의 자동 logical ID를 반환합니다.
FName FCFWeaponGuideVM::GetDerivedWeaponId() const
{
	return Draft.BaseAssetName.IsEmpty() ? NAME_None : FName(*Draft.BaseAssetName);
}

// 신규 ProjectileData의 자동 logical ID를 반환합니다.
FName FCFWeaponGuideVM::GetDerivedProjectileId() const
{
	return Draft.BaseAssetName.IsEmpty() ? NAME_None : FName(*FString::Printf(TEXT("%s_Projectile"), *Draft.BaseAssetName));
}

// 신규 DamageData의 자동 logical ID를 반환합니다.
FName FCFWeaponGuideVM::GetDerivedDamageId() const
{
	return Draft.BaseAssetName.IsEmpty() ? NAME_None : FName(*FString::Printf(TEXT("%s_Damage"), *Draft.BaseAssetName));
}

// 신규 AmmoData의 자동 logical ID를 반환합니다.
FName FCFWeaponGuideVM::GetDerivedAmmoId() const
{
	return Draft.BaseAssetName.IsEmpty() ? NAME_None : FName(*FString::Printf(TEXT("%s_Ammo"), *Draft.BaseAssetName));
}

// 현재 단계의 사용자 표시 이름을 반환합니다.
FString FCFWeaponGuideVM::GetStepDisplayName(const ECFWeaponGuideStep Step)
{
	switch (Step)
	{
	case ECFWeaponGuideStep::IdentityTemplate: return TEXT("기본 정보 / 제작 방식");
	case ECFWeaponGuideStep::MountCompatibility: return TEXT("장착 호환성");
	case ECFWeaponGuideStep::MountGeometry: return TEXT("장착 / 포탑 구조");
	case ECFWeaponGuideStep::FireBehavior: return TEXT("발사 동작");
	case ECFWeaponGuideStep::Projectile: return TEXT("발사체");
	case ECFWeaponGuideStep::Damage: return TEXT("피해");
	case ECFWeaponGuideStep::Ammo: return TEXT("탄약");
	case ECFWeaponGuideStep::Launcher: return TEXT("런처 / 다중 발사");
	case ECFWeaponGuideStep::Propulsion: return TEXT("자체 추진");
	case ECFWeaponGuideStep::MissileFlight: return TEXT("미사일 비행");
	case ECFWeaponGuideStep::Guidance: return TEXT("유도");
	case ECFWeaponGuideStep::Heat: return TEXT("열");
	case ECFWeaponGuideStep::Charge: return TEXT("충전");
	case ECFWeaponGuideStep::ReviewCreate: return TEXT("최종 검토 / 생성");
	case ECFWeaponGuideStep::Complete: return TEXT("완료");
	default: return TEXT("알 수 없는 단계");
	}
}

#if WITH_DEV_AUTOMATION_TESTS
// Automation fixture가 Product namespace 대신 disposable root를 사용하도록 설정합니다.
void FCFWeaponGuideVM::SetDisposableTestRoot(const FString& TestRoot)
{
	DisposableTestRoot = TestRoot;
	DisposableTestRoot.TrimEndInline();
	while (DisposableTestRoot.EndsWith(TEXT("/")))
	{
		DisposableTestRoot.LeftChopInline(1);
	}
}

// Confirmed durable child 직후 의도적 중단을 주입해 same-session partial recovery를 검증합니다.
void FCFWeaponGuideVM::SetDurableInterruptionStepForTests(const ECFWeaponGuideStep Step)
{
	TestDurableInterruptionStep = Step;
}

// Automation partial-recovery 중단 주입을 해제합니다.
void FCFWeaponGuideVM::ClearDurableInterruptionForTests()
{
	TestDurableInterruptionStep.Reset();
}
#endif

// 사용자 입력 기본 이름이 Unreal asset leaf name으로 안전한지 검증합니다.
bool FCFWeaponGuideVM::ValidateBaseAssetName(FString& OutError) const
{
	if (Draft.BaseAssetName.IsEmpty())
	{
		OutError = TEXT("무장 에셋 이름을 입력하세요.");
		return false;
	}
	if (Draft.BaseAssetName.Contains(TEXT("/")) || Draft.BaseAssetName.Contains(TEXT(".")) || Draft.BaseAssetName.Contains(TEXT("\\")) || Draft.BaseAssetName.Contains(TEXT(" ")))
	{
		OutError = TEXT("에셋 이름에는 경로 구분자, 점, 공백을 사용할 수 없습니다. 예: HeavyCannon_Mk2");
		return false;
	}
	// 파생된 대표 object path입니다.
	const FSoftObjectPath ProbePath(GetDerivedWeaponObjectPath());
	if (!ProbePath.IsValid())
	{
		OutError = TEXT("입력한 에셋 이름으로 유효한 Unreal object path를 만들 수 없습니다.");
		return false;
	}
	OutError.Reset();
	return true;
}

// exact typed existing child가 persisted clean state인지 검증합니다.
bool FCFWeaponGuideVM::ValidateExistingChild(
	const FSoftObjectPath& ObjectPath,
	UClass* ExpectedClass,
	const FString& UserLabel,
	FString& OutError) const
{
	if (ObjectPath.IsNull() || !ObjectPath.IsValid())
	{
		OutError = FString::Printf(TEXT("%s를 선택하세요."), *UserLabel);
		return false;
	}
	// Existing exact child asset입니다.
	UObject* Asset = ObjectPath.TryLoad();
	if (Asset == nullptr || ExpectedClass == nullptr || !Asset->IsA(ExpectedClass))
	{
		OutError = FString::Printf(TEXT("선택한 %s가 존재하지 않거나 타입이 맞지 않습니다: %s"), *UserLabel, *ObjectPath.ToString());
		return false;
	}
	// Existing child package입니다.
	UPackage* Package = Asset->GetOutermost();
	if (Package == nullptr || Package->IsDirty() || !FPackageName::DoesPackageExist(Package->GetName()))
	{
		OutError = FString::Printf(TEXT("%s는 저장된 clean asset만 재사용할 수 있습니다. 현재 package가 dirty이거나 persisted 확인이 되지 않습니다."), *UserLabel);
		return false;
	}
	OutError.Reset();
	return true;
}

// 신규 child target이 memory/disk 어디에도 없는지 fail-closed 검증합니다.
bool FCFWeaponGuideVM::ValidateCreateTargetAbsent(const FString& ObjectPath, FString& OutError) const
{
	// Target soft object path입니다.
	const FSoftObjectPath TargetPath(ObjectPath);
	// Target package long name입니다.
	const FString PackageName = FPackageName::ObjectPathToPackageName(ObjectPath);
	if (!TargetPath.IsValid() || !FPackageName::IsValidLongPackageName(PackageName))
	{
		OutError = FString::Printf(TEXT("신규 child target path가 유효하지 않습니다: %s"), *ObjectPath);
		return false;
	}
	if (TargetPath.ResolveObject() != nullptr || FindPackage(nullptr, *PackageName) != nullptr || FPackageName::DoesPackageExist(PackageName))
	{
		OutError = FString::Printf(TEXT("같은 이름의 child DataAsset이 이미 존재합니다. 기존 자산 재사용을 선택하거나 다른 무장 에셋 이름을 사용하세요: %s"), *ObjectPath);
		return false;
	}
	OutError.Reset();
	return true;
}

// 현재 Turret 선택/신규 draft를 검증합니다.
bool FCFWeaponGuideVM::ValidateTurretDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	if (Draft.Turret.Mode == ECFWeaponGuideChildMode::ReuseExisting)
	{
		// Existing Turret validation 오류입니다.
		FString Error;
		if (!ValidateExistingChild(Draft.Turret.ExistingAssetPath, UCFTurretMountData::StaticClass(), TEXT("TurretMountData"), Error))
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::MountGeometry, Error);
		}
		return !HasBlockingIssue(OutIssues);
	}
	if (Draft.Turret.PrimaryMuzzleSocketName.IsNone())
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::MountGeometry, TEXT("기본 Muzzle Socket 이름은 비울 수 없습니다."));
	}
	if (Draft.Turret.MinYawDeg > Draft.Turret.MaxYawDeg || Draft.Turret.MinPitchDeg > Draft.Turret.MaxPitchDeg)
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::MountGeometry, TEXT("Yaw/Pitch 최소각은 최대각보다 클 수 없습니다."));
	}
	if (!FMath::IsFinite(Draft.Turret.YawTurnRateDegPerSec) || !FMath::IsFinite(Draft.Turret.PitchTurnRateDegPerSec)
		|| Draft.Turret.YawTurnRateDegPerSec < 0.0f || Draft.Turret.PitchTurnRateDegPerSec < 0.0f)
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::MountGeometry, TEXT("Yaw/Pitch 회전 속도는 0 이상의 유한값이어야 합니다."));
	}
	if (!FMath::IsFinite(Draft.Turret.TurretMountWeightKg) || Draft.Turret.TurretMountWeightKg < 0.0f)
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::MountGeometry, TEXT("TurretMount 질량은 0 이상의 유한값이어야 합니다."));
	}
	// New target collision validation 오류입니다.
	FString TargetError;
	if (SessionCreatedTurretPath.IsNull() && !ValidateCreateTargetAbsent(GetDerivedTurretObjectPath(), TargetError))
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::MountGeometry, TargetError);
	}
	return !HasBlockingIssue(OutIssues);
}

// 현재 Weapon 선택/신규 draft와 mount/size/finite-ammo 계약을 검증합니다.
bool FCFWeaponGuideVM::ValidateWeaponDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	if (Draft.Weapon.Mode == ECFWeaponGuideChildMode::ReuseExisting)
	{
		// Existing Weapon validation 오류입니다.
		FString Error;
		if (!ValidateExistingChild(Draft.Weapon.ExistingAssetPath, UCFWeaponData::StaticClass(), TEXT("WeaponData"), Error))
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::FireBehavior, Error);
			return false;
		}
		// Existing Weapon exact asset입니다.
		UCFWeaponData* WeaponData = Cast<UCFWeaponData>(Draft.Weapon.ExistingAssetPath.TryLoad());
		// Native Weapon validator 오류 목록입니다.
		TArray<FText> ValidationErrors;
		if (WeaponData == nullptr || !WeaponData->ValidateWeaponDataContract(ValidationErrors))
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::FireBehavior,
				ValidationErrors.IsEmpty() ? TEXT("기존 WeaponData가 native contract validation을 통과하지 못했습니다.") : ValidationErrors[0].ToString());
		}
		if (WeaponData && !WeaponData->SupportsMountType(Draft.RequiredMountType))
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::FireBehavior, TEXT("선택한 WeaponData가 현재 Required Mount Type을 지원하지 않습니다."));
		}
		if (WeaponData && WeaponData->WeaponSize != Draft.RequiredWeaponSize)
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::FireBehavior, TEXT("선택한 WeaponData의 WeaponSize와 Guide의 Required Weapon Size가 다릅니다."));
		}
		return !HasBlockingIssue(OutIssues);
	}

	// 신규 Weapon draft를 native validator로 검사할 transient object입니다.
	UCFWeaponData* ProbeWeapon = NewObject<UCFWeaponData>(GetTransientPackage());
	if (ProbeWeapon == nullptr)
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::FireBehavior, TEXT("Weapon validation용 transient object를 만들지 못했습니다."));
		return false;
	}
	// 후속 child 선택 전에도 native scalar/mount/ammo 계약을 검사합니다.
	FCFWeaponGuideWeaponPayload ProbePayload;
	ProbePayload.WeaponId = GetDerivedWeaponId();
	ProbePayload.WeaponSize = Draft.RequiredWeaponSize;
	ProbePayload.CompatibleMountType = Draft.RequiredMountType;
	ProbePayload.WeaponMassKg = Draft.Weapon.WeaponMassKg;
	ProbePayload.FireMode = Draft.Weapon.FireMode;
	ProbePayload.FireRatePerMinute = Draft.Weapon.FireRatePerMinute;
	ProbePayload.MaxRange = Draft.Weapon.MaxRange;
	ProbePayload.SpreadDeg = Draft.Weapon.SpreadDeg;
	ProbePayload.MagazineSize = Draft.Weapon.MagazineSize;
	ProbePayload.InitialLoadedAmmoCount = Draft.Weapon.InitialLoadedAmmoCount;
	ProbePayload.AmmoUnitsPerShot = Draft.Weapon.AmmoUnitsPerShot;
	ProbePayload.ReloadTimeSeconds = Draft.Weapon.ReloadTimeSeconds;
	// AmmoData 선택은 별도 Ammo Step 소유이므로 FireBehavior native probe에서는 finite-ammo reference requirement만 지연합니다.
	ProbePayload.bUseFiniteAmmo = false;
	ProbePayload.AmmoDataPath = FSoftObjectPath();
	ProbePayload.AmmoTypeId = Draft.Weapon.AmmoTypeId;
	ProbePayload.LauncherFirePatternConfig = Draft.Weapon.LauncherFirePatternConfig;
	ProbePayload.LauncherReleaseConfig = Draft.Weapon.LauncherReleaseConfig;
	ProbePayload.bUseHeat = Draft.Weapon.bUseHeat;
	ProbePayload.HeatPerShot = Draft.Weapon.HeatPerShot;
	ProbePayload.MaxHeat = Draft.Weapon.MaxHeat;
	ProbePayload.HeatDissipationPerSecond = Draft.Weapon.HeatDissipationPerSecond;
	ProbePayload.bUseCharge = Draft.Weapon.bUseCharge;
	ProbePayload.MaximumWeaponCharge = Draft.Weapon.MaximumWeaponCharge;
	ProbePayload.InitialWeaponCharge = Draft.Weapon.InitialWeaponCharge;
	ProbePayload.WeaponChargePerShot = Draft.Weapon.WeaponChargePerShot;
	ProbePayload.WeaponChargeRecoveryPerSecond = Draft.Weapon.WeaponChargeRecoveryPerSecond;
	MaterializeWeaponPayload(*ProbeWeapon, ProbePayload);
	if (Draft.Weapon.bUseFiniteAmmo
		&& (Draft.Weapon.MagazineSize <= 0
			|| Draft.Weapon.InitialLoadedAmmoCount < 0
			|| Draft.Weapon.InitialLoadedAmmoCount > Draft.Weapon.MagazineSize
			|| Draft.Weapon.AmmoUnitsPerShot <= 0))
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::FireBehavior,
			TEXT("유한 탄약을 사용할 때 MagazineSize는 1 이상, InitialLoaded는 0..MagazineSize, AmmoUnitsPerShot은 1 이상이어야 합니다."));
	}
	// Native validator의 detailed error 목록입니다.
	TArray<FText> NativeErrors;
	if (!ProbeWeapon->ValidateWeaponDataContract(NativeErrors))
	{
		for (const FText& NativeError : NativeErrors)
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::FireBehavior, NativeError.ToString());
		}
	}
	if (SessionCreatedWeaponPath.IsNull())
	{
		// New Weapon target collision validation 오류입니다.
		FString TargetError;
		if (!ValidateCreateTargetAbsent(GetDerivedWeaponObjectPath(), TargetError))
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::FireBehavior, TargetError);
		}
	}
	return !HasBlockingIssue(OutIssues);
}

// physical 또는 virtual ProjectileData draft를 검증합니다.
bool FCFWeaponGuideVM::ValidateProjectileDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	if (Draft.Projectile.Mode == ECFWeaponGuideChildMode::ReuseExisting)
	{
		// Existing Projectile validation 오류입니다.
		FString Error;
		if (!ValidateExistingChild(Draft.Projectile.ExistingAssetPath, UCFProjectileData::StaticClass(), TEXT("ProjectileData"), Error))
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Projectile, Error);
			return false;
		}

		// Existing Projectile exact asset입니다.
		UCFProjectileData* ProjectileData = Cast<UCFProjectileData>(Draft.Projectile.ExistingAssetPath.TryLoad());
		if (Draft.Weapon.FireMode == ECFWeaponFireMode::Projectile
			&& (ProjectileData == nullptr || !ProjectileData->HasProjectileActorClass()))
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Projectile,
				TEXT("Projectile 발사 모드에서는 ProjectileData.ProjectileActorClass가 필요합니다."));
		}
		return !HasBlockingIssue(OutIssues);
	}

	if (GetDerivedProjectileId().IsNone())
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Projectile,
			TEXT("Projectile ID를 파생할 수 없습니다. 기본 에셋 이름을 확인하세요."));
	}

	if (Draft.Weapon.FireMode == ECFWeaponFireMode::Projectile)
	{
		if (Draft.Projectile.ProjectileActorClassPath.IsNull())
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Projectile,
				TEXT("Projectile 발사 모드에서는 Projectile Actor Class를 선택하세요."));
		}

		if (!FMath::IsFinite(Draft.Projectile.InitialSpeed) || Draft.Projectile.InitialSpeed < 0.0f
			|| !FMath::IsFinite(Draft.Projectile.LifeTimeSeconds) || Draft.Projectile.LifeTimeSeconds <= 0.0f
			|| !FMath::IsFinite(Draft.Projectile.CollisionRadius) || Draft.Projectile.CollisionRadius < 0.0f)
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Projectile,
				TEXT("Projectile 속도/수명/충돌 반경 값이 유효하지 않습니다."));
		}
	}

	if (SessionCreatedProjectilePath.IsNull())
	{
		// New Projectile target collision validation 오류입니다.
		FString TargetError;
		if (!ValidateCreateTargetAbsent(GetDerivedProjectileObjectPath(), TargetError))
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Projectile, TargetError);
		}
	}

	return !HasBlockingIssue(OutIssues);
}

// DamageData create/reuse draft를 검증합니다.
bool FCFWeaponGuideVM::ValidateDamageDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	if (Draft.Damage.Mode == ECFWeaponGuideChildMode::CreateNew)
	{
		if (!SessionCreatedDamagePath.IsNull())
		{
			if (!SessionCreatedDamagePath.ToString().Equals(GetDerivedDamageObjectPath(), ESearchCase::CaseSensitive))
			{
				AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Damage,
					TEXT("세션에서 이미 저장한 DamageData path가 현재 derived target과 다릅니다."));
				return false;
			}

			// Same-session recovery에서 재사용할 persisted DamageData입니다.
			FString ExistingError;
			if (!ValidateExistingChild(SessionCreatedDamagePath, UCFDamageData::StaticClass(), TEXT("세션 생성 DamageData"), ExistingError))
			{
				AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Damage, ExistingError);
				return false;
			}
			// Current Draft exact12 desired payload입니다.
			const FCFDADamagePayload DesiredPayload = BuildDamageProviderPayload(Draft, GetDerivedDamageId());
			// Current Draft desired semantic fingerprint입니다.
			FString DesiredFingerprint;
			if (!CFDADamageProviderImpl::BuildSemanticFingerprint(DesiredPayload, DesiredFingerprint, ExistingError))
			{
				AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Damage, ExistingError);
				return false;
			}
			// Persisted session DamageData입니다.
			UCFDamageData* SessionDamage = Cast<UCFDamageData>(SessionCreatedDamagePath.TryLoad());
			if (SessionDamage == nullptr
				|| !CFDADurableCore::ValidateTypedAssetFingerprint<UCFDamageData, FCFDADamagePayload>(
					*SessionDamage,
					DesiredFingerprint,
					&CFDADamageProviderImpl::ExtractPayload,
					&CFDADamageProviderImpl::BuildSemanticFingerprint,
					ExistingError))
			{
				AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Damage,
					TEXT("이미 durable 저장한 DamageData가 현재 Draft와 달라 안전하게 재사용할 수 없습니다. ") + ExistingError);
				return false;
			}
			return true;
		}

		// 신규 Damage target collision validation 오류입니다.
		FString TargetError;
		if (!ValidateCreateTargetAbsent(GetDerivedDamageObjectPath(), TargetError))
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Damage, TargetError);
			return false;
		}

		// Existing Damage provider에 매핑한 mutation0 preview 결과입니다.
		FString ProviderSummary;
		// Existing Damage provider preview 실패 상세입니다.
		FString ProviderError;
		if (!BuildDamageProviderPreview(ProviderSummary, ProviderError))
		{
			AddIssue(
				OutIssues,
				ECFWeaponGuideIssueSeverity::Error,
				ECFWeaponGuideStep::Damage,
				TEXT("신규 DamageData Draft를 CFDADamageProvider 계약으로 검증하지 못했습니다. ") + ProviderError);
			return false;
		}
		return true;
	}

	// Existing Damage validation 오류입니다.
	FString DamageError;
	if (!ValidateExistingChild(Draft.Damage.ExistingAssetPath, UCFDamageData::StaticClass(), TEXT("DamageData"), DamageError))
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Damage, DamageError);
	}
	return !HasBlockingIssue(OutIssues);
}

// finite ammo에서 AmmoData create/reuse draft를 검증합니다.
bool FCFWeaponGuideVM::ValidateAmmoDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	if (!Draft.Weapon.bUseFiniteAmmo)
	{
		if (Draft.Weapon.AmmoTypeId.IsNone())
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Warning, ECFWeaponGuideStep::Ammo,
				TEXT("무한 탄약 compatibility를 사용하지만 AmmoTypeId가 비어 있습니다. 런타임 식별이 필요하면 fallback 탄종 ID를 지정하세요."));
		}
		return true;
	}

	if (Draft.Ammo.Mode == ECFWeaponGuideChildMode::CreateNew)
	{
		if (Draft.Ammo.AmmoDisplayName.TrimStartAndEnd().IsEmpty())
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Ammo,
				TEXT("신규 AmmoData의 게임 표시 이름을 입력하세요."));
			return false;
		}

		if (!SessionCreatedAmmoPath.IsNull())
		{
			if (!SessionCreatedAmmoPath.ToString().Equals(GetDerivedAmmoObjectPath(), ESearchCase::CaseSensitive))
			{
				AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Ammo,
					TEXT("세션에서 이미 저장한 AmmoData path가 현재 derived target과 다릅니다."));
				return false;
			}

			// Same-session recovery에서 재사용할 persisted AmmoData입니다.
			FString ExistingError;
			if (!ValidateExistingChild(SessionCreatedAmmoPath, UCFAmmoData::StaticClass(), TEXT("세션 생성 AmmoData"), ExistingError))
			{
				AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Ammo, ExistingError);
				return false;
			}
			// Current Draft exact8 desired payload입니다.
			const FCFDAAmmoPayload DesiredPayload = BuildAmmoProviderPayload(Draft, GetDerivedAmmoId());
			// Current Draft desired semantic fingerprint입니다.
			FString DesiredFingerprint;
			if (!CFDAAmmoProviderImpl::BuildSemanticFingerprint(DesiredPayload, DesiredFingerprint, ExistingError))
			{
				AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Ammo, ExistingError);
				return false;
			}
			// Persisted session AmmoData입니다.
			UCFAmmoData* SessionAmmo = Cast<UCFAmmoData>(SessionCreatedAmmoPath.TryLoad());
			if (SessionAmmo == nullptr
				|| !CFDADurableCore::ValidateTypedAssetFingerprint<UCFAmmoData, FCFDAAmmoPayload>(
					*SessionAmmo,
					DesiredFingerprint,
					&CFDAAmmoProviderImpl::ExtractPayload,
					&CFDAAmmoProviderImpl::BuildSemanticFingerprint,
					ExistingError))
			{
				AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Ammo,
					TEXT("이미 durable 저장한 AmmoData가 현재 Draft와 달라 안전하게 재사용할 수 없습니다. ") + ExistingError);
				return false;
			}
			return true;
		}

		// 신규 Ammo target collision validation 오류입니다.
		FString TargetError;
		if (!ValidateCreateTargetAbsent(GetDerivedAmmoObjectPath(), TargetError))
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Ammo, TargetError);
			return false;
		}

		// Existing Ammo provider에 매핑한 mutation0 preview 결과입니다.
		FString ProviderSummary;
		// Existing Ammo provider preview 실패 상세입니다.
		FString ProviderError;
		if (!BuildAmmoProviderPreview(ProviderSummary, ProviderError))
		{
			AddIssue(
				OutIssues,
				ECFWeaponGuideIssueSeverity::Error,
				ECFWeaponGuideStep::Ammo,
				TEXT("신규 AmmoData Draft를 CFDAAmmoProvider 계약으로 검증하지 못했습니다. ") + ProviderError);
			return false;
		}
		return true;
	}

	// finite ammo exact source validation 오류입니다.
	FString AmmoError;
	if (!ValidateExistingChild(Draft.Ammo.ExistingAssetPath, UCFAmmoData::StaticClass(), TEXT("AmmoData"), AmmoError))
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Ammo,
			TEXT("유한 탄약을 사용하려면 유효한 AmmoData가 필요합니다. ") + AmmoError);
		return false;
	}

	// finite ammo exact asset입니다.
	UCFAmmoData* AmmoData = Cast<UCFAmmoData>(Draft.Ammo.ExistingAssetPath.TryLoad());
	if (AmmoData == nullptr || !AmmoData->IsAmmoDataValid())
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Ammo,
			TEXT("선택한 AmmoData가 유효한 AmmoId를 가지지 않습니다."));
	}
	return !HasBlockingIssue(OutIssues);
}

// Launcher conditional draft를 검증합니다.
bool FCFWeaponGuideVM::ValidateLauncherDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	if (!Draft.Weapon.bUseLauncher)
	{
		return true;
	}

	if (Draft.Weapon.LauncherFirePatternConfig.ProjectileCountPerTrigger < 1)
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Launcher,
			TEXT("Launcher ProjectileCountPerTrigger는 최소 1이어야 합니다."));
	}
	if (!FMath::IsFinite(Draft.Weapon.LauncherFirePatternConfig.InterMuzzleDelaySeconds)
		|| Draft.Weapon.LauncherFirePatternConfig.InterMuzzleDelaySeconds < 0.0f)
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Launcher,
			TEXT("Launcher Muzzle 간격은 0 이상의 유한값이어야 합니다."));
	}
	if (!FMath::IsFinite(Draft.Weapon.LauncherReleaseConfig.EjectionSpeed)
		|| Draft.Weapon.LauncherReleaseConfig.EjectionSpeed < 0.0f
		|| !FMath::IsFinite(Draft.Weapon.LauncherReleaseConfig.CarrierVelocityRatio))
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Launcher,
			TEXT("Launcher Release 사출 속도와 차량 속도 상속 비율이 유효하지 않습니다."));
	}
	return !HasBlockingIssue(OutIssues);
}

// Projectile propulsion conditional draft를 검증합니다.
bool FCFWeaponGuideVM::ValidatePropulsionDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	// current Projectile propulsion 설정입니다.
	const FCFProjectilePropulsionConfig& Config = Draft.Projectile.PropulsionConfig;
	if (!Config.bUsePropulsion)
	{
		return true;
	}

	if (!FMath::IsFinite(Config.IgnitionDelaySeconds) || Config.IgnitionDelaySeconds < 0.0f
		|| !FMath::IsFinite(Config.BurnDurationSeconds) || Config.BurnDurationSeconds < 0.0f
		|| !FMath::IsFinite(Config.ThrustAccelerationCmPerSecSq) || Config.ThrustAccelerationCmPerSecSq < 0.0f
		|| !FMath::IsFinite(Config.MaximumPropelledSpeed) || Config.MaximumPropelledSpeed <= 0.0f)
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Propulsion,
			TEXT("자체 추진을 사용할 때 점화 지연/연소 시간/추진 가속도는 0 이상, 최대 추진 속도는 0보다 큰 유한값이어야 합니다."));
	}
	return !HasBlockingIssue(OutIssues);
}

// Missile flight conditional draft를 검증합니다.
bool FCFWeaponGuideVM::ValidateMissileFlightDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	// current Missile Flight 설정입니다.
	const FCFMissileFlightConfig& Config = Draft.Projectile.MissileFlightConfig;
	if (!Config.bUseMissileFlight)
	{
		return true;
	}

	if (!FMath::IsFinite(Config.MinimumClearanceTimeSeconds) || Config.MinimumClearanceTimeSeconds < 0.0f || Config.MinimumClearanceTimeSeconds > 30.0f
		|| !FMath::IsFinite(Config.MinimumClearanceDistanceCm) || Config.MinimumClearanceDistanceCm < 0.0f || Config.MinimumClearanceDistanceCm > 1000000.0f
		|| !FMath::IsFinite(Config.TransitionDurationSeconds) || Config.TransitionDurationSeconds < 0.0f || Config.TransitionDurationSeconds > 30.0f
		|| (Config.bUseTerminalPhase && (!FMath::IsFinite(Config.TerminalPhaseStartDistanceCm) || Config.TerminalPhaseStartDistanceCm < 0.0f || Config.TerminalPhaseStartDistanceCm > 1000000.0f)))
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::MissileFlight,
			TEXT("미사일 비행의 Clearance/Transition/Terminal 값이 Runtime authored 범위를 벗어났습니다."));
	}
	return !HasBlockingIssue(OutIssues);
}

// Missile guidance conditional draft를 검증합니다.
bool FCFWeaponGuideVM::ValidateGuidanceDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	// current Missile Guidance 설정입니다.
	const FCFMissileGuideConfig& Config = Draft.Projectile.MissileGuideConfig;
	if (!Config.bUseGuidance)
	{
		return true;
	}

	if (Config.GuideMode == ECFMissileGuideMode::None)
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Guidance,
			TEXT("유도를 사용할 때 GuideMode는 None일 수 없습니다."));
	}

	if (!FMath::IsFinite(Config.NavigationConstant) || Config.NavigationConstant < 0.0f || Config.NavigationConstant > 10.0f
		|| !FMath::IsFinite(Config.MaximumTurnRateDegPerSec) || Config.MaximumTurnRateDegPerSec < 0.0f || Config.MaximumTurnRateDegPerSec > 720.0f
		|| !FMath::IsFinite(Config.MaximumLateralAccelerationCmPerSecSq) || Config.MaximumLateralAccelerationCmPerSecSq < 0.0f || Config.MaximumLateralAccelerationCmPerSecSq > 1000000.0f
		|| !FMath::IsFinite(Config.GuidanceResponseTimeSeconds) || Config.GuidanceResponseTimeSeconds < 0.001f || Config.GuidanceResponseTimeSeconds > 10.0f
		|| !FMath::IsFinite(Config.MinimumGuidanceSpeedCmPerSec) || Config.MinimumGuidanceSpeedCmPerSec < 0.0f || Config.MinimumGuidanceSpeedCmPerSec > 1000000.0f
		|| !FMath::IsFinite(Config.SeekerFieldOfViewDeg) || Config.SeekerFieldOfViewDeg < 0.0f || Config.SeekerFieldOfViewDeg > 360.0f
		|| !FMath::IsFinite(Config.LockBreakAngleDeg) || Config.LockBreakAngleDeg < 0.0f || Config.LockBreakAngleDeg > 180.0f
		|| !FMath::IsFinite(Config.TargetLostGraceTimeSeconds) || Config.TargetLostGraceTimeSeconds < 0.0f || Config.TargetLostGraceTimeSeconds > 30.0f
		|| !FMath::IsFinite(Config.GuidanceActivationDelaySeconds) || Config.GuidanceActivationDelaySeconds < 0.0f || Config.GuidanceActivationDelaySeconds > 30.0f
		|| !FMath::IsFinite(Config.GuidanceActivationDistanceCm) || Config.GuidanceActivationDistanceCm < 0.0f || Config.GuidanceActivationDistanceCm > 1000000.0f)
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Guidance,
			TEXT("유도 성능/Seeker/활성화 핵심값 중 하나 이상이 Runtime authored 범위를 벗어났습니다."));
	}
	return !HasBlockingIssue(OutIssues);
}

// Weapon heat conditional draft를 검증합니다.
bool FCFWeaponGuideVM::ValidateHeatDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	if (!Draft.Weapon.bUseHeat)
	{
		return true;
	}

	if (!FMath::IsFinite(Draft.Weapon.HeatPerShot) || Draft.Weapon.HeatPerShot <= 0.0f
		|| !FMath::IsFinite(Draft.Weapon.MaxHeat) || Draft.Weapon.MaxHeat <= 0.0f
		|| !FMath::IsFinite(Draft.Weapon.HeatDissipationPerSecond) || Draft.Weapon.HeatDissipationPerSecond <= 0.0f)
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Heat,
			TEXT("Heat Runtime을 사용할 때 HeatPerShot, MaxHeat, HeatDissipationPerSecond는 모두 0보다 큰 유한값이어야 합니다."));
	}
	return !HasBlockingIssue(OutIssues);
}

// Weapon charge conditional draft를 검증합니다.
bool FCFWeaponGuideVM::ValidateChargeDraft(TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	if (!Draft.Weapon.bUseCharge)
	{
		return true;
	}

	if (!FMath::IsFinite(Draft.Weapon.MaximumWeaponCharge) || Draft.Weapon.MaximumWeaponCharge <= 0.0f
		|| !FMath::IsFinite(Draft.Weapon.InitialWeaponCharge) || Draft.Weapon.InitialWeaponCharge < 0.0f || Draft.Weapon.InitialWeaponCharge > Draft.Weapon.MaximumWeaponCharge
		|| !FMath::IsFinite(Draft.Weapon.WeaponChargePerShot) || Draft.Weapon.WeaponChargePerShot <= 0.0f
		|| !FMath::IsFinite(Draft.Weapon.WeaponChargeRecoveryPerSecond) || Draft.Weapon.WeaponChargeRecoveryPerSecond <= 0.0f)
	{
		AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Charge,
			TEXT("Charge Runtime을 사용할 때 Maximum/PerShot/Recovery는 0보다 커야 하고 Initial은 0..Maximum 범위여야 합니다."));
	}
	return !HasBlockingIssue(OutIssues);
}

// existing Weapon/Projectile가 Guide에서 선택한 child reference와 exact 일치하는지 검증합니다.
bool FCFWeaponGuideVM::ValidateExistingReferenceConsistency(TArray<FCFWeaponGuideIssue>& OutIssues) const
{
	// 현재 Draft가 의미하는 최종 Projectile path입니다.
	const FSoftObjectPath SelectedProjectilePath = Draft.Projectile.Mode == ECFWeaponGuideChildMode::ReuseExisting
		? Draft.Projectile.ExistingAssetPath
		: (SessionCreatedProjectilePath.IsNull() ? FSoftObjectPath(GetDerivedProjectileObjectPath()) : SessionCreatedProjectilePath);

	// 현재 Draft가 의미하는 최종 Damage path입니다.
	const FSoftObjectPath SelectedDamagePath = Draft.Damage.Mode == ECFWeaponGuideChildMode::ReuseExisting
		? Draft.Damage.ExistingAssetPath
		: (SessionCreatedDamagePath.IsNull() ? FSoftObjectPath(GetDerivedDamageObjectPath()) : SessionCreatedDamagePath);

	// 현재 Draft가 의미하는 최종 finite Ammo path입니다.
	const FSoftObjectPath SelectedAmmoPath = Draft.Ammo.Mode == ECFWeaponGuideChildMode::ReuseExisting
		? Draft.Ammo.ExistingAssetPath
		: (SessionCreatedAmmoPath.IsNull() ? FSoftObjectPath(GetDerivedAmmoObjectPath()) : SessionCreatedAmmoPath);

	if (Draft.Weapon.Mode == ECFWeaponGuideChildMode::ReuseExisting)
	{
		// 기존 Weapon reference truth입니다.
		UCFWeaponData* WeaponData = Cast<UCFWeaponData>(Draft.Weapon.ExistingAssetPath.TryLoad());
		if (WeaponData != nullptr)
		{
			if (BuildObjectPath(WeaponData->DefaultProjectileData.Get()) != SelectedProjectilePath)
			{
				AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Projectile,
					TEXT("기존 WeaponData를 재사용할 때 Guide에서 다른 ProjectileData로 바꿀 수 없습니다. persisted DefaultProjectileData와 exact 일치해야 합니다."));
			}
			if (Draft.Weapon.bUseFiniteAmmo && BuildObjectPath(WeaponData->DefaultAmmoData.Get()) != SelectedAmmoPath)
			{
				AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Ammo,
					TEXT("기존 finite WeaponData를 재사용할 때 Guide에서 다른 AmmoData로 바꿀 수 없습니다. persisted DefaultAmmoData와 exact 일치해야 합니다."));
			}
		}
	}

	if (Draft.Projectile.Mode == ECFWeaponGuideChildMode::ReuseExisting)
	{
		// 기존 Projectile reference truth입니다.
		UCFProjectileData* ProjectileData = Cast<UCFProjectileData>(Draft.Projectile.ExistingAssetPath.TryLoad());
		if (ProjectileData != nullptr && BuildObjectPath(ProjectileData->DefaultDamageData.Get()) != SelectedDamagePath)
		{
			AddIssue(OutIssues, ECFWeaponGuideIssueSeverity::Error, ECFWeaponGuideStep::Damage,
				TEXT("기존 ProjectileData를 재사용할 때 Guide에서 다른 DamageData로 바꿀 수 없습니다. persisted DefaultDamageData와 exact 일치해야 합니다."));
		}
	}

	return !HasBlockingIssue(OutIssues);
}

// 현재 draft의 resolved final child path를 사용해 deterministic completion Bundle을 계산합니다.
bool FCFWeaponGuideVM::BuildResultBundle(FCFWeaponGuideResultBundle& OutBundle, FString& OutError) const
{
	OutBundle = FCFWeaponGuideResultBundle();
	OutBundle.SuggestedEquipmentDisplayName = Draft.SuggestedDisplayName;
	OutBundle.RequiredMountType = Draft.RequiredMountType;
	OutBundle.RequiredWeaponSize = Draft.RequiredWeaponSize;
	OutBundle.TurretMountDataPath = Draft.Turret.Mode == ECFWeaponGuideChildMode::ReuseExisting ? Draft.Turret.ExistingAssetPath : SessionCreatedTurretPath;
	OutBundle.ProjectileDataPath = Draft.Projectile.Mode == ECFWeaponGuideChildMode::ReuseExisting ? Draft.Projectile.ExistingAssetPath : SessionCreatedProjectilePath;
	OutBundle.WeaponDataPath = Draft.Weapon.Mode == ECFWeaponGuideChildMode::ReuseExisting ? Draft.Weapon.ExistingAssetPath : SessionCreatedWeaponPath;
	OutBundle.AmmoDataPath = Draft.Weapon.bUseFiniteAmmo
		? (Draft.Ammo.Mode == ECFWeaponGuideChildMode::ReuseExisting ? Draft.Ammo.ExistingAssetPath : SessionCreatedAmmoPath)
		: FSoftObjectPath();
	OutBundle.DamageDataPath = Draft.Damage.Mode == ECFWeaponGuideChildMode::ReuseExisting ? Draft.Damage.ExistingAssetPath : SessionCreatedDamagePath;
	OutBundle.AmmoTypeId = Draft.Weapon.AmmoTypeId;
	if (Draft.Weapon.bUseFiniteAmmo && !OutBundle.AmmoDataPath.IsNull())
	{
		// Final finite AmmoData exact persisted identity입니다.
		const UCFAmmoData* FinalAmmoData = Cast<UCFAmmoData>(OutBundle.AmmoDataPath.TryLoad());
		if (FinalAmmoData == nullptr || !FinalAmmoData->IsAmmoDataValid())
		{
			OutError = TEXT("완료 Bundle의 finite AmmoData exact persisted identity를 읽지 못했습니다.");
			return false;
		}
		OutBundle.AmmoTypeId = FinalAmmoData->AmmoId;
	}
	if (OutBundle.TurretMountDataPath.IsNull() || OutBundle.ProjectileDataPath.IsNull() || OutBundle.WeaponDataPath.IsNull() || OutBundle.DamageDataPath.IsNull())
	{
		OutError = TEXT("완료 Bundle에 필요한 Turret/Weapon/Projectile/Damage reference가 모두 확정되지 않았습니다.");
		return false;
	}
	if (Draft.Weapon.bUseFiniteAmmo && OutBundle.AmmoDataPath.IsNull())
	{
		OutError = TEXT("유한 탄약 Bundle에는 AmmoData reference가 필요합니다.");
		return false;
	}
	if (!BuildBundleFingerprint(OutBundle, OutBundle.DeterministicBundleFingerprint, OutError))
	{
		return false;
	}
	OutBundle.bComplete = true;
	OutError.Reset();
	return true;
}

// 현재 draft 의미와 final child references를 canonical SHA-256 Bundle fingerprint로 계산합니다.
bool FCFWeaponGuideVM::BuildBundleFingerprint(const FCFWeaponGuideResultBundle& Bundle, FString& OutFingerprint, FString& OutError) const
{
	// Bundle semantic byte stream입니다.
	TArray<uint8> Bytes;
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("DisplayName"), Bundle.SuggestedEquipmentDisplayName);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("MountType"), LexToString(static_cast<int32>(Bundle.RequiredMountType)));
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("WeaponSize"), LexToString(static_cast<int32>(Bundle.RequiredWeaponSize)));
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("Turret"), Bundle.TurretMountDataPath.ToString());
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("Weapon"), Bundle.WeaponDataPath.ToString());
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("Projectile"), Bundle.ProjectileDataPath.ToString());
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("Ammo"), Bundle.AmmoDataPath.ToString());
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("AmmoTypeId"), CFDACommonPrimitives::CanonicalNameText(Bundle.AmmoTypeId));
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("Damage"), Bundle.DamageDataPath.ToString());
	return CFDACommonPrimitives::HashCanonicalBytes(Bytes, OutFingerprint, OutError);
}


// P0-03 Bundle fingerprint를 변경하지 않고 persisted exact5 + 완료 USER 의미를 별도 semantic fingerprint로 계산합니다.
bool FCFWeaponGuideVM::BuildCompletionSemanticFingerprint(
	const FCFWeaponGuideResultBundle& Bundle,
	FString& OutFingerprint,
	FString& OutError) const
{
	if (!Bundle.bComplete || Bundle.DeterministicBundleFingerprint.IsEmpty())
	{
		OutError = TEXT("P0-04 completion semantic fingerprint는 complete Bundle에서만 계산할 수 있습니다.");
		return false;
	}

	// Final exact5 typed assets입니다.
	const UCFTurretMountData* TurretData = Cast<UCFTurretMountData>(Bundle.TurretMountDataPath.TryLoad());
	const UCFDamageData* DamageData = Cast<UCFDamageData>(Bundle.DamageDataPath.TryLoad());
	const UCFProjectileData* ProjectileData = Cast<UCFProjectileData>(Bundle.ProjectileDataPath.TryLoad());
	const UCFWeaponData* WeaponData = Cast<UCFWeaponData>(Bundle.WeaponDataPath.TryLoad());
	const UCFAmmoData* AmmoData = Bundle.AmmoDataPath.IsNull()
		? nullptr
		: Cast<UCFAmmoData>(Bundle.AmmoDataPath.TryLoad());
	if (TurretData == nullptr || DamageData == nullptr || ProjectileData == nullptr || WeaponData == nullptr
		|| (!Bundle.AmmoDataPath.IsNull() && AmmoData == nullptr))
	{
		OutError = TEXT("P0-04 completion semantic fingerprint용 persisted exact5 typed asset을 읽지 못했습니다.");
		return false;
	}

	// Persisted Turret semantic payload/fingerprint입니다.
	FCFWeaponGuideTurretPayload PersistedTurretPayload;
	TArray<FCFDAStagingIssue> ExtractIssues;
	FString TurretFingerprint;
	if (!ExtractTurretPayload(*TurretData, PersistedTurretPayload, ExtractIssues)
		|| !BuildTurretFingerprint(PersistedTurretPayload, TurretFingerprint, OutError))
	{
		OutError = TEXT("persisted TurretMountData semantic fingerprint를 계산하지 못했습니다. ") + OutError;
		return false;
	}

	// Persisted Damage provider payload/fingerprint입니다.
	FCFDADamagePayload PersistedDamagePayload;
	ExtractIssues.Reset();
	FString DamageFingerprint;
	if (!CFDADamageProviderImpl::ExtractPayload(*DamageData, PersistedDamagePayload, ExtractIssues)
		|| !CFDADamageProviderImpl::BuildSemanticFingerprint(PersistedDamagePayload, DamageFingerprint, OutError))
	{
		OutError = TEXT("persisted DamageData semantic fingerprint를 계산하지 못했습니다. ") + OutError;
		return false;
	}

	// Persisted Projectile semantic payload/fingerprint입니다.
	FCFWeaponGuideProjectilePayload PersistedProjectilePayload;
	ExtractIssues.Reset();
	FString ProjectileFingerprint;
	if (!ExtractProjectilePayload(*ProjectileData, PersistedProjectilePayload, ExtractIssues)
		|| !BuildProjectileFingerprint(PersistedProjectilePayload, ProjectileFingerprint, OutError))
	{
		OutError = TEXT("persisted ProjectileData semantic fingerprint를 계산하지 못했습니다. ") + OutError;
		return false;
	}

	// Persisted Weapon semantic payload/fingerprint입니다.
	FCFWeaponGuideWeaponPayload PersistedWeaponPayload;
	ExtractIssues.Reset();
	FString WeaponFingerprint;
	if (!ExtractWeaponPayload(*WeaponData, PersistedWeaponPayload, ExtractIssues)
		|| !BuildWeaponFingerprint(PersistedWeaponPayload, WeaponFingerprint, OutError))
	{
		OutError = TEXT("persisted WeaponData semantic fingerprint를 계산하지 못했습니다. ") + OutError;
		return false;
	}

	// finite ammo일 때 persisted Ammo provider payload/fingerprint입니다.
	FString AmmoFingerprint = TEXT("<none>");
	if (AmmoData != nullptr)
	{
		FCFDAAmmoPayload PersistedAmmoPayload;
		ExtractIssues.Reset();
		if (!CFDAAmmoProviderImpl::ExtractPayload(*AmmoData, PersistedAmmoPayload, ExtractIssues)
			|| !CFDAAmmoProviderImpl::BuildSemanticFingerprint(PersistedAmmoPayload, AmmoFingerprint, OutError))
		{
			OutError = TEXT("persisted AmmoData semantic fingerprint를 계산하지 못했습니다. ") + OutError;
			return false;
		}
	}

	// Guide가 직접 생성한 child는 current Draft desired semantic과 persisted semantic이 계속 exact 일치해야 합니다.
	if (Draft.Turret.Mode == ECFWeaponGuideChildMode::CreateNew)
	{
		const FCFWeaponGuideTurretPayload DesiredPayload = BuildGuideTurretPayload(Draft, TurretData->TurretMountId);
		FString DesiredFingerprint;
		if (!BuildTurretFingerprint(DesiredPayload, DesiredFingerprint, OutError)
			|| !DesiredFingerprint.Equals(TurretFingerprint, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("완료 후 TurretMountData Draft 또는 persisted semantic이 확정 상태와 달라졌습니다.");
			return false;
		}
	}
	if (Draft.Damage.Mode == ECFWeaponGuideChildMode::CreateNew)
	{
		const FCFDADamagePayload DesiredPayload = BuildDamageProviderPayload(Draft, DamageData->DamageId);
		FString DesiredFingerprint;
		if (!CFDADamageProviderImpl::BuildSemanticFingerprint(DesiredPayload, DesiredFingerprint, OutError)
			|| !DesiredFingerprint.Equals(DamageFingerprint, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("완료 후 DamageData Draft 또는 persisted semantic이 확정 상태와 달라졌습니다.");
			return false;
		}
	}
	if (Draft.Weapon.bUseFiniteAmmo && Draft.Ammo.Mode == ECFWeaponGuideChildMode::CreateNew)
	{
		if (AmmoData == nullptr)
		{
			OutError = TEXT("완료 후 finite AmmoData persisted semantic을 확인할 수 없습니다.");
			return false;
		}
		const FCFDAAmmoPayload DesiredPayload = BuildAmmoProviderPayload(Draft, AmmoData->AmmoId);
		FString DesiredFingerprint;
		if (!CFDAAmmoProviderImpl::BuildSemanticFingerprint(DesiredPayload, DesiredFingerprint, OutError)
			|| !DesiredFingerprint.Equals(AmmoFingerprint, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("완료 후 AmmoData Draft 또는 persisted semantic이 확정 상태와 달라졌습니다.");
			return false;
		}
	}
	if (Draft.Projectile.Mode == ECFWeaponGuideChildMode::CreateNew)
	{
		const FCFWeaponGuideProjectilePayload DesiredPayload = BuildGuideProjectilePayload(
			Draft,
			ProjectileData->ProjectileId,
			Bundle.DamageDataPath);
		FString DesiredFingerprint;
		if (!BuildProjectileFingerprint(DesiredPayload, DesiredFingerprint, OutError)
			|| !DesiredFingerprint.Equals(ProjectileFingerprint, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("완료 후 ProjectileData Draft 또는 persisted semantic이 확정 상태와 달라졌습니다.");
			return false;
		}
	}
	if (Draft.Weapon.Mode == ECFWeaponGuideChildMode::CreateNew)
	{
		const FCFWeaponGuideWeaponPayload DesiredPayload = BuildGuideWeaponPayload(
			Draft,
			WeaponData->WeaponId,
			Bundle.ProjectileDataPath,
			Bundle.AmmoDataPath,
			Bundle.AmmoTypeId);
		FString DesiredFingerprint;
		if (!BuildWeaponFingerprint(DesiredPayload, DesiredFingerprint, OutError)
			|| !DesiredFingerprint.Equals(WeaponFingerprint, ESearchCase::CaseSensitive))
		{
			OutError = TEXT("완료 후 WeaponData Draft 또는 persisted semantic이 확정 상태와 달라졌습니다.");
			return false;
		}
	}

	// P0-03 Bundle identity와 별개로 P0-04 USER summary/handoff 의미를 고정하는 canonical byte stream입니다.
	TArray<uint8> Bytes;
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("BundleFingerprint"), Bundle.DeterministicBundleFingerprint);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("TurretFingerprint"), TurretFingerprint);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("DamageFingerprint"), DamageFingerprint);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("AmmoFingerprint"), AmmoFingerprint);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("ProjectileFingerprint"), ProjectileFingerprint);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("WeaponFingerprint"), WeaponFingerprint);
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("TurretMode"), LexToString(static_cast<int32>(Draft.Turret.Mode)));
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("DamageMode"), LexToString(static_cast<int32>(Draft.Damage.Mode)));
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("AmmoMode"), LexToString(static_cast<int32>(Draft.Ammo.Mode)));
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("ProjectileMode"), LexToString(static_cast<int32>(Draft.Projectile.Mode)));
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("WeaponMode"), LexToString(static_cast<int32>(Draft.Weapon.Mode)));
	CFDACommonPrimitives::AppendStringToken(Bytes, TEXT("FireMode"), LexToString(static_cast<int32>(Draft.Weapon.FireMode)));
	CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("FiniteAmmo"), Draft.Weapon.bUseFiniteAmmo);
	CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("UseLauncher"), Draft.Weapon.bUseLauncher);
	CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("UsePropulsion"), Draft.Projectile.PropulsionConfig.bUsePropulsion);
	CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("UseMissileFlight"), Draft.Projectile.MissileFlightConfig.bUseMissileFlight);
	CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("UseGuidance"), Draft.Projectile.MissileGuideConfig.bUseGuidance);
	CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("UseHeat"), Draft.Weapon.bUseHeat);
	CFDACommonPrimitives::AppendBoolToken(Bytes, TEXT("UseCharge"), Draft.Weapon.bUseCharge);
	return CFDACommonPrimitives::HashCanonicalBytes(Bytes, OutFingerprint, OutError);
}

// completion summary/handoff 전에 Bundle identity, persisted graph와 P0-04 semantic fingerprint 불변성을 fail-closed 검증합니다.
bool FCFWeaponGuideVM::ValidateCompletionBundleIntegrity(FString& OutError) const
{
	if (!ResultBundle.bComplete || CompletionSemanticFingerprint.IsEmpty())
	{
		OutError = TEXT("완료 Bundle 또는 P0-04 completion semantic snapshot이 아직 확정되지 않았습니다.");
		return false;
	}

	// 현재 Draft/session truth로 다시 계산한 fresh completion Bundle입니다.
	FCFWeaponGuideResultBundle FreshBundle;
	if (!BuildResultBundle(FreshBundle, OutError))
	{
		return false;
	}
	if (!FreshBundle.DeterministicBundleFingerprint.Equals(
		ResultBundle.DeterministicBundleFingerprint,
		ESearchCase::CaseSensitive))
	{
		OutError = TEXT("완료 Bundle fingerprint가 현재 Draft/reference truth와 달라 handoff를 차단했습니다.");
		return false;
	}

	if (!ValidateDurableReferenceGraph(
		ResultBundle.TurretMountDataPath,
		ResultBundle.DamageDataPath,
		ResultBundle.AmmoDataPath,
		ResultBundle.ProjectileDataPath,
		ResultBundle.WeaponDataPath,
		OutError))
	{
		return false;
	}

	// P0-03 Bundle identity와 별개로 P0-04에서 고정한 persisted exact5 + USER summary 의미를 fresh 재계산합니다.
	FString FreshCompletionSemanticFingerprint;
	if (!BuildCompletionSemanticFingerprint(ResultBundle, FreshCompletionSemanticFingerprint, OutError))
	{
		return false;
	}
	if (!FreshCompletionSemanticFingerprint.Equals(CompletionSemanticFingerprint, ESearchCase::CaseSensitive))
	{
		OutError = TEXT("완료 후 FireMode/Capability/child semantic이 확정된 P0-04 completion 의미와 달라 summary/handoff를 차단했습니다.");
		return false;
	}

	OutError.Reset();
	return true;
}

// 확정된 completion Bundle을 다시 fingerprint 검증하고 사용자 중심 결과 요약을 구성합니다.
bool FCFWeaponGuideVM::BuildCompletionUserSummary(FString& OutSummary, FString& OutError) const
{
	if (!ValidateCompletionBundleIntegrity(OutError))
	{
		OutSummary.Reset();
		return false;
	}

	// 사용자용 장착 타입 label입니다.
	const FString MountLabel = ResultBundle.RequiredMountType == ECFVehicleMountType::Fixed
		? TEXT("고정형 (Fixed)")
		: ResultBundle.RequiredMountType == ECFVehicleMountType::Gimbal
			? TEXT("짐벌 (Gimbal)")
			: ResultBundle.RequiredMountType == ECFVehicleMountType::Turret
				? TEXT("포탑 (Turret)")
				: ResultBundle.RequiredMountType == ECFVehicleMountType::Launcher
					? TEXT("런처 (Launcher)")
					: TEXT("없음/지원하지 않음");

	// 사용자용 무기 크기 label입니다.
	const FString SizeLabel = ResultBundle.RequiredWeaponSize == ECFVehicleWeaponSize::Small
		? TEXT("소형 (Small)")
		: ResultBundle.RequiredWeaponSize == ECFVehicleWeaponSize::Medium
			? TEXT("중형 (Medium)")
			: ResultBundle.RequiredWeaponSize == ECFVehicleWeaponSize::Large
				? TEXT("대형 (Large)")
				: TEXT("없음");

	// 사용자용 발사 방식 label입니다.
	const FString FireModeLabel = Draft.Weapon.FireMode == ECFWeaponFireMode::Projectile
		? TEXT("발사체 (Projectile)")
		: TEXT("즉시 판정 (HitScan)");

	// 활성 Capability를 사용자에게 보여줄 목록입니다.
	TArray<FString> ActiveCapabilities;
	if (Draft.Weapon.bUseLauncher)
	{
		ActiveCapabilities.Add(TEXT("Launcher"));
	}
	if (Draft.Projectile.PropulsionConfig.bUsePropulsion)
	{
		ActiveCapabilities.Add(TEXT("Propulsion"));
	}
	if (Draft.Projectile.MissileFlightConfig.bUseMissileFlight)
	{
		ActiveCapabilities.Add(TEXT("Missile Flight"));
	}
	if (Draft.Projectile.MissileGuideConfig.bUseGuidance)
	{
		ActiveCapabilities.Add(TEXT("Guidance"));
	}
	if (Draft.Weapon.bUseHeat)
	{
		ActiveCapabilities.Add(TEXT("Heat"));
	}
	if (Draft.Weapon.bUseCharge)
	{
		ActiveCapabilities.Add(TEXT("Charge"));
	}

	// 활성 Capability 목록의 사용자 표시 문자열입니다.
	const FString CapabilityLabel = ActiveCapabilities.IsEmpty()
		? TEXT("추가 Capability 없음")
		: FString::Join(ActiveCapabilities, TEXT(", "));

	// child 생성/재사용 상태를 사용자에게 설명하는 helper입니다.
	const auto ChildModeText = [](const ECFWeaponGuideChildMode Mode) -> const TCHAR*
	{
		return Mode == ECFWeaponGuideChildMode::CreateNew ? TEXT("새로 생성") : TEXT("기존 데이터 재사용");
	};

	// finite/infinite 탄약 결과를 사용자에게 설명하는 문자열입니다.
	const FString AmmoSummary = Draft.Weapon.bUseFiniteAmmo
		? FString::Printf(TEXT("%s / AmmoTypeId=%s"), ChildModeText(Draft.Ammo.Mode), *ResultBundle.AmmoTypeId.ToString())
		: FString::Printf(TEXT("무한 탄약 / fallback AmmoTypeId=%s"), *ResultBundle.AmmoTypeId.ToString());

	OutSummary = FString::Printf(
		TEXT("무장 제작 완료\n")
		TEXT("표시 이름 제안: %s\n")
		TEXT("장착 요구: %s / %s\n")
		TEXT("발사 방식: %s\n")
		TEXT("활성 기능: %s\n")
		TEXT("TurretMountData: %s\n")
		TEXT("DamageData: %s\n")
		TEXT("AmmoData: %s\n")
		TEXT("ProjectileData: %s\n")
		TEXT("WeaponData: %s\n")
		TEXT("다음 작업: 필요하면 장비 제작 가이드를 열어 EquipmentPreset 조립을 시작하세요. 이 이동은 값을 자동 전달하거나 CF-FQ-054를 재개하지 않습니다."),
		*ResultBundle.SuggestedEquipmentDisplayName,
		*MountLabel,
		*SizeLabel,
		*FireModeLabel,
		*CapabilityLabel,
		ChildModeText(Draft.Turret.Mode),
		ChildModeText(Draft.Damage.Mode),
		*AmmoSummary,
		ChildModeText(Draft.Projectile.Mode),
		ChildModeText(Draft.Weapon.Mode));

	OutError.Reset();
	return true;
}

// 확정된 completion Bundle에서 Equipment Builder navigation-only 인계 계약을 구성합니다.
bool FCFWeaponGuideVM::BuildEquipmentBuilderHandoff(
	FCFWeaponGuideEquipmentHandoff& OutHandoff,
	FString& OutError) const
{
	if (!ValidateCompletionBundleIntegrity(OutError))
	{
		OutHandoff = FCFWeaponGuideEquipmentHandoff();
		return false;
	}

	OutHandoff = FCFWeaponGuideEquipmentHandoff();
	OutHandoff.TargetTabId = TEXT("CarFight.EquipmentBuilder");
	OutHandoff.SourceBundleFingerprint = ResultBundle.DeterministicBundleFingerprint;
	OutHandoff.bNavigationOnly = true;
	OutHandoff.bInjectContext = false;
	OutError.Reset();
	return true;
}

// Multi-asset mutation 전에 모든 새 target과 이미 confirmed된 session child를 fresh fail-closed 검증합니다.
bool FCFWeaponGuideVM::ValidateFreshDurablePreflight(FString& OutError) const
{
	// Same-session confirmed child의 exact path/type/clean state를 공통 검사합니다.
	const auto ValidateSessionChild = [this, &OutError](
		const FSoftObjectPath& SessionPath,
		const FString& DerivedPath,
		UClass* ExpectedClass,
		const FString& Label) -> bool
	{
		if (SessionPath.IsNull())
		{
			return true;
		}
		if (!SessionPath.ToString().Equals(DerivedPath, ESearchCase::CaseSensitive))
		{
			OutError = FString::Printf(TEXT("%s same-session durable path가 현재 derived target과 다릅니다."), *Label);
			return false;
		}
		return ValidateExistingChild(SessionPath, ExpectedClass, Label, OutError);
	};

	if (!ValidateSessionChild(SessionCreatedDamagePath, GetDerivedDamageObjectPath(), UCFDamageData::StaticClass(), TEXT("DamageData"))
		|| !ValidateSessionChild(SessionCreatedAmmoPath, GetDerivedAmmoObjectPath(), UCFAmmoData::StaticClass(), TEXT("AmmoData"))
		|| !ValidateSessionChild(SessionCreatedTurretPath, GetDerivedTurretObjectPath(), UCFTurretMountData::StaticClass(), TEXT("TurretMountData"))
		|| !ValidateSessionChild(SessionCreatedProjectilePath, GetDerivedProjectileObjectPath(), UCFProjectileData::StaticClass(), TEXT("ProjectileData"))
		|| !ValidateSessionChild(SessionCreatedWeaponPath, GetDerivedWeaponObjectPath(), UCFWeaponData::StaticClass(), TEXT("WeaponData")))
	{
		return false;
	}

	// Session-created exact3은 current Draft 의미와 persisted semantic fingerprint까지 같아야 재사용합니다.
	if (!SessionCreatedTurretPath.IsNull())
	{
		// Current Draft 기준 expected Turret payload입니다.
		const FCFWeaponGuideTurretPayload DesiredPayload = BuildGuideTurretPayload(Draft, GetDerivedTurretId());
		// Current Draft 기준 expected Turret fingerprint입니다.
		FString DesiredFingerprint;
		if (!BuildTurretFingerprint(DesiredPayload, DesiredFingerprint, OutError))
		{
			return false;
		}
		// Persisted same-session Turret asset입니다.
		UCFTurretMountData* SessionTurret = Cast<UCFTurretMountData>(SessionCreatedTurretPath.TryLoad());
		if (SessionTurret == nullptr
			|| !CFDADurableCore::ValidateTypedAssetFingerprint<UCFTurretMountData, FCFWeaponGuideTurretPayload>(
				*SessionTurret,
				DesiredFingerprint,
				&ExtractTurretPayload,
				&BuildTurretFingerprint,
				OutError))
		{
			OutError = TEXT("이미 durable 저장한 TurretMountData가 현재 Draft와 달라 안전하게 재사용할 수 없습니다. ") + OutError;
			return false;
		}
	}

	// Current Draft가 의미하는 final Damage reference입니다.
	const FSoftObjectPath ResolvedDamagePath = Draft.Damage.Mode == ECFWeaponGuideChildMode::ReuseExisting
		? Draft.Damage.ExistingAssetPath
		: (SessionCreatedDamagePath.IsNull() ? FSoftObjectPath(GetDerivedDamageObjectPath()) : SessionCreatedDamagePath);
	if (!SessionCreatedProjectilePath.IsNull())
	{
		// Current Draft + resolved Damage 기준 expected Projectile payload입니다.
		const FCFWeaponGuideProjectilePayload DesiredPayload = BuildGuideProjectilePayload(Draft, GetDerivedProjectileId(), ResolvedDamagePath);
		// Current Draft 기준 expected Projectile fingerprint입니다.
		FString DesiredFingerprint;
		if (!BuildProjectileFingerprint(DesiredPayload, DesiredFingerprint, OutError))
		{
			return false;
		}
		// Persisted same-session Projectile asset입니다.
		UCFProjectileData* SessionProjectile = Cast<UCFProjectileData>(SessionCreatedProjectilePath.TryLoad());
		if (SessionProjectile == nullptr
			|| !CFDADurableCore::ValidateTypedAssetFingerprint<UCFProjectileData, FCFWeaponGuideProjectilePayload>(
				*SessionProjectile,
				DesiredFingerprint,
				&ExtractProjectilePayload,
				&BuildProjectileFingerprint,
				OutError))
		{
			OutError = TEXT("이미 durable 저장한 ProjectileData가 현재 Draft/reference와 달라 안전하게 재사용할 수 없습니다. ") + OutError;
			return false;
		}
	}

	// Current Draft가 의미하는 final Projectile reference입니다.
	const FSoftObjectPath ResolvedProjectilePath = Draft.Projectile.Mode == ECFWeaponGuideChildMode::ReuseExisting
		? Draft.Projectile.ExistingAssetPath
		: (SessionCreatedProjectilePath.IsNull() ? FSoftObjectPath(GetDerivedProjectileObjectPath()) : SessionCreatedProjectilePath);
	// Current Draft가 의미하는 final finite Ammo reference입니다.
	const FSoftObjectPath ResolvedAmmoPath = Draft.Weapon.bUseFiniteAmmo
		? (Draft.Ammo.Mode == ECFWeaponGuideChildMode::ReuseExisting
			? Draft.Ammo.ExistingAssetPath
			: (SessionCreatedAmmoPath.IsNull() ? FSoftObjectPath(GetDerivedAmmoObjectPath()) : SessionCreatedAmmoPath))
		: FSoftObjectPath();
	if (!SessionCreatedWeaponPath.IsNull())
	{
		// Current finite Ammo identity 또는 infinite fallback ID입니다.
		FName ResolvedAmmoTypeId = Draft.Weapon.AmmoTypeId;
		if (Draft.Weapon.bUseFiniteAmmo)
		{
			// Resolved finite Ammo exact persisted asset입니다.
			const UCFAmmoData* ResolvedAmmoData = Cast<UCFAmmoData>(ResolvedAmmoPath.TryLoad());
			if (ResolvedAmmoData == nullptr || !ResolvedAmmoData->IsAmmoDataValid())
			{
				OutError = TEXT("same-session WeaponData 재검증 전에 finite AmmoData identity를 확정하지 못했습니다.");
				return false;
			}
			ResolvedAmmoTypeId = ResolvedAmmoData->AmmoId;
		}

		// Current Draft + resolved child refs 기준 expected Weapon payload입니다.
		const FCFWeaponGuideWeaponPayload DesiredPayload = BuildGuideWeaponPayload(
			Draft,
			GetDerivedWeaponId(),
			ResolvedProjectilePath,
			ResolvedAmmoPath,
			ResolvedAmmoTypeId);
		// Current Draft 기준 expected Weapon fingerprint입니다.
		FString DesiredFingerprint;
		if (!BuildWeaponFingerprint(DesiredPayload, DesiredFingerprint, OutError))
		{
			return false;
		}
		// Persisted same-session Weapon asset입니다.
		UCFWeaponData* SessionWeapon = Cast<UCFWeaponData>(SessionCreatedWeaponPath.TryLoad());
		if (SessionWeapon == nullptr
			|| !CFDADurableCore::ValidateTypedAssetFingerprint<UCFWeaponData, FCFWeaponGuideWeaponPayload>(
				*SessionWeapon,
				DesiredFingerprint,
				&ExtractWeaponPayload,
				&BuildWeaponFingerprint,
				OutError))
		{
			OutError = TEXT("이미 durable 저장한 WeaponData가 현재 Draft/reference와 달라 안전하게 재사용할 수 없습니다. ") + OutError;
			return false;
		}
	}

	if (Draft.Damage.Mode == ECFWeaponGuideChildMode::CreateNew && SessionCreatedDamagePath.IsNull())
	{
		// Damage provider memory-only candidate source입니다.
		FString ProviderJson;
		// Damage exact Create Preview입니다.
		FCFDACommonPreviewRow PreviewRow;
		if (!BuildDamageProviderCreateCandidate(Draft, GetDerivedDamageId(), GetDerivedDamageObjectPath(), ProviderJson, PreviewRow, OutError))
		{
			return false;
		}
	}
	if (Draft.Weapon.bUseFiniteAmmo
		&& Draft.Ammo.Mode == ECFWeaponGuideChildMode::CreateNew
		&& SessionCreatedAmmoPath.IsNull())
	{
		// Ammo provider memory-only candidate source입니다.
		FString ProviderJson;
		// Ammo exact Create Preview입니다.
		FCFDACommonPreviewRow PreviewRow;
		if (!BuildAmmoProviderCreateCandidate(Draft, GetDerivedAmmoId(), GetDerivedAmmoObjectPath(), ProviderJson, PreviewRow, OutError))
		{
			return false;
		}
	}

	// Provider가 없는 Guide-local exact3 중 아직 생성되지 않은 target 목록입니다.
	TArray<FString> LocalCreateTargets;
	if (Draft.Turret.Mode == ECFWeaponGuideChildMode::CreateNew && SessionCreatedTurretPath.IsNull())
	{
		LocalCreateTargets.Add(GetDerivedTurretObjectPath());
	}
	if (Draft.Projectile.Mode == ECFWeaponGuideChildMode::CreateNew && SessionCreatedProjectilePath.IsNull())
	{
		LocalCreateTargets.Add(GetDerivedProjectileObjectPath());
	}
	if (Draft.Weapon.Mode == ECFWeaponGuideChildMode::CreateNew && SessionCreatedWeaponPath.IsNull())
	{
		LocalCreateTargets.Add(GetDerivedWeaponObjectPath());
	}
	for (const FString& CreateTarget : LocalCreateTargets)
	{
		if (!ValidateCreateTargetAbsent(CreateTarget, OutError))
		{
			return false;
		}
	}

	OutError.Reset();
	return true;
}

// 모든 durable child의 persisted clean state와 Projectile→Damage / Weapon→Projectile/Ammo exact graph를 readback합니다.
bool FCFWeaponGuideVM::ValidateDurableReferenceGraph(
	const FSoftObjectPath& TurretPath,
	const FSoftObjectPath& DamagePath,
	const FSoftObjectPath& AmmoPath,
	const FSoftObjectPath& ProjectilePath,
	const FSoftObjectPath& WeaponPath,
	FString& OutError) const
{
	if (!ValidateExistingChild(TurretPath, UCFTurretMountData::StaticClass(), TEXT("최종 TurretMountData"), OutError)
		|| !ValidateExistingChild(DamagePath, UCFDamageData::StaticClass(), TEXT("최종 DamageData"), OutError)
		|| !ValidateExistingChild(ProjectilePath, UCFProjectileData::StaticClass(), TEXT("최종 ProjectileData"), OutError)
		|| !ValidateExistingChild(WeaponPath, UCFWeaponData::StaticClass(), TEXT("최종 WeaponData"), OutError))
	{
		return false;
	}
	if (Draft.Weapon.bUseFiniteAmmo
		&& !ValidateExistingChild(AmmoPath, UCFAmmoData::StaticClass(), TEXT("최종 AmmoData"), OutError))
	{
		return false;
	}

	// Persisted exact TurretMountData입니다.
	const UCFTurretMountData* TurretData = Cast<UCFTurretMountData>(TurretPath.TryLoad());
	// Persisted exact DamageData입니다.
	const UCFDamageData* DamageData = Cast<UCFDamageData>(DamagePath.TryLoad());
	// Persisted exact ProjectileData입니다.
	const UCFProjectileData* ProjectileData = Cast<UCFProjectileData>(ProjectilePath.TryLoad());
	// Persisted exact WeaponData입니다.
	const UCFWeaponData* WeaponData = Cast<UCFWeaponData>(WeaponPath.TryLoad());
	// Persisted exact finite AmmoData입니다.
	const UCFAmmoData* AmmoData = Draft.Weapon.bUseFiniteAmmo ? Cast<UCFAmmoData>(AmmoPath.TryLoad()) : nullptr;
	if (TurretData == nullptr || DamageData == nullptr || ProjectileData == nullptr || WeaponData == nullptr
		|| (Draft.Weapon.bUseFiniteAmmo && AmmoData == nullptr))
	{
		OutError = TEXT("최종 durable graph의 typed DataAsset readback에 실패했습니다.");
		return false;
	}

	if (!BuildObjectPath(ProjectileData->DefaultDamageData.Get()).ToString().Equals(DamagePath.ToString(), ESearchCase::CaseSensitive)
		|| ProjectileData->DamageProfileId != DamageData->DamageId)
	{
		OutError = TEXT("ProjectileData.DefaultDamageData/DamageProfileId가 최종 DamageData와 exact 일치하지 않습니다.");
		return false;
	}
	if (!BuildObjectPath(WeaponData->DefaultProjectileData.Get()).ToString().Equals(ProjectilePath.ToString(), ESearchCase::CaseSensitive)
		|| WeaponData->ProjectileDataId != ProjectileData->ProjectileId)
	{
		OutError = TEXT("WeaponData.DefaultProjectileData/ProjectileDataId가 최종 ProjectileData와 exact 일치하지 않습니다.");
		return false;
	}
	if (Draft.Weapon.bUseFiniteAmmo
		&& (!BuildObjectPath(WeaponData->DefaultAmmoData.Get()).ToString().Equals(AmmoPath.ToString(), ESearchCase::CaseSensitive)
			|| WeaponData->AmmoTypeId != AmmoData->AmmoId))
	{
		OutError = TEXT("WeaponData.DefaultAmmoData/AmmoTypeId가 최종 AmmoData와 exact 일치하지 않습니다.");
		return false;
	}

	if (Draft.Turret.Mode == ECFWeaponGuideChildMode::CreateNew && TurretData->TurretMountId != GetDerivedTurretId())
	{
		OutError = TEXT("신규 TurretMountData logical ID readback이 derived ID와 다릅니다.");
		return false;
	}
	if (Draft.Damage.Mode == ECFWeaponGuideChildMode::CreateNew && DamageData->DamageId != GetDerivedDamageId())
	{
		OutError = TEXT("신규 DamageData logical ID readback이 derived ID와 다릅니다.");
		return false;
	}
	if (Draft.Weapon.bUseFiniteAmmo && Draft.Ammo.Mode == ECFWeaponGuideChildMode::CreateNew && AmmoData->AmmoId != GetDerivedAmmoId())
	{
		OutError = TEXT("신규 AmmoData logical ID readback이 derived ID와 다릅니다.");
		return false;
	}
	if (Draft.Projectile.Mode == ECFWeaponGuideChildMode::CreateNew && ProjectileData->ProjectileId != GetDerivedProjectileId())
	{
		OutError = TEXT("신규 ProjectileData logical ID readback이 derived ID와 다릅니다.");
		return false;
	}
	if (Draft.Weapon.Mode == ECFWeaponGuideChildMode::CreateNew && WeaponData->WeaponId != GetDerivedWeaponId())
	{
		OutError = TEXT("신규 WeaponData logical ID readback이 derived ID와 다릅니다.");
		return false;
	}

	OutError.Reset();
	return true;
}

// 신규 DamageData를 existing reviewed-mutation provider authority로 exact Create합니다.
bool FCFWeaponGuideVM::CreateDamageAsset(FSoftObjectPath& OutObjectPath, FString& OutError) const
{
	// Mutation 직전 fresh provider memory source입니다.
	FString ProviderJson;
	// Mutation 직전 exact current truth를 반영한 Create row입니다.
	FCFDACommonPreviewRow FreshRow;
	if (!BuildDamageProviderCreateCandidate(Draft, GetDerivedDamageId(), GetDerivedDamageObjectPath(), ProviderJson, FreshRow, OutError))
	{
		return false;
	}

	// Existing Damage provider operation table authority입니다.
	const FCFDADamageTypeProvider& DamageProvider = CFDADamageProvider::GetProvider();
	// Existing Damage provider durable terminal report입니다.
	FCFDAStagingTargetApplyReport Report;
	DamageProvider.Operations.ApplyReviewedMutation(ProviderJson, FreshRow, Report);
	if (Report.Result != ECFDAStagingTargetApplyResult::DurableApplied)
	{
		OutError = FString::Printf(TEXT("DamageData provider durable Create에 실패했습니다: %s"), *Report.Diagnostic);
		return false;
	}
	OutObjectPath = FSoftObjectPath(GetDerivedDamageObjectPath());
	OutError.Reset();
	return true;
}

// 신규 finite AmmoData를 existing reviewed-mutation provider authority로 exact Create합니다.
bool FCFWeaponGuideVM::CreateAmmoAsset(FSoftObjectPath& OutObjectPath, FString& OutError) const
{
	// Mutation 직전 fresh provider memory source입니다.
	FString ProviderJson;
	// Mutation 직전 exact current truth를 반영한 Create row입니다.
	FCFDACommonPreviewRow FreshRow;
	if (!BuildAmmoProviderCreateCandidate(Draft, GetDerivedAmmoId(), GetDerivedAmmoObjectPath(), ProviderJson, FreshRow, OutError))
	{
		return false;
	}

	// Existing Ammo provider operation table authority입니다.
	const FCFDAAmmoTypeProvider& AmmoProvider = CFDAAmmoProvider::GetProvider();
	// Existing Ammo provider durable terminal report입니다.
	FCFDAStagingTargetApplyReport Report;
	AmmoProvider.Operations.ApplyReviewedMutation(ProviderJson, FreshRow, Report);
	if (Report.Result != ECFDAStagingTargetApplyResult::DurableApplied)
	{
		OutError = FString::Printf(TEXT("AmmoData provider durable Create에 실패했습니다: %s"), *Report.Diagnostic);
		return false;
	}
	OutObjectPath = FSoftObjectPath(GetDerivedAmmoObjectPath());
	OutError.Reset();
	return true;
}

// 신규 TurretMountData를 existing shared durable core로 exact Create합니다.
bool FCFWeaponGuideVM::CreateTurretAsset(FSoftObjectPath& OutObjectPath, FString& OutError) const
{
	// Guide Turret local payload입니다.
	FCFWeaponGuideTurretPayload Payload;
	Payload.TurretMountId = GetDerivedTurretId();
	Payload.TurretBaseMeshPath = Draft.Turret.TurretBaseMeshPath;
	Payload.TurretYawMeshPath = Draft.Turret.TurretYawMeshPath;
	Payload.TurretPitchMeshPath = Draft.Turret.TurretPitchMeshPath;
	Payload.YawPivotSocketName = Draft.Turret.YawPivotSocketName;
	Payload.PitchPivotSocketName = Draft.Turret.PitchPivotSocketName;
	Payload.PrimaryMuzzleSocketName = Draft.Turret.PrimaryMuzzleSocketName;
	Payload.MuzzleSocketNames = Draft.Turret.AdditionalMuzzleSocketNames;
	if (!Payload.PrimaryMuzzleSocketName.IsNone() && !Payload.MuzzleSocketNames.Contains(Payload.PrimaryMuzzleSocketName))
	{
		Payload.MuzzleSocketNames.Insert(Payload.PrimaryMuzzleSocketName, 0);
	}
	Payload.bRequireAllMuzzles = Draft.Turret.bRequireAllMuzzles;
	Payload.MinYawDeg = Draft.Turret.MinYawDeg;
	Payload.MaxYawDeg = Draft.Turret.MaxYawDeg;
	Payload.MinPitchDeg = Draft.Turret.MinPitchDeg;
	Payload.MaxPitchDeg = Draft.Turret.MaxPitchDeg;
	Payload.YawTurnRateDegPerSec = Draft.Turret.YawTurnRateDegPerSec;
	Payload.PitchTurnRateDegPerSec = Draft.Turret.PitchTurnRateDegPerSec;
	Payload.TurretMountWeightKg = Draft.Turret.TurretMountWeightKg;
	// Desired payload fingerprint입니다.
	FString Fingerprint;
	if (!BuildTurretFingerprint(Payload, Fingerprint, OutError))
	{
		return false;
	}
	// exact Create row입니다.
	const FCFDACommonPreviewRow Row = BuildCreateRow(TEXT("CarFight.WeaponGuide.TurretMount"), UCFTurretMountData::StaticClass(), Payload.TurretMountId, GetDerivedTurretObjectPath(), Fingerprint);
	// durable terminal report입니다.
	FCFDAStagingTargetApplyReport Report;
	CFDADurableCore::ApplyTypedTarget<UCFTurretMountData, FCFWeaponGuideTurretPayload>(Row, Payload, &MaterializeTurretPayload, &ExtractTurretPayload, &BuildTurretFingerprint, Report);
	if (Report.Result != ECFDAStagingTargetApplyResult::DurableApplied)
	{
		OutError = FString::Printf(TEXT("TurretMountData 저장에 실패했습니다: %s"), *Report.Diagnostic);
		return false;
	}
	OutObjectPath = FSoftObjectPath(GetDerivedTurretObjectPath());
	OutError.Reset();
	return true;
}

// 신규 ProjectileData를 existing shared durable core로 exact Create합니다.
bool FCFWeaponGuideVM::CreateProjectileAsset(const FSoftObjectPath& DamagePath, FSoftObjectPath& OutObjectPath, FString& OutError) const
{
	// Guide Projectile local payload입니다.
	FCFWeaponGuideProjectilePayload Payload;
	Payload.ProjectileId = GetDerivedProjectileId();
	Payload.ProjectileActorClassPath = Draft.Projectile.ProjectileActorClassPath;
	Payload.ProjectileStaticMeshPath = Draft.Projectile.ProjectileStaticMeshPath;
	Payload.InitialSpeed = Draft.Projectile.InitialSpeed;
	Payload.LifeTimeSeconds = Draft.Projectile.LifeTimeSeconds;
	Payload.bAffectedByGravity = Draft.Projectile.bAffectedByGravity;
	Payload.GravityScale = Draft.Projectile.GravityScale;
	Payload.CollisionRadius = Draft.Projectile.CollisionRadius;
	Payload.bUseSweepCollision = Draft.Projectile.bUseSweepCollision;
	Payload.PropulsionConfig = Draft.Projectile.PropulsionConfig;
	Payload.MissileFlightConfig = Draft.Projectile.MissileFlightConfig;
	Payload.MissileGuideConfig = Draft.Projectile.MissileGuideConfig;
	Payload.DamageDataPath = DamagePath;
	// Desired payload fingerprint입니다.
	FString Fingerprint;
	if (!BuildProjectileFingerprint(Payload, Fingerprint, OutError))
	{
		return false;
	}
	// exact Create row입니다.
	const FCFDACommonPreviewRow Row = BuildCreateRow(TEXT("CarFight.WeaponGuide.Projectile"), UCFProjectileData::StaticClass(), Payload.ProjectileId, GetDerivedProjectileObjectPath(), Fingerprint);
	// durable terminal report입니다.
	FCFDAStagingTargetApplyReport Report;
	CFDADurableCore::ApplyTypedTarget<UCFProjectileData, FCFWeaponGuideProjectilePayload>(Row, Payload, &MaterializeProjectilePayload, &ExtractProjectilePayload, &BuildProjectileFingerprint, Report);
	if (Report.Result != ECFDAStagingTargetApplyResult::DurableApplied)
	{
		OutError = FString::Printf(TEXT("ProjectileData 저장에 실패했습니다: %s"), *Report.Diagnostic);
		return false;
	}
	OutObjectPath = FSoftObjectPath(GetDerivedProjectileObjectPath());
	OutError.Reset();
	return true;
}

// 신규 WeaponData를 existing shared durable core로 exact Create합니다.
bool FCFWeaponGuideVM::CreateWeaponAsset(const FSoftObjectPath& ProjectilePath, const FSoftObjectPath& AmmoPath, FSoftObjectPath& OutObjectPath, FString& OutError) const
{
	// Guide Weapon local payload입니다.
	FCFWeaponGuideWeaponPayload Payload;
	Payload.WeaponId = GetDerivedWeaponId();
	Payload.WeaponSize = Draft.RequiredWeaponSize;
	Payload.CompatibleMountType = Draft.RequiredMountType;
	Payload.WeaponMassKg = Draft.Weapon.WeaponMassKg;
	Payload.FireMode = Draft.Weapon.FireMode;
	Payload.FireRatePerMinute = Draft.Weapon.FireRatePerMinute;
	Payload.MaxRange = Draft.Weapon.MaxRange;
	Payload.SpreadDeg = Draft.Weapon.SpreadDeg;
	Payload.MagazineSize = Draft.Weapon.MagazineSize;
	Payload.InitialLoadedAmmoCount = Draft.Weapon.InitialLoadedAmmoCount;
	Payload.AmmoUnitsPerShot = Draft.Weapon.AmmoUnitsPerShot;
	Payload.ReloadTimeSeconds = Draft.Weapon.ReloadTimeSeconds;
	Payload.bUseFiniteAmmo = Draft.Weapon.bUseFiniteAmmo;
	Payload.AmmoDataPath = Draft.Weapon.bUseFiniteAmmo ? AmmoPath : FSoftObjectPath();
	Payload.AmmoTypeId = Draft.Weapon.AmmoTypeId;
	if (Payload.bUseFiniteAmmo)
	{
		// Final finite AmmoData exact persisted identity입니다.
		const UCFAmmoData* FinalAmmoData = Cast<UCFAmmoData>(AmmoPath.TryLoad());
		if (FinalAmmoData == nullptr || !FinalAmmoData->IsAmmoDataValid())
		{
			OutError = TEXT("WeaponData 생성 전에 finite AmmoData persisted identity를 확인하지 못했습니다.");
			return false;
		}
		Payload.AmmoTypeId = FinalAmmoData->AmmoId;
	}
	Payload.ProjectileDataPath = ProjectilePath;
	Payload.LauncherFirePatternConfig = Draft.Weapon.LauncherFirePatternConfig;
	Payload.LauncherReleaseConfig = Draft.Weapon.LauncherReleaseConfig;
	Payload.bUseHeat = Draft.Weapon.bUseHeat;
	Payload.HeatPerShot = Draft.Weapon.HeatPerShot;
	Payload.MaxHeat = Draft.Weapon.MaxHeat;
	Payload.HeatDissipationPerSecond = Draft.Weapon.HeatDissipationPerSecond;
	Payload.bUseCharge = Draft.Weapon.bUseCharge;
	Payload.MaximumWeaponCharge = Draft.Weapon.MaximumWeaponCharge;
	Payload.InitialWeaponCharge = Draft.Weapon.InitialWeaponCharge;
	Payload.WeaponChargePerShot = Draft.Weapon.WeaponChargePerShot;
	Payload.WeaponChargeRecoveryPerSecond = Draft.Weapon.WeaponChargeRecoveryPerSecond;
	// Desired payload fingerprint입니다.
	FString Fingerprint;
	if (!BuildWeaponFingerprint(Payload, Fingerprint, OutError))
	{
		return false;
	}
	// exact Create row입니다.
	const FCFDACommonPreviewRow Row = BuildCreateRow(TEXT("CarFight.WeaponGuide.Weapon"), UCFWeaponData::StaticClass(), Payload.WeaponId, GetDerivedWeaponObjectPath(), Fingerprint);
	// durable terminal report입니다.
	FCFDAStagingTargetApplyReport Report;
	CFDADurableCore::ApplyTypedTarget<UCFWeaponData, FCFWeaponGuideWeaponPayload>(Row, Payload, &MaterializeWeaponPayload, &ExtractWeaponPayload, &BuildWeaponFingerprint, Report);
	if (Report.Result != ECFDAStagingTargetApplyResult::DurableApplied)
	{
		OutError = FString::Printf(TEXT("WeaponData 저장에 실패했습니다: %s"), *Report.Diagnostic);
		return false;
	}
	OutObjectPath = FSoftObjectPath(GetDerivedWeaponObjectPath());
	OutError.Reset();
	return true;
}

// current naming root가 Product 또는 approved disposable test namespace인지 반환합니다.
FString FCFWeaponGuideVM::GetAssetRoot() const
{
#if WITH_DEV_AUTOMATION_TESTS
	if (!DisposableTestRoot.IsEmpty())
	{
		return DisposableTestRoot;
	}
#endif
	return WeaponGuideProductRoot;
}

// validation issue를 사용자용 메시지 배열에 추가합니다.
void FCFWeaponGuideVM::AddIssue(
	TArray<FCFWeaponGuideIssue>& OutIssues,
	const ECFWeaponGuideIssueSeverity Severity,
	const ECFWeaponGuideStep Step,
	const FString& Message)
{
	// 새 validation issue입니다.
	FCFWeaponGuideIssue Issue;
	Issue.Severity = Severity;
	Issue.Step = Step;
	Issue.Message = Message;
	OutIssues.Add(MoveTemp(Issue));
}

// blocking Error issue가 하나라도 있는지 반환합니다.
bool FCFWeaponGuideVM::HasBlockingIssue(const TArray<FCFWeaponGuideIssue>& Issues)
{
	for (const FCFWeaponGuideIssue& Issue : Issues)
	{
		if (Issue.Severity == ECFWeaponGuideIssueSeverity::Error)
		{
			return true;
		}
	}
	return false;
}
