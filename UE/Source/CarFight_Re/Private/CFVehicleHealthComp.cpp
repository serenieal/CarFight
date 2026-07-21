// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-14
// Description: CarFight 차량 체력 런타임 컴포넌트 구현
// Scope: VehicleData 최대 체력 초기화, DamageHitContext 기반 직접 피해 적용, 파괴 상태와 BP 이벤트를 제공합니다.
// Changelog:
// - v1.0.0: 최대/현재 체력 초기화, BaseDamage 적용, 자기 피해 차단, 파괴 상태 1회 전환과 결과 요약을 구현.
// Migration:
// - 기존 차량은 VehicleData.VehicleDurabilityConfig.MaxHealth의 C++ 기본값 100을 사용한다.
// - DamageData가 비어 있거나 BaseDamage가 0 이하이면 피해를 적용하지 않고 명시적인 RejectReason을 반환한다.
// - 파괴 상태는 이벤트만 발생시키며 차량 입력, 물리, 렌더링을 자동으로 변경하지 않는다.

#include "CFVehicleHealthComp.h"

#include "CFDamageData.h"
#include "CFVehicleData.h"

#include "GameFramework/Actor.h"

namespace
{
	// [v1.0.0] 피해 적용 거부 사유를 한글 표시 문자열로 변환합니다.
	FString BuildDamageRejectReasonText(const ECFDamageApplyRejectReason RejectReason)
	{
		switch (RejectReason)
		{
		case ECFDamageApplyRejectReason::None:
			return TEXT("없음");
		case ECFDamageApplyRejectReason::NoBlockingHit:
			return TEXT("Blocking Hit 없음");
		case ECFDamageApplyRejectReason::MissingHitActor:
			return TEXT("피격 Actor 없음");
		case ECFDamageApplyRejectReason::MissingDamageData:
			return TEXT("DamageData 없음");
		case ECFDamageApplyRejectReason::NonPositiveDamage:
			return TEXT("피해량 0 이하");
		case ECFDamageApplyRejectReason::SelfDamageBlocked:
			return TEXT("자기 피해 금지");
		case ECFDamageApplyRejectReason::MissingHealthComponent:
			return TEXT("VehicleHealthComp 없음");
		case ECFDamageApplyRejectReason::TargetMismatch:
			return TEXT("체력 소유 Actor 불일치");
		case ECFDamageApplyRejectReason::TargetDestroyed:
			return TEXT("이미 파괴된 대상");
		default:
			return TEXT("알 수 없음");
		}
	}
}

// [v1.0.0] Tick을 사용하지 않는 차량 체력 컴포넌트 기본값을 초기화합니다.
UCFVehicleHealthComp::UCFVehicleHealthComp()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// [v1.0.0] VehicleData의 내구도 설정 또는 안전 기본값으로 최대 체력을 준비합니다.
bool UCFVehicleHealthComp::InitializeFromVehicleData(const UCFVehicleData* InVehicleData)
{
	// [v1.0.0] VehicleData가 제공하는 최대 체력 후보값입니다.
	const float VehicleDataMaxHealth = InVehicleData ? InVehicleData->VehicleDurabilityConfig.MaxHealth : 0.0f;

	// [v1.0.0] VehicleData 값이 유효하지 않을 때 사용할 보정된 안전 기본 최대 체력입니다.
	const float SafeFallbackMaxHealth = FMath::Max(FallbackMaxHealth, 1.0f);

	// [v1.0.0] 이번 초기화에서 적용할 최종 최대 체력입니다.
	const float ResolvedMaxHealth = VehicleDataMaxHealth >= 1.0f ? VehicleDataMaxHealth : SafeFallbackMaxHealth;

	MaxHealth = ResolvedMaxHealth;

	if (!bHealthInitialized)
	{
		CurrentHealth = MaxHealth;
		bDestroyed = false;
	}
	else
	{
		CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
		bDestroyed = CurrentHealth <= 0.0f;
	}

	bHealthInitialized = true;
	return true;
}

