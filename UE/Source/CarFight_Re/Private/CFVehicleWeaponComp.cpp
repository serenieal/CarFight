// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.18.1
// Date: 2026-07-24
// Description: CarFight 차량 전투 장착 프로파일과 선택 대상 사용 평가 컴포넌트 구현
// Scope: 장착 데이터, FireOrigin, 터렛 상태와 활성 무기의 선택 대상 사용 가능 캐시를 제공합니다.
// Changelog:
// - v1.18.1: 이동 중 사거리 진입·이탈을 반영하도록 저빈도 선택 대상 재평가 Tick을 추가.
// - v1.18.0: TS-P0-07 TargetSelectComp 이벤트 구독과 활성 무기 대상 평가 요청·결과·변경 알림을 구현.
// - v1.17.0: EquipmentPresetData 내부 참조만 WeaponData / TurretMountData로 해석하고 MountProfile legacy 직접 fallback을 제거.
// - v1.16.0: MountProfile.DefaultEquipmentPresetData를 우선 해석하고 WeaponData / TurretMountData 직접 참조 fallback을 유지.
// - v1.15.0: 터렛 회전 제한을 MountProfile 각도와의 교집합에서 TurretMountData 단독 기준으로 전환.
// - v1.14.0: 터렛 회전 / 안정화 값의 MountProfile legacy fallback을 런타임에서 제거하고 TurretMountData를 필수 소스로 전환.
// - v1.13.0: DamageData 해석을 ProjectileData 단일 소유 경로로 정리하고 WeaponData DamageData fallback을 제거.
// - v1.12.0: ProjectileData / WeaponData의 DamageData 참조를 해석하고 활성 DamageData 디버그 상태를 캐시.
// - v1.11.0: 터렛 추적 보간 이후 CurrentYaw/Pitch를 최종 유효 각도 안으로 다시 고정.
// - v1.10.0: Pawn 단계에서 Muzzle 소켓으로 보정된 최종 FireOrigin을 마지막 결과로 기록하는 함수를 구현.
// - v1.9.0: HardpointSlot.SocketName이 유효하면 FireOrigin 계산에서 차체 소켓 Transform을 LocalTransform보다 우선 사용.
// - v1.8.0: 활성 터렛 마운트의 Yaw / Pitch 목표각과 회전 추적 상태를 계산하는 런타임 상태를 추가.
// - v1.7.0: FireRatePerMinute를 활성 무기 런타임 값으로 노출하고 쿨다운 검증은 환산 발사 간격을 사용.
// - v1.6.0: Projectile 실행 요약을 Pool Acquire 기반 실제 실행 경로에 맞게 갱신.
// - v1.5.0: WeaponData.FireMode가 Projectile일 때만 Projectile 스폰 준비 상태로 판정.
// - v1.4.0: 활성 ProjectileData의 ProjectileActorClass 기반 스폰 준비 상태와 전환 요약을 캐시.
// - v1.3.0: 활성 WeaponData의 DefaultProjectileData를 캐시하고 디버그 요약에 포함.
// - v1.2.0: 활성 WeaponData의 MaxRange / CooldownSeconds를 Fire 검증과 Trace에서 사용할 수 있도록 런타임 함수를 추가.
// - v1.1.0: 활성 MountProfile의 DefaultWeaponData를 캐시하고 장착 타입/크기 호환성을 디버그 요약에 포함.
// - v1.0.0: P0 Top_01 터렛 발사 원점 계산을 위한 최소 WeaponComp 구현.
// Migration:
// - DefaultEquipmentPresetData가 지정되면 TurretMountData / WeaponData의 단일 소스로 사용하고, 프리셋 내부 참조가 비면 해당 장비 데이터는 Missing 상태가 된다.
// - 터렛이 조준 목표를 따라가는 중이어도 발사는 막지 않으며, 시각 회전값은 TurretMountData의 Min/Max Yaw/Pitch 안에 고정한다.
// - EquipmentPresetData 내부 TurretMountData가 비어 있으면 터렛 조준 추적은 MissingTurretMountData로 건너뛰며, 발사 / Projectile / 쿨다운 흐름은 유지한다.
// - SM_Body가 없으면 기존 차량 Actor Transform을 기준으로 fallback한다.
// - EquipmentPresetData 내부 WeaponData가 비어 있거나 호환되지 않아도 기존 FireOrigin 계산은 막지 않는다.
// - WeaponData가 없거나 호환되지 않으면 MaxRange / FireRatePerMinute는 기존 Aim Profile / 즉시 발사 fallback을 사용한다.
// - ProjectileData 또는 ProjectileActorClass가 비어 있거나 FireMode가 HitScan이면 Dummy HitScan / FireOrigin / 발사 간격 검증 흐름은 유지한다.
// - DamageData 직접 참조는 ProjectileData.DefaultDamageData만 사용하고, HitScan / Laser는 가상 ProjectileData로 연결한다.
// - DamageData가 비어 있거나 ProjectileData.DamageProfileId fallback만 있어도 발사 가능 여부와 Projectile 전환 조건은 변경하지 않는다.
// - 실제 Projectile Actor 확보 / 활성화는 Pawn의 로컬 Fire 적용 단계에서 ProjectilePoolComp를 통해 실행한다.
// - 터렛 각도 상태는 WeaponComp가 소유하고, Pawn은 계산된 각도를 시각 피벗 컴포넌트에 적용만 한다.
// - HardpointSlot.SocketName이 비어 있거나 부모 컴포넌트에 소켓이 없으면 기존 LocalTransform fallback을 유지한다.
// - Muzzle 소켓 FireOrigin 보정은 Pawn이 수행하고, WeaponComp는 최종 결과 기록만 담당한다.

#include "CFVehicleWeaponComp.h"

#include "CFDamageData.h"
#include "CFEquipmentPresetData.h"
#include "CFProjectileData.h"
#include "CFTargetSelectComp.h"
#include "CFTurretMountData.h"
#include "CFWeaponData.h"
#include "CFVehicleData.h"
#include "CFVehiclePawn.h"

#include "Components/SceneComponent.h"

