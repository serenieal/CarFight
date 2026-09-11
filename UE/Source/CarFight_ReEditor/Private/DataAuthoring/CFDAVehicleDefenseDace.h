// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAVehicleDefenseDace.h
// Version: v1.0.0
// Date: 2026-09-11
// Description: CF-FQ-053 VDR-P0-02 VehicleDefenseData independent DACE descriptor/history authority 계약입니다.
// Changelog:
// - v1.0.0: VehicleDefense SourceShape exact29, AdapterShape exact37, SourceAdapterMapping exact30, SemanticContract exact14와 independent accepted bootstrap/current migration gate API를 최초 추가했습니다.
// Migration:
// - Editor Private internal contract입니다. shared FCFDAContractGuard와 Missile/Ammo/Damage accepted history는 변경하지 않습니다.
// - DACE-VehicleDefenseData-S1-A1-Bootstrap exact1 accepted history는 CFDAVehicleDefenseDaceBase.cpp dedicated append-only owner가 소유합니다.
// - Product canonical VehicleDefense DACE target exact0과 persisted protected VehicleDefense exact1은 서로 다른 authority로 유지합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFDAContractGuard.h"

/** VehicleDefenseData exact TypeKey의 independent DACE contract authority입니다. */
class FCFDAVehicleDefenseDace
{
public:
	// Current VehicleDefenseData recursive authored SourceShape exact29 descriptor를 반환합니다.
	static const TArray<FCFDASourceFieldDescriptor>& GetSourceShapeDescriptor();

	// Current VehicleDefenseData strict whole-record AdapterShape exact37 descriptor를 반환합니다.
	static const TArray<FCFDAAdapterFieldDescriptor>& GetAdapterShapeDescriptor();

	// Current VehicleDefenseData Source↔Adapter exact30 mapping descriptor를 반환합니다.
	static const TArray<FCFDASourceAdapterMappingDescriptor>& GetSourceAdapterMappingDescriptor();

	// Current VehicleDefenseData authored SemanticContract exact14 descriptor를 반환합니다.
	static const TArray<FCFDASemanticRuleDescriptor>& GetSemanticContractDescriptor();

	// Current VehicleDefense descriptor exact4 component signatures를 shared canonical algorithm으로 계산합니다.
	static bool BuildCurrentSignatures(FCFDAContractSignatures& OutSignatures, FString& OutError);

	// DACE-VehicleDefenseData independent append-only accepted history를 반환합니다.
	static const TArray<FCFDAAcceptedContractSnapshot>& GetAcceptedSnapshots();

	// Current accepted baseline 대비 미승인 contract delta declaration을 반환합니다. Bootstrap과 동일한 current 상태에서는 nullptr입니다.
	static const FCFDACurrentChangeDeclaration* GetCurrentChangeDeclaration();

	// Recursive Reflection/mapping/accepted-chain/revision을 결합해 current VehicleDefense DACE contract를 fail-closed 검증합니다.
	static FCFDAContractGuardResult ValidateCurrentContract();

	// Product canonical exact0 compatibility와 no-delta migration state를 결합해 append/promotion gate를 평가합니다.
	static FCFDAMigrationGateResult EvaluateCurrentMigrationGate();
};
