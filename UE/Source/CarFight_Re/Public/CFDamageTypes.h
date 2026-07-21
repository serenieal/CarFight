// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-07-14
// Description: CarFight Damage 공용 타입 정의
// Scope: DamageData, HitContext와 최소 차량 체력 피해 적용 결과가 공유할 공용 타입을 제공합니다.
// Changelog:
// - v1.3.0: 직접 피해 적용 거부 사유와 체력 변화 결과를 기록하는 ECFDamageApplyRejectReason / FCFDamageApplyResult 추가.
// - v1.2.0: HitScan / Projectile의 실제 피격 컴포넌트 이름 Debug 필드 추가.
// - v1.1.0: Dummy HitScan miss와 Projectile hit을 구분하기 위한 bBlockingHit Debug 필드 추가.
// - v1.0.0: DamageData 분리의 첫 단계로 ECFDamageType과 FCFDamageHitContext 타입을 추가.
// Migration:
// - HitScan과 Projectile은 같은 FCFDamageHitContext를 UCFVehicleHealthComp 피해 적용 진입점에 전달한다.
// - 실제 피해가 적용되지 않은 경우에도 FCFDamageApplyResult.RejectReason으로 원인을 구분한다.
// - 장갑, 모듈, 범위 피해와 물리 충격은 아직 이 공용 결과 구조의 계산 범위에 포함하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageTypes.generated.h"

class AActor;
class UCFDamageData;

/**
 * DamageData가 표현하는 기본 피해 종류입니다.
 */
UENUM(BlueprintType)
enum class ECFDamageType : uint8
{
	None UMETA(DisplayName="None"),
	Kinetic UMETA(DisplayName="Kinetic"),
	Explosive UMETA(DisplayName="Explosive"),
	Energy UMETA(DisplayName="Energy")
};

/**
 * 직접 피해 적용이 거부된 원인입니다.
 */
UENUM(BlueprintType)
enum class ECFDamageApplyRejectReason : uint8
{
	None UMETA(DisplayName="없음"),
	NoBlockingHit UMETA(DisplayName="Blocking Hit 없음"),
	MissingHitActor UMETA(DisplayName="피격 Actor 없음"),
	MissingDamageData UMETA(DisplayName="DamageData 없음"),
	NonPositiveDamage UMETA(DisplayName="피해량 0 이하"),
	SelfDamageBlocked UMETA(DisplayName="자기 피해 금지"),
	MissingHealthComponent UMETA(DisplayName="VehicleHealthComp 없음"),
	TargetMismatch UMETA(DisplayName="체력 소유 Actor 불일치"),
	TargetDestroyed UMETA(DisplayName="이미 파괴된 대상")
};

/**
 * 최소 차량 체력 피해 적용 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFDamageApplyResult
{
	GENERATED_BODY()

	// [v1.3.0] 피해가 실제 현재 체력에 적용됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="피해 적용 여부 (bApplied)", ToolTip="True이면 요청 피해량 중 0보다 큰 값이 대상의 현재 체력에서 실제로 차감됐습니다."))
	bool bApplied = false;

	// [v1.3.0] 피해가 적용되지 않았을 때의 거부 사유입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="피해 적용 거부 사유 (RejectReason)", ToolTip="피해가 적용되지 않은 경우 Blocking Hit, DamageData, 자기 피해 정책, 체력 컴포넌트 또는 파괴 상태 중 어느 조건이 원인인지 표시합니다."))
	ECFDamageApplyRejectReason RejectReason = ECFDamageApplyRejectReason::None;

	// [v1.3.0] 피해 적용 대상 Actor입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="피해 대상 Actor (TargetActor)", ToolTip="이번 피해 적용 시도에서 체력을 변경하려고 한 Actor입니다."))
	TObjectPtr<AActor> TargetActor = nullptr;

	// [v1.3.0] 피해 적용에 사용한 DamageData 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="피해 ID (DamageId)", ToolTip="이번 피해 적용에 사용한 DamageData의 DamageId입니다."))
	FName DamageId = NAME_None;

	// [v1.3.0] DamageData가 요청한 원본 직접 피해량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(ClampMin="0.0", DisplayName="요청 피해량 (RequestedDamage)", ToolTip="DamageData.BaseDamage에서 읽은 원본 직접 피해량입니다."))
	float RequestedDamage = 0.0f;

	// [v1.3.0] 체력 하한 0을 반영해 실제로 감소한 피해량입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(ClampMin="0.0", DisplayName="실제 적용 피해량 (AppliedDamage)", ToolTip="현재 체력에서 실제로 감소한 피해량입니다. 남은 체력보다 큰 피해는 남은 체력만큼만 기록됩니다."))
	float AppliedDamage = 0.0f;

	// [v1.3.0] 피해 적용 직전 대상 체력입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(ClampMin="0.0", DisplayName="적용 전 체력 (HealthBefore)", ToolTip="이번 피해 적용 직전 대상 VehicleHealthComp의 현재 체력입니다."))
	float HealthBefore = 0.0f;

	// [v1.3.0] 피해 적용 직후 대상 체력입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(ClampMin="0.0", DisplayName="적용 후 체력 (HealthAfter)", ToolTip="이번 피해 적용 직후 대상 VehicleHealthComp의 현재 체력입니다."))
	float HealthAfter = 0.0f;

	// [v1.3.0] 이번 피해로 대상이 처음 파괴 상태가 됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="이번 타격 파괴 여부 (bDestroyedThisHit)", ToolTip="True이면 이번 피해로 현재 체력이 처음 0 이하가 되어 파괴 상태 이벤트가 발생했습니다."))
	bool bDestroyedThisHit = false;
};

/**
 * Projectile / HitScan 명중 결과를 같은 형식으로 기록하기 위한 최소 런타임 컨텍스트입니다.
 */
