// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentCore.h
// Version: v1.2.0
// Date: 2026-09-30
// Description: CF-FQ-058 CCAS canonical parsing, validation, planning metadata/lifecycle hashing과 schema migration API입니다.
// Changelog:
// - v1.2.0: P0-06 PlanningMetadata/Lifecycle와 optional BaseCatalogSnapshotFingerprint shape validation을 추가.
// - v1.1.0: typed ChangeSet validation과 WorkbookSource/authoring metadata contract 검증 seam을 추가.
// - v1.0.0: tri-state cell parser, schema/content fail-closed validator, semantic hash, migration preview 계약을 최초 구현.
// Migration:
// - 모든 기능은 in-memory canonical model에만 작동하며 Product UObject mutation을 수행하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFContentTypes.h"

/** Workbook cell text를 schema-declared canonical value로 변환합니다. */
class FCFContentCanonicalizer
{
public:
	// blank/@none/@empty/@@ escape와 typed VALUE를 deterministic canonical value로 parse합니다.
	static bool ParseCell(
		const FString& CellText,
		const FCFContentFieldDescriptor& FieldDescriptor,
		FCFContentValue& OutValue,
		FString& OutError);
};

/** Canonical schema/model의 fail-closed invariant를 검증합니다. */
class FCFContentSchemaValidator
{
public:
	// Workbook schema와 record/collection identity/value invariant를 모두 검증합니다.
	static bool ValidateWorkbook(
		const FCFContentWorkbookModel& Workbook,
		TArray<FCFContentValidationIssue>& OutIssues);

	// Frozen P0 typed ChangeSet identity/concurrency contract를 fail-closed 검증합니다.
	static bool ValidateChangeSet(
		const FCFContentChangeSet& ChangeSet,
		TArray<FCFContentValidationIssue>& OutIssues);

	// Provider가 구축한 dependency graph에서 missing target와 directed cycle을 검증합니다.
	static bool ValidateDependencyEdges(
		const FCFContentWorkbookModel& Workbook,
		const TArray<FCFContentDependencyEdge>& Edges,
		TArray<FCFContentValidationIssue>& OutIssues);

	// Record가 CCAS mutation 대상이 될 수 있는지 management state만으로 fail-closed 판정합니다.
	static bool CanMutateRecord(const FCFContentRecord& Record);
};

/** Presentation-only 정보와 physical order를 제외한 canonical semantic fingerprint를 생성합니다. */
class FCFContentSemanticHasher
{
public:
	// Workbook canonical semantic model을 sha256:<64 lowercase hex> fingerprint로 변환합니다.
	static bool BuildWorkbookSemanticHash(
		const FCFContentWorkbookModel& Workbook,
		FString& OutSemanticHash,
		FString& OutError);
};

/** Explicit old-schema -> new-schema deterministic migration preview를 생성합니다. */
class FCFContentSchemaMigration
{
public:
	// Product mutation 없이 in-memory migrated canonical model과 before/after semantic hash를 생성합니다.
	static bool BuildPreview(
		const FCFContentWorkbookModel& SourceWorkbook,
		const FCFContentMigrationRule& Rule,
		FCFContentMigrationPreview& OutPreview,
		FString& OutError);
};
