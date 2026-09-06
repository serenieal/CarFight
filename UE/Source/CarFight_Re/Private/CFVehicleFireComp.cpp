// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.1
// Date: 2026-09-06
// Description: CF-FQ-048 VPS-P0-02 차량 발사 행동 전용 내부 coordinator 구현
// Changelog:
// - v1.0.1: FireRequest ID/시간 할당과 입력 시 LastFireRequest commit을 Pawn Authority로 복귀시키고 FireComp는 전달받은 요청의 계산·검증·실행만 수행.
// - v1.0.0: Fire Command/검증/Muzzle-Aim/HitScan/Projectile/Launcher 후속 발사와 Fire side effect를 Pawn에서 분리하고 기존 Pawn observable state authority를 유지.
// Migration:
// - v1.0.1부터 FireComp는 NextFireRequestId/LastFireRequest를 직접 변경하지 않습니다. 기존 compatibility facade와 Product Asset에는 변경이 필요 없습니다.
// - ACFVehiclePawn의 기존 함수 시그니처는 compatibility wrapper로 유지합니다. Launcher/Automation/Blueprint 호출자는 새 컴포넌트를 직접 알 필요가 없습니다.

#include "CFVehicleFireComp.h"

#include "CFCollisionChannels.h"
#include "CFCombatFxComp.h"
#include "CFDamageData.h"
#include "CFLauncherComp.h"
#include "CFLauncherTypes.h"
#include "CFProjectileActor.h"
#include "CFProjectileData.h"
#include "CFProjectilePoolComp.h"
#include "CFTargetSelectComp.h"
#include "CFTurretMountData.h"
#include "CFVehicleAimComp.h"
#include "CFVehicleAmmoComp.h"
#include "CFVehicleCameraComp.h"
#include "CFVehicleDefenseComp.h"
#include "CFVehicleHealthComp.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"

#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

namespace
{
	// 현재 활성 무기가 SingleCycle 유한탄 Transaction 대상인지 반환합니다.
	bool ShouldUseSingleFireAmmoTransaction(const ACFVehiclePawn* VehiclePawn)
	{
		if (!VehiclePawn)
		{
			return false;
		}

		// 현재 활성 WeaponData와 Launcher Pattern을 제공하는 Weapon 컴포넌트입니다.
		const UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();
		if (!VehicleWeaponComp)
		{
			return false;
		}

		// 유한탄 사용 여부를 판정할 현재 활성 WeaponData입니다.
		const UCFWeaponData* ActiveWeaponData = VehicleWeaponComp->GetActiveWeaponData();
		if (!ActiveWeaponData || !ActiveWeaponData->UsesFiniteAmmoRuntime())
		{
			return false;
		}

		// SingleCycle과 Ripple/Salvo 책임 경계를 구분할 현재 발사 패턴입니다.
		const FCFLauncherFirePatternConfig FirePatternConfig = VehicleWeaponComp->GetActiveLauncherFirePatternConfig();
		return FirePatternConfig.FirePattern == ECFLauncherFirePattern::SingleCycle;
	}

	// 현재 활성 MountProfileId를 Ammo Runtime의 WeaponInstanceId로 반환합니다.
	FName ResolveActiveAmmoWeaponInstanceId(const ACFVehiclePawn* VehiclePawn)
	{
		// WeaponInstanceId 원본을 제공하는 현재 Weapon 컴포넌트입니다.
		const UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn ? VehiclePawn->GetVehicleWeaponComp() : nullptr;
		return VehicleWeaponComp ? VehicleWeaponComp->GetActiveMountProfileId() : NAME_None;
	}

	// Ammo Transaction 결과를 기존 Fire reject reason으로 변환합니다.
	ECFVehicleFireRejectReason ResolveAmmoTransactionFireRejectReason(const ECFAmmoTransactionResult AmmoTransactionResult)
	{
		switch (AmmoTransactionResult)
		{
		case ECFAmmoTransactionResult::Accepted:
			return ECFVehicleFireRejectReason::None;
		case ECFAmmoTransactionResult::Reloading:
			return ECFVehicleFireRejectReason::Reloading;
		case ECFAmmoTransactionResult::SequenceAlreadyActive:
		case ECFAmmoTransactionResult::ActionLocked:
			return ECFVehicleFireRejectReason::WeaponActionLocked;
		case ECFAmmoTransactionResult::ExecutionFailed:
			return ECFVehicleFireRejectReason::InvalidLocalState;
		case ECFAmmoTransactionResult::MissingAmmoData:
		case ECFAmmoTransactionResult::MissingWeaponRuntime:
		case ECFAmmoTransactionResult::InvalidAmmoAmount:
		case ECFAmmoTransactionResult::NotEnoughLoadedAmmo:
		default:
			return ECFVehicleFireRejectReason::NoAmmo;
		}
	}

	// 실패한 SingleCycle 실행의 Ammo 예약을 안전하게 반환합니다.
	void RollbackSingleFireAmmoReservation(ACFVehiclePawn* VehiclePawn, const FName WeaponInstanceId, const bool bAmmoReservationActive)
	{
		if (!bAmmoReservationActive || !VehiclePawn)
		{
			return;
		}

		// 현재 SingleCycle 예약을 소유하는 VehicleAmmoComp입니다.
		UCFVehicleAmmoComp* VehicleAmmoComp = VehiclePawn->GetVehicleAmmoComp();
		if (VehicleAmmoComp)
		{
			VehicleAmmoComp->RollbackSingleFireAmmo(WeaponInstanceId);
		}
	}

	// 성공한 SingleCycle 실행의 Ammo 예약을 실제 소비로 확정합니다.
	bool CommitSingleFireAmmoReservation(ACFVehiclePawn* VehiclePawn, const FName WeaponInstanceId, const bool bAmmoReservationActive)
	{
		if (!bAmmoReservationActive)
		{
			return true;
		}
		if (!VehiclePawn)
		{
			return false;
		}

		// 현재 SingleCycle 예약을 실제 소비로 확정할 VehicleAmmoComp입니다.
		UCFVehicleAmmoComp* VehicleAmmoComp = VehiclePawn->GetVehicleAmmoComp();
		return VehicleAmmoComp
			&& VehicleAmmoComp->CommitSingleFireAmmo(WeaponInstanceId) == ECFAmmoTransactionResult::Accepted;
	}
}

// 내부 Fire coordinator의 기본 tick 비활성 설정을 초기화합니다.
UCFVehicleFireComp::UCFVehicleFireComp()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// 이 컴포넌트를 소유한 차량 Pawn을 매 호출 확인합니다.
ACFVehiclePawn* UCFVehicleFireComp::ResolveVehiclePawn() const
{
	return Cast<ACFVehiclePawn>(GetOwner());
}

// 이 컴포넌트를 소유한 차량 Pawn을 const 형태로 매 호출 확인합니다.
const ACFVehiclePawn* UCFVehicleFireComp::ResolveVehiclePawnConst() const
{
	return Cast<ACFVehiclePawn>(GetOwner());
}

// Pawn이 할당한 요청 ID/시간을 사용해 현재 Aim 상태의 일반 발사 명령을 생성합니다.
FCFVehicleFireRequest UCFVehicleFireComp::BuildFireCommand(const int32 FireRequestId, const float ClientFireTimeSeconds)
{
	return BuildFireCommandForTarget(FireRequestId, ClientFireTimeSeconds, FVector::ZeroVector, false);
}

// Pawn이 할당한 요청 ID/시간을 사용해 현재 Muzzle과 선택적 고정 Command Target의 발사 명령을 생성합니다.
FCFVehicleFireRequest UCFVehicleFireComp::BuildFireCommandForTarget(
	const int32 FireRequestId,
	const float ClientFireTimeSeconds,
	const FVector& OverrideCommandTargetLocation,
	const bool bUseOverrideTarget)
{
	// Fire 행동 대상 차량 Pawn입니다. Request ID/시간 Authority는 호출한 Pawn wrapper가 소유합니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return FCFVehicleFireRequest();
	}

	// AimComp가 없을 때도 크래시 없이 반환할 fallback 발사 명령입니다.
	FCFVehicleFireRequest FireRequest;
	if (VehiclePawn->VehicleAimComp)
	{
		FireRequest = VehiclePawn->VehicleAimComp->BuildFireRequest(FireRequestId, ClientFireTimeSeconds);
	}
	else
	{
		FireRequest.FireRequestId = FireRequestId;
		FireRequest.ClientFireTimeSeconds = ClientFireTimeSeconds;
		FireRequest.AimOrigin = VehiclePawn->GetActorLocation();
		FireRequest.AimDirection = VehiclePawn->GetActorForwardVector();
		FireRequest.PredictedAimTargetLocation = VehiclePawn->GetActorLocation() + VehiclePawn->GetActorForwardVector() * 1000.0f;
	}

	if (bUseOverrideTarget && !OverrideCommandTargetLocation.ContainsNaN())
	{
		FireRequest.PredictedAimTargetLocation = OverrideCommandTargetLocation;
	}

	if (VehiclePawn->VehicleWeaponComp)
	{
		// Muzzle 기준으로 계산한 공통 Weapon Aim Solution입니다.
		FCFVehicleWeaponAimSolution WeaponAimSolution;

		// Weapon Aim Solution과 같은 기준으로 기록할 FireOrigin입니다.
		FCFVehicleFireOrigin FinalFireOrigin;

		// Weapon Aim Solution 계산 결과를 WeaponComp 디버그에 남길 요약입니다.
		FString WeaponAimSolutionSummary;

		// 후속 Ripple/Salvo 발사에서만 사용할 첫 입력 순간 고정 Command Target 포인터입니다.
		const FVector* OverrideTargetLocation = bUseOverrideTarget ? &OverrideCommandTargetLocation : nullptr;

		if (BuildWeaponAimSolution(WeaponAimSolution, &FinalFireOrigin, &WeaponAimSolutionSummary, OverrideTargetLocation))
		{
			VehiclePawn->VehicleWeaponComp->RecordResolvedFireOrigin(FinalFireOrigin, WeaponAimSolutionSummary);
			if (VehiclePawn->VehicleAimComp && !bUseOverrideTarget)
			{
				VehiclePawn->VehicleAimComp->SetWeaponAimSolution(WeaponAimSolution);
			}

			FireRequest.AimOrigin = WeaponAimSolution.AimOrigin;
			FireRequest.AimDirection = WeaponAimSolution.AimDirection;
			FireRequest.PredictedAimTargetLocation = WeaponAimSolution.AimTargetLocation;
			FireRequest.MuzzleSocketName = FinalFireOrigin.MuzzleSocketName;
			FireRequest.MuzzleSocketIndex = FinalFireOrigin.MuzzleSocketIndex;
			FireRequest.MuzzleSocketCount = FinalFireOrigin.MuzzleSocketCount;
			FireRequest.WeaponGroupId = FinalFireOrigin.MountProfileId;
		}
		else
		{
			if (VehiclePawn->VehicleAimComp && !bUseOverrideTarget)
			{
				VehiclePawn->VehicleAimComp->SetWeaponAimSolution(WeaponAimSolution);
			}
			FireRequest.AimDirection = FVector::ZeroVector;
		}
	}

	return FireRequest;
}

