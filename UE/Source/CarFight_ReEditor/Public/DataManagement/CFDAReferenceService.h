// Copyright (c) CarFight. All Rights Reserved.
// File: CFDAReferenceService.h
// Version: v1.1.0
// Date: 2026-09-03
// Description: CF-FQ-045 DAM-P0-03 on-demand Asset Registry Reference/Referencer query service입니다.
// Changelog:
// - v1.1.0: Asset Registry query bool evidence를 보존해 query failure와 successful-empty evidence를 분리.
// - v1.0.0: InventoryGeneration-bound package dependency/referencer read-only query를 추가.
// Migration:
// - Asset load/save/mutation 없음. Refresh에서는 호출하지 않고 explicit Detail query에서만 사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataManagement/CFDAManagementTypes.h"

// Reference query 자체가 수행됐는지 나타냅니다.
enum class ECFDAReferenceQueryState : uint8
{
	NotQueried,
	Succeeded,
	QueryFailed
};

// 사용자에게 표시할 보수적인 referencer evidence 상태입니다.
enum class ECFDAReferencerState : uint8
{
	NotQueried,
	Referenced,
	NoKnownReferencer,
	QueryFailed
};

// on-demand package-level Reference/Referencer 결과입니다.
struct FCFDAReferenceResult
{
	uint64 InventoryGeneration = 0;
	FString ObjectPath;
	ECFDAReferenceQueryState QueryState = ECFDAReferenceQueryState::NotQueried;
	ECFDAReferencerState ReferencerState = ECFDAReferencerState::NotQueried;
	// GetDependencies가 해당 package dependency node를 성공적으로 조회했는지 나타냅니다.
	bool bReferenceEvidenceAvailable = false;

	// GetReferencers가 해당 package dependency node를 성공적으로 조회했는지 나타냅니다.
	bool bReferencerEvidenceAvailable = false;

	TArray<FString> References;
	TArray<FString> Referencers;
	FString Message;

	bool IsFreshForInventoryGeneration(const uint64 CurrentInventoryGeneration) const
	{
		return InventoryGeneration != 0
			&& InventoryGeneration == CurrentInventoryGeneration;
	}
};

// Asset Registry on-disk package evidence만 읽는 Editor-only service입니다.
class CARFIGHT_REEDITOR_API FCFDAReferenceService
{
public:
	// Asset Registry query 성공 여부와 현재 목록을 보수적인 Manager Reference 상태로 확정합니다.
	static void FinalizeQueryEvidence(
		bool bReferenceQuerySucceeded,
		bool bReferencerQuerySucceeded,
		FCFDAReferenceResult& InOutResult);

	// current Inventory snapshot의 exact ObjectPath에 대해 package dependency/referencer를 on-demand 조회합니다.
	static FCFDAReferenceResult Query(
		const FCFDAInventoryResult& Inventory,
		const FString& ObjectPath);
};
