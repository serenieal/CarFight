// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleEngineCurveUtils.h
// Version: v1.1.0
// Date: 2026-09-01
// Description: CarFight vehicle-specific engine torque curve typed payload를 검증하고 UE 5.8 Chaos EngineSetup.TorqueCurve로 변환하는 Runtime 공통 유틸리티입니다.
// Changelog:
// - v1.1.0: current/proposed Chaos TorqueCurve의 key shape equality를 비교해 불필요한 PhysicsState 재생성을 피하는 helper 추가.
// - v1.0.0: normalized torque point 구조 검증과 FRuntimeFloatCurve 적용을 최초 구현.
// Migration:
// - bUseEngineTorqueCurve=false인 기존 VehicleData는 이 유틸리티를 호출해도 기존 BP/Chaos TorqueCurve를 유지합니다.

#pragma once

#include "CoreMinimal.h"

struct FCFVehicleEngineTorqueCurve;
struct FVehicleEngineConfig;

/** VehicleData Engine Torque Curve의 구조 검증과 Chaos 변환을 소유합니다. */
struct CARFIGHT_RE_API FCFVehicleEngineCurveUtils
{
	// RPM은 엄격 증가하고 TorqueMultiplier는 0..1인 최소 2개 point인지 검증합니다.
	static bool ValidateCurve(const FCFVehicleEngineTorqueCurve& Curve, FString& OutError);

	// 검증된 CarFight typed curve를 Chaos EngineSetup TorqueCurve에 exact 적용합니다.
	static bool ApplyCurveToChaos(const FCFVehicleEngineTorqueCurve& Curve, FVehicleEngineConfig& InOutEngineConfig, FString& OutError);

	// 두 Chaos EngineSetup의 TorqueCurve key 시간/값/보간이 동일한지 비교합니다.
	static bool AreTorqueCurvesEquivalent(const FVehicleEngineConfig& LeftEngineConfig, const FVehicleEngineConfig& RightEngineConfig);
};