// 플레이어 입력 발사의 전체 검증을 수행합니다.
bool UCFVehicleFireComp::ValidateFireCommand(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& OutFireResult)
{
	return ValidateFireCommandInternal(FireCommand, OutFireResult, false);
}

// Launcher 후속 발사에서 입력 단위 쿨다운만 선택적으로 우회해 나머지 발사 조건을 동일하게 검증합니다.
bool UCFVehicleFireComp::ValidateFireCommandInternal(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& OutFireResult, const bool bIgnoreWeaponCooldown)
{
	// 발사 검증 대상 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	OutFireResult = FCFVehicleFireResult();
	OutFireResult.FireRequestId = FireCommand.FireRequestId;
	OutFireResult.ValidationAimTargetLocation = FireCommand.PredictedAimTargetLocation;
	OutFireResult.LocalHitLocation = FireCommand.PredictedAimTargetLocation;

	if (!VehiclePawn || !VehiclePawn->GetController())
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidOwner;
		return false;
	}

	if (!VehiclePawn->VehicleAimComp || !VehiclePawn->VehicleWeaponComp)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::NoWeapon;
		return false;
	}

	if (!VehiclePawn->VehicleAimComp->IsAimRuntimeReady())
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::VehicleDisabled;
		return false;
	}

	// 일반 입력은 AimComp 캐시를, 후속 시퀀스는 고정 Command Target으로 다시 계산한 최신 Weapon Aim Solution을 사용합니다.
	FCFVehicleWeaponAimSolution WeaponAimSolution;
	if (bIgnoreWeaponCooldown)
	{
		// 첫 입력 순간 고정된 후속 발사 Command Target입니다.
		const FVector ScheduledCommandTargetLocation = FVector(FireCommand.PredictedAimTargetLocation);
		if (!BuildWeaponAimSolution(WeaponAimSolution, nullptr, nullptr, &ScheduledCommandTargetLocation))
		{
			OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimDirection;
			return false;
		}
	}
	else
	{
		WeaponAimSolution = VehiclePawn->VehicleAimComp->GetWeaponAimSolution();
	}

	if (!WeaponAimSolution.bHasValidSolution)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimDirection;
		return false;
	}

	// 로컬 검증에 사용할 발사 명령 조준 방향입니다.
	const FVector AimDirection = FVector(FireCommand.AimDirection);
	if (AimDirection.ContainsNaN() || AimDirection.IsNearlyZero())
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimDirection;
		return false;
	}

	// 로컬 검증에 사용할 발사 명령 조준 시작 위치입니다.
	const FVector AimOrigin = FVector(FireCommand.AimOrigin);
	if (AimOrigin.ContainsNaN())
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimOrigin;
		return false;
	}

	// Pawn 위치와 발사 명령 조준 시작점 사이 거리입니다.
	const float AimOriginDistance = FVector::Dist(AimOrigin, VehiclePawn->GetActorLocation());

	// 기본 Aim Profile에서 허용하는 최대 거리입니다.
	const float MaxAimDistance = VehiclePawn->VehicleAimComp->GetDefaultAimProfile().MaxAimDistance;
	if (AimOriginDistance > MaxAimDistance)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimOrigin;
		return false;
	}

	// Direct는 기존 Command Target 방향의 MuzzleBlocked 결과를 사용하고 비Direct는 실제 사출 방향 검사를 실행 직전에 수행합니다.
	const FCFLauncherReleaseConfig ActiveReleaseConfig = VehiclePawn->VehicleWeaponComp->GetActiveLauncherReleaseConfig();
	if (ActiveReleaseConfig.ReleaseMode == ECFProjectileReleaseMode::Direct && WeaponAimSolution.bMuzzleBlocked)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::MuzzleBlocked;
		return false;
	}

	if (!WeaponAimSolution.bAllowFireWhileAligning && WeaponAimSolution.bTurretAligning)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::TurretAligning;
		return false;
	}

	if (!WeaponAimSolution.bAllowFireWhileAligning && WeaponAimSolution.bWeaponNotAligned)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::WeaponNotAligned;
		return false;
	}

	if (VehiclePawn->VehicleWeaponComp->GetActiveWeaponData())
	{
		// 활성 WeaponData가 현재 MountProfile과 호환되는지 여부입니다.
		const bool bActiveWeaponDataCompatible = VehiclePawn->VehicleWeaponComp->IsActiveWeaponDataCompatible();
		if (!bActiveWeaponDataCompatible)
		{
			OutFireResult.RejectReason = ECFVehicleFireRejectReason::NoWeapon;
			return false;
		}

		if (!VehiclePawn->VehicleWeaponComp->CanActiveWeaponAcceptHeatShot())
		{
			OutFireResult.RejectReason = ECFVehicleFireRejectReason::WeaponOverheated;
			return false;
		}

		if (!VehiclePawn->VehicleWeaponComp->CanActiveWeaponAcceptChargeShot())
		{
			OutFireResult.RejectReason = ECFVehicleFireRejectReason::WeaponChargeInsufficient;
			return false;
		}

		// 이번 발사 명령의 월드 시간입니다.
		const float CurrentFireTimeSeconds = FireCommand.ClientFireTimeSeconds;

		// 현재 활성 무기가 아직 쿨다운 중인지 여부입니다.
		const bool bActiveWeaponOnCooldown = !bIgnoreWeaponCooldown
			&& VehiclePawn->VehicleWeaponComp->IsActiveWeaponOnCooldown(CurrentFireTimeSeconds);
		if (bActiveWeaponOnCooldown)
		{
			OutFireResult.RejectReason = ECFVehicleFireRejectReason::WeaponCooldown;
			return false;
		}
	}

	if (ShouldUseSingleFireAmmoTransaction(VehiclePawn))
	{
		if (!VehiclePawn->VehicleAmmoComp)
		{
			OutFireResult.RejectReason = ECFVehicleFireRejectReason::NoAmmo;
			return false;
		}

		// 현재 MountProfile에 대응하는 Ammo Runtime WeaponInstanceId입니다.
		const FName AmmoWeaponInstanceId = ResolveActiveAmmoWeaponInstanceId(VehiclePawn);

		// 실제 발사 실행 전에 장전량/Reload/Action Lock을 확인한 결과입니다.
		const ECFAmmoTransactionResult AmmoValidationResult = VehiclePawn->VehicleAmmoComp->ValidateSingleFireAmmo(AmmoWeaponInstanceId);
		if (AmmoValidationResult != ECFAmmoTransactionResult::Accepted)
		{
			OutFireResult.RejectReason = ResolveAmmoTransactionFireRejectReason(AmmoValidationResult);
			return false;
		}
	}

	OutFireResult.bAccepted = true;
	OutFireResult.RejectReason = ECFVehicleFireRejectReason::None;
	if (ShouldUseProjectileActorFire())
	{
		// Projectile 모드에서 예측 표시용으로 사용할 최대 비행 거리입니다.
		const float ProjectilePredictionDistance = VehiclePawn->VehicleWeaponComp->GetActiveWeaponMaxRange(MaxAimDistance);

		// Projectile 모드에서 즉시 HitScan 없이 표시할 예측 위치입니다.
		const FVector ProjectilePredictionLocation = AimOrigin + AimDirection.GetSafeNormal() * ProjectilePredictionDistance;

		OutFireResult.ValidationAimTargetLocation = ProjectilePredictionLocation;
		OutFireResult.LocalHitLocation = ProjectilePredictionLocation;
		OutFireResult.LocalHitNormal = FVector::UpVector;
	}

	return true;
}

// 싱글플레이 로컬 HitScan Trace를 실행하고 결과 및 Damage Debug를 갱신합니다.
bool UCFVehicleFireComp::RunLocalDummyHitScan(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& InOutFireResult)
{
	// HitScan 실행 대상 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn || !VehiclePawn->VehicleAimComp)
	{
		return false;
	}

	// 로컬 Trace에 사용할 월드입니다.
	UWorld* World = VehiclePawn->GetWorld();

	// 로컬 Trace 시작 위치입니다.
	const FVector TraceStart = FVector(FireCommand.AimOrigin);

	// 로컬 Trace 방향입니다.
	const FVector TraceDirection = FVector(FireCommand.AimDirection).GetSafeNormal();

	// WeaponData가 없을 때 사용할 기존 Aim Profile 최대 거리입니다.
	const float FallbackTraceDistance = VehiclePawn->VehicleAimComp->GetDefaultAimProfile().MaxAimDistance;

	// 로컬 Trace에 사용할 최종 최대 거리입니다.
	const float TraceDistance = VehiclePawn->VehicleWeaponComp ? VehiclePawn->VehicleWeaponComp->GetActiveWeaponMaxRange(FallbackTraceDistance) : FallbackTraceDistance;

	// 로컬 Trace 종료 위치입니다.
	const FVector TraceEnd = TraceStart + TraceDirection * TraceDistance;

	if (!World || TraceDirection.IsNearlyZero())
	{
		InOutFireResult.ValidationAimTargetLocation = TraceEnd;
		InOutFireResult.LocalHitLocation = TraceEnd;
		InOutFireResult.LocalHitNormal = FVector::UpVector;
		RecordDummyHitScanDamageHitContext(FireCommand, InOutFireResult, nullptr, false);
		return false;
	}

	// 로컬 Trace 적중 결과입니다.
	FHitResult HitResult;

	// 로컬 Trace에서 Owner Pawn과 현재 활성 source projectile을 무시하기 위한 Query 설정입니다.
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CarFightLocalAimTrace), false);
	QueryParams.AddIgnoredActor(VehiclePawn);
	if (VehiclePawn->ProjectilePoolComp)
	{
		VehiclePawn->ProjectilePoolComp->AddActiveSourceProjectilesToQueryParams(VehiclePawn, QueryParams);
	}

	// 로컬 Trace가 무기 피격 전용 채널에 적중했는지 여부입니다.
	const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CFCollisionChannels::WeaponHit, QueryParams);
	if (bHit)
	{
		InOutFireResult.ValidationAimTargetLocation = HitResult.ImpactPoint;
		InOutFireResult.LocalHitLocation = HitResult.ImpactPoint;
		InOutFireResult.LocalHitNormal = HitResult.ImpactNormal;
	}
	else
	{
		InOutFireResult.ValidationAimTargetLocation = TraceEnd;
		InOutFireResult.LocalHitLocation = TraceEnd;
		InOutFireResult.LocalHitNormal = FVector::UpVector;
	}

	RecordDummyHitScanDamageHitContext(FireCommand, InOutFireResult, bHit ? &HitResult : nullptr, bHit);

	if (VehiclePawn->bDrawLocalAimTraceDebug)
	{
		// 로컬 Trace 디버그 라인 색상입니다.
		const FColor TraceColor = bHit ? FColor::Green : FColor::Red;
		DrawDebugLine(World, TraceStart, bHit ? HitResult.ImpactPoint : TraceEnd, TraceColor, false, VehiclePawn->LocalAimTraceDebugDuration, 0, 2.0f);
		if (bHit)
		{
			DrawDebugSphere(World, HitResult.ImpactPoint, 24.0f, 12, FColor::Yellow, false, VehiclePawn->LocalAimTraceDebugDuration);
		}
	}

	return bHit;
}

