// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-07-31
// Description: CarFight 차량 쉴드·6방향 장갑 런타임 컴포넌트 구현
// Scope: 방어 데이터 초기화, 방향 판정, 쉴드·장갑·관통·내구도 피해 분배, Legacy Fallback, 재생과 기존 결과 호환을 구현합니다.
// Changelog:
// - v1.2.0: 실제 방어층 피해의 마지막 전체 결과를 캐시하고 초기화·Reset 수명과 Blueprint Debug 조회 계약을 구현.
// - v1.1.0: DR-P0-03 HitScan·Projectile 통합용 Integrity 호환 결과 변환을 구현.
// - v1.0.0: CF-FQ-033 DR-P0-02 VehicleDefenseComp 전체 Foundation 구현.
// Migration:
// - Projectile·HitScan은 TryApplyDamageToActor의 FCFVehicleDamageResult를 정식 결과로 사용한다.
// - DefenseComp가 없거나 ActiveDefenseData가 None이면 기존 VehicleHealthComp BaseDamage 적용을 유지한다.
// - 기존 Debug와 Pool에는 BuildIntegrityCompatibilityResult 결과를 전달한다.
// - 부품 피해와 물리 충격 결과는 DR-P1 전까지 0과 빈 배열을 유지한다.

#include "CFVehicleDefenseComp.h"

#include "CFDamageData.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseData.h"
#include "CFVehicleHealthComp.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

namespace
{
	// [v1.0.0] HitContext 공통 입력을 검증하고 기존 RejectReason을 반환합니다.
	bool ValidateDamageHitContext(
		const FCFDamageHitContext& InDamageHitContext,
		ECFDamageApplyRejectReason& OutRejectReason)
	{
		OutRejectReason = ECFDamageApplyRejectReason::None;

		if (!InDamageHitContext.bBlockingHit)
		{
			OutRejectReason = ECFDamageApplyRejectReason::NoBlockingHit;
			return false;
		}

		if (!InDamageHitContext.HitActor)
		{
			OutRejectReason = ECFDamageApplyRejectReason::MissingHitActor;
			return false;
		}

		if (!InDamageHitContext.DamageData)
		{
			OutRejectReason = ECFDamageApplyRejectReason::MissingDamageData;
			return false;
		}

		if (InDamageHitContext.DamageData->BaseDamage <= 0.0f)
		{
			OutRejectReason = ECFDamageApplyRejectReason::NonPositiveDamage;
			return false;
		}

		if (InDamageHitContext.InstigatorActor
			&& InDamageHitContext.InstigatorActor == InDamageHitContext.HitActor
			&& !InDamageHitContext.DamageData->bCanDamageSelf)
		{
			OutRejectReason = ECFDamageApplyRejectReason::SelfDamageBlocked;
			return false;
		}

		return true;
	}

	// [v1.0.0] HitContext에서 공통 식별값과 원본 피해량을 결과 구조에 기록합니다.
	void InitializeVehicleDamageResult(
		const FCFDamageHitContext& InDamageHitContext,
		FCFVehicleDamageResult& OutVehicleDamageResult)
	{
		OutVehicleDamageResult = FCFVehicleDamageResult();
		OutVehicleDamageResult.TargetActor = InDamageHitContext.HitActor;
		OutVehicleDamageResult.DamageId = InDamageHitContext.DamageData
			? InDamageHitContext.DamageData->DamageId
			: InDamageHitContext.DamageId;
		OutVehicleDamageResult.RequestedDamage = InDamageHitContext.DamageData
			? FMath::Max(InDamageHitContext.DamageData->BaseDamage, 0.0f)
			: 0.0f;
	}

	// [v1.0.0] 기존 Health 직접 피해 결과를 전체 VehicleDamageResult Legacy Fallback 형식으로 변환합니다.
	void PopulateLegacyFallbackResult(
		const FCFDamageHitContext& InDamageHitContext,
		const FCFDamageApplyResult& InDamageApplyResult,
		FCFVehicleDamageResult& OutVehicleDamageResult)
	{
		InitializeVehicleDamageResult(InDamageHitContext, OutVehicleDamageResult);
		OutVehicleDamageResult.bDamageAccepted = InDamageApplyResult.RejectReason == ECFDamageApplyRejectReason::None;
		OutVehicleDamageResult.bAppliedToAnyLayer = InDamageApplyResult.bApplied;
		OutVehicleDamageResult.bUsedLegacyHealthFallback = true;
		OutVehicleDamageResult.RejectReason = InDamageApplyResult.RejectReason;
		OutVehicleDamageResult.DamageAfterShield = OutVehicleDamageResult.RequestedDamage;
		OutVehicleDamageResult.DirectionDamageMultiplier = 1.0f;
		OutVehicleDamageResult.DirectionalDamage = OutVehicleDamageResult.RequestedDamage;
		OutVehicleDamageResult.DamageRequestedForIntegrity = InDamageApplyResult.RequestedDamage;
		OutVehicleDamageResult.DamageAppliedToIntegrity = InDamageApplyResult.AppliedDamage;
		OutVehicleDamageResult.IntegrityOverkillDamage = FMath::Max(
			InDamageApplyResult.RequestedDamage - InDamageApplyResult.AppliedDamage,
			0.0f);
		OutVehicleDamageResult.IntegrityBefore = InDamageApplyResult.HealthBefore;
		OutVehicleDamageResult.IntegrityAfter = InDamageApplyResult.HealthAfter;
		OutVehicleDamageResult.bDestroyedThisHit = InDamageApplyResult.bDestroyedThisHit;
		OutVehicleDamageResult.IntegrityApplyResult = InDamageApplyResult;
	}
}

