// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.13.0
// Date: 2026-04-01
// Description: 레거시 VehicleMovement 실험값을 현재 프로젝트 차량 기준값으로 자동 보정합니다.
// Scope: CFVehicleData PostLoad 기반 마이그레이션, 프로젝트 기준 기본값 정렬, 에디터 저장 유도

#include "CFVehicleData.h"

#include "UObject/Package.h"

namespace CFVehicleDataMigration
{
	// 부동소수 비교에서 허용할 오차 범위입니다.
	constexpr float FloatTolerance = KINDA_SMALL_NUMBER;

	// 두 부동소수 값이 사실상 같은지 판정합니다.
	bool IsNearlyEqualFloat(const float LeftValue, const float RightValue)
	{
		return FMath::IsNearlyEqual(LeftValue, RightValue, FloatTolerance);
	}

	// 현재 설정이 과거 임시 실험값 세트와 같은지 판정합니다.
	bool IsLegacyPrototypeMovementConfig(const FCFVehicleMovementConfig& VehicleMovementConfig)
	{
		return VehicleMovementConfig.bUseMovementOverrides
			&& VehicleMovementConfig.MovementProfileName.IsNone()
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelMaxSteerAngle, 40.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelMaxBrakeTorque, 4500.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelMaxBrakeTorque, 4500.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelMaxHandBrakeTorque, 6000.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelRadius, 39.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelRadius, 39.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelWidth, 35.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelWidth, 35.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelFrictionForceMultiplier, 3.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelFrictionForceMultiplier, 3.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelCorneringStiffness, 750.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelCorneringStiffness, 750.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelLoadRatio, 1.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelLoadRatio, 1.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelSpringRate, 100.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelSpringRate, 100.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelSpringPreload, 100.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelSpringPreload, 100.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelSuspensionMaxRaise, 20.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelSuspensionMaxRaise, 20.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelSuspensionMaxDrop, 20.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelSuspensionMaxDrop, 20.0f)
			&& VehicleMovementConfig.bFrontWheelAffectedByEngine
			&& !VehicleMovementConfig.bRearWheelAffectedByEngine
			&& VehicleMovementConfig.FrontWheelSweepShape == ESweepShape::Shapecast
			&& VehicleMovementConfig.RearWheelSweepShape == ESweepShape::Shapecast
			&& IsNearlyEqualFloat(VehicleMovementConfig.ChassisHeight, 160.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.DragCoefficient, 0.1f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.DownforceCoefficient, 0.1f)
			&& !VehicleMovementConfig.bEnableCenterOfMassOverride
			&& VehicleMovementConfig.CenterOfMassOverride.IsZero()
			&& IsNearlyEqualFloat(VehicleMovementConfig.EngineMaxTorque, 500.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.EngineMaxRPM, 4500.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.EngineIdleRPM, 1200.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.EngineBrakeEffect, 0.05f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.EngineRevUpMOI, 5.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.EngineRevDownRate, 600.0f)
			&& VehicleMovementConfig.DifferentialType == EVehicleDifferential::RearWheelDrive
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontRearSplit, 0.5f)
			&& VehicleMovementConfig.SteeringType == ESteeringType::AngleRatio
			&& IsNearlyEqualFloat(VehicleMovementConfig.SteeringAngleRatio, 0.7f)
			&& VehicleMovementConfig.bLegacyWheelFrictionPosition
			&& VehicleMovementConfig.FrontWheelAdditionalOffset.IsZero()
			&& VehicleMovementConfig.RearWheelAdditionalOffset.IsZero();
	}

	// 현재 프로젝트 WheelClass와 BP_CFVehiclePawn 기준값을 VehicleMovement 설정에 채워 넣습니다.
	void ApplyProjectBaselineMovementConfig(FCFVehicleMovementConfig& VehicleMovementConfig)
	{
		VehicleMovementConfig.FrontWheelMaxSteerAngle = 35.0f;
		VehicleMovementConfig.FrontWheelMaxBrakeTorque = 1500.0f;
		VehicleMovementConfig.RearWheelMaxBrakeTorque = 1500.0f;
		VehicleMovementConfig.RearWheelMaxHandBrakeTorque = 3000.0f;
		VehicleMovementConfig.FrontWheelRadius = 30.0f;
		VehicleMovementConfig.RearWheelRadius = 30.0f;
		VehicleMovementConfig.FrontWheelWidth = 6.0f;
		VehicleMovementConfig.RearWheelWidth = 6.0f;
		VehicleMovementConfig.FrontWheelFrictionForceMultiplier = 2.0f;
		VehicleMovementConfig.RearWheelFrictionForceMultiplier = 2.0f;
		VehicleMovementConfig.FrontWheelCorneringStiffness = 1000.0f;
		VehicleMovementConfig.RearWheelCorneringStiffness = 1000.0f;
		VehicleMovementConfig.FrontWheelLoadRatio = 0.5f;
		VehicleMovementConfig.RearWheelLoadRatio = 0.5f;
		VehicleMovementConfig.FrontWheelSpringRate = 250.0f;
		VehicleMovementConfig.RearWheelSpringRate = 250.0f;
		VehicleMovementConfig.FrontWheelSpringPreload = 50.0f;
		VehicleMovementConfig.RearWheelSpringPreload = 50.0f;
		VehicleMovementConfig.FrontWheelSuspensionMaxRaise = 8.0f;
		VehicleMovementConfig.RearWheelSuspensionMaxRaise = 10.0f;
		VehicleMovementConfig.FrontWheelSuspensionMaxDrop = 10.0f;
		VehicleMovementConfig.RearWheelSuspensionMaxDrop = 10.0f;
		VehicleMovementConfig.bFrontWheelAffectedByEngine = false;
		VehicleMovementConfig.bRearWheelAffectedByEngine = true;
		VehicleMovementConfig.FrontWheelSweepShape = ESweepShape::Raycast;
		VehicleMovementConfig.RearWheelSweepShape = ESweepShape::Raycast;
		VehicleMovementConfig.ChassisHeight = 140.0f;
		VehicleMovementConfig.DragCoefficient = 0.3f;
		VehicleMovementConfig.DownforceCoefficient = 0.3f;
		VehicleMovementConfig.bEnableCenterOfMassOverride = false;
		VehicleMovementConfig.CenterOfMassOverride = FVector::ZeroVector;
		VehicleMovementConfig.EngineMaxTorque = 750.0f;
		VehicleMovementConfig.EngineMaxRPM = 7000.0f;
		VehicleMovementConfig.EngineIdleRPM = 900.0f;
		VehicleMovementConfig.EngineBrakeEffect = 0.2f;
		VehicleMovementConfig.EngineRevUpMOI = 5.0f;
		VehicleMovementConfig.EngineRevDownRate = 600.0f;
		VehicleMovementConfig.DifferentialType = EVehicleDifferential::RearWheelDrive;
		VehicleMovementConfig.FrontRearSplit = 0.5f;
		VehicleMovementConfig.SteeringType = ESteeringType::AngleRatio;
		VehicleMovementConfig.SteeringAngleRatio = 0.7f;
		VehicleMovementConfig.bLegacyWheelFrictionPosition = true;
		VehicleMovementConfig.FrontWheelAdditionalOffset = FVector::ZeroVector;
		VehicleMovementConfig.RearWheelAdditionalOffset = FVector::ZeroVector;
	}
}

// 레거시 VehicleMovement 실험값 자산을 현재 프로젝트 기준값으로 보정합니다.
void UCFVehicleData::PostLoad()
{
	Super::PostLoad();

	// 현재 로드된 VehicleMovement 설정 참조입니다.
	FCFVehicleMovementConfig& MutableVehicleMovementConfig = this->VehicleMovementConfig;
	if (!CFVehicleDataMigration::IsLegacyPrototypeMovementConfig(MutableVehicleMovementConfig))
	{
		return;
	}

	Modify();
	CFVehicleDataMigration::ApplyProjectBaselineMovementConfig(MutableVehicleMovementConfig);

#if WITH_EDITOR
	MarkPackageDirty();
#endif
}
