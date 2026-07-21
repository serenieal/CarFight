// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-14
// Description: CarFight 차량 체력 런타임 컴포넌트
// Scope: VehicleData 최대 체력 초기화, DamageHitContext 기반 직접 피해 적용, 파괴 상태와 BP 이벤트를 제공합니다.
// Changelog:
// - v1.0.0: 최대/현재 체력, BaseDamage 적용, 자기 피해 차단, 파괴 상태 1회 전환과 BP 이벤트를 추가.
// Migration:
// - 기존 BP_CFVehiclePawn 계열은 ACFVehiclePawn의 기본 서브오브젝트로 이 컴포넌트를 자동 상속하므로 BP에 수동으로 추가하지 않는다.
// - VehicleData가 없거나 Durability 설정이 유효하지 않으면 FallbackMaxHealth=100을 사용한다.
// - 파괴 상태는 이번 P0에서 상태와 이벤트만 제공하며 차량 입력, 물리, 시각 메시를 자동으로 중지하거나 교체하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageTypes.h"
#include "Components/ActorComponent.h"
#include "CFVehicleHealthComp.generated.h"

class UCFVehicleData;

// [v1.0.0] 차량 체력이 변경됐을 때 이전/현재/최대 체력을 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCFVehicleHealthChangedSignature, float, PreviousHealth, float, CurrentHealth, float, MaxHealth);

// [v1.0.0] 차량이 유효한 직접 피해를 받았을 때 적용 피해량과 HitContext를 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCFVehicleDamagedSignature, float, AppliedDamage, FCFDamageHitContext, DamageHitContext);

// [v1.0.0] 차량 체력이 0 이하로 처음 전환됐을 때 HitContext를 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCFVehicleDestroyedSignature, FCFDamageHitContext, DamageHitContext);

