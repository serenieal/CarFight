// Copyright Epic Games, Inc. All Rights Reserved.

#include "vehicleTestWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UvehicleTestWheelFront::UvehicleTestWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}