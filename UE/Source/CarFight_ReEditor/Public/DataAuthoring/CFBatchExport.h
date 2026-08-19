// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchExport.h
// Version: v1.1.0
// Date: 2026-08-18
// Description: DAUTH-P0-08J~P0-11 canonical UTF-8 CSV + immutable baseline service입니다.
// Scope: Recipe/Profile read-only projection, deterministic export와 reviewed one-cell edited CSV reconstruction을 제공합니다.
// Changelog:
// - v1.1.0: normal Shared Profile Editor가 B2 pipeline을 재사용하도록 immutable manifest 기준 one-cell edited CSV builder 추가.
// - v1.0.0: Section 26.26~26.30 export foundation 최초 구현.
// Migration:
// - File save/import/3-way merge/source commit/Definition Apply를 수행하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFBatchTypes.h"

class UCFVehicleData;
class UCFVehicleRecipeData;

/** Canonical export를 만들 때 필요한 row set과 export identity입니다. */
struct FCFBatchExportRequest
{
	// 한 export snapshot을 식별할 explicit GUID입니다.
	FGuid BatchExportId;

	// 정확히 하나의 Dataset Kind입니다.
	ECFBatchDatasetKind DatasetKind = ECFBatchDatasetKind::VehicleSummaryReport;

	// ProfileNumericEdit일 때 exact single Domain입니다.
	ECFVehicleProfileDomain ProfileDomain = ECFVehicleProfileDomain::None;

	// Row order와 무관하게 builder가 RowId 기준 canonical 정렬할 baseline rows입니다.
	TArray<FCFBatchExportRow> Rows;
};

/** Spreadsheet를 Source Truth로 만들지 않고 export snapshot evidence만 생성하는 pure-style Editor service입니다. */
class CARFIGHT_REEDITOR_API FCFBatchExportService
{
public:
	// Persistent Recipe/Target을 수정하지 않고 RecipeNumericEdit 한 row를 current schema에서 projection합니다.
	static bool BuildRecipeNumericRow(
		const UCFVehicleRecipeData& Recipe,
		const UCFVehicleData* OptionalTargetVehicleData,
		FCFBatchExportRow& OutRow,
		TArray<FString>& OutErrors);

	// Persistent Profile을 수정하지 않고 ProfileNumericEdit 한 row를 current typed schema에서 projection합니다.
	static bool BuildProfileNumericRow(
		const UObject& ProfileObject,
		ECFVehicleProfileDomain ProfileDomain,
		FCFBatchExportRow& OutRow,
		TArray<FString>& OutErrors);

		// Immutable export manifest는 유지한 채 exact one editable row/column cell만 바꾼 canonical CSV text를 재구성합니다.
	static bool BuildEditedCellCsv(
		const FCFBatchExportArtifact& Artifact,
		const FString& RowId,
		const FString& ColumnId,
		const FString& CanonicalValue,
		FString& OutCsvText,
		TArray<FString>& OutErrors);

	// Registry schema와 row baseline으로 canonical CSV/UTF-8/manifest/hash를 deterministic하게 생성합니다.
	static bool BuildExport(
		const FCFBatchExportRequest& Request,
		FCFBatchExportArtifact& OutArtifact,
		TArray<FString>& OutErrors);

	// Manifest semantic baseline을 다시 hash/검사해 손상 또는 descriptor mismatch를 탐지합니다.
	static bool ValidateManifestBaseline(const FCFBatchManifest& Manifest, TArray<FString>& OutErrors);

	// ExportSetHash authority를 manifest semantic data에서 deterministic하게 계산합니다.
	static FString BuildExportSetHash(const FCFBatchManifest& Manifest);
};
