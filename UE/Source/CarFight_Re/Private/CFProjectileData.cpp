// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.9.0
// Date: 2026-07-30
// Description: CarFight 차량 발사체 DataAsset 구현
// Scope: 발사체 데이터 기본값, 추진·비행 FX, Projectile 요격과 기본 비활성 Missile Foundation 요약을 제공합니다.
// Changelog:
// - v1.9.0: 요격 가능 여부와 요격 시 폭발 FX 정책을 통합 Projectile 요약에 추가.
// - v1.8.0: Missile Flight·Guidance 유효 설정 Getter와 Foundation 요약을 추가하고 통합 Projectile 요약에 포함.
// - v1.7.1: Trail·Thruster 독립 FX Scale을 발사체 요약에 추가하고 소켓·Fallback 공통 적용 계약 반영.
// - v1.7.0: PropulsionConfig의 활성, 점화, 연소, 가속과 최대 속도를 발사체 요약에 추가.
// - v1.6.0: Trail·Thruster 기본 소켓과 비행 FX 구성 상태를 디버그 요약에 추가.
// - v1.5.0: Sweep / Sub-step / 보조 연속 Sweep / CCD 설정을 디버그 요약에 추가.
// - v1.4.0: ProjectileData를 물리 발사체와 가상 HitScan / Laser 발사 데이터의 공통 피해 소유자로 정리.
// - v1.3.0: DefaultDamageData 연결 여부와 DamageData 요약을 발사체 디버그 요약에 포함.
// - v1.2.0: 공통 Projectile Actor용 메시 설정을 디버그 요약에 포함.
// - v1.1.0: ProjectileActorClass 연결 여부를 스폰 준비 판정과 요약 문자열에 포함.
// - v1.0.0: ProjectileData 최소 필드와 디버그 요약 생성을 추가.
// Migration:
// - 기존 Dummy HitScan 흐름은 유지하고, FireMode가 Projectile일 때만 ProjectileActorClass를 실제 스폰 경로로 사용한다.
// - HitScan / Laser 피해는 ProjectileActorClass가 비어 있는 가상 ProjectileData의 DefaultDamageData로 연결한다.
// - 기존 ProjectileData 자산은 신규 연속 충돌 필드의 C++ 기본값을 사용하며 별도 자산 저장 없이 안전 기본값이 적용된다.
// - 신규 Trail·Thruster 슬롯은 기본 비활성이고 각각 FX_Trail, FX_Exhaust 소켓 이름을 기본값으로 사용한다.
// - 각 슬롯 RelativeTransform의 Scale은 소켓 사용 여부와 관계없이 독립 FX Scale로 적용한다.
// - PropulsionConfig는 기본 비활성이라 기존 ProjectileData의 InitialSpeed 고정 비행을 변경하지 않는다.
// - MissileFlightConfig와 MissileGuideConfig는 기본 비활성이며 기존 Projectile Actor·Motor·Movement 실행 경로에 연결하지 않는다.

#include "CFProjectileData.h"

#include "CFDamageData.h"

#include "Engine/StaticMesh.h"
#include "NiagaraSystem.h"

// [v1.6.0] 기존 호환 기본값과 Trail·추진 화염 기본 소켓 이름을 초기화합니다.
UCFProjectileData::UCFProjectileData()
{
	// [v1.6.0] Trail FX가 메시 소켓을 사용할 때 우선 검색할 기본 소켓입니다.
	TrailFxSettings.AttachSocketName = TEXT("FX_Trail");

	// [v1.6.0] 추진 화염 FX가 메시 소켓을 사용할 때 우선 검색할 기본 소켓입니다.
	ThrusterFxSettings.AttachSocketName = TEXT("FX_Exhaust");
}

// [v1.8.0] 안전하게 보정된 Missile Flight 설정을 값으로 반환합니다.
FCFMissileFlightConfig UCFProjectileData::GetEffectiveMissileFlightConfig() const
{
	return MissileFlightConfig.GetEffectiveConfig();
}

