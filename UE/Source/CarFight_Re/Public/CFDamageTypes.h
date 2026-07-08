// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-07-02
// Description: CarFight Damage 공용 타입 정의
// Scope: DamageData와 명중 이벤트 Debug가 공유할 최소 피해 타입과 HitContext 구조를 제공합니다.
// Changelog:
// - v1.1.0: Dummy HitScan miss와 Projectile hit을 구분하기 위한 bBlockingHit Debug 필드 추가.
// - v1.0.0: DamageData 분리의 첫 단계로 ECFDamageType과 FCFDamageHitContext 타입을 추가.
// Migration:
// - P0에서는 이 타입을 Debug와 데이터 연결 검증에 먼저 사용하고, 실제 HP 차감 / 모듈 손상은 후속 Damage Runtime에서 처리한다.
// - Projectile Actor 충돌과 Dummy HitScan은 같은 FCFDamageHitContext 형식으로 기록한다.

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
 * Projectile / HitScan 명중 결과를 같은 형식으로 기록하기 위한 최소 런타임 컨텍스트입니다.
 */
USTRUCT(BlueprintType)
struct FCFDamageHitContext
{
	GENERATED_BODY()

	// [v1.0.0] 이번 명중에서 적용 후보로 해석된 DamageData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="DamageData (DamageData)", ToolTip="이번 명중에서 적용 후보로 해석된 DamageData입니다. P0에서는 실제 HP 차감 없이 Debug 확인에 사용합니다."))
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="피격 Actor (HitActor)", ToolTip="이번 명중에서 맞은 Actor입니다."))
	TObjectPtr<AActor> HitActor = nullptr;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Damage", meta=(DisplayName="발사 주체 (InstigatorActor)", ToolTip="이번 명중을 만든 발사 주체 Actor입니다."))
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
