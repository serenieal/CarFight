// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-20
// Description: CarFight HUD 전용 비Semantic 시각 자산 DataAsset 계약
// Scope: D1-11 Production UI Rework와 UI-P0-08에서 VehiclePanel 전용 아트와 Radar Frame/Blip/Player/Selection 시각 자산 참조를 중앙 관리합니다.
// Changelog:
// - v1.2.0: UI-P0-08B용 Radar Friendly/Hostile/Unknown Blip, Player Marker, Selected Edge Bracket Soft Reference와 complete Radar presentation 조회를 additive 추가. Neutral은 Unknown diamond Texture를 관계색만 바꿔 재사용.
// - v1.1.1: P2 Hardening에서 HasCompleteVehicleArt 선언부의 소스 들여쓰기만 교정. 계약/직렬화/Runtime 의미 변경 없음.
// - v1.1.0: Balanced Combat VehiclePanel Vertical Slice용 VehiclePanelFrame Texture2D Soft Reference를 additive 추가하고 Vehicle Art 완성도 계약에 Frame + Track/Material 대체 조건을 반영.
// - v1.0.0: Vehicle·Armor·SpeedGauge·Radar 전용 Texture/UI Material Soft Reference와 완성도 조회 계약을 최초 추가.
// Migration:
// - Semantic Icon 18종은 기존 UCFUIStyleData.IconSet이 계속 소유하며 이 DataAsset에 중복 등록하지 않습니다.
// - Widget은 콘텐츠 경로를 직접 Load하지 않고 UI Visual Context 또는 Editor Assetization 단계에서 이 DataAsset의 Soft Reference를 전달받습니다.
// - 모든 필드는 구조 자산화 시 Null을 허용하지만 Production targeted visual apply는 해당 slice가 요구하는 필드를 모두 연결한 뒤에만 저장합니다.
// - UI-P0-08B Radar Blip은 TextBlock 문자기호가 아니라 전용 Texture Image로 표시하며 Neutral은 Unknown diamond Texture에 Neutral 관계색을 적용합니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CFHUDVisualData.generated.h"

class UMaterialInterface;
class UTexture2D;

