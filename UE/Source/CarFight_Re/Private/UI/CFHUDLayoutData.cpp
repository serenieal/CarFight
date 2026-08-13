// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-10
// Description: CarFight HUD Layout Profile DataAsset 기본값과 검증 구현
// Scope: D1-07 USER PASS 1920x1080 Slot을 Native Fallback으로 제공하고 D1-09A Layout 계약을 검증합니다.
// Changelog:
// - v1.0.0: 1080p 7-Slot Native Profile, Typography Floor 해석과 Layout 검증을 최초 구현.
// Migration:
// - 1440p·21:9·32:9 Profile은 이 Native Fallback에 합치지 않고 후속 DataAsset으로 분리합니다.

#include "UI/CFHUDLayoutData.h"

namespace
{
	// [v1.0.0] 한 HUD Slot의 Anchor·Alignment·Screen-space Geometry를 일관되게 생성합니다.
	FCFHUDSlotLayout MakeHUDSlotLayout(
		const ECFHUDSlotId SlotId,
		const FVector2D& AnchorMinimum,
		const FVector2D& AnchorMaximum,
		const FVector2D& Alignment,
		const FVector2D& PixelOffset,
		const FVector2D& DesiredSize,
		const int32 ZOrder)
	{
		// [v1.0.0] 생성 인수를 그대로 보존할 HUD Slot Layout입니다.
		FCFHUDSlotLayout SlotLayout;
		SlotLayout.SlotId = SlotId;
		SlotLayout.AnchorMinimum = AnchorMinimum;
		SlotLayout.AnchorMaximum = AnchorMaximum;
		SlotLayout.Alignment = Alignment;
		SlotLayout.PixelOffset = PixelOffset;
		SlotLayout.DesiredSize = DesiredSize;
		SlotLayout.RenderScale = 1.0f;
		SlotLayout.ZOrder = ZOrder;
		SlotLayout.bVisibleByDefault = true;
		return SlotLayout;
	}

	// [v1.0.0] 한 Anchor 축 값이 UMG 정규화 범위 0~1 안에 있는지 확인합니다.
	bool IsNormalizedAnchorValue(const double AnchorValue)
	{
		return AnchorValue >= 0.0 && AnchorValue <= 1.0;
	}
}

// [v1.0.0] 비어 있는 Slot Layout을 안전한 기본값으로 준비합니다.
FCFHUDSlotLayout::FCFHUDSlotLayout() = default;

// [v1.0.0] D1-07 1080p 판독성 하한을 기본값으로 준비합니다.
FCFUITypographyFloor::FCFUITypographyFloor() = default;

// [v1.0.0] D1-07 USER PASS 1920x1080 Profile을 Native Safe Fallback으로 준비합니다.
UCFHUDLayoutData::UCFHUDLayoutData()
	: ProfileId(TEXT("HUD_1080_16"))
{
	SlotLayouts.Reserve(7);
	SlotLayouts.Add(MakeHUDSlotLayout(
		ECFHUDSlotId::MissionSummary,
		FVector2D(0.0, 0.0), FVector2D(0.0, 0.0), FVector2D(0.0, 0.0),
		FVector2D(24.0, 24.0), FVector2D(375.0, 87.0), 10));
	SlotLayouts.Add(MakeHUDSlotLayout(
		ECFHUDSlotId::AlertFeed,
		FVector2D(0.5, 0.0), FVector2D(0.5, 0.0), FVector2D(0.5, 0.0),
		FVector2D(6.0, 24.0), FVector2D(552.0, 81.0), 40));
	SlotLayouts.Add(MakeHUDSlotLayout(
		ECFHUDSlotId::TargetPanel,
		FVector2D(1.0, 0.0), FVector2D(1.0, 0.0), FVector2D(1.0, 0.0),
		FVector2D(-24.0, 24.0), FVector2D(312.0, 232.5), 10));
	SlotLayouts.Add(MakeHUDSlotLayout(
		ECFHUDSlotId::VehiclePanel,
		FVector2D(0.0, 1.0), FVector2D(0.0, 1.0), FVector2D(0.0, 1.0),
		FVector2D(24.0, -24.0), FVector2D(672.0, 312.0), 10));
	SlotLayouts.Add(MakeHUDSlotLayout(
		ECFHUDSlotId::RadarPanel,
		FVector2D(0.5, 1.0), FVector2D(0.5, 1.0), FVector2D(0.5, 1.0),
		FVector2D(0.0, -24.0), FVector2D(285.0, 270.0), 10));
	SlotLayouts.Add(MakeHUDSlotLayout(
		ECFHUDSlotId::WeaponPanel,
		FVector2D(1.0, 1.0), FVector2D(1.0, 1.0), FVector2D(1.0, 1.0),
		FVector2D(-24.0, -24.0), FVector2D(270.0, 190.5), 10));
	SlotLayouts.Add(MakeHUDSlotLayout(
		ECFHUDSlotId::ReticleLayer,
		FVector2D(0.0, 0.0), FVector2D(1.0, 1.0), FVector2D(0.0, 0.0),
		FVector2D::ZeroVector, FVector2D::ZeroVector, 30));
}

// [v1.0.0] 지정 Slot ID의 Layout Entry를 찾고 없으면 Null을 반환합니다.
const FCFHUDSlotLayout* UCFHUDLayoutData::FindSlotLayout(const ECFHUDSlotId SlotId) const
{
	return SlotLayouts.FindByPredicate([SlotId](const FCFHUDSlotLayout& SlotLayout)
	{
		return SlotLayout.SlotId == SlotId;
	});
}

