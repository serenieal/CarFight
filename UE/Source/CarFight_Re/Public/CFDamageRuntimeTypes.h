// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-31
// Description: CarFight 차량 방어·손상 런타임 공용 타입
// Scope: 피해 전달 방식, 장갑 방향·분류, 방향별 장갑 설정과 방어층별 피해 결과 계약을 제공합니다.
// Changelog:
// - v1.0.0: CF-FQ-033 DR-P0-01 공용 enum, FCFDirectionalArmorConfig와 FCFVehicleDamageResult를 추가.
// Migration:
// - 기존 FCFDamageApplyResult는 차량 내구도 적용 결과로 유지하며 FCFVehicleDamageResult.IntegrityApplyResult에 포함한다.
// - 기존 HitScan / Projectile 호출 경로는 DR-P0-03 전까지 이 타입을 소비하지 않으므로 현재 런타임 동작은 변경되지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageTypes.h"
#include "CFDamageRuntimeTypes.generated.h"

class AActor;

/**
 * 피해가 대상에게 전달된 방식입니다.
 */
UENUM(BlueprintType)
enum class ECFDamageDeliveryType : uint8
{
	DirectHit UMETA(DisplayName="직접 명중 (Direct Hit)"),
	RadialExplosion UMETA(DisplayName="범위 폭발 (Radial Explosion)"),
	Collision UMETA(DisplayName="충돌 (Collision)"),
	Environmental UMETA(DisplayName="환경 피해 (Environmental)"),
	DamageOverTime UMETA(DisplayName="지속 피해 (Damage Over Time)"),
	Scripted UMETA(DisplayName="스크립트 피해 (Scripted)")
};

/**
 * 차량 로컬 축 기준으로 판정한 장갑 방향입니다.
 */
UENUM(BlueprintType)
enum class ECFArmorDirection : uint8
{
	None UMETA(DisplayName="없음 (None)"),
	Front UMETA(DisplayName="정면 (Front)"),
	Left UMETA(DisplayName="좌측 (Left)"),
	Right UMETA(DisplayName="우측 (Right)"),
	Rear UMETA(DisplayName="후면 (Rear)"),
	Top UMETA(DisplayName="상부 (Top)"),
	Bottom UMETA(DisplayName="하부 (Bottom)")
};

/**
 * 차량 장갑의 분류와 UI 표시용 타입입니다.
 */
UENUM(BlueprintType)
enum class ECFArmorType : uint8
{
	Light UMETA(DisplayName="경량 장갑 (Light)"),
	Standard UMETA(DisplayName="표준 장갑 (Standard)"),
	Heavy UMETA(DisplayName="중장갑 (Heavy)")
};

