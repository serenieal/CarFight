// Copyright Epic Games, Inc. All Rights Reserved.
// Version: 1.0.0
// Notes:
// - Added missing constructor definition for AvehicleTestSportsCar.
// - This file restores the C++ parent implementation required by the migrated Blueprint.

#include "vehicleTestSportsCar.h"
#include "vehicleTestSportsWheelFront.h"
#include "vehicleTestSportsWheelRear.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

AvehicleTestSportsCar::AvehicleTestSportsCar()
{
	// Get the inherited Chaos Vehicle movement component.
	UChaosWheeledVehicleMovementComponent* SportsCarMovementComponent = GetChaosVehicleMovement().Get();
	if (!SportsCarMovementComponent)
	{
		return;
	}

	// Tune the inherited camera boom positions for the sports car profile.
	GetFrontSpringArm()->SetRelativeLocation(FVector(0.0f, -25.0f, 115.0f));
	GetBackSpringArm()->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	GetBackSpringArm()->TargetArmLength = 500.0f;

	// Configure the chassis behavior.
	SportsCarMovementComponent->ChassisHeight = 144.0f;
	SportsCarMovementComponent->DragCoefficient = 0.31f;
	SportsCarMovementComponent->DownforceCoefficient = 0.7f;
	SportsCarMovementComponent->CenterOfMassOverride = FVector(0.0f, 0.0f, -5.0f);
	SportsCarMovementComponent->bEnableCenterOfMassOverride = true;

	// Configure wheel classes and bone bindings.
	SportsCarMovementComponent->bLegacyWheelFrictionPosition = false;
	SportsCarMovementComponent->WheelSetups.SetNum(4);

	SportsCarMovementComponent->WheelSetups[0].WheelClass = UvehicleTestSportsWheelFront::StaticClass();
	SportsCarMovementComponent->WheelSetups[0].BoneName = FName("PhysWheel_FL");
	SportsCarMovementComponent->WheelSetups[0].AdditionalOffset = FVector::ZeroVector;

	SportsCarMovementComponent->WheelSetups[1].WheelClass = UvehicleTestSportsWheelFront::StaticClass();
	SportsCarMovementComponent->WheelSetups[1].BoneName = FName("PhysWheel_FR");
	SportsCarMovementComponent->WheelSetups[1].AdditionalOffset = FVector::ZeroVector;

	SportsCarMovementComponent->WheelSetups[2].WheelClass = UvehicleTestSportsWheelRear::StaticClass();
	SportsCarMovementComponent->WheelSetups[2].BoneName = FName("PhysWheel_BL");
	SportsCarMovementComponent->WheelSetups[2].AdditionalOffset = FVector::ZeroVector;

	SportsCarMovementComponent->WheelSetups[3].WheelClass = UvehicleTestSportsWheelRear::StaticClass();
	SportsCarMovementComponent->WheelSetups[3].BoneName = FName("PhysWheel_BR");
	SportsCarMovementComponent->WheelSetups[3].AdditionalOffset = FVector::ZeroVector;

	// Configure engine tuning.
	SportsCarMovementComponent->EngineSetup.MaxTorque = 500.0f;
	SportsCarMovementComponent->EngineSetup.MaxRPM = 4500.0f;
	SportsCarMovementComponent->EngineSetup.EngineIdleRPM = 1200.0f;
	SportsCarMovementComponent->EngineSetup.EngineBrakeEffect = 0.05f;
	SportsCarMovementComponent->EngineSetup.EngineRevUpMOI = 5.0f;
	SportsCarMovementComponent->EngineSetup.EngineRevDownRate = 600.0f;

	// Configure differential behavior.
	SportsCarMovementComponent->DifferentialSetup.DifferentialType = EVehicleDifferential::RearWheelDrive;
	SportsCarMovementComponent->DifferentialSetup.FrontRearSplit = 0.65f;

	// Configure steering feel.
	SportsCarMovementComponent->SteeringSetup.SteeringType = ESteeringType::AngleRatio;
	SportsCarMovementComponent->SteeringSetup.AngleRatio = 0.8f;
}
