// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-09-18
// Description: Phase 7 Guided Weapon Locked Target source migration + 차량 발사 행동 전용 내부 coordinator
// Changelog:
// - v1.1.0: TargetActor Guidance가 실제 활성인 Projectile만 Vehicle Locked Target을 발사 전 exact-once Guidance snapshot으로 요구하고, Direct Fire/비유도 Projectile은 기존 Aim/Fire 계약을 유지하도록 Guidance source/validation helper를 추가.
// - v1.0.1: FireRequest ID/시간과 입력 시 LastFireRequest 갱신을 Pawn Authority로 복귀. FireComp는 Pawn이 전달한 요청을 계산·검증·실행만 수행.
// - v1.0.0: Fire Command 생성/검증, Muzzle/Aim glue, HitScan/Projectile 실행, Launcher 후속 발사와 Fire side effect를 Pawn에서 분리. Pawn observable state와 기존 facade는 유지.
// Migration:
// - v1.1.0부터 TargetActor Guided Projectile의 GuidanceTargetActor source는 TargetSelect가 아니라 VehicleTargetingComp의 Locked Target입니다. HitScan/비유도 Projectile/TargetActor 이외 GuideMode에는 새 Lock 요구를 추가하지 않습니다.
// - v1.0.1부터 FireComp는 NextFireRequestId/LastFireRequest를 직접 변경하지 않습니다. 기존 Blueprint/Product Asset 수정은 필요하지 않습니다.
// - 기존 BP_CFVehiclePawn 계열은 VehicleFireComp 기본 서브오브젝트를 자동 상속합니다. Blueprint/Product Asset 수정은 필요하지 않으며 Launcher는 계속 Pawn compatibility callback만 호출합니다.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CFProjectileLaunchTypes.h"
#include "CFVehicleAimTypes.h"
#include "CFVehicleWeaponTypes.h"
#include "CFVehicleFireComp.generated.h"

class ACFProjectileActor;
class ACFVehiclePawn;
class UCFProjectileData;

/**
 * ACFVehiclePawn의 발사 계산과 실행 행동을 수행하는 내부 C++ coordinator입니다.
 * Pawn은 Fire observable/Damage Debug state Authority와 기존 compatibility facade를 계속 소유합니다.
 */
UCLASS()
class CARFIGHT_RE_API UCFVehicleFireComp : public UActorComponent
{
	GENERATED_BODY()

	friend class FCFPhase7GuidedWeaponTargetSourceTest;

public:
	// 내부 Fire coordinator의 기본 tick 비활성 설정을 초기화합니다.
	UCFVehicleFireComp();

	// Pawn이 할당한 요청 ID/시간을 사용해 현재 Aim 상태의 일반 발사 명령을 생성합니다.
	FCFVehicleFireRequest BuildFireCommand(int32 FireRequestId, float ClientFireTimeSeconds);

	// Pawn이 할당한 요청 ID/시간을 사용해 현재 Muzzle과 선택적 고정 Command Target의 발사 명령을 생성합니다.
	FCFVehicleFireRequest BuildFireCommandForTarget(int32 FireRequestId, float ClientFireTimeSeconds, const FVector& OverrideCommandTargetLocation, bool bUseOverrideTarget);

	// 플레이어 입력 발사의 전체 검증을 수행합니다.
	bool ValidateFireCommand(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& OutFireResult);

	// Launcher 후속 발사에서 입력 단위 쿨다운만 선택적으로 우회해 나머지 발사 조건을 동일하게 검증합니다.
	bool ValidateFireCommandInternal(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& OutFireResult, bool bIgnoreWeaponCooldown);

	// 싱글플레이 로컬 HitScan Trace를 실행하고 결과 및 Damage Debug를 갱신합니다.
	bool RunLocalDummyHitScan(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& InOutFireResult);

	// Dummy HitScan 결과를 기존 Pawn-owned Damage Debug state로 기록합니다.
	void RecordDummyHitScanDamageHitContext(const FCFVehicleFireRequest& FireCommand, const FCFVehicleFireResult& FireResult, const FHitResult* HitResult, bool bBlockingHit);

