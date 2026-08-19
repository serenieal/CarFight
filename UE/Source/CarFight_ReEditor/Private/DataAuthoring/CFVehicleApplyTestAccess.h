// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleApplyTestAccess.h
// Version: v1.0.0
// Date: 2026-08-17
// Description: DAUTH-P0-08H Automation이 post-mutation rollback branch를 실제로 실행하기 위한 Private test access입니다.
// Changelog:
// - v1.0.0: Production public API를 오염시키지 않는 transaction failure injection bridge 최초 구현.
// Migration:
// - 이 파일은 CarFight_ReEditor/Private에만 존재하며 제품 Apply caller는 사용하지 않습니다.

#pragma once

#include "DataAuthoring/CFVehicleApplyService.h"

/** Automation에서 실제 Target mutation 이후 rollback 경로를 검증하는 Private bridge입니다. */
class FCFVehicleApplyTestAccess
{
public:
	// Target diff 적용 직후 의도적 실패를 주입해 service 자체 rollback을 검증합니다.
	static bool ApplyWithPostMutationFailure(
		const FCFVehicleApplyRequest& Request,
		FCFVehicleApplyResult& OutResult)
	{
		return FCFVehicleApplyService::ApplyInternal(Request, OutResult, true);
	}
};
