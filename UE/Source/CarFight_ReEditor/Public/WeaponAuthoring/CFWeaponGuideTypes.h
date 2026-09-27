// Copyright (c) CarFight. All Rights Reserved.
// File: CFWeaponGuideTypes.h
// Version: v1.2.0
// Date: 2026-09-18
// Description: CF-FQ-055 Weapon Equipment Authoring Guide의 Stable Step, Capability Draft, 완료 Bundle과 navigation-only handoff 계약입니다.
// Changelog:
// - v1.2.0: WEA-P0-04 Equipment Builder navigation-only handoff struct를 추가하고 context injection을 명시적으로 false로 고정했습니다.
// - v1.1.0: BuilderAuthoringStandard v1.0.0에 맞춰 fixed exact8 Wizard를 Stable StepId exact15 + Conditional Step 모델로 교정하고 Template, Damage/Ammo create-reuse draft, Launcher intent, Propulsion/MissileFlight/MissileGuide config를 추가.
// - v1.0.0: exact8 Wizard 단계, child reuse/create mode, guided Turret/Weapon/Projectile draft와 deterministic completion bundle 계약 추가.
// Migration:
// - Runtime DataAsset schema는 변경하지 않습니다. 신규 Capability flag와 Step state는 CarFight_ReEditor transient authoring state입니다.
// - v1.0.0의 PayloadChain/OptionalFeatures 고정 단계는 폐기되며 UI와 ViewModel은 VisibleSteps projection을 사용해야 합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFAmmoTypes.h"
#include "CFDamageTypes.h"
#include "CFLauncherTypes.h"
#include "CFMissileFlightTypes.h"
#include "CFMissileGuideTypes.h"
#include "CFProjectileMotorTypes.h"
#include "CFVehicleWeaponTypes.h"
#include "CFWeaponData.h"
#include "UObject/SoftObjectPath.h"

// 무장 제작 가이드의 semantic Stable StepId입니다. 배열 index가 사용자 의미를 소유하지 않습니다.
enum class ECFWeaponGuideStep : uint8
{
	IdentityTemplate = 0,
	MountCompatibility,
	MountGeometry,
	FireBehavior,
	Projectile,
	Damage,
	Ammo,
	Launcher,
	Propulsion,
	MissileFlight,
	Guidance,
	Heat,
	Charge,
	ReviewCreate,
	Complete
};

// 새 무장의 일반적인 Capability 조합을 제안하는 제작 Template입니다. Runtime hard class가 아닙니다.
enum class ECFWeaponGuideTemplate : uint8
{
	DirectFireCannon,
	AutomaticGun,
	RocketLauncher,
	GuidedMissileLauncher,
	Custom
};

// child DataAsset을 기존 자산으로 재사용할지 새로 생성할지 나타냅니다.
enum class ECFWeaponGuideChildMode : uint8
{
	ReuseExisting,
	CreateNew
};

// Guide validation 진단의 표시 심각도입니다.
enum class ECFWeaponGuideIssueSeverity : uint8
{
	Info,
	Warning,
	Error
};

// Guide validation 한 건의 사용자 표시 진단입니다.
struct FCFWeaponGuideIssue
{
	// 사용자에게 표시할 진단 심각도입니다.
	ECFWeaponGuideIssueSeverity Severity = ECFWeaponGuideIssueSeverity::Info;

	// 진단이 속한 Stable StepId입니다.
	ECFWeaponGuideStep Step = ECFWeaponGuideStep::IdentityTemplate;

	// 사용자에게 보여줄 한글 중심 진단 문구입니다.
	FString Message;
};

// 신규 TurretMountData를 Guided UX에서 작성할 핵심값입니다.
struct FCFWeaponGuideTurretDraft
{
	// 기존 TurretMountData를 재사용할지 새 자산을 만들지 결정합니다.
	ECFWeaponGuideChildMode Mode = ECFWeaponGuideChildMode::CreateNew;

	// 기존 TurretMountData를 재사용할 때의 exact object path입니다.
	FSoftObjectPath ExistingAssetPath;

	// 신규 TurretMountData의 베이스 Mesh입니다.
	FSoftObjectPath TurretBaseMeshPath;

	// 신규 TurretMountData의 Yaw Mesh입니다.
	FSoftObjectPath TurretYawMeshPath;

	// 신규 TurretMountData의 Pitch Mesh입니다.
	FSoftObjectPath TurretPitchMeshPath;