// Dummy HitScan 결과를 기존 Pawn-owned Damage Debug state로 기록합니다.
void UCFVehicleFireComp::RecordDummyHitScanDamageHitContext(const FCFVehicleFireRequest& FireCommand, const FCFVehicleFireResult& FireResult, const FHitResult* HitResult, const bool bBlockingHit)
{
	// Damage/CombatFx observable Authority를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return;
	}

	// 현재 활성 DamageData입니다.
	UCFDamageData* ActiveDamageData = VehiclePawn->VehicleWeaponComp ? VehiclePawn->VehicleWeaponComp->GetActiveDamageData() : nullptr;

	// 현재 활성 DamageId 또는 fallback DamageProfileId입니다.
	const FName ActiveDamageId = VehiclePawn->VehicleWeaponComp ? VehiclePawn->VehicleWeaponComp->GetActiveDamageId() : NAME_None;

	// 현재 활성 WeaponData입니다.
	UCFWeaponData* ActiveWeaponData = VehiclePawn->VehicleWeaponComp ? VehiclePawn->VehicleWeaponComp->GetActiveWeaponData() : nullptr;

	// 현재 활성 ProjectileData입니다.
	UCFProjectileData* ActiveProjectileData = VehiclePawn->VehicleWeaponComp ? VehiclePawn->VehicleWeaponComp->GetActiveProjectileData() : nullptr;

	// 기록할 Damage HitContext입니다.
	FCFDamageHitContext DamageHitContext;
	DamageHitContext.DamageData = ActiveDamageData;
	DamageHitContext.DamageId = ActiveDamageData ? ActiveDamageData->DamageId : ActiveDamageId;
	DamageHitContext.WeaponId = ActiveWeaponData ? ActiveWeaponData->WeaponId : FireCommand.WeaponGroupId;
	DamageHitContext.ProjectileId = ActiveProjectileData ? ActiveProjectileData->ProjectileId : (ActiveWeaponData ? ActiveWeaponData->ProjectileDataId : NAME_None);
	DamageHitContext.HitActor = (bBlockingHit && HitResult) ? HitResult->GetActor() : nullptr;
	DamageHitContext.HitComponentName = (bBlockingHit && HitResult && HitResult->GetComponent()) ? HitResult->GetComponent()->GetFName() : NAME_None;
	DamageHitContext.ImpactLocation = FireResult.LocalHitLocation;
	DamageHitContext.ImpactNormal = FireResult.LocalHitNormal.GetSafeNormal();
	if (DamageHitContext.ImpactNormal.IsNearlyZero())
	{
		DamageHitContext.ImpactNormal = FVector::UpVector;
	}
	DamageHitContext.IncomingDirection = FVector(FireCommand.AimDirection).GetSafeNormal();
	if (DamageHitContext.IncomingDirection.IsNearlyZero())
	{
		DamageHitContext.IncomingDirection = VehiclePawn->GetActorForwardVector().GetSafeNormal();
	}
	if (DamageHitContext.IncomingDirection.IsNearlyZero())
	{
		DamageHitContext.IncomingDirection = FVector::ForwardVector;
	}
	DamageHitContext.InstigatorActor = VehiclePawn;
	DamageHitContext.FlightDurationSeconds = 0.0f;
	DamageHitContext.bFromProjectileActor = false;
	DamageHitContext.bBlockingHit = bBlockingHit;

	// HitScan DamageHitContext를 정식 방어층에 적용한 전체 결과입니다.
	FCFVehicleDamageResult VehicleDamageResult;

	// 기존 VehicleHealth Debug와 Pool 호환에 사용할 차량 내구도 적용 결과입니다.
	FCFDamageApplyResult DamageApplyResult;

	// 다른 차량 Projectile 적중을 차량 피해가 아닌 Projectile 요격 경로로 처리했는지 여부입니다.
	bool bProjectileIntercepted = false;
	if (bBlockingHit && HitResult)
	{
		if (ACFProjectileActor* HitProjectileActor = Cast<ACFProjectileActor>(HitResult->GetActor()))
		{
			bProjectileIntercepted = HitProjectileActor->TryResolveProjectileInterception(VehiclePawn, VehiclePawn, *HitResult);
		}
	}

	if (VehiclePawn->CombatFxComp && ActiveProjectileData)
	{
		VehiclePawn->CombatFxComp->PlayImpactFx(ActiveProjectileData->DefaultImpactFxData, DamageHitContext);
	}

	if (!bProjectileIntercepted)
	{
		UCFVehicleDefenseComp::TryApplyDamageToActor(DamageHitContext, VehicleDamageResult);
		DamageApplyResult = UCFVehicleDefenseComp::BuildIntegrityCompatibilityResult(VehicleDamageResult);
	}

	VehiclePawn->StoreLastDamageHitContext(DamageHitContext);
	VehiclePawn->StoreLastDamageApplyResult(DamageApplyResult);
}

// Projectile Pool에서 반환된 Hit 발사체의 Damage 결과를 기존 Pawn-owned Damage Debug state로 기록합니다.
void UCFVehicleFireComp::RecordProjectileDamageHitContextFromPool(const ACFProjectileActor* InProjectileActor)
{
	// Damage/CombatFx observable Authority를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn || !InProjectileActor || !InProjectileActor->HasLastDamageHitContext())
	{
		return;
	}

	// Projectile Actor의 첫 유효 Impact에서 이미 생성된 Damage HitContext 복사본입니다.
	FCFDamageHitContext DamageHitContext = InProjectileActor->GetLastDamageHitContext();

	// Projectile의 실제 비활성화 사유입니다.
	const ECFProjectileDeactivateReason ProjectileDeactivateReason = InProjectileActor->GetLastDeactivateReason();

	// 일반 World/차량 Hit에서만 누락된 WeaponId를 현재 활성 WeaponData로 보완합니다.
	UCFWeaponData* ActiveWeaponData = VehiclePawn->VehicleWeaponComp ? VehiclePawn->VehicleWeaponComp->GetActiveWeaponData() : nullptr;
	if (ProjectileDeactivateReason == ECFProjectileDeactivateReason::Hit && DamageHitContext.WeaponId.IsNone() && ActiveWeaponData)
	{
		DamageHitContext.WeaponId = ActiveWeaponData->WeaponId;
	}

	// Projectile Actor가 첫 Impact에서 이미 실행한 직접 피해 적용 결과 복사본입니다.
	FCFDamageApplyResult DamageApplyResult = InProjectileActor->GetLastDamageApplyResult();

	// Impact FX 정책을 해석할 실제 비활성화 ProjectileData입니다.
	UCFProjectileData* ImpactProjectileData = InProjectileActor->GetLastDeactivatedProjectileData();

	// 일반 Hit 또는 detonate-on-intercept일 때 Impact FX를 표시할지 여부입니다.
	const bool bShouldPlayProjectileImpactFx = ImpactProjectileData
		&& (ProjectileDeactivateReason == ECFProjectileDeactivateReason::Hit
			|| (ProjectileDeactivateReason == ECFProjectileDeactivateReason::Intercepted && ImpactProjectileData->bDetonateWhenIntercepted));
	if (VehiclePawn->CombatFxComp && bShouldPlayProjectileImpactFx)
	{
		VehiclePawn->CombatFxComp->PlayImpactFx(ImpactProjectileData->DefaultImpactFxData, DamageHitContext);
	}

	VehiclePawn->StoreLastDamageHitContext(DamageHitContext);
	VehiclePawn->StoreLastDamageApplyResult(DamageApplyResult);
}