// [v1.0.0] 필요할 때만 Tick을 켜는 차량 방어 컴포넌트 기본값을 초기화합니다.
UCFVehicleDefenseComp::UCFVehicleDefenseComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// [v1.0.0] 재생 지연과 실제 쉴드 재생을 진행합니다.
void UCFVehicleDefenseComp::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	AdvanceShieldRegeneration(DeltaTime);
}

// [v1.0.0] VehicleData의 선택적 DefenseData와 같은 Actor의 HealthComp로 방어 런타임을 초기화합니다.
bool UCFVehicleDefenseComp::InitializeFromVehicleData(const UCFVehicleData* InVehicleData)
{
	// [v1.0.0] 같은 Actor에서 기존 차량 내구도를 소유하는 컴포넌트입니다.
	UCFVehicleHealthComp* HealthComponent = ResolveBoundHealthComponent();
	if (HealthComponent && !HealthComponent->IsHealthInitialized())
	{
		HealthComponent->InitializeFromVehicleData(InVehicleData);
	}

	// [v1.0.0] VehicleData가 제공하는 선택적 차량 방어 데이터입니다.
	UCFVehicleDefenseData* DefenseData = InVehicleData ? InVehicleData->DefaultDefenseData : nullptr;
	return InitializeFromDefenseData(DefenseData);
}

// [v1.0.0] 지정 DefenseData와 같은 Actor의 HealthComp로 방어 런타임을 직접 초기화합니다.
bool UCFVehicleDefenseComp::InitializeFromDefenseData(UCFVehicleDefenseData* InDefenseData)
{
	// [v1.2.0] 새 방어 데이터 초기화는 이전 전투의 마지막 전체 피해 결과를 이어받지 않습니다.
	ClearLastVehicleDamageResult();
	ResolveBoundHealthComponent();

	if (!InDefenseData)
	{
		ActiveDefenseData = nullptr;
		MaximumShield = 0.0f;
		CurrentShield = 0.0f;
		FrontArmor = 0.0f;
		LeftArmor = 0.0f;
		RightArmor = 0.0f;
		RearArmor = 0.0f;
		TopArmor = 0.0f;
		BottomArmor = 0.0f;
		bDefenseInitialized = false;
		StopShieldRegeneration();
		return false;
	}

	// [v1.0.0] 같은 DefenseData 재초기화에서 현재 손상 상태를 최대값 안으로 보존할지 여부입니다.
	const bool bPreserveCurrentDefense = bDefenseInitialized && ActiveDefenseData == InDefenseData;

	ActiveDefenseData = InDefenseData;
	MaximumShield = ActiveDefenseData->GetEffectiveMaximumShield();

	// [v1.0.0] 현재 DefenseData의 정면 장갑 최대값입니다.
	const float MaximumFrontArmor = GetMaximumArmor(ECFArmorDirection::Front);

	// [v1.0.0] 현재 DefenseData의 좌측 장갑 최대값입니다.
	const float MaximumLeftArmor = GetMaximumArmor(ECFArmorDirection::Left);

	// [v1.0.0] 현재 DefenseData의 우측 장갑 최대값입니다.
	const float MaximumRightArmor = GetMaximumArmor(ECFArmorDirection::Right);

	// [v1.0.0] 현재 DefenseData의 후면 장갑 최대값입니다.
	const float MaximumRearArmor = GetMaximumArmor(ECFArmorDirection::Rear);

	// [v1.0.0] 현재 DefenseData의 상부 장갑 최대값입니다.
	const float MaximumTopArmor = GetMaximumArmor(ECFArmorDirection::Top);

	// [v1.0.0] 현재 DefenseData의 하부 장갑 최대값입니다.
	const float MaximumBottomArmor = GetMaximumArmor(ECFArmorDirection::Bottom);

	if (bPreserveCurrentDefense)
	{
		CurrentShield = FMath::Clamp(CurrentShield, 0.0f, MaximumShield);
		FrontArmor = FMath::Clamp(FrontArmor, 0.0f, MaximumFrontArmor);
		LeftArmor = FMath::Clamp(LeftArmor, 0.0f, MaximumLeftArmor);
		RightArmor = FMath::Clamp(RightArmor, 0.0f, MaximumRightArmor);
		RearArmor = FMath::Clamp(RearArmor, 0.0f, MaximumRearArmor);
		TopArmor = FMath::Clamp(TopArmor, 0.0f, MaximumTopArmor);
		BottomArmor = FMath::Clamp(BottomArmor, 0.0f, MaximumBottomArmor);
	}
	else
	{
		CurrentShield = MaximumShield;
		FrontArmor = MaximumFrontArmor;
		LeftArmor = MaximumLeftArmor;
		RightArmor = MaximumRightArmor;
		RearArmor = MaximumRearArmor;
		TopArmor = MaximumTopArmor;
		BottomArmor = MaximumBottomArmor;
	}

	bDefenseInitialized = true;
	StopShieldRegeneration();
	return true;
}