// [v1.0.0] 지정 Typography Role의 이 Profile 최소 실효 Font Size를 반환합니다.
int32 UCFHUDLayoutData::ResolveMinimumFontSize(const ECFUITypographyRole TypographyRole) const
{
	switch (TypographyRole)
	{
	case ECFUITypographyRole::DisplayXL: return MinimumEffectiveFontSizes.DisplayXL;
	case ECFUITypographyRole::DisplayL: return MinimumEffectiveFontSizes.DisplayL;
	case ECFUITypographyRole::ValueM: return MinimumEffectiveFontSizes.ValueM;
	case ECFUITypographyRole::ValueS: return MinimumEffectiveFontSizes.ValueS;
	case ECFUITypographyRole::HeadingL: return MinimumEffectiveFontSizes.HeadingL;
	case ECFUITypographyRole::HeadingM: return MinimumEffectiveFontSizes.HeadingM;
	case ECFUITypographyRole::Body: return MinimumEffectiveFontSizes.Body;
	case ECFUITypographyRole::Label: return MinimumEffectiveFontSizes.Label;
	case ECFUITypographyRole::Caption: return MinimumEffectiveFontSizes.Caption;
	default: return MinimumEffectiveFontSizes.Body;
	}
}

// [v1.0.0] Profile 기본값, 7개 Slot 유일성, Anchor·Scale·크기 계약을 검증합니다.
bool UCFHUDLayoutData::ValidateLayoutData(FString& OutFailureReason) const
{
	OutFailureReason.Reset();

	if (ProfileId.IsNone() || ProfileVersion <= 0)
	{
		OutFailureReason = TEXT("HUD Layout ProfileId가 비었거나 ProfileVersion이 1보다 작습니다.");
		return false;
	}

	if (ReferenceViewportSize.X <= 0.0 || ReferenceViewportSize.Y <= 0.0 || GeometryScale <= 0.0f || TypographyScale <= 0.0f)
	{
		OutFailureReason = TEXT("Reference Viewport와 Geometry/Typography Scale은 0보다 커야 합니다.");
		return false;
	}

	const int32 TypographyFloors[] =
	{
		MinimumEffectiveFontSizes.DisplayXL,
		MinimumEffectiveFontSizes.DisplayL,
		MinimumEffectiveFontSizes.ValueM,
		MinimumEffectiveFontSizes.ValueS,
		MinimumEffectiveFontSizes.HeadingL,
		MinimumEffectiveFontSizes.HeadingM,
		MinimumEffectiveFontSizes.Body,
		MinimumEffectiveFontSizes.Label,
		MinimumEffectiveFontSizes.Caption
	};
	for (const int32 TypographyFloor : TypographyFloors)
	{
		if (TypographyFloor <= 0)
		{
			OutFailureReason = TEXT("Typography 최소 실효 Font Size는 모두 0보다 커야 합니다.");
			return false;
		}
	}

	if (SlotLayouts.Num() != 7)
	{
		OutFailureReason = TEXT("HUD Layout은 D1-09A 기준 정확히 7개 Required Slot을 포함해야 합니다.");
		return false;
	}

	// [v1.0.0] 중복 Slot ID와 Required Slot 누락을 함께 검출할 방문 집합입니다.
	TSet<ECFHUDSlotId> SeenSlotIds;
	for (const FCFHUDSlotLayout& SlotLayout : SlotLayouts)
	{
		if (SeenSlotIds.Contains(SlotLayout.SlotId))
		{
			OutFailureReason = TEXT("HUD Layout에 중복 Slot ID가 존재합니다.");
			return false;
		}
		SeenSlotIds.Add(SlotLayout.SlotId);

		if (!IsNormalizedAnchorValue(SlotLayout.AnchorMinimum.X) || !IsNormalizedAnchorValue(SlotLayout.AnchorMinimum.Y)
			|| !IsNormalizedAnchorValue(SlotLayout.AnchorMaximum.X) || !IsNormalizedAnchorValue(SlotLayout.AnchorMaximum.Y)
			|| SlotLayout.AnchorMinimum.X > SlotLayout.AnchorMaximum.X || SlotLayout.AnchorMinimum.Y > SlotLayout.AnchorMaximum.Y)
		{
			OutFailureReason = TEXT("HUD Slot Anchor가 0~1 범위를 벗어나거나 Minimum이 Maximum보다 큽니다.");
			return false;
		}

		if (SlotLayout.Alignment.X < 0.0 || SlotLayout.Alignment.X > 1.0 || SlotLayout.Alignment.Y < 0.0 || SlotLayout.Alignment.Y > 1.0)
		{
			OutFailureReason = TEXT("HUD Slot Alignment는 0~1 범위여야 합니다.");
			return false;
		}

		if (SlotLayout.DesiredSize.X < 0.0 || SlotLayout.DesiredSize.Y < 0.0 || SlotLayout.RenderScale <= 0.0f)
		{
			OutFailureReason = TEXT("HUD Slot DesiredSize는 음수일 수 없고 RenderScale은 0보다 커야 합니다.");
			return false;
		}

		if (SlotLayout.SlotId != ECFHUDSlotId::ReticleLayer && (SlotLayout.DesiredSize.X <= 0.0 || SlotLayout.DesiredSize.Y <= 0.0))
		{
			OutFailureReason = TEXT("ReticleLayer 외 고정 HUD Slot은 0보다 큰 DesiredSize를 가져야 합니다.");
			return false;
		}
	}

	for (uint8 SlotIndex = static_cast<uint8>(ECFHUDSlotId::MissionSummary); SlotIndex <= static_cast<uint8>(ECFHUDSlotId::ReticleLayer); ++SlotIndex)
	{
		if (!SeenSlotIds.Contains(static_cast<ECFHUDSlotId>(SlotIndex)))
		{
			OutFailureReason = TEXT("HUD Layout에 Required Slot이 누락됐습니다.");
			return false;
		}
	}

	return true;
}