// 현재 터렛 Pitch 메쉬의 유효 Muzzle 소켓으로 최종 FireOrigin을 보정합니다.
bool UCFVehicleFireComp::TryBuildMuzzleFireOrigin(FCFVehicleFireOrigin& InOutFireOrigin, FString& OutFireOriginSummary) const
{
	// Muzzle/Turret/Weapon runtime을 소유한 차량 Pawn입니다.
	const ACFVehiclePawn* VehiclePawn = ResolveVehiclePawnConst();
	OutFireOriginSummary = FString();
	if (!VehiclePawn || !VehiclePawn->LastTurretMountData || !VehiclePawn->TurretPitchMeshComp || !VehiclePawn->TurretPitchMeshComp->GetStaticMesh())
	{
		return false;
	}

	// 신규 가변 Muzzle 배열입니다. 비어 있으면 기존 단일 MuzzleSocketName을 사용합니다.
	const TArray<FName>& ConfiguredMuzzleSocketNames = VehiclePawn->LastTurretMountData->MuzzleSocketNames;

	// Legacy 단일 Muzzle fallback을 사용해야 하는지 여부입니다.
	const bool bUsingLegacySingleMuzzle = ConfiguredMuzzleSocketNames.IsEmpty();

	// 이번 Mount에서 사용할 전체 Muzzle 소켓 수입니다.
	const int32 MuzzleSocketCount = bUsingLegacySingleMuzzle ? 1 : ConfiguredMuzzleSocketNames.Num();
	if (MuzzleSocketCount <= 0)
	{
		return false;
	}

	if (!bUsingLegacySingleMuzzle && VehiclePawn->LastTurretMountData->bRequireAllMuzzles)
	{
		for (const FName RequiredMuzzleSocketName : ConfiguredMuzzleSocketNames)
		{
			if (RequiredMuzzleSocketName.IsNone() || !VehiclePawn->TurretPitchMeshComp->DoesSocketExist(RequiredMuzzleSocketName))
			{
				return false;
			}
		}
	}

	// 다음 승인 발사 전까지 유지되는 SingleCycle 검색 시작 인덱스입니다.
	const int32 RequestedStartIndex = VehiclePawn->VehicleWeaponComp ? VehiclePawn->VehicleWeaponComp->GetNextMuzzleSocketIndex() : 0;

	// Muzzle 배열 범위로 제한한 검색 시작 인덱스입니다.
	const int32 SafeStartIndex = FMath::Clamp(RequestedStartIndex, 0, MuzzleSocketCount - 1);

	// 실제 Pitch 메쉬에 존재해 이번 FireOrigin에 사용할 Muzzle 소켓입니다.
	FName SelectedMuzzleSocketName = NAME_None;

	// 선택된 Muzzle 소켓의 배열 인덱스입니다.
	int32 SelectedMuzzleSocketIndex = INDEX_NONE;
	for (int32 SearchOffset = 0; SearchOffset < MuzzleSocketCount; ++SearchOffset)
	{
		// 현재 검색할 Muzzle 배열 인덱스입니다.
		const int32 CandidateMuzzleSocketIndex = (SafeStartIndex + SearchOffset) % MuzzleSocketCount;

		// 현재 검색할 실제 Muzzle 소켓 이름입니다.
		const FName CandidateMuzzleSocketName = bUsingLegacySingleMuzzle
			? VehiclePawn->LastTurretMountData->MuzzleSocketName
			: ConfiguredMuzzleSocketNames[CandidateMuzzleSocketIndex];
		if (!CandidateMuzzleSocketName.IsNone() && VehiclePawn->TurretPitchMeshComp->DoesSocketExist(CandidateMuzzleSocketName))
		{
			SelectedMuzzleSocketName = CandidateMuzzleSocketName;
			SelectedMuzzleSocketIndex = CandidateMuzzleSocketIndex;
			break;
		}
	}

	if (SelectedMuzzleSocketName.IsNone() || SelectedMuzzleSocketIndex == INDEX_NONE)
	{
		return false;
	}

	// 선택된 Pitch 메쉬 Muzzle 소켓의 월드 Transform입니다.
	const FTransform MuzzleSocketWorldTransform = VehiclePawn->TurretPitchMeshComp->GetSocketTransform(SelectedMuzzleSocketName, RTS_World);

	// 최종 FireOrigin에 사용할 Muzzle 소켓 월드 위치입니다.
	const FVector MuzzleSocketWorldLocation = MuzzleSocketWorldTransform.GetLocation();
	if (MuzzleSocketWorldLocation.ContainsNaN())
	{
		return false;
	}

	// 최종 발사 방향을 어느 기준에서 얻었는지 표시할 디버그 문자열입니다.
	FString DirectionSourceText = TEXT("MuzzleSocketX");

	// 선택된 Muzzle 소켓의 X축을 기준으로 계산한 월드 발사 방향입니다.
	FVector MuzzleSocketForwardDirection = MuzzleSocketWorldTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
	if (MuzzleSocketForwardDirection.ContainsNaN() || MuzzleSocketForwardDirection.IsNearlyZero())
	{
		MuzzleSocketForwardDirection = VehiclePawn->ResolveTurretAimWorldDirection().GetSafeNormal();
		DirectionSourceText = TEXT("TurretAimFallback");
	}
	if (MuzzleSocketForwardDirection.ContainsNaN() || MuzzleSocketForwardDirection.IsNearlyZero())
	{
		MuzzleSocketForwardDirection = InOutFireOrigin.WorldFireDirection.GetSafeNormal();
		DirectionSourceText = TEXT("PreviousFireOriginFallback");
	}
	if (MuzzleSocketForwardDirection.ContainsNaN() || MuzzleSocketForwardDirection.IsNearlyZero())
	{
		return false;
	}

	InOutFireOrigin.bResolved = true;
	InOutFireOrigin.WorldFireLocation = MuzzleSocketWorldLocation;
	InOutFireOrigin.WorldFireDirection = MuzzleSocketForwardDirection;
	InOutFireOrigin.MuzzleSocketName = SelectedMuzzleSocketName;
	InOutFireOrigin.MuzzleSocketIndex = SelectedMuzzleSocketIndex;
	InOutFireOrigin.MuzzleSocketCount = MuzzleSocketCount;

	if (VehiclePawn->VehicleWeaponComp)
	{
		VehiclePawn->VehicleWeaponComp->RecordResolvedMuzzleSelection(SelectedMuzzleSocketName, SelectedMuzzleSocketIndex, MuzzleSocketCount);
	}

	OutFireOriginSummary = FString::Printf(
		TEXT("VehicleWeaponFireOrigin: Resolved, Source=MuzzleSocket, Mode=%s, Profile=%s, Slot=%s, Muzzle=%s, MuzzleIndex=%d, MuzzleCount=%d, PitchMesh=%s, DirectionSource=%s, Location=(%.1f, %.1f, %.1f), Direction=(%.3f, %.3f, %.3f)"),
		bUsingLegacySingleMuzzle ? TEXT("LegacySingle") : TEXT("ConfiguredArray"),
		*InOutFireOrigin.MountProfileId.ToString(),
		*InOutFireOrigin.LocationSlotId.ToString(),
		*SelectedMuzzleSocketName.ToString(),
		SelectedMuzzleSocketIndex,
		MuzzleSocketCount,
		*VehiclePawn->TurretPitchMeshComp->GetStaticMesh()->GetName(),
		*DirectionSourceText,
		InOutFireOrigin.WorldFireLocation.X,
		InOutFireOrigin.WorldFireLocation.Y,
		InOutFireOrigin.WorldFireLocation.Z,
		InOutFireOrigin.WorldFireDirection.X,
		InOutFireOrigin.WorldFireDirection.Y,
		InOutFireOrigin.WorldFireDirection.Z);
	return true;
}