namespace
{
	bool AreTargetUseResultsEquivalent(const FCFTargetUseResult& LeftResult, const FCFTargetUseResult& RightResult)
	{
		return LeftResult.EquipmentId == RightResult.EquipmentId
			&& LeftResult.TargetActor == RightResult.TargetActor
			&& LeftResult.DisplayInfo.TargetId == RightResult.DisplayInfo.TargetId
			&& LeftResult.DisplayInfo.TargetCategory == RightResult.DisplayInfo.TargetCategory
			&& LeftResult.DisplayInfo.Relation == RightResult.DisplayInfo.Relation
			&& LeftResult.TrackState == RightResult.TrackState
			&& FMath::IsNearlyEqual(LeftResult.MaxUseDistanceCm, RightResult.MaxUseDistanceCm, 0.1f)
			&& LeftResult.bEquipmentReady == RightResult.bEquipmentReady
			&& LeftResult.bHasSelectedTarget == RightResult.bHasSelectedTarget
			&& LeftResult.bSelectedTargetValid == RightResult.bSelectedTargetValid
			&& LeftResult.bTargetCompatible == RightResult.bTargetCompatible
			&& LeftResult.bWithinUseDistance == RightResult.bWithinUseDistance
			&& LeftResult.bCanUseTarget == RightResult.bCanUseTarget
			&& LeftResult.FailureReason == RightResult.FailureReason
			&& LeftResult.FailureAttributeTag == RightResult.FailureAttributeTag;
	}
}

// [v1.0.0] 기본 컴포넌트 값을 초기화합니다.
UCFVehicleWeaponComp::UCFVehicleWeaponComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UCFVehicleWeaponComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindTargetSelectEvents();
	Super::EndPlay(EndPlayReason);
}

void UCFVehicleWeaponComp::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bAutoRefreshTargetUseResult
		|| !BoundTargetSelectComp.IsValid()
		|| !BoundTargetSelectComp->HasSelectedTarget())
	{
		return;
	}

	TargetUseRefreshElapsedSeconds += FMath::Max(DeltaTime, 0.0f);
	const float RefreshIntervalSeconds = FMath::Max(TargetUseRefreshIntervalSeconds, 0.01f);
	if (TargetUseRefreshElapsedSeconds + KINDA_SMALL_NUMBER < RefreshIntervalSeconds)
	{
		return;
	}

	TargetUseRefreshElapsedSeconds = 0.0f;
	RefreshActiveWeaponTargetUseResult();
}

FCFTargetUseRequest UCFVehicleWeaponComp::BuildActiveWeaponTargetUseRequest() const
{
	FCFTargetUseRequest UseRequest;
	UseRequest.EquipmentId = ActiveWeaponData
		? ActiveWeaponData->WeaponId
		: (ActiveEquipmentPresetData ? ActiveEquipmentPresetData->EquipmentId : ActiveMountProfileId);
	UseRequest.bEquipmentReady = bWeaponRuntimeReady && ActiveWeaponData && bActiveWeaponDataCompatible;
	UseRequest.MaxUseDistanceCm = ActiveWeaponData ? FMath::Max(ActiveWeaponData->MaxRange, 0.0f) : 0.0f;
	UseRequest.bUseExplicitOrigin = OwnerVehiclePawn != nullptr;
	UseRequest.ExplicitUseOrigin = OwnerVehiclePawn ? OwnerVehiclePawn->GetActorLocation() : FVector::ZeroVector;
	if (ActiveWeaponData)
	{
		UseRequest.TargetPolicy = ActiveWeaponData->TargetUsePolicy;
	}
	return UseRequest;
}

FCFTargetUseResult UCFVehicleWeaponComp::EvaluateSelectedTargetForActiveWeapon() const
{
	const FCFTargetUseRequest UseRequest = BuildActiveWeaponTargetUseRequest();
	const UCFTargetSelectComp* TargetSelectComponent = BoundTargetSelectComp.Get();
	if (!TargetSelectComponent && OwnerVehiclePawn)
	{
		TargetSelectComponent = OwnerVehiclePawn->GetTargetSelectComp();
	}
	if (!TargetSelectComponent)
	{
		FCFTargetUseResult MissingSystemResult;
		MissingSystemResult.EquipmentId = UseRequest.EquipmentId;
		MissingSystemResult.bEquipmentReady = UseRequest.bEquipmentReady;
		MissingSystemResult.MaxUseDistanceCm = UseRequest.MaxUseDistanceCm;
		MissingSystemResult.FailureReason = ECFTargetUseFailureReason::TargetSystemUnavailable;
		MissingSystemResult.ResultMessage = FText::FromString(TEXT("차량에 TargetSelectComp가 없어 선택 대상을 평가할 수 없습니다."));
		return MissingSystemResult;
	}
	return TargetSelectComponent->EvaluateSelectedTargetForUse(UseRequest);
}

bool UCFVehicleWeaponComp::RefreshActiveWeaponTargetUseResult()
{
	const FCFTargetUseResult NewTargetUseResult = EvaluateSelectedTargetForActiveWeapon();
	const bool bResultChanged = !AreTargetUseResultsEquivalent(LastActiveWeaponTargetUseResult, NewTargetUseResult);
	LastActiveWeaponTargetUseResult = NewTargetUseResult;

	if (const UCFTargetSelectComp* TargetSelectComponent = BoundTargetSelectComp.Get())
	{
		LastActiveWeaponTargetUseSummary = TargetSelectComponent->BuildTargetUseDebugSummary(LastActiveWeaponTargetUseResult);
	}
	else
	{
		LastActiveWeaponTargetUseSummary = FString::Printf(
			TEXT("ActiveWeaponTargetUse: Equipment=%s, TargetSelectComp=Missing, Failure=%s"),
			*LastActiveWeaponTargetUseResult.EquipmentId.ToString(),
			*UEnum::GetValueAsString(LastActiveWeaponTargetUseResult.FailureReason));
	}

	if (bResultChanged)
	{
		OnActiveWeaponTargetUseChanged.Broadcast(LastActiveWeaponTargetUseResult);
	}
	return LastActiveWeaponTargetUseResult.bCanUseTarget;
}

void UCFVehicleWeaponComp::BindTargetSelectEvents()
{
	UnbindTargetSelectEvents();
	if (!OwnerVehiclePawn)
	{
		return;
	}

	UCFTargetSelectComp* TargetSelectComponent = OwnerVehiclePawn->GetTargetSelectComp();
	if (!TargetSelectComponent)
	{
		return;
	}

	BoundTargetSelectComp = TargetSelectComponent;
	TargetSelectComponent->OnSelectedTargetChanged.AddUniqueDynamic(this, &UCFVehicleWeaponComp::HandleSelectedTargetChangedForWeapon);
	TargetSelectComponent->OnSelectedTargetCleared.AddUniqueDynamic(this, &UCFVehicleWeaponComp::HandleSelectedTargetClearedForWeapon);
	TargetSelectComponent->OnSelectedTargetValidityChanged.AddUniqueDynamic(this, &UCFVehicleWeaponComp::HandleSelectedTargetValidityChangedForWeapon);
	TargetSelectComponent->OnSelectedTargetTrackStateChanged.AddUniqueDynamic(this, &UCFVehicleWeaponComp::HandleSelectedTargetTrackStateChangedForWeapon);
}

