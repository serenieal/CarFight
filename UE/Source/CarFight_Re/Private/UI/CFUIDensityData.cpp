// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-10
// Description: CarFight UI Density Profile 기본값과 검증 구현
// Scope: D1-05에서 승인된 Compact·Standard·Expanded 값을 Native Profile로 제공합니다.
// Changelog:
// - v1.0.0: 세 Density Preset 적용과 안전 값 검증을 최초 구현.
// Migration:
// - Custom은 현재 값을 유지하며 실제 사용자 HUD 편집은 이번 Gate에서 구현하지 않습니다.

#include "UI/CFUIDensityData.h"

// [v1.0.0] Standard Density 기본값을 Native Fallback으로 준비합니다.
FCFUIDensityTokens::FCFUIDensityTokens() = default;

// [v1.0.0] Standard Density를 Native Safe Fallback으로 준비합니다.
UCFUIDensityData::UCFUIDensityData()
{
	ApplyPresetDefaults(ECFUIDensityPreset::Standard);
}

// [v1.0.0] 지정 Preset의 승인 기본값을 현재 Tokens에 적용합니다.
void UCFUIDensityData::ApplyPresetDefaults(const ECFUIDensityPreset NewPreset)
{
	Preset = NewPreset;

	switch (NewPreset)
	{
	case ECFUIDensityPreset::Compact:
		Tokens.PanelPadding = 12.0f;
		Tokens.PanelInnerGap = 8.0f;
		Tokens.SectionGap = 8.0f;
		Tokens.InfoRowGap = 4.0f;
		Tokens.HeaderHeight = 36.0f;
		Tokens.InfoRowHeight = 28.0f;
		Tokens.InfoRowHeightLarge = 36.0f;
		Tokens.ChipHeight = 22.0f;
		Tokens.IconSmall = 16.0f;
		Tokens.IconMedium = 20.0f;
		Tokens.IconLarge = 28.0f;
		Tokens.PrimaryValueScale = 0.86f;
		Tokens.SecondaryTextScale = 0.90f;
		Tokens.OutlineScale = 0.75f;
		Tokens.CaptionPolicy = ECFUICaptionPolicy::CoreOnly;
		break;

	case ECFUIDensityPreset::Expanded:
		Tokens.PanelPadding = 32.0f;
		Tokens.PanelInnerGap = 16.0f;
		Tokens.SectionGap = 24.0f;
		Tokens.InfoRowGap = 12.0f;
		Tokens.HeaderHeight = 56.0f;
		Tokens.InfoRowHeight = 44.0f;
		Tokens.InfoRowHeightLarge = 52.0f;
		Tokens.ChipHeight = 32.0f;
		Tokens.IconSmall = 24.0f;
		Tokens.IconMedium = 32.0f;
		Tokens.IconLarge = 40.0f;
		Tokens.PrimaryValueScale = 1.12f;
		Tokens.SecondaryTextScale = 1.08f;
		Tokens.OutlineScale = 1.0f;
		Tokens.CaptionPolicy = ECFUICaptionPolicy::Expanded;
		break;

	case ECFUIDensityPreset::Custom:
		break;

	case ECFUIDensityPreset::Standard:
	default:
		Tokens.PanelPadding = 24.0f;
		Tokens.PanelInnerGap = 12.0f;
		Tokens.SectionGap = 16.0f;
		Tokens.InfoRowGap = 8.0f;
		Tokens.HeaderHeight = 48.0f;
		Tokens.InfoRowHeight = 36.0f;
		Tokens.InfoRowHeightLarge = 44.0f;
		Tokens.ChipHeight = 28.0f;
		Tokens.IconSmall = 20.0f;
		Tokens.IconMedium = 24.0f;
		Tokens.IconLarge = 32.0f;
		Tokens.PrimaryValueScale = 1.0f;
		Tokens.SecondaryTextScale = 1.0f;
		Tokens.OutlineScale = 1.0f;
		Tokens.CaptionPolicy = ECFUICaptionPolicy::Contextual;
		break;
	}
}

// [v1.0.0] Density Token이 음수 Geometry나 0 이하 Scale을 포함하지 않는지 검증합니다.
bool UCFUIDensityData::ValidateDensityData(FString& OutFailureReason) const
{
	OutFailureReason.Reset();

	const float GeometryValues[] =
	{
		Tokens.PanelPadding,
		Tokens.PanelInnerGap,
		Tokens.SectionGap,
		Tokens.InfoRowGap,
		Tokens.HeaderHeight,
		Tokens.InfoRowHeight,
		Tokens.InfoRowHeightLarge,
		Tokens.ChipHeight,
		Tokens.IconSmall,
		Tokens.IconMedium,
		Tokens.IconLarge
	};
	for (const float GeometryValue : GeometryValues)
	{
		if (GeometryValue < 0.0f)
		{
			OutFailureReason = TEXT("Density Geometry와 Icon 크기는 음수일 수 없습니다.");
			return false;
		}
	}

	if (Tokens.HeaderHeight <= 0.0f || Tokens.InfoRowHeight <= 0.0f || Tokens.InfoRowHeightLarge <= 0.0f || Tokens.IconSmall <= 0.0f || Tokens.IconMedium <= 0.0f || Tokens.IconLarge <= 0.0f)
	{
		OutFailureReason = TEXT("Density의 Header, InfoRow와 Icon 크기는 0보다 커야 합니다.");
		return false;
	}

	if (Tokens.PrimaryValueScale <= 0.0f || Tokens.SecondaryTextScale <= 0.0f || Tokens.OutlineScale <= 0.0f)
	{
		OutFailureReason = TEXT("Density의 Typography와 Outline Scale은 0보다 커야 합니다.");
		return false;
	}

	return true;
}
