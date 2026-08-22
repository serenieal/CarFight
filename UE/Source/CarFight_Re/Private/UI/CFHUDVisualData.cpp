// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-20
// Description: CarFight HUD 전용 비Semantic 시각 자산 DataAsset 구현
// Scope: VehiclePanel과 UI-P0-08 Radar presentation 전용 Soft Reference 완성도 조회만 수행합니다.
// Changelog:
// - v1.2.0: Radar Frame/3관계 Blip/Player/4-Corner Selected/2-Corner Edge Texture의 complete presentation 조회를 추가.
// - v1.1.0: VehiclePanelFrame을 핵심 Vehicle Art에 포함하고 RPM Track Texture 또는 UI Material 중 하나를 허용하는 완성도 계약으로 갱신.
// - v1.0.0: 6방향 Armor 완성도, Vehicle HUD 핵심 아트와 Radar 아트 연결 상태 조회를 최초 구현.
// Migration:
// - 이 파일은 Soft Reference를 Load하지 않으며 Gameplay Actor, Component 또는 Runtime ViewData를 조회하지 않습니다.
// - RadarSweepMaterial은 Active Scan sweep 후속 slice라 complete Radar presentation의 필수 항목에 포함하지 않습니다.

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

// [v1.1.0] VehiclePanel 핵심 전용 아트가 모두 연결됐는지 Asset Load 없이 확인합니다.
bool UCFHUDVisualData::HasCompleteVehicleArt() const
{
	// [v1.1.0] 정적 RPM Track Texture 또는 동적 UI Material 중 하나가 연결됐는지 나타냅니다.
	const bool bHasRpmTrackVisual = !SpeedArcTrack.IsNull() || !SpeedArcMaterial.IsNull();
	return !VehiclePanelFrame.IsNull()
		&& !VehicleSilhouette.IsNull()
		&& ArmorPlates.IsComplete()
		&& bHasRpmTrackVisual;
}

// [v1.0.0] Radar의 최소 정적/동적 전용 아트 중 하나 이상이 연결됐는지 Asset Load 없이 확인합니다.
bool UCFHUDVisualData::HasRadarArt() const
{
	return !RadarFrame.IsNull() || !RadarSweepMaterial.IsNull();
}

// [v1.2.0] UI-P0-08B Production Radar presentation에 필요한 교체 가능한 Texture가 모두 연결됐는지 확인합니다.
bool UCFHUDVisualData::HasCompleteRadarPresentationArt() const
{
	return !RadarFrame.IsNull()
		&& !RadarFriendlyBlip.IsNull()
		&& !RadarHostileBlip.IsNull()
		&& !RadarUnknownBlip.IsNull()
		&& !RadarPlayerMarker.IsNull()
		&& !SelectedTargetBracket.IsNull()
		&& !RadarSelectedEdgeBracket.IsNull();
}
