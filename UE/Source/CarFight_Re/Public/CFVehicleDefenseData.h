// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-08-01
// Description: CarFight 차량 쉴드·방향별 장갑 DataAsset과 피팅 방어 질량
// Scope: 차량별 쉴드, 재생, 장갑 분류·저항, 6방향 장갑 설정, 피팅 방어 질량과 후속 부품 피해 배율을 제공합니다.
// Changelog:
// - v1.1.0: CF-FQ-034 FIT-P0-02 DefenseMassKg와 안전 Getter·요약·DataValidation을 추가.
// - v1.0.0: CF-FQ-033 DR-P0-01 VehicleDefenseData Foundation, 안전 Getter, 요약과 DataValidation 계약을 추가.
// Migration:
// - 기존 VehicleDefenseData는 DefenseMassKg=0 기본값으로 현재 쉴드·장갑 피해 결과와 차량 주행을 유지한다.
// - 피팅에 사용할 VehicleDefenseData만 실제 방어 패키지 질량을 명시하며 0은 미설정 질량으로 후속 피팅 검증에서 처리한다.
// - 기존 VehicleData.DefaultDefenseData는 None으로 시작하므로 기존 BaseDamage 직접 Health 동작을 유지한다.
// - 이 DataAsset은 차량 내구도 최대값을 소유하지 않으며 기존 VehicleDurabilityConfig.MaxHealth를 계속 사용한다.
// - 부품 피해 배율은 DR-P1 예약 데이터이며 현재 피해 런타임에는 연결하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageRuntimeTypes.h"
#include "Engine/DataAsset.h"
#include "Misc/DataValidation.h"
#include "CFVehicleDefenseData.generated.h"

