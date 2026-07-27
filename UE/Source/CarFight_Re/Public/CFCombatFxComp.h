// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-07-25
// Description: CarFight 전투 FX 런타임 컴포넌트
// Scope: 승인된 발사, 첫 유효 Impact와 최초 차량 파괴를 Niagara FX로 표현하고 Scale·Socket·Loop 안전 계약을 적용합니다.
// Changelog:
// - v1.2.0: Niagara User Vector Scale 적용과 차량 SM_Body의 FX_Destroyed 소켓 우선 파괴 위치 해석을 추가.
// - v1.1.0: CombatFxData MaximumLifetimeSeconds 기반 Loop Niagara 강제 제거 안전 퓨즈를 추가.
// - v1.0.0: 데이터 기반 Fire, Impact, Destroyed FX 재생과 파괴 이벤트 구독, null 안전성과 디버그 카운터를 추가.
// Migration:
// - 기존 CombatFxData는 ComponentTransform Scale 모드로 기존 동작을 유지한다.
// - 파괴 FX는 VehicleData.DestroyedFxSocketName 소켓을 우선하고, 없으면 SM_Body Bounds 중심, Actor 위치 순으로 fallback한다.
// - FX 자산이 없거나 생성에 실패해도 발사, 피해와 파괴 판정을 취소하지 않는다.
// - MaximumLifetimeSeconds는 Loop 자산을 정상 일회성 후보로 허용하는 기능이 아니라 영구 잔류 방지 안전장치다.
// - Projectile Trail과 추진 화염은 P0 후속에서 ACFProjectileActor가 소유한다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageTypes.h"
#include "Components/ActorComponent.h"
#include "CFCombatFxComp.generated.h"

class ACFVehiclePawn;
class UCFCombatFxData;
class UCFVehicleData;
class UCFVehicleHealthComp;

/** 차량 전투 결과를 Niagara 기반 시각 연출로 변환하는 프레젠테이션 컴포넌트입니다. */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFCombatFxComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UCFCombatFxComp();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="CarFight|CombatFx", meta=(DisplayName="전투 FX 런타임 초기화 (Initialize Combat FX Runtime)"))
	bool InitializeCombatFxRuntime(ACFVehiclePawn* InOwnerVehiclePawn, UCFVehicleData* InVehicleData, UCFVehicleHealthComp* InVehicleHealthComp);

	UFUNCTION(BlueprintCallable, Category="CarFight|CombatFx", meta=(DisplayName="발사 FX 재생 (Play Fire FX)"))
	bool PlayFireFx(UCFCombatFxData* InCombatFxData, FVector FireLocation, FVector FireDirection);

	UFUNCTION(BlueprintCallable, Category="CarFight|CombatFx", meta=(DisplayName="Impact FX 재생 (Play Impact FX)"))
	bool PlayImpactFx(UCFCombatFxData* InCombatFxData, const FCFDamageHitContext& DamageHitContext);

	UFUNCTION(BlueprintCallable, Category="CarFight|CombatFx", meta=(DisplayName="파괴 FX 재생 (Play Destroyed FX)"))
	bool PlayDestroyedFx(UCFCombatFxData* InCombatFxData, const FCFDamageHitContext& DamageHitContext);

	UFUNCTION(BlueprintPure, Category="CarFight|CombatFx|Debug")
	bool IsCombatFxRuntimeReady() const { return bCombatFxRuntimeReady; }

	UFUNCTION(BlueprintPure, Category="CarFight|CombatFx|Debug")
	FString GetLastCombatFxSummary() const { return LastCombatFxSummary; }

	UFUNCTION(BlueprintPure, Category="CarFight|CombatFx|Debug")
	FString GetLastDestroyedFxSpawnSource() const { return LastDestroyedFxSpawnSource; }

	UFUNCTION(BlueprintPure, Category="CarFight|CombatFx|Debug")
	int32 GetFireFxSpawnCount() const { return FireFxSpawnCount; }

	UFUNCTION(BlueprintPure, Category="CarFight|CombatFx|Debug")
	int32 GetImpactFxSpawnCount() const { return ImpactFxSpawnCount; }

	UFUNCTION(BlueprintPure, Category="CarFight|CombatFx|Debug")
	int32 GetDestroyedFxSpawnCount() const { return DestroyedFxSpawnCount; }

private:
	UFUNCTION()
	void HandleOwnerVehicleDestroyed(FCFDamageHitContext DamageHitContext);

	bool ResolveDestroyedFxTransform(const FCFDamageHitContext& DamageHitContext, FTransform& OutDestroyedFxTransform, FString& OutSpawnSource) const;
	bool SpawnOneShotFx(UCFCombatFxData* InCombatFxData, const FVector& SpawnLocation, const FRotator& BaseRotation, const TCHAR* FxRoleText);
	void UnbindVehicleHealthEvents();

	UPROPERTY(Transient)
	TObjectPtr<ACFVehiclePawn> OwnerVehiclePawn = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCFVehicleHealthComp> BoundVehicleHealthComp = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCFCombatFxData> ActiveDestroyedFxData = nullptr;

	UPROPERTY(Transient)
	FName ActiveDestroyedFxSocketName = TEXT("FX_Destroyed");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|CombatFx|Debug", meta=(AllowPrivateAccess="true"))
	bool bCombatFxRuntimeReady = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|CombatFx|Debug", meta=(AllowPrivateAccess="true"))
	bool bDestroyedFxHandled = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|CombatFx|Debug", meta=(AllowPrivateAccess="true"))
	int32 FireFxSpawnCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|CombatFx|Debug", meta=(AllowPrivateAccess="true"))
	int32 ImpactFxSpawnCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|CombatFx|Debug", meta=(AllowPrivateAccess="true"))
	int32 DestroyedFxSpawnCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|CombatFx|Debug", meta=(AllowPrivateAccess="true"))
	FString LastDestroyedFxSpawnSource = TEXT("DestroyedFxSource: NotResolved");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|CombatFx|Debug", meta=(AllowPrivateAccess="true"))
	FString LastCombatFxSummary = TEXT("CombatFx: NotInitialized");
};