void UCFVehicleWeaponComp::UnbindTargetSelectEvents()
{
	if (UCFTargetSelectComp* TargetSelectComponent = BoundTargetSelectComp.Get())
	{
		TargetSelectComponent->OnSelectedTargetChanged.RemoveDynamic(this, &UCFVehicleWeaponComp::HandleSelectedTargetChangedForWeapon);
		TargetSelectComponent->OnSelectedTargetCleared.RemoveDynamic(this, &UCFVehicleWeaponComp::HandleSelectedTargetClearedForWeapon);
		TargetSelectComponent->OnSelectedTargetValidityChanged.RemoveDynamic(this, &UCFVehicleWeaponComp::HandleSelectedTargetValidityChangedForWeapon);
		TargetSelectComponent->OnSelectedTargetTrackStateChanged.RemoveDynamic(this, &UCFVehicleWeaponComp::HandleSelectedTargetTrackStateChangedForWeapon);
	}
	BoundTargetSelectComp.Reset();
}

void UCFVehicleWeaponComp::HandleSelectedTargetChangedForWeapon(AActor*, AActor*, FCFTargetDisplayInfo)
{
	RefreshActiveWeaponTargetUseResult();
}

void UCFVehicleWeaponComp::HandleSelectedTargetClearedForWeapon(AActor*, ECFTargetClearReason)
{
	RefreshActiveWeaponTargetUseResult();
}

void UCFVehicleWeaponComp::HandleSelectedTargetValidityChangedForWeapon(AActor*, bool)
{
	RefreshActiveWeaponTargetUseResult();
}

void UCFVehicleWeaponComp::HandleSelectedTargetTrackStateChangedForWeapon(AActor*, ECFTargetTrackState, ECFTargetTrackState)
{
	RefreshActiveWeaponTargetUseResult();
}

// [v1.0.0] Owner Pawn과 VehicleData 참조를 준비합니다.
bool UCFVehicleWeaponComp::InitializeWeaponRuntime(ACFVehiclePawn* InOwnerVehiclePawn, UCFVehicleData* InVehicleData)
{
	UnbindTargetSelectEvents();
	OwnerVehiclePawn = InOwnerVehiclePawn;
	CachedVehicleData = InVehicleData;
	LastActiveWeaponTargetUseResult = FCFTargetUseResult();
	LastActiveWeaponTargetUseSummary = TEXT("ActiveWeaponTargetUse: Initializing");
	TargetUseRefreshElapsedSeconds = 0.0f;
	BindTargetSelectEvents();
	bWeaponRuntimeReady = false;
	ActiveEquipmentPresetData = nullptr;
	bActiveEquipmentPresetCompatible = false;
	ActiveEquipmentPresetSummary = TEXT("EquipmentPresetData: MissingOptional");
	ActiveWeaponData = nullptr;
	bActiveWeaponDataCompatible = false;
	ActiveWeaponSummary = TEXT("WeaponData: MissingOptional");
	ActiveProjectileData = nullptr;
	ActiveProjectileSummary = TEXT("ProjectileData: MissingOptional");
	ActiveDamageData = nullptr;
	ActiveDamageId = NAME_None;
	ActiveDamageSummary = TEXT("DamageData: MissingOptional");
	ActiveDamageResolutionSummary = TEXT("DamageResolution: MissingOptional");
	bActiveProjectileSpawnReady = false;
	ActiveProjectileExecutionSummary = TEXT("ProjectileExecution: DummyHitScanFallback");
	LastFireOrigin = FCFVehicleFireOrigin();
	ResetTurretState();

	if (!OwnerVehiclePawn)
	{
		LastWeaponRuntimeSummary = TEXT("VehicleWeaponRuntime: OwnerVehiclePawn=Missing");
		RefreshActiveWeaponTargetUseResult();
		return false;
	}

	if (!CachedVehicleData)
	{
		LastWeaponRuntimeSummary = TEXT("VehicleWeaponRuntime: VehicleData=Missing");
		RefreshActiveWeaponTargetUseResult();
		return false;
	}

	// [v1.0.0] 현재 활성 장착 프로파일 후보입니다.
	const FCFVehicleMountProfile* ActiveMountProfile = FindActiveMountProfile();
	if (!ActiveMountProfile)
	{
		LastWeaponRuntimeSummary = FString::Printf(TEXT("VehicleWeaponRuntime: ActiveMountProfile=Missing, Requested=%s"), *ActiveMountProfileId.ToString());
		RefreshActiveWeaponTargetUseResult();
		return false;
	}

	// [v1.0.0] 활성 장착 프로파일이 참조하는 하드포인트 슬롯 후보입니다.
	const FCFVehicleHardpointSlot* HardpointSlot = FindHardpointSlot(ActiveMountProfile->LocationSlotRef);
	if (!HardpointSlot)
	{
		LastWeaponRuntimeSummary = FString::Printf(TEXT("VehicleWeaponRuntime: HardpointSlot=Missing, LocationSlotRef=%s"), *ActiveMountProfile->LocationSlotRef.ToString());
		RefreshActiveWeaponTargetUseResult();
		return false;
	}

	CacheActiveWeaponData(*ActiveMountProfile);

	// [v1.16.0] 활성 EquipmentPresetData 호환 상태를 런타임 요약에 넣기 위한 표시 문자열입니다.
	const FString EquipmentPresetCompatibilityText = ActiveEquipmentPresetData
		? (bActiveEquipmentPresetCompatible ? TEXT("Yes") : TEXT("No"))
		: TEXT("MissingOptional");

	// [v1.1.0] 활성 WeaponData 호환 상태를 런타임 요약에 넣기 위한 표시 문자열입니다.
	const FString WeaponCompatibilityText = ActiveWeaponData
		? (bActiveWeaponDataCompatible ? TEXT("Yes") : TEXT("No"))
		: TEXT("MissingOptional");

	bWeaponRuntimeReady = true;
	LastWeaponRuntimeSummary = FString::Printf(
		TEXT("VehicleWeaponRuntime: Ready, Profile=%s, Slot=%s, Equipment=%s, EquipmentCompatible=%s, WeaponData=%s, Compatible=%s, Projectile=%s, Damage=%s"),
		*ActiveMountProfile->MountProfileId.ToString(),
		*HardpointSlot->LocationSlotId.ToString(),
		*ActiveEquipmentPresetSummary,
		*EquipmentPresetCompatibilityText,
		*ActiveWeaponSummary,
		*WeaponCompatibilityText,
		*ActiveProjectileExecutionSummary,
		*ActiveDamageResolutionSummary);
	RefreshActiveWeaponTargetUseResult();
	return true;
}

