// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-08-25
// Description: CarFight HUD 전용 비Semantic 시각 자산 DataAsset 계약
// Scope: VehiclePanel modular Armor/차량별 silhouette/Defense와 Radar Frame/Blip/Player/Selection 시각 자산 참조를 중앙 관리합니다.
// Changelog:
// - v1.4.0: CF-FQ-039 Defense common frame/fill mask/hex/chevron Texture와 UI Material Soft Reference, HasDefenseArt 조회를 additive 추가.
// - v1.3.0: CF-FQ-039 Armor modular 구조용 공통 Plate, Arrow/Chevron2 2종 방향 Icon, VehicleData identity별 silhouette catalog와 fallback resolver를 additive 추가. 기존 단일 VehicleSilhouette/ArmorPlates 6종은 compatibility fallback으로 보존.
// - v1.2.0: UI-P0-08B용 Radar Friendly/Hostile/Unknown Blip, Player Marker, Selected Edge Bracket Soft Reference와 complete Radar presentation 조회를 additive 추가. Neutral은 Unknown diamond Texture를 관계색만 바꿔 재사용.
// - v1.1.1: P2 Hardening에서 HasCompleteVehicleArt 선언부의 소스 들여쓰기만 교정. 계약/직렬화/Runtime 의미 변경 없음.
// - v1.1.0: Balanced Combat VehiclePanel Vertical Slice용 VehiclePanelFrame Texture2D Soft Reference를 additive 추가하고 Vehicle Art 완성도 계약에 Frame + Track/Material 대체 조건을 반영.
// - v1.0.0: Vehicle·Armor·SpeedGauge·Radar 전용 Texture/UI Material Soft Reference와 완성도 조회 계약을 최초 추가.
// Migration:
// - Semantic Icon 18종은 기존 UCFUIStyleData.IconSet이 계속 소유하며 이 DataAsset에 중복 등록하지 않습니다.
// - Widget은 콘텐츠 경로를 직접 Load하지 않고 UI Visual Context 또는 Editor Assetization 단계에서 이 DataAsset의 Soft Reference를 전달받습니다.
// - 모든 필드는 구조 자산화 시 Null을 허용하지만 Production targeted visual apply는 해당 slice가 요구하는 필드를 모두 연결한 뒤에만 저장합니다.
// - UI-P0-08B Radar Blip은 TextBlock 문자기호가 아니라 전용 Texture Image로 표시하며 Neutral은 Unknown diamond Texture에 Neutral 관계색을 적용합니다.
// - v1.3.0 Armor modular migration은 기존 직렬화 필드를 삭제/rename하지 않습니다. 새 common Plate/2-icon/catalog가 준비되지 않은 Content는 기존 VehicleSilhouette/ArmorPlates 경로로 계속 동작합니다.
// - v1.4.0 Defense Visual 필드는 완전 additive이며 기존 ProgressBar_Shield/Integrity runtime sink, ViewData, Presenter binding을 변경하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CFHUDVisualData.generated.h"

class UCFVehicleData;
class UMaterialInterface;
class UTexture2D;

