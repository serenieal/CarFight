// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-10
// Description: CarFight HUD Layout Profile DataAsset 타입
// Scope: D1-09A에서 승인된 1920x1080 Slot 배치, 내부 Geometry/Typography Scale과 Font 하한 계약을 제공합니다.
// Changelog:
// - v1.0.0: HUD Slot ID, Slot Layout, Typography Floor와 UCFHUDLayoutData Native 1080p Fallback을 최초 추가.
// Migration:
// - D1-07 SR-1080-16 USER PASS의 Screen-space 위치·크기를 Native Fallback으로 보존합니다.
// - 1440p·21:9·32:9 Profile은 Deferred이며 이 타입 추가로 생성하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UI/CFUIStyleData.h"
#include "CFHUDLayoutData.generated.h"

/**
 * HUD Layout Profile이 소유하는 고정 Slot 식별자입니다.
 */
UENUM(BlueprintType)
enum class ECFHUDSlotId : uint8
{
	MissionSummary UMETA(DisplayName="임무 요약 (Mission Summary)"),
	AlertFeed UMETA(DisplayName="경고 피드 (Alert Feed)"),
	TargetPanel UMETA(DisplayName="타겟 패널 (Target Panel)"),
	VehiclePanel UMETA(DisplayName="차량 패널 (Vehicle Panel)"),
	RadarPanel UMETA(DisplayName="레이더 패널 (Radar Panel)"),
	WeaponPanel UMETA(DisplayName="무기 패널 (Weapon Panel)"),
	ReticleLayer UMETA(DisplayName="레티클 레이어 (Reticle Layer)")
};