// [v1.8.0] 터렛 조준 추적 상태를 기본값으로 초기화합니다.
void UCFVehicleWeaponComp::ResetTurretState()
{
	TurretState = FCFVehicleTurretState();
	TurretSettleElapsedSeconds = 0.0f;
	LastTurretRuntimeSummary = TEXT("TurretRuntime: Reset");
}

// [v1.8.0] 활성 터렛 마운트의 Yaw / Pitch 목표각과 현재 추적 각도를 갱신합니다.
bool UCFVehicleWeaponComp::UpdateTurretState(
	const float DeltaSeconds,
	const FVector& WorldAimDirection,
	const FTransform& TurretReferenceTransform,
	const bool bHasTurretVisual)
{
	if (!OwnerVehiclePawn || !CachedVehicleData)
	{
		LastTurretRuntimeSummary = TEXT("TurretRuntime: RuntimeRefs=Missing");
		return false;
	}

	// [v1.8.0] 현재 터렛 조준 계산에 사용할 활성 장착 프로파일입니다.
	const FCFVehicleMountProfile* ActiveMountProfile = FindActiveMountProfile();
	if (!ActiveMountProfile)
	{
		LastTurretRuntimeSummary = FString::Printf(TEXT("TurretRuntime: MountProfile=Missing, Requested=%s"), *ActiveMountProfileId.ToString());
		return false;
	}

	if (ActiveMountProfile->MountType != ECFVehicleMountType::Turret)
	{
		LastTurretRuntimeSummary = FString::Printf(TEXT("TurretRuntime: SkippedNonTurret, Profile=%s"), *ActiveMountProfile->MountProfileId.ToString());
		return false;
	}

	// [v1.17.0] 터렛 마운트 데이터가 EquipmentPresetData에서 해석됐는지 여부입니다.
	const bool bUsingEquipmentPresetTurretMountData = ActiveMountProfile->DefaultEquipmentPresetData && ActiveMountProfile->DefaultEquipmentPresetData->DefaultTurretMountData;

	// [v1.17.0] 활성 EquipmentPresetData에서 해석한 터렛 마운트 데이터입니다.
	UCFTurretMountData* ActiveTurretMountData = bUsingEquipmentPresetTurretMountData
		? ActiveMountProfile->DefaultEquipmentPresetData->DefaultTurretMountData.Get()
		: nullptr;
	if (!ActiveTurretMountData)
	{
		ResetTurretState();
		LastTurretRuntimeSummary = FString::Printf(
			TEXT("TurretRuntime: TurretMountData=Missing, Profile=%s, Source=MissingEquipmentPresetTurretMountData"),
			*ActiveMountProfile->MountProfileId.ToString());
		return false;
	}

	if (!bHasTurretVisual)
	{
		LastTurretRuntimeSummary = FString::Printf(
			TEXT("TurretRuntime: Visual=Missing, Profile=%s, Source=TurretMountData"),
			*ActiveMountProfile->MountProfileId.ToString());
		return false;
	}

	// [v1.8.0] 조준 계산에 사용할 안전한 월드 방향입니다.
	FVector SafeWorldAimDirection = WorldAimDirection.GetSafeNormal();
	if (SafeWorldAimDirection.IsNearlyZero())
	{
		SafeWorldAimDirection = OwnerVehiclePawn->GetActorForwardVector().GetSafeNormal();
	}
	if (SafeWorldAimDirection.IsNearlyZero())
	{
		LastTurretRuntimeSummary = TEXT("TurretRuntime: AimDirection=Invalid");
		return false;
	}

	// [v1.8.0] 터렛 루트 기준 로컬 조준 방향입니다.
	const FVector LocalAimDirection = TurretReferenceTransform.InverseTransformVectorNoScale(SafeWorldAimDirection).GetSafeNormal();
	if (LocalAimDirection.IsNearlyZero())
	{
		LastTurretRuntimeSummary = TEXT("TurretRuntime: LocalAimDirection=Invalid");
		return false;
	}

	// [v1.8.0] Pitch 계산에서 사용할 수평 방향 길이입니다.
	const float HorizontalLength = FVector2D(LocalAimDirection.X, LocalAimDirection.Y).Size();

	// [v1.8.0] 제한 적용 전 터렛 기준 목표 Yaw 각도입니다.
	const float RawTargetYawDeg = FMath::RadiansToDegrees(FMath::Atan2(LocalAimDirection.Y, LocalAimDirection.X));

	// [v1.8.0] 제한 적용 전 터렛 기준 목표 Pitch 각도입니다.
	const float RawTargetPitchDeg = FMath::RadiansToDegrees(FMath::Atan2(LocalAimDirection.Z, HorizontalLength));

	// [v1.15.0] TurretMountData에서 읽은 최소 Yaw 각도입니다.
	float EffectiveMinYawDeg = ActiveTurretMountData->MinYawDeg;

	// [v1.15.0] TurretMountData에서 읽은 최대 Yaw 각도입니다.
	float EffectiveMaxYawDeg = ActiveTurretMountData->MaxYawDeg;

	// [v1.15.0] TurretMountData에서 읽은 최소 Pitch 각도입니다.
	float EffectiveMinPitchDeg = ActiveTurretMountData->MinPitchDeg;

	// [v1.15.0] TurretMountData에서 읽은 최대 Pitch 각도입니다.
	float EffectiveMaxPitchDeg = ActiveTurretMountData->MaxPitchDeg;

	// [v1.14.0] TurretMountData에서 읽은 터렛 Yaw 회전 속도입니다.
	const float YawTurnRateDegPerSec = ActiveTurretMountData->YawTurnRateDegPerSec;

	// [v1.14.0] TurretMountData에서 읽은 터렛 Pitch 회전 속도입니다.
	const float PitchTurnRateDegPerSec = ActiveTurretMountData->PitchTurnRateDegPerSec;

	// [v1.14.0] TurretMountData에서 읽은 안정화 판정 허용 오차입니다.
	const float StabilizationToleranceDeg = ActiveTurretMountData->StabilizationToleranceDeg;

	// [v1.14.0] TurretMountData에서 읽은 안정화 판정 유지 시간입니다.
	const float AimSettleTimeSeconds = ActiveTurretMountData->AimSettleTimeSeconds;

	// [v1.15.0] TurretMountData 회전 제한의 최소 / 최대 순서가 유효한지 여부입니다.
	const bool bLimitRangeValid = (EffectiveMinYawDeg <= EffectiveMaxYawDeg) && (EffectiveMinPitchDeg <= EffectiveMaxPitchDeg);
	if (!bLimitRangeValid)
	{
		// [v1.15.0] 잘못 입력된 Yaw 제한의 더 작은 값입니다.
		const float SortedMinYawDeg = FMath::Min(EffectiveMinYawDeg, EffectiveMaxYawDeg);

		// [v1.15.0] 잘못 입력된 Yaw 제한의 더 큰 값입니다.
		const float SortedMaxYawDeg = FMath::Max(EffectiveMinYawDeg, EffectiveMaxYawDeg);

		// [v1.15.0] 잘못 입력된 Pitch 제한의 더 작은 값입니다.
		const float SortedMinPitchDeg = FMath::Min(EffectiveMinPitchDeg, EffectiveMaxPitchDeg);

		// [v1.15.0] 잘못 입력된 Pitch 제한의 더 큰 값입니다.
		const float SortedMaxPitchDeg = FMath::Max(EffectiveMinPitchDeg, EffectiveMaxPitchDeg);

		EffectiveMinYawDeg = SortedMinYawDeg;
		EffectiveMaxYawDeg = SortedMaxYawDeg;
		EffectiveMinPitchDeg = SortedMinPitchDeg;
		EffectiveMaxPitchDeg = SortedMaxPitchDeg;
	}

	// [v1.8.0] 회전 제한이 적용된 목표 Yaw 각도입니다.
	const float TargetYawDeg = FMath::Clamp(RawTargetYawDeg, EffectiveMinYawDeg, EffectiveMaxYawDeg);

	// [v1.8.0] 회전 제한이 적용된 목표 Pitch 각도입니다.
	const float TargetPitchDeg = FMath::Clamp(RawTargetPitchDeg, EffectiveMinPitchDeg, EffectiveMaxPitchDeg);

	// [v1.8.0] 안전하게 사용할 프레임 DeltaSeconds입니다.
	const float SafeDeltaSeconds = FMath::Max(DeltaSeconds, 0.0f);

	// [v1.8.0] 음수 입력을 방지한 Yaw 회전 속도입니다.
	const float SafeYawTurnRateDegPerSec = FMath::Max(YawTurnRateDegPerSec, 0.0f);

	// [v1.8.0] 음수 입력을 방지한 Pitch 회전 속도입니다.
	const float SafePitchTurnRateDegPerSec = FMath::Max(PitchTurnRateDegPerSec, 0.0f);

	// [v1.8.0] 음수 입력을 방지한 안정화 허용 오차입니다.
	const float SafeStabilizationToleranceDeg = FMath::Max(StabilizationToleranceDeg, 0.0f);

	// [v1.8.0] 음수 입력을 방지한 안정화 시간입니다.
	const float SafeAimSettleTimeSeconds = FMath::Max(AimSettleTimeSeconds, 0.0f);

	TurretState.TargetYawDeg = TargetYawDeg;
	TurretState.TargetPitchDeg = TargetPitchDeg;
	TurretState.CurrentYawDeg = FMath::Clamp(TurretState.CurrentYawDeg, EffectiveMinYawDeg, EffectiveMaxYawDeg);
	TurretState.CurrentPitchDeg = FMath::Clamp(TurretState.CurrentPitchDeg, EffectiveMinPitchDeg, EffectiveMaxPitchDeg);

	if (SafeYawTurnRateDegPerSec > KINDA_SMALL_NUMBER)
	{
		TurretState.CurrentYawDeg = FMath::FInterpConstantTo(TurretState.CurrentYawDeg, TurretState.TargetYawDeg, SafeDeltaSeconds, SafeYawTurnRateDegPerSec);
	}

	if (SafePitchTurnRateDegPerSec > KINDA_SMALL_NUMBER)
	{
		TurretState.CurrentPitchDeg = FMath::FInterpConstantTo(TurretState.CurrentPitchDeg, TurretState.TargetPitchDeg, SafeDeltaSeconds, SafePitchTurnRateDegPerSec);
	}

	// [v1.11.0] 보간 이후에도 시각 Yaw 회전값이 최종 유효 제한각을 벗어나지 않게 고정합니다.
	TurretState.CurrentYawDeg = FMath::Clamp(TurretState.CurrentYawDeg, EffectiveMinYawDeg, EffectiveMaxYawDeg);

	// [v1.11.0] 보간 이후에도 시각 Pitch 회전값이 최종 유효 제한각을 벗어나지 않게 고정합니다.
	TurretState.CurrentPitchDeg = FMath::Clamp(TurretState.CurrentPitchDeg, EffectiveMinPitchDeg, EffectiveMaxPitchDeg);

	// [v1.8.0] 현재 Yaw와 목표 Yaw 사이의 절대 오차입니다.
	const float YawErrorDeg = FMath::Abs(TurretState.TargetYawDeg - TurretState.CurrentYawDeg);

	// [v1.8.0] 현재 Pitch와 목표 Pitch 사이의 절대 오차입니다.
	const float PitchErrorDeg = FMath::Abs(TurretState.TargetPitchDeg - TurretState.CurrentPitchDeg);

	// [v1.8.0] 현재 터렛 각도가 안정화 허용 오차 안에 있는지 여부입니다.
	const bool bWithinStabilizationTolerance = (YawErrorDeg <= SafeStabilizationToleranceDeg) && (PitchErrorDeg <= SafeStabilizationToleranceDeg);
	if (bWithinStabilizationTolerance)
	{
		TurretSettleElapsedSeconds += SafeDeltaSeconds;
	}
	else
	{
		TurretSettleElapsedSeconds = 0.0f;
	}

	TurretState.bTurretSettled = bWithinStabilizationTolerance && (TurretSettleElapsedSeconds >= SafeAimSettleTimeSeconds);
	LastTurretRuntimeSummary = FString::Printf(
		TEXT("TurretRuntime: Updated, Profile=%s, Source=%s, RawYaw=%.1f, RawPitch=%.1f, TargetYaw=%.1f, TargetPitch=%.1f, CurrentYaw=%.1f, CurrentPitch=%.1f, Settled=%s, LimitCorrection=%s"),
		*ActiveMountProfile->MountProfileId.ToString(),
		bUsingEquipmentPresetTurretMountData ? TEXT("EquipmentPresetData") : TEXT("MissingEquipmentPresetTurretMountData"),
		RawTargetYawDeg,
		RawTargetPitchDeg,
		TurretState.TargetYawDeg,
		TurretState.TargetPitchDeg,
		TurretState.CurrentYawDeg,
		TurretState.CurrentPitchDeg,
		TurretState.bTurretSettled ? TEXT("Yes") : TEXT("No"),
		bLimitRangeValid ? TEXT("No") : TEXT("TurretMountDataRangeSorted"));

	return true;
}