	// Yaw 회전축 Socket 이름입니다.
	FName YawPivotSocketName = TEXT("YawPivot");

	// Pitch 회전축 Socket 이름입니다.
	FName PitchPivotSocketName = TEXT("PitchPivot");

	// 기본 Muzzle Socket 이름입니다.
	FName PrimaryMuzzleSocketName = TEXT("Muzzle");

	// 다중 Muzzle을 사용할 때 추가로 연결할 Socket 이름입니다.
	TArray<FName> AdditionalMuzzleSocketNames;

	// 모든 Muzzle Socket 존재를 강제할지 여부입니다.
	bool bRequireAllMuzzles = false;

	// Yaw 최소 회전각입니다.
	float MinYawDeg = -180.0f;

	// Yaw 최대 회전각입니다.
	float MaxYawDeg = 180.0f;

	// Pitch 최소 회전각입니다.
	float MinPitchDeg = -10.0f;

	// Pitch 최대 회전각입니다.
	float MaxPitchDeg = 45.0f;

	// 초당 Yaw 회전 속도입니다.
	float YawTurnRateDegPerSec = 45.0f;

	// 초당 Pitch 회전 속도입니다.
	float PitchTurnRateDegPerSec = 30.0f;

	// 차량 피팅 질량에 더할 TurretMount 질량입니다.
	float TurretMountWeightKg = 0.0f;
};

// 신규 WeaponData를 Guided UX에서 작성할 핵심값입니다.
struct FCFWeaponGuideWeaponDraft
{
	// 기존 WeaponData를 재사용할지 새 자산을 만들지 결정합니다.
	ECFWeaponGuideChildMode Mode = ECFWeaponGuideChildMode::CreateNew;

	// 기존 WeaponData를 재사용할 때의 exact object path입니다.
	FSoftObjectPath ExistingAssetPath;

	// 신규 무기의 발사 처리 방식입니다.
	ECFWeaponFireMode FireMode = ECFWeaponFireMode::Projectile;

	// 분당 발사 수입니다.
	float FireRatePerMinute = 60.0f;

	// 기본 유효 사거리입니다.
	float MaxRange = 10000.0f;

	// 발사 방향 탄퍼짐 각도입니다.
	float SpreadDeg = 0.0f;

	// 차량 피팅 질량에 더할 무기 본체 질량입니다.
	float WeaponMassKg = 0.0f;

	// 탄창 한 개의 탄약 단위 용량입니다.
	int32 MagazineSize = 0;

	// 출격 초기 장전 탄약 단위 수입니다.
	int32 InitialLoadedAmmoCount = 0;

	// 정상 발사 한 번이 소비하는 탄약 단위 수입니다.
	int32 AmmoUnitsPerShot = 1;

	// 탄창 재장전 시간입니다.
	float ReloadTimeSeconds = 0.0f;

	// 유한 탄약 Runtime을 명시적으로 사용할지 여부입니다.
	bool bUseFiniteAmmo = false;

	// AmmoData 없이 compatibility 경로에서 사용할 탄종 ID입니다.
	FName AmmoTypeId = TEXT("ProtoShell");

	// 고급 Launcher 발사 패턴 Page를 사용할지 결정하는 Editor-only workflow intent입니다.
	bool bUseLauncher = false;

	// 입력 한 번의 런처 발사 패턴 설정입니다.
	FCFLauncherFirePatternConfig LauncherFirePatternConfig;

	// Muzzle에서 발사체가 분리되는 Release 설정입니다.
	FCFLauncherReleaseConfig LauncherReleaseConfig;

	// Heat 기능을 사용할지 여부입니다.
	bool bUseHeat = false;

	// 한 발 발사 시 누적할 열량입니다.
	float HeatPerShot = 0.0f;

	// Heat Runtime 최대 열량입니다.
	float MaxHeat = 0.0f;

	// 초당 자연 냉각량입니다.
	float HeatDissipationPerSecond = 0.0f;

	// Charge 기능을 사용할지 여부입니다.
	bool bUseCharge = false;

	// 무기 내부 최대 Charge입니다.
	float MaximumWeaponCharge = 0.0f;

	// 무기 Runtime 초기 Charge입니다.
	float InitialWeaponCharge = 0.0f;

	// 실제 한 발이 소비할 Charge입니다.
	float WeaponChargePerShot = 0.0f;

