// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-10
// Description: UCFStyledWidgetBase 외부 Visual Context Bridge 구현
// Scope: D1-10A 외부 Context 중복 방지와 Geometry/Typography 해석만 수행하며 Gameplay·Asset Load 책임을 갖지 않습니다.
// Changelog:
// - v1.0.0: Set/Refresh/Get/Scale/Typography 해석과 Native CDO Fallback 구현.
// Migration:
// - Content 경로를 직접 Load하지 않으며 D1-09B Config 해석은 UCFUISubsystem에 남습니다.

#include "UI/CFStyledWidgetBase.h"

// [v1.0.0] 외부 UI Presenter/Root가 해석한 Style·Density·Scale Context를 한 번에 적용하고 실제 변경이 있을 때만 Visual Refresh를 요청합니다.
void UCFStyledWidgetBase::SetUIVisualContext(
	UCFUIStyleData* InStyleData,
	UCFUIDensityData* InDensityData,
	float InGeometryScale,
	float InTypographyScale,
	const FCFUITypographyFloor& InTypographyFloor)
{
	// [v1.0.0] 0 이하 Scale 입력을 Native Safe Fallback 1.0으로 정규화한 Geometry Scale입니다.
	const float ResolvedGeometryScale = InGeometryScale > 0.0f ? InGeometryScale : 1.0f;

	// [v1.0.0] 0 이하 Scale 입력을 Native Safe Fallback 1.0으로 정규화한 Typography Scale입니다.
	const float ResolvedTypographyScale = InTypographyScale > 0.0f ? InTypographyScale : 1.0f;

	// [v1.0.0] 동일 Visual Context 재적용 여부입니다.
	const bool bSameContext =
		StyleData == InStyleData
		&& DensityData == InDensityData
		&& FMath::IsNearlyEqual(GeometryScale, ResolvedGeometryScale)
		&& FMath::IsNearlyEqual(TypographyScale, ResolvedTypographyScale)
		&& IsSameTypographyFloor(TypographyFloor, InTypographyFloor);

	if (bSameContext)
	{
		return;
	}

	StyleData = InStyleData;
	DensityData = InDensityData;
	GeometryScale = ResolvedGeometryScale;
	TypographyScale = ResolvedTypographyScale;
	TypographyFloor = InTypographyFloor;
	RefreshUIVisualStyle();
}

// [v1.0.0] 현재 Visual Context를 사용해 Blueprint Visual Layer 갱신 Event를 다시 호출합니다.
void UCFStyledWidgetBase::RefreshUIVisualStyle()
{
	++VisualContextRevision;
	OnUIVisualStyleChanged();
}

// [v1.0.0] 현재 적용된 Style Data를 반환하며 외부 Context가 Null이면 Native CDO Fallback을 반환합니다.
const UCFUIStyleData* UCFStyledWidgetBase::GetUIStyleData() const
{
	return StyleData ? StyleData.Get() : GetDefault<UCFUIStyleData>();
}

// [v1.0.0] 현재 적용된 Density Data를 반환하며 외부 Context가 Null이면 Native Standard CDO Fallback을 반환합니다.
const UCFUIDensityData* UCFStyledWidgetBase::GetUIDensityData() const
{
	return DensityData ? DensityData.Get() : GetDefault<UCFUIDensityData>();
}

// [v1.0.0] Panel 내부 Design Token에 적용할 유효 Geometry Scale을 반환합니다.
float UCFStyledWidgetBase::GetResolvedGeometryScale() const
{
	return GeometryScale > 0.0f ? GeometryScale : 1.0f;
}

// [v1.0.0] Design Unit 간격을 현재 Geometry Scale에 맞춘 실효 값으로 변환합니다.
float UCFStyledWidgetBase::ResolveScaledSpacing(float DesignUnits) const
{
	return FMath::Max(0.0f, DesignUnits) * GetResolvedGeometryScale();
}

// [v1.0.0] Font Family·Typography Role을 현재 Typography Scale과 Role별 안전 하한이 적용된 Slate Font로 해석합니다.
FSlateFontInfo UCFStyledWidgetBase::ResolveTypographyFont(ECFUIFontFamilyRole FontFamilyRole, ECFUITypographyRole TypographyRole) const
{
	// [v1.0.0] 외부 Style이 없을 때도 Native Fallback을 제공하는 실제 Style Data입니다.
	const UCFUIStyleData* ResolvedStyleData = GetUIStyleData();
	if (!ResolvedStyleData)
	{
		return FSlateFontInfo();
	}

	// [v1.0.0] 0 이하 입력을 방어한 실제 Typography Scale입니다.
	const float ResolvedTypographyScale = TypographyScale > 0.0f ? TypographyScale : 1.0f;
	return ResolvedStyleData->ResolveSlateFontInfo(
		FontFamilyRole,
		TypographyRole,
		ResolvedTypographyScale,
		ResolveMinimumTypographySize(TypographyRole));
}

// [v1.0.0] 동일 Context 중복 적용 방지와 테스트 Evidence에 사용할 Visual Context Revision을 반환합니다.
int32 UCFStyledWidgetBase::GetVisualContextRevision() const
{
	return VisualContextRevision;
}

// [v1.0.0] 두 Typography Floor 구조가 모든 Role에서 같은지 비교합니다.
bool UCFStyledWidgetBase::IsSameTypographyFloor(const FCFUITypographyFloor& Left, const FCFUITypographyFloor& Right) const
{
	return Left.DisplayXL == Right.DisplayXL
		&& Left.DisplayL == Right.DisplayL
		&& Left.ValueM == Right.ValueM
		&& Left.ValueS == Right.ValueS
		&& Left.HeadingL == Right.HeadingL
		&& Left.HeadingM == Right.HeadingM
		&& Left.Body == Right.Body
		&& Left.Label == Right.Label
		&& Left.Caption == Right.Caption;
}

// [v1.0.0] 지정 Typography Role의 현재 최소 실효 Font Size를 반환합니다.
int32 UCFStyledWidgetBase::ResolveMinimumTypographySize(ECFUITypographyRole TypographyRole) const
{
	switch (TypographyRole)
	{
	case ECFUITypographyRole::DisplayXL:
		return TypographyFloor.DisplayXL;
	case ECFUITypographyRole::DisplayL:
		return TypographyFloor.DisplayL;
	case ECFUITypographyRole::ValueM:
		return TypographyFloor.ValueM;
	case ECFUITypographyRole::ValueS:
		return TypographyFloor.ValueS;
	case ECFUITypographyRole::HeadingL:
		return TypographyFloor.HeadingL;
	case ECFUITypographyRole::HeadingM:
		return TypographyFloor.HeadingM;
	case ECFUITypographyRole::Body:
		return TypographyFloor.Body;
	case ECFUITypographyRole::Label:
		return TypographyFloor.Label;
	case ECFUITypographyRole::Caption:
	default:
		return TypographyFloor.Caption;
	}
}
