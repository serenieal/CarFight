// Copyright (c) CarFight. All Rights Reserved.
//
// Step-1: BP_ModularVehicle -> C++ base class (skeleton)
// - Goal: Provide a stable C++ parent so BP can be thinned down later.
// - This step intentionally does NOT move logic yet (Tick / inputs stay in BP for now).

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h" // ChaosVehicles
#include "CFModVehiclePawn.generated.h"

class UCFWheelSyncComp;

/**
 * C++ base for BP_ModularVehicle.
 *
 * Step-1 scope:
 * - Only provides a parent class and stable access points.
 * - Avoid creating duplicate components that BP may already own.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API ACFModVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	ACFModVehiclePawn();

protected:
	virtual void BeginPlay() override;

public:
	/**
	 * Cached pointer to WheelSyncComp (owned by BP or inherited later).
	 *
	 * NOTE: We deliberately do NOT CreateDefaultSubobject here in Step-1
	 * to avoid duplicating an existing BP component named WheelSyncComp.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category="CarFight|Vehicle", meta=(AllowPrivateAccess="true", DisplayName="휠 동기화 컴포넌트 (WheelSyncComp)", ToolTip="현재 Pawn에서 캐시한 WheelSyncComp 참조입니다. BP가 소유한 컴포넌트를 안전하게 찾아 재사용할 때 사용합니다."))
	TObjectPtr<UCFWheelSyncComp> WheelSyncComp = nullptr;

	/** BP should use this accessor instead of directly wiring variables. */
	UFUNCTION(BlueprintPure, Category="CarFight|Vehicle")
	UCFWheelSyncComp* GetWheelSyncComp() const { return WheelSyncComp; }

	/** Refresh cached component pointer (safe to call multiple times). */
	UFUNCTION(BlueprintCallable, Category="CarFight|Vehicle")
	void RefreshWheelSyncComp();
};
