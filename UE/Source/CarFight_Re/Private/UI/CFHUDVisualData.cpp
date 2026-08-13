// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-10
// Description: CarFight HUD 전용 비Semantic 시각 자산 DataAsset 구현
// Scope: D1-11 Production UI Rework의 HUD 전용 Soft Reference 완성도 조회만 수행합니다.
// Changelog:
// - v1.0.0: 6방향 Armor 완성도, Vehicle HUD 핵심 아트와 Radar 아트 연결 상태 조회를 최초 구현.
// Migration:
// - 이 파일은 Soft Reference를 Load하지 않으며 Gameplay Actor, Component 또는 Runtime ViewData를 조회하지 않습니다.

#include "UI/CFHUDVisualData.h"

// [v1.0.0] 여섯 방향 Armor Plate 이미지가 모두 연결됐는지 Asset Load 없이 확인합니다.
bool FCFHUDArmorVisualSet::IsComplete() const
{
	return !FrontPlate.IsNull()
		&& !RightPlate.IsNull()
		&& !RearPlate.IsNull()
		&& !LeftPlate.IsNull()
		&& !TopPlate.IsNull()
		&& !BottomPlate.IsNull();
}

// [v1.0.0] VehiclePanel 핵심 전용 아트가 모두 연결됐는지 Asset Load 없이 확인합니다.
bool UCFHUDVisualData::HasCompleteVehicleArt() const
{
	return !VehicleSilhouette.IsNull()
		&& ArmorPlates.IsComplete()
		&& !SpeedArcMaterial.IsNull();
}

// [v1.0.0] Radar의 최소 정적/동적 전용 아트 중 하나 이상이 연결됐는지 Asset Load 없이 확인합니다.
bool UCFHUDVisualData::HasRadarArt() const
{
	return !RadarFrame.IsNull() || !RadarSweepMaterial.IsNull();
}