// [v1.0.0] 현재 쉴드와 6방향 장갑을 설정 최대값으로 초기화합니다.
void UCFVehicleDefenseComp::ResetDefenseToMaximum()
{
	// [v1.2.0] 수동 방어 Reset은 이전 전투 결과를 지워 현재 상태와 Debug 캐시를 함께 초기화합니다.
	ClearLastVehicleDamageResult();
	if (!ActiveDefenseData)
	{
		InitializeFromDefenseData(nullptr);
		return;
	}

	MaximumShield = ActiveDefenseData->GetEffectiveMaximumShield();
	CurrentShield = MaximumShield;
	FrontArmor = GetMaximumArmor(ECFArmorDirection::Front);
	LeftArmor = GetMaximumArmor(ECFArmorDirection::Left);
	RightArmor = GetMaximumArmor(ECFArmorDirection::Right);
	RearArmor = GetMaximumArmor(ECFArmorDirection::Rear);
	TopArmor = GetMaximumArmor(ECFArmorDirection::Top);
	BottomArmor = GetMaximumArmor(ECFArmorDirection::Bottom);
	bDefenseInitialized = true;
	StopShieldRegeneration();
}

// [v1.0.0] 이 컴포넌트의 소유 차량에 쉴드·장갑·내구도 순서로 피해를 분배합니다.
bool UCFVehicleDefenseComp::ApplyDamageFromHitContext(
	const FCFDamageHitContext& InDamageHitContext,
	FCFVehicleDamageResult& OutVehicleDamageResult)
{
	InitializeVehicleDamageResult(InDamageHitContext, OutVehicleDamageResult);

	// [v1.0.0] 공통 입력 검증에서 반환된 피해 적용 거부 사유입니다.
	ECFDamageApplyRejectReason ValidationRejectReason = ECFDamageApplyRejectReason::None;
	if (!ValidateDamageHitContext(InDamageHitContext, ValidationRejectReason))
	{
		OutVehicleDamageResult.RejectReason = ValidationRejectReason;
		return false;
	}

	// [v1.0.0] 이 방어 컴포넌트를 실제로 소유한 Actor입니다.
	AActor* DefenseOwnerActor = GetOwner();
	if (!DefenseOwnerActor || DefenseOwnerActor != InDamageHitContext.HitActor)
	{
		OutVehicleDamageResult.RejectReason = ECFDamageApplyRejectReason::TargetMismatch;
		return false;
	}

	// [v1.0.0] 쉴드와 장갑을 통과한 피해를 받을 차량 내구도 컴포넌트입니다.
	UCFVehicleHealthComp* HealthComponent = ResolveBoundHealthComponent();
	if (!HealthComponent)
	{
		OutVehicleDamageResult.RejectReason = ECFDamageApplyRejectReason::MissingHealthComponent;
		return false;
	}

	if (!HealthComponent->IsHealthInitialized())
	{
		HealthComponent->InitializeFromVehicleData(nullptr);
	}

	if (HealthComponent->IsDestroyed() || HealthComponent->GetCurrentIntegrity() <= 0.0f)
	{
		OutVehicleDamageResult.RejectReason = ECFDamageApplyRejectReason::TargetDestroyed;
		return false;
	}

	if (!bDefenseInitialized || !ActiveDefenseData)
	{
		// [v1.0.0] DefenseData가 없을 때 기존 Health 직접 피해 경로가 반환한 결과입니다.
		FCFDamageApplyResult LegacyDamageApplyResult;

		// [v1.0.0] DefenseData 없는 기존 차량에 BaseDamage를 직접 적용한 결과입니다.
		const bool bLegacyDamageApplied = HealthComponent->ApplyDamageFromHitContext(
			InDamageHitContext,
			LegacyDamageApplyResult);

				PopulateLegacyFallbackResult(InDamageHitContext, LegacyDamageApplyResult, OutVehicleDamageResult);
		if (OutVehicleDamageResult.bAppliedToAnyLayer)
		{
			// [v1.2.0] DefenseData 없는 Legacy Fallback 결과도 이벤트 처리 전에 같은 Debug 캐시에 저장합니다.
			StoreLastVehicleDamageResult(OutVehicleDamageResult);
			OnVehicleDamageResolved.Broadcast(OutVehicleDamageResult);
		}
		return bLegacyDamageApplied;
	}

	OutVehicleDamageResult.bDamageAccepted = true;
	OutVehicleDamageResult.RejectReason = ECFDamageApplyRejectReason::None;
	OutVehicleDamageResult.IntegrityBefore = HealthComponent->GetCurrentIntegrity();
	OutVehicleDamageResult.IntegrityAfter = OutVehicleDamageResult.IntegrityBefore;

	// [v1.0.0] 이번 피해가 적용되기 직전 현재 쉴드입니다.
	const float PreviousShield = CurrentShield;

	OutVehicleDamageResult.ShieldBefore = PreviousShield;
	OutVehicleDamageResult.DamageAbsorbedByShield = FMath::Min(CurrentShield, OutVehicleDamageResult.RequestedDamage);
	CurrentShield = FMath::Max(CurrentShield - OutVehicleDamageResult.DamageAbsorbedByShield, 0.0f);
	OutVehicleDamageResult.ShieldAfter = CurrentShield;
	OutVehicleDamageResult.DamageAfterShield = FMath::Max(
		OutVehicleDamageResult.RequestedDamage - OutVehicleDamageResult.DamageAbsorbedByShield,
		0.0f);
	OutVehicleDamageResult.bShieldBrokenThisHit = PreviousShield > 0.0f && CurrentShield <= 0.0f;

	if (!FMath::IsNearlyEqual(PreviousShield, CurrentShield))
	{
		OnShieldChanged.Broadcast(PreviousShield, CurrentShield, MaximumShield);
	}

	if (OutVehicleDamageResult.DamageAfterShield > 0.0f)
	{
		OutVehicleDamageResult.ArmorDirection = ResolveArmorDirectionFromHitContext(InDamageHitContext);

		// [v1.0.0] 피격 방향의 최대 장갑과 피해 배율을 제공하는 정적 설정입니다.
		const FCFDirectionalArmorConfig DirectionalArmorConfig = ActiveDefenseData->GetEffectiveDirectionalArmorConfig(
			OutVehicleDamageResult.ArmorDirection);

		OutVehicleDamageResult.DirectionDamageMultiplier = DirectionalArmorConfig.GetEffectiveDamageMultiplier();
		OutVehicleDamageResult.DirectionalDamage = OutVehicleDamageResult.DamageAfterShield
			* OutVehicleDamageResult.DirectionDamageMultiplier;
		OutVehicleDamageResult.ArmorPenetration = FMath::Max(InDamageHitContext.DamageData->ArmorPenetration, 0.0f);
		OutVehicleDamageResult.EffectiveArmorResistance = ActiveDefenseData->GetEffectiveArmorResistance();
		OutVehicleDamageResult.ArmorPenetrationRatio = OutVehicleDamageResult.EffectiveArmorResistance <= 0.0f
			? 1.0f
			: FMath::Clamp(
				OutVehicleDamageResult.ArmorPenetration / OutVehicleDamageResult.EffectiveArmorResistance,
				0.0f,
				1.0f);

		// [v1.0.0] 방향 배율 피해 중 장갑 Pool을 우회해 내구도로 직접 전달되는 피해량입니다.
		const float DirectPenetrationDamage = OutVehicleDamageResult.DirectionalDamage
			* OutVehicleDamageResult.ArmorPenetrationRatio;

		// [v1.0.0] 방향 배율 피해 중 장갑이 흡수할 후보 피해량입니다.
		const float ArmorBlockCandidate = FMath::Max(
			OutVehicleDamageResult.DirectionalDamage - DirectPenetrationDamage,
			0.0f);

		// [v1.0.0] 이번 피해가 적용되기 직전 피격 방향 장갑입니다.
		const float PreviousArmor = GetCurrentArmorInternal(OutVehicleDamageResult.ArmorDirection);

		OutVehicleDamageResult.ArmorBefore = PreviousArmor;
		OutVehicleDamageResult.DamageAbsorbedByArmor = FMath::Min(PreviousArmor, ArmorBlockCandidate);

		// [v1.0.0] 피해 흡수 후 피격 방향에 남은 장갑 내구도입니다.
		const float CurrentDirectionalArmor = FMath::Max(
			PreviousArmor - OutVehicleDamageResult.DamageAbsorbedByArmor,
			0.0f);

		SetCurrentArmorInternal(OutVehicleDamageResult.ArmorDirection, CurrentDirectionalArmor);
		OutVehicleDamageResult.ArmorAfter = CurrentDirectionalArmor;
		OutVehicleDamageResult.bArmorBrokenThisHit = PreviousArmor > 0.0f && CurrentDirectionalArmor <= 0.0f;

		// [v1.0.0] 현재 방향 장갑 Pool이 부족해 흡수하지 못한 초과 피해량입니다.
		const float ArmorOverflowDamage = FMath::Max(
			ArmorBlockCandidate - OutVehicleDamageResult.DamageAbsorbedByArmor,
			0.0f);

		OutVehicleDamageResult.DamageRequestedForIntegrity = DirectPenetrationDamage + ArmorOverflowDamage;

		if (!FMath::IsNearlyEqual(PreviousArmor, CurrentDirectionalArmor))
		{
			OnArmorChanged.Broadcast(
				OutVehicleDamageResult.ArmorDirection,
				PreviousArmor,
				CurrentDirectionalArmor,
				DirectionalArmorConfig.GetEffectiveMaximumArmor());
		}

		if (OutVehicleDamageResult.DamageRequestedForIntegrity > 0.0f)
		{
			HealthComponent->ApplyIntegrityDamageFromHitContext(
				InDamageHitContext,
				OutVehicleDamageResult.DamageRequestedForIntegrity,
				OutVehicleDamageResult.IntegrityApplyResult);

			OutVehicleDamageResult.DamageAppliedToIntegrity = OutVehicleDamageResult.IntegrityApplyResult.AppliedDamage;
			OutVehicleDamageResult.IntegrityBefore = OutVehicleDamageResult.IntegrityApplyResult.HealthBefore;
			OutVehicleDamageResult.IntegrityAfter = OutVehicleDamageResult.IntegrityApplyResult.HealthAfter;
			OutVehicleDamageResult.IntegrityOverkillDamage = FMath::Max(
				OutVehicleDamageResult.DamageRequestedForIntegrity - OutVehicleDamageResult.DamageAppliedToIntegrity,
				0.0f);
			OutVehicleDamageResult.bDestroyedThisHit = OutVehicleDamageResult.IntegrityApplyResult.bDestroyedThisHit;
		}
	}

	OutVehicleDamageResult.bAppliedToAnyLayer =
		OutVehicleDamageResult.DamageAbsorbedByShield > 0.0f
		|| OutVehicleDamageResult.DamageAbsorbedByArmor > 0.0f
		|| OutVehicleDamageResult.DamageAppliedToIntegrity > 0.0f;

	if (OutVehicleDamageResult.bShieldBrokenThisHit)
	{
		OnShieldBroken.Broadcast(OutVehicleDamageResult);
	}

	if (OutVehicleDamageResult.bArmorBrokenThisHit)
	{
		OnArmorBroken.Broadcast(OutVehicleDamageResult.ArmorDirection, OutVehicleDamageResult);
	}

		if (OutVehicleDamageResult.bAppliedToAnyLayer)
	{
		RestartShieldRegenerationAfterDamage();

		// [v1.2.0] Blueprint 이벤트 핸들러가 즉시 조회해도 현재 결과를 보도록 Broadcast 전에 캐시합니다.
		StoreLastVehicleDamageResult(OutVehicleDamageResult);
		OnVehicleDamageResolved.Broadcast(OutVehicleDamageResult);
	}

	return OutVehicleDamageResult.bAppliedToAnyLayer;
}