/**
 * 한 방향의 최대 장갑 내구도와 피격 피해 배율입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFDirectionalArmorConfig
{
	GENERATED_BODY()

	// [v1.0.0] 해당 방향 장갑이 가질 수 있는 최대 내구도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefense|Armor", meta=(ClampMin="0.0", DisplayName="최대 장갑 내구도 (MaximumArmor)", ToolTip="해당 방향의 장갑 런타임이 초기화할 최대 내구도입니다. 0이면 그 방향에는 장갑이 없습니다."))
	float MaximumArmor = 0.0f;

	// [v1.0.0] 쉴드를 통과한 피해에 적용할 방향별 피해 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDefense|Armor", meta=(ClampMin="0.0", DisplayName="방향 피해 배율 (DamageMultiplier)", ToolTip="쉴드를 통과한 피해에 곱할 방향별 배율입니다. 정면 기준값은 1.0입니다."))
	float DamageMultiplier = 1.0f;

	// [v1.0.0] 음수를 제거한 유효 최대 장갑 내구도를 반환합니다.
	float GetEffectiveMaximumArmor() const
	{
		return FMath::Max(MaximumArmor, 0.0f);
	}

	// [v1.0.0] 음수를 제거한 유효 방향 피해 배율을 반환합니다.
	float GetEffectiveDamageMultiplier() const
	{
		return FMath::Max(DamageMultiplier, 0.0f);
	}
};

/**
 * 한 번의 차량 피해 요청에서 방어층별 계산과 기존 차량 내구도 적용 결과를 보존합니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFVehicleDamageResult
{
	GENERATED_BODY()

	// [v1.0.0] 입력 검증을 통과해 피해 계산 요청이 수락됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage", meta=(DisplayName="피해 요청 수락 여부 (bDamageAccepted)", ToolTip="True이면 피해 입력 검증을 통과해 방어층 계산이 수행됐습니다."))
	bool bDamageAccepted = false;

	// [v1.0.0] 쉴드, 장갑 또는 차량 내구도 중 하나 이상이 실제 감소했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage", meta=(DisplayName="방어층 적용 여부 (bAppliedToAnyLayer)", ToolTip="True이면 이번 요청으로 쉴드, 장갑 또는 차량 내구도 중 하나 이상이 실제 감소했습니다."))
	bool bAppliedToAnyLayer = false;

	// [v1.0.0] DefenseData가 없어 기존 직접 체력 피해 경로를 사용했는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage", meta=(DisplayName="레거시 체력 Fallback 사용 (bUsedLegacyHealthFallback)", ToolTip="True이면 VehicleData.DefaultDefenseData가 없어 기존 BaseDamage 직접 체력 적용 경로를 사용했습니다."))
	bool bUsedLegacyHealthFallback = false;

	// [v1.0.0] 피해 요청이 거부됐을 때의 기존 공용 거부 사유입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage", meta=(DisplayName="피해 거부 사유 (RejectReason)", ToolTip="피해 요청이 적용되지 않은 경우 기존 Damage Runtime의 공용 거부 사유를 기록합니다."))
	ECFDamageApplyRejectReason RejectReason = ECFDamageApplyRejectReason::None;

	// [v1.0.0] 이번 피해 계산 대상 Actor입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage", meta=(DisplayName="피해 대상 Actor (TargetActor)", ToolTip="이번 차량 방어·손상 계산의 대상 Actor입니다."))
	TObjectPtr<AActor> TargetActor = nullptr;

	// [v1.0.0] 이번 계산에 사용한 DamageData 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage", meta=(DisplayName="피해 ID (DamageId)", ToolTip="이번 계산에 사용한 DamageData의 DamageId입니다."))
	FName DamageId = NAME_None;

	// [v1.0.0] 이번 피해가 대상에게 전달된 방식입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage", meta=(DisplayName="피해 전달 방식 (DamageDeliveryType)", ToolTip="직접 명중, 범위 폭발, 충돌 등 피해가 전달된 방식을 기록합니다."))
	ECFDamageDeliveryType DamageDeliveryType = ECFDamageDeliveryType::DirectHit;

	// [v1.0.0] 차량 로컬 축 기준으로 판정한 장갑 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage", meta=(DisplayName="장갑 방향 (ArmorDirection)", ToolTip="이번 피해가 도달한 정면, 좌측, 우측, 후면, 상부 또는 하부 장갑 방향입니다."))
	ECFArmorDirection ArmorDirection = ECFArmorDirection::None;

	// [v1.0.0] DamageData.BaseDamage에서 읽은 원본 피해량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage", meta=(ClampMin="0.0", DisplayName="요청 피해량 (RequestedDamage)", ToolTip="쉴드 계산을 시작하기 전의 원본 직접 피해량입니다."))
	float RequestedDamage = 0.0f;

	// [v1.0.0] 쉴드가 실제로 흡수한 피해량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Shield", meta=(ClampMin="0.0", DisplayName="쉴드 흡수 피해 (DamageAbsorbedByShield)", ToolTip="현재 쉴드가 원본 피해에서 실제로 흡수한 피해량입니다."))
	float DamageAbsorbedByShield = 0.0f;

	// [v1.0.0] 쉴드를 통과한 원본 기준 피해량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Shield", meta=(ClampMin="0.0", DisplayName="쉴드 통과 피해 (DamageAfterShield)", ToolTip="원본 피해에서 쉴드 흡수량을 뺀 뒤 장갑 방향 계산으로 전달되는 피해량입니다."))
	float DamageAfterShield = 0.0f;

	// [v1.0.0] 이번 장갑 방향에 사용한 피해 배율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Armor", meta=(ClampMin="0.0", DisplayName="방향 피해 배율 (DirectionDamageMultiplier)", ToolTip="쉴드 통과 피해에 적용한 방향별 피해 배율입니다."))
	float DirectionDamageMultiplier = 1.0f;

	// [v1.0.0] 쉴드 통과 피해에 방향 배율을 적용한 장갑 계산 기준 피해량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Armor", meta=(ClampMin="0.0", DisplayName="방향 적용 피해 (DirectionalDamage)", ToolTip="쉴드를 통과한 피해에 방향별 피해 배율을 적용한 값입니다."))
	float DirectionalDamage = 0.0f;

	// [v1.0.0] 이번 DamageData에서 읽은 장갑 관통력입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Armor", meta=(ClampMin="0.0", DisplayName="장갑 관통력 (ArmorPenetration)", ToolTip="이번 장갑 관통 비율 계산에 사용한 DamageData의 관통력입니다."))
	float ArmorPenetration = 0.0f;

	// [v1.0.0] 음수를 제거한 대상 장갑 저항입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Armor", meta=(ClampMin="0.0", DisplayName="유효 장갑 저항 (EffectiveArmorResistance)", ToolTip="장갑 관통 비율 계산에 사용한 0 이상의 장갑 저항입니다."))
	float EffectiveArmorResistance = 0.0f;

	// [v1.0.0] 0에서 1 사이로 제한된 장갑 관통 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Armor", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="장갑 관통 비율 (ArmorPenetrationRatio)", ToolTip="ArmorPenetration을 EffectiveArmorResistance로 나눈 뒤 0~1로 제한한 관통 비율입니다."))
	float ArmorPenetrationRatio = 0.0f;

	// [v1.0.0] 해당 방향 장갑이 실제로 흡수한 피해량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Armor", meta=(ClampMin="0.0", DisplayName="장갑 흡수 피해 (DamageAbsorbedByArmor)", ToolTip="관통하지 않은 피해 중 해당 방향 장갑이 실제로 흡수한 양입니다."))
	float DamageAbsorbedByArmor = 0.0f;

	// [v1.0.0] 관통과 장갑 고갈 초과분을 합친 차량 내구도 요청 피해량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Integrity", meta=(ClampMin="0.0", DisplayName="차량 내구도 요청 피해 (DamageRequestedForIntegrity)", ToolTip="장갑 관통 피해와 장갑이 막지 못한 초과 피해를 합친 차량 내구도 요청량입니다."))
	float DamageRequestedForIntegrity = 0.0f;

	// [v1.0.0] 차량 내구도에서 실제로 감소한 피해량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Integrity", meta=(ClampMin="0.0", DisplayName="차량 내구도 적용 피해 (DamageAppliedToIntegrity)", ToolTip="차량 내구도 하한 0을 반영해 실제로 감소한 피해량입니다."))
	float DamageAppliedToIntegrity = 0.0f;

	// [v1.0.0] 남은 차량 내구도를 초과해 실제 감소로 기록되지 못한 피해량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Integrity", meta=(ClampMin="0.0", DisplayName="차량 내구도 초과 피해 (IntegrityOverkillDamage)", ToolTip="차량 내구도 요청 피해 중 남은 내구도를 초과한 양입니다."))
	float IntegrityOverkillDamage = 0.0f;

	// [v1.0.0] 후속 모듈 시스템에 실제 적용된 피해량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Component", meta=(ClampMin="0.0", DisplayName="부품 적용 피해 (DamageAppliedToComponents)", ToolTip="DR-P1 모듈 피해 시스템이 연결되기 전에는 항상 0입니다."))
	float DamageAppliedToComponents = 0.0f;

	// [v1.0.0] 후속 물리 반응 시스템이 실제 적용한 힘 크기입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Physics", meta=(ClampMin="0.0", DisplayName="적용 물리 충격 (PhysicalImpulseApplied)", ToolTip="DR-P1 물리 충격 시스템이 연결되기 전에는 항상 0입니다."))
	float PhysicalImpulseApplied = 0.0f;

	// [v1.0.0] 피해 적용 직전 쉴드 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Shield", meta=(ClampMin="0.0", DisplayName="적용 전 쉴드 (ShieldBefore)", ToolTip="이번 피해 계산 직전 대상 차량의 현재 쉴드입니다."))
	float ShieldBefore = 0.0f;

	// [v1.0.0] 피해 적용 직후 쉴드 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Shield", meta=(ClampMin="0.0", DisplayName="적용 후 쉴드 (ShieldAfter)", ToolTip="이번 피해 계산 직후 대상 차량의 현재 쉴드입니다."))
	float ShieldAfter = 0.0f;

	// [v1.0.0] 피해 적용 직전 해당 방향 장갑 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Armor", meta=(ClampMin="0.0", DisplayName="적용 전 장갑 (ArmorBefore)", ToolTip="이번 피해 계산 직전 피격 방향 장갑의 현재 내구도입니다."))
	float ArmorBefore = 0.0f;

	// [v1.0.0] 피해 적용 직후 해당 방향 장갑 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Armor", meta=(ClampMin="0.0", DisplayName="적용 후 장갑 (ArmorAfter)", ToolTip="이번 피해 계산 직후 피격 방향 장갑의 현재 내구도입니다."))
	float ArmorAfter = 0.0f;

	// [v1.0.0] 피해 적용 직전 차량 내구도 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Integrity", meta=(ClampMin="0.0", DisplayName="적용 전 차량 내구도 (IntegrityBefore)", ToolTip="이번 피해 계산 직전 VehicleHealthComp의 현재 체력입니다."))
	float IntegrityBefore = 0.0f;

	// [v1.0.0] 피해 적용 직후 차량 내구도 값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Integrity", meta=(ClampMin="0.0", DisplayName="적용 후 차량 내구도 (IntegrityAfter)", ToolTip="이번 피해 계산 직후 VehicleHealthComp의 현재 체력입니다."))
	float IntegrityAfter = 0.0f;

	// [v1.0.0] 이번 피해로 쉴드가 처음 0이 됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Shield", meta=(DisplayName="이번 타격 쉴드 파괴 (bShieldBrokenThisHit)", ToolTip="True이면 피해 전 쉴드가 0보다 크고 피해 후 처음 0이 됐습니다."))
	bool bShieldBrokenThisHit = false;

	// [v1.0.0] 이번 피해로 해당 방향 장갑이 처음 0이 됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Armor", meta=(DisplayName="이번 타격 장갑 파괴 (bArmorBrokenThisHit)", ToolTip="True이면 피해 전 장갑이 0보다 크고 피해 후 처음 0이 됐습니다."))
	bool bArmorBrokenThisHit = false;

	// [v1.0.0] 이번 피해로 차량이 처음 파괴 상태가 됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Integrity", meta=(DisplayName="이번 타격 차량 파괴 (bDestroyedThisHit)", ToolTip="True이면 이번 피해로 차량 내구도가 처음 0이 되어 파괴 상태로 전환됐습니다."))
	bool bDestroyedThisHit = false;

	// [v1.0.0] 기존 VehicleHealthComp가 반환한 차량 내구도 적용 결과입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Integrity", meta=(DisplayName="차량 내구도 적용 결과 (IntegrityApplyResult)", ToolTip="기존 Health 이벤트와 디버그 호환을 위해 FCFDamageApplyResult를 그대로 보존합니다."))
	FCFDamageApplyResult IntegrityApplyResult;

	// [v1.0.0] 후속 모듈 피해가 영향을 준 부품 ID 목록입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehicleDamage|Component", meta=(DisplayName="영향받은 부품 ID (AffectedComponentIds)", ToolTip="DR-P1 모듈 피해 시스템이 연결되기 전에는 빈 배열입니다."))
	TArray<FName> AffectedComponentIds;
};