/**
 * 특정 VehicleData identity에 대응하는 Armor Body Map 차량 실루엣 Texture를 연결합니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFHUDVehicleSilhouetteEntry
{
	GENERATED_BODY()

	// [v1.3.0] 이 HUD 실루엣을 선택할 정확한 VehicleData Asset identity입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Vehicle", meta=(DisplayName="차량 데이터 (Vehicle Data)", ToolTip="현재 플레이어 차량의 VehicleData Asset과 동일할 때 이 항목의 실루엣을 사용합니다. UI가 VehicleData 설정을 수정하지 않습니다."))
	TSoftObjectPtr<UCFVehicleData> VehicleData;

	// [v1.3.0] 해당 차량을 왼쪽 전방으로 정규화한 탑다운 HUD 실루엣 Texture입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Vehicle", meta=(DisplayName="차량별 HUD 실루엣 (Vehicle HUD Silhouette)", ToolTip="Armor Body Map에서 해당 VehicleData 차량을 나타낼 투명 탑다운 좌향 실루엣 Texture입니다."))
	TSoftObjectPtr<UTexture2D> SilhouetteTexture;

	// [v1.3.0] Catalog 항목이 VehicleData와 Texture를 모두 가지고 있는지 Asset Load 없이 확인합니다.
	bool IsValidEntry() const;
};

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

	// [v1.0.0] 차량별 catalog에 항목이 없을 때 Armor Body Map 중앙에 표시할 compatibility fallback 실루엣 이미지입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Vehicle", meta=(DisplayName="기본 차량 실루엣 (Default Vehicle Silhouette)", ToolTip="VehicleData별 실루엣 Catalog에 현재 차량 항목이 없을 때 사용할 투명 탑다운 좌향 fallback Texture입니다."))
	TSoftObjectPtr<UTexture2D> VehicleSilhouette;

	// [v1.3.0] 현재 VehicleData identity별로 Armor Body Map 차량 이미지를 선택할 HUD 전용 catalog입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Vehicle", meta=(DisplayName="차량별 실루엣 목록 (Vehicle Silhouette Catalog)", ToolTip="VehicleData Asset identity를 HUD 실루엣 Texture에 연결합니다. 미등록 차량은 기본 차량 실루엣을 사용합니다."))
	TArray<FCFHUDVehicleSilhouetteEntry> VehicleSilhouettes;

	// [v1.3.0] 모든 Armor Sector가 공유할 방향 glyph 없는 tintable mechanical Plate Texture입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Armor", meta=(DisplayName="공통 장갑 Plate (Common Armor Plate)", ToolTip="WBP_CFArmorSector 6개가 공통으로 사용할 방향 glyph 없는 Plate Texture입니다. 방향 표시는 별도 Icon Image가 담당합니다."))
	TSoftObjectPtr<UTexture2D> ArmorCommonPlate;

	// [v1.3.0] Designer가 회전해 재사용할 canonical 우향 단일 Arrow Texture입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Armor", meta=(DisplayName="장갑 방향 Arrow (Armor Direction Arrow)", ToolTip="기본 방향이 화면 오른쪽인 단일 Arrow Texture입니다. 각 Armor Sector의 Designer Render Transform Rotation으로 방향을 정합니다."))
	TSoftObjectPtr<UTexture2D> ArmorDirectionArrow;

	// [v1.3.0] Designer가 회전해 재사용할 canonical 우향 이중 Chevron Texture입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Armor", meta=(DisplayName="장갑 방향 이중 Chevron (Armor Direction Chevron2)", ToolTip="기본 방향이 화면 오른쪽인 이중 Chevron Texture입니다. 각 Armor Sector의 Designer Render Transform Rotation으로 방향을 정합니다."))
	TSoftObjectPtr<UTexture2D> ArmorDirectionChevron2;

	// [v1.0.0] migration 동안 기존 Content가 계속 사용할 여섯 방향 완성 Armor Plate compatibility 묶음입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Vehicle", meta=(DisplayName="Legacy 방향별 장갑 이미지 (Legacy Directional Armor Images)", ToolTip="기존 Front, Right, Rear, Left, Top, Bottom 완성 Plate의 compatibility fallback입니다. 신규 Production은 공통 Plate + 2종 방향 Icon을 사용합니다."))
	FCFHUDArmorVisualSet ArmorPlates;

	// [v1.4.0] Shield/Integrity가 공통으로 사용할 mechanical frame + dark track shell Texture입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Defense", meta=(DisplayName="방어 공통 프레임 (Defense Common Frame)", ToolTip="Shield와 Integrity가 공통으로 사용할 mechanical frame과 dark track shell Texture입니다. 실제 값과 semantic 색상은 포함하지 않습니다."))
	TSoftObjectPtr<UTexture2D> DefenseBarFrame;

	// [v1.4.0] Defense Fill 영역의 사선 시작형 coverage shape를 정의하는 grayscale mask Texture입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Defense", meta=(DisplayName="방어 Fill 마스크 (Defense Fill Mask)", ToolTip="Shield/Integrity Fill의 유효 영역 모양만 정의하는 grayscale mask입니다. Runtime Ratio와 색상은 Material/ProgressBar가 소유합니다."))
	TSoftObjectPtr<UTexture2D> DefenseFillMask;

	// [v1.4.0] Defense Fill 내부에 반복될 grayscale honeycomb pattern Texture입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Defense", meta=(DisplayName="방어 Honeycomb 패턴 (Defense Honeycomb Pattern)", ToolTip="Shield/Integrity Fill 내부에 반복 샘플링할 grayscale honeycomb pattern Texture입니다. semantic 색상은 Material parameter로 적용합니다."))
	TSoftObjectPtr<UTexture2D> DefenseHexPattern;

	// [v1.4.0] Recovery/Repair 상태에서 현재 Fill 끝을 따라 1~3개 반복 표시할 canonical 우향 Chevron Texture입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Defense", meta=(DisplayName="방어 회복 Chevron (Defense Recovery Chevron)", ToolTip="Shield 재생 또는 Integrity 수리 중 현재 Fill 끝을 따라 1~3개 표시할 단순 굵은 우향 Chevron Texture입니다."))
	TSoftObjectPtr<UTexture2D> DefenseChevron;

	// [v1.4.0] Fill Mask, honeycomb pattern과 semantic color를 조합할 UI Domain Material입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|HUD|Visual|Defense", meta=(DisplayName="방어 Fill UI Material (Defense Fill UI Material)", ToolTip="Shield/Integrity 공통 Fill Material입니다. FillMask와 HexPattern을 조합하고 DefenseColor/PatternStrength 같은 표현 파라미터를 적용합니다. Runtime Ratio sink는 별도 ProgressBar가 소유합니다."))
	TSoftObjectPtr<UMaterialInterface> DefenseFillMaterial;

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


	// [v1.3.0] 현재 VehicleData identity에 맞는 catalog 실루엣을 찾고 없으면 기본 VehicleSilhouette fallback을 반환합니다.
	TSoftObjectPtr<UTexture2D> ResolveVehicleSilhouette(const TSoftObjectPtr<UCFVehicleData>& VehicleDataAsset) const;

	// [v1.3.0] 공통 Plate와 Arrow/Chevron2 두 Icon이 모두 연결됐는지 Asset Load 없이 확인합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Visual", meta=(DisplayName="모듈형 장갑 아트 준비됨 (Has Modular Armor Art)", ToolTip="공통 Armor Plate와 canonical Arrow, Chevron2 두 방향 Icon Texture가 모두 연결됐는지 확인합니다."))
	bool HasModularArmorArt() const;

	// [v1.4.0] Defense common frame/mask/hex/chevron와 Fill Material이 모두 연결됐는지 Asset Load 없이 확인합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Visual", meta=(DisplayName="방어 HUD 아트 준비됨 (Has Defense HUD Art)", ToolTip="Shield/Integrity 공통 frame, fill mask, honeycomb pattern, recovery chevron과 UI Fill Material이 모두 연결됐는지 확인합니다."))
	bool HasDefenseArt() const;

	// [v1.3.0] VehiclePanel 핵심 전용 아트가 legacy 또는 modular Armor 경로 중 하나로 준비됐는지 Asset Load 없이 확인합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Visual", meta=(DisplayName="차량 HUD 아트 준비됨 (Has Complete Vehicle HUD Art)", ToolTip="Vehicle Panel Frame, 하나 이상의 차량 실루엣 source, legacy 6방향 Plate 또는 modular 공통 Plate+2 Icon, 그리고 RPM Track Texture 또는 UI Material이 준비됐는지 확인합니다."))
	bool HasCompleteVehicleArt() const;

		// [v1.0.0] Radar의 최소 정적/동적 전용 아트 중 하나 이상이 연결됐는지 Asset Load 없이 확인합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Visual", meta=(DisplayName="레이더 HUD 아트 있음 (Has Radar HUD Art)", ToolTip="Radar Frame 또는 Radar Sweep Material 중 하나 이상이 연결됐는지 확인합니다."))
	bool HasRadarArt() const;

	// [v1.2.0] UI-P0-08B Production Radar presentation에 필요한 Frame/Blip/Player/Selection Texture가 모두 연결됐는지 Asset Load 없이 확인합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|HUD|Visual", meta=(DisplayName="레이더 표현 아트 준비됨 (Has Complete Radar Presentation Art)", ToolTip="Radar Frame, Friendly/Hostile/Unknown Blip, Player Marker, 4-Corner Selected Target Bracket과 2-Corner Selected Edge Bracket이 모두 연결됐는지 확인합니다. Sweep Material은 선택적이라 포함하지 않습니다."))
	bool HasCompleteRadarPresentationArt() const;
};
