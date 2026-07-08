// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-02
// Description: CarFight 차량 피해 DataAsset
// Scope: ProjectileData가 참조할 최소 DamageData와 디버그 요약 함수를 제공합니다.
// Changelog:
// - v1.0.0: DamageData 최소 필드, 피해 타입, 직접 피해 / 관통 / 폭발 / 모듈 피해 / 물리 반응 후보값을 추가.
// Migration:
// - P0에서는 DamageData를 실제 HP 차감에 바로 사용하지 않고 데이터 연결과 Debug 확인에 먼저 사용한다.
// - DamageData 직접 참조 슬롯은 ProjectileData.DefaultDamageData 하나만 사용한다.
// - HitScan / Laser처럼 실제 Actor를 스폰하지 않는 무기도 가상 ProjectileData를 통해 이 DamageData를 참조한다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageTypes.h"
#include "Engine/DataAsset.h"
#include "CFDamageData.generated.h"

/**
 * 맞았을 때 적용할 최소 피해 규칙 데이터입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFDamageData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] 기본 피해 데이터 값을 초기화합니다.
	UCFDamageData();

	// [v1.0.0] 디버그 패널과 로그에 표시할 피해 데이터 요약 문자열을 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|DamageData", meta=(DisplayName="피해 요약 생성 (Build Damage Summary)", ToolTip="디버그 패널과 로그에 표시할 핵심 피해 데이터 요약 문자열을 생성합니다."))
	FString BuildDamageSummary() const;

	// [v1.0.0] 폭발 / 범위 피해를 사용할 수 있는 설정인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|DamageData", meta=(DisplayName="범위 피해 사용 가능 여부 (Can Use Radial Damage)", ToolTip="범위 피해 옵션과 폭발 반경 / 폭발 피해량이 모두 유효한지 반환합니다."))
	bool CanUseRadialDamage() const;

	// [v1.0.0] 피해 데이터를 식별하는 안정적인 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|DamageData|Identity", meta=(DisplayName="피해 ID (DamageId)", ToolTip="전투 로그, 디버그, 저장 데이터에서 이 피해 규칙을 식별할 이름입니다. 예: ProtoDirectHit"))
	FName DamageId = TEXT("ProtoDirectHit");

	// [v1.0.0] 이 DamageData가 표현하는 피해 종류입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|DamageData|Type", meta=(DisplayName="피해 타입 (DamageType)", ToolTip="이 피해 규칙의 기본 종류입니다. P0 후보는 Kinetic, Explosive, Energy입니다."))
	ECFDamageType DamageType = ECFDamageType::Kinetic;

	// [v1.0.0] 직접 명중 시 사용할 기본 피해량입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|DamageData|Direct", meta=(ClampMin="0.0", DisplayName="기본 피해량 (BaseDamage)", ToolTip="직접 명중 시 사용할 기본 피해량입니다. P0에서는 실제 HP 차감 없이 Debug 확인에 먼저 사용합니다."))
	float BaseDamage = 25.0f;

	// [v1.0.0] 장갑 계산에서 사용할 관통 기준값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|DamageData|Direct", meta=(ClampMin="0.0", DisplayName="장갑 관통력 (ArmorPenetration)", ToolTip="후속 장갑 계산에서 사용할 관통 기준값입니다. P0에서는 저장과 표시만 합니다."))
	float ArmorPenetration = 0.0f;

	// [v1.0.0] 폭발 / 범위 피해를 사용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|DamageData|Radial", meta=(DisplayName="범위 피해 사용 여부 (bUseRadialDamage)", ToolTip="True이면 후속 Damage Runtime에서 폭발 / 범위 피해 후보로 해석합니다. P0에서는 저장과 표시만 합니다."))
	bool bUseRadialDamage = false;

	// [v1.0.0] 폭발 피해가 적용될 최대 반경입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|DamageData|Radial", meta=(ClampMin="0.0", EditCondition="bUseRadialDamage", DisplayName="폭발 반경 (ExplosionRadius)", ToolTip="폭발 피해가 적용될 최대 반경입니다."))
	float ExplosionRadius = 0.0f;

	// [v1.0.0] 최대 폭발 피해가 유지되는 내부 반경입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|DamageData|Radial", meta=(ClampMin="0.0", EditCondition="bUseRadialDamage", DisplayName="폭발 내부 반경 (ExplosionInnerRadius)", ToolTip="최대 폭발 피해가 유지되는 내부 반경입니다. ExplosionRadius보다 크게 입력해도 후속 계산에서 안전하게 보정해야 합니다."))
	float ExplosionInnerRadius = 0.0f;

	// [v1.0.0] 폭발 중심에서 사용할 피해량입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|DamageData|Radial", meta=(ClampMin="0.0", EditCondition="bUseRadialDamage", DisplayName="폭발 피해량 (ExplosionDamage)", ToolTip="폭발 중심에서 사용할 피해량입니다."))
	float ExplosionDamage = 0.0f;

	// [v1.0.0] 폭발 반경 가장자리에서 유지할 최소 피해 비율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|DamageData|Radial", meta=(ClampMin="0.0", ClampMax="1.0", EditCondition="bUseRadialDamage", DisplayName="최소 폭발 피해 비율 (MinExplosionDamageScale)", ToolTip="폭발 반경 가장자리에서 유지할 최소 피해 비율입니다. 0이면 가장자리 피해가 0까지 줄 수 있습니다."))
	float MinExplosionDamageScale = 0.0f;

	// [v1.0.0] 후속 모듈 손상 계산에서 사용할 피해 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|DamageData|Module", meta=(ClampMin="0.0", DisplayName="모듈 피해 배율 (ModuleDamageScale)", ToolTip="후속 모듈 손상 계산에서 사용할 피해 배율입니다. P0에서는 저장과 표시만 합니다."))
	float ModuleDamageScale = 1.0f;

	// [v1.0.0] 피격 물리 반응 후보로 사용할 힘 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|DamageData|Physics", meta=(ClampMin="0.0", DisplayName="충격 힘 (ImpulseStrength)", ToolTip="후속 물리 반응에서 사용할 충격 힘 후보값입니다. P0에서는 실제 물리 적용을 하지 않습니다."))
	float ImpulseStrength = 0.0f;
};