// [v1.0.0] HitActor의 DefenseComp를 우선 사용하고 없으면 기존 HealthComp 직접 피해로 Fallback합니다.
bool UCFVehicleDefenseComp::TryApplyDamageToActor(
	const FCFDamageHitContext& InDamageHitContext,
	FCFVehicleDamageResult& OutVehicleDamageResult)
{
	InitializeVehicleDamageResult(InDamageHitContext, OutVehicleDamageResult);

	// [v1.0.0] 공통 입력 검증에서 반환된 피해 적용 거부 사유입니다.
	ECFDamageApplyRejectReason ValidationRejectReason = ECFDamageApplyRejectReason::None;
	if (!ValidateDamageHitContext(InDamageHitContext, ValidationRejectReason))
	{
		OutVehicleDamageResult.RejectReason = ValidationRejectReason;
		return false;
	}

	// [v1.0.0] 피격 Actor에서 찾은 정식 차량 방어 컴포넌트입니다.
	UCFVehicleDefenseComp* TargetDefenseComponent = InDamageHitContext.HitActor->FindComponentByClass<UCFVehicleDefenseComp>();
	if (TargetDefenseComponent)
	{
		return TargetDefenseComponent->ApplyDamageFromHitContext(InDamageHitContext, OutVehicleDamageResult);
	}

	// [v1.0.0] DefenseComp가 없는 기존 차량에서 찾은 차량 내구도 컴포넌트입니다.
	UCFVehicleHealthComp* TargetHealthComponent = InDamageHitContext.HitActor->FindComponentByClass<UCFVehicleHealthComp>();
	if (!TargetHealthComponent)
	{
		OutVehicleDamageResult.RejectReason = ECFDamageApplyRejectReason::MissingHealthComponent;
		return false;
	}

	// [v1.0.0] 기존 Health 직접 피해 경로가 반환한 결과입니다.
	FCFDamageApplyResult LegacyDamageApplyResult;

	// [v1.0.0] DefenseComp 없는 기존 차량에 BaseDamage를 직접 적용한 결과입니다.
	const bool bLegacyDamageApplied = TargetHealthComponent->ApplyDamageFromHitContext(
		InDamageHitContext,
		LegacyDamageApplyResult);

	PopulateLegacyFallbackResult(InDamageHitContext, LegacyDamageApplyResult, OutVehicleDamageResult);
	return bLegacyDamageApplied;
}

