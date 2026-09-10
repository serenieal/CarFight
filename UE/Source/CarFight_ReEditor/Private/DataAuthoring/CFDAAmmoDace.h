// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAAmmoDace.h
// Version: v1.1.0
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-04 AmmoData independent DACE descriptor/history authority 계약입니다.
// Changelog:
// - v1.1.0: accepted history 구현을 CFDAAmmoDaceBase.cpp dedicated append-only owner로 분리하고 기존 public/internal API를 유지했습니다.
// - v1.0.0: Ammo SourceShape/AdapterShape/SourceAdapterMapping/SemanticContract, independent accepted bootstrap와 current migration gate를 최초 추가.
// Migration:
// - Editor Private internal contract입니다. Missile FCFDAContractGuard compatibility facade와 CFDAContractBase.cpp accepted history는 변경하지 않습니다.
// - 기존 DACE-AmmoData-S1-A1-Bootstrap 값/signature는 immutable이며 CFDAAmmoDaceBase.cpp에서만 소유합니다.
// - Ammo Product canonical target exact0은 provider-owned explicit empty set으로 유지합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFDAContractGuard.h"

/** AmmoData exact TypeKey의 independent DACE contract authority입니다. */
class FCFDAAmmoDace
{
public:
	// Current AmmoData direct authored SourceShape exact8 descriptor를 반환합니다.
	static const TArray<FCFDASourceFieldDescriptor>& GetSourceShapeDescriptor();

	// Current AmmoData strict whole-record AdapterShape descriptor를 반환합니다.
	static const TArray<FCFDAAdapterFieldDescriptor>& GetAdapterShapeDescriptor();

	// Current AmmoData Source↔Adapter exact mapping descriptor를 반환합니다.
	static const TArray<FCFDASourceAdapterMappingDescriptor>& GetSourceAdapterMappingDescriptor();

	// Current AmmoData authored semantic policy descriptor를 반환합니다.
	static const TArray<FCFDASemanticRuleDescriptor>& GetSemanticContractDescriptor();

	// Current Ammo descriptor exact4 component signatures를 계산합니다.
	static bool BuildCurrentSignatures(FCFDAContractSignatures& OutSignatures, FString& OutError);

	// DACE-AmmoData independent append-only accepted history를 반환합니다.
	static const TArray<FCFDAAcceptedContractSnapshot>& GetAcceptedSnapshots();

	// Current accepted baseline 대비 미승인 contract delta declaration을 반환합니다. 현재 baseline과 동일하므로 nullptr입니다.
	static const FCFDACurrentChangeDeclaration* GetCurrentChangeDeclaration();

	// Reflection/mapping/accepted-chain/revision을 결합해 current Ammo DACE contract를 fail-closed 검증합니다.
	static FCFDAContractGuardResult ValidateCurrentContract();

	// Product canonical exact0 compatibility와 migration state를 결합해 append/promotion gate를 평가합니다.
	static FCFDAMigrationGateResult EvaluateCurrentMigrationGate();
};
