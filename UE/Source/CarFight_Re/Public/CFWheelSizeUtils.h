// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFWheelSizeUtils.h
// Version: v1.1.0
// Date: 2026-08-28
// Description: WSA Wheel StaticMesh Bounds + USER Socket Scale의 공용 deterministic 크기 계산/검증 계약입니다.
// Scope: canonical 100x25x100 bounds, Socket Scale, base radius, derived radius/width와 axle-size 비교를 담당합니다.
// Changelog:
// - v1.1.0: WSA-P0-04 Builder가 Runtime/Resolver와 같은 canonical Wheel Bounds 계약을 재사용하도록 ValidateCanonicalWheelBounds 추가.
// - v1.0.0: Wheel Size Authority 공용 계산 helper 최초 추가.
// Migration:
// - Runtime/Resolver가 같은 계산식을 공유하기 위한 순수 값 계산 유틸이며 UObject/Asset 조회, 자동 보정, Save를 수행하지 않습니다.

#pragma once

#include "CoreMinimal.h"

/** Wheel Bounds와 USER Socket Scale에서 파생한 최종 시각/물리 크기입니다. */
struct CARFIGHT_RE_API FCFDerivedWheelSize
{
	// Chaos WheelRadius에 저장할 최종 반지름 cm입니다.
	float RadiusCm = 0.0f;

	// Chaos WheelWidth에 저장할 최종 전체 폭 cm입니다.
	float WidthCm = 0.0f;

	// Wheel_Mesh_*에 exact set할 USER-authored 시각 Scale입니다.
	FVector VisualScale = FVector::OneVector;
};

/** WSA Wheel Size 계산을 Runtime/Editor가 동일하게 사용하기 위한 순수 helper입니다. */
class CARFIGHT_RE_API FCFWheelSizeUtils
{
public:
	// SocketScaleFromChassis가 사용하는 canonical 원본 Wheel 직경 cm입니다.
	static constexpr double CanonicalWheelDiameterCm = 100.0;

	// SocketScaleFromChassis가 사용하는 canonical 원본 Wheel 폭 cm입니다.
	static constexpr double CanonicalWheelWidthCm = 25.0;

	// canonical source dimension에 허용할 기본 절대 오차 cm입니다.
	static constexpr double DefaultCanonicalBoundsToleranceCm = 0.1;

	// Wheel bounds center가 hub origin에 있다고 볼 기본 절대 오차 cm입니다.
	static constexpr double DefaultBoundsCenterToleranceCm = 0.1;

	// Socket Scale의 X/Z radial equality에 허용할 기본 절대 오차입니다.
	static constexpr double DefaultRadialScaleTolerance = 0.001;

	// Wheel Bounds의 X/Z radial extent equality에 허용할 기본 절대 오차 cm입니다.
	static constexpr double DefaultCircularBoundsToleranceCm = 0.1;

	// 파생 Radius/Width 좌우 비교에 허용할 기본 절대 오차 cm입니다.
	static constexpr float DefaultSizeCompatibilityToleranceCm = 0.01f;

	// Wheel local Bounds가 canonical 100x25x100cm이고 center가 hub origin인지 검증합니다.
	static bool ValidateCanonicalWheelBounds(
		const FVector& BoundsOrigin,
		const FVector& BoundsExtent,
		FString& OutError,
		double BoundsToleranceCm = DefaultCanonicalBoundsToleranceCm,
		double BoundsCenterToleranceCm = DefaultBoundsCenterToleranceCm);

	// USER Socket Scale이 finite/positive이고 X/Z radial scale이 같은지 검증합니다.
	static bool ValidateWheelSocketScale(
		const FVector& SocketScale,
		FString& OutError,
		double RadialScaleTolerance = DefaultRadialScaleTolerance);

	// Wheel local BoundsExtent의 X/Z가 원형성을 만족할 때 평균 radial extent를 base radius cm로 반환합니다.
	static bool MeasureBaseWheelRadius(
		const FVector& BoundsExtent,
		float& OutBaseRadiusCm,
		FString& OutError,
		double CircularBoundsToleranceCm = DefaultCircularBoundsToleranceCm);

	// Wheel local BoundsExtent와 USER Socket Scale에서 최종 Radius/Width/VisualScale을 계산합니다.
	static bool DeriveWheelSizeFromBoundsAndScale(
		const FVector& BoundsExtent,
		const FVector& SocketScale,
		FCFDerivedWheelSize& OutWheelSize,
		FString& OutError,
		double RadialScaleTolerance = DefaultRadialScaleTolerance,
		double CircularBoundsToleranceCm = DefaultCircularBoundsToleranceCm);

	// 두 per-wheel derived size가 axle 단위 동일 크기로 취급 가능한지 비교합니다.
	static bool AreWheelSizesCompatible(
		const FCFDerivedWheelSize& LeftWheelSize,
		const FCFDerivedWheelSize& RightWheelSize,
		float RadiusToleranceCm = DefaultSizeCompatibilityToleranceCm,
		float WidthToleranceCm = DefaultSizeCompatibilityToleranceCm);
};