	// 초당 자연 회복할 Charge입니다.
	float WeaponChargeRecoveryPerSecond = 0.0f;
};

// 신규 ProjectileData를 Guided UX에서 작성할 핵심값입니다.
struct FCFWeaponGuideProjectileDraft
{
	// 기존 ProjectileData를 재사용할지 새 자산을 만들지 결정합니다.
	ECFWeaponGuideChildMode Mode = ECFWeaponGuideChildMode::CreateNew;

	// 기존 ProjectileData를 재사용할 때의 exact object path입니다.
	FSoftObjectPath ExistingAssetPath;

	// Projectile 모드에서 생성할 Projectile Actor class입니다. HitScan 가상 ProjectileData에서는 비어 있을 수 있습니다.
	FSoftClassPath ProjectileActorClassPath;

	// 신규 ProjectileData가 표시할 StaticMesh입니다.
	FSoftObjectPath ProjectileStaticMeshPath;

	// Projectile 초기 속도입니다.
	float InitialSpeed = 50000.0f;

	// Projectile 수명입니다.
	float LifeTimeSeconds = 3.0f;

	// 중력 영향을 받을지 여부입니다.
	bool bAffectedByGravity = false;

	// 중력 배율입니다.
	float GravityScale = 1.0f;

	// 충돌 판정 반경입니다.
	float CollisionRadius = 8.0f;

	// 이동 Sweep 충돌을 사용할지 여부입니다.
	bool bUseSweepCollision = true;

	// 자체 추진 Rocket/Missile 설정입니다.
	FCFProjectilePropulsionConfig PropulsionConfig;

	// Released~Terminal 미사일 비행 상태 설정입니다.
	FCFMissileFlightConfig MissileFlightConfig;

	// 목표 관측/유도/Seeker 물리 제한 설정입니다.
	FCFMissileGuideConfig MissileGuideConfig;
};

// 신규 DamageData를 Guide 내부에서 작성하거나 기존 자산을 재사용하기 위한 transient draft입니다.
struct FCFWeaponGuideDamageDraft
{
	// 기존 DamageData를 재사용할지 새 자산을 만들지 결정합니다.
	ECFWeaponGuideChildMode Mode = ECFWeaponGuideChildMode::ReuseExisting;

	// 기존 DamageData를 재사용할 때의 exact object path입니다.
	FSoftObjectPath ExistingAssetPath;

	// 신규 DamageData의 피해 종류입니다.
	ECFDamageType DamageType = ECFDamageType::Kinetic;

	// 신규 DamageData의 직접 기본 피해량입니다.
	float BaseDamage = 0.0f;

	// 자기 자신에게 피해를 허용할지 여부입니다.
	bool bCanDamageSelf = false;

	// 방어 계산에서 사용할 관통 값입니다.
	float ArmorPenetration = 0.0f;

	// 범위 피해를 사용할지 여부입니다.
	bool bUseRadialDamage = false;

	// 범위 피해 외곽 반경입니다.
	float ExplosionRadius = 0.0f;

	// 최대 피해를 유지할 내부 반경입니다.
	float ExplosionInnerRadius = 0.0f;

	// 범위 폭발 피해량입니다.
	float ExplosionDamage = 0.0f;

	// 외곽에서 허용할 최소 폭발 피해 배율입니다.
	float MinExplosionDamageScale = 0.0f;

	// 모듈 피해 배율입니다.
	float ModuleDamageScale = 0.0f;

	// 물리 충격량입니다.
	float ImpulseStrength = 0.0f;
};

// 신규 AmmoData를 Guide 내부에서 작성하거나 기존 자산을 재사용하기 위한 transient draft입니다.
struct FCFWeaponGuideAmmoDraft
{
	// 기존 AmmoData를 재사용할지 새 자산을 만들지 결정합니다.
	ECFWeaponGuideChildMode Mode = ECFWeaponGuideChildMode::ReuseExisting;

	// 기존 AmmoData를 재사용할 때의 exact object path입니다.
	FSoftObjectPath ExistingAssetPath;

	// 사용자에게 표시할 신규 AmmoData 이름입니다.
	FString AmmoDisplayName;

	// 선택적인 탄약 계열 ID입니다.
	FName AmmoFamilyId = NAME_None;

	// 탄약 한 단위의 질량 kg입니다.
	float UnitMassKg = 0.0f;