// [v1.2.0] 활성 WeaponData의 최대 사거리를 반환하고, 사용할 수 없으면 fallback 사거리를 반환합니다.
float UCFVehicleWeaponComp::GetActiveWeaponMaxRange(const float FallbackRange) const
{
	if (!ActiveWeaponData || !bActiveWeaponDataCompatible || ActiveWeaponData->MaxRange <= 0.0f)
	{
		return FallbackRange;
	}

	return ActiveWeaponData->MaxRange;
}

// [v1.7.0] 활성 WeaponData의 분당 발사속도를 반환합니다.
float UCFVehicleWeaponComp::GetActiveWeaponFireRatePerMinute() const
{
	if (!ActiveWeaponData || !bActiveWeaponDataCompatible)
	{
		return 0.0f;
	}

	return ActiveWeaponData->GetEffectiveFireRatePerMinute();
}

// [v1.7.0] 활성 WeaponData의 분당 발사속도를 초 단위 발사 간격으로 환산해 반환합니다.
float UCFVehicleWeaponComp::GetActiveWeaponCooldownSeconds() const
{
	if (!ActiveWeaponData || !bActiveWeaponDataCompatible)
	{
		return 0.0f;
	}

	return ActiveWeaponData->GetFireIntervalSeconds();
}

// [v1.2.0] 현재 시간 기준 남은 무기 쿨다운 시간을 반환합니다.
float UCFVehicleWeaponComp::GetRemainingCooldownSeconds(const float CurrentTimeSeconds) const
{
	// [v1.7.0] 활성 WeaponData의 분당 발사속도에서 환산한 발사 간격입니다.
	const float ActiveCooldownSeconds = GetActiveWeaponCooldownSeconds();
	if (ActiveCooldownSeconds <= 0.0f || LastAcceptedFireTimeSeconds < 0.0f)
	{
		return 0.0f;
	}

	// [v1.2.0] 현재 쿨다운이 끝나는 월드 시간입니다.
	const float CooldownEndTimeSeconds = LastAcceptedFireTimeSeconds + ActiveCooldownSeconds;

	return FMath::Max(CooldownEndTimeSeconds - CurrentTimeSeconds, 0.0f);
}