// [v1.1.0] 전체 방어 결과에서 기존 VehicleHealth Debug·Pool이 사용할 Integrity 결과를 생성합니다.
FCFDamageApplyResult UCFVehicleDefenseComp::BuildIntegrityCompatibilityResult(
	const FCFVehicleDamageResult& InVehicleDamageResult)
{
	// [v1.1.0] 실제 VehicleHealthComp 적용이 수행됐거나 Legacy Fallback을 사용한 기존 결과입니다.
	const FCFDamageApplyResult& ExistingIntegrityApplyResult = InVehicleDamageResult.IntegrityApplyResult;
	if (InVehicleDamageResult.bUsedLegacyHealthFallback
		|| ExistingIntegrityApplyResult.TargetActor
		|| ExistingIntegrityApplyResult.bApplied
		|| ExistingIntegrityApplyResult.RejectReason != ECFDamageApplyRejectReason::None)
	{
		return ExistingIntegrityApplyResult;
	}

	// [v1.1.0] 쉴드·장갑에서 피해가 끝난 경우에도 기존 Debug가 차량 내구도 무변화를 정확히 표시할 호환 결과입니다.
	FCFDamageApplyResult CompatibilityResult;
	CompatibilityResult.bApplied = InVehicleDamageResult.DamageAppliedToIntegrity > 0.0f;
	CompatibilityResult.RejectReason = InVehicleDamageResult.RejectReason;
	CompatibilityResult.TargetActor = InVehicleDamageResult.TargetActor;
	CompatibilityResult.DamageId = InVehicleDamageResult.DamageId;
	CompatibilityResult.RequestedDamage = InVehicleDamageResult.DamageRequestedForIntegrity;
	CompatibilityResult.AppliedDamage = InVehicleDamageResult.DamageAppliedToIntegrity;
	CompatibilityResult.HealthBefore = InVehicleDamageResult.IntegrityBefore;
	CompatibilityResult.HealthAfter = InVehicleDamageResult.IntegrityAfter;
	CompatibilityResult.bDestroyedThisHit = InVehicleDamageResult.bDestroyedThisHit;
	return CompatibilityResult;
}