// [v1.0.0] 현재 체력과 파괴 상태를 최대 체력 기준으로 초기화합니다.
void UCFVehicleHealthComp::ResetHealthToMaximum()
{
	// [v1.0.0] 체력 초기화 이벤트에 전달할 초기화 전 현재 체력입니다.
	const float PreviousHealth = CurrentHealth;

	MaxHealth = FMath::Max(MaxHealth, FMath::Max(FallbackMaxHealth, 1.0f));
	CurrentHealth = MaxHealth;
	bHealthInitialized = true;
	bDestroyed = false;

	if (!FMath::IsNearlyEqual(PreviousHealth, CurrentHealth))
	{
		OnVehicleHealthChanged.Broadcast(PreviousHealth, CurrentHealth, MaxHealth);
	}
}

// [v1.0.0] 이 컴포넌트의 소유 차량에 DamageHitContext의 직접 피해를 적용합니다.
bool UCFVehicleHealthComp::ApplyDamageFromHitContext(
	const FCFDamageHitContext& InDamageHitContext,
	FCFDamageApplyResult& OutDamageApplyResult)
{
	OutDamageApplyResult = FCFDamageApplyResult();
	OutDamageApplyResult.TargetActor = InDamageHitContext.HitActor;
	OutDamageApplyResult.DamageId = InDamageHitContext.DamageData
		? InDamageHitContext.DamageData->DamageId
		: InDamageHitContext.DamageId;
	OutDamageApplyResult.RequestedDamage = InDamageHitContext.DamageData
		? FMath::Max(InDamageHitContext.DamageData->BaseDamage, 0.0f)
		: 0.0f;
	OutDamageApplyResult.HealthBefore = CurrentHealth;
	OutDamageApplyResult.HealthAfter = CurrentHealth;

	if (!InDamageHitContext.bBlockingHit)
	{
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::NoBlockingHit;
		return false;
	}

	if (!InDamageHitContext.HitActor)
	{
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::MissingHitActor;
		return false;
	}

	if (!InDamageHitContext.DamageData)
	{
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::MissingDamageData;
		return false;
	}

	if (InDamageHitContext.DamageData->BaseDamage <= 0.0f)
	{
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::NonPositiveDamage;
		return false;
	}

	if (InDamageHitContext.InstigatorActor
		&& InDamageHitContext.InstigatorActor == InDamageHitContext.HitActor
		&& !InDamageHitContext.DamageData->bCanDamageSelf)
	{
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::SelfDamageBlocked;
		return false;
	}

	// [v1.0.0] 이 체력 컴포넌트를 실제로 소유한 Actor입니다.
	AActor* HealthOwnerActor = GetOwner();
	if (!HealthOwnerActor || HealthOwnerActor != InDamageHitContext.HitActor)
	{
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::TargetMismatch;
		return false;
	}

	if (!bHealthInitialized)
	{
		InitializeFromVehicleData(nullptr);
		OutDamageApplyResult.HealthBefore = CurrentHealth;
		OutDamageApplyResult.HealthAfter = CurrentHealth;
	}

	if (bDestroyed || CurrentHealth <= 0.0f)
	{
		bDestroyed = true;
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::TargetDestroyed;
		return false;
	}

	// [v1.0.0] 실제 체력에서 차감할 0 이상 직접 피해량입니다.
	const float RequestedDamage = FMath::Max(InDamageHitContext.DamageData->BaseDamage, 0.0f);

	// [v1.0.0] 피해 적용 직전 현재 체력입니다.
	const float PreviousHealth = CurrentHealth;

	CurrentHealth = FMath::Clamp(CurrentHealth - RequestedDamage, 0.0f, MaxHealth);

	// [v1.0.0] 현재 체력에서 실제로 감소한 피해량입니다.
	const float AppliedDamage = FMath::Max(PreviousHealth - CurrentHealth, 0.0f);

	OutDamageApplyResult.bApplied = AppliedDamage > 0.0f;
	OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::None;
	OutDamageApplyResult.RequestedDamage = RequestedDamage;
	OutDamageApplyResult.AppliedDamage = AppliedDamage;
	OutDamageApplyResult.HealthBefore = PreviousHealth;
	OutDamageApplyResult.HealthAfter = CurrentHealth;

	if (!OutDamageApplyResult.bApplied)
	{
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::NonPositiveDamage;
		return false;
	}

	OnVehicleHealthChanged.Broadcast(PreviousHealth, CurrentHealth, MaxHealth);
	OnVehicleDamaged.Broadcast(AppliedDamage, InDamageHitContext);

	if (!bDestroyed && CurrentHealth <= 0.0f)
	{
		bDestroyed = true;
		OutDamageApplyResult.bDestroyedThisHit = true;
		OnVehicleDestroyed.Broadcast(InDamageHitContext);
	}

	return true;
}