// [v1.2.0] 현재 시간 기준 활성 무기가 쿨다운 중인지 반환합니다.
bool UCFVehicleWeaponComp::IsActiveWeaponOnCooldown(const float CurrentTimeSeconds) const
{
	return GetRemainingCooldownSeconds(CurrentTimeSeconds) > KINDA_SMALL_NUMBER;
}

// [v1.2.0] 승인된 발사 시간을 기록해 이후 쿨다운 검증에 사용합니다.
void UCFVehicleWeaponComp::RecordAcceptedFire(const float AcceptedFireTimeSeconds)
{
	LastAcceptedFireTimeSeconds = FMath::Max(AcceptedFireTimeSeconds, 0.0f);
}

// [v1.0.0] 활성 장착 프로파일과 하드포인트 슬롯에서 실제 발사 원점을 계산합니다.
bool UCFVehicleWeaponComp::BuildFireOrigin(const FVector& FallbackAimDirection, FCFVehicleFireOrigin& OutFireOrigin)
{
	OutFireOrigin = FCFVehicleFireOrigin();

	if (!OwnerVehiclePawn || !CachedVehicleData)
	{
		LastFireOrigin = OutFireOrigin;
		LastWeaponRuntimeSummary = TEXT("VehicleWeaponFireOrigin: RuntimeRefs=Missing");
		return false;
	}

	// [v1.0.0] 발사 원점 계산에 사용할 활성 장착 프로파일입니다.
	const FCFVehicleMountProfile* ActiveMountProfile = FindActiveMountProfile();
	if (!ActiveMountProfile)
	{
		LastFireOrigin = OutFireOrigin;
		LastWeaponRuntimeSummary = FString::Printf(TEXT("VehicleWeaponFireOrigin: ActiveMountProfile=Missing, Requested=%s"), *ActiveMountProfileId.ToString());
		return false;
	}

	// [v1.0.0] 발사 원점 계산에 사용할 하드포인트 슬롯입니다.
	const FCFVehicleHardpointSlot* HardpointSlot = FindHardpointSlot(ActiveMountProfile->LocationSlotRef);
	if (!HardpointSlot)
	{
		LastFireOrigin = OutFireOrigin;
		LastWeaponRuntimeSummary = FString::Printf(TEXT("VehicleWeaponFireOrigin: HardpointSlot=Missing, LocationSlotRef=%s"), *ActiveMountProfile->LocationSlotRef.ToString());
		return false;
	}

	CacheActiveWeaponData(*ActiveMountProfile);

	// [v1.9.0] 하드포인트 LocalTransform 또는 SocketTransform을 월드로 변환할 부모 컴포넌트입니다.
	USceneComponent* MountParentComponent = FindMountParentComponent();

	// [v1.9.0] 하드포인트 부모 컴포넌트가 없을 때 사용할 fallback 부모 Transform입니다.
	const FTransform MountParentTransform = MountParentComponent ? MountParentComponent->GetComponentTransform() : OwnerVehiclePawn->GetActorTransform();

	// [v1.0.0] DataAsset에 저장된 하드포인트 슬롯 LocalTransform입니다.
	const FTransform HardpointLocalTransform(HardpointSlot->LocalRotation, HardpointSlot->LocalLocation);

	// [v1.0.0] 월드 기준 장착 Transform입니다.
	FTransform MountWorldTransform = HardpointLocalTransform * MountParentTransform;

	// [v1.9.0] 하드포인트 SocketName을 실제 소켓으로 해결했는지 여부입니다.
	bool bHardpointSocketResolved = false;

	if (MountParentComponent && !HardpointSlot->SocketName.IsNone() && MountParentComponent->DoesSocketExist(HardpointSlot->SocketName))
	{
		MountWorldTransform = MountParentComponent->GetSocketTransform(HardpointSlot->SocketName, RTS_World);
		bHardpointSocketResolved = true;
	}

	// [v1.0.0] 장착 Transform에서 계산한 기본 발사 방향입니다.
	FVector ResolvedFireDirection = MountWorldTransform.GetRotation().GetForwardVector().GetSafeNormal();
	if (ResolvedFireDirection.IsNearlyZero())
	{
		ResolvedFireDirection = FallbackAimDirection.GetSafeNormal();
	}
	if (ResolvedFireDirection.IsNearlyZero())
	{
		ResolvedFireDirection = OwnerVehiclePawn->GetActorForwardVector();
	}

	OutFireOrigin.bResolved = true;
	OutFireOrigin.MountProfileId = ActiveMountProfile->MountProfileId;
	OutFireOrigin.LocationSlotId = HardpointSlot->LocationSlotId;
	OutFireOrigin.MountType = ActiveMountProfile->MountType;
	OutFireOrigin.WorldFireLocation = MountWorldTransform.GetLocation();
	OutFireOrigin.WorldFireDirection = ResolvedFireDirection;

	// [v1.16.0] 활성 EquipmentPresetData 호환 상태를 발사 원점 요약에 넣기 위한 표시 문자열입니다.
	const FString EquipmentPresetCompatibilityText = ActiveEquipmentPresetData
		? (bActiveEquipmentPresetCompatible ? TEXT("Yes") : TEXT("No"))
		: TEXT("MissingOptional");

	// [v1.1.0] 활성 WeaponData 호환 상태를 발사 원점 요약에 넣기 위한 표시 문자열입니다.
	const FString WeaponCompatibilityText = ActiveWeaponData
		? (bActiveWeaponDataCompatible ? TEXT("Yes") : TEXT("No"))
		: TEXT("MissingOptional");

	LastFireOrigin = OutFireOrigin;
	LastWeaponRuntimeSummary = FString::Printf(
		TEXT("VehicleWeaponFireOrigin: Resolved, Profile=%s, Slot=%s, Socket=%s, SocketResolved=%s, Equipment=%s, EquipmentCompatible=%s, WeaponData=%s, Compatible=%s, Projectile=%s, Damage=%s, Location=(%.1f, %.1f, %.1f), Direction=(%.3f, %.3f, %.3f)"),
		*OutFireOrigin.MountProfileId.ToString(),
		*OutFireOrigin.LocationSlotId.ToString(),
		*HardpointSlot->SocketName.ToString(),
		bHardpointSocketResolved ? TEXT("Yes") : TEXT("No"),
		*ActiveEquipmentPresetSummary,
		*EquipmentPresetCompatibilityText,
		*ActiveWeaponSummary,
		*WeaponCompatibilityText,
		*ActiveProjectileExecutionSummary,
		*ActiveDamageResolutionSummary,
		OutFireOrigin.WorldFireLocation.X,
		OutFireOrigin.WorldFireLocation.Y,
		OutFireOrigin.WorldFireLocation.Z,
		OutFireOrigin.WorldFireDirection.X,
		OutFireOrigin.WorldFireDirection.Y,
		OutFireOrigin.WorldFireDirection.Z);
	return true;
}