// Muzzle 위치와 현재/고정 Command Target을 사용해 공통 Weapon Aim Solution을 계산합니다.
bool UCFVehicleFireComp::BuildWeaponAimSolution(FCFVehicleWeaponAimSolution& OutWeaponAimSolution, FCFVehicleFireOrigin* OutFireOrigin, FString* OutFireOriginSummary, const FVector* OverrideAimTargetLocation) const
{
	// Aim/Weapon/Turret runtime을 소유한 차량 Pawn입니다.
	const ACFVehiclePawn* VehiclePawn = ResolveVehiclePawnConst();
	OutWeaponAimSolution = FCFVehicleWeaponAimSolution();
	if (OutFireOrigin)
	{
		*OutFireOrigin = FCFVehicleFireOrigin();
	}
	if (OutFireOriginSummary)
	{
		*OutFireOriginSummary = FString();
	}
	if (!VehiclePawn || !VehiclePawn->VehicleAimComp || !VehiclePawn->VehicleWeaponComp)
	{
		return false;
	}

	// AimComp가 보유한 현재 로컬 조준 상태입니다.
	const FCFVehicleLocalAimState LocalAimState = VehiclePawn->VehicleAimComp->GetLocalAimState();

	// Camera Runtime에서 읽은 현재 Aim Trace 상태입니다.
	const FCFVehicleCameraRuntimeState CameraRuntimeState = VehiclePawn->VehicleCameraComp
		? VehiclePawn->VehicleCameraComp->GetCameraRuntimeState()
		: FCFVehicleCameraRuntimeState();

	// 고정 Volley 목표에서는 현재 Camera Trace를 다른 목표의 표면 일치 근거로 사용하지 않습니다.
	const bool bAimTraceHasBlockingHit = OverrideAimTargetLocation
		? false
		: (VehiclePawn->VehicleCameraComp ? CameraRuntimeState.bAimTraceHasBlockingHit : LocalAimState.bLocalAimTraceHasBlockingHit);
	OutWeaponAimSolution.bAimTraceHasBlockingHit = bAimTraceHasBlockingHit;

	// 일반 조준에서만 Camera Aim Trace가 선택한 Actor를 Command 목표 표면으로 사용합니다.
	AActor* AimTraceHitActor = OverrideAimTargetLocation
		? nullptr
		: (VehiclePawn->VehicleCameraComp ? CameraRuntimeState.AimTraceHitActor.Get() : nullptr);

	// 일반 발사는 현재 Reticle 목표를, 후속 Ripple/Salvo는 첫 입력 순간 고정 목표를 사용합니다.
	const FVector AimTargetLocation = OverrideAimTargetLocation ? *OverrideAimTargetLocation : LocalAimState.LocalAimTargetLocation;
	if (AimTargetLocation.ContainsNaN() || AimTargetLocation.IsNearlyZero())
	{
		return false;
	}

	// Muzzle 계산 전 VehicleWeaponComp가 만든 기본 FireOrigin입니다.
	FCFVehicleFireOrigin ResolvedFireOrigin;
	if (!VehiclePawn->VehicleWeaponComp->BuildFireOrigin(LocalAimState.LocalAimDirection, ResolvedFireOrigin))
	{
		return false;
	}

	// Muzzle 소켓 위치를 반영할 최종 FireOrigin입니다.
	FCFVehicleFireOrigin FinalFireOrigin = ResolvedFireOrigin;

	// Muzzle FireOrigin 계산 결과 요약입니다.
	FString MuzzleFireOriginSummary;
	if (!TryBuildMuzzleFireOrigin(FinalFireOrigin, MuzzleFireOriginSummary))
	{
		return false;
	}

	// Muzzle에서 Command 목표점까지의 원본 방향 벡터입니다.
	const FVector RawDesiredLaunchDirection = AimTargetLocation - FinalFireOrigin.WorldFireLocation;
	if (RawDesiredLaunchDirection.ContainsNaN() || RawDesiredLaunchDirection.IsNearlyZero())
	{
		return false;
	}

	// Muzzle에서 Command 목표점으로 향하는 요구 발사 방향입니다.
	const FVector DesiredAimDirection = RawDesiredLaunchDirection.GetSafeNormal();

	// Muzzle 소켓 X축 기준 현재 포신 방향입니다.
	const FVector CurrentMuzzleDirection = FinalFireOrigin.WorldFireDirection.GetSafeNormal();
	if (CurrentMuzzleDirection.ContainsNaN() || CurrentMuzzleDirection.IsNearlyZero())
	{
		return false;
	}

	// 총구 X축과 요구 발사 방향의 내적입니다.
	const float MuzzleAlignmentDot = FMath::Clamp(FVector::DotProduct(CurrentMuzzleDirection, DesiredAimDirection), -1.0f, 1.0f);

	// 총구 X축과 요구 발사 방향 사이의 각도 오차입니다.
	const float WeaponAlignmentErrorDeg = FMath::RadiansToDegrees(FMath::Acos(MuzzleAlignmentDot));

	// TurretMountData에서 재사용할 안정화 허용 오차입니다.
	const float StabilizationToleranceDeg = VehiclePawn->LastTurretMountData
		? FMath::Max(0.0f, VehiclePawn->LastTurretMountData->StabilizationToleranceDeg)
		: 0.0f;

	// 현재 터렛 추적 상태입니다.
	const FCFVehicleTurretState CurrentTurretState = VehiclePawn->VehicleWeaponComp->GetTurretState();

	// 터렛이 안정화 대기 중인지 여부입니다.
	const bool bTurretAligning = VehiclePawn->LastTurretMountData && !CurrentTurretState.bTurretSettled;

	// 총구 방향이 요구 발사 방향 허용 오차 밖인지 여부입니다.
	const bool bWeaponNotAligned = WeaponAlignmentErrorDeg > StabilizationToleranceDeg;

	// 활성 TurretMountData가 정렬 중 발사를 허용하는지 여부입니다.
	const bool bAllowFireWhileAligning = VehiclePawn->LastTurretMountData && VehiclePawn->LastTurretMountData->bAllowFireWhileAligning;

	// 터렛 또는 총구가 아직 요구 방향에 정렬 중인지 여부입니다.
	const bool bWeaponIsAligning = bTurretAligning || bWeaponNotAligned;

	// BuildFireCommand/HitScan/Projectile이 공통으로 사용할 실제 최종 발사 방향입니다.
	const FVector FinalFireDirection = (bWeaponIsAligning && bAllowFireWhileAligning) ? CurrentMuzzleDirection : DesiredAimDirection;

	// 터렛 Reticle과 Weapon Preview가 사용할 명령 목표까지의 거리입니다.
	const float CommandPathDistance = RawDesiredLaunchDirection.Size();

	// TurretMountData가 없을 때 사용할 총구 안전 검사 기본 거리입니다.
	const float DefaultMuzzleClearanceDistanceCm = 150.0f;

	// 총구 바로 앞에서 발사체가 안전하게 빠져나갈 수 있는지 검사할 최대 거리입니다.
	const float MuzzleClearanceDistanceCm = VehiclePawn->LastTurretMountData
		? FMath::Max(0.0f, VehiclePawn->LastTurretMountData->MuzzleClearanceDistanceCm)
		: DefaultMuzzleClearanceDistanceCm;

	// Command 목표가 총구 안전 거리보다 가까울 때 목표를 넘겨 검사하지 않도록 제한한 거리입니다.
	const float MuzzleBlockTraceDistance = FMath::Min(CommandPathDistance, MuzzleClearanceDistanceCm);

	// 사용자 조준점과 같은 깊이에서 터렛 방향을 비교할 수 있는 거리인지 여부입니다.
	const bool bHasValidTurretReticleDistance = FMath::IsFinite(CommandPathDistance) && CommandPathDistance > UE_KINDA_SMALL_NUMBER;

	// CurrentMuzzleDirection을 사용자 조준점과 같은 거리까지 연장한 터렛 조준 월드 지점입니다.
	const FVector TurretReticleWorldLocation = bHasValidTurretReticleDistance
		? FinalFireOrigin.WorldFireLocation + CurrentMuzzleDirection * CommandPathDistance
		: FVector::ZeroVector;

	// 터렛 레티클 월드 지점을 UI에 제공할 수 있는지 여부입니다.
	const bool bHasValidTurretReticlePoint = bHasValidTurretReticleDistance && !TurretReticleWorldLocation.ContainsNaN();

	// Preview 거리 fallback에 사용할 기본 AimProfile 최대 거리입니다.
	const float FallbackPreviewDistance = FMath::Max(0.0f, VehiclePawn->VehicleAimComp->GetDefaultAimProfile().MaxAimDistance);

	// 현재 활성 WeaponData입니다.
	UCFWeaponData* ActiveWeaponData = VehiclePawn->VehicleWeaponComp->GetActiveWeaponData();

	// 현재 활성 ProjectileData입니다.
	UCFProjectileData* ActiveProjectileData = VehiclePawn->VehicleWeaponComp->GetActiveProjectileData();

	// 활성 WeaponData가 현재 MountProfile에서 호환되는지 여부입니다.
	const bool bActiveWeaponCompatible = ActiveWeaponData && VehiclePawn->VehicleWeaponComp->IsActiveWeaponDataCompatible();

	// 현재 활성 무기가 제공할 Weapon Reticle 월드 데이터의 의미입니다.
	ECFWeaponReticleMode WeaponReticleMode = ECFWeaponReticleMode::Hidden;
	if (bActiveWeaponCompatible)
	{
		if (ActiveWeaponData->FireMode == ECFWeaponFireMode::HitScan)
		{
			WeaponReticleMode = ECFWeaponReticleMode::DirectImpact;
		}
		else if (ActiveWeaponData->FireMode == ECFWeaponFireMode::Projectile && ActiveProjectileData)
		{
			WeaponReticleMode = ActiveProjectileData->bAffectedByGravity ? ECFWeaponReticleMode::LaunchDirection : ECFWeaponReticleMode::DirectImpact;
		}
	}

	// 활성 무기 또는 AimProfile fallback에서 얻은 Weapon Reticle Preview 최대 거리입니다.
	const float PreviewMaxRange = WeaponReticleMode != ECFWeaponReticleMode::Hidden
		? VehiclePawn->VehicleWeaponComp->GetActiveWeaponMaxRange(FallbackPreviewDistance)
		: 0.0f;

	// Preview 계산에 사용할 최대 거리가 안전한지 여부입니다.
	const bool bHasValidPreviewRange = FMath::IsFinite(PreviewMaxRange) && PreviewMaxRange > 0.0f;

	// DirectImpact만 최대 사거리 Preview를 위해 Trace를 확장합니다.
	const bool bShouldTracePreviewMaxRange = WeaponReticleMode == ECFWeaponReticleMode::DirectImpact && bHasValidPreviewRange;

	// MuzzleBlocked와 DirectImpact Preview가 공유할 WeaponHit Trace 거리입니다.
	const float SharedTraceDistance = bShouldTracePreviewMaxRange ? FMath::Max(CommandPathDistance, PreviewMaxRange) : CommandPathDistance;

	// 실제 최종 발사 방향을 따라 계산한 공유 WeaponHit Trace 종료 위치입니다.
	const FVector SharedTraceEnd = FinalFireOrigin.WorldFireLocation + FinalFireDirection * SharedTraceDistance;

	// MuzzleBlocked와 Preview가 함께 해석할 공유 WeaponHit Trace 결과입니다.
	FHitResult SharedWeaponHitResult;

	// Muzzle 앞 장애물 Trace에서 현재 Pawn을 무시하기 위한 쿼리 설정입니다.
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CarFightMuzzleBlockTrace), false);
	QueryParams.AddIgnoredActor(VehiclePawn);

	// Muzzle 앞 장애물 Trace에 사용할 월드입니다.
	UWorld* World = VehiclePawn->GetWorld();

	// 공유 WeaponHit Trace가 Blocking Hit을 얻었는지 여부입니다.
	bool bHasSharedWeaponHit = false;

	// 실제 최종 발사 경로에서 안전 거리보다 앞에 WeaponHit Blocking Hit이 발생했는지 여부입니다.
	bool bMuzzleBlocked = false;
	if (World && FMath::IsFinite(SharedTraceDistance) && SharedTraceDistance > 0.0f && !SharedTraceEnd.ContainsNaN())
	{
		bHasSharedWeaponHit = World->LineTraceSingleByChannel(SharedWeaponHitResult, FinalFireOrigin.WorldFireLocation, SharedTraceEnd, CFCollisionChannels::WeaponHit, QueryParams);

		// 총구 Trace가 Camera Aim Trace와 같은 목표 Actor를 적중했는지 여부입니다.
		const bool bSharedWeaponHitMatchesAimTarget = bAimTraceHasBlockingHit && IsValid(AimTraceHitActor) && SharedWeaponHitResult.GetActor() == AimTraceHitActor;

		// 첫 적중 Actor입니다.
		AActor* SharedWeaponHitActor = SharedWeaponHitResult.GetActor();

		// 첫 적중 Actor가 실제 발사로 피해를 받을 수 있는 차량 표적인지 여부입니다.
		const bool bSharedWeaponHitIsDamageableVehicle = IsValid(SharedWeaponHitActor) && IsValid(SharedWeaponHitActor->FindComponentByClass<UCFVehicleHealthComp>());

		// 첫 적중이 총구 안전 검사 거리 안에 있는지 여부입니다.
		const bool bSharedWeaponHitWithinMuzzleClearance = FMath::IsFinite(SharedWeaponHitResult.Distance) && SharedWeaponHitResult.Distance < MuzzleBlockTraceDistance;

		bMuzzleBlocked = bHasSharedWeaponHit && !bSharedWeaponHitMatchesAimTarget && !bSharedWeaponHitIsDamageableVehicle && bSharedWeaponHitWithinMuzzleClearance;
	}

	if (!bHasValidPreviewRange)
	{
		WeaponReticleMode = ECFWeaponReticleMode::Hidden;
	}

	// Weapon Reticle Preview가 유효한지 여부입니다.
	bool bHasValidWeaponPreview = WeaponReticleMode != ECFWeaponReticleMode::Hidden && bHasValidPreviewRange;

	// Preview 결과가 최대 거리 안의 Blocking Hit인지 여부입니다.
	bool bWeaponPreviewHasBlockingHit = false;

	// Preview가 표시할 월드 위치입니다.
	FVector WeaponPreviewWorldLocation = FVector::ZeroVector;

	// Preview 시작점에서 결과 위치까지의 거리입니다.
	float WeaponPreviewDistance = 0.0f;
	if (bHasValidWeaponPreview)
	{
		if (WeaponReticleMode == ECFWeaponReticleMode::DirectImpact && bHasSharedWeaponHit && FMath::IsFinite(SharedWeaponHitResult.Distance) && SharedWeaponHitResult.Distance <= PreviewMaxRange)
		{
			bWeaponPreviewHasBlockingHit = true;
			WeaponPreviewWorldLocation = SharedWeaponHitResult.ImpactPoint;
			WeaponPreviewDistance = FMath::Max(0.0f, SharedWeaponHitResult.Distance);
		}
		else if (WeaponReticleMode == ECFWeaponReticleMode::LaunchDirection)
		{
			WeaponPreviewWorldLocation = FinalFireOrigin.WorldFireLocation + FinalFireDirection * CommandPathDistance;
			WeaponPreviewDistance = CommandPathDistance;
			bWeaponPreviewHasBlockingHit = false;
		}
		else
		{
			WeaponPreviewWorldLocation = FinalFireOrigin.WorldFireLocation + FinalFireDirection * PreviewMaxRange;
			WeaponPreviewDistance = PreviewMaxRange;
		}

		if (WeaponPreviewWorldLocation.ContainsNaN() || !FMath::IsFinite(WeaponPreviewDistance) || WeaponPreviewDistance <= 0.0f)
		{
			WeaponReticleMode = ECFWeaponReticleMode::Hidden;
			bHasValidWeaponPreview = false;
			bWeaponPreviewHasBlockingHit = false;
			WeaponPreviewWorldLocation = FVector::ZeroVector;
			WeaponPreviewDistance = 0.0f;
		}
	}

	FinalFireOrigin.WorldFireDirection = FinalFireDirection;
	OutWeaponAimSolution.bHasValidSolution = true;
	OutWeaponAimSolution.bTurretAligning = bTurretAligning;
	OutWeaponAimSolution.bWeaponNotAligned = bWeaponNotAligned;
	OutWeaponAimSolution.bAllowFireWhileAligning = bAllowFireWhileAligning;
	OutWeaponAimSolution.bMuzzleBlocked = bMuzzleBlocked;
	OutWeaponAimSolution.bHasValidTurretReticlePoint = bHasValidTurretReticlePoint;
	OutWeaponAimSolution.TurretReticleWorldLocation = bHasValidTurretReticlePoint ? TurretReticleWorldLocation : FVector::ZeroVector;
	OutWeaponAimSolution.TurretReticleDistance = bHasValidTurretReticlePoint ? CommandPathDistance : 0.0f;
	OutWeaponAimSolution.WeaponReticleMode = WeaponReticleMode;
	OutWeaponAimSolution.bHasValidWeaponPreview = bHasValidWeaponPreview;
	OutWeaponAimSolution.bWeaponPreviewHasBlockingHit = bWeaponPreviewHasBlockingHit;
	OutWeaponAimSolution.WeaponPreviewWorldLocation = WeaponPreviewWorldLocation;
	OutWeaponAimSolution.WeaponPreviewDistance = WeaponPreviewDistance;
	OutWeaponAimSolution.AimOrigin = FinalFireOrigin.WorldFireLocation;
	OutWeaponAimSolution.AimDirection = FinalFireDirection;
	OutWeaponAimSolution.DesiredAimDirection = DesiredAimDirection;
	OutWeaponAimSolution.CurrentMuzzleDirection = CurrentMuzzleDirection;
	OutWeaponAimSolution.AimTargetLocation = AimTargetLocation;
	OutWeaponAimSolution.WeaponAlignmentErrorDeg = WeaponAlignmentErrorDeg;

	if (OutFireOrigin)
	{
		*OutFireOrigin = FinalFireOrigin;
	}
	if (OutFireOriginSummary)
	{
		// FireOrigin 요약에 기록할 Weapon Reticle 모드 문자열입니다.
		const TCHAR* WeaponReticleModeText = TEXT("Hidden");
		switch (WeaponReticleMode)
		{
		case ECFWeaponReticleMode::DirectImpact:
			WeaponReticleModeText = TEXT("DirectImpact");
			break;
		case ECFWeaponReticleMode::LaunchDirection:
			WeaponReticleModeText = TEXT("LaunchDirection");
			break;
		case ECFWeaponReticleMode::Hidden:
		default:
			break;
		}

		*OutFireOriginSummary = FString::Printf(
			TEXT("%s, AllowFireWhileAligning=%s, DesiredAimDirection=(%.3f, %.3f, %.3f), CurrentMuzzleDirection=(%.3f, %.3f, %.3f), FinalAimDirection=(%.3f, %.3f, %.3f), Target=(%.1f, %.1f, %.1f), AlignmentErrorDeg=%.2f, TurretAligning=%s, WeaponNotAligned=%s, MuzzleBlocked=%s, WeaponReticleMode=%s, WeaponPreviewValid=%s, WeaponPreviewHit=%s, WeaponPreviewLocation=(%.1f, %.1f, %.1f), WeaponPreviewDistance=%.1f"),
			*MuzzleFireOriginSummary,
			bAllowFireWhileAligning ? TEXT("True") : TEXT("False"),
			DesiredAimDirection.X, DesiredAimDirection.Y, DesiredAimDirection.Z,
			CurrentMuzzleDirection.X, CurrentMuzzleDirection.Y, CurrentMuzzleDirection.Z,
			FinalFireDirection.X, FinalFireDirection.Y, FinalFireDirection.Z,
			AimTargetLocation.X, AimTargetLocation.Y, AimTargetLocation.Z,
			WeaponAlignmentErrorDeg,
			bTurretAligning ? TEXT("Yes") : TEXT("No"),
			bWeaponNotAligned ? TEXT("Yes") : TEXT("No"),
			bMuzzleBlocked ? TEXT("Yes") : TEXT("No"),
			WeaponReticleModeText,
			bHasValidWeaponPreview ? TEXT("Yes") : TEXT("No"),
			bWeaponPreviewHasBlockingHit ? TEXT("Yes") : TEXT("No"),
			WeaponPreviewWorldLocation.X, WeaponPreviewWorldLocation.Y, WeaponPreviewWorldLocation.Z,
			WeaponPreviewDistance);
	}
	return true;
}

