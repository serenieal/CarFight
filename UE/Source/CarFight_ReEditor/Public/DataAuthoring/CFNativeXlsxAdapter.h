// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFNativeXlsxAdapter.h
// Version: v1.0.0
// Date: 2026-10-02
// Description: CF-FQ-058 CCAS-P0-07 OpenXLSX 0.5.1 기반 Editor-only persistent .xlsx Workbook adapter입니다.
// Changelog:
// - v1.0.0: repository-vendored OpenXLSX/pugixml/miniz 기반 read/create/staged-write/reopen production contract를 구현.
// - v0.1.0: UE native ZIP/XML availability compile probe.
// Migration:
// - Runtime module dependency는 추가하지 않습니다.
// - build-time network fetch를 사용하지 않습니다.
// - WriteStagedWorkbook은 Base Workbook을 직접 덮어쓰지 않습니다.
// - CreateWorkbookFile은 initial migration baseline 생성 전용이며 existing file overwrite를 거부합니다.

#pragma once

#include "CoreMinimal.h"
#include "DataAuthoring/CFWorkbookAdapter.h"

/** Repository-vendored OpenXLSX stack으로 .xlsx persistent I/O를 수행하는 Editor-only adapter입니다. */
class FCFNativeXlsxAdapter final : public ICFContentWorkbookAdapter
{
public:
	/** Adapter identity와 verified persistence capability를 반환합니다. */
	virtual FCFWorkbookAdapterInfo DescribeAdapter() const override;

	/** Canonical .xlsx Workbook을 typed canonical model로 read-only parse합니다. */
	virtual bool ReadWorkbook(
		const FString& WorkbookPath,
		FCFContentWorkbookModel& OutWorkbook,
		FString& OutError) override;

	/** Base Workbook을 직접 덮어쓰지 않고 sibling staged .xlsx로 작성합니다. */
	virtual bool WriteStagedWorkbook(
		const FString& BaseWorkbookPath,
		const FString& StagedWorkbookPath,
		const FCFContentWorkbookModel& ReviewedWorkbook,
		FString& OutError) override;

	/** Staged .xlsx를 실제 파일에서 다시 열어 canonical model을 반환합니다. */
	virtual bool ReopenWorkbook(
		const FString& StagedWorkbookPath,
		FCFContentWorkbookModel& OutWorkbook,
		FString& OutError) override;

	/** Existing file overwrite 없이 initial migration baseline .xlsx를 생성합니다. */
	bool CreateWorkbookFile(
		const FString& WorkbookPath,
		const FCFContentWorkbookModel& Workbook,
		FString& OutError);
};
