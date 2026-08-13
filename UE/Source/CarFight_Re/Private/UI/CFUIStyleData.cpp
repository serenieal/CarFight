// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-10
// Description: CarFight 공통 UI Style Token과 DataAsset 기본값 구현
// Scope: D1-08 Native Style Fallback과 D1-09 Font/Icon Asset 의미 해석을 제공합니다.
// Changelog:
// - v1.2.0: Semantic ID 기반 Soft Icon Asset Resolver와 Icon Set ID/Asset 유효성·중복 검증을 추가.
// - v1.1.0: UFont Family Binding, Weight별 Typeface 이름과 Typography Scale·하한 기반 FSlateFontInfo 해석을 추가.
// - v1.0.0: 모든 Style Token 기본값, 의미 Color·Typography 해석과 최소 안전 하한 검증을 최초 구현.
// Migration:
// - Font Asset이 없는 Native Fallback은 Engine Core Style Font를 사용하고 D1-09B DA_CFUIStyle이 실제 UFont를 연결하면 해당 Font를 우선합니다.
// - Semantic Icon Resolver는 Soft Reference만 반환하며 동기 Load하지 않습니다.
// - 콘텐츠 경로는 이 파일에 하드코딩하지 않습니다.

#include "UI/CFUIStyleData.h"

#include "Engine/Font.h"
#include "Styling/CoreStyle.h"

namespace
{
	// [v1.0.0] 문서의 sRGB Hex와 별도 Alpha를 Unreal FLinearColor로 변환합니다.
		FLinearColor MakeStyleColor(const TCHAR* HexValue, const float Alpha)
	{
		// [v1.0.0] 문서 Hex 문자열을 변환한 뒤 별도 Alpha를 적용할 sRGB 색상입니다.
		FColor SRGBColor = FColor::FromHex(FString(HexValue));
		SRGBColor.A = static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(Alpha * 255.0f), 0, 255));
		return FLinearColor::FromSRGBColor(SRGBColor);
	}

	// [v1.0.0] Typography Role 기본값을 한 번에 설정합니다.
	void ConfigureTypographyStyle(FCFUITypographyStyle& Style, const int32 FontSize, const int32 LineHeight, const ECFUIFontWeight Weight)
	{
		Style.FontSize = FontSize;
		Style.LineHeight = LineHeight;
		Style.Weight = Weight;
	}

	// [v1.0.0] Status Bar Variant의 Density 높이·Fill 의미를 한 번에 설정합니다.
	void ConfigureStatusBarVariant(
		FCFUIStatusBarVariantStyle& Style,
		const float CompactHeight,
		const float StandardHeight,
		const float ExpandedHeight,
		const ECFUIColorToken FillColorToken,
		const bool bUseStateColor,
		const int32 VisualSegmentCount)
	{
		Style.CompactHeight = CompactHeight;
		Style.StandardHeight = StandardHeight;
		Style.ExpandedHeight = ExpandedHeight;
		Style.FillColorToken = FillColorToken;
		Style.bUseStateColor = bUseStateColor;
		Style.VisualSegmentCount = VisualSegmentCount;
	}
}

// [v1.0.0] Typography Role의 안전한 일반 본문 Fallback을 준비합니다.
FCFUITypographyStyle::FCFUITypographyStyle() = default;

