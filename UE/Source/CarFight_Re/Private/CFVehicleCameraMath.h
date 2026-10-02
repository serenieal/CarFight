// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleCameraMath.h
// Version: v1.2.0
// Date: 2026-09-30
// Description: VCFX normalized Driving Camera FX의 overspeed-aware ratio / Lateral Presentation 계산을 공유하는 Private helper입니다.
// Changelog:
// - v1.2.0: SpeedRatio의 기본 1.0 parity는 유지하면서 runtime이 명시한 MaxSpeedPresentationRatio까지 overspeed headroom을 허용.
// - v1.1.0: 차체 Roll/Yaw motion intensity, 자유시점 signed ViewAlignment, 최종 Lateral Presentation scale pure helper를 추가.
// - v1.0.0: 차량별 ReferenceSpeed 기반 Speed/Acceleration/Braking/Lateral 정규화와 invalid-input exact0 fail-safe 추가.
// Migration:
// - Public API / Blueprint 노출을 추가하지 않습니다.
// - 기존 LateralRate는 좌우 방향/기본 요구량 authority로 유지하고 새 helper는 Presentation Roll 강도와 화면 기준 방향만 조절합니다.
// - invalid body/view 입력은 Lateral Presentation scale exact0으로 fail-safe합니다.

#pragma once

#include "CoreMinimal.h"

namespace CFVehicleCameraMath
{
	// Reference speed가 normalized Camera FX 분모로 사용할 수 있는 finite positive 값인지 확인합니다.
	inline bool IsValidReferenceSpeed(const float ReferenceSpeedCmPerSec)
	{
		return FMath::IsFinite(ReferenceSpeedCmPerSec)
			&& ReferenceSpeedCmPerSec > KINDA_SMALL_NUMBER;
	}

	// Vector의 모든 성분이 finite인지 확인합니다.
	inline bool IsFiniteVector(const FVector& Value)
	{
		return FMath::IsFinite(Value.X)
			&& FMath::IsFinite(Value.Y)
			&& FMath::IsFinite(Value.Z);
	}

	// 현재 평면 속력을 차량별 ReferenceSpeed에 대해 0~MaxSpeedPresentationRatio SpeedRatio로 정규화합니다.
	// 세 번째 인자를 생략하면 기존 0~1 parity를 그대로 유지합니다.
	inline float NormalizeSpeedRatio(
		const float CurrentPlanarSpeedCmPerSec,
		const float ReferenceSpeedCmPerSec,
		const float MaxSpeedPresentationRatio = 1.0f)
	{
		if (!IsValidReferenceSpeed(ReferenceSpeedCmPerSec)
			|| !FMath::IsFinite(CurrentPlanarSpeedCmPerSec)
			|| !FMath::IsFinite(MaxSpeedPresentationRatio)
			|| MaxSpeedPresentationRatio < 1.0f)
		{
			return 0.0f;
		}

		return FMath::Clamp(
			CurrentPlanarSpeedCmPerSec / ReferenceSpeedCmPerSec,
			0.0f,
			MaxSpeedPresentationRatio);
	}

	// 기준속도 100%를 넘은 SpeedRatio를 0~1 OverspeedPhase로 변환합니다.
	inline float CalculateOverspeedPhase(
		const float SpeedRatio,
		const float MaxSpeedPresentationRatio)
	{
		if (!FMath::IsFinite(SpeedRatio)
			|| !FMath::IsFinite(MaxSpeedPresentationRatio)
			|| MaxSpeedPresentationRatio <= 1.0f)
		{
			return 0.0f;
		}

		return FMath::Clamp(
			(SpeedRatio - 1.0f) / (MaxSpeedPresentationRatio - 1.0f),
			0.0f,
			1.0f);
	}

	// 진행축 속도 증가율을 차량별 ReferenceSpeed 대비 0~1 AccelerationRate로 정규화합니다.
	inline float NormalizeAccelerationRate(
		const float LongitudinalSpeedRateCmPerSec2,
		const float ReferenceSpeedCmPerSec)
	{
		if (!IsValidReferenceSpeed(ReferenceSpeedCmPerSec)
			|| !FMath::IsFinite(LongitudinalSpeedRateCmPerSec2))
		{
			return 0.0f;
		}

		return FMath::Clamp(
			FMath::Max(
				LongitudinalSpeedRateCmPerSec2 / ReferenceSpeedCmPerSec,
				0.0f),
			0.0f,
			1.0f);
	}

	// 진행축 속도 감소율을 차량별 ReferenceSpeed 대비 0~1 BrakingRate로 정규화합니다.
	inline float NormalizeBrakingRate(
		const float LongitudinalSpeedRateCmPerSec2,
		const float ReferenceSpeedCmPerSec)
	{
		if (!IsValidReferenceSpeed(ReferenceSpeedCmPerSec)
			|| !FMath::IsFinite(LongitudinalSpeedRateCmPerSec2))
		{
			return 0.0f;
		}

		return FMath::Clamp(
			FMath::Max(
				-LongitudinalSpeedRateCmPerSec2 / ReferenceSpeedCmPerSec,
				0.0f),
			0.0f,
			1.0f);
	}

