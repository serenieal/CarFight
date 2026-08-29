// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFWheelSizeUtils.cpp
// Version: v1.1.0
// Date: 2026-08-28
// Description: WSA Wheel Size 공용 deterministic 계산/Canonical Bounds 검증 구현입니다.
// Scope: 외부 상태 조회나 mutation 없이 Bounds/SocketScale 값만 검증·계산합니다.
// Changelog:
// - v1.1.0: canonical 100x25x100cm + centered Bounds validator 추가.
// - v1.0.0: Validate/Measure/Derive/Axle compatibility helper 최초 구현.

#include "CFWheelSizeUtils.h"

namespace CFWheelSizeUtilsPrivate
{
	// FVector의 세 성분이 모두 finite인지 반환합니다.
	bool IsFiniteVector(const FVector& Value)
	{
		return FMath::IsFinite(Value.X)
			&& FMath::IsFinite(Value.Y)
			&& FMath::IsFinite(Value.Z);
	}

	// 파생 size 두 scalar가 유효한 양수인지 반환합니다.
	bool IsFinitePositiveSize(const float Value)
	{
		return FMath::IsFinite(Value) && Value > 0.0f;
	}
}

bool FCFWheelSizeUtils::ValidateCanonicalWheelBounds(
	const FVector& BoundsOrigin,
	const FVector& BoundsExtent,
	FString& OutError,
	const double BoundsToleranceCm,
	const double BoundsCenterToleranceCm)
{
	OutError.Reset();

	if (!FMath::IsFinite(BoundsToleranceCm) || BoundsToleranceCm < 0.0
		|| !FMath::IsFinite(BoundsCenterToleranceCm) || BoundsCenterToleranceCm < 0.0)
	{
		OutError = TEXT("Canonical Wheel Bounds tolerance가 유효한 0 이상 finite 값이 아닙니다.");
		return false;
	}
	if (!CFWheelSizeUtilsPrivate::IsFiniteVector(BoundsOrigin)
		|| !CFWheelSizeUtilsPrivate::IsFiniteVector(BoundsExtent))
	{
		OutError = TEXT("Canonical Wheel Bounds origin/extent에 non-finite 성분이 있습니다.");
		return false;
	}

	const double FullSizeX = FMath::Abs(BoundsExtent.X) * 2.0;
	const double FullSizeY = FMath::Abs(BoundsExtent.Y) * 2.0;
	const double FullSizeZ = FMath::Abs(BoundsExtent.Z) * 2.0;
	if (!FMath::IsNearlyEqual(FullSizeX, CanonicalWheelDiameterCm, BoundsToleranceCm)
		|| !FMath::IsNearlyEqual(FullSizeY, CanonicalWheelWidthCm, BoundsToleranceCm)
		|| !FMath::IsNearlyEqual(FullSizeZ, CanonicalWheelDiameterCm, BoundsToleranceCm))
	{
		OutError = FString::Printf(
			TEXT("Wheel Bounds가 canonical %.1fx%.1fx%.1fcm가 아닙니다. Current=%.4fx%.4fx%.4fcm"),
			CanonicalWheelDiameterCm,
			CanonicalWheelWidthCm,
			CanonicalWheelDiameterCm,
			FullSizeX,
			FullSizeY,
			FullSizeZ);
		return false;
	}

	if (FMath::Abs(BoundsOrigin.X) > BoundsCenterToleranceCm
		|| FMath::Abs(BoundsOrigin.Y) > BoundsCenterToleranceCm
		|| FMath::Abs(BoundsOrigin.Z) > BoundsCenterToleranceCm)
	{
		OutError = FString::Printf(
			TEXT("Wheel Bounds center가 hub origin tolerance를 벗어납니다. Current=%s Tolerance=%.4fcm"),
			*BoundsOrigin.ToCompactString(),
			BoundsCenterToleranceCm);
		return false;
	}

	return true;
}

bool FCFWheelSizeUtils::ValidateWheelSocketScale(
	const FVector& SocketScale,
	FString& OutError,
	const double RadialScaleTolerance)
{
	OutError.Reset();

	if (!FMath::IsFinite(RadialScaleTolerance) || RadialScaleTolerance < 0.0)
	{
		OutError = TEXT("Wheel Socket radial scale tolerance가 유효한 0 이상 finite 값이 아닙니다.");
		return false;
	}

	if (!CFWheelSizeUtilsPrivate::IsFiniteVector(SocketScale))
	{
		OutError = TEXT("Wheel Socket Scale에 non-finite 성분이 있습니다.");
		return false;
	}

	if (SocketScale.X <= 0.0 || SocketScale.Y <= 0.0 || SocketScale.Z <= 0.0)
	{
		OutError = TEXT("Wheel Socket Scale의 X/Y/Z는 모두 0보다 커야 합니다.");
		return false;
	}

	if (!FMath::IsNearlyEqual(SocketScale.X, SocketScale.Z, RadialScaleTolerance))
	{
		OutError = FString::Printf(
			TEXT("Wheel Socket radial Scale X/Z가 일치하지 않습니다. X=%.6f Z=%.6f Tolerance=%.6f"),
			SocketScale.X,
			SocketScale.Z,
			RadialScaleTolerance);
		return false;
	}

	return true;
}

