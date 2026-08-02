// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-07-31
// Description: CarFight 차량 내구도 런타임 컴포넌트
// Scope: VehicleData 최대 내구도 초기화, HitContext 기반 명시 내구도 피해, 기존 Health API 호환과 파괴 이벤트를 제공합니다.
// Changelog:
// - v1.1.0: ApplyIntegrityDamageFromHitContext와 Integrity 명칭 Getter를 추가하고 기존 BaseDamage 경로를 호환 Wrapper로 전환.
// - v1.0.0: 최대/현재 체력, BaseDamage 적용, 자기 피해 차단, 파괴 상태 1회 전환과 BP 이벤트를 추가.
// Migration:
// - 기존 ApplyDamageFromHitContext, TryApplyDamageToActor, Health Getter와 이벤트 이름은 유지한다.
// - CurrentHealth와 MaxHealth는 저장·Blueprint 호환을 위해 이름을 유지하지만 의미상 Vehicle Integrity다.
// - VehicleDefenseComp는 방어 계산 후 남은 피해를 ApplyIntegrityDamageFromHitContext로 전달한다.
// - 기존 BP_CFVehiclePawn 계열은 ACFVehiclePawn의 기본 서브오브젝트로 이 컴포넌트를 자동 상속하므로 BP에 수동으로 추가하지 않는다.
// - VehicleData가 없거나 Durability 설정이 유효하지 않으면 FallbackMaxHealth=100을 사용한다.
// - 파괴 상태는 상태와 이벤트만 제공하며 차량 입력, 물리, 시각 메시를 자동으로 중지하거나 교체하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageTypes.h"
#include "Components/ActorComponent.h"
#include "CFVehicleHealthComp.generated.h"

class UCFVehicleData;

// [v1.0.0] 차량 내구도가 변경됐을 때 이전/현재/최대 값을 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCFVehicleHealthChangedSignature, float, PreviousHealth, float, CurrentHealth, float, MaxHealth);

// [v1.0.0] 차량 내구도에 유효한 피해가 적용됐을 때 적용 피해량과 HitContext를 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCFVehicleDamagedSignature, float, AppliedDamage, FCFDamageHitContext, DamageHitContext);

// [v1.0.0] 차량 내구도가 0 이하로 처음 전환됐을 때 HitContext를 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCFVehicleDestroyedSignature, FCFDamageHitContext, DamageHitContext);

