// Copyright (c) CarFight. All Rights Reserved.
// File: CFDADamageDaceBase.cpp
// Version: v1.0.0
// Date: 2026-09-11
// Description: CF-FQ-052 DDO-P0-03 DamageData production-owned append-only accepted DACE history authority입니다.
// Changelog:
// - v1.0.0: DACE-DamageData-S1-A1-Bootstrap exact1의 descriptor component signatures와 snapshot chain signature를 최초 freeze했습니다.
// Migration:
// - 이 bootstrap exact1의 모든 field/signature는 immutable baseline입니다.
// - 새 accepted Damage contract는 기존 record를 수정/rebaseline하지 않고 이 history 뒤에 append해야 합니다.
// - Missile CFDAContractBase.cpp와 Ammo CFDAAmmoDaceBase.cpp accepted history는 별도 authority로 유지합니다.

#include "CFDADamageDace.h"

// DACE-DamageData independent append-only accepted history를 반환합니다.
const TArray<FCFDAAcceptedContractSnapshot>& FCFDADamageDace::GetAcceptedSnapshots()
{
	// DDO-P0-03 current exact contract를 migration impact 없음으로 승인한 immutable bootstrap exact1입니다.
	static const TArray<FCFDAAcceptedContractSnapshot> Snapshots =
	{
		{
			TEXT("DACE-DamageData-S1-A1-Bootstrap"),
			TEXT(""),
			TEXT("CarFight.DataAsset.DamageData"),
			1,
			1,
			TEXT("/Script/CarFight_Re.CFDamageData"),
			TEXT("sha256:1082759fbf56bcd3a1fa7e1f2beab95a2fdc08738eaec81fe589e8c8294db663"),
			TEXT("sha256:da4d501862867d6b5da4adf6f233d2cd107b39073efe1061850d42e6537e888a"),
			TEXT("sha256:ad3edca41d0d1083b8795c1afc07d84bf826457202e161dc9b73f6a1d8f09db7"),
			TEXT("sha256:3f6d76d67ac3749ab3abd9e4a8fccdd4ed242d44202ed8a71f7203e5e69a159a"),
			ECFDAContractMigrationImpact::NoMigration,
			ECFDAContractMigrationResolution::NotRequired,
			TEXT(""),
			TEXT("sha256:d3bc8e2c004d5e86a4339f3869badd9a146af5e0bdbe7faf476f14914dfea7ee")
		}
	};
	return Snapshots;
}