// [v1.0.0] D1-05 승인 Palette를 Native Safe Fallback으로 설정합니다.
FCFUIColorTokens::FCFUIColorTokens()
{
	SurfaceBase = MakeStyleColor(TEXT("#0B1117"), 0.80f);
	SurfaceRaised = MakeStyleColor(TEXT("#111B24"), 0.88f);
	SurfaceOverlay = MakeStyleColor(TEXT("#17232D"), 0.94f);
	SurfaceSoft = MakeStyleColor(TEXT("#111B24"), 0.56f);
	DimOverlay = MakeStyleColor(TEXT("#02070B"), 0.72f);
	LineSubtle = MakeStyleColor(TEXT("#324653"), 0.65f);
	LineDefault = MakeStyleColor(TEXT("#577080"), 0.82f);
	LineStrong = MakeStyleColor(TEXT("#7F98A8"), 0.92f);
	TextPrimary = MakeStyleColor(TEXT("#EAF2F7"), 1.00f);
	TextSecondary = MakeStyleColor(TEXT("#A3B2BC"), 1.00f);
	TextMuted = MakeStyleColor(TEXT("#7A8A95"), 1.00f);
	TextDisabled = MakeStyleColor(TEXT("#61717C"), 0.88f);
	TextOnAccent = MakeStyleColor(TEXT("#071015"), 1.00f);
	AccentTactical = MakeStyleColor(TEXT("#55C7E8"), 1.00f);
	AccentHover = MakeStyleColor(TEXT("#75D5EF"), 1.00f);
	AccentPressed = MakeStyleColor(TEXT("#36A8C9"), 1.00f);
	StateNotice = MakeStyleColor(TEXT("#72C7D9"), 1.00f);
	StateCaution = MakeStyleColor(TEXT("#F1B84B"), 1.00f);
	StateDanger = MakeStyleColor(TEXT("#FF714D"), 1.00f);
	StateCritical = MakeStyleColor(TEXT("#FF3F46"), 1.00f);
	StateDisabled = MakeStyleColor(TEXT("#65727B"), 0.90f);
	StateSuccess = MakeStyleColor(TEXT("#6FC7A2"), 1.00f);
	ShieldColor = MakeStyleColor(TEXT("#59D5E6"), 1.00f);
	ArmorColor = MakeStyleColor(TEXT("#D6A448"), 1.00f);
	IntegrityColor = MakeStyleColor(TEXT("#F0644F"), 1.00f);
	FriendlyColor = MakeStyleColor(TEXT("#53A9FF"), 1.00f);
	NeutralColor = MakeStyleColor(TEXT("#D5C777"), 1.00f);
	HostileColor = MakeStyleColor(TEXT("#FF644F"), 1.00f);
	UnknownColor = MakeStyleColor(TEXT("#A7ADB3"), 1.00f);
	EstimatedColor = MakeStyleColor(TEXT("#B5C2C9"), 1.00f);
}

// [v1.0.0] D1-06 Font Preset과 D1-05 Typography Role 기본값을 설정합니다.
FCFUITypographyTokens::FCFUITypographyTokens()
	: UIFontFamily(TEXT("Pretendard"))
	, NumericFontFamily(TEXT("IBM Plex Mono"))
{
	ConfigureTypographyStyle(DisplayXL, 52, 56, ECFUIFontWeight::Bold);
	ConfigureTypographyStyle(DisplayL, 44, 48, ECFUIFontWeight::Bold);
	ConfigureTypographyStyle(ValueM, 32, 36, ECFUIFontWeight::SemiBold);
	ConfigureTypographyStyle(ValueS, 20, 24, ECFUIFontWeight::SemiBold);
	ConfigureTypographyStyle(HeadingL, 28, 34, ECFUIFontWeight::SemiBold);
	ConfigureTypographyStyle(HeadingM, 24, 30, ECFUIFontWeight::SemiBold);
	ConfigureTypographyStyle(Body, 20, 26, ECFUIFontWeight::Regular);
	ConfigureTypographyStyle(Label, 16, 22, ECFUIFontWeight::Medium);
	ConfigureTypographyStyle(Caption, 14, 20, ECFUIFontWeight::Regular);
}

// [v1.1.0] D1-09B Composite Font의 Weight별 Typeface 이름 기본값을 준비합니다.
FCFUIFontAssets::FCFUIFontAssets()
	: RegularTypefaceName(TEXT("Regular"))
	, MediumTypefaceName(TEXT("Medium"))
	, SemiBoldTypefaceName(TEXT("SemiBold"))
	, BoldTypefaceName(TEXT("Bold"))
{
}

// [v1.0.0] D1-05 Spacing·안전 크기 기본값을 명시적으로 유지합니다.
FCFUISpacingTokens::FCFUISpacingTokens() = default;

// [v1.0.0] D1-05 Shape·Outline 기본값을 명시적으로 유지합니다.
FCFUIShapeTokens::FCFUIShapeTokens() = default;

// [v1.0.0] D1-05 Opacity 기본값을 명시적으로 유지합니다.
FCFUIOpacityTokens::FCFUIOpacityTokens() = default;

// [v1.0.0] 일반 상태 변경용 Motion Fallback을 준비합니다.
FCFUIMotionStyle::FCFUIMotionStyle() = default;

