// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-09-08
// Description: CF-FQ-030 미사일 유도 Guidance Preset DataAsset passive container 구현 단위
// Scope: UCFMissileGuidePresetData는 저장된 값만 소유하며 UObject 수명주기나 자산 이름에 따라 Low/Normal/High 기본값을 주입하지 않습니다.
// Changelog:
// - v1.2.0: PostInitProperties/InitializeAuthoringDefaultsFromAssetName를 제거해 Product 모듈의 Low/Normal/High 이름 분기와 load-time seed를 0으로 교정. 신규 Preset seed는 CarFight_ReEditor authoring 경로가 명시적으로 수행합니다.
// - v1.1.0: Low/Normal/High 신규 DA 생성 기본값 최초 구현. v1.2.0에서 사용자 저장 튜닝값 보존 계약을 강화하기 위해 폐기.
// Migration:
// - 기존 저장 Preset은 재저장이나 마이그레이션 없이 현재 직렬화 값을 그대로 사용합니다.
// - 신규 Low/Normal/High Preset 생성 기본값이 필요하면 CarFight.Authoring.MissileFeel.EnsurePresets Editor authoring 경로를 사용합니다.

#include "CFMissileGuidePresetData.h"
