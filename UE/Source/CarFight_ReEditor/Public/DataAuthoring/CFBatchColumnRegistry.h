// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchColumnRegistry.h
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08J Section 26.23~26.25 Batch Column projection registry입니다.
// Scope: Reserved metadata, Recipe numeric allowlist, typed Profile numeric projection, 117 Field Registry report projection을 제공합니다.
// Changelog:
// - v1.0.0: FCFBatchColumnRegistry 최초 구현 계약.
// Migration:
// - 이 Registry는 Recipe/Profile/FCFVehicleFieldRegistry의 projection이며 새 Source Truth를 소유하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFBatchTypes.h"

/** Existing Authoring schema/Field Registry를 Batch transport metadata로 projection하는 정적 Registry입니다. */
class CARFIGHT_REEDITOR_API FCFBatchColumnRegistry
{
public:
	// Dataset/Profile Domain의 canonical CSV column descriptor를 stable ColumnId 순서로 반환합니다.
	static bool GetDatasetColumns(
		ECFBatchDatasetKind DatasetKind,
		ECFVehicleProfileDomain ProfileDomain,
		TArray<FCFBatchColumnDescriptor>& OutColumns,
		TArray<FString>& OutErrors);

	// Existing FCFVehicleFieldRegistry 117 descriptors를 read-only report metadata로 projection합니다.
	static bool GetResolvedFieldProjections(
		TArray<FCFBatchResolvedProjection>& OutProjections,
		TArray<FString>& OutErrors);

	// Dataset/Profile Domain의 stable schema id를 반환합니다.
	static FString GetSchemaId(ECFBatchDatasetKind DatasetKind, ECFVehicleProfileDomain ProfileDomain);

	// P0-08J Batch schema revision입니다.
	static int32 GetSchemaRevision() { return 1; }

	// __cf_ importer-owned reserved technical prefix인지 검사합니다.
	static bool IsReservedColumnId(const FString& ColumnId);

	// Duplicate/reserved collision/editability invariant를 검사합니다.
	static bool ValidateDescriptorSet(const TArray<FCFBatchColumnDescriptor>& Columns, TArray<FString>& OutErrors);
};
