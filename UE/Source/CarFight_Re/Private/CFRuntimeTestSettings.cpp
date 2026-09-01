// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-01
// Description: CF-FQ-041 기본 Runtime Test Catalog Config 구현
// Scope: Config soft reference를 Editor API 없이 동기 로드하고 Catalog runtime 계약을 검증합니다.
// Changelog:
// - v1.0.0: RTA-P0-01 default catalog load helper 구현.
// Migration:
// - Asset Registry enumeration이나 Content Browser API를 사용하지 않습니다.

#include "CFRuntimeTestSettings.h"

#include "CFRuntimeTestCatalogData.h"

// [v1.0.0] Config에 지정된 기본 Catalog를 동기 로드하고 유효하지 않으면 nullptr을 반환합니다.
UCFRuntimeTestCatalogData* UCFRuntimeTestSettings::LoadDefaultCatalog() const
{
	if (DefaultCatalog.IsNull())
	{
		return nullptr;
	}

	// [v1.0.0] Config soft reference에서 동기 로드한 기본 Catalog입니다.
	UCFRuntimeTestCatalogData* LoadedCatalog = DefaultCatalog.LoadSynchronous();
	return IsValid(LoadedCatalog) && LoadedCatalog->IsRuntimeTestCatalogUsable()
		? LoadedCatalog
		: nullptr;
}
