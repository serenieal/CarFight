// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-07-02
// Description: CarFight 차량 발사체 DataAsset 구현
// Scope: 발사체 데이터 기본값과 디버그 요약 생성을 제공합니다.
// Changelog:
// - v1.4.0: ProjectileData를 물리 발사체와 가상 HitScan / Laser 발사 데이터의 공통 피해 소유자로 정리.
// - v1.3.0: DefaultDamageData 연결 여부와 DamageData 요약을 발사체 디버그 요약에 포함.
// - v1.2.0: 공통 Projectile Actor용 메시 설정을 디버그 요약에 포함.
// - v1.1.0: ProjectileActorClass 연결 여부를 스폰 준비 판정과 요약 문자열에 포함.
// - v1.0.0: ProjectileData 최소 필드와 디버그 요약 생성을 추가.
// Migration:
// - 기존 Dummy HitScan 흐름은 유지하고, FireMode가 Projectile일 때만 ProjectileActorClass를 실제 스폰 경로로 사용한다.
// - HitScan / Laser 피해는 ProjectileActorClass가 비어 있는 가상 ProjectileData의 DefaultDamageData로 연결한다.

#include "CFProjectileData.h"

#include "CFDamageData.h"

#include "Engine/StaticMesh.h"

// [v1.0.0] 기본 발사체 데이터 값을 초기화합니다.
UCFProjectileData::UCFProjectileData()
{
}

// [v1.3.0] 디버그 패널에 표시할 발사체 데이터, 스폰 준비, DamageData 연결 요약 문자열을 생성합니다.
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

	// [v1.4.0] 직접 참조된 DamageData가 있으면 DamageId를 우선 표시하고, 없으면 ProjectileData의 fallback ID를 표시합니다.
	const FString DamageDataText = DefaultDamageData ? DefaultDamageData->DamageId.ToString() : DamageProfileId.ToString();

	// [v1.3.0] 직접 참조된 DamageData 에셋 이름입니다.
	const FString DamageAssetText = DefaultDamageData ? DefaultDamageData->GetName() : TEXT("MissingOptional");

	// [v1.3.0] 직접 참조된 DamageData 요약입니다.
	const FString DamageSummaryText = DefaultDamageData ? DefaultDamageData->BuildDamageSummary() : TEXT("DamageData: MissingOptional");

	return FString::Printf(
		TEXT("ProjectileData: Id=%s, Speed=%.1f, Gravity=%s, GravityScale=%.2f, Life=%.2fs, CollisionRadius=%.1f, Actor=%s, ActorGate=%s, Mesh=%s, MeshScale=(%.2f, %.2f, %.2f), Impact=%s, Damage=%s, DamageAsset=%s, DamageSummary=[%s]"),
		*ProjectileId.ToString(),
		InitialSpeed,
		*GravityEnabledText,
		GravityScale,
		LifeTimeSeconds,
		CollisionRadius,
		*ProjectileActorClassText,
		*ProjectileActorGateText,
		*ProjectileMeshText,
		ProjectileMeshRelativeScale.X,
		ProjectileMeshRelativeScale.Y,
		ProjectileMeshRelativeScale.Z,
		*ImpactEffectId.ToString(),
		*DamageDataText,
		*DamageAssetText,
		*DamageSummaryText);
}

// [v1.1.0] Projectile Actor 스폰 후보 클래스가 지정되어 있는지 반환합니다.
bool UCFProjectileData::HasProjectileActorClass() const
{
	return ProjectileActorClass.Get() != nullptr;
}
