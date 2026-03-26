// Copyright Epic Games, Inc. All Rights Reserved.

#include "vehicleTestWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UvehicleTestWheelRear::UvehicleTestWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}