// [v1.0.0] D1-05 Motion Duration·Easing·Loop 제한 기본값을 설정합니다.
FCFUIMotionTokens::FCFUIMotionTokens()
{
	Hover.DurationSeconds = 0.10f;
	Hover.Easing = ECFUIEasing::EaseOutCubic;

	Pressed.DurationSeconds = 0.08f;
	Pressed.Easing = ECFUIEasing::EaseOutQuad;

	State.DurationSeconds = 0.16f;
	State.Easing = ECFUIEasing::EaseInOutQuad;

	Panel.DurationSeconds = 0.20f;
	Panel.Easing = ECFUIEasing::EaseOutCubic;

	Gauge.DurationSeconds = 0.18f;
	Gauge.Easing = ECFUIEasing::EaseOutQuad;

	Alert.DurationSeconds = 0.36f;
	Alert.Easing = ECFUIEasing::EaseOutCubic;

	CriticalPulse.DurationSeconds = 0.70f;
	CriticalPulse.Easing = ECFUIEasing::EaseInOutSine;
	CriticalPulse.bAllowLoop = true;
}

// [v1.0.0] Panel Style은 의미 Color Token을 참조하고 실제 색은 Style Data에서 해석합니다.
FCFUIPanelStyle::FCFUIPanelStyle() = default;

// [v1.0.0] 일반 Button 상태 Fallback을 준비합니다.
FCFUIButtonStateStyle::FCFUIButtonStateStyle() = default;

// [v1.0.0] D1-05 Button 상태별 의미 Color·Outline 기본값을 설정합니다.
FCFUIButtonStyle::FCFUIButtonStyle()
{
	Normal.BackgroundColorToken = ECFUIColorToken::SurfaceRaised;
	Normal.OutlineColorToken = ECFUIColorToken::LineDefault;
	Normal.TextColorToken = ECFUIColorToken::TextPrimary;
	Normal.OutlineThickness = 2.0f;

	Hover.BackgroundColorToken = ECFUIColorToken::SurfaceRaised;
	Hover.OutlineColorToken = ECFUIColorToken::AccentHover;
	Hover.TextColorToken = ECFUIColorToken::TextPrimary;
	Hover.OutlineThickness = 2.0f;

	Focused.BackgroundColorToken = ECFUIColorToken::SurfaceRaised;
	Focused.OutlineColorToken = ECFUIColorToken::AccentTactical;
	Focused.TextColorToken = ECFUIColorToken::TextPrimary;
	Focused.OutlineThickness = 3.0f;

	Pressed.BackgroundColorToken = ECFUIColorToken::AccentPressed;
	Pressed.OutlineColorToken = ECFUIColorToken::AccentPressed;
	Pressed.TextColorToken = ECFUIColorToken::TextPrimary;
	Pressed.OutlineThickness = 3.0f;

	Disabled.BackgroundColorToken = ECFUIColorToken::SurfaceBase;
	Disabled.OutlineColorToken = ECFUIColorToken::LineSubtle;
	Disabled.TextColorToken = ECFUIColorToken::TextDisabled;
	Disabled.OutlineThickness = 1.0f;
}

// [v1.0.0] 일반 Status Bar Variant Fallback을 준비합니다.
FCFUIStatusBarVariantStyle::FCFUIStatusBarVariantStyle() = default;

// [v1.0.0] D1-05 Status Bar Variant별 Density 높이와 의미 Fill을 설정합니다.
FCFUIStatusBarStyle::FCFUIStatusBarStyle()
{
	ConfigureStatusBarVariant(Shield, 8.0f, 10.0f, 12.0f, ECFUIColorToken::Shield, false, 0);
	ConfigureStatusBarVariant(Armor, 10.0f, 12.0f, 14.0f, ECFUIColorToken::Armor, false, 10);
	ConfigureStatusBarVariant(Integrity, 12.0f, 16.0f, 20.0f, ECFUIColorToken::Integrity, false, 0);
	ConfigureStatusBarVariant(Resource, 8.0f, 10.0f, 12.0f, ECFUIColorToken::AccentTactical, true, 0);
	ConfigureStatusBarVariant(Progress, 6.0f, 8.0f, 10.0f, ECFUIColorToken::AccentTactical, false, 0);
}

// [v1.0.0] D1-05 Info Row Geometry와 지식 상태 기본 문구를 설정합니다.
FCFUIInfoRowStyle::FCFUIInfoRowStyle()
	: UnknownText(FText::FromString(TEXT("???")))
	, UnavailableText(FText::FromString(TEXT("N/A")))
{
}

// [v1.0.0] 일반 Alert Severity Fallback을 준비합니다.
FCFUIAlertSeverityStyle::FCFUIAlertSeverityStyle() = default;