// [v1.0.0] 차량 로컬 오프셋의 지배 축으로 6방향 장갑 방향을 결정합니다.
ECFArmorDirection UCFVehicleDefenseComp::DetermineArmorDirectionFromLocalOffset(const FVector LocalOffset)
{
	if (LocalOffset.IsNearlyZero())
	{
		return ECFArmorDirection::Front;
	}

	// [v1.0.0] 차량 로컬 X축 오프셋 절대값입니다.
	const float AbsoluteX = FMath::Abs(LocalOffset.X);

	// [v1.0.0] 차량 로컬 Y축 오프셋 절대값입니다.
	const float AbsoluteY = FMath::Abs(LocalOffset.Y);

	// [v1.0.0] 차량 로컬 Z축 오프셋 절대값입니다.
	const float AbsoluteZ = FMath::Abs(LocalOffset.Z);

	if (AbsoluteZ > FMath::Max(AbsoluteX, AbsoluteY))
	{
		return LocalOffset.Z >= 0.0f ? ECFArmorDirection::Top : ECFArmorDirection::Bottom;
	}

	if (AbsoluteX >= AbsoluteY)
	{
		return LocalOffset.X >= 0.0f ? ECFArmorDirection::Front : ECFArmorDirection::Rear;
	}

	return LocalOffset.Y >= 0.0f ? ECFArmorDirection::Right : ECFArmorDirection::Left;
}

// [v1.2.0] 실제 Shield, Armor 또는 Integrity 피해가 적용된 전체 결과를 Debug 캐시에 저장합니다.
void UCFVehicleDefenseComp::StoreLastVehicleDamageResult(const FCFVehicleDamageResult& InVehicleDamageResult)
{
	bHasLastVehicleDamageResult = InVehicleDamageResult.bAppliedToAnyLayer;
	LastVehicleDamageResult = InVehicleDamageResult;
	LastVehicleDamageResultSummary = bHasLastVehicleDamageResult
		? BuildVehicleDamageResultSummary(LastVehicleDamageResult)
		: TEXT("차량 방어 피해 기록 없음");
}

// [v1.2.0] 방어 초기화 또는 수동 Reset 시 이전 전체 피해 결과 캐시를 비웁니다.
void UCFVehicleDefenseComp::ClearLastVehicleDamageResult()
{
	bHasLastVehicleDamageResult = false;
	LastVehicleDamageResult = FCFVehicleDamageResult();
	LastVehicleDamageResultSummary = TEXT("차량 방어 피해 기록 없음");
}

// [v1.0.0] 디버그와 로그에서 사용할 현재 방어 상태 요약 문자열을 생성합니다.
FString UCFVehicleDefenseComp::BuildVehicleDefenseSummary() const
{
	return FString::Printf(
		TEXT("차량 방어: 초기화=%s, 데이터=%s, 쉴드=%.1f/%.1f, 장갑[F=%.1f, L=%.1f, R=%.1f, Rear=%.1f, Top=%.1f, Bottom=%.1f], 재생=%s, 지연=%.2f초"),
		bDefenseInitialized ? TEXT("예") : TEXT("아니오"),
		ActiveDefenseData ? *ActiveDefenseData->DefenseId.ToString() : TEXT("LegacyFallback"),
		CurrentShield,
		MaximumShield,
		FrontArmor,
		LeftArmor,
		RightArmor,
		RearArmor,
		TopArmor,
		BottomArmor,
		bShieldRegenerating ? TEXT("진행 중") : TEXT("대기/중지"),
		RemainingShieldRegenerationDelaySeconds);
}

// [v1.0.0] 디버그와 로그에서 사용할 한 번의 방어 피해 결과 요약 문자열을 생성합니다.
FString UCFVehicleDefenseComp::BuildVehicleDamageResultSummary(const FCFVehicleDamageResult& InVehicleDamageResult)
{
	return FString::Printf(
		TEXT("차량 방어 피해: 수락=%s, 적용=%s, Legacy=%s, 방향=%s, 요청=%.1f, 쉴드 흡수=%.1f, 장갑 흡수=%.1f, 내구도 요청/적용=%.1f/%.1f, 내구도=%.1f→%.1f, 파괴=%s"),
		InVehicleDamageResult.bDamageAccepted ? TEXT("예") : TEXT("아니오"),
		InVehicleDamageResult.bAppliedToAnyLayer ? TEXT("예") : TEXT("아니오"),
		InVehicleDamageResult.bUsedLegacyHealthFallback ? TEXT("예") : TEXT("아니오"),
		*UEnum::GetValueAsString(InVehicleDamageResult.ArmorDirection),
		InVehicleDamageResult.RequestedDamage,
		InVehicleDamageResult.DamageAbsorbedByShield,
		InVehicleDamageResult.DamageAbsorbedByArmor,
		InVehicleDamageResult.DamageRequestedForIntegrity,
		InVehicleDamageResult.DamageAppliedToIntegrity,
		InVehicleDamageResult.IntegrityBefore,
		InVehicleDamageResult.IntegrityAfter,
		InVehicleDamageResult.bDestroyedThisHit ? TEXT("예") : TEXT("아니오"));
}

// [v1.0.0] 현재 쉴드의 0~1 비율을 반환합니다.
float UCFVehicleDefenseComp::GetShieldRatio() const
{
	return MaximumShield > 0.0f ? FMath::Clamp(CurrentShield / MaximumShield, 0.0f, 1.0f) : 0.0f;
}

