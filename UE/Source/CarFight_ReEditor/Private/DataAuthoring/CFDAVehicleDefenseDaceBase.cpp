// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAVehicleDefenseDaceBase.cpp
// Version: v1.0.0
// Date: 2026-09-11
// Description: CF-FQ-053 VDR-P0-02 VehicleDefenseData production-owned append-only accepted DACE history authority입니다.
// Changelog:
// - v1.0.0: SchemaRevision 1 / AdapterContractRevision 1 VehicleDefense bootstrap accepted snapshot exact1을 independent history로 최초 추가했습니다.
// Migration:
// - 새 VehicleDefense accepted contract는 기존 record를 수정하지 않고 이 history 뒤에 append해야 합니다.
// - Missile/Ammo/Damage accepted history와 record signature를 변경하지 않습니다.

#include "CFDAVehicleDefenseDace.h"

// DACE-VehicleDefenseData production-owned append-only accepted snapshot history를 반환합니다.
const TArray<FCFDAAcceptedContractSnapshot>& FCFDAVehicleDefenseDace::GetAcceptedSnapshots()
{
	// VDR-P0-02 current VehicleDefense S1/A1 contract를 bootstrap으로 동결한 accepted snapshot exact1입니다.
	static const TArray<FCFDAAcceptedContractSnapshot> Snapshots =
	{
		{
			TEXT("DACE-VehicleDefenseData-S1-A1-Bootstrap"),
			TEXT(""),
			TEXT("CarFight.DataAsset.VehicleDefenseData"),
			1,
			1,
			TEXT("/Script/CarFight_Re.CFVehicleDefenseData"),
			TEXT("sha256:d96af45c383bee9ba47d4942a29895c13031218142736965dd7173134ea027f4"),
			TEXT("sha256:5e394b556ebd9fb94c94c22d729bb0d24acb8dd9770f42cc117652fc68a9f221"),
			TEXT("sha256:f60ae70c33594342803824d9db714b69d4951e0fbd3a325bea18ef19f2ffd176"),
			TEXT("sha256:480551a50d3ea141a691efc694eb7541bb081bb60c7e95e73bc484adc581fe5e"),
			ECFDAContractMigrationImpact::NoMigration,
			ECFDAContractMigrationResolution::NotRequired,
			TEXT(""),
			TEXT("sha256:56b9c2905e025df97ba61934bc9c5fd31a7b3da7e7c902db9baa1b326c92fd60")
		}
	};
	return Snapshots;
}