/**
 * 차량별 쉴드와 방향별 장갑 정적 설정을 제공하는 DataAsset입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFVehicleDefenseData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] P0 테스트 기준 쉴드·장갑 기본값을 초기화합니다.
	UCFVehicleDefenseData();

	// [v1.0.0] 음수를 제거한 유효 최대 쉴드를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefenseData", meta=(DisplayName="유효 최대 쉴드 반환 (Get Effective Maximum Shield)", ToolTip="쉴드 비활성 또는 잘못된 음수 값을 반영해 실제 초기화에 사용할 0 이상의 최대 쉴드를 반환합니다."))
	float GetEffectiveMaximumShield() const;

	// [v1.0.0] 음수를 제거한 유효 쉴드 재생 지연 시간을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefenseData", meta=(DisplayName="유효 쉴드 재생 지연 반환 (Get Effective Shield Regeneration Delay)", ToolTip="실제 쉴드 재생 상태에서 사용할 0 이상의 재생 지연 시간을 반환합니다."))
	float GetEffectiveShieldRegenerationDelaySeconds() const;

	// [v1.0.0] 음수를 제거한 유효 초당 쉴드 재생량을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefenseData", meta=(DisplayName="유효 초당 쉴드 재생량 반환 (Get Effective Shield Regeneration Per Second)", ToolTip="실제 쉴드 재생 상태에서 사용할 0 이상의 초당 재생량을 반환합니다."))
	float GetEffectiveShieldRegenerationPerSecond() const;

		// [v1.0.0] 음수를 제거한 유효 장갑 저항을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefenseData", meta=(DisplayName="유효 장갑 저항 반환 (Get Effective Armor Resistance)", ToolTip="장갑 관통 비율 계산에 사용할 0 이상의 장갑 저항을 반환합니다."))
	float GetEffectiveArmorResistance() const;

	// [v1.1.0] 음수나 비유한 값을 제거한 피팅용 유효 방어 질량을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefenseData|Mass", meta=(DisplayName="유효 방어 질량 반환 (Get Effective Defense Mass)", ToolTip="피팅 질량 계산에서 사용할 0 이상의 유한한 방어 패키지 질량을 kg 단위로 반환합니다."))
	float GetEffectiveDefenseMassKg() const;

	// [v1.0.0] 지정 방향의 음수 값을 보정한 장갑 설정 복사본을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefenseData", meta=(DisplayName="유효 방향 장갑 설정 반환 (Get Effective Directional Armor Config)", ToolTip="지정한 방향의 최대 장갑 내구도와 피해 배율을 0 이상으로 보정해 반환합니다. None이면 장갑 0과 배율 1을 반환합니다."))
	FCFDirectionalArmorConfig GetEffectiveDirectionalArmorConfig(ECFArmorDirection ArmorDirection) const;

	// [v1.0.0] 음수를 제거한 유효 쉴드 단계 부품 피해 배율을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefenseData", meta=(DisplayName="유효 쉴드 부품 피해 배율 반환 (Get Effective Shield Component Damage Scale)", ToolTip="DR-P1 부품 피해 계산에서 사용할 0 이상의 쉴드 단계 배율을 반환합니다."))
	float GetEffectiveShieldComponentDamageScale() const;

	// [v1.0.0] 음수를 제거한 유효 장갑 단계 부품 피해 배율을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefenseData", meta=(DisplayName="유효 장갑 부품 피해 배율 반환 (Get Effective Armor Component Damage Scale)", ToolTip="DR-P1 부품 피해 계산에서 사용할 0 이상의 장갑 단계 배율을 반환합니다."))
	float GetEffectiveArmorComponentDamageScale() const;

	// [v1.0.0] 음수를 제거한 유효 차량 내구도 단계 부품 피해 배율을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefenseData", meta=(DisplayName="유효 내구도 부품 피해 배율 반환 (Get Effective Integrity Component Damage Scale)", ToolTip="DR-P1 부품 피해 계산에서 사용할 0 이상의 차량 내구도 단계 배율을 반환합니다."))
	float GetEffectiveIntegrityComponentDamageScale() const;

	// [v1.0.0] 디버그 패널과 로그에서 사용할 방어 데이터 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDefenseData", meta=(DisplayName="차량 방어 요약 생성 (Build Vehicle Defense Summary)", ToolTip="쉴드, 재생, 장갑 분류·저항과 6방향 장갑 설정을 한 줄 요약으로 생성합니다."))
	FString BuildVehicleDefenseSummary() const;

	// [v1.0.0] DataValidation과 Automation이 공유할 데이터 계약 오류 목록을 생성합니다.
	bool ValidateDefenseDataContract(TArray<FText>& OutValidationErrors) const;

#if WITH_EDITOR
	// [v1.0.0] Unreal Data Validation에서 잘못된 방어 설정을 보고합니다.
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

		// [v1.0.0] 전투 로그와 디버그에서 이 방어 설정을 식별할 안정적인 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Identity", meta=(DisplayName="방어 ID (DefenseId)", ToolTip="차량 방어 설정을 로그, 디버그와 저장 데이터에서 식별할 이름입니다. 예: StandardCombatDefense"))
	FName DefenseId = TEXT("ProtoVehicleDefense");

	// [v1.1.0] 쉴드 생성기와 장갑 패키지를 포함한 방어 구성의 피팅 질량입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Mass", meta=(ClampMin="0.0", Units="kg", DisplayName="방어 패키지 질량 kg (DefenseMassKg)", ToolTip="이 VehicleDefenseData 전체가 차량 피팅 총중량에 더하는 질량입니다. 0은 미설정이며 현재 피해 계산에는 영향을 주지 않습니다."))
	float DefenseMassKg = 0.0f;

	// [v1.0.0] 이 차량 방어 설정이 쉴드 계층을 사용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Shield", meta=(DisplayName="쉴드 사용 (bUseShield)", ToolTip="True이면 최대 쉴드와 재생 설정을 사용합니다. False이면 유효 최대 쉴드는 0입니다."))
	bool bUseShield = true;

	// [v1.0.0] 쉴드 런타임이 초기화할 최대 쉴드입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Shield", meta=(ClampMin="0.0", EditCondition="bUseShield", DisplayName="최대 쉴드 (MaximumShield)", ToolTip="차량 쉴드 런타임이 초기화할 최대 쉴드입니다. 쉴드 비활성 시 사용하지 않습니다."))
	float MaximumShield = 100.0f;

	// [v1.0.0] 유효 피해 후 쉴드 재생을 시작하기까지 기다릴 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Shield", meta=(ClampMin="0.0", EditCondition="bUseShield", DisplayName="쉴드 재생 지연 초 (ShieldRegenerationDelaySeconds)", ToolTip="유효한 피해가 방어층에 적용된 뒤 쉴드 재생을 시작하기까지 기다릴 시간입니다."))
	float ShieldRegenerationDelaySeconds = 5.0f;

	// [v1.0.0] 재생 중 매초 회복할 쉴드 양입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Shield", meta=(ClampMin="0.0", EditCondition="bUseShield", DisplayName="초당 쉴드 재생량 (ShieldRegenerationPerSecond)", ToolTip="재생 지연이 끝난 뒤 매초 회복할 쉴드 양입니다. 0이면 자동 재생하지 않습니다."))
	float ShieldRegenerationPerSecond = 10.0f;

	// [v1.0.0] 장갑 분류와 UI 표시용 타입입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Armor", meta=(DisplayName="장갑 타입 (ArmorType)", ToolTip="경량, 표준, 중장갑 분류와 UI 표시에 사용합니다. 실제 관통 공식은 ArmorResistance를 사용합니다."))
	ECFArmorType ArmorType = ECFArmorType::Standard;

	// [v1.0.0] DamageData.ArmorPenetration과 비교할 장갑 저항입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Armor", meta=(ClampMin="0.0", DisplayName="장갑 저항 (ArmorResistance)", ToolTip="장갑 관통 비율 계산에서 ArmorPenetration과 같은 단위로 비교할 저항 값입니다."))
	float ArmorResistance = 100.0f;

	// [v1.0.0] 차량 정면 장갑의 최대 내구도와 피해 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Armor", meta=(DisplayName="정면 장갑 설정 (FrontArmorConfig)", ToolTip="차량 로컬 +X 방향에서 맞았을 때 사용할 정면 장갑 설정입니다."))
	FCFDirectionalArmorConfig FrontArmorConfig;

	// [v1.0.0] 차량 좌측 장갑의 최대 내구도와 피해 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Armor", meta=(DisplayName="좌측 장갑 설정 (LeftArmorConfig)", ToolTip="차량 로컬 -Y 방향에서 맞았을 때 사용할 좌측 장갑 설정입니다."))
	FCFDirectionalArmorConfig LeftArmorConfig;

	// [v1.0.0] 차량 우측 장갑의 최대 내구도와 피해 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Armor", meta=(DisplayName="우측 장갑 설정 (RightArmorConfig)", ToolTip="차량 로컬 +Y 방향에서 맞았을 때 사용할 우측 장갑 설정입니다."))
	FCFDirectionalArmorConfig RightArmorConfig;

	// [v1.0.0] 차량 후면 장갑의 최대 내구도와 피해 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Armor", meta=(DisplayName="후면 장갑 설정 (RearArmorConfig)", ToolTip="차량 로컬 -X 방향에서 맞았을 때 사용할 후면 장갑 설정입니다."))
	FCFDirectionalArmorConfig RearArmorConfig;

	// [v1.0.0] 차량 상부 장갑의 최대 내구도와 피해 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Armor", meta=(DisplayName="상부 장갑 설정 (TopArmorConfig)", ToolTip="차량 로컬 +Z 방향에서 맞았을 때 사용할 상부 장갑 설정입니다."))
	FCFDirectionalArmorConfig TopArmorConfig;

	// [v1.0.0] 차량 하부 장갑의 최대 내구도와 피해 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Armor", meta=(DisplayName="하부 장갑 설정 (BottomArmorConfig)", ToolTip="차량 로컬 -Z 방향에서 맞았을 때 사용할 하부 장갑 설정입니다."))
	FCFDirectionalArmorConfig BottomArmorConfig;

	// [v1.0.0] 쉴드에 흡수된 피해가 후속 부품 피해로 전환될 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Component", meta=(ClampMin="0.0", DisplayName="쉴드 부품 피해 배율 (ShieldComponentDamageScale)", ToolTip="DR-P1 부품 피해 계산용 예약 값입니다. 쉴드 단계 기본값은 부품 피해 없음인 0입니다."))
	float ShieldComponentDamageScale = 0.0f;

	// [v1.0.0] 장갑에 도달한 피해가 후속 부품 피해로 전환될 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Component", meta=(ClampMin="0.0", DisplayName="장갑 부품 피해 배율 (ArmorComponentDamageScale)", ToolTip="DR-P1 부품 피해 계산용 예약 값입니다. 장갑 단계는 제한적인 부품 피해를 위한 낮은 기본값을 사용합니다."))
	float ArmorComponentDamageScale = 0.25f;

	// [v1.0.0] 차량 내구도에 도달한 피해가 후속 부품 피해로 전환될 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefenseData|Component", meta=(ClampMin="0.0", DisplayName="내구도 부품 피해 배율 (IntegrityComponentDamageScale)", ToolTip="DR-P1 부품 피해 계산용 예약 값입니다. 차량 내구도 단계의 기준 배율입니다."))
	float IntegrityComponentDamageScale = 1.0f;
};