	// 분류와 검색에 사용할 set-like 탄약 태그입니다.
	TArray<FName> AmmoTags;

	// 선택적인 탄약 아이콘입니다.
	FSoftObjectPath AmmoIconPath;

	// 차량에 적재할 수 있는 최대 탄약 단위 수입니다.
	int32 MaximumLoadableAmmoCount = 0;

	// 재보급 가능 여부입니다.
	bool bCanBeResupplied = true;
};

// 무장 제작 가이드가 세션 동안 관리하는 전체 transient draft입니다.
struct FCFWeaponGuideDraft
{
	// 사용자가 입력하는 무장 에셋 기본 이름입니다.
	FString BaseAssetName;

	// 최종 EquipmentPreset에 넘길 게임 표시 이름 제안값입니다.
	FString SuggestedDisplayName;

	// 사용자가 이해하기 위한 무장 개념 메모입니다.
	FString ConceptNote;

	// 현재 선택한 제작 Template입니다.
	ECFWeaponGuideTemplate Template = ECFWeaponGuideTemplate::DirectFireCannon;

	// 최종 EquipmentPreset이 요구할 장착 타입입니다.
	ECFVehicleMountType RequiredMountType = ECFVehicleMountType::Turret;

	// 최종 EquipmentPreset이 요구할 무기 크기입니다.
	ECFVehicleWeaponSize RequiredWeaponSize = ECFVehicleWeaponSize::Large;

	// 장착/포탑 구조 Guided draft입니다.
	FCFWeaponGuideTurretDraft Turret;

	// 무기 동작 Guided draft입니다.
	FCFWeaponGuideWeaponDraft Weapon;

	// Projectile Guided draft입니다.
	FCFWeaponGuideProjectileDraft Projectile;

	// DamageData Guided draft입니다.
	FCFWeaponGuideDamageDraft Damage;

	// AmmoData Guided draft입니다.
	FCFWeaponGuideAmmoDraft Ammo;
};

// Weapon Guide 완료 후 Equipment Builder가 소비할 reference bundle입니다.
struct FCFWeaponGuideResultBundle
{
	// 신규 EquipmentPreset DisplayName 초안에 사용할 사용자 표시 이름입니다.
	FString SuggestedEquipmentDisplayName;

	// 최종 EquipmentPreset이 요구할 장착 타입입니다.
	ECFVehicleMountType RequiredMountType = ECFVehicleMountType::None;

	// 최종 EquipmentPreset이 요구할 무기 크기입니다.
	ECFVehicleWeaponSize RequiredWeaponSize = ECFVehicleWeaponSize::None;

	// 최종 TurretMountData exact object path입니다.
	FSoftObjectPath TurretMountDataPath;

	// 최종 WeaponData exact object path입니다.
	FSoftObjectPath WeaponDataPath;

	// 최종 ProjectileData exact object path입니다.
	FSoftObjectPath ProjectileDataPath;

	// finite ammo에서 선택하거나 생성한 AmmoData exact object path입니다.
	FSoftObjectPath AmmoDataPath;

	// AmmoData가 없을 때 compatibility fallback으로 전달할 탄종 ID입니다.
	FName AmmoTypeId = NAME_None;

	// ProjectileData가 참조하는 DamageData exact object path입니다.
	FSoftObjectPath DamageDataPath;

	// Bundle 전체 의미를 고정하는 canonical SHA-256 fingerprint입니다.
	FString DeterministicBundleFingerprint;

	// 모든 required reference와 fingerprint가 완성됐는지 여부입니다.
	bool bComplete = false;
};

// WEA-P0-04에서 Equipment Builder로 넘기는 navigation-only 인계 계약입니다.
struct FCFWeaponGuideEquipmentHandoff
{
	// 이동 대상 Nomad Tab의 고정 식별자입니다.
	FName TargetTabId = TEXT("CarFight.EquipmentBuilder");

	// 인계 계약이 어떤 완료 Bundle에서 만들어졌는지 고정하는 fingerprint입니다.
	FString SourceBundleFingerprint;

	// 현재 계약이 탭 이동만 허용하는지 여부입니다.
	bool bNavigationOnly = true;

	// Equipment Builder Draft/VM으로 값을 자동 주입하는지 여부입니다. WEA-P0-04에서는 반드시 false입니다.
	bool bInjectContext = false;
};