// [v1.0.0] Notice·Warning·Critical Alert의 승인 기본 높이·Accent·지속시간을 설정합니다.
FCFUIAlertStyle::FCFUIAlertStyle()
{
	Notice.Height = 48.0f;
	Notice.AccentColorToken = ECFUIColorToken::StateNotice;
	Notice.DefaultDurationSeconds = 2.0f;
	Notice.bPersistentByDefault = false;

	Warning.Height = 56.0f;
	Warning.AccentColorToken = ECFUIColorToken::StateCaution;
	Warning.DefaultDurationSeconds = 3.0f;
	Warning.bPersistentByDefault = false;

	Critical.Height = 64.0f;
	Critical.AccentColorToken = ECFUIColorToken::StateCritical;
	Critical.DefaultDurationSeconds = 0.0f;
	Critical.bPersistentByDefault = true;
}

// [v1.0.0] 비어 있는 Semantic Icon Entry를 안전하게 초기화합니다.
FCFUISemanticIconEntry::FCFUISemanticIconEntry() = default;

// [v1.0.0] D1-06 Solid Core + Tactical Cut Icon Preset Geometry를 설정합니다.
FCFUIIconSet::FCFUIIconSet()
	: StyleName(TEXT("Solid Core + Tactical Cut"))
{
}

// [v1.0.0] 의미 기반 Color Token을 현재 Style Data의 실제 색으로 해석합니다.
FLinearColor UCFUIStyleData::ResolveColor(const ECFUIColorToken ColorToken) const
{
	switch (ColorToken)
	{
	case ECFUIColorToken::SurfaceBase: return Colors.SurfaceBase;
	case ECFUIColorToken::SurfaceRaised: return Colors.SurfaceRaised;
	case ECFUIColorToken::SurfaceOverlay: return Colors.SurfaceOverlay;
	case ECFUIColorToken::SurfaceSoft: return Colors.SurfaceSoft;
	case ECFUIColorToken::DimOverlay: return Colors.DimOverlay;
	case ECFUIColorToken::LineSubtle: return Colors.LineSubtle;
	case ECFUIColorToken::LineDefault: return Colors.LineDefault;
	case ECFUIColorToken::LineStrong: return Colors.LineStrong;
	case ECFUIColorToken::TextPrimary: return Colors.TextPrimary;
	case ECFUIColorToken::TextSecondary: return Colors.TextSecondary;
	case ECFUIColorToken::TextMuted: return Colors.TextMuted;
	case ECFUIColorToken::TextDisabled: return Colors.TextDisabled;
	case ECFUIColorToken::TextOnAccent: return Colors.TextOnAccent;
	case ECFUIColorToken::AccentTactical: return Colors.AccentTactical;
	case ECFUIColorToken::AccentHover: return Colors.AccentHover;
	case ECFUIColorToken::AccentPressed: return Colors.AccentPressed;
	case ECFUIColorToken::StateNotice: return Colors.StateNotice;
	case ECFUIColorToken::StateCaution: return Colors.StateCaution;
	case ECFUIColorToken::StateDanger: return Colors.StateDanger;
	case ECFUIColorToken::StateCritical: return Colors.StateCritical;
	case ECFUIColorToken::StateDisabled: return Colors.StateDisabled;
	case ECFUIColorToken::StateSuccess: return Colors.StateSuccess;
	case ECFUIColorToken::Shield: return Colors.ShieldColor;
	case ECFUIColorToken::Armor: return Colors.ArmorColor;
	case ECFUIColorToken::Integrity: return Colors.IntegrityColor;
	case ECFUIColorToken::Friendly: return Colors.FriendlyColor;
	case ECFUIColorToken::Neutral: return Colors.NeutralColor;
	case ECFUIColorToken::Hostile: return Colors.HostileColor;
	case ECFUIColorToken::Unknown: return Colors.UnknownColor;
	case ECFUIColorToken::Estimated: return Colors.EstimatedColor;
	default: return Colors.TextPrimary;
	}
}