	// Projectile Pool에서 반환된 Hit 발사체의 Damage 결과를 기존 Pawn-owned Damage Debug state로 기록합니다.
	void RecordProjectileDamageHitContextFromPool(const ACFProjectileActor* InProjectileActor);

	// 현재 터렛 Pitch 메쉬의 유효 Muzzle 소켓으로 최종 FireOrigin을 보정합니다.
	bool TryBuildMuzzleFireOrigin(FCFVehicleFireOrigin& InOutFireOrigin, FString& OutFireOriginSummary) const;

	// Muzzle 위치와 현재/고정 Command Target을 사용해 공통 Weapon Aim Solution을 계산합니다.
	bool BuildWeaponAimSolution(FCFVehicleWeaponAimSolution& OutWeaponAimSolution, FCFVehicleFireOrigin* OutFireOrigin = nullptr, FString* OutFireOriginSummary = nullptr, const FVector* OverrideAimTargetLocation = nullptr) const;

	// 현재 Weapon Aim Solution을 다시 계산해 AimComp에 저장합니다.
	void RefreshWeaponAimSolution();

	// 현재 활성 무기가 Projectile Actor 실행 경로를 사용할 수 있는지 반환합니다.
	bool ShouldUseProjectileActorFire() const;

	// 현재 Release 설정과 고정 Guidance Actor Snapshot으로 Projectile Launch Context를 생성합니다.
	bool BuildDirectProjectileLaunchContext(const FCFVehicleFireRequest& FireCommand, const UCFProjectileData& InProjectileData, AActor* GuidanceTargetActorSnapshot, FCFProjectileLaunchContext& OutLaunchContext) const;

	// Phase 7 Guidance source 계약을 사용하는 기존 단발 Projectile 호환 실행 경로를 수행합니다.
	bool TrySpawnProjectileActorFromFireCommand(const FCFVehicleFireRequest& FireCommand);

	// 검증 승인된 명령을 Ammo Transaction과 함께 Projectile 또는 HitScan 경로로 실행합니다.
	bool ExecuteAcceptedFireCommand(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& InOutFireResult, AActor* GuidanceTargetActorSnapshot, bool bAllowProjectileFallback);

	// Launcher 예약 후속 발사를 첫 발사 순간 위치/Actor Snapshot을 유지해 실행합니다.
	bool ExecuteScheduledLauncherShot(int32 VolleyId, int32 SequenceShotIndex, const FVector& CommandTargetLocation, AActor* GuidanceTargetActorSnapshot);

	// Pawn이 먼저 observable로 기록한 입력 FireRequest와 목표 Snapshot을 사용해 Ammo/Launcher 발사 흐름을 실행합니다.
	void HandleFireStarted(const FCFVehicleFireRequest& FireRequest);

	// Pawn이 Fire observable state를 commit한 뒤 Weapon/Aim/CombatFx side effect를 기존 순서로 적용합니다.
	void ApplyFireResultSideEffects(const FCFVehicleFireRequest& FireCommand, const FCFVehicleFireResult& FireResult, bool bRecordCooldown);

private:
	// 이 컴포넌트를 소유한 차량 Pawn을 매 호출 확인합니다.
	ACFVehiclePawn* ResolveVehiclePawn() const;

	// 이 컴포넌트를 소유한 차량 Pawn을 const 형태로 매 호출 확인합니다.
	const ACFVehiclePawn* ResolveVehiclePawnConst() const;

	// Phase 7 기준 실제 Projectile Actor 실행 + TargetActor Guidance인 현재 무기만 Vehicle Locked Target을 요구하는지 반환합니다.
	bool DoesActiveProjectileRequireLockedGuidanceTarget() const;

	// 첫 발사 순간 TargetActor Guidance용 Vehicle Locked Target을 exact-once로 해석합니다. Lock이 필요 없는 무기는 nullptr 성공을 허용합니다.
	bool ResolveInitialGuidanceTargetActor(AActor*& OutGuidanceTargetActor) const;

	// 명시 전달된 Guidance Actor Snapshot이 현재 Projectile의 TargetActor Guidance 요구를 만족하는지 검사합니다.
	bool ValidateGuidanceTargetActorSnapshot(const UCFProjectileData& ProjectileData, AActor* GuidanceTargetActorSnapshot) const;
};
