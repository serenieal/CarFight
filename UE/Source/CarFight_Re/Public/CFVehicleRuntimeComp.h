// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-14
// Description: CF-FQ-048 VPS-P0-03 차량 Runtime orchestration 내부 coordinator
// Changelog:
// - v1.0.0: Initialize/Refresh, Initial Mass prepare/verify, VehicleData 적용 순서를 Pawn 공개 표면 변경 없이 내부 coordinator로 추출.
// Migration:
// - ACFVehiclePawn의 기본 서브오브젝트로 자동 생성되며 Blueprint/Product Asset에 수동 추가하거나 저장할 필요가 없습니다.
// - 기존 InitializeVehicleRuntime/RefreshFittingDependentRuntime 및 Automation facade는 ACFVehiclePawn에 그대로 유지됩니다.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CFVehicleRuntimeComp.generated.h"

class ACFVehiclePawn;

/**
 * 차량 Runtime 초기화 순서와 재구성 행동만 수행하는 내부 coordinator입니다.
 * - Runtime observable/serialized state Authority는 ACFVehiclePawn에 유지합니다.
 * - 독립 Tick/BeginPlay/Blueprint 공개 API를 만들지 않습니다.
 */
UCLASS(ClassGroup=(CarFight))
class CARFIGHT_RE_API UCFVehicleRuntimeComp : public UActorComponent
{
	GENERATED_BODY()

	friend class ACFVehiclePawn;

public:
	// [v1.0.0] 독립 Tick 없이 Pawn lifecycle에서만 호출되는 내부 Runtime coordinator를 초기화합니다.
	UCFVehicleRuntimeComp();

private:
	// [v1.0.0] 기존 Pawn Runtime 초기화 순서를 그대로 실행하고 결과를 Pawn Authority 상태에 기록합니다.
	bool InitializeVehicleRuntime(ACFVehiclePawn& VehiclePawn);

	// [v1.0.0] 이미 Commit된 Fitting 기준 장비 의존 Runtime만 다시 구성합니다.
	bool RefreshFittingDependentRuntime(ACFVehiclePawn& VehiclePawn);

	// [v1.0.0] PreRegister Super 호출 전에 Initial Sortie 질량을 준비해 Chaos Movement에 기록합니다.
	bool PrepareInitialSortieRuntimeMass(ACFVehiclePawn& VehiclePawn);

	// [v1.0.0] BeginPlay Runtime 초기화에서 Initial Sortie 질량과 실제 Physics 상태를 검증합니다.
	bool VerifyInitialSortieRuntimeMass(ACFVehiclePawn& VehiclePawn);

	// [v1.0.0] VehicleData의 Movement→Reference→WheelPhysics→WheelVisual→TurretVisual→DriveState 적용 순서를 보존합니다.
	void ApplyVehicleDataConfig(ACFVehiclePawn& VehiclePawn);
};
