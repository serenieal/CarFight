// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAAmmoDaceBase.cpp
// Version: v1.0.0
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-04 AmmoData production-owned append-only accepted DACE history authority입니다.
// Changelog:
// - v1.0.0: 기존 CFDAAmmoDace.cpp의 DACE-AmmoData bootstrap exact1을 값/signature 변경 없이 dedicated append-only owner로 분리했습니다.
// Migration:
// - 기존 DACE-AmmoData-S1-A1-Bootstrap record의 모든 field와 signature는 immutable baseline입니다.
// - 새 accepted contract는 기존 record를 수정하지 않고 이 history 뒤에 append해야 합니다.

#include "CFDAAmmoDace.h"

// DACE-AmmoData independent append-only accepted history를 반환합니다.
const TArray<FCFDAAcceptedContractSnapshot>& FCFDAAmmoDace::GetAcceptedSnapshots()
{
	// DAO-P0-04 current exact contract를 migration impact 없음으로 승인한 immutable bootstrap exact1입니다.
	static const TArray<FCFDAAcceptedContractSnapshot> Snapshots =
	{
		{
			TEXT("DACE-AmmoData-S1-A1-Bootstrap"),
			TEXT(""),
			TEXT("CarFight.DataAsset.AmmoData"),
			1,
			1,
			TEXT("/Script/CarFight_Re.CFAmmoData"),
			TEXT("sha256:cd2aab5d0899a7c8c6def7e46bc90c69041801012b4fe874b25d2dd762d92339"),
			TEXT("sha256:a0541028ddf1dd8b760e440f9af80456e7d1d8ae6d133c2727f67b9052d34073"),
			TEXT("sha256:9199d6eaaa37fb5a5493f85d230ce3cfff56543928d9b118902d188615097b4b"),
			TEXT("sha256:22cab6b7fc8951cd9d556fdfd32781570b17fb5c40f864a4df331a668e48ceff"),
			ECFDAContractMigrationImpact::NoMigration,
			ECFDAContractMigrationResolution::NotRequired,
			TEXT(""),
			TEXT("sha256:b6de5965fefe3ddf25c2318e9e426bb4f67072f3e5d8da4278cfcab15973cdd3")
		}
	};
	return Snapshots;
}