// 현재 Weapon Aim Solution을 다시 계산해 AimComp에 저장합니다.
void UCFVehicleFireComp::RefreshWeaponAimSolution()
{
	// Aim state source/owner를 제공하는 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn || !VehiclePawn->VehicleAimComp)
	{
		return;
	}

	// AimComp에 저장할 최신 Weapon Aim Solution입니다.
	FCFVehicleWeaponAimSolution WeaponAimSolution;
	BuildWeaponAimSolution(WeaponAimSolution);
	VehiclePawn->VehicleAimComp->SetWeaponAimSolution(WeaponAimSolution);
}

// 현재 활성 무기가 Projectile Actor 실행 경로를 사용할 수 있는지 반환합니다.
bool UCFVehicleFireComp::ShouldUseProjectileActorFire() const
{
	// 현재 Weapon runtime을 소유한 차량 Pawn입니다.
	const ACFVehiclePawn* VehiclePawn = ResolveVehiclePawnConst();
	return VehiclePawn && VehiclePawn->VehicleWeaponComp && VehiclePawn->VehicleWeaponComp->IsActiveProjectileSpawnReady();
}

// 현재 Release 설정과 고정 Guidance Actor Snapshot으로 Projectile Launch Context를 생성합니다.
bool UCFVehicleFireComp::BuildDirectProjectileLaunchContext(const FCFVehicleFireRequest& FireCommand, const UCFProjectileData& InProjectileData, AActor* GuidanceTargetActorSnapshot, FCFProjectileLaunchContext& OutLaunchContext) const
{
	// Projectile launch source를 소유한 차량 Pawn입니다.
	const ACFVehiclePawn* VehiclePawn = ResolveVehiclePawnConst();
	OutLaunchContext = FCFProjectileLaunchContext();
	if (!VehiclePawn)
	{
		return false;
	}

	// Direct 호환 방향과 Muzzle Transform fallback에 사용할 정규화 조준 방향입니다.
	const FVector DirectAimDirection = FVector(FireCommand.AimDirection).GetSafeNormal();
	if (DirectAimDirection.ContainsNaN() || DirectAimDirection.IsNearlyZero())
	{
		return false;
	}

	// 실제 선택 Muzzle의 월드 위치입니다.
	const FVector LaunchLocation = FVector(FireCommand.AimOrigin);
	if (LaunchLocation.ContainsNaN())
	{
		return false;
	}

	// 실제 Muzzle 소켓 Transform을 우선하고 없으면 Direct Aim 기준 Transform을 사용합니다.
	FTransform MuzzleWorldTransform(DirectAimDirection.Rotation(), LaunchLocation);
	if (VehiclePawn->TurretPitchMeshComp && !FireCommand.MuzzleSocketName.IsNone() && VehiclePawn->TurretPitchMeshComp->DoesSocketExist(FireCommand.MuzzleSocketName))
	{
		// 현재 FireRequest가 캡처한 Muzzle 소켓의 월드 Transform입니다.
		const FTransform ResolvedMuzzleTransform = VehiclePawn->TurretPitchMeshComp->GetSocketTransform(FireCommand.MuzzleSocketName, RTS_World);
		if (!ResolvedMuzzleTransform.ContainsNaN())
		{
			MuzzleWorldTransform = ResolvedMuzzleTransform;
			MuzzleWorldTransform.SetLocation(LaunchLocation);
		}
	}

	// 활성 WeaponData가 없거나 호환되지 않을 때 Direct 기본값을 유지할 Release 설정입니다.
	const FCFLauncherReleaseConfig ReleaseConfig = VehiclePawn->VehicleWeaponComp ? VehiclePawn->VehicleWeaponComp->GetActiveLauncherReleaseConfig() : FCFLauncherReleaseConfig();

	// Release Mode에 따라 계산한 초기 분리 방향입니다.
	const FVector InitialLaunchDirection = ReleaseConfig.ResolveInitialLaunchDirection(MuzzleWorldTransform, DirectAimDirection);
	if (InitialLaunchDirection.ContainsNaN() || InitialLaunchDirection.IsNearlyZero())
	{
		return false;
	}

	// 발사 순간 차량의 월드 Velocity 스냅샷입니다.
	FVector CarrierWorldVelocity = VehiclePawn->GetVelocity();
	if (CarrierWorldVelocity.ContainsNaN())
	{
		CarrierWorldVelocity = FVector::ZeroVector;
	}

	// Launch Context Debug와 후속 비행 계산에 별도로 보존할 차량 속도 상속 성분입니다.
	const FVector InheritedCarrierVelocity = ReleaseConfig.ResolveInheritedCarrierVelocity(CarrierWorldVelocity);

	// 사출 속도와 차량 속도 상속을 합친 실제 초기 월드 Velocity입니다.
	const FVector InitialLaunchVelocity = ReleaseConfig.ResolveInitialLaunchVelocity(InitialLaunchDirection, InProjectileData.InitialSpeed, CarrierWorldVelocity);
	if (InitialLaunchVelocity.ContainsNaN() || InitialLaunchVelocity.IsNearlyZero())
	{
		return false;
	}

	// 발사 명령 목표가 NaN일 때 초기 방향 앞쪽으로 복구한 안전한 명령 목표입니다.
	FVector SafeCommandTargetLocation = FVector(FireCommand.PredictedAimTargetLocation);
	if (SafeCommandTargetLocation.ContainsNaN())
	{
		SafeCommandTargetLocation = LaunchLocation + InitialLaunchDirection * FMath::Max(InProjectileData.InitialSpeed, 1.0f);
	}

	OutLaunchContext.LaunchTransform = FTransform(InitialLaunchDirection.Rotation(), LaunchLocation);
	OutLaunchContext.InitialLaunchDirection = InitialLaunchDirection;
	OutLaunchContext.InitialLaunchVelocity = InitialLaunchVelocity;
	OutLaunchContext.InheritedCarrierVelocity = InheritedCarrierVelocity;
	OutLaunchContext.CommandTargetLocation = SafeCommandTargetLocation;
	OutLaunchContext.GuidanceTargetActor = GuidanceTargetActorSnapshot;
	OutLaunchContext.MuzzleSocketName = FireCommand.MuzzleSocketName;
	OutLaunchContext.MuzzleSocketIndex = FireCommand.MuzzleSocketIndex;
	OutLaunchContext.MuzzleSocketCount = FireCommand.MuzzleSocketCount;
	OutLaunchContext.ReleaseMode = ReleaseConfig.ReleaseMode;
	OutLaunchContext.FireRequestId = FireCommand.FireRequestId;
	OutLaunchContext.WeaponGroupId = FireCommand.WeaponGroupId;
	return true;
}

