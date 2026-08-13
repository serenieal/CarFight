// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-06
// Description: CarFight 차량 쉴드·6방향 장갑 런타임 컴포넌트
// Scope: VehicleDefenseData 초기화, 피해 입력 검증, 쉴드·방향 장갑·관통·내구도 분배, 재생과 기존 Health 결과 호환을 제공합니다.
// Changelog:
// - v1.3.0: 유효 DefenseData 초기화가 방어 컴포넌트 활성 상태를 명시적으로 복구해 실제 Pawn의 Shield 재생 Tick이 진행되도록 보강.
// - v1.2.0: 마지막 전체 차량 방어 피해 결과 캐시와 Blueprint Debug 조회 API를 추가하고 기존 방어 이벤트 노출 계약을 고정.
// - v1.1.0: DR-P0-03 HitScan·Projectile 통합용 Integrity 호환 결과 변환 함수를 추가.
// - v1.0.0: CF-FQ-033 DR-P0-02 VehicleDefenseComp와 Legacy Health Fallback을 추가.
// Migration:
// - 유효 DefenseData 초기화는 VehicleDefenseComp를 활성화하며 초기화 직후 Tick은 계속 꺼진 상태로 유지된다. 실제 피해가 적용된 뒤에만 재생 지연 Tick이 켜진다.
// - ACFVehiclePawn, HitScan과 Projectile은 DR-P0-03부터 TryApplyDamageToActor를 정식 피해 진입점으로 사용한다.
// - VehicleData.DefaultDefenseData가 None이거나 DefenseComp가 없으면 기존 VehicleHealthComp 직접 피해 결과를 유지한다.
// - 기존 Debug와 Pool은 BuildIntegrityCompatibilityResult가 반환하는 FCFDamageApplyResult를 계속 사용할 수 있다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageRuntimeTypes.h"
#include "Components/ActorComponent.h"
#include "CFVehicleDefenseComp.generated.h"

class UCFVehicleData;
class UCFVehicleDefenseData;
class UCFVehicleHealthComp;

// [v1.0.0] 한 번의 유효 방어 피해 분배 결과를 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCFVehicleDamageResolvedSignature, FCFVehicleDamageResult, VehicleDamageResult);

// [v1.0.0] 쉴드 현재값이 변경됐을 때 이전/현재/최대값을 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCFShieldChangedSignature, float, PreviousShield, float, CurrentShield, float, MaximumShield);

// [v1.0.0] 쉴드가 이번 타격으로 처음 0이 됐을 때 전체 피해 결과를 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCFShieldBrokenSignature, FCFVehicleDamageResult, VehicleDamageResult);

// [v1.0.0] 특정 방향 장갑이 변경됐을 때 방향과 이전/현재/최대값을 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FCFArmorChangedSignature, ECFArmorDirection, ArmorDirection, float, PreviousArmor, float, CurrentArmor, float, MaximumArmor);

// [v1.0.0] 특정 방향 장갑이 이번 타격으로 처음 0이 됐을 때 방향과 전체 피해 결과를 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCFArmorBrokenSignature, ECFArmorDirection, ArmorDirection, FCFVehicleDamageResult, VehicleDamageResult);

// [v1.0.0] 지연 종료 후 실제 쉴드 증가가 시작될 때 현재/최대값을 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCFShieldRegenStartedSignature, float, CurrentShield, float, MaximumShield);

// [v1.0.0] 쉴드가 최대값에 도달했을 때 현재/최대값을 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCFShieldFullyRestoredSignature, float, CurrentShield, float, MaximumShield);