/**
 * 한 HUD Slot의 Anchor·Offset·크기·ZOrder를 정의합니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFHUDSlotLayout
{
	GENERATED_BODY()

	// [v1.0.0] 비어 있는 Slot Layout을 안전한 기본값으로 준비합니다.
	FCFHUDSlotLayout();

	// [v1.0.0] 이 Layout Entry가 배치하는 안정적인 Slot ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="HUD Slot ID", ToolTip="Mission, Alert, Target, Vehicle, Radar, Weapon, Reticle 중 하나의 고정 Slot 식별자입니다."))
	ECFHUDSlotId SlotId = ECFHUDSlotId::MissionSummary;

	// [v1.0.0] Canvas Slot의 최소 Anchor입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="최소 Anchor (Anchor Minimum)", ToolTip="0~1 정규화 좌표의 Canvas 최소 Anchor입니다."))
	FVector2D AnchorMinimum = FVector2D::ZeroVector;

	// [v1.0.0] Canvas Slot의 최대 Anchor입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="최대 Anchor (Anchor Maximum)", ToolTip="0~1 정규화 좌표의 Canvas 최대 Anchor입니다. ReticleLayer는 1,1까지 Stretch합니다."))
	FVector2D AnchorMaximum = FVector2D::ZeroVector;

	// [v1.0.0] Slot Desired Size가 Anchor 기준 어느 점에 맞춰질지 결정하는 Alignment입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="정렬 (Alignment)", ToolTip="Canvas Slot의 0~1 Alignment입니다."))
	FVector2D Alignment = FVector2D::ZeroVector;

	// [v1.0.0] Reference Viewport에서 사용할 최종 Screen-space 위치 Offset입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="픽셀 위치 Offset (Pixel Offset)", ToolTip="ReferenceViewportSize 기준 최종 Screen-space Offset입니다. 내부 Geometry Scale을 다시 곱하지 않습니다."))
	FVector2D PixelOffset = FVector2D::ZeroVector;

	// [v1.0.0] Reference Viewport에서 사용할 최종 Screen-space Slot 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="희망 크기 (Desired Size)", ToolTip="ReferenceViewportSize 기준 최종 Screen-space Slot 크기입니다. ReticleLayer Stretch는 0,0을 사용합니다."))
	FVector2D DesiredSize = FVector2D::ZeroVector;

	// [v1.0.0] 사용자·Panel Override가 없을 때 Slot 전체에 적용할 추가 Render Scale입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="렌더 스케일 (Render Scale)", ToolTip="Slot 전체의 추가 Render Scale입니다. 기본값은 1이며 내부 Token GeometryScale과 분리합니다."))
	float RenderScale = 1.0f;

	// [v1.0.0] 같은 HUD Canvas 안에서의 명시적 ZOrder입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="Z Order", ToolTip="HUD Canvas 안에서 Slot 사이의 표시 우선순위입니다."))
	int32 ZOrder = 10;

	// [v1.0.0] Profile 적용 시 이 Slot을 기본 표시 대상으로 둘지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="기본 표시 (Visible By Default)", ToolTip="사용자 Override가 없을 때 이 Slot을 기본 표시 대상으로 둘지 결정합니다."))
	bool bVisibleByDefault = true;
};

/**
 * Typography Role별 1920x1080 최소 실효 Font Size를 정의합니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUITypographyFloor
{
	GENERATED_BODY()

	// [v1.0.0] D1-07 1080p 판독성 하한을 기본값으로 준비합니다.
	FCFUITypographyFloor();

	// [v1.0.0] DisplayXL 최소 실효 Font Size입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout|Typography", meta=(DisplayName="Display XL 최소 크기"))
	int32 DisplayXL = 38;

	// [v1.0.0] DisplayL 최소 실효 Font Size입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout|Typography", meta=(DisplayName="Display L 최소 크기"))
	int32 DisplayL = 32;

	// [v1.0.0] ValueM 최소 실효 Font Size입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout|Typography", meta=(DisplayName="Value M 최소 크기"))
	int32 ValueM = 24;

	// [v1.0.0] ValueS 최소 실효 Font Size입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout|Typography", meta=(DisplayName="Value S 최소 크기"))
	int32 ValueS = 14;

	// [v1.0.0] HeadingL 최소 실효 Font Size입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout|Typography", meta=(DisplayName="Heading L 최소 크기"))
	int32 HeadingL = 22;

	// [v1.0.0] HeadingM 최소 실효 Font Size입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout|Typography", meta=(DisplayName="Heading M 최소 크기"))
	int32 HeadingM = 18;

	// [v1.0.0] Body 최소 실효 Font Size입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout|Typography", meta=(DisplayName="Body 최소 크기"))
	int32 Body = 16;

	// [v1.0.0] Label 최소 실효 Font Size입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout|Typography", meta=(DisplayName="Label 최소 크기"))
	int32 Label = 14;

	// [v1.0.0] Caption 최소 실효 Font Size입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Layout|Typography", meta=(DisplayName="Caption 최소 크기"))
	int32 Caption = 12;
};

/**
 * HUD Slot 배치와 해상도별 내부 Scale을 소유하는 Layout Profile DataAsset입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFHUDLayoutData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] D1-07 USER PASS 1920x1080 Profile을 Native Safe Fallback으로 준비합니다.
	UCFHUDLayoutData();

	// [v1.0.0] 사람이 읽고 Migration에서 식별할 Layout Profile ID입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="프로필 ID (Profile ID)", ToolTip="현재 Layout Profile의 안정적인 논리 ID입니다."))
	FName ProfileId;

	// [v1.0.0] 향후 사용자 Layout Override Migration에 사용할 Profile Version입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="프로필 버전 (Profile Version)", ToolTip="Layout 계약 변경 시 증가시키는 정수 버전입니다."))
	int32 ProfileVersion = 1;

	// [v1.0.0] PixelOffset과 DesiredSize가 직접 표현하는 기준 Viewport 크기입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="기준 Viewport 크기 (Reference Viewport Size)", ToolTip="Slot Screen-space 값이 직접 대응하는 기준 해상도입니다."))
	FVector2D ReferenceViewportSize = FVector2D(1920.0, 1080.0);

	// [v1.0.0] Density·Shape·Icon 같은 내부 Design Token에 적용할 해상도 Geometry Scale입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="Geometry 스케일 (Geometry Scale)", ToolTip="Panel 내부 Padding, Gap, Icon, Outline 같은 Design Token에 적용합니다. Slot 위치·크기에는 다시 곱하지 않습니다."))
	float GeometryScale = 0.75f;

	// [v1.0.0] Typography Token에 적용하고 Role별 안전 하한으로 Clamp할 해상도 Scale입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="Typography 스케일 (Typography Scale)", ToolTip="Typography 기본 크기에 적용한 뒤 MinimumEffectiveFontSizes로 Clamp합니다."))
	float TypographyScale = 0.75f;

	// [v1.0.0] Typography Scale 이후에도 지켜야 하는 Role별 최소 실효 Font Size입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="최소 실효 Font 크기 (Minimum Effective Font Sizes)", ToolTip="1920x1080에서 D1-07 판독성 하한을 보호합니다."))
	FCFUITypographyFloor MinimumEffectiveFontSizes;

	// [v1.0.0] 이 Profile이 소유하는 정확한 HUD Slot 배치 목록입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Layout", meta=(DisplayName="HUD Slot 배치 (HUD Slot Layouts)", ToolTip="Mission, Alert, Target, Vehicle, Radar, Weapon, Reticle 7개 Slot을 정확히 한 번씩 포함합니다."))
	TArray<FCFHUDSlotLayout> SlotLayouts;

	// [v1.0.0] 지정 Slot ID의 Layout Entry를 찾고 없으면 Null을 반환합니다.
	const FCFHUDSlotLayout* FindSlotLayout(ECFHUDSlotId SlotId) const;

	// [v1.0.0] 지정 Typography Role의 이 Profile 최소 실효 Font Size를 반환합니다.
	int32 ResolveMinimumFontSize(ECFUITypographyRole TypographyRole) const;

	// [v1.0.0] Profile 기본값, 7개 Slot 유일성, Anchor·Scale·크기 계약을 검증합니다.
	bool ValidateLayoutData(FString& OutFailureReason) const;
};