// [v1.8.0] 안전하게 보정된 Missile Guidance 설정을 값으로 반환합니다.
FCFMissileGuideConfig UCFProjectileData::GetEffectiveMissileGuideConfig() const
{
	return MissileGuideConfig.GetEffectiveConfig();
}

// [v1.8.0] Missile Flight·Guidance 활성 상태와 제한값을 한 줄로 반환합니다.
FString UCFProjectileData::BuildMissileFoundationSummary() const
{
	const FCFMissileFlightConfig EffectiveFlightConfig = GetEffectiveMissileFlightConfig();
	const FCFMissileGuideConfig EffectiveGuideConfig = GetEffectiveMissileGuideConfig();
	const UEnum* FlightStateEnum = StaticEnum<ECFMissileFlightState>();
	const UEnum* AttackProfileEnum = StaticEnum<ECFMissileAttackProfile>();
	const UEnum* GuideModeEnum = StaticEnum<ECFMissileGuideMode>();

	const FString InitialFlightStateText = FlightStateEnum
		? FlightStateEnum->GetNameStringByValue(static_cast<int64>(EffectiveFlightConfig.GetInitialFlightState()))
		: TEXT("Unknown");
	const FString AttackProfileText = AttackProfileEnum
		? AttackProfileEnum->GetNameStringByValue(static_cast<int64>(EffectiveFlightConfig.AttackProfile))
		: TEXT("Unknown");
	const FString GuideModeText = GuideModeEnum
		? GuideModeEnum->GetNameStringByValue(static_cast<int64>(EffectiveGuideConfig.GuideMode))
		: TEXT("Unknown");

	return FString::Printf(
		TEXT("MissileFoundation: Flight=%s, InitialState=%s, AttackProfile=%s, Clearance=%.3fs/%.1fcm, Transition=%.3fs, Guidance=%s, GuideMode=%s, MaxTurn=%.1fdeg/s, MaxLateralAccel=%.1fcm/s2, MinimumGuideSpeed=%.1fcm/s"),
		EffectiveFlightConfig.bUseMissileFlight ? TEXT("Enabled") : TEXT("Disabled"),
		*InitialFlightStateText,
		*AttackProfileText,
		EffectiveFlightConfig.MinimumClearanceTimeSeconds,
		EffectiveFlightConfig.MinimumClearanceDistanceCm,
		EffectiveFlightConfig.TransitionDurationSeconds,
		EffectiveGuideConfig.IsGuidanceEnabled() ? TEXT("Enabled") : TEXT("Disabled"),
		*GuideModeText,
		EffectiveGuideConfig.MaximumTurnRateDegPerSec,
		EffectiveGuideConfig.MaximumLateralAccelerationCmPerSecSq,
		EffectiveGuideConfig.MinimumGuidanceSpeedCmPerSec);
}