// [v1.0.0] 지정 방향 장갑의 최대 내구도를 반환합니다.
float UCFVehicleDefenseComp::GetMaximumArmor(const ECFArmorDirection ArmorDirection) const
{
	return ActiveDefenseData
		? ActiveDefenseData->GetEffectiveDirectionalArmorConfig(ArmorDirection).GetEffectiveMaximumArmor()
		: 0.0f;
}

// [v1.0.0] 지정 방향 장갑의 현재 내구도를 반환합니다.
float UCFVehicleDefenseComp::GetCurrentArmor(const ECFArmorDirection ArmorDirection) const
{
	return GetCurrentArmorInternal(ArmorDirection);
}

// [v1.0.0] 지정 방향 장갑의 0~1 비율을 반환합니다.
float UCFVehicleDefenseComp::GetArmorRatio(const ECFArmorDirection ArmorDirection) const
{
	// [v1.0.0] 지정 방향 장갑의 유효 최대 내구도입니다.
	const float MaximumDirectionalArmor = GetMaximumArmor(ArmorDirection);
	return MaximumDirectionalArmor > 0.0f
		? FMath::Clamp(GetCurrentArmorInternal(ArmorDirection) / MaximumDirectionalArmor, 0.0f, 1.0f)
		: 0.0f;
}

// [v1.0.0] 같은 Actor의 VehicleHealthComp를 다시 찾고 캐시합니다.
UCFVehicleHealthComp* UCFVehicleDefenseComp::ResolveBoundHealthComponent()
{
	if (BoundHealthComponent && BoundHealthComponent->GetOwner() == GetOwner())
	{
		return BoundHealthComponent;
	}

	BoundHealthComponent = GetOwner() ? GetOwner()->FindComponentByClass<UCFVehicleHealthComp>() : nullptr;
	return BoundHealthComponent;
}

// [v1.0.0] HitContext의 위치, 노멀과 입사 방향으로 피격 장갑 방향을 판정합니다.
ECFArmorDirection UCFVehicleDefenseComp::ResolveArmorDirectionFromHitContext(const FCFDamageHitContext& InDamageHitContext) const
{
	if (!InDamageHitContext.HitActor)
	{
		return ECFArmorDirection::Front;
	}

	// [v1.0.0] 피격 방향 판정에 사용할 컴포넌트 또는 Actor Bounds 중심입니다.
	const FVector DefenseBoundsCenter = ResolveDefenseBoundsCenter(InDamageHitContext);

	// [v1.0.0] 피격 중심에서 실제 피격 위치로 향하는 월드 오프셋입니다.
	const FVector WorldImpactOffset = InDamageHitContext.ImpactLocation - DefenseBoundsCenter;

	// [v1.0.0] 차량 로컬 축 기준으로 변환한 피격 위치 오프셋입니다.
	FVector LocalDirectionSource = InDamageHitContext.HitActor->GetActorTransform().InverseTransformVectorNoScale(WorldImpactOffset);

	if (LocalDirectionSource.IsNearlyZero())
	{
		LocalDirectionSource = InDamageHitContext.HitActor->GetActorTransform().InverseTransformVectorNoScale(
			InDamageHitContext.ImpactNormal);
	}

	if (LocalDirectionSource.IsNearlyZero())
	{
		LocalDirectionSource = InDamageHitContext.HitActor->GetActorTransform().InverseTransformVectorNoScale(
			-InDamageHitContext.IncomingDirection);
	}

	return DetermineArmorDirectionFromLocalOffset(LocalDirectionSource);
}

// [v1.0.0] HitComponentName 또는 Actor Bounds에서 방향 판정 기준 중심을 계산합니다.
FVector UCFVehicleDefenseComp::ResolveDefenseBoundsCenter(const FCFDamageHitContext& InDamageHitContext) const
{
	if (!InDamageHitContext.HitActor)
	{
		return FVector::ZeroVector;
	}

	if (!InDamageHitContext.HitComponentName.IsNone())
	{
		// [v1.0.0] 피격 Actor가 소유한 PrimitiveComponent 목록입니다.
		TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
		InDamageHitContext.HitActor->GetComponents(PrimitiveComponents);

		for (const UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			if (PrimitiveComponent && PrimitiveComponent->GetFName() == InDamageHitContext.HitComponentName)
			{
				return PrimitiveComponent->Bounds.Origin;
			}
		}
	}

	// [v1.0.0] Actor 전체 Bounds에서 계산한 중심입니다.
	FVector ActorBoundsOrigin = InDamageHitContext.HitActor->GetActorLocation();

	// [v1.0.0] Actor 전체 Bounds의 Extent이며 중심 계산 결과 유효성 확인에만 사용합니다.
	FVector ActorBoundsExtent = FVector::ZeroVector;
	InDamageHitContext.HitActor->GetActorBounds(false, ActorBoundsOrigin, ActorBoundsExtent, true);

	return ActorBoundsOrigin.ContainsNaN()
		? InDamageHitContext.HitActor->GetActorLocation()
		: ActorBoundsOrigin;
}

// [v1.0.0] 지정 방향 장갑의 현재값을 내부 상태에서 읽습니다.
float UCFVehicleDefenseComp::GetCurrentArmorInternal(const ECFArmorDirection ArmorDirection) const
{
	switch (ArmorDirection)
	{
	case ECFArmorDirection::Front:
		return FrontArmor;
	case ECFArmorDirection::Left:
		return LeftArmor;
	case ECFArmorDirection::Right:
		return RightArmor;
	case ECFArmorDirection::Rear:
		return RearArmor;
	case ECFArmorDirection::Top:
		return TopArmor;
	case ECFArmorDirection::Bottom:
		return BottomArmor;
	case ECFArmorDirection::None:
	default:
		return 0.0f;
	}
}

