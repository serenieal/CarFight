// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBatchApplyTestAccess.h
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08M B3 partial-failure Automation을 위한 Private-only access입니다.
// Changelog:
// - v1.0.0: Global Preflight 뒤 per-target Apply 직전 controlled state change를 주입하는 friend access 추가.
// Migration:
// - Public/DataAuthoring에 노출되지 않으며 production B3 request/result 계약에 test flag를 추가하지 않습니다.

#pragma once

#include "DataAuthoring/CFBatchApply.h"

/** Production public API를 오염시키지 않고 B3 sequential partial failure를 검증하는 Private Automation access입니다. */
class FCFBatchApplyTestAccess
{
public:
	// Global Preflight 완료 뒤 각 eligible target Apply 직전에 Automation hook을 실행합니다.
	static bool ApplyWithPreApplyHook(
		const FCFBatchDefinitionApplyRequest& Request,
		FCFBatchDefinitionApplyResult& OutResult,
		const TFunction<void(int32, UCFVehicleData*)>& PreApplyHook)
	{
		return FCFBatchApplyService::ApplyBatchInternal(Request, OutResult, PreApplyHook);
	}
};