	// signed 횡가속도를 차량별 ReferenceSpeed 대비 -1~1 LateralRate로 정규화합니다.
	inline float NormalizeLateralRate(
		const float LateralAccelerationCmPerSec2,
		const float ReferenceSpeedCmPerSec)
	{
		if (!IsValidReferenceSpeed(ReferenceSpeedCmPerSec)
			|| !FMath::IsFinite(LateralAccelerationCmPerSec2))
		{
			return 0.0f;
		}

		return FMath::Clamp(
			LateralAccelerationCmPerSec2 / ReferenceSpeedCmPerSec,
			-1.0f,
			1.0f);
	}

	// 절대 Motion 값이 full-intensity 기준에서 차지하는 비율을 0~1로 정규화합니다.
	inline float NormalizeMotionMagnitude(
		const float MotionMagnitude,
		const float FullIntensityMagnitude)
	{
		if (!FMath::IsFinite(MotionMagnitude)
			|| !FMath::IsFinite(FullIntensityMagnitude)
			|| FullIntensityMagnitude <= KINDA_SMALL_NUMBER)
		{
			return 0.0f;
		}

		return FMath::Clamp(
			FMath::Abs(MotionMagnitude) / FullIntensityMagnitude,
			0.0f,
			1.0f);
	}

	// 실제 차체 Roll 각도와 Roll/Yaw 각속도 중 가장 강한 신호를 0~1 Lateral body-motion intensity로 계산합니다.
	inline float CalculateLateralBodyMotionIntensity(
		const float BodyRollDeg,
		const float BodyRollRateDegPerSec,
		const float BodyYawRateDegPerSec,
		const float RollAngleForFullIntensityDeg,
		const float RollRateForFullIntensityDegPerSec,
		const float YawRateForFullIntensityDegPerSec)
	{
		// 차체 Roll 각도 기반 Motion intensity입니다.
		const float RollAngleIntensity = NormalizeMotionMagnitude(
			BodyRollDeg,
			RollAngleForFullIntensityDeg);

		// 차체 Roll 각속도 기반 Motion intensity입니다.
		const float RollRateIntensity = NormalizeMotionMagnitude(
			BodyRollRateDegPerSec,
			RollRateForFullIntensityDegPerSec);

		// 차체 Yaw 각속도 기반 Motion intensity입니다.
		const float YawRateIntensity = NormalizeMotionMagnitude(
			BodyYawRateDegPerSec,
			YawRateForFullIntensityDegPerSec);

		return FMath::Max(
			RollAngleIntensity,
			FMath::Max(RollRateIntensity, YawRateIntensity));
	}

	// 차량 수평 전방과 현재 수평 View 전방의 signed 정렬값을 -1~1로 계산합니다.
	inline float CalculateSignedViewAlignment(
		const FVector& VehicleForwardDirection,
		const FVector& ViewForwardDirection)
	{
		if (!IsFiniteVector(VehicleForwardDirection)
			|| !IsFiniteVector(ViewForwardDirection))
		{
			return 0.0f;
		}

		// Pitch 영향을 제거한 차량 수평 전방 벡터입니다.
		FVector VehicleForwardXY(
			VehicleForwardDirection.X,
			VehicleForwardDirection.Y,
			0.0f);

		// Pitch 영향을 제거한 현재 View 수평 전방 벡터입니다.
		FVector ViewForwardXY(
			ViewForwardDirection.X,
			ViewForwardDirection.Y,
			0.0f);

		if (VehicleForwardXY.IsNearlyZero()
			|| ViewForwardXY.IsNearlyZero())
		{
			return 0.0f;
		}

		VehicleForwardXY.Normalize();
		ViewForwardXY.Normalize();

		return FMath::Clamp(
			FVector::DotProduct(VehicleForwardXY, ViewForwardXY),
			-1.0f,
			1.0f);
	}

	// 차체 움직임·자유시점 정렬·상태 감쇠를 signed 최종 Lateral Presentation 배율로 합성합니다.
	inline float CalculateLateralPresentationScale(
		const float BodyMotionIntensity,
		const float SignedViewAlignment,
		const float MotionFXScale)
	{
		if (!FMath::IsFinite(BodyMotionIntensity)
			|| !FMath::IsFinite(SignedViewAlignment)
			|| !FMath::IsFinite(MotionFXScale))
		{
			return 0.0f;
		}

		// 0~1 범위로 안전 제한한 실제 차체 Motion 강도입니다.
		const float SafeBodyMotionIntensity = FMath::Clamp(
			BodyMotionIntensity,
			0.0f,
			1.0f);

		// -1~1 범위로 안전 제한한 자유시점 signed 정렬값입니다.
		const float SafeViewAlignment = FMath::Clamp(
			SignedViewAlignment,
			-1.0f,
			1.0f);

		// 0~1 범위로 안전 제한한 기존 Motion 상태 감쇠 배율입니다.
		const float SafeMotionFXScale = FMath::Clamp(
			MotionFXScale,
			0.0f,
			1.0f);

		return SafeBodyMotionIntensity
			* SafeViewAlignment
			* SafeMotionFXScale;
	}
}