/**
 * 차량의 최대/현재 체력과 최소 파괴 상태를 소유하는 런타임 컴포넌트입니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFVehicleHealthComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] Tick을 사용하지 않는 차량 체력 컴포넌트 기본값을 초기화합니다.
	UCFVehicleHealthComp();

	// [v1.0.0] VehicleData의 내구도 설정 또는 안전 기본값으로 최대 체력을 준비합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleHealth", meta=(DisplayName="차량 체력 초기화 (Initialize Vehicle Health)", ToolTip="VehicleData.VehicleDurabilityConfig.MaxHealth를 읽어 최대 체력을 준비합니다. 최초 초기화에서는 현재 체력을 최대값으로 채우고, 재초기화에서는 현재 체력을 새 최대값 안으로 제한합니다."))
	bool InitializeFromVehicleData(const UCFVehicleData* InVehicleData);

	// [v1.0.0] 현재 체력과 파괴 상태를 최대 체력 기준으로 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleHealth", meta=(DisplayName="차량 체력 완전 초기화 (Reset Health To Maximum)", ToolTip="현재 체력을 최대 체력으로 회복하고 파괴 상태를 해제합니다. 리스폰 또는 PIE 반복 검증용 명시적 초기화 함수입니다."))
	void ResetHealthToMaximum();

	// [v1.0.0] 이 컴포넌트의 소유 차량에 DamageHitContext의 직접 피해를 적용합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleHealth", meta=(DisplayName="HitContext 피해 적용 (Apply Damage From Hit Context)", ToolTip="유효한 Blocking Hit과 DamageData.BaseDamage를 현재 차량 체력에 한 번 적용하고 결과를 반환합니다."))
	bool ApplyDamageFromHitContext(const FCFDamageHitContext& InDamageHitContext, FCFDamageApplyResult& OutDamageApplyResult);

	// [v1.0.0] HitContext의 HitActor에서 VehicleHealthComp를 찾아 공통 피해 적용 경로를 실행합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleHealth", meta=(DisplayName="피격 Actor에 피해 적용 (Try Apply Damage To Actor)", ToolTip="HitContext.HitActor에서 VehicleHealthComp를 찾아 HitScan과 Projectile이 공유하는 직접 피해 적용 경로를 실행합니다."))
	static bool TryApplyDamageToActor(const FCFDamageHitContext& InDamageHitContext, FCFDamageApplyResult& OutDamageApplyResult);

	// [v1.0.0] VehicleDebug와 로그에서 사용할 피해 적용 결과 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="피해 적용 결과 요약 생성 (Build Damage Apply Result Summary)", ToolTip="피해 적용 여부, 거부 사유, 적용량, 체력 변화와 파괴 전환을 한글 요약 문자열로 생성합니다."))
	static FString BuildDamageApplyResultSummary(const FCFDamageApplyResult& InDamageApplyResult);

	// [v1.0.0] 현재 차량의 최대 체력을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="최대 체력 반환 (Get Max Health)", ToolTip="VehicleData 또는 안전 기본값에서 초기화된 최대 체력을 반환합니다."))
	float GetMaxHealth() const { return MaxHealth; }

	// [v1.0.0] 현재 차량의 남은 체력을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="현재 체력 반환 (Get Current Health)", ToolTip="현재 차량에 남아 있는 체력을 반환합니다."))
	float GetCurrentHealth() const { return CurrentHealth; }

	// [v1.0.0] 현재 체력을 최대 체력으로 나눈 0~1 비율을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="체력 비율 반환 (Get Health Ratio)", ToolTip="현재 체력을 최대 체력으로 나눈 0~1 범위의 비율을 반환합니다."))
	float GetHealthRatio() const;

	// [v1.0.0] 체력 초기화가 한 번 이상 완료됐는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="체력 준비 여부 (Is Health Initialized)", ToolTip="VehicleData 또는 안전 기본값으로 최대/현재 체력이 준비됐는지 반환합니다."))
	bool IsHealthInitialized() const { return bHealthInitialized; }

	// [v1.0.0] 현재 차량이 파괴 상태인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="차량 파괴 여부 (Is Destroyed)", ToolTip="현재 체력이 0 이하로 내려가 파괴 상태로 전환됐는지 반환합니다."))
	bool IsDestroyed() const { return bDestroyed; }

	// [v1.0.0] 현재 체력이 변경될 때 호출되는 BP 할당 가능 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|VehicleHealth|Events", meta=(DisplayName="체력 변경 이벤트 (OnVehicleHealthChanged)", ToolTip="피해 적용 또는 명시적 초기화로 현재 체력이 변경될 때 호출됩니다."))
	FCFVehicleHealthChangedSignature OnVehicleHealthChanged;

	// [v1.0.0] 유효한 피해가 실제 체력에 적용될 때 호출되는 BP 할당 가능 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|VehicleHealth|Events", meta=(DisplayName="차량 피격 이벤트 (OnVehicleDamaged)", ToolTip="DamageData.BaseDamage가 현재 체력에 실제 적용될 때 적용 피해량과 HitContext를 전달합니다."))
	FCFVehicleDamagedSignature OnVehicleDamaged;

	// [v1.0.0] 차량이 처음 파괴 상태로 전환될 때 호출되는 BP 할당 가능 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|VehicleHealth|Events", meta=(DisplayName="차량 파괴 이벤트 (OnVehicleDestroyed)", ToolTip="현재 체력이 처음 0 이하로 내려갈 때 한 번 호출됩니다. 차량 정지나 파괴 연출은 BP에서 선택적으로 연결합니다."))
	FCFVehicleDestroyedSignature OnVehicleDestroyed;

private:
	// [v1.0.0] VehicleData가 없거나 최대 체력 값이 유효하지 않을 때 사용할 안전 기본 최대 체력입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehicleHealth|Fallback", meta=(AllowPrivateAccess="true", ClampMin="1.0", DisplayName="기본 최대 체력 (FallbackMaxHealth)", ToolTip="VehicleData가 없거나 VehicleDurabilityConfig.MaxHealth가 1보다 작을 때 사용할 안전 기본 최대 체력입니다."))
	float FallbackMaxHealth = 100.0f;

	// [v1.0.0] 현재 차량의 최대 체력입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleHealth|Runtime", meta=(AllowPrivateAccess="true", DisplayName="최대 체력 (MaxHealth)", ToolTip="VehicleData 또는 안전 기본값으로 초기화된 최대 체력입니다."))
	float MaxHealth = 100.0f;

	// [v1.0.0] 현재 차량에 남아 있는 체력입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleHealth|Runtime", meta=(AllowPrivateAccess="true", DisplayName="현재 체력 (CurrentHealth)", ToolTip="유효한 직접 피해가 적용될 때 감소하는 현재 체력입니다."))
	float CurrentHealth = 100.0f;

	// [v1.0.0] 최대/현재 체력 초기화가 완료됐는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleHealth|Runtime", meta=(AllowPrivateAccess="true", DisplayName="체력 준비 여부 (bHealthInitialized)", ToolTip="VehicleData 또는 안전 기본값으로 체력이 준비됐는지 여부입니다."))
	bool bHealthInitialized = false;

	// [v1.0.0] 현재 체력이 0 이하로 내려가 파괴 상태가 됐는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleHealth|Runtime", meta=(AllowPrivateAccess="true", DisplayName="차량 파괴 여부 (bDestroyed)", ToolTip="현재 체력이 0 이하로 내려가 파괴 상태로 한 번 전환됐는지 여부입니다."))
	bool bDestroyed = false;
};