// 현재 선택 목표 Snapshot을 사용하는 기존 단발 Projectile 호환 실행 경로를 수행합니다.
bool UCFVehicleFireComp::TrySpawnProjectileActorFromFireCommand(const FCFVehicleFireRequest& FireCommand)
{
	// Projectile/TargetSelect runtime을 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn || !VehiclePawn->VehicleWeaponComp || !VehiclePawn->ProjectilePoolComp)
	{
		return false;
	}

	// Projectile Actor 스폰에 사용할 활성 ProjectileData입니다.
	UCFProjectileData* ActiveProjectileData = VehiclePawn->VehicleWeaponComp->GetActiveProjectileData();
	if (!ActiveProjectileData || !ActiveProjectileData->ProjectileActorClass)
	{
		return false;
	}

	// 이 호환 단발 호출이 Launch Context에 전달할 현재 선택 목표 Actor Snapshot입니다.
	AActor* GuidanceTargetActorSnapshot = VehiclePawn->TargetSelectComp ? VehiclePawn->TargetSelectComp->GetSelectedTargetActor() : nullptr;

	// 현재 Release 설정과 목표 Actor Snapshot을 보존하는 Launch Context입니다.
	FCFProjectileLaunchContext LaunchContext;
	if (!BuildDirectProjectileLaunchContext(FireCommand, *ActiveProjectileData, GuidanceTargetActorSnapshot, LaunchContext))
	{
		return false;
	}

	// Pool에서 재사용하거나 새로 확보한 공통 Projectile Actor입니다.
	ACFProjectileActor* AcquiredProjectileActor = VehiclePawn->ProjectilePoolComp->AcquireProjectileWithContext(ActiveProjectileData, LaunchContext, VehiclePawn);
	return AcquiredProjectileActor != nullptr;
}

// 검증 승인된 명령을 Ammo Transaction과 함께 Projectile 또는 HitScan 경로로 실행합니다.
bool UCFVehicleFireComp::ExecuteAcceptedFireCommand(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& InOutFireResult, AActor* GuidanceTargetActorSnapshot, const bool bAllowProjectileFallback)
{
	// Fire/Ammo/Projectile runtime을 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn || !InOutFireResult.bAccepted)
	{
		return false;
	}

	// 이번 실행이 SingleCycle 유한탄 Transaction을 사용할지 여부입니다.
	const bool bUseSingleFireAmmoTransaction = ShouldUseSingleFireAmmoTransaction(VehiclePawn);

	// Ammo Runtime에서 이번 발사 무기를 식별할 현재 MountProfileId입니다.
	const FName AmmoWeaponInstanceId = ResolveActiveAmmoWeaponInstanceId(VehiclePawn);

	// 실제 실행 직전 탄약 예약이 성공해 Commit 또는 Rollback이 필요한지 여부입니다.
	bool bAmmoReservationActive = false;
	if (bUseSingleFireAmmoTransaction)
	{
		if (!VehiclePawn->VehicleAmmoComp)
		{
			InOutFireResult.bAccepted = false;
			InOutFireResult.RejectReason = ECFVehicleFireRejectReason::NoAmmo;
			return false;
		}

		// 검증 이후 실행 직전에 다시 원자적으로 확보한 SingleCycle 탄약 예약 결과입니다.
		const ECFAmmoTransactionResult AmmoReservationResult = VehiclePawn->VehicleAmmoComp->ReserveSingleFireAmmo(AmmoWeaponInstanceId);
		if (AmmoReservationResult != ECFAmmoTransactionResult::Accepted)
		{
			InOutFireResult.bAccepted = false;
			InOutFireResult.RejectReason = ResolveAmmoTransactionFireRejectReason(AmmoReservationResult);
			return false;
		}
		bAmmoReservationActive = true;
	}

	if (!ShouldUseProjectileActorFire())
	{
		RunLocalDummyHitScan(FireCommand, InOutFireResult);

		// HitScan은 Miss도 실제 한 발이므로 예약 탄약 소비를 확정한 결과입니다.
		const bool bAmmoCommitted = CommitSingleFireAmmoReservation(VehiclePawn, AmmoWeaponInstanceId, bAmmoReservationActive);
		if (!bAmmoCommitted)
		{
			InOutFireResult.bAccepted = false;
			InOutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidLocalState;
			return false;
		}
		return true;
	}

	// 현재 발사 실행에 사용할 활성 ProjectileData입니다.
	UCFProjectileData* ActiveProjectileData = VehiclePawn->VehicleWeaponComp ? VehiclePawn->VehicleWeaponComp->GetActiveProjectileData() : nullptr;
	if (!ActiveProjectileData || !ActiveProjectileData->ProjectileActorClass)
	{
		RollbackSingleFireAmmoReservation(VehiclePawn, AmmoWeaponInstanceId, bAmmoReservationActive);
		InOutFireResult.bAccepted = false;
		InOutFireResult.RejectReason = ECFVehicleFireRejectReason::NoWeapon;
		return false;
	}

	// Pool Acquire와 사출 안전 검사가 함께 사용할 위치/Actor 목표 Snapshot 포함 Launch Context입니다.
	FCFProjectileLaunchContext LaunchContext;
	if (!BuildDirectProjectileLaunchContext(FireCommand, *ActiveProjectileData, GuidanceTargetActorSnapshot, LaunchContext))
	{
		RollbackSingleFireAmmoReservation(VehiclePawn, AmmoWeaponInstanceId, bAmmoReservationActive);
		InOutFireResult.bAccepted = false;
		InOutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimDirection;
		return false;
	}

	// 현재 발사 실행에 사용할 Release 설정입니다.
	const FCFLauncherReleaseConfig ReleaseConfig = VehiclePawn->VehicleWeaponComp->GetActiveLauncherReleaseConfig();

	// 비Direct 실제 사출 방향의 안전 검사 거리입니다.
	const float ClearanceTraceDistanceCm = ReleaseConfig.GetEffectiveLauncherClearanceTraceDistanceCm();
	if (LaunchContext.ReleaseMode != ECFProjectileReleaseMode::Direct && ClearanceTraceDistanceCm > KINDA_SMALL_NUMBER)
	{
		// 비Direct 사출 안전 검사를 시작할 실제 Muzzle 위치입니다.
		const FVector TraceStart = LaunchContext.LaunchTransform.GetLocation();

		// 비Direct 사출 안전 검사가 사용할 정규화 초기 분리 방향입니다.
		const FVector TraceDirection = LaunchContext.InitialLaunchDirection.GetSafeNormal();

		// 사출 안전 Trace를 실행할 현재 World입니다.
		UWorld* World = VehiclePawn->GetWorld();
		if (World && !TraceStart.ContainsNaN() && !TraceDirection.ContainsNaN() && !TraceDirection.IsNearlyZero())
		{
			// 설정된 안전 검사 거리 앞쪽의 Trace 종료점입니다.
			const FVector TraceEnd = TraceStart + TraceDirection * ClearanceTraceDistanceCm;

			// 사출 경로의 첫 Blocking Hit 결과입니다.
			FHitResult ReleaseHitResult;

			// 발사 차량 자신을 제외한 사출 경로 Query 설정입니다.
			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CarFightLauncherReleaseTrace), false);
			QueryParams.AddIgnoredActor(VehiclePawn);

			// 실제 초기 사출 방향 앞쪽이 Blocking Hit으로 막혔는지 여부입니다.
			const bool bReleasePathBlocked = World->LineTraceSingleByChannel(ReleaseHitResult, TraceStart, TraceEnd, CFCollisionChannels::WeaponHit, QueryParams);
			if (bReleasePathBlocked)
			{
				// 사출 경로에서 처음 적중한 Actor입니다.
				AActor* HitActor = ReleaseHitResult.GetActor();

				// 적중 Actor가 정상 타격 대상으로 허용할 피해 가능 차량인지 여부입니다.
				const bool bHitDamageableVehicle = IsValid(HitActor) && IsValid(HitActor->FindComponentByClass<UCFVehicleHealthComp>());
				if (!bHitDamageableVehicle)
				{
					RollbackSingleFireAmmoReservation(VehiclePawn, AmmoWeaponInstanceId, bAmmoReservationActive);
					InOutFireResult.bAccepted = false;
					InOutFireResult.RejectReason = ECFVehicleFireRejectReason::MuzzleBlocked;
					InOutFireResult.ValidationAimTargetLocation = ReleaseHitResult.ImpactPoint;
					InOutFireResult.LocalHitLocation = ReleaseHitResult.ImpactPoint;
					InOutFireResult.LocalHitNormal = ReleaseHitResult.ImpactNormal;
					return false;
				}
			}
		}
	}

	// 안전 검사에 사용한 같은 Launch Context 인스턴스를 Pool 활성화에 전달한 Projectile입니다.
	ACFProjectileActor* AcquiredProjectileActor = VehiclePawn->ProjectilePoolComp
		? VehiclePawn->ProjectilePoolComp->AcquireProjectileWithContext(ActiveProjectileData, LaunchContext, VehiclePawn)
		: nullptr;
	if (AcquiredProjectileActor)
	{
		// 실제 Projectile을 확보한 경우에만 예약 탄약을 소비로 확정합니다.
		const bool bAmmoCommitted = CommitSingleFireAmmoReservation(VehiclePawn, AmmoWeaponInstanceId, bAmmoReservationActive);
		if (!bAmmoCommitted)
		{
			InOutFireResult.bAccepted = false;
			InOutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidLocalState;
			return false;
		}
		return true;
	}

	if (bAllowProjectileFallback && LaunchContext.ReleaseMode == ECFProjectileReleaseMode::Direct)
	{
		RunLocalDummyHitScan(FireCommand, InOutFireResult);

		// Direct fallback HitScan의 실제 발사 성공을 탄약 소비로 확정한 결과입니다.
		const bool bAmmoCommitted = CommitSingleFireAmmoReservation(VehiclePawn, AmmoWeaponInstanceId, bAmmoReservationActive);
		if (!bAmmoCommitted)
		{
			InOutFireResult.bAccepted = false;
			InOutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidLocalState;
			return false;
		}
		return true;
	}

	RollbackSingleFireAmmoReservation(VehiclePawn, AmmoWeaponInstanceId, bAmmoReservationActive);
	InOutFireResult.bAccepted = false;
	InOutFireResult.RejectReason = ECFVehicleFireRejectReason::NoWeapon;
	return false;
}

// Launcher 예약 후속 발사를 첫 발사 순간 위치/Actor Snapshot을 유지해 실행합니다.
bool UCFVehicleFireComp::ExecuteScheduledLauncherShot(const int32 VolleyId, const int32 SequenceShotIndex, const FVector& CommandTargetLocation, AActor* GuidanceTargetActorSnapshot)
{
	// Launcher compatibility callback과 Fire observable Authority를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return false;
	}

	(void)VolleyId;
	(void)SequenceShotIndex;

	// Pawn Authority가 새 Request ID/시간을 할당하고 첫 발사 순간 Command Target 위치를 유지해 만든 후속 발사 요청입니다.
	FCFVehicleFireRequest ScheduledFireRequest = VehiclePawn->BuildFireCommandForTarget(CommandTargetLocation, true);

	// 후속 발사의 검증과 실행 결과입니다.
	FCFVehicleFireResult ScheduledFireResult;

	// 후속 발사에서는 입력 단위 Weapon 쿨다운만 우회하고 나머지 조건을 동일하게 검증한 결과입니다.
	const bool bFireCommandAccepted = ValidateFireCommandInternal(ScheduledFireRequest, ScheduledFireResult, true);

	// 첫 발사 순간 Guidance Target Actor Snapshot을 현재 TargetSelect 재조회 없이 전달한 실행 결과입니다.
	const bool bFireCommandExecuted = bFireCommandAccepted
		&& ExecuteAcceptedFireCommand(ScheduledFireRequest, ScheduledFireResult, GuidanceTargetActorSnapshot, false);

	VehiclePawn->ApplyFireResultInternal(ScheduledFireRequest, ScheduledFireResult, false);
	return bFireCommandExecuted && ScheduledFireResult.bAccepted;
}