// [v1.0.0] HitContext의 HitActor에서 VehicleHealthComp를 찾아 공통 피해 적용 경로를 실행합니다.
bool UCFVehicleHealthComp::TryApplyDamageToActor(
	const FCFDamageHitContext& InDamageHitContext,
	FCFDamageApplyResult& OutDamageApplyResult)
{
	OutDamageApplyResult = FCFDamageApplyResult();
	OutDamageApplyResult.TargetActor = InDamageHitContext.HitActor;
	OutDamageApplyResult.DamageId = InDamageHitContext.DamageData
		? InDamageHitContext.DamageData->DamageId
		: InDamageHitContext.DamageId;
	OutDamageApplyResult.RequestedDamage = InDamageHitContext.DamageData
		? FMath::Max(InDamageHitContext.DamageData->BaseDamage, 0.0f)
		: 0.0f;

	if (!InDamageHitContext.bBlockingHit)
	{
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::NoBlockingHit;
		return false;
	}

	if (!InDamageHitContext.HitActor)
	{
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::MissingHitActor;
		return false;
	}

	if (!InDamageHitContext.DamageData)
	{
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::MissingDamageData;
		return false;
	}

	if (InDamageHitContext.DamageData->BaseDamage <= 0.0f)
	{
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::NonPositiveDamage;
		return false;
	}

	if (InDamageHitContext.InstigatorActor
		&& InDamageHitContext.InstigatorActor == InDamageHitContext.HitActor
		&& !InDamageHitContext.DamageData->bCanDamageSelf)
	{
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::SelfDamageBlocked;
		return false;
	}

	// [v1.0.0] 실제 피격 Actor에서 찾은 차량 체력 컴포넌트입니다.
	UCFVehicleHealthComp* TargetHealthComponent = InDamageHitContext.HitActor->FindComponentByClass<UCFVehicleHealthComp>();
	if (!TargetHealthComponent)
	{
		OutDamageApplyResult.RejectReason = ECFDamageApplyRejectReason::MissingHealthComponent;
		return false;
	}

	return TargetHealthComponent->ApplyDamageFromHitContext(InDamageHitContext, OutDamageApplyResult);
}

// [v1.0.0] VehicleDebug와 로그에서 사용할 피해 적용 결과 요약 문자열을 생성합니다.
FString UCFVehicleHealthComp::BuildDamageApplyResultSummary(const FCFDamageApplyResult& InDamageApplyResult)
{
	// [v1.0.0] 피해가 실제 체력에 적용됐는지 표시할 한글 문자열입니다.
	const FString AppliedText = InDamageApplyResult.bApplied ? TEXT("예") : TEXT("아니오");

	// [v1.0.0] 이번 피해로 파괴 상태가 처음 발생했는지 표시할 한글 문자열입니다.
	const FString DestroyedText = InDamageApplyResult.bDestroyedThisHit ? TEXT("예") : TEXT("아니오");

	// [v1.0.0] 피해 적용 거부 사유의 한글 표시 문자열입니다.
	const FString RejectReasonText = BuildDamageRejectReasonText(InDamageApplyResult.RejectReason);

	// [v1.0.0] 피해 적용 대상 Actor 이름입니다.
	const FString TargetActorName = InDamageApplyResult.TargetActor
		? InDamageApplyResult.TargetActor->GetName()
		: TEXT("없음");

	return FString::Printf(
		TEXT("피해 적용: 적용=%s, 거부 사유=%s, 대상=%s, 피해 ID=%s, 요청=%.1f, 실제=%.1f, 체력=%.1f→%.1f, 이번 타격 파괴=%s"),
		*AppliedText,
		*RejectReasonText,
		*TargetActorName,
		*InDamageApplyResult.DamageId.ToString(),
		InDamageApplyResult.RequestedDamage,
		InDamageApplyResult.AppliedDamage,
		InDamageApplyResult.HealthBefore,
		InDamageApplyResult.HealthAfter,
		*DestroyedText);
}

// [v1.0.0] 현재 체력을 최대 체력으로 나눈 0~1 비율을 반환합니다.
float UCFVehicleHealthComp::GetHealthRatio() const
{
	return MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
}