bool FCFWheelSizeUtils::MeasureBaseWheelRadius(
	const FVector& BoundsExtent,
	float& OutBaseRadiusCm,
	FString& OutError,
	const double CircularBoundsToleranceCm)
{
	OutBaseRadiusCm = 0.0f;
	OutError.Reset();

	if (!FMath::IsFinite(CircularBoundsToleranceCm) || CircularBoundsToleranceCm < 0.0)
	{
		OutError = TEXT("Wheel Bounds circular tolerance가 유효한 0 이상 finite 값이 아닙니다.");
		return false;
	}

	if (!CFWheelSizeUtilsPrivate::IsFiniteVector(BoundsExtent))
	{
		OutError = TEXT("Wheel BoundsExtent에 non-finite 성분이 있습니다.");
		return false;
	}

	const double RadialExtentX = FMath::Abs(BoundsExtent.X);
	const double RadialExtentZ = FMath::Abs(BoundsExtent.Z);
	if (RadialExtentX <= 0.0 || RadialExtentZ <= 0.0)
	{
		OutError = TEXT("Wheel BoundsExtent X/Z radial 크기는 모두 0보다 커야 합니다.");
		return false;
	}

	if (!FMath::IsNearlyEqual(RadialExtentX, RadialExtentZ, CircularBoundsToleranceCm))
	{
		OutError = FString::Printf(
			TEXT("Wheel Bounds X/Z radial extent가 원형 tolerance를 벗어납니다. X=%.6fcm Z=%.6fcm Tolerance=%.6fcm"),
			RadialExtentX,
			RadialExtentZ,
			CircularBoundsToleranceCm);
		return false;
	}

	OutBaseRadiusCm = static_cast<float>((RadialExtentX + RadialExtentZ) * 0.5);
	if (!CFWheelSizeUtilsPrivate::IsFinitePositiveSize(OutBaseRadiusCm))
	{
		OutBaseRadiusCm = 0.0f;
		OutError = TEXT("Wheel Bounds에서 계산한 base radius가 float 범위의 유효한 양수가 아닙니다.");
		return false;
	}

	return true;
}

bool FCFWheelSizeUtils::DeriveWheelSizeFromBoundsAndScale(
	const FVector& BoundsExtent,
	const FVector& SocketScale,
	FCFDerivedWheelSize& OutWheelSize,
	FString& OutError,
	const double RadialScaleTolerance,
	const double CircularBoundsToleranceCm)
{
	OutWheelSize = FCFDerivedWheelSize();
	OutError.Reset();

	if (!ValidateWheelSocketScale(SocketScale, OutError, RadialScaleTolerance))
	{
		return false;
	}

	if (!CFWheelSizeUtilsPrivate::IsFiniteVector(BoundsExtent) || FMath::Abs(BoundsExtent.Y) <= 0.0)
	{
		OutError = TEXT("Wheel BoundsExtent는 finite이며 Y width extent가 0보다 커야 합니다.");
		return false;
	}

	float BaseRadiusCm = 0.0f;
	if (!MeasureBaseWheelRadius(BoundsExtent, BaseRadiusCm, OutError, CircularBoundsToleranceCm))
	{
		return false;
	}

	const double DerivedRadiusCm = static_cast<double>(BaseRadiusCm) * SocketScale.X;
	const double DerivedWidthCm = FMath::Abs(BoundsExtent.Y) * 2.0 * SocketScale.Y;
	if (!FMath::IsFinite(DerivedRadiusCm) || DerivedRadiusCm <= 0.0
		|| !FMath::IsFinite(DerivedWidthCm) || DerivedWidthCm <= 0.0)
	{
		OutError = TEXT("Wheel Bounds와 Socket Scale에서 파생한 Radius/Width가 유효한 양수가 아닙니다.");
		return false;
	}

	OutWheelSize.RadiusCm = static_cast<float>(DerivedRadiusCm);
	OutWheelSize.WidthCm = static_cast<float>(DerivedWidthCm);
	if (!CFWheelSizeUtilsPrivate::IsFinitePositiveSize(OutWheelSize.RadiusCm)
		|| !CFWheelSizeUtilsPrivate::IsFinitePositiveSize(OutWheelSize.WidthCm))
	{
		OutWheelSize = FCFDerivedWheelSize();
		OutError = TEXT("Wheel Radius/Width가 float 저장 범위를 벗어났습니다.");
		return false;
	}

	OutWheelSize.VisualScale = SocketScale;
	return true;
}

bool FCFWheelSizeUtils::AreWheelSizesCompatible(
	const FCFDerivedWheelSize& LeftWheelSize,
	const FCFDerivedWheelSize& RightWheelSize,
	const float RadiusToleranceCm,
	const float WidthToleranceCm)
{
	if (!FMath::IsFinite(RadiusToleranceCm) || RadiusToleranceCm < 0.0f
		|| !FMath::IsFinite(WidthToleranceCm) || WidthToleranceCm < 0.0f)
	{
		return false;
	}

	if (!CFWheelSizeUtilsPrivate::IsFinitePositiveSize(LeftWheelSize.RadiusCm)
		|| !CFWheelSizeUtilsPrivate::IsFinitePositiveSize(LeftWheelSize.WidthCm)
		|| !CFWheelSizeUtilsPrivate::IsFinitePositiveSize(RightWheelSize.RadiusCm)
		|| !CFWheelSizeUtilsPrivate::IsFinitePositiveSize(RightWheelSize.WidthCm))
	{
		return false;
	}

	return FMath::IsNearlyEqual(LeftWheelSize.RadiusCm, RightWheelSize.RadiusCm, RadiusToleranceCm)
		&& FMath::IsNearlyEqual(LeftWheelSize.WidthCm, RightWheelSize.WidthCm, WidthToleranceCm);
}
