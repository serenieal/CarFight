// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-10
// Description: CarFight HUD 전용 비Semantic 시각 자산 DataAsset 계약
// Scope: D1-11 Production UI Rework에서 차량 실루엣, 방향별 Armor Plate, Speed Arc와 Radar 전용 아트 참조만 중앙 관리합니다.
// Changelog:
// - v1.0.0: Vehicle·Armor·SpeedGauge·Radar 전용 Texture/UI Material Soft Reference와 완성도 조회 계약을 최초 추가.
// Migration:
// - Semantic Icon 18종은 기존 UCFUIStyleData.IconSet이 계속 소유하며 이 DataAsset에 중복 등록하지 않습니다.
// - Widget은 콘텐츠 경로를 직접 Load하지 않고 UI Visual Context 또는 Editor Assetization 단계에서 이 DataAsset의 Soft Reference를 전달받습니다.
// - 모든 필드는 D1-11 구조 자산화 시 Null을 허용하며 실제 아트 연결 전에는 Image Placeholder로만 표시할 수 있습니다.

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

	// [v1.0.0] 선택 Target의 Cyan Outer Bracket을 텍스트 기호나 Border 조각 대신 표시할 이미지입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Target", meta=(DisplayName="선택 타겟 Bracket (Selected Target Bracket)", ToolTip="선택 Target 또는 Radar Contact를 감싸는 Cyan Outer Bracket 이미지입니다."))
	TSoftObjectPtr<UTexture2D> SelectedTargetBracket;

	// [v1.0.0] 선택 Weapon Card의 장식 프레임을 실제 이미지로 표시할 때 사용하는 자산입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Weapon", meta=(DisplayName="선택 무기 프레임 (Selected Weapon Frame)", ToolTip="선택된 Weapon Card의 장식 프레임 이미지입니다. 패널 배경 자체는 공통 Style/Panel Surface가 담당합니다."))
	TSoftObjectPtr<UTexture2D> SelectedWeaponFrame;

	// [v1.0.0] VehiclePanel 핵심 전용 아트가 모두 연결됐는지 Asset Load 없이 확인합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Visual", meta=(DisplayName="차량 HUD 아트 준비됨 (Has Complete Vehicle HUD Art)", ToolTip="Vehicle Silhouette, 6방향 Armor Plate와 Speed Arc Material이 모두 연결됐는지 Soft Reference만 확인합니다."))
	bool HasCompleteVehicleArt() const;

	// [v1.0.0] Radar의 최소 정적/동적 전용 아트 중 하나 이상이 연결됐는지 Asset Load 없이 확인합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Visual", meta=(DisplayName="레이더 HUD 아트 있음 (Has Radar HUD Art)", ToolTip="Radar Frame 또는 Radar Sweep Material 중 하나 이상이 연결됐는지 확인합니다."))
	bool HasRadarArt() const;
};
