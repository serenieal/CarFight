// Copyright (c) CarFight. All Rights Reserved.
//
// Step-1: BP_ModularVehicle -> C++ base class (skeleton)

#include "CFModVehiclePawn.h"

// [Component]
#include "CFWheelSyncComp.h"

#include "GameFramework/Actor.h"

ACFModVehiclePawn::ACFModVehiclePawn()
{
	// Step-1 policy:
	// - Do NOT create a default subobject for WheelSyncComp.
	//   BP_ModularVehicle already has a component variable named WheelSyncComp
	//   (confirmed via BP graph dump). Creating another one here can cause double-up.
	// - We only cache it at runtime.
}

void ACFModVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	RefreshWheelSyncComp();
}

void ACFModVehiclePawn::RefreshWheelSyncComp()
{
	// Find the first WheelSyncComp attached to this Actor.
	WheelSyncComp = FindComponentByClass<UCFWheelSyncComp>();
}