// [v1.0.0] 지정 방향 장갑의 현재값을 0 이상으로 저장합니다.
void UCFVehicleDefenseComp::SetCurrentArmorInternal(
	const ECFArmorDirection ArmorDirection,
	const float NewArmorValue)
{
	// [v1.0.0] 지정 방향에 저장할 0 이상의 안전 장갑 값입니다.
	const float SafeArmorValue = FMath::Max(NewArmorValue, 0.0f);

	switch (ArmorDirection)
	{
	case ECFArmorDirection::Front:
		FrontArmor = SafeArmorValue;
		break;
	case ECFArmorDirection::Left:
		LeftArmor = SafeArmorValue;
		break;
	case ECFArmorDirection::Right:
		RightArmor = SafeArmorValue;
		break;
	case ECFArmorDirection::Rear:
		RearArmor = SafeArmorValue;
		break;
	case ECFArmorDirection::Top:
		TopArmor = SafeArmorValue;
		break;
	case ECFArmorDirection::Bottom:
		BottomArmor = SafeArmorValue;
		break;
	case ECFArmorDirection::None:
	default:
		break;
	}
}

// [v1.0.0] 유효 피해 뒤 쉴드 재생 지연을 초기화하고 필요할 때만 Tick을 켭니다.
void UCFVehicleDefenseComp::RestartShieldRegenerationAfterDamage()
{
	if (!ActiveDefenseData
		|| MaximumShield <= 0.0f
		|| CurrentShield >= MaximumShield
		|| ActiveDefenseData->GetEffectiveShieldRegenerationPerSecond() <= 0.0f)
	{
		StopShieldRegeneration();
		return;
	}

	bShieldRegenerating = false;
	RemainingShieldRegenerationDelaySeconds = ActiveDefenseData->GetEffectiveShieldRegenerationDelaySeconds();
	SetComponentTickEnabled(true);
}

// [v1.0.0] 쉴드 재생과 지연 상태를 정리하고 Tick을 끕니다.
void UCFVehicleDefenseComp::StopShieldRegeneration()
{
	bShieldRegenerating = false;
	RemainingShieldRegenerationDelaySeconds = 0.0f;
	SetComponentTickEnabled(false);
}

// [v1.0.0] 한 Tick의 지연 소비와 실제 쉴드 증가를 처리합니다.
void UCFVehicleDefenseComp::AdvanceShieldRegeneration(const float DeltaSeconds)
{
	if (DeltaSeconds <= 0.0f
		|| !IsActive()
		|| !bDefenseInitialized
		|| !ActiveDefenseData
		|| MaximumShield <= 0.0f
		|| CurrentShield >= MaximumShield)
	{
		if (CurrentShield >= MaximumShield || !ActiveDefenseData || !bDefenseInitialized)
		{
			StopShieldRegeneration();
		}
		return;
	}

	// [v1.0.0] 현재 설정에서 사용할 0 이상의 초당 쉴드 재생량입니다.
	const float ShieldRegenerationPerSecond = ActiveDefenseData->GetEffectiveShieldRegenerationPerSecond();
	if (ShieldRegenerationPerSecond <= 0.0f)
	{
		StopShieldRegeneration();
		return;
	}

	// [v1.0.0] 지연 시간을 소비하고 남은 실제 재생 가능 DeltaSeconds입니다.
	float RegenerationDeltaSeconds = DeltaSeconds;

	if (RemainingShieldRegenerationDelaySeconds > 0.0f)
	{
		// [v1.0.0] 이번 Tick에서 소비할 재생 지연 시간입니다.
		const float ConsumedDelaySeconds = FMath::Min(
			RemainingShieldRegenerationDelaySeconds,
			RegenerationDeltaSeconds);

		RemainingShieldRegenerationDelaySeconds = FMath::Max(
			RemainingShieldRegenerationDelaySeconds - ConsumedDelaySeconds,
			0.0f);
		RegenerationDeltaSeconds = FMath::Max(RegenerationDeltaSeconds - ConsumedDelaySeconds, 0.0f);

		if (RemainingShieldRegenerationDelaySeconds > 0.0f || RegenerationDeltaSeconds <= 0.0f)
		{
			return;
		}
	}

	if (!bShieldRegenerating)
	{
		bShieldRegenerating = true;
		OnShieldRegenerationStarted.Broadcast(CurrentShield, MaximumShield);
	}

	// [v1.0.0] 이번 Tick의 쉴드 증가 직전 현재값입니다.
	const float PreviousShield = CurrentShield;

	CurrentShield = FMath::Min(
		CurrentShield + ShieldRegenerationPerSecond * RegenerationDeltaSeconds,
		MaximumShield);

	if (!FMath::IsNearlyEqual(PreviousShield, CurrentShield))
	{
		OnShieldChanged.Broadcast(PreviousShield, CurrentShield, MaximumShield);
	}

	if (CurrentShield >= MaximumShield)
	{
		CurrentShield = MaximumShield;
		OnShieldFullyRestored.Broadcast(CurrentShield, MaximumShield);
		StopShieldRegeneration();
	}
}