// [v1.10.0] Pawn에서 보정한 최종 FireOrigin을 마지막 결과로 기록합니다.
void UCFVehicleWeaponComp::RecordResolvedFireOrigin(const FCFVehicleFireOrigin& InFireOrigin, const FString& InRuntimeSummary)
{
	LastFireOrigin = InFireOrigin;
	LastWeaponRuntimeSummary = InRuntimeSummary.IsEmpty() ? TEXT("VehicleWeaponFireOrigin: ResolvedExternal") : InRuntimeSummary;
}

// [v1.0.0] 현재 활성 장착 프로파일을 찾습니다.
const FCFVehicleMountProfile* UCFVehicleWeaponComp::FindActiveMountProfile() const
{
	if (!CachedVehicleData)
	{
		return nullptr;
	}

	for (const FCFVehicleMountProfile& MountProfile : CachedVehicleData->MountProfiles)
	{
		if (MountProfile.MountProfileId == ActiveMountProfileId)
		{
			return &MountProfile;
		}
	}

	if (ActiveMountProfileId.IsNone() && !CachedVehicleData->MountProfiles.IsEmpty())
	{
		return &CachedVehicleData->MountProfiles[0];
	}

	return nullptr;
}

// [v1.0.0] 위치 슬롯 ID와 일치하는 하드포인트 슬롯을 찾습니다.
const FCFVehicleHardpointSlot* UCFVehicleWeaponComp::FindHardpointSlot(const FName LocationSlotId) const
{
	if (!CachedVehicleData || LocationSlotId.IsNone())
	{
		return nullptr;
	}

	for (const FCFVehicleHardpointSlot& HardpointSlot : CachedVehicleData->HardpointSlots)
	{
		if (HardpointSlot.LocationSlotId == LocationSlotId)
		{
			return &HardpointSlot;
		}
	}

	return nullptr;
}

