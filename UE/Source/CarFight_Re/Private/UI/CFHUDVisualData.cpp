// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-08-25
// Description: CarFight HUD 전용 비Semantic 시각 자산 DataAsset 구현
// Scope: VehiclePanel modular Armor/차량별 silhouette/Defense와 Radar presentation Soft Reference 조회를 수행합니다.
// Changelog:
// - v1.4.0: Defense common frame/mask/hex/chevron/UI Material 완성도 조회를 additive 추가.
// - v1.3.0: VehicleData identity별 silhouette resolver, common Plate + Arrow/Chevron2 modular Armor 완성도와 legacy fallback 호환을 추가.
// - v1.2.0: Radar Frame/3관계 Blip/Player/4-Corner Selected/2-Corner Edge Texture의 complete presentation 조회를 추가.
// - v1.1.0: VehiclePanelFrame을 핵심 Vehicle Art에 포함하고 RPM Track Texture 또는 UI Material 중 하나를 허용하는 완성도 계약으로 갱신.
// - v1.0.0: 6방향 Armor 완성도, Vehicle HUD 핵심 아트와 Radar 아트 연결 상태 조회를 최초 구현.
// Migration:
// - 이 파일은 Soft Reference를 Load하지 않으며 Gameplay Actor, Component 또는 Runtime ViewData를 조회하지 않습니다.
// - RadarSweepMaterial은 Active Scan sweep 후속 slice라 complete Radar presentation의 필수 항목에 포함하지 않습니다.
// - v1.3.0 silhouette resolver는 Soft Reference path만 비교하며 VehicleData 또는 Texture를 Load하지 않습니다. catalog miss는 기존 VehicleSilhouette fallback으로 내려갑니다.
// - v1.4.0 Defense 완성도 조회도 Soft Reference null 여부만 확인하며 Asset을 Load하지 않습니다.

#include "UI/CFHUDVisualData.h"

// [v1.3.0] VehicleData identity와 HUD silhouette Texture가 모두 지정된 유효 catalog 항목인지 확인합니다.
bool FCFHUDVehicleSilhouetteEntry::IsValidEntry() const
{
	return !VehicleData.IsNull() && !SilhouetteTexture.IsNull();
}

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

// [v1.3.0] VehicleData identity에 맞는 catalog Texture를 찾고 없으면 기본 VehicleSilhouette fallback을 반환합니다.
TSoftObjectPtr<UTexture2D> UCFHUDVisualData::ResolveVehicleSilhouette(const TSoftObjectPtr<UCFVehicleData>& VehicleDataAsset) const
{
	// [v1.3.0] Load 없이 catalog entry와 비교할 현재 VehicleData의 안정 Soft Object Path입니다.
	const FSoftObjectPath RequestedVehicleDataPath = VehicleDataAsset.ToSoftObjectPath();
	if (RequestedVehicleDataPath.IsValid())
	{
		for (const FCFHUDVehicleSilhouetteEntry& SilhouetteEntry : VehicleSilhouettes)
		{
			if (SilhouetteEntry.IsValidEntry()
				&& SilhouetteEntry.VehicleData.ToSoftObjectPath() == RequestedVehicleDataPath)
			{
				return SilhouetteEntry.SilhouetteTexture;
			}
		}
	}

	return VehicleSilhouette;
}

// [v1.3.0] 공통 Plate와 두 canonical 방향 Icon이 모두 연결됐는지 Asset Load 없이 확인합니다.
bool UCFHUDVisualData::HasModularArmorArt() const
{
	return !ArmorCommonPlate.IsNull()
		&& !ArmorDirectionArrow.IsNull()
		&& !ArmorDirectionChevron2.IsNull();
}

// [v1.4.0] Defense common frame/mask/hex/chevron와 UI Fill Material이 모두 연결됐는지 Asset Load 없이 확인합니다.
bool UCFHUDVisualData::HasDefenseArt() const
{
	return !DefenseBarFrame.IsNull()
		&& !DefenseFillMask.IsNull()
		&& !DefenseHexPattern.IsNull()
		&& !DefenseChevron.IsNull()
		&& !DefenseFillMaterial.IsNull();
}

// [v1.3.0] VehiclePanel 핵심 전용 아트가 legacy 또는 modular Armor 경로 중 하나로 준비됐는지 Asset Load 없이 확인합니다.
bool UCFHUDVisualData::HasCompleteVehicleArt() const
{
	// [v1.3.0] 기본 fallback 또는 하나 이상의 유효 catalog entry가 있는지 나타냅니다.
	bool bHasVehicleSilhouetteSource = !VehicleSilhouette.IsNull();
	if (!bHasVehicleSilhouetteSource)
	{
		for (const FCFHUDVehicleSilhouetteEntry& SilhouetteEntry : VehicleSilhouettes)
		{
			if (SilhouetteEntry.IsValidEntry())
			{
				bHasVehicleSilhouetteSource = true;
				break;
			}
		}
	}

	// [v1.3.0] 기존 6방향 완성 Plate 또는 새 common Plate + 2 Icon 중 하나가 준비됐는지 나타냅니다.
	const bool bHasArmorVisual = ArmorPlates.IsComplete() || HasModularArmorArt();
	// [v1.1.0] 정적 RPM Track Texture 또는 동적 UI Material 중 하나가 연결됐는지 나타냅니다.
	const bool bHasRpmTrackVisual = !SpeedArcTrack.IsNull() || !SpeedArcMaterial.IsNull();
	return !VehiclePanelFrame.IsNull()
		&& bHasVehicleSilhouetteSource
		&& bHasArmorVisual
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