// [v1.0.0] Typography Role을 현재 Style Data의 크기·행높이·Weight로 해석합니다.
FCFUITypographyStyle UCFUIStyleData::ResolveTypography(const ECFUITypographyRole TypographyRole) const
{
	switch (TypographyRole)
	{
	case ECFUITypographyRole::DisplayXL: return Typography.DisplayXL;
	case ECFUITypographyRole::DisplayL: return Typography.DisplayL;
	case ECFUITypographyRole::ValueM: return Typography.ValueM;
	case ECFUITypographyRole::ValueS: return Typography.ValueS;
	case ECFUITypographyRole::HeadingL: return Typography.HeadingL;
	case ECFUITypographyRole::HeadingM: return Typography.HeadingM;
	case ECFUITypographyRole::Body: return Typography.Body;
	case ECFUITypographyRole::Label: return Typography.Label;
	case ECFUITypographyRole::Caption: return Typography.Caption;
	default: return Typography.Body;
	}
}

// [v1.1.0] UI 또는 Numeric Family Role에 연결된 Composite UFont를 반환합니다.
UFont* UCFUIStyleData::ResolveFontAsset(const ECFUIFontFamilyRole FontFamilyRole) const
{
	return FontFamilyRole == ECFUIFontFamilyRole::Numeric ? FontAssets.NumericFontAsset.Get() : FontAssets.UIFontAsset.Get();
}

// [v1.1.0] 논리적 Font Weight를 Composite Font의 Typeface 이름으로 해석합니다.
FName UCFUIStyleData::ResolveTypefaceName(const ECFUIFontWeight FontWeight) const
{
	switch (FontWeight)
	{
	case ECFUIFontWeight::Medium: return FontAssets.MediumTypefaceName;
	case ECFUIFontWeight::SemiBold: return FontAssets.SemiBoldTypefaceName;
	case ECFUIFontWeight::Bold: return FontAssets.BoldTypefaceName;
	case ECFUIFontWeight::Regular:
	default:
		return FontAssets.RegularTypefaceName;
	}
}

// [v1.2.0] 실제 Texture/Material 경로 대신 Semantic ID로 등록된 Soft Icon Asset을 반환합니다.
TSoftObjectPtr<UObject> UCFUIStyleData::ResolveSemanticIconAsset(const FName SemanticId) const
{
	if (SemanticId.IsNone())
	{
		return TSoftObjectPtr<UObject>();
	}

	for (const FCFUISemanticIconEntry& IconEntry : IconSet.SemanticIcons)
	{
		if (IconEntry.SemanticId == SemanticId)
		{
			return IconEntry.IconAsset;
		}
	}

	return TSoftObjectPtr<UObject>();
}

// [v1.1.0] Font Family·Typography Role·Scale·안전 하한을 실제 Slate Font 정보로 해석합니다.
FSlateFontInfo UCFUIStyleData::ResolveSlateFontInfo(
	const ECFUIFontFamilyRole FontFamilyRole,
	const ECFUITypographyRole TypographyRole,
	const float TypographyScale,
	const int32 MinimumEffectiveFontSize) const
{
	// [v1.1.0] 요청된 Typography Role의 기본 크기와 논리 Weight입니다.
	const FCFUITypographyStyle TypographyStyle = ResolveTypography(TypographyRole);

	// [v1.1.0] 잘못된 음수 Scale을 차단한 뒤 화면 배율을 반영한 글자 크기입니다.
	const int32 ScaledFontSize = FMath::RoundToInt(static_cast<float>(TypographyStyle.FontSize) * FMath::Max(TypographyScale, 0.0f));

	// [v1.1.0] 1080p Role별 하한과 최소 1px을 함께 보장한 최종 글자 크기입니다.
	const int32 EffectiveFontSize = FMath::Max3(ScaledFontSize, MinimumEffectiveFontSize, 1);

	// [v1.1.0] 논리 Weight에 대응하는 Composite Font Typeface 이름입니다.
	const FName TypefaceName = ResolveTypefaceName(TypographyStyle.Weight);

	if (UFont* FontAsset = ResolveFontAsset(FontFamilyRole))
	{
		return FSlateFontInfo(FontAsset, EffectiveFontSize, TypefaceName);
	}

	// [v1.1.0] D1-09B Font Asset이 아직 없거나 로드에 실패한 경우 사용할 Engine 기본 Font입니다.
	return FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), EffectiveFontSize);
}