/**
 * Vehicle Armor Body Map에서 방향별로 교체할 실제 Plate 이미지 묶음입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFHUDArmorVisualSet
{
	GENERATED_BODY()

	// [v1.0.0] 화면 좌측 Front Armor 영역에 사용할 이미지입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Armor", meta=(DisplayName="전면 장갑 이미지 (Front Armor Image)", ToolTip="왼쪽을 바라보는 Vehicle Body Map의 좌측 Front Armor Plate 이미지입니다."))
	TSoftObjectPtr<UTexture2D> FrontPlate;

	// [v1.0.0] 화면 상단 Right Armor 영역에 사용할 이미지입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Armor", meta=(DisplayName="우측 장갑 이미지 (Right Armor Image)", ToolTip="왼쪽을 바라보는 Vehicle Body Map의 상단 Right Armor Plate 이미지입니다."))
	TSoftObjectPtr<UTexture2D> RightPlate;

	// [v1.0.0] 화면 우측 Rear Armor 영역에 사용할 이미지입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Armor", meta=(DisplayName="후면 장갑 이미지 (Rear Armor Image)", ToolTip="왼쪽을 바라보는 Vehicle Body Map의 우측 Rear Armor Plate 이미지입니다."))
	TSoftObjectPtr<UTexture2D> RearPlate;

	// [v1.0.0] 화면 하단 Left Armor 영역에 사용할 이미지입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Armor", meta=(DisplayName="좌측 장갑 이미지 (Left Armor Image)", ToolTip="왼쪽을 바라보는 Vehicle Body Map의 하단 Left Armor Plate 이미지입니다."))
	TSoftObjectPtr<UTexture2D> LeftPlate;

	// [v1.0.0] Armor Cluster 좌상단 Top Badge에 사용할 이미지입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Armor", meta=(DisplayName="상부 장갑 이미지 (Top Armor Image)", ToolTip="Vehicle Body Map 좌상단 Top Armor Badge에 사용할 이미지입니다."))
	TSoftObjectPtr<UTexture2D> TopPlate;

	// [v1.0.0] Armor Cluster 우하단 Bottom Badge에 사용할 이미지입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Armor", meta=(DisplayName="하부 장갑 이미지 (Bottom Armor Image)", ToolTip="Vehicle Body Map 우하단 Bottom Armor Badge에 사용할 이미지입니다."))
	TSoftObjectPtr<UTexture2D> BottomPlate;

	// [v1.0.0] 여섯 방향 Armor Plate 이미지가 모두 연결됐는지 Asset Load 없이 확인합니다.
	bool IsComplete() const;
};

/**
 * Semantic Icon과 분리된 CarFight HUD 전용 시각 아트 참조를 소유합니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFHUDVisualData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.1.0] VehiclePanel 전체 Surface에 9-Slice로 사용할 Balanced Combat 프레임 이미지입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Vehicle", meta=(DisplayName="차량 패널 프레임 (Vehicle Panel Frame)", ToolTip="WBP_CFVehiclePanel의 단일 Surface Border에 9-Slice로 적용할 프레임 Texture입니다. 중앙에는 Gameplay 값이나 텍스트를 굽지 않습니다."))
	TSoftObjectPtr<UTexture2D> VehiclePanelFrame;

	// [v1.0.0] Armor Body Map 중앙에 표시할 왼쪽 전방 차량 실루엣 이미지입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Vehicle", meta=(DisplayName="차량 실루엣 (Vehicle Silhouette)", ToolTip="WBP_CFArmorBodyMap 중앙에 표시할 투명 배경 왼쪽 전방 차량 실루엣 이미지입니다."))
	TSoftObjectPtr<UTexture2D> VehicleSilhouette;

	// [v1.0.0] Vehicle Body Map의 여섯 방향 Armor Plate 이미지 묶음입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Vehicle", meta=(DisplayName="방향별 장갑 이미지 (Directional Armor Images)", ToolTip="Front, Right, Rear, Left, Top, Bottom Armor Plate의 교체 가능한 이미지 묶음입니다."))
	FCFHUDArmorVisualSet ArmorPlates;

	// [v1.0.0] Speed Gauge의 정적 Track 장식을 사용할 경우 연결할 이미지입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Speed", meta=(DisplayName="속도 원호 Track 이미지 (Speed Arc Track Image)", ToolTip="속도 원호의 정적 Track 장식 이미지입니다. UI Material이 Track까지 그리면 비워둘 수 있습니다."))
	TSoftObjectPtr<UTexture2D> SpeedArcTrack;

	// [v1.0.0] Speed Ratio를 파라미터로 표현할 UI Domain Material입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Speed", meta=(DisplayName="속도 원호 UI Material (Speed Arc UI Material)", ToolTip="WBP_CFSpeedGauge의 Image Brush에서 사용할 UI Domain Material입니다. FillRatio 같은 스칼라 파라미터는 후속 Runtime Presenter 단계에서 적용합니다."))
	TSoftObjectPtr<UMaterialInterface> SpeedArcMaterial;

	// [v1.0.0] Radar의 정적 Grid·Ring·Frame을 한 장으로 표현할 이미지입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Radar", meta=(DisplayName="레이더 프레임 이미지 (Radar Frame Image)", ToolTip="Border 선 조각 대신 Radar의 Grid, Ring과 Frame을 한 번에 표현할 투명 배경 이미지입니다."))
	TSoftObjectPtr<UTexture2D> RadarFrame;

		// [v1.0.0] Radar Sweep이 필요할 때 Image Brush로 사용할 UI Domain Material입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Radar", meta=(DisplayName="레이더 Sweep UI Material (Radar Sweep UI Material)", ToolTip="레이더 회전 Sweep과 Fade를 표현할 선택적 UI Domain Material입니다."))
	TSoftObjectPtr<UMaterialInterface> RadarSweepMaterial;

	// [v1.2.0] Friendly Contact를 원형 Solid Blip으로 표시할 투명 Texture입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Radar", meta=(DisplayName="아군 레이더 Blip (Friendly Radar Blip)", ToolTip="Friendly Contact를 원형 Solid Image로 표시합니다. 실제 관계색은 UI Style Token이 적용됩니다."))
	TSoftObjectPtr<UTexture2D> RadarFriendlyBlip;

	// [v1.2.0] Hostile Contact를 하향 삼각형 Solid Blip으로 표시할 투명 Texture입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Radar", meta=(DisplayName="적대 레이더 Blip (Hostile Radar Blip)", ToolTip="Hostile Contact를 하향 삼각형 Solid Image로 표시합니다. 실제 관계색은 UI Style Token이 적용됩니다."))
	TSoftObjectPtr<UTexture2D> RadarHostileBlip;

	// [v1.2.0] Unknown과 Neutral Contact가 공유할 마름모 Solid Blip Texture입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Radar", meta=(DisplayName="미식별 레이더 Blip (Unknown Radar Blip)", ToolTip="Unknown과 Neutral Contact가 공유하는 마름모 Solid Image입니다. Unknown/Neutral 관계색은 각각 Style Token으로 구분합니다."))
	TSoftObjectPtr<UTexture2D> RadarUnknownBlip;

	// [v1.2.0] Heading-Up Radar 중앙의 Player 차량을 나타낼 상향 삼각형 Texture입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Radar", meta=(DisplayName="플레이어 레이더 마커 (Player Radar Marker)", ToolTip="Heading-Up Radar 중앙에 고정되는 Player 상향 삼각형 Image입니다."))
	TSoftObjectPtr<UTexture2D> RadarPlayerMarker;

	// [v1.2.0] 표시 Range 밖 선택 Target 방향을 Radar Edge에서 나타낼 2-Corner Open Bracket Texture입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Radar", meta=(DisplayName="레이더 선택 Edge Bracket (Radar Selected Edge Bracket)", ToolTip="표시 Range 밖 선택 Contact의 방향을 Radar 가장자리에서 나타내는 2-Corner Open Bracket Image입니다."))
	TSoftObjectPtr<UTexture2D> RadarSelectedEdgeBracket;

	// [v1.0.0] 선택 Target의 Cyan Outer Bracket을 텍스트 기호나 Border 조각 대신 표시할 이미지입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Target", meta=(DisplayName="선택 타겟 Bracket (Selected Target Bracket)", ToolTip="선택 Target 또는 Radar Contact를 감싸는 Cyan Outer Bracket 이미지입니다."))
	TSoftObjectPtr<UTexture2D> SelectedTargetBracket;

	// [v1.0.0] 선택 Weapon Card의 장식 프레임을 실제 이미지로 표시할 때 사용하는 자산입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Weapon", meta=(DisplayName="선택 무기 프레임 (Selected Weapon Frame)", ToolTip="선택된 Weapon Card의 장식 프레임 이미지입니다. 패널 배경 자체는 공통 Style/Panel Surface가 담당합니다."))
	TSoftObjectPtr<UTexture2D> SelectedWeaponFrame;

	// [v1.1.0] VehiclePanel 핵심 전용 아트가 모두 연결됐는지 Asset Load 없이 확인합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Visual", meta=(DisplayName="차량 HUD 아트 준비됨 (Has Complete Vehicle HUD Art)", ToolTip="Vehicle Panel Frame, Vehicle Silhouette, 6방향 Armor Plate와 RPM Track Texture 또는 UI Material 중 하나가 모두 연결됐는지 Soft Reference만 확인합니다."))
	bool HasCompleteVehicleArt() const;

		// [v1.0.0] Radar의 최소 정적/동적 전용 아트 중 하나 이상이 연결됐는지 Asset Load 없이 확인합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Visual", meta=(DisplayName="레이더 HUD 아트 있음 (Has Radar HUD Art)", ToolTip="Radar Frame 또는 Radar Sweep Material 중 하나 이상이 연결됐는지 확인합니다."))
	bool HasRadarArt() const;

	// [v1.2.0] UI-P0-08B Production Radar presentation에 필요한 Frame/Blip/Player/Selection Texture가 모두 연결됐는지 Asset Load 없이 확인합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Visual", meta=(DisplayName="레이더 표현 아트 준비됨 (Has Complete Radar Presentation Art)", ToolTip="Radar Frame, Friendly/Hostile/Unknown Blip, Player Marker, 4-Corner Selected Target Bracket과 2-Corner Selected Edge Bracket이 모두 연결됐는지 확인합니다. Sweep Material은 선택적이라 포함하지 않습니다."))
	bool HasCompleteRadarPresentationArt() const;
};