/**
 * 차량의 쉴드, 6방향 장갑과 방어 피해 분배를 소유하는 런타임 컴포넌트입니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFVehicleDefenseComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] 필요할 때만 Tick을 켜는 차량 방어 컴포넌트 기본값을 초기화합니다.
	UCFVehicleDefenseComp();

	// [v1.0.0] 재생 지연과 실제 쉴드 재생을 진행합니다.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// [v1.0.0] VehicleData의 선택적 DefenseData와 같은 Actor의 HealthComp로 방어 런타임을 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDefense", meta=(DisplayName="VehicleData에서 차량 방어 초기화 (Initialize Vehicle Defense From Vehicle Data)", ToolTip="VehicleData.DefaultDefenseData를 읽어 쉴드와 6방향 장갑을 초기화합니다. DefenseData가 비어 있으면 Legacy Health Fallback 상태를 유지합니다."))
	bool InitializeFromVehicleData(const UCFVehicleData* InVehicleData);

	// [v1.0.0] 지정 DefenseData와 같은 Actor의 HealthComp로 방어 런타임을 직접 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDefense", meta=(DisplayName="방어 데이터로 차량 방어 초기화 (Initialize Vehicle Defense From Defense Data)", ToolTip="지정한 VehicleDefenseData의 쉴드, 재생과 6방향 장갑 설정으로 현재 런타임을 초기화합니다. 자동 테스트와 명시적 런타임 재설정에 사용합니다."))
	bool InitializeFromDefenseData(UCFVehicleDefenseData* InDefenseData);

	// [v1.0.0] 현재 쉴드와 6방향 장갑을 설정 최대값으로 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDefense", meta=(DisplayName="차량 방어 완전 초기화 (Reset Defense To Maximum)", ToolTip="현재 쉴드와 모든 방향 장갑을 VehicleDefenseData 최대값으로 채우고 재생 상태를 중지합니다."))
	void ResetDefenseToMaximum();

	// [v1.0.0] 이 컴포넌트의 소유 차량에 쉴드·장갑·내구도 순서로 피해를 분배합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDefense", meta=(DisplayName="차량 방어 피해 적용 (Apply Damage From Hit Context)", ToolTip="DamageHitContext를 검증한 뒤 쉴드, 피격 방향 장갑과 Vehicle Integrity 순서로 피해를 분배하고 전체 결과를 반환합니다."))
	bool ApplyDamageFromHitContext(const FCFDamageHitContext& InDamageHitContext, FCFVehicleDamageResult& OutVehicleDamageResult);

	// [v1.0.0] HitActor의 DefenseComp를 우선 사용하고 없으면 기존 HealthComp 직접 피해로 Fallback합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDefense", meta=(DisplayName="피격 Actor에 방어 피해 적용 (Try Apply Damage To Actor)", ToolTip="HitContext.HitActor에서 VehicleDefenseComp를 찾아 정식 방어 피해를 적용합니다. 컴포넌트가 없으면 기존 VehicleHealthComp 직접 피해를 사용합니다."))
	static bool TryApplyDamageToActor(const FCFDamageHitContext& InDamageHitContext, FCFVehicleDamageResult& OutVehicleDamageResult);

		// [v1.1.0] 전체 방어 결과에서 기존 VehicleHealth Debug·Pool이 사용할 Integrity 결과를 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="내구도 호환 결과 생성 (Build Integrity Compatibility Result)", ToolTip="FCFVehicleDamageResult에서 기존 VehicleHealth Debug와 Projectile Pool 복사 경로가 사용할 FCFDamageApplyResult를 생성합니다. 쉴드·장갑만 감소한 경우 차량 내구도는 변화 없음으로 기록합니다."))
	static FCFDamageApplyResult BuildIntegrityCompatibilityResult(const FCFVehicleDamageResult& InVehicleDamageResult);

	// [v1.0.0] 차량 로컬 오프셋의 지배 축으로 6방향 장갑 방향을 결정합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="로컬 오프셋 장갑 방향 판정 (Determine Armor Direction From Local Offset)", ToolTip="차량 로컬 +X 전방, +Y 우측, +Z 상부 기준으로 지배 축을 선택해 Front, Left, Right, Rear, Top, Bottom 중 하나를 반환합니다."))
	static ECFArmorDirection DetermineArmorDirectionFromLocalOffset(FVector LocalOffset);

	// [v1.0.0] 디버그와 로그에서 사용할 현재 방어 상태 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="차량 방어 상태 요약 생성 (Build Vehicle Defense Summary)", ToolTip="방어 초기화, 쉴드, 6방향 장갑, 재생 상태와 지연 시간을 한 줄 문자열로 생성합니다."))
		FString BuildVehicleDefenseSummary() const;

	// [v1.2.0] 마지막으로 실제 방어층에 적용된 전체 차량 피해 결과가 존재하는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense|Debug", meta=(DisplayName="마지막 차량 방어 결과 존재 여부 (Has Last Vehicle Damage Result)", ToolTip="현재 초기화 이후 Shield, Armor 또는 Integrity에 실제 피해가 적용된 마지막 전체 방어 결과가 존재하는지 반환합니다."))
	bool HasLastVehicleDamageResult() const { return bHasLastVehicleDamageResult; }

	// [v1.2.0] 마지막으로 실제 방어층에 적용된 전체 차량 피해 결과를 값으로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense|Debug", meta=(DisplayName="마지막 차량 방어 결과 반환 (Get Last Vehicle Damage Result)", ToolTip="마지막 유효 피해의 Shield, Armor, 관통과 Integrity 전체 결과를 반환합니다. 기록이 없으면 기본값 구조체를 반환합니다."))
	FCFVehicleDamageResult GetLastVehicleDamageResult() const { return LastVehicleDamageResult; }

	// [v1.2.0] 마지막 전체 차량 피해 결과를 VehicleDebug와 Blueprint가 바로 표시할 수 있는 문자열로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense|Debug", meta=(DisplayName="마지막 차량 방어 결과 요약 반환 (Get Last Vehicle Damage Result Summary)", ToolTip="마지막 유효 피해의 방향, Shield, Armor, 관통과 Integrity 결과를 한 줄 요약으로 반환합니다."))
	FString GetLastVehicleDamageResultSummary() const { return LastVehicleDamageResultSummary; }

	// [v1.0.0] 디버그와 로그에서 사용할 한 번의 방어 피해 결과 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="차량 방어 피해 결과 요약 생성 (Build Vehicle Damage Result Summary)", ToolTip="피해 수락, Fallback, 쉴드·장갑·내구도 적용량과 파괴 전환을 한 줄 문자열로 생성합니다."))
	static FString BuildVehicleDamageResultSummary(const FCFVehicleDamageResult& InVehicleDamageResult);

	// [v1.0.0] 현재 사용 중인 정적 방어 데이터를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="활성 차량 방어 데이터 반환 (Get Active Defense Data)", ToolTip="현재 쉴드와 장갑 런타임을 초기화한 VehicleDefenseData를 반환합니다. Legacy Fallback 상태에서는 None입니다."))
	UCFVehicleDefenseData* GetActiveDefenseData() const { return ActiveDefenseData; }

	// [v1.0.0] 쉴드와 방향별 장갑 초기화가 완료됐는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="차량 방어 준비 여부 (Is Defense Initialized)", ToolTip="유효한 VehicleDefenseData로 쉴드와 6방향 장갑 상태가 준비됐는지 반환합니다."))
	bool IsDefenseInitialized() const { return bDefenseInitialized; }

	// [v1.0.0] 현재 차량의 최대 쉴드를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="최대 쉴드 반환 (Get Maximum Shield)", ToolTip="현재 VehicleDefenseData에서 초기화된 최대 쉴드를 반환합니다."))
	float GetMaximumShield() const { return MaximumShield; }

	// [v1.0.0] 현재 차량의 남은 쉴드를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="현재 쉴드 반환 (Get Current Shield)", ToolTip="현재 차량에 남아 있는 쉴드 값을 반환합니다."))
	float GetCurrentShield() const { return CurrentShield; }

	// [v1.0.0] 현재 쉴드의 0~1 비율을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="쉴드 비율 반환 (Get Shield Ratio)", ToolTip="현재 쉴드를 최대 쉴드로 나눈 0~1 범위 비율을 반환합니다."))
	float GetShieldRatio() const;

	// [v1.0.0] 지정 방향 장갑의 최대 내구도를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="방향 최대 장갑 반환 (Get Maximum Armor)", ToolTip="지정한 Front, Left, Right, Rear, Top 또는 Bottom 방향의 최대 장갑 내구도를 반환합니다."))
	float GetMaximumArmor(ECFArmorDirection ArmorDirection) const;

	// [v1.0.0] 지정 방향 장갑의 현재 내구도를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="방향 현재 장갑 반환 (Get Current Armor)", ToolTip="지정한 Front, Left, Right, Rear, Top 또는 Bottom 방향의 현재 장갑 내구도를 반환합니다."))
	float GetCurrentArmor(ECFArmorDirection ArmorDirection) const;

	// [v1.0.0] 지정 방향 장갑의 0~1 비율을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="방향 장갑 비율 반환 (Get Armor Ratio)", ToolTip="지정한 방향의 현재 장갑을 최대 장갑으로 나눈 0~1 범위 비율을 반환합니다."))
	float GetArmorRatio(ECFArmorDirection ArmorDirection) const;

	// [v1.0.0] 쉴드가 실제 증가 중인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="쉴드 재생 중 여부 (Is Shield Regenerating)", ToolTip="재생 지연이 끝나고 현재 쉴드가 실제 증가 중이면 True를 반환합니다."))
	bool IsShieldRegenerating() const { return bShieldRegenerating; }

	// [v1.0.0] 쉴드 재생 시작까지 남은 지연 시간을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefense", meta=(DisplayName="남은 쉴드 재생 지연 반환 (Get Remaining Shield Regeneration Delay)", ToolTip="마지막 유효 피해 후 쉴드 재생 시작까지 남은 0 이상의 시간을 반환합니다."))
	float GetRemainingShieldRegenerationDelaySeconds() const { return RemainingShieldRegenerationDelaySeconds; }

	// [v1.0.0] 쉴드, 장갑 또는 내구도에 실제 피해가 적용됐을 때 호출됩니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|VehicleDefense|Events", meta=(DisplayName="차량 방어 피해 해결 이벤트 (OnVehicleDamageResolved)", ToolTip="유효 피해 요청으로 쉴드, 장갑 또는 차량 내구도 중 하나 이상이 실제 감소했을 때 전체 결과를 전달합니다."))
	FCFVehicleDamageResolvedSignature OnVehicleDamageResolved;

	// [v1.0.0] 현재 쉴드가 피해 또는 재생으로 실제 변경됐을 때 호출됩니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|VehicleDefense|Events", meta=(DisplayName="쉴드 변경 이벤트 (OnShieldChanged)", ToolTip="현재 쉴드가 피해 흡수 또는 자동 재생으로 실제 변경됐을 때 이전, 현재와 최대 쉴드를 전달합니다."))
	FCFShieldChangedSignature OnShieldChanged;

	// [v1.0.0] 이번 타격으로 쉴드가 처음 0이 됐을 때 호출됩니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|VehicleDefense|Events", meta=(DisplayName="쉴드 파괴 이벤트 (OnShieldBroken)", ToolTip="피해 전 쉴드가 0보다 크고 피해 후 처음 0이 됐을 때 전체 피해 결과를 전달합니다."))
	FCFShieldBrokenSignature OnShieldBroken;

	// [v1.0.0] 특정 방향 장갑이 실제 변경됐을 때 호출됩니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|VehicleDefense|Events", meta=(DisplayName="방향 장갑 변경 이벤트 (OnArmorChanged)", ToolTip="특정 방향 장갑이 피해를 흡수해 실제 변경됐을 때 방향과 이전, 현재, 최대 장갑을 전달합니다."))
	FCFArmorChangedSignature OnArmorChanged;

	// [v1.0.0] 이번 타격으로 특정 방향 장갑이 처음 0이 됐을 때 호출됩니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|VehicleDefense|Events", meta=(DisplayName="방향 장갑 파괴 이벤트 (OnArmorBroken)", ToolTip="피해 전 장갑이 0보다 크고 피해 후 처음 0이 됐을 때 방향과 전체 피해 결과를 전달합니다."))
	FCFArmorBrokenSignature OnArmorBroken;

	// [v1.0.0] 지연 종료 후 실제 쉴드 증가가 시작되는 첫 프레임에 호출됩니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|VehicleDefense|Events", meta=(DisplayName="쉴드 재생 시작 이벤트 (OnShieldRegenerationStarted)", ToolTip="재생 지연이 끝난 뒤 현재 쉴드가 실제 증가하기 시작하는 첫 프레임에 현재와 최대 쉴드를 전달합니다."))
	FCFShieldRegenStartedSignature OnShieldRegenerationStarted;

	// [v1.0.0] 자동 재생으로 쉴드가 최대값에 도달한 순간 호출됩니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|VehicleDefense|Events", meta=(DisplayName="쉴드 완전 회복 이벤트 (OnShieldFullyRestored)", ToolTip="자동 재생으로 현재 쉴드가 최대 쉴드에 도달한 순간 현재와 최대 쉴드를 전달합니다."))
	FCFShieldFullyRestoredSignature OnShieldFullyRestored;

private:
	// [v1.0.0] 같은 Actor의 VehicleHealthComp를 다시 찾고 캐시합니다.
	UCFVehicleHealthComp* ResolveBoundHealthComponent();

	// [v1.0.0] HitContext의 위치, 노멀과 입사 방향으로 피격 장갑 방향을 판정합니다.
	ECFArmorDirection ResolveArmorDirectionFromHitContext(const FCFDamageHitContext& InDamageHitContext) const;

	// [v1.0.0] HitComponentName 또는 Actor Bounds에서 방향 판정 기준 중심을 계산합니다.
	FVector ResolveDefenseBoundsCenter(const FCFDamageHitContext& InDamageHitContext) const;

	// [v1.0.0] 지정 방향 장갑의 현재값을 내부 상태에서 읽습니다.
	float GetCurrentArmorInternal(ECFArmorDirection ArmorDirection) const;

	// [v1.0.0] 지정 방향 장갑의 현재값을 0 이상으로 저장합니다.
	void SetCurrentArmorInternal(ECFArmorDirection ArmorDirection, float NewArmorValue);

	// [v1.0.0] 유효 피해 뒤 쉴드 재생 지연을 초기화하고 필요할 때만 Tick을 켭니다.
	void RestartShieldRegenerationAfterDamage();

	// [v1.0.0] 쉴드 재생과 지연 상태를 정리하고 Tick을 끕니다.
		void StopShieldRegeneration();

	// [v1.2.0] 실제 Shield, Armor 또는 Integrity 피해가 적용된 전체 결과를 Debug 캐시에 저장합니다.
	void StoreLastVehicleDamageResult(const FCFVehicleDamageResult& InVehicleDamageResult);

	// [v1.2.0] 방어 초기화 또는 수동 Reset 시 이전 전체 피해 결과 캐시를 비웁니다.
	void ClearLastVehicleDamageResult();

	// [v1.0.0] 한 Tick의 지연 소비와 실제 쉴드 증가를 처리합니다.
	void AdvanceShieldRegeneration(float DeltaSeconds);

	// [v1.0.0] 현재 쉴드·장갑 설정을 제공하는 정적 방어 데이터입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Runtime", meta=(AllowPrivateAccess="true", DisplayName="활성 차량 방어 데이터 (ActiveDefenseData)", ToolTip="현재 방어 런타임을 초기화한 VehicleDefenseData입니다. None이면 Legacy Health Fallback 상태입니다."))
	TObjectPtr<UCFVehicleDefenseData> ActiveDefenseData = nullptr;

	// [v1.0.0] 같은 Actor에서 차량 내구도 적용을 담당하는 컴포넌트입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFVehicleHealthComp> BoundHealthComponent = nullptr;

	// [v1.0.0] 현재 차량의 최대 쉴드입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Runtime", meta=(AllowPrivateAccess="true", ClampMin="0.0", DisplayName="최대 쉴드 (MaximumShield)", ToolTip="VehicleDefenseData에서 초기화된 최대 쉴드입니다."))
	float MaximumShield = 0.0f;

	// [v1.0.0] 현재 차량에 남아 있는 쉴드입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Runtime", meta=(AllowPrivateAccess="true", ClampMin="0.0", DisplayName="현재 쉴드 (CurrentShield)", ToolTip="피해 흡수와 자동 재생으로 변경되는 현재 쉴드입니다."))
	float CurrentShield = 0.0f;

	// [v1.0.0] 현재 정면 장갑 내구도입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Runtime", meta=(AllowPrivateAccess="true", ClampMin="0.0", DisplayName="현재 정면 장갑 (FrontArmor)", ToolTip="차량 로컬 +X 방향의 현재 장갑 내구도입니다."))
	float FrontArmor = 0.0f;

	// [v1.0.0] 현재 좌측 장갑 내구도입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Runtime", meta=(AllowPrivateAccess="true", ClampMin="0.0", DisplayName="현재 좌측 장갑 (LeftArmor)", ToolTip="차량 로컬 -Y 방향의 현재 장갑 내구도입니다."))
	float LeftArmor = 0.0f;

	// [v1.0.0] 현재 우측 장갑 내구도입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Runtime", meta=(AllowPrivateAccess="true", ClampMin="0.0", DisplayName="현재 우측 장갑 (RightArmor)", ToolTip="차량 로컬 +Y 방향의 현재 장갑 내구도입니다."))
	float RightArmor = 0.0f;

	// [v1.0.0] 현재 후면 장갑 내구도입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Runtime", meta=(AllowPrivateAccess="true", ClampMin="0.0", DisplayName="현재 후면 장갑 (RearArmor)", ToolTip="차량 로컬 -X 방향의 현재 장갑 내구도입니다."))
	float RearArmor = 0.0f;

	// [v1.0.0] 현재 상부 장갑 내구도입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Runtime", meta=(AllowPrivateAccess="true", ClampMin="0.0", DisplayName="현재 상부 장갑 (TopArmor)", ToolTip="차량 로컬 +Z 방향의 현재 장갑 내구도입니다."))
	float TopArmor = 0.0f;

	// [v1.0.0] 현재 하부 장갑 내구도입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Runtime", meta=(AllowPrivateAccess="true", ClampMin="0.0", DisplayName="현재 하부 장갑 (BottomArmor)", ToolTip="차량 로컬 -Z 방향의 현재 장갑 내구도입니다."))
	float BottomArmor = 0.0f;

		// [v1.2.0] 현재 초기화 이후 실제 방어층 피해 결과가 한 번 이상 저장됐는지 여부입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 차량 방어 결과 존재 여부 (bHasLastVehicleDamageResult)", ToolTip="True이면 현재 초기화 이후 Shield, Armor 또는 Integrity에 실제 피해가 적용된 전체 결과가 저장되어 있습니다."))
	bool bHasLastVehicleDamageResult = false;

	// [v1.2.0] 마지막으로 실제 방어층에 적용된 Shield·Armor·Integrity 전체 피해 결과입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 차량 방어 결과 (LastVehicleDamageResult)", ToolTip="마지막 유효 피해에서 계산된 방향, Shield, Armor, 관통과 Integrity 전체 결과입니다."))
	FCFVehicleDamageResult LastVehicleDamageResult;

	// [v1.2.0] 마지막 전체 차량 피해 결과의 VehicleDebug 표시용 문자열입니다.
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Debug", meta=(AllowPrivateAccess="true", DisplayName="마지막 차량 방어 결과 요약 (LastVehicleDamageResultSummary)", ToolTip="마지막 유효 피해 결과를 VehicleDebug와 Blueprint UI가 계산 없이 표시할 수 있도록 만든 요약입니다."))
		FString LastVehicleDamageResultSummary = TEXT("차량 방어 피해 기록 없음");

	// [v1.0.0] 유효한 VehicleDefenseData로 런타임 초기화가 완료됐는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Runtime", meta=(AllowPrivateAccess="true", DisplayName="차량 방어 준비 여부 (bDefenseInitialized)", ToolTip="쉴드와 6방향 장갑이 유효한 VehicleDefenseData로 준비됐는지 여부입니다."))
	bool bDefenseInitialized = false;

	// [v1.0.0] 재생 지연 종료 후 현재 쉴드가 실제 증가 중인지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Runtime", meta=(AllowPrivateAccess="true", DisplayName="쉴드 재생 중 여부 (bShieldRegenerating)", ToolTip="현재 쉴드가 실제 증가하는 재생 단계에 진입했는지 여부입니다."))
	bool bShieldRegenerating = false;

	// [v1.0.0] 쉴드 재생 시작까지 남은 지연 시간입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehicleDefense|Runtime", meta=(AllowPrivateAccess="true", ClampMin="0.0", DisplayName="남은 쉴드 재생 지연 (RemainingShieldRegenerationDelaySeconds)", ToolTip="마지막 유효 피해 후 쉴드 재생이 시작되기까지 남은 시간입니다."))
	float RemainingShieldRegenerationDelaySeconds = 0.0f;
};