// [v1.0.0] 현재 Style Data가 최소 안전 하한과 승인 기본 계약을 위반하지 않는지 검증합니다.
bool UCFUIStyleData::ValidateStyleData(FString& OutFailureReason) const
{
	OutFailureReason.Reset();

		if (Typography.UIFontFamily.IsNone() || Typography.NumericFontFamily.IsNone())
	{
		OutFailureReason = TEXT("UI 또는 Numeric Font Family Role이 비어 있습니다.");
		return false;
	}

	if (FontAssets.RegularTypefaceName.IsNone() || FontAssets.MediumTypefaceName.IsNone() || FontAssets.SemiBoldTypefaceName.IsNone() || FontAssets.BoldTypefaceName.IsNone())
	{
		OutFailureReason = TEXT("Font Weight에 대응하는 Typeface 이름이 비어 있습니다.");
		return false;
	}

	// [v1.0.0] 공통 안전 하한을 일괄 검증할 모든 Typography Role 값입니다.
	const FCFUITypographyStyle TypographyRoles[] =
	{
		Typography.DisplayXL,
		Typography.DisplayL,
		Typography.ValueM,
		Typography.ValueS,
		Typography.HeadingL,
		Typography.HeadingM,
		Typography.Body,
		Typography.Label,
		Typography.Caption
	};
	for (const FCFUITypographyStyle& TypographyRole : TypographyRoles)
	{
		if (TypographyRole.FontSize <= 0 || TypographyRole.LineHeight < TypographyRole.FontSize)
		{
			OutFailureReason = TEXT("Typography Role의 FontSize 또는 LineHeight가 유효하지 않습니다.");
			return false;
		}
	}

	if (Spacing.MinimumHitSize < 48.0f)
	{
		OutFailureReason = TEXT("MinimumHitSize는 접근성 하한 48보다 작을 수 없습니다.");
		return false;
	}

	if (Shape.OutlineHairline < 1.0f || Shape.OutlineStandard < Shape.OutlineHairline)
	{
		OutFailureReason = TEXT("OutlineHairline은 1 이상이어야 하며 Standard Outline은 Hairline보다 작을 수 없습니다.");
		return false;
	}

	if (Motion.CriticalPulse.DurationSeconds < 0.5f || !Motion.CriticalPulse.bAllowLoop)
	{
		OutFailureReason = TEXT("Critical Pulse는 2Hz를 넘지 않도록 0.5초 이상이며 반복 허용 상태여야 합니다.");
		return false;
	}

	if (Motion.Hover.bAllowLoop || Motion.Pressed.bAllowLoop || Motion.State.bAllowLoop || Motion.Panel.bAllowLoop || Motion.Gauge.bAllowLoop || Motion.Alert.bAllowLoop)
	{
		OutFailureReason = TEXT("Critical Pulse 외 일반 UI Motion은 반복을 허용할 수 없습니다.");
		return false;
	}

	if (StatusBarStyle.Armor.VisualSegmentCount != 10)
	{
		OutFailureReason = TEXT("Armor Bar의 기본 시각 분절 수는 10이어야 하며 방향별 Armor 의미와 분리됩니다.");
		return false;
	}

	if (AlertStyle.MaximumVisibleAlerts <= 0 || AlertStyle.MaximumVisibleAlerts > 3)
	{
		OutFailureReason = TEXT("Alert Feed의 기본 동시 표시 수는 1~3 범위여야 합니다.");
		return false;
	}

		if (IconSet.BaseGridSize <= 0.0f || IconSet.IconSmall <= 0.0f || IconSet.IconMedium <= 0.0f || IconSet.IconLarge <= 0.0f)
	{
		OutFailureReason = TEXT("Semantic Icon Grid와 기본 표시 크기는 0보다 커야 합니다.");
		return false;
	}

	// [v1.2.0] 등록된 Semantic Icon ID의 중복 여부를 추적하는 집합입니다.
	TSet<FName> RegisteredSemanticIds;
	for (const FCFUISemanticIconEntry& IconEntry : IconSet.SemanticIcons)
	{
		if (IconEntry.SemanticId.IsNone())
		{
			OutFailureReason = TEXT("Semantic Icon 목록에 비어 있는 ID가 있습니다.");
			return false;
		}
		if (IconEntry.IconAsset.IsNull())
		{
			OutFailureReason = FString::Printf(TEXT("Semantic Icon '%s'의 Asset 참조가 비어 있습니다."), *IconEntry.SemanticId.ToString());
			return false;
		}
		if (RegisteredSemanticIds.Contains(IconEntry.SemanticId))
		{
			OutFailureReason = FString::Printf(TEXT("Semantic Icon ID '%s'가 중복 등록됐습니다."), *IconEntry.SemanticId.ToString());
			return false;
		}
		RegisteredSemanticIds.Add(IconEntry.SemanticId);
	}

	return true;
}