// Pawn이 먼저 observable로 기록한 입력 FireRequest와 목표 Snapshot을 사용해 Ammo/Launcher 발사 흐름을 실행합니다.
void UCFVehicleFireComp::HandleFireStarted(const FCFVehicleFireRequest& FireRequest)
{
	// Fire observable/Launcher/Ammo Authority를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return;
	}

	// 진행 중인 Ripple/Salvo 시퀀스가 새 입력으로 교체되지 않도록 사용할 발사 결과입니다.
	FCFVehicleFireResult FireResult;
	if (VehiclePawn->LauncherComp && VehiclePawn->LauncherComp->IsFireSequenceActive())
	{
		FireResult.FireRequestId = FireRequest.FireRequestId;
		FireResult.ValidationAimTargetLocation = FireRequest.PredictedAimTargetLocation;
		FireResult.LocalHitLocation = FireRequest.PredictedAimTargetLocation;
		FireResult.bAccepted = false;

		// 유한탄 Sequence Action Lock이 실제로 활성 상태인지 확인할 현재 Ammo WeaponInstanceId입니다.
		const FName ActiveAmmoWeaponInstanceId = ResolveActiveAmmoWeaponInstanceId(VehiclePawn);
		FireResult.RejectReason = VehiclePawn->VehicleAmmoComp
			&& VehiclePawn->VehicleAmmoComp->HasActiveLauncherSequenceReservation(ActiveAmmoWeaponInstanceId)
			? ECFVehicleFireRejectReason::WeaponActionLocked
			: ECFVehicleFireRejectReason::WeaponCooldown;
		VehiclePawn->ApplyFireResultInternal(FireRequest, FireResult, false);
		return;
	}

	// 첫 Projectile과 같은 Volley의 모든 후속 Projectile이 공유할 발사 순간 선택 목표 Actor Snapshot입니다.
	AActor* GuidanceTargetActorSnapshot = VehiclePawn->TargetSelectComp ? VehiclePawn->TargetSelectComp->GetSelectedTargetActor() : nullptr;

	// 첫 발 실행 전에 유한탄 예약 수량에 맞게 축소될 수 있는 현재 Launcher 발사 패턴 설정입니다.
	FCFLauncherFirePatternConfig FirePatternConfig = VehiclePawn->VehicleWeaponComp
		? VehiclePawn->VehicleWeaponComp->GetActiveLauncherFirePatternConfig()
		: FCFLauncherFirePatternConfig();

	// 이번 입력의 유한탄 여부와 부분 시퀀스 정책을 제공할 현재 활성 WeaponData입니다.
	UCFWeaponData* ActiveWeaponData = VehiclePawn->VehicleWeaponComp ? VehiclePawn->VehicleWeaponComp->GetActiveWeaponData() : nullptr;

	// SingleCycle이 아닌 Ripple/Salvo에서 전체 발수 예약을 사용해야 하는지 여부입니다.
	const bool bUseLauncherAmmoReservation = ActiveWeaponData
		&& ActiveWeaponData->UsesFiniteAmmoRuntime()
		&& FirePatternConfig.FirePattern != ECFLauncherFirePattern::SingleCycle;

	// Launcher 예약이 활성화되면 Ammo Runtime에서 현재 무기를 식별할 WeaponInstanceId입니다.
	const FName LauncherAmmoWeaponInstanceId = bUseLauncherAmmoReservation ? ResolveActiveAmmoWeaponInstanceId(VehiclePawn) : NAME_None;

	// 첫 발 실행 전에 전체 시퀀스 예약이 성공해 이후 Commit/Release가 필요한지 여부입니다.
	bool bLauncherAmmoReservationActive = false;

	// 첫 Projectile은 기존 쿨다운을 포함한 전체 조준/Muzzle/장비 검증을 먼저 수행합니다.
	const bool bFireCommandAccepted = ValidateFireCommand(FireRequest, FireResult);
	if (bFireCommandAccepted && bUseLauncherAmmoReservation)
	{
		if (!VehiclePawn->LauncherComp || !VehiclePawn->VehicleAmmoComp)
		{
			FireResult.bAccepted = false;
			FireResult.RejectReason = ECFVehicleFireRejectReason::InvalidLocalState;
		}
		else
		{
			// 현재 장전량과 부분 시퀀스 정책으로 실제 실행할 수 있도록 예약된 Launcher 총 발수입니다.
			int32 ReservedLauncherShotCount = 0;

			// 첫 발을 포함한 전체 유효 발수의 사전 예약 결과입니다.
			const ECFAmmoTransactionResult LauncherReservationResult = VehiclePawn->VehicleAmmoComp->ReserveLauncherSequenceAmmo(
				LauncherAmmoWeaponInstanceId,
				FirePatternConfig.GetEffectiveProjectileCount(),
				ActiveWeaponData->bAllowPartialSequence,
				ReservedLauncherShotCount);
			if (LauncherReservationResult == ECFAmmoTransactionResult::Accepted && ReservedLauncherShotCount > 0)
			{
				FirePatternConfig.ProjectileCountPerTrigger = ReservedLauncherShotCount;
				bLauncherAmmoReservationActive = true;
			}
			else
			{
				FireResult.bAccepted = false;
				FireResult.RejectReason = ResolveAmmoTransactionFireRejectReason(LauncherReservationResult);
			}
		}
	}

	if (FireResult.bAccepted)
	{
		ExecuteAcceptedFireCommand(FireRequest, FireResult, GuidanceTargetActorSnapshot, true);
	}

	if (bLauncherAmmoReservationActive)
	{
		if (FireResult.bAccepted)
		{
			// 실제 성공한 첫 Launcher 발사를 사전 예약량에서 정확히 한 발 소비로 확정한 결과입니다.
			const ECFAmmoTransactionResult FirstShotCommitResult = VehiclePawn->VehicleAmmoComp->CommitReservedLauncherShot(LauncherAmmoWeaponInstanceId);
			if (FirstShotCommitResult != ECFAmmoTransactionResult::Accepted)
			{
				VehiclePawn->VehicleAmmoComp->ReleaseLauncherSequenceReservation(LauncherAmmoWeaponInstanceId);
				bLauncherAmmoReservationActive = false;
				FireResult.bAccepted = false;
				FireResult.RejectReason = ECFVehicleFireRejectReason::InvalidLocalState;
			}
		}
		else
		{
			VehiclePawn->VehicleAmmoComp->ReleaseLauncherSequenceReservation(LauncherAmmoWeaponInstanceId);
			bLauncherAmmoReservationActive = false;
		}
	}

	// 첫 승인 시점에 쿨다운을 시작하는 정책인지 여부입니다.
	const bool bRecordCooldownOnFirstAcceptedProjectile = !VehiclePawn->LauncherComp
		|| FirePatternConfig.CooldownStartPolicy == ECFLauncherCooldownStartPolicy::FirstAcceptedProjectile;
	VehiclePawn->ApplyFireResultInternal(FireRequest, FireResult, bRecordCooldownOnFirstAcceptedProjectile);

	if (FireResult.bAccepted && VehiclePawn->LauncherComp)
	{
		// 첫 발사 순간 목표 Snapshot과 남은 발사 및 선택적 유한탄 예약을 LauncherComp에 인계한 결과입니다.
		const bool bSequenceStarted = VehiclePawn->LauncherComp->StartFireSequenceAfterFirstAcceptedShot(
			FirePatternConfig,
			FVector(FireRequest.PredictedAimTargetLocation),
			GuidanceTargetActorSnapshot,
			FireRequest.ClientFireTimeSeconds,
			bLauncherAmmoReservationActive ? LauncherAmmoWeaponInstanceId : NAME_None);

		if (!bSequenceStarted && bLauncherAmmoReservationActive && VehiclePawn->VehicleAmmoComp)
		{
			VehiclePawn->VehicleAmmoComp->ReleaseLauncherSequenceReservation(LauncherAmmoWeaponInstanceId);
			bLauncherAmmoReservationActive = false;
		}

		if (!bSequenceStarted && !bRecordCooldownOnFirstAcceptedProjectile && VehiclePawn->VehicleWeaponComp)
		{
			VehiclePawn->VehicleWeaponComp->RecordAcceptedFire(FireRequest.ClientFireTimeSeconds);
		}
	}
}

// Pawn이 Fire observable state를 commit한 뒤 Weapon/Aim/CombatFx side effect를 기존 순서로 적용합니다.
void UCFVehicleFireComp::ApplyFireResultSideEffects(const FCFVehicleFireRequest& FireCommand, const FCFVehicleFireResult& FireResult, const bool bRecordCooldown)
{
	// Fire side effect 대상 Domain Component를 소유한 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = ResolveVehiclePawn();
	if (!VehiclePawn)
	{
		return;
	}

	if (FireResult.bAccepted && VehiclePawn->VehicleWeaponComp)
	{
		VehiclePawn->VehicleWeaponComp->RecordAcceptedWeaponShotHeat();
		VehiclePawn->VehicleWeaponComp->RecordAcceptedWeaponShotCharge();
		if (bRecordCooldown)
		{
			VehiclePawn->VehicleWeaponComp->RecordAcceptedFire(FireCommand.ClientFireTimeSeconds);
		}
		VehiclePawn->VehicleWeaponComp->AdvanceMuzzleSequenceAfterAcceptedFire(FireCommand.MuzzleSocketName, FireCommand.MuzzleSocketIndex, FireCommand.MuzzleSocketCount);

		// Fire FX 설정을 제공할 현재 활성 WeaponData입니다.
		UCFWeaponData* ActiveWeaponData = VehiclePawn->VehicleWeaponComp->GetActiveWeaponData();
		if (VehiclePawn->CombatFxComp && ActiveWeaponData)
		{
			VehiclePawn->CombatFxComp->PlayFireFx(ActiveWeaponData->DefaultFireFxData, FVector(FireCommand.AimOrigin), FVector(FireCommand.AimDirection));
		}
	}

	if (VehiclePawn->VehicleAimComp)
	{
		VehiclePawn->VehicleAimComp->BuildFireValidationStateFromFireCommand(FireCommand, FireResult.RejectReason, FireResult.bAccepted);
		VehiclePawn->VehicleAimComp->ApplyFireValidationResult(FireResult);
		VehiclePawn->VehicleAimComp->UpdateAimVisualFromFireResult(FireCommand, FireResult);
	}
}