/**
 * 차량의 최대/현재 내구도와 최소 파괴 상태를 소유하는 런타임 컴포넌트입니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFVehicleHealthComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] Tick을 사용하지 않는 차량 내구도 컴포넌트 기본값을 초기화합니다.
	UCFVehicleHealthComp();

	// [v1.0.0] VehicleData의 내구도 설정 또는 안전 기본값으로 최대 내구도를 준비합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleHealth", meta=(DisplayName="차량 내구도 초기화 (Initialize Vehicle Health)", ToolTip="VehicleData.VehicleDurabilityConfig.MaxHealth를 읽어 최대 차량 내구도를 준비합니다. 최초 초기화에서는 현재 내구도를 최대값으로 채우고, 재초기화에서는 현재 내구도를 새 최대값 안으로 제한합니다."))
	bool InitializeFromVehicleData(const UCFVehicleData* InVehicleData);

	// [v1.0.0] 현재 내구도와 파괴 상태를 최대 내구도 기준으로 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleHealth", meta=(DisplayName="차량 내구도 완전 초기화 (Reset Health To Maximum)", ToolTip="현재 차량 내구도를 최대값으로 회복하고 파괴 상태를 해제합니다. 리스폰 또는 PIE 반복 검증용 명시적 초기화 함수입니다."))
	void ResetHealthToMaximum();

	// [v1.1.0] DamageData.BaseDamage를 읽어 기존 직접 피해 결과를 유지하는 호환 Wrapper입니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleHealth", meta=(DisplayName="HitContext 피해 적용 (Apply Damage From Hit Context)", ToolTip="유효한 Blocking Hit과 DamageData.BaseDamage를 차량 내구도에 적용합니다. 기존 Blueprint와 C++ 호출 호환을 유지하는 함수입니다."))
	bool ApplyDamageFromHitContext(const FCFDamageHitContext& InDamageHitContext, FCFDamageApplyResult& OutDamageApplyResult);

	// [v1.1.0] 방어 계산 후 남은 명시 피해량을 차량 내구도에 적용합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleHealth", meta=(DisplayName="명시 차량 내구도 피해 적용 (Apply Integrity Damage From Hit Context)", ToolTip="VehicleDefenseComp가 쉴드와 장갑을 계산한 뒤 남은 피해량을 차량 내구도에 적용합니다. 기존 체력 변경·피격·파괴 이벤트는 이 함수에서 발생합니다."))
	bool ApplyIntegrityDamageFromHitContext(const FCFDamageHitContext& InDamageHitContext, float RequestedIntegrityDamage, FCFDamageApplyResult& OutDamageApplyResult);

	// [v1.0.0] HitContext의 HitActor에서 VehicleHealthComp를 찾아 기존 직접 피해 적용 경로를 실행합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleHealth", meta=(DisplayName="피격 Actor에 피해 적용 (Try Apply Damage To Actor)", ToolTip="HitContext.HitActor에서 VehicleHealthComp를 찾아 기존 BaseDamage 직접 피해 경로를 실행합니다. VehicleDefenseComp 정식 통합 전과 Legacy Fallback을 위해 유지합니다."))
	static bool TryApplyDamageToActor(const FCFDamageHitContext& InDamageHitContext, FCFDamageApplyResult& OutDamageApplyResult);

	// [v1.0.0] VehicleDebug와 로그에서 사용할 피해 적용 결과 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="피해 적용 결과 요약 생성 (Build Damage Apply Result Summary)", ToolTip="피해 적용 여부, 거부 사유, 적용량, 차량 내구도 변화와 파괴 전환을 한글 요약 문자열로 생성합니다."))
	static FString BuildDamageApplyResultSummary(const FCFDamageApplyResult& InDamageApplyResult);

	// [v1.0.0] 현재 차량의 최대 내구도를 기존 Health 이름으로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="최대 체력 반환 (Get Max Health)", ToolTip="기존 Blueprint 호환 이름으로 현재 차량의 최대 내구도를 반환합니다."))
	float GetMaxHealth() const { return MaxHealth; }

	// [v1.0.0] 현재 차량의 남은 내구도를 기존 Health 이름으로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="현재 체력 반환 (Get Current Health)", ToolTip="기존 Blueprint 호환 이름으로 현재 차량의 남은 내구도를 반환합니다."))
	float GetCurrentHealth() const { return CurrentHealth; }

	// [v1.0.0] 현재 내구도를 최대 내구도로 나눈 0~1 비율을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="체력 비율 반환 (Get Health Ratio)", ToolTip="기존 Blueprint 호환 이름으로 현재 차량 내구도 비율을 반환합니다."))
	float GetHealthRatio() const;

	// [v1.1.0] 현재 차량의 최대 내구도를 명시적 Integrity 이름으로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="최대 차량 내구도 반환 (Get Maximum Integrity)", ToolTip="현재 차량의 최대 Vehicle Integrity 값을 반환합니다."))
	float GetMaximumIntegrity() const { return MaxHealth; }

	// [v1.1.0] 현재 차량의 남은 내구도를 명시적 Integrity 이름으로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="현재 차량 내구도 반환 (Get Current Integrity)", ToolTip="현재 차량에 남아 있는 Vehicle Integrity 값을 반환합니다."))
	float GetCurrentIntegrity() const { return CurrentHealth; }

	// [v1.1.0] 현재 차량 내구도의 0~1 비율을 명시적 Integrity 이름으로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="차량 내구도 비율 반환 (Get Integrity Ratio)", ToolTip="현재 Vehicle Integrity를 최대 Vehicle Integrity로 나눈 0~1 범위 비율을 반환합니다."))
	float GetIntegrityRatio() const { return GetHealthRatio(); }

	// [v1.0.0] 차량 내구도 초기화가 한 번 이상 완료됐는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="체력 준비 여부 (Is Health Initialized)", ToolTip="VehicleData 또는 안전 기본값으로 최대/현재 차량 내구도가 준비됐는지 반환합니다."))
	bool IsHealthInitialized() const { return bHealthInitialized; }

	// [v1.0.0] 현재 차량이 파괴 상태인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleHealth", meta=(DisplayName="차량 파괴 여부 (Is Destroyed)", ToolTip="현재 차량 내구도가 0 이하로 내려가 파괴 상태로 전환됐는지 반환합니다."))
	bool IsDestroyed() const { return bDestroyed; }

	// [v1.0.0] 현재 차량 내구도가 변경될 때 호출되는 기존 BP 할당 가능 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|VehicleHealth|Events", meta=(DisplayName="체력 변경 이벤트 (OnVehicleHealthChanged)", ToolTip="피해 적용 또는 명시적 초기화로 현재 차량 내구도가 변경될 때 호출됩니다. 기존 Blueprint 호환을 위해 이벤트 이름을 유지합니다."))
	FCFVehicleHealthChangedSignature OnVehicleHealthChanged;

	// [v1.0.0] 유효한 피해가 실제 차량 내구도에 적용될 때 호출되는 기존 BP 할당 가능 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|VehicleHealth|Events", meta=(DisplayName="차량 피격 이벤트 (OnVehicleDamaged)", ToolTip="쉴드와 장갑을 통과한 피해가 차량 내구도에 실제 적용될 때 적용 피해량과 HitContext를 전달합니다."))
	FCFVehicleDamagedSignature OnVehicleDamaged;

	// [v1.0.0] 차량이 처음 파괴 상태로 전환될 때 호출되는 기존 BP 할당 가능 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|VehicleHealth|Events", meta=(DisplayName="차량 파괴 이벤트 (OnVehicleDestroyed)", ToolTip="현재 차량 내구도가 처음 0 이하로 내려갈 때 한 번 호출됩니다. 차량 정지나 파괴 연출은 BP에서 선택적으로 연결합니다."))
	FCFVehicleDestroyedSignature OnVehicleDestroyed;

private:
	// [v1.0.0] VehicleData가 없거나 최대 내구도 값이 유효하지 않을 때 사용할 안전 기본 최대 내구도입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehicleHealth|Fallback", meta=(AllowPrivateAccess="true", ClampMin="1.0", DisplayName="기본 최대 체력 (FallbackMaxHealth)", ToolTip="VehicleData가 없거나 VehicleDurabilityConfig.MaxHealth가 1보다 작을 때 사용할 안전 기본 최대 차량 내구도입니다."))
	float FallbackMaxHealth = 100.0f;

	// [v1.0.0] 현재 차량의 최대 내구도입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleHealth|Runtime", meta=(AllowPrivateAccess="true", DisplayName="최대 체력 (MaxHealth)", ToolTip="VehicleData 또는 안전 기본값으로 초기화된 최대 Vehicle Integrity입니다. 기존 직렬화 이름을 유지합니다."))
	float MaxHealth = 100.0f;

	// [v1.0.0] 현재 차량에 남아 있는 내구도입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleHealth|Runtime", meta=(AllowPrivateAccess="true", DisplayName="현재 체력 (CurrentHealth)", ToolTip="쉴드와 장갑을 통과한 피해가 적용될 때 감소하는 현재 Vehicle Integrity입니다. 기존 직렬화 이름을 유지합니다."))
	float CurrentHealth = 100.0f;

	// [v1.0.0] 최대/현재 내구도 초기화가 완료됐는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleHealth|Runtime", meta=(AllowPrivateAccess="true", DisplayName="체력 준비 여부 (bHealthInitialized)", ToolTip="VehicleData 또는 안전 기본값으로 차량 내구도가 준비됐는지 여부입니다."))
	bool bHealthInitialized = false;

	// [v1.0.0] 현재 차량 내구도가 0 이하로 내려가 파괴 상태가 됐는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleHealth|Runtime", meta=(AllowPrivateAccess="true", DisplayName="차량 파괴 여부 (bDestroyed)", ToolTip="현재 차량 내구도가 0 이하로 내려가 파괴 상태로 한 번 전환됐는지 여부입니다."))
	bool bDestroyed = false;
};