// [v1.8.0] 디버그 패널에 표시할 발사체 데이터, 추진, Missile Foundation, 연속 충돌, 비행 FX와 DamageData 연결 요약 문자열을 생성합니다.
FString UCFProjectileData::BuildProjectileSummary() const
{
	// [v1.0.0] Projectile Actor Class 지정 여부를 표시할 문자열입니다.
	const FString ProjectileActorClassText = ProjectileActorClass ? ProjectileActorClass->GetName() : TEXT("None");

	// [v1.2.0] Projectile Actor Class 지정 여부를 표시할 문자열입니다.
	const FString ProjectileActorGateText = HasProjectileActorClass() ? TEXT("ActorClassAssigned") : TEXT("ActorClassMissing");

	// [v1.2.0] Projectile StaticMesh 지정 여부를 표시할 문자열입니다.
	const FString ProjectileMeshText = ProjectileStaticMesh ? ProjectileStaticMesh->GetName() : TEXT("None");

	// [v1.0.0] 중력 적용 여부를 표시할 문자열입니다.
	const FString GravityEnabledText = bAffectedByGravity ? TEXT("Yes") : TEXT("No");

	// [v1.7.0] 실제 자체 추진 사용 여부를 표시할 문자열입니다.
	const FString PropulsionEnabledText = PropulsionConfig.bUsePropulsion ? TEXT("Yes") : TEXT("No");

	// [v1.5.0] ProjectileMovement Sweep 사용 여부를 표시할 문자열입니다.
	const FString SweepCollisionText = bUseSweepCollision ? TEXT("Yes") : TEXT("No");

	// [v1.5.0] 강제 Sub-step 사용 여부를 표시할 문자열입니다.
	const FString ForceSubSteppingText = bForceSubStepping ? TEXT("Yes") : TEXT("No");

	// [v1.5.0] 보조 연속 Sphere Sweep 사용 여부를 표시할 문자열입니다.
	const FString SupplementalSweepText = bUseSupplementalContinuousSweep ? TEXT("Yes") : TEXT("No");

		// [v1.5.0] CCD 보조 사용 여부를 표시할 문자열입니다.
	const FString CCDText = bUseCCD ? TEXT("Yes") : TEXT("No");

	// [v1.9.0] 다른 발사 차량의 유효 Projectile·Hitscan 적중으로 요격 가능한지 표시할 문자열입니다.
	const FString InterceptableText = bCanBeIntercepted ? TEXT("Yes") : TEXT("No");

	// [v1.9.0] Intercepted 종료 시 기본 Impact FX를 요청하는지 표시할 문자열입니다.
	const FString DetonateOnInterceptText = bDetonateWhenIntercepted ? TEXT("Yes") : TEXT("No");

	// [v1.6.0] 비행 FX 슬롯의 활성화와 Niagara 연결 상태를 짧게 표시하는 함수입니다.
	const auto BuildFlightFxStatusText = [](const FCFProjectileAttachedFxSettings& InFxSettings) -> FString
	{
		if (!InFxSettings.bEnabled)
		{
			return TEXT("Disabled");
		}

		return InFxSettings.NiagaraSystem ? InFxSettings.NiagaraSystem->GetName() : TEXT("MissingSystem");
	};

	// [v1.6.0] 비행 FX 슬롯의 부착 모드를 표시하는 함수입니다.
	const auto BuildFlightFxAttachModeText = [](const FCFProjectileAttachedFxSettings& InFxSettings) -> FString
	{
		return InFxSettings.AttachMode == ECFProjectileFxAttachMode::MeshSocketWithFallback
			? TEXT("MeshSocketWithFallback")
			: TEXT("ProjectileRelative");
	};

	// [v1.6.0] Trail FX의 활성화와 Niagara 연결 상태입니다.
	const FString TrailFxStatusText = BuildFlightFxStatusText(TrailFxSettings);

	// [v1.6.0] Trail FX의 부착 모드입니다.
	const FString TrailFxAttachModeText = BuildFlightFxAttachModeText(TrailFxSettings);

	// [v1.6.0] 추진 화염 FX의 활성화와 Niagara 연결 상태입니다.
	const FString ThrusterFxStatusText = BuildFlightFxStatusText(ThrusterFxSettings);

	// [v1.6.0] 추진 화염 FX의 부착 모드입니다.
	const FString ThrusterFxAttachModeText = BuildFlightFxAttachModeText(ThrusterFxSettings);

	// [v1.4.0] 직접 참조된 DamageData가 있으면 DamageId를 우선 표시하고, 없으면 ProjectileData의 fallback ID를 표시합니다.
	const FString DamageDataText = DefaultDamageData ? DefaultDamageData->DamageId.ToString() : DamageProfileId.ToString();

	// [v1.3.0] 직접 참조된 DamageData 에셋 이름입니다.
	const FString DamageAssetText = DefaultDamageData ? DefaultDamageData->GetName() : TEXT("MissingOptional");

			// [v1.3.0] 직접 참조된 DamageData 요약입니다.
	const FString DamageSummaryText = DefaultDamageData ? DefaultDamageData->BuildDamageSummary() : TEXT("DamageData: MissingOptional");

	// [v1.7.1] Trail 슬롯이 소켓·Fallback 경로에 공통 사용할 독립 FX Scale입니다.
	const FVector TrailFxScale = TrailFxSettings.RelativeTransform.GetScale3D();

		// [v1.7.1] 추진 슬롯이 소켓·Fallback 경로에 공통 사용할 독립 FX Scale입니다.
	const FVector ThrusterFxScale = ThrusterFxSettings.RelativeTransform.GetScale3D();

	// [v1.8.0] 기존 런타임과 독립적인 Missile Flight·Guidance Foundation 요약입니다.
	const FString MissileFoundationSummary = BuildMissileFoundationSummary();

	return FString::Printf(
				TEXT("ProjectileData: Id=%s, InitialSpeed=%.1f, Propulsion=%s, IgnitionDelay=%.3fs, BurnDuration=%.3fs, ThrustAcceleration=%.1fcm/s2, MaximumPropelledSpeed=%.1fcm/s, Gravity=%s, GravityScale=%.2f, Life=%.2fs, CollisionRadius=%.1f, Sweep=%s, SubStep=%s, MaxStep=%.6f, Iterations=%d, SupplementalSweep=%s, CCD=%s, Interceptable=%s, DetonateOnIntercept=%s, Actor=%s, ActorGate=%s, Mesh=%s, MeshScale=(%.2f, %.2f, %.2f), TrailFx=%s, TrailAttach=%s, TrailSocket=%s, TrailScale=(%.3f, %.3f, %.3f), ThrusterFx=%s, ThrusterAttach=%s, ThrusterSocket=%s, ThrusterScale=(%.3f, %.3f, %.3f), Impact=%s, Damage=%s, DamageAsset=%s, DamageSummary=[%s], Missile=[%s]"),
		*ProjectileId.ToString(),
		InitialSpeed,
		*PropulsionEnabledText,
		PropulsionConfig.IgnitionDelaySeconds,
		PropulsionConfig.BurnDurationSeconds,
		PropulsionConfig.ThrustAccelerationCmPerSecSq,
		PropulsionConfig.MaximumPropelledSpeed,
		*GravityEnabledText,
		GravityScale,
		LifeTimeSeconds,
		CollisionRadius,
		*SweepCollisionText,
		*ForceSubSteppingText,
		MaxSimulationTimeStep,
		MaxSimulationIterations,
				*SupplementalSweepText,
		*CCDText,
		*InterceptableText,
		*DetonateOnInterceptText,
		*ProjectileActorClassText,
		*ProjectileActorGateText,
		*ProjectileMeshText,
		ProjectileMeshRelativeScale.X,
		ProjectileMeshRelativeScale.Y,
		ProjectileMeshRelativeScale.Z,
		*TrailFxStatusText,
		*TrailFxAttachModeText,
		*TrailFxSettings.AttachSocketName.ToString(),
		TrailFxScale.X,
		TrailFxScale.Y,
		TrailFxScale.Z,
		*ThrusterFxStatusText,
		*ThrusterFxAttachModeText,
		*ThrusterFxSettings.AttachSocketName.ToString(),
		ThrusterFxScale.X,
		ThrusterFxScale.Y,
		ThrusterFxScale.Z,
		*ImpactEffectId.ToString(),
				*DamageDataText,
		*DamageAssetText,
		*DamageSummaryText,
		*MissileFoundationSummary);
}

// [v1.1.0] Projectile Actor 스폰 후보 클래스가 지정되어 있는지 반환합니다.
bool UCFProjectileData::HasProjectileActorClass() const
{
	return ProjectileActorClass.Get() != nullptr;
}