// [v1.17.0] 활성 장착 프로파일에 연결된 EquipmentPresetData / WeaponData / ProjectileData / DamageData와 Projectile 스폰 준비 상태를 캐시합니다.
void UCFVehicleWeaponComp::CacheActiveWeaponData(const FCFVehicleMountProfile& ActiveMountProfile)
{
	ActiveEquipmentPresetData = ActiveMountProfile.DefaultEquipmentPresetData;
	bActiveEquipmentPresetCompatible = false;
	ActiveEquipmentPresetSummary = TEXT("EquipmentPresetData: MissingOptional");
	ActiveWeaponData = ActiveEquipmentPresetData ? ActiveEquipmentPresetData->DefaultWeaponData.Get() : nullptr;
	bActiveWeaponDataCompatible = false;
	ActiveWeaponSummary = TEXT("WeaponData: MissingOptional");
	ActiveProjectileData = nullptr;
	ActiveProjectileSummary = TEXT("ProjectileData: MissingOptional");
	ActiveDamageData = nullptr;
	ActiveDamageId = NAME_None;
	ActiveDamageSummary = TEXT("DamageData: MissingOptional");
	ActiveDamageResolutionSummary = TEXT("DamageResolution: MissingOptional");
	bActiveProjectileSpawnReady = false;
	ActiveProjectileExecutionSummary = TEXT("ProjectileExecution: DummyHitScanFallback");

	if (ActiveEquipmentPresetData)
	{
		bActiveEquipmentPresetCompatible = ActiveEquipmentPresetData->CanUseOnMount(ActiveMountProfile.MountType, ActiveMountProfile.SizeLimit);
		ActiveEquipmentPresetSummary = ActiveEquipmentPresetData->BuildEquipmentSummary();
	}

	if (!ActiveWeaponData)
	{
		return;
	}

	bActiveWeaponDataCompatible = ActiveWeaponData->CanUseOnMount(ActiveMountProfile.MountType, ActiveMountProfile.SizeLimit);
	ActiveWeaponSummary = ActiveWeaponData->BuildWeaponSummary();
	ActiveProjectileData = ActiveWeaponData->DefaultProjectileData;
	CacheActiveDamageData();

	if (!bActiveWeaponDataCompatible)
	{
		ActiveProjectileExecutionSummary = TEXT("ProjectileExecution: WeaponDataIncompatibleNoSpawn");
	}

	if (ActiveProjectileData)
	{
		ActiveProjectileSummary = ActiveProjectileData->BuildProjectileSummary();

		// [v1.5.0] 활성 무기가 실제 Projectile 발사 모드인지 여부입니다.
		const bool bActiveWeaponUsesProjectileMode = ActiveWeaponData->FireMode == ECFWeaponFireMode::Projectile;

		if (bActiveWeaponDataCompatible && bActiveWeaponUsesProjectileMode && ActiveProjectileData->HasProjectileActorClass())
		{
			bActiveProjectileSpawnReady = true;
			ActiveProjectileExecutionSummary = TEXT("ProjectileExecution: ReadyToPoolAcquire");
		}
		else if (bActiveWeaponDataCompatible && !bActiveWeaponUsesProjectileMode)
		{
			ActiveProjectileExecutionSummary = TEXT("ProjectileExecution: HitScanFireModeDummyHitScanFallback");
		}
		else if (bActiveWeaponDataCompatible)
		{
			ActiveProjectileExecutionSummary = TEXT("ProjectileExecution: ProjectileActorClassMissingDummyHitScanFallback");
		}
	}
}

// [v1.13.0] ProjectileData 단일 소유 기준으로 활성 DamageData를 캐시합니다.
void UCFVehicleWeaponComp::CacheActiveDamageData()
{
	ActiveDamageData = nullptr;
	ActiveDamageId = NAME_None;
	ActiveDamageSummary = TEXT("DamageData: MissingOptional");
	ActiveDamageResolutionSummary = TEXT("DamageResolution: MissingOptional");

	if (!ActiveWeaponData)
	{
		return;
	}

	// [v1.13.0] WeaponData가 현재 마운트와 호환되는지 표시할 문자열입니다.
	const FString CompatibilityText = bActiveWeaponDataCompatible ? TEXT("Compatible") : TEXT("IncompatiblePreview");

	if (!ActiveProjectileData)
	{
		ActiveDamageResolutionSummary = FString::Printf(
			TEXT("DamageResolution: Source=MissingProjectileData, State=%s, DamageId=None, Requirement=WeaponData.DefaultProjectileData"),
			*CompatibilityText);
		return;
	}

	if (ActiveProjectileData && ActiveProjectileData->DefaultDamageData)
	{
		ActiveDamageData = ActiveProjectileData->DefaultDamageData;
		ActiveDamageId = ActiveDamageData->DamageId;
		ActiveDamageSummary = ActiveDamageData->BuildDamageSummary();
		ActiveDamageResolutionSummary = FString::Printf(
			TEXT("DamageResolution: Source=ProjectileData.DefaultDamageData, State=%s, DamageId=%s"),
			*CompatibilityText,
			*ActiveDamageId.ToString());
		return;
	}

	if (!ActiveProjectileData->DamageProfileId.IsNone())
	{
		ActiveDamageId = ActiveProjectileData->DamageProfileId;
		ActiveDamageSummary = FString::Printf(
			TEXT("DamageData: MissingOptional, FallbackDamageProfileId=%s"),
			*ActiveDamageId.ToString());
		ActiveDamageResolutionSummary = FString::Printf(
			TEXT("DamageResolution: Source=ProjectileData.DamageProfileIdFallback, State=%s, DamageId=%s"),
			*CompatibilityText,
			*ActiveDamageId.ToString());
		return;
	}

	ActiveDamageResolutionSummary = FString::Printf(
		TEXT("DamageResolution: Source=ProjectileData.DamageMissing, State=%s, DamageId=None"),
		*CompatibilityText);
}

// [v1.0.0] 발사 원점 계산에 사용할 부모 Transform을 반환합니다.
FTransform UCFVehicleWeaponComp::ResolveMountParentTransform() const
{
	// [v1.0.0] 장착 기준으로 사용할 명시 컴포넌트입니다.
	const USceneComponent* MountParentComponent = FindMountParentComponent();
	if (MountParentComponent)
	{
		return MountParentComponent->GetComponentTransform();
	}

	return OwnerVehiclePawn ? OwnerVehiclePawn->GetActorTransform() : FTransform::Identity;
}

// [v1.0.0] 장착 부모 컴포넌트 이름과 일치하는 SceneComponent를 찾습니다.
USceneComponent* UCFVehicleWeaponComp::FindMountParentComponent() const
{
	if (!OwnerVehiclePawn || MountParentComponentName.IsNone())
	{
		return nullptr;
	}

	// [v1.0.0] Owner Pawn에 등록된 SceneComponent 후보 목록입니다.
	TArray<USceneComponent*> SceneComponents;
	OwnerVehiclePawn->GetComponents<USceneComponent>(SceneComponents);
	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (SceneComponent && SceneComponent->GetFName() == MountParentComponentName)
		{
			return SceneComponent;
		}
	}

	return nullptr;
}
