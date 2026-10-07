// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFWorkbookAdapter.h
// Version: v1.0.0
// Date: 2026-09-29
// Description: CF-FQ-058 CCAS-P0-01 Editor-only Workbook adapter boundary입니다.
// Changelog:
// - v1.0.0: Core와 concrete .xlsx library를 분리하는 read/staged-write/reopen contract를 최초 구현.
// Migration:
// - OpenXLSX 0.5.1은 frozen first candidate일 뿐 이 header는 해당 library에 의존하지 않습니다.
// - P0-01에서는 concrete OpenXLSX implementation이나 build-time network fetch를 추가하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFContentTypes.h"

/** Concrete Workbook adapter의 capability/identity metadata입니다. */
struct FCFWorkbookAdapterInfo
{
	// Stable adapter implementation identity입니다.
	FString AdapterId;

	// Exact pinned implementation/library version입니다.
	FString AdapterVersion;

	// Non-semantic Workbook presentation 보존을 검증한 adapter인지 여부입니다.
	bool bPreservesPresentation = false;

	// Workbook/worksheet protection 보존을 검증한 adapter인지 여부입니다.
	bool bPreservesProtection = false;
};

/** .xlsx physical I/O를 Canonical Content Core에서 격리하는 Editor-only boundary입니다. */
class ICFContentWorkbookAdapter
{
public:
	// Interface를 polymorphic하게 안전하게 정리합니다.
	virtual ~ICFContentWorkbookAdapter() = default;

	// Adapter identity와 verified preservation capabilities를 설명합니다.
	virtual FCFWorkbookAdapterInfo DescribeAdapter() const = 0;

	// Canonical Workbook file을 typed canonical content model로 read-only parse합니다.
	virtual bool ReadWorkbook(
		const FString& WorkbookPath,
		FCFContentWorkbookModel& OutWorkbook,
		FString& OutError) = 0;

	// Base Workbook을 직접 덮어쓰지 않고 reviewed model을 staged file로 작성합니다.
	virtual bool WriteStagedWorkbook(
		const FString& BaseWorkbookPath,
		const FString& StagedWorkbookPath,
		const FCFContentWorkbookModel& ReviewedWorkbook,
		FString& OutError) = 0;

	// Staged file을 다시 열어 full parse/validation에 사용할 canonical model을 반환합니다.
	virtual bool ReopenWorkbook(
		const FString& StagedWorkbookPath,
		FCFContentWorkbookModel& OutWorkbook,
		FString& OutError) = 0;
};