USTRUCT(BlueprintType)
struct FCFDamageHitContext
{
	GENERATED_BODY()

	// [v1.0.0] 이번 명중에서 적용할 DamageData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="DamageData (DamageData)", ToolTip="이번 명중에서 직접 피해 적용에 사용할 DamageData입니다. 비어 있으면 피해 적용은 거부되고 HitContext 기록만 유지됩니다."))
	TObjectPtr<UCFDamageData> DamageData = nullptr;

	// [v1.0.0] DamageData가 없을 때도 로그에 남길 피해 프로파일 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="피해 ID (DamageId)", ToolTip="DamageData가 없을 때도 로그와 Debug에 남길 피해 프로파일 ID입니다."))
	FName DamageId = NAME_None;

	// [v1.0.0] 발사한 무기 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="무기 ID (WeaponId)", ToolTip="이번 명중을 만든 무기 ID입니다."))
	FName WeaponId = NAME_None;

	// [v1.0.0] 발사체 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="발사체 ID (ProjectileId)", ToolTip="이번 명중을 만든 발사체 ID입니다. Dummy HitScan이면 None일 수 있습니다."))
	FName ProjectileId = NAME_None;

	// [v1.0.0] 맞은 Actor입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="피격 Actor (HitActor)", ToolTip="이번 명중에서 맞은 Actor이며 VehicleHealthComp 검색 대상입니다."))
	TObjectPtr<AActor> HitActor = nullptr;

	// [v1.2.0] 실제 Trace 또는 Projectile 충돌이 맞춘 컴포넌트 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="피격 컴포넌트 이름 (HitComponentName)", ToolTip="이번 명중에서 실제로 맞은 컴포넌트 이름입니다. 차량 차체 피격이면 보통 SM_Body가 표시됩니다."))
	FName HitComponentName = NAME_None;

	// [v1.0.0] 피격 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="피격 위치 (ImpactLocation)", ToolTip="이번 명중의 월드 피격 위치입니다."))
	FVector ImpactLocation = FVector::ZeroVector;

	// [v1.0.0] 피격 표면 노멀입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="피격 노멀 (ImpactNormal)", ToolTip="이번 명중의 피격 표면 노멀입니다."))
	FVector ImpactNormal = FVector::UpVector;

	// [v1.0.0] 탄이 들어온 월드 방향입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="입사 방향 (IncomingDirection)", ToolTip="이번 명중에서 탄이 들어온 월드 방향입니다."))
	FVector IncomingDirection = FVector::ForwardVector;

	// [v1.0.0] 발사 주체 Actor입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="발사 주체 (InstigatorActor)", ToolTip="이번 명중을 만든 발사 주체 Actor이며 자기 피해 정책 판정에 사용됩니다."))
	TObjectPtr<AActor> InstigatorActor = nullptr;

	// [v1.0.0] Projectile Actor 비행 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(ClampMin="0.0", DisplayName="비행 시간 초 (FlightDurationSeconds)", ToolTip="Projectile Actor 활성화부터 명중까지의 비행 시간입니다. Dummy HitScan이면 0입니다."))
	float FlightDurationSeconds = 0.0f;

	// [v1.0.0] Projectile Actor 충돌에서 온 명중인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="Projectile Actor 명중 여부 (bFromProjectileActor)", ToolTip="True이면 Projectile Actor 충돌에서 온 명중이고, False이면 Dummy HitScan 같은 즉시 판정 경로입니다."))
	bool bFromProjectileActor = false;

	// [v1.1.0] 실제 Blocking Hit이 있었는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="Blocking Hit 여부 (bBlockingHit)", ToolTip="True이면 실제 충돌 또는 HitScan 적중이 있었고, False이면 Trace 끝점 같은 miss 결과입니다."))
	bool bBlockingHit = false;
};
