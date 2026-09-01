// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-09-01
// Description: CF-FQ-041 기본 Runtime Test Catalog 경로 설정
// Scope: Editor-only 검색 없이 Packaged Runtime이 기본 Catalog를 찾을 단일 Config reference를 제공합니다.
// Changelog:
// - v1.0.0: RTA-P0-01 기본 Catalog soft config reference와 동기 load helper를 추가.
// Migration:
// - 실제 Vehicle/Equipment dependency는 Catalog 내부 hard reference가 소유합니다.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CFRuntimeTestSettings.generated.h"

class UCFRuntimeTestCatalogData;

/**
 * 패키징된 Runtime에서도 기본 Runtime Test Catalog를 찾기 위한 Game Config입니다.
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="런타임 테스트 설정 (Runtime Test Settings)", ToolTip="개발·시연 Runtime 메뉴가 사용할 기본 Catalog Asset을 지정합니다."))
class CARFIGHT_RE_API UCFRuntimeTestSettings : public UObject
{
	GENERATED_BODY()

public:
	// [v1.0.0] Config에 지정된 기본 Catalog를 동기 로드하고 유효하지 않으면 nullptr을 반환합니다.
	UCFRuntimeTestCatalogData* LoadDefaultCatalog() const;

	// [v1.0.0] Packaged Runtime에서 읽을 기본 Runtime Test Catalog soft reference입니다.
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|RuntimeTest|Catalog", meta=(DisplayName="기본 런타임 테스트 Catalog (Default Runtime Test Catalog)", ToolTip="Runtime 테스트 메뉴가 기본으로 읽을 Catalog입니다. Catalog 자체는 허용 Vehicle/Equipment를 hard reference로 소유합니다."))
	TSoftObjectPtr<UCFRuntimeTestCatalogData> DefaultCatalog;
};
