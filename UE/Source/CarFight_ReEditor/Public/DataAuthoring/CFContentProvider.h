// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFContentProvider.h
// Version: v1.1.0
// Date: 2026-09-30
// Description: CF-FQ-058 CCAS provider schema/validation/diff/reviewed apply seam과 registry 계약입니다.
// Changelog:
// - v1.1.0: P0-06 immutable Catalog Snapshot용 read-only ProviderSchemaFingerprint API를 추가.
// - v1.0.0: Core가 Weapon/Vehicle 의미를 소유하지 않도록 exact provider responsibilities와 duplicate-safe registry를 최초 구현.
// Migration:
// - ApplyReviewed는 seam만 정의합니다. P0-01은 Product provider mutation 구현을 등록하지 않습니다.
// - 향후 Product mutation은 existing typed Provider/CFDADurableCore를 adapter하는 방식으로만 연결합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFContentTypes.h"

/** Provider가 Core에 등록하는 stable schema descriptor입니다. */
struct FCFContentProviderDescriptor
{
	// Provider implementation의 stable identity입니다.
	FString ProviderId;

	// Provider가 소유하는 logical ContentType입니다.
	FCFContentTypeId ContentTypeId;

	// Provider-local positive schema revision입니다.
	int32 SchemaRevision = 0;

	// Provider가 소유하는 primary/child logical sheet schema입니다.
	TArray<FCFContentSheetDescriptor> Sheets;
};

/** Existing Product read/import 결과입니다. */
struct FCFContentImportResult
{
	// Product를 mutation하지 않고 읽어 만든 canonical records입니다.
	TArray<FCFContentRecord> Records;

	// Import 중 발견한 typed validation diagnostics입니다.
	TArray<FCFContentValidationIssue> Issues;
};

/** Existing Product/current truth의 typed canonical snapshot입니다. */
struct FCFContentSnapshot
{
	// Snapshot 대상 logical key입니다.
	FCFContentKey Key;

	// Provider가 읽은 canonical current record입니다.
	FCFContentRecord Record;

	// Provider readback semantic fingerprint입니다.
	FString ReadbackFingerprint;
};

/** Current -> Desired 차이를 provider가 typed 의미로 표현한 diff입니다. */
struct FCFContentDiff
{
	// Diff 대상 logical key입니다.
	FCFContentKey Key;

	// Current provider readback fingerprint입니다.
	FString CurrentFingerprint;

	// Desired canonical record fingerprint입니다.
	FString DesiredFingerprint;

	// 변경된 stable field/collection path 목록입니다.
	TArray<FString> ChangedPaths;
};

/** Review 뒤에만 Apply seam으로 전달할 immutable-style mutation plan입니다. */
struct FCFContentReviewedMutationPlan
{
	// Mutation 대상 logical key입니다.
	FCFContentKey Key;

	// Review가 결속된 Base WorkbookSemanticHash입니다.
	FString BaseWorkbookSemanticHash;

	// Review가 결속된 desired semantic fingerprint입니다.
	FString DesiredFingerprint;

	// Provider가 deterministic하게 구성한 changed path 목록입니다.
	TArray<FString> ChangedPaths;

	// 사용자/상위 review layer가 승인한 plan인지 나타냅니다.
	bool bApproved = false;
};

/** Provider ApplyReviewed seam의 결과입니다. */
struct FCFContentApplyResult
{
	// 실제 persistent mutation이 수행됐는지 여부입니다.
	bool bApplied = false;

	// Apply 뒤 typed readback fingerprint입니다.
	FString ReadbackFingerprint;

	// 실패 또는 blocked 사유입니다.
	FString Error;
};

/** Content type 고유 의미와 Product typed mutation 경계를 소유하는 provider interface입니다. */
class ICFContentProvider
{
public:
	// Interface를 polymorphic하게 안전하게 정리합니다.
	virtual ~ICFContentProvider() = default;

	// Provider-owned stable sheet/field schema를 설명합니다.
	virtual bool DescribeSchema(
		FCFContentProviderDescriptor& OutDescriptor,
		FString& OutError) const = 0;

	// Existing Product truth를 read-only canonical records로 import합니다.
	virtual bool ImportExisting(
		FCFContentImportResult& OutImport,
		FString& OutError) const = 0;

	// Exact logical key의 current Product truth를 read-only snapshot으로 구성합니다.
	virtual bool BuildCurrentSnapshot(
		const FCFContentKey& Key,
		FCFContentSnapshot& OutSnapshot,
		FString& OutError) const = 0;

	// Provider-specific semantic invariant를 typed record에 적용합니다.
	virtual bool ValidateRecord(
		const FCFContentRecord& Record,
		TArray<FCFContentValidationIssue>& OutIssues) const = 0;

	// Record가 만드는 provider-specific dependency edges를 구성합니다.
	virtual bool BuildDependencyEdges(
		const FCFContentRecord& Record,
		TArray<FCFContentDependencyEdge>& OutEdges,
		FString& OutError) const = 0;

	// Current snapshot과 desired canonical record 사이 typed diff를 구성합니다.
	virtual bool BuildDiff(
		const FCFContentSnapshot& Current,
		const FCFContentRecord& Desired,
		FCFContentDiff& OutDiff,
		FString& OutError) const = 0;

	// Review 가능한 immutable-style mutation plan을 구성합니다.
	virtual bool BuildReviewedMutationPlan(
		const FCFContentDiff& Diff,
		const FString& BaseWorkbookSemanticHash,
		FCFContentReviewedMutationPlan& OutPlan,
		FString& OutError) const = 0;

	// 승인된 plan을 typed persistent writer로 적용하는 seam입니다. P0-01 Core는 이 호출을 직접 구현하지 않습니다.
	virtual bool ApplyReviewed(
		const FCFContentReviewedMutationPlan& ReviewedPlan,
		FCFContentApplyResult& OutResult) = 0;

	// Apply 또는 drift inspection 뒤 exact Product semantic fingerprint를 read-only로 읽습니다.
	virtual bool ReadbackFingerprint(
		const FCFContentKey& Key,
		FString& OutFingerprint,
		FString& OutError) const = 0;
};

/** ContentType별 provider registration을 Core에서 분리해 관리하는 registry입니다. */
class FCFContentProviderRegistry
{
public:
	// Provider descriptor를 검증한 뒤 exact ContentType에 중복 없이 등록합니다.
	bool RegisterProvider(
		const TSharedRef<ICFContentProvider>& Provider,
		FString& OutError);

	// Exact ContentType에 등록된 provider를 반환하며 없으면 nullptr입니다.
	ICFContentProvider* FindProvider(const FCFContentTypeId& ContentTypeId) const;

	// 전체 registry descriptor가 duplicate/invalid 상태 없이 일관적인지 검증합니다.
	bool ValidateRegistry(FString& OutError) const;

	// 등록된 Provider schema/ownership 전체를 canonical SHA-256 fingerprint로 생성합니다.
	bool BuildSchemaFingerprint(FString& OutFingerprint, FString& OutError) const;

	// 등록된 provider 수를 반환합니다.
	int32 Num() const
	{
		return ProvidersByContentType.Num();
	}

private:
	// Stable ContentTypeId -> provider instance registry입니다.
	TMap<FString, TSharedRef<ICFContentProvider>> ProvidersByContentType;
};